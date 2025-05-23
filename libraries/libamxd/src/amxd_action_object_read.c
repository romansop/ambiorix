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
#include <string.h>
#include <stdio.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>

#include <amxd/amxd_types.h>
#include <amxd/amxd_common.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_action.h>
#include <amxd/amxd_object.h>

#include "amxd_priv.h"
#include "amxd_assert.h"

static void filter_add_params(amxc_string_t* filter, const amxc_llist_t* names) {
    const char* sep = "";

    when_true(amxc_llist_is_empty(names), exit);

    if(amxc_string_is_empty(filter)) {
        amxc_string_appendf(filter, "name in [ ");
    } else {
        amxc_string_appendf(filter, " && name in [");
    }

    amxc_llist_iterate(it, names) {
        amxc_var_t* var_name = amxc_var_from_llist_it(it);
        const char* str_name = amxc_var_constcast(cstring_t, var_name);
        amxc_string_appendf(filter, "%s'%s' ", sep, str_name);
        sep = ",";
    }

    amxc_string_appendf(filter, "]");

exit:
    return;
}

static bool amxd_action_parameter_is_in(const char* name, amxc_var_t* params) {
    bool retval = false;

    when_str_empty(name, exit)

    amxc_var_for_each(param, params) {
        const char* param_name = GET_CHAR(param, NULL);
        if((param_name != NULL) && (strcmp(name, param_name) == 0)) {
            retval = true;
            break;
        }
    }

exit:
    return retval;
}

static amxd_status_t amxd_action_object_read_check_params(amxd_object_t* object,
                                                          const amxc_llist_t* names) {
    amxd_status_t status = amxd_status_ok;
    amxc_var_t obj_params;

    amxc_var_init(&obj_params);
    when_true(amxc_llist_is_empty(names), exit);

    amxd_object_list_params(object, &obj_params, amxd_dm_access_private);
    amxc_llist_iterate(it, names) {
        amxc_var_t* var_name = amxc_var_from_llist_it(it);
        const char* str_name = amxc_var_constcast(cstring_t, var_name);
        if(!amxd_action_parameter_is_in(str_name, &obj_params)) {
            status = amxd_status_parameter_not_found;
            break;
        }
    }

exit:
    amxc_var_clean(&obj_params);
    return status;
}

static void filter_add_attributes(amxc_string_t* filter, uint32_t attrs) {
    static const char* attr_name[] = {
        "template",
        "instance",
        "private",
        "read-only",
        "persistent",
        "volatile",
        "counter",
        "key",
        "unique",
        "protected"
    };

    const char* sep = "";
    if(!amxc_string_is_empty(filter)) {
        sep = " && ";
    }

    for(uint32_t i = amxd_pattr_template; i < amxd_pattr_max; i++) {
        if(IS_BIT_SET(attrs, i)) {
            amxc_string_appendf(filter, "%sattributes.%s == true", sep, attr_name[i]);
            sep = " && ";
        }
    }
}

static amxd_status_t amxd_action_get_values(amxd_object_t* const object,
                                            amxd_dm_access_t access,
                                            uint32_t attributes,
                                            amxc_var_t* params,
                                            const char* expression,
                                            amxc_var_t* const retval) {
    amxd_status_t status = amxd_status_unknown_error;
    amxp_expr_t expr;
    amxc_var_t description;
    amxc_var_t args;
    amxp_expr_status_t expr_status = amxp_expr_status_ok;

    amxc_var_init(&description);
    amxc_var_set_type(retval, AMXC_VAR_ID_HTABLE);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);

    if(expression != NULL) {
        when_failed(amxp_expr_init(&expr, expression), exit);
    }

    amxc_var_add_key(uint32_t, &args, "access", access);
    amxc_var_add_key(bool, &args, "no-param-value", true);

    amxc_llist_for_each(it, (&object->parameters)) {
        amxd_param_t* param = amxc_llist_it_get_data(it, amxd_param_t, it);
        const char* name = amxd_param_get_name(param);
        uint32_t param_attrs = amxd_param_get_attrs(param);
        amxc_var_t* value = NULL;
        bool in_filter_list = amxd_action_parameter_is_in(name, params);
        if(!amxd_action_can_add_param(param_attrs,
                                      amxd_object_get_type(object),
                                      access,
                                      true)) {
            if(in_filter_list) {
                status = amxd_status_parameter_not_found;
                goto exit;
            }
            continue;
        }
        if((attributes & param_attrs) != attributes) {
            continue;
        }
        if(params && !in_filter_list) {
            continue;
        }
        if(expression != NULL) {
            amxd_dm_invoke_action(object, param, action_param_describe, &args, &description);
            if(amxp_expr_eval_var(&expr, &description, &expr_status)) {
                value = amxc_var_add_new_key(retval, name);
                amxd_dm_invoke_action(object, param, action_param_read, &args, value);
            }
            if(expr_status != amxp_expr_status_ok) {
                status = amxd_status_parameter_not_found;
                goto exit;
            }
        } else {
            value = amxc_var_add_new_key(retval, name);
            amxd_dm_invoke_action(object, param, action_param_read, &args, value);
        }
    }

    status = amxd_status_ok;

