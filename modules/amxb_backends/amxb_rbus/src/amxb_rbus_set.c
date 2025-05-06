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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>

#include "amxb_rbus.h"

static int amxb_rbus_call_set(amxb_rbus_t* amxb_rbus_ctx,
                              const char* object,
                              const char* search_path,
                              amxc_var_t* values,
                              amxc_var_t* ovalues,
                              uint32_t access,
                              amxc_var_t* ret) {
    int retval = -1;
    amxc_var_t args;
    amxb_request_t* request = NULL;
    amxc_var_t* params = NULL;
    amxc_var_t* result = NULL;

    amxc_var_init(&args);
    when_failed(amxb_request_new(&request), exit);

    request->result = ret;

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "rel_path", search_path);
    params = amxc_var_add_key(amxc_htable_t, &args, "parameters", NULL);
    amxc_var_move(params, values);
    if(!amxc_var_is_null(ovalues)) {
        params = amxc_var_add_key(amxc_htable_t, &args, "oparameters", NULL);
        amxc_var_move(params, ovalues);
    }
    amxc_var_add_key(uint32_t, &args, "access", access);

    retval = amxb_rbus_invoke_root(amxb_rbus_ctx, object, "_set", &args, request);
    result = GETI_ARG(ret, 0);
    amxc_var_take_it(result);
    amxc_var_move(ret, result);
    amxc_var_delete(&result);

exit:
    request->result = NULL;
    amxb_close_request(&request);
    amxc_var_clean(&args);
    return retval;
}

static int amxb_rbus_send_set_request(amxb_rbus_t* amxb_rbus_ctx,
                                      const char* object,
                                      amxc_var_t* values) {
    int retval = -1;
    uint32_t nr_params = amxc_htable_size(amxc_var_constcast(amxc_htable_t, values));
    rbusProperty_t next = NULL;
    rbusProperty_t properties = NULL;
    rbusValue_t rbus_value = NULL;
    amxc_string_t full_path;
    rbusSetOptions_t opts = {true, 0};

    amxc_string_init(&full_path, 0);
    when_true(nr_params == 0, exit);

    amxc_var_for_each(data, values) {
        amxc_string_setf(&full_path, "%s%s", object, amxc_var_key(data));
        rbusValue_Init(&rbus_value);
        amxb_rbus_var_to_rvalue(rbus_value, data);
        rbusProperty_Init(&next, amxc_string_get(&full_path, 0), rbus_value);
        if(properties == NULL) {
            properties = next;
        } else {
            rbusProperty_Append(properties, next);
            rbusProperty_Release(next);
        }
        rbusValue_Release(rbus_value);
    }

    // Each parameter is send individually to the data model provider
    // When the last is send it will set the commit to true.
    // The data model provider should apply the values when the commit is set
    // to true.
    retval = rbus_setMulti(amxb_rbus_ctx->handle, nr_params, properties, &opts);
    retval = amxb_rbus_translate_rbus2status(retval);
    rbusProperty_Release(properties);


exit:
    amxc_string_clean(&full_path);
    return retval;
}

static int amxb_rbus_check_resolved(amxb_rbus_t* amxb_rbus_ctx,
                                    uint32_t flags,
                                    amxc_var_t* resolved_table,
                                    amxc_var_t* values,
                                    amxc_var_t* ovalues) {
    int retval = 0;

    amxc_var_for_each(data, resolved_table) {
        const char* path = amxc_var_key(data);

        amxc_var_set_type(data, AMXC_VAR_ID_HTABLE);
        if((flags & AMXB_FLAG_PARTIAL) == 0) {
            if(!amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, values))) {
                retval = amxb_rbus_send_set_request(amxb_rbus_ctx, path, values);
                if(retval == 0) {
                    amxc_var_copy(data, values);
                }
            }
        } else {
            if(!amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, values))) {
                retval = amxb_rbus_set_values(amxb_rbus_ctx, true, path, values, data);
            }
        }
        if(!amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, ovalues))) {
            amxb_rbus_set_values(amxb_rbus_ctx, false, path, ovalues, data);
        }
        if(amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, data))) {
            amxc_var_delete(&data);
        }
    }

    if(amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, resolved_table))) {
        amxc_var_set_type(resolved_table, AMXC_VAR_ID_NULL);
    }

    return retval;
}

