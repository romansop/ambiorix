/****************************************************************************
**
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
** SPDX-FileCopyrightText: Copyright (c) 2023 SoftAtHome
**
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice,
** this list of conditions and the following disclaimer.
**
** 2. Redistributions in binary form must reproduce the above copyright notice,
** this list of conditions and the following disclaimer in the documentation
** and/or other materials provided with the distribution.
**
** Subject to the terms and conditions of this license, each copyright holder
** and contributor hereby grants to those receiving rights under this license
** a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable
** (except for failure to satisfy the conditions of this license) patent license
** to make, have made, use, offer to sell, sell, import, and otherwise transfer
** this software, where such license applies only to those patent claims, already
** acquired or hereafter acquired, licensable by such copyright holder or contributor
** that are necessarily infringed by:
**
** (a) their Contribution(s) (the licensed copyrights of copyright holders and
** non-copyrightable additions of contributors, in source or binary form) alone;
** or
**
** (b) combination of their Contribution(s) with the work of authorship to which
** such Contribution(s) was added by such copyright holder or contributor, if,
** at the time the Contribution is added, such addition causes such combination
** to be necessarily infringed. The patent license shall not apply to any other
** combinations which include the Contribution.
**
** Except as expressly stated above, no rights or licenses from any copyright
** holder or contributor is granted under this license, whether expressly, by
** implication, estoppel or otherwise.
**
** DISCLAIMER
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
** OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
** USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "string.h"
#include "amxb_rbus.h"

static void amxb_rbus_add_param(amxc_var_t* params, const char* object, const char* name, rbusValue_t val) {
    uint32_t length = strlen(object);
    amxc_var_t* current = amxc_var_get_key(params, object, AMXC_VAR_FLAG_DEFAULT);
    amxc_var_t* param = NULL;
    amxd_path_t param_path;
    char* part = NULL;

    amxd_path_init(&param_path, name + length);
    part = amxd_path_get_first(&param_path, true);
    while(part != NULL) {
        amxc_var_t* sub = NULL;
        int partlen = strlen(part);
        if(part[partlen - 1] == '.') {
            part[partlen - 1] = 0;
        }
        if(part[0] == 0) {
            free(part);
            part = amxd_path_get_first(&param_path, true);
            continue;
        }
        sub = amxc_var_get_key(current, part, AMXC_VAR_FLAG_DEFAULT);
        if(sub == NULL) {
            current = amxc_var_add_key(amxc_htable_t, current, part, NULL);
        } else {
            current = sub;
        }
        free(part);
        part = amxd_path_get_first(&param_path, true);
    }

    param = amxc_var_add_new_key(current, amxd_path_get_param(&param_path));
    amxb_rbus_value_to_var(param, val);

    amxd_path_clean(&param_path);
}

static int amxb_rbus_fetch_params(amxb_rbus_t* amxb_rbus_ctx, const char* object, amxc_var_t* params) {
    int rv = 0;
    int num_vals = 0;
    rbusProperty_t output_vals = NULL;
    rbusProperty_t next = NULL;
    const char* input_param[1] = {0};

    amxc_var_set_type(params, AMXC_VAR_ID_HTABLE);

    input_param[0] = object;
    // Fetch the full tree starting from the provided object path
    rv = rbus_getExt(amxb_rbus_ctx->handle, 1, input_param, &num_vals, &output_vals);
    rv = amxb_rbus_translate_rbus2status(rv);
    when_failed(rv, exit);

    amxc_var_add_key(amxc_htable_t, params, object, NULL);
    next = output_vals;
    // Loop over all retrieved parameters and add them to the variant structure
    // in a hierarchical way. This will make it possible to apply any
    // search path parts and filter out the non-matching instances.
    for(int i = 0; i < num_vals; i++) {
        rbusValue_t val = rbusProperty_GetValue(next);
        const char* name = rbusProperty_GetName(next);
        amxb_rbus_add_param(params, object, name, val);
        next = rbusProperty_GetNext(next);
    }

    rbusProperty_Release(output_vals);

exit:
    return rv;
}

static void amxb_rbus_filter_sub_objects(const char* name, amxc_var_t* sub) {
    when_null(name, exit);
    amxc_var_for_each(item, sub) {
        const char* item_name = amxc_var_key(item);
        if(amxc_var_type_of(item) != AMXC_VAR_ID_HTABLE) {
            continue;
        }
        if(strncmp(item_name, name, strlen(item_name)) != 0) {
            amxc_var_delete(&item);
        }
    }
exit:
    return;
}

static bool amxb_rbus_is_table(amxb_rbus_t* amxb_rbus_ctx, amxc_var_t* current) {
    rbusElementInfo_t* elems = NULL;
    amxc_string_t current_path;
    bool retval = true;
    const amxc_var_t* skip_verify = amxb_rbus_get_config_option("skip-verify-type");
    amxc_string_init(&current_path, 0);

    // Is data element type verification disabled?
    // when "skip-verify-type" is set to true do not check if the element is a table
    when_true(GET_BOOL(skip_verify, NULL), exit);

    while(amxc_var_key(current) != NULL) {
        amxc_string_prependf(&current_path, "%s.", amxc_var_key(current));
        current = amxc_var_get_parent(current);
    }
    amxc_string_trimr(&current_path, isdot);
    amxc_string_append(&current_path, ".", 1);

    rbusElementInfo_get(amxb_rbus_ctx->handle, amxc_string_get(&current_path, 0), 1, &elems);
    when_null(elems, exit);

    if(elems->type != RBUS_ELEMENT_TYPE_TABLE) {
        retval = false;
    }
    rbusElementInfo_free(amxb_rbus_ctx->handle, elems);

exit:
    amxc_string_clean(&current_path);
    return retval;
}

static int amxb_rbus_filter_objects(amxb_rbus_t* amxb_rbus_ctx,
                                    const char* object,
                                    amxd_path_t* path,
                                    amxc_var_t* params) {
    int retval = amxd_status_object_not_found;
    char* part = NULL;
    amxc_var_t* current = GET_ARG(params, object);

    if(current == NULL) {
        amxc_string_t tmp;
        amxc_string_init(&tmp, 0);
        amxc_string_set(&tmp, object);
        amxc_string_trimr(&tmp, isdot);
        current = GET_ARG(params, amxc_string_get(&tmp, 0));
        amxc_string_clean(&tmp);
    }
    if(current == NULL) {
        const char* key = amxc_var_key(params);
        char* endptr = NULL;
        int64_t index = strtoll(key, &endptr, 0);
        if((*endptr == 0) && (index != 0)) {
            retval = amxd_status_ok;
        }
        goto exit;
    }

    retval = 0;
    part = amxd_path_get_first(path, true);
    when_null(part, exit);

    switch(part[0]) {
    case '*':
        free(part);
        // the current path should be a table, if this is not the case
        // an error must be returned, wildcards are not allowed on
        // non-instances. See USP specification for details.
        when_false_status(amxb_rbus_is_table(amxb_rbus_ctx, current), exit, retval = amxd_status_invalid_path);
        part = amxd_path_get_first(path, true); // the dot
        free(part);
        part = amxd_path_get_first(path, true);
        if(part != NULL) {
            // loop over all instances
            amxc_var_for_each(sub, current) {
                if(amxb_rbus_filter_objects(amxb_rbus_ctx, part, path, sub) == amxd_status_object_not_found) {
                    retval = amxd_status_object_not_found;
                    amxc_var_delete(&sub);
                }
                // only keep correct sub-objects
                amxb_rbus_filter_sub_objects(part, sub);
            }
            if((retval != amxd_status_ok) && amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, current))) {
                retval = amxd_status_object_not_found;
            } else {
                retval = amxd_status_ok;
            }
            amxd_path_prepend(path, part);
        }
        amxd_path_prepend(path, "*.");
        break;
    case '[':
    {
        char* next = NULL;
        amxp_expr_t expr;
        uint32_t len = strlen(part);
        if(!amxb_rbus_is_table(amxb_rbus_ctx, current)) {
            free(part);
            retval = amxd_status_invalid_path;
            goto exit;
        }
        part[len - 1] = 0;
        amxp_expr_init(&expr, part + 1);
        next = amxd_path_get_first(path, true); // the dot
        free(next);
        next = amxd_path_get_first(path, true);
        // Filter all instances
        amxc_var_for_each(sub, current) {
            if(!amxp_expr_eval_var(&expr, sub, NULL)) {
                amxc_var_delete(&sub);
            } else if(next != NULL) {
                retval = amxb_rbus_filter_objects(amxb_rbus_ctx, next, path, sub);
                // only keep correct sub-objects
                amxb_rbus_filter_sub_objects(next, sub);
            }
        }
        amxp_expr_clean(&expr);
        amxd_path_prepend(path, next);
        part[len - 1] = ']';
        amxd_path_prepend(path, ".");
        amxd_path_prepend(path, part);
        free(next);
    }
    break;
    default: {
        int len = strlen(part);
        if(part[len - 1] == '.') {
            len--;
        }
        amxc_var_for_each(sub, current) {
            const char* key = amxc_var_key(sub);
            if(amxc_var_type_of(sub) == AMXC_VAR_ID_HTABLE) {
                if(strncmp(key, part, len) != 0) {
                    amxc_var_delete(&sub);
                } else {
                    amxb_rbus_filter_objects(amxb_rbus_ctx, key, path, sub);
                }
            }
        }
        if(amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, current))) {
            retval = amxd_status_object_not_found;
        }
        amxd_path_prepend(path, part);
    }
    break;
    }

    free(part);

exit:
    return retval;
}

static void amxb_rbus_build_response(amxc_string_t* full_path,
                                     amxc_var_t* params,
                                     uint32_t current_depth,
                                     uint32_t needed_depth,
                                     uint32_t max_depth,
                                     amxc_var_t* resolved) {
    const char* path = NULL;
    uint32_t pos = amxc_string_text_length(full_path);
    uint32_t len = 0;
    amxc_string_t name;

    amxc_string_init(&name, 32);
    amxc_var_for_each(sub, params) {
        if(amxc_var_type_of(sub) != AMXC_VAR_ID_HTABLE) {
            // skip non-tables (parameters)
            continue;
        }
        path = amxc_var_key(sub);
        len = strlen(path);
        amxc_string_append(full_path, path, len);
        if(path[len - 1] != '.') {
            amxc_string_append(full_path, ".", 1);
        }
        amxc_string_set(&name, path);
        len += 1;
        if((current_depth <= max_depth) ||
           ((current_depth == max_depth + 1) && (current_depth - 1 == needed_depth) && amxc_string_is_numeric(&name))) { // max_depth + 1 if current is an instance.
            amxb_rbus_build_response(full_path, sub, current_depth + 1, needed_depth, max_depth, resolved);
            if(current_depth >= needed_depth) {
                amxc_var_take_it(sub);
                if(amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, sub))) {
                    amxc_var_delete(&sub);
                } else {
                    amxc_var_set_key(resolved, amxc_string_get(full_path, 0), sub, AMXC_VAR_FLAG_DEFAULT);
                }
            }
        } else {
            amxc_var_delete(&sub);
        }
        amxc_string_remove_at(full_path, pos, len);
    }
    amxc_string_clean(&name);
}

static int amxb_rbus_filter_parameters(const char* param_name, amxc_var_t* resolved_objects) {
    int rv = amxd_status_ok;
    when_true(amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, resolved_objects)), exit);
    rv = amxd_status_parameter_not_found;

    amxc_var_for_each(item, resolved_objects) {
        amxc_var_t* param = GET_ARG(item, param_name);
        if(param == NULL) {
            amxc_var_delete(&item);
        } else {
            amxc_var_take_it(param);
            amxc_var_clean(item);
            amxc_var_set_type(item, AMXC_VAR_ID_HTABLE);
            amxc_var_set_key(item, param_name, param, AMXC_VAR_FLAG_DEFAULT);
            rv = amxd_status_ok;
        }
    }

exit:
    return rv;
}

int amxb_rbus_resolve(amxb_rbus_t* amxb_rbus_ctx,
                      const char* object,
                      const char* search_path,
                      int32_t depth,
                      amxc_var_t* resolved_objects) {
    int rv = 0;
    amxd_path_t path;
    amxc_var_t params;
    amxc_string_t full_path;
    char* p = NULL;
    uint32_t needed_depth = 0;
    uint32_t current_depth = 0;
    uint32_t max_depth = 0;

    amxc_var_init(&params);
    amxc_string_init(&full_path, 0);
    amxd_path_init(&path, object);
    amxd_path_append(&path, search_path, false);
    needed_depth = amxd_path_get_depth(&path);

    if(depth < 0) {
        depth = INT32_MAX;
    }
    // Step 1: Fetch all parameters - recursive
    p = amxd_path_get_fixed_part(&path, true);
    current_depth = needed_depth - amxd_path_get_depth(&path);
    max_depth = needed_depth + depth;
    // Will fill params variant with a hierarchical structure of objects
    // This is needed when a search path with expressions is provided
    // otherwise it is not possible to evaluate the expression on the
    // returned data model objects.
    rv = amxb_rbus_fetch_params(amxb_rbus_ctx, p, &params);
    when_failed(rv, exit);

    // Step 2: filter the hierarchical structure.
    // Remove all not matching objects
    rv = amxb_rbus_filter_objects(amxb_rbus_ctx, p, &path, &params);
    when_failed(rv, exit);

    // Step 3: Build correct output
    // Flatten the data structure
    // Make sure the given depth is respected.
    // Only keep the parameters with matching name (if a parameter was requested)
    amxc_var_set_type(resolved_objects, AMXC_VAR_ID_HTABLE);
    amxb_rbus_build_response(&full_path,
                             &params,
                             current_depth,
                             needed_depth,
                             max_depth,
                             resolved_objects);

    // Step 4: Only needed when a specific parameter was requested
    // At least one item should contain the requested parameter
    // All items not containing the requested parameter are removed
    // All items containing the requested parameter are reset and only the
    // requested parameter is kept.
    if(amxd_path_get_param(&path) != NULL) {
        const char* param_name = amxd_path_get_param(&path);
        rv = amxb_rbus_filter_parameters(param_name, resolved_objects);
    }

exit:
    free(p);
    amxc_var_clean(&params);
    amxd_path_clean(&path);
    amxc_string_clean(&full_path);
    return rv;
}