exit:
    if(expression != NULL) {
        amxp_expr_clean(&expr);
    }
    amxc_var_clean(&description);
    amxc_var_clean(&args);
    return status;
}

amxd_status_t amxd_action_object_read_filter(amxc_string_t* filter,
                                             const amxc_var_t* args) {
    amxd_status_t status = amxd_status_ok;
    amxc_var_t* temp = NULL;
    const char* f = GET_CHAR(args, "filter");

    if((f != NULL) && (*f != 0)) {
        amxc_string_setf(filter, "%s", f);
        goto exit;
    }

    temp = GET_ARG(args, "parameters");
    when_true_status(!amxc_var_is_null(temp) && amxc_var_type_of(temp) != AMXC_VAR_ID_LIST,
                     exit,
                     status = amxd_status_invalid_function_argument);
    if(temp != NULL) {
        filter_add_params(filter, amxc_var_constcast(amxc_llist_t, temp));
    }

    temp = GET_ARG(args, "attributes");
    if(temp != NULL) {
        filter_add_attributes(filter, amxc_var_dyncast(uint32_t, temp));
    }

exit:
    return status;
}

amxd_status_t amxd_action_object_read(amxd_object_t* const object,
                                      UNUSED amxd_param_t* const p,
                                      amxd_action_t reason,
                                      const amxc_var_t* const args,
                                      amxc_var_t* const retval,
                                      UNUSED void* priv) {
    amxd_status_t status = amxd_status_unknown_error;
    amxd_dm_access_t access = amxd_dm_access_public;
    const char* f = GET_CHAR(args, "filter");
    amxc_var_t* params = NULL;
    uint32_t attributes = 0;
    amxc_var_t* temp = NULL;

    when_null(object, exit);
    when_null(retval, exit);
    when_true_status(reason != action_object_read,
                     exit,
                     status = amxd_status_function_not_implemented);

    when_true_status(!amxc_var_is_null(args) &&
                     amxc_var_type_of(args) != AMXC_VAR_ID_HTABLE,
                     exit,
                     status = amxd_status_invalid_function_argument);

    access = (amxd_dm_access_t) GET_UINT32(args, "access");
    when_true_status(!amxd_action_verify_access(object, access),
                     exit,
                     status = amxd_status_object_not_found);

    params = GET_ARG(args, "parameters");
    when_true_status(!amxc_var_is_null(params) && amxc_var_type_of(params) != AMXC_VAR_ID_LIST,
                     exit,
                     status = amxd_status_invalid_function_argument);
    if(amxc_llist_is_empty(amxc_var_constcast(amxc_llist_t, params))) {
        params = NULL;
    } else {
        status = amxd_action_object_read_check_params(object,
                                                      amxc_var_constcast(amxc_llist_t, params));
        when_failed(status, exit);
    }

    temp = GET_ARG(args, "attributes");
    attributes = temp == NULL? 0 : GET_UINT32(temp, NULL);

    status = amxd_action_get_values(object, access, attributes, params, f, retval);

exit:
    if(status != amxd_status_ok) {
        amxc_var_clean(retval);
    }
    return status;
}

amxd_status_t amxd_object_get_params(amxd_object_t* const object,
                                     amxc_var_t* const params,
                                     amxd_dm_access_t access) {
    amxd_status_t retval = amxd_status_unknown_error;
    amxc_var_t args;

    amxc_var_init(&args);
    when_null(object, exit);
    when_null(params, exit);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "access", access);
    retval = amxd_dm_invoke_action(object,
                                   NULL,
                                   action_object_read,
                                   &args,
                                   params);

exit:
    amxc_var_clean(&args);
    return retval;
}

amxd_status_t amxd_object_get_params_with_attr(amxd_object_t* const object,
                                               amxc_var_t* const params,
                                               uint32_t attrs,
                                               amxd_dm_access_t access) {
    amxd_status_t retval = amxd_status_unknown_error;
    amxc_var_t args;

    amxc_var_init(&args);
    when_null(object, exit);
    when_null(params, exit);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "access", access);
    amxc_var_add_key(uint32_t, &args, "attributes", attrs);
    retval = amxd_dm_invoke_action(object,
                                   NULL,
                                   action_object_read,
                                   &args,
                                   params);

exit:
    amxc_var_clean(&args);
    return retval;
}

amxd_status_t amxd_object_get_params_filtered(amxd_object_t* const object,
                                              amxc_var_t* const params,
                                              const char* filter,
                                              amxd_dm_access_t access) {
    amxd_status_t retval = amxd_status_unknown_error;
    amxc_var_t args;

    amxc_var_init(&args);
    when_null(object, exit);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "access", access);
    if(filter != NULL) {
        amxc_var_add_key(cstring_t, &args, "filter", filter);
    }
    retval = amxd_dm_invoke_action(object,
                                   NULL,
                                   action_object_read,
                                   &args,
                                   params);

exit:
    amxc_var_clean(&args);
    return retval;
}

amxd_status_t amxd_object_get_key_params(amxd_object_t* const object,
                                         amxc_var_t* const params,
                                         amxd_dm_access_t access) {
    return amxd_object_get_params_with_attr(object,
                                            params,
                                            SET_BIT(amxd_pattr_key),
                                            access);
}