static void amxb_rbus_normalize(amxc_var_t* resolved_table) {
    amxc_var_t tmp;
    amxc_string_t path;

    amxc_string_init(&path, 0);
    amxc_var_init(&tmp);
    amxc_var_set_type(&tmp, AMXC_VAR_ID_HTABLE);

    amxc_var_for_each(data, resolved_table) {
        const char* object = amxc_var_key(data);
        amxc_var_for_each(value, data) {
            const char* name = amxc_var_key(value);
            const char* last_dot = strrchr(name, '.');
            char* key = NULL;
            amxc_var_t* other = NULL;
            uint32_t offset = 0;
            if(last_dot == NULL) {
                continue;
            }
            offset = last_dot - name;
            amxc_string_set(&path, object);
            amxc_string_append(&path, name, offset + 1);
            other = amxc_var_get_key(&tmp, amxc_string_get(&path, 0), AMXC_VAR_FLAG_DEFAULT);
            if(other == NULL) {
                other = amxc_var_add_key(amxc_htable_t, &tmp, amxc_string_get(&path, 0), NULL);
            }
            key = strdup(last_dot + 1);
            amxc_var_set_key(other, key, value, AMXC_VAR_FLAG_DEFAULT);
            free(key);
        }
    }

    amxc_var_for_each(data, &tmp) {
        amxc_var_set_key(resolved_table, amxc_var_key(data), data, AMXC_VAR_FLAG_DEFAULT);
    }

    amxc_var_clean(&tmp);
    amxc_string_clean(&path);
}

// This function is also used by amxb_rbus_add
int amxb_rbus_set_values(amxb_rbus_t* amxb_rbus_ctx,
                         bool required,
                         const char* path,
                         amxc_var_t* values,
                         amxc_var_t* data) {
    int retval = 0;
    amxc_var_t value;
    amxc_var_init(&value);

    amxc_var_for_each(v, values) {
        const char* name = amxc_var_key(v);
        amxc_var_set_type(&value, AMXC_VAR_ID_HTABLE);
        amxc_var_set_key(&value, name, v, AMXC_VAR_FLAG_COPY);
        retval = amxb_rbus_send_set_request(amxb_rbus_ctx, path, &value);
        if(retval != 0) {
            amxc_var_t* param = amxc_var_add_key(amxc_htable_t, data, name, NULL);
            amxc_var_add_key(uint32_t, param, "error_code", retval);
            amxc_var_add_key(bool, param, "required", required);
        } else {
            amxc_var_set_key(data, name, GET_ARG(&value, name), AMXC_VAR_FLAG_DEFAULT);
        }
    }
    amxc_var_clean(&value);

    return retval;
}

int amxb_rbus_set(void* const ctx,
                  const char* object,
                  const char* search_path,
                  uint32_t flags,
                  amxc_var_t* values,
                  amxc_var_t* ovalues,
                  uint32_t access,
                  amxc_var_t* ret,
                  int timeout) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    const amxc_var_t* check_amx = amxb_rbus_get_config_option("use-amx-calls");
    amxc_var_t* resolved_table = NULL;
    int retval = -1;

    amxc_var_set_type(ret, AMXC_VAR_ID_LIST);
    resolved_table = amxc_var_add(amxc_htable_t, ret, NULL);

    // apply the timeout
    amxb_rbus_set_timeout(timeout);

    if(GET_BOOL(check_amx, NULL) && amxb_rbus_remote_is_amx(amxb_rbus_ctx, object)) {
        retval = amxb_rbus_call_set(amxb_rbus_ctx, object, search_path, values, ovalues, access, resolved_table);
    } else {
        retval = amxb_rbus_resolve(amxb_rbus_ctx, object, search_path, 0, resolved_table);
        when_failed(retval, exit);
        // Now resolved_table contains all object on which a set should be applied.
        retval = amxb_rbus_check_resolved(amxb_rbus_ctx, flags, resolved_table, values, ovalues);
        when_failed(retval, exit);
        amxb_rbus_normalize(resolved_table);
    }

exit:
    return retval;
}

