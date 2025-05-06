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

#include <stdlib.h>

#include <amxc/amxc.h>
#include <amxp/amxp_signal.h>
#include <amxd/amxd_function.h>
#include <amxd/amxd_action.h>
#include <amxd/amxd_types.h>
#include <amxd/amxd_path.h>

#include <usp/uspl.h>

#include "amxb_usp.h"


static void amxb_usp_set_add_params(amxc_var_t* updated_params, amxc_var_t* param_errs, amxc_var_t* set_result) {
    amxc_var_for_each(res, set_result) {
        const char* param_name = amxc_var_key(res);

        if(amxc_var_type_of(res) == AMXC_VAR_ID_HTABLE) {
            amxc_var_t* param_err = amxc_var_add(amxc_htable_t, param_errs, NULL);
            uint32_t err_code_amx = GET_UINT32(res, "error_code");
            uint32_t err_code_usp = uspl_amxd_status_to_usp_error(err_code_amx);
            const char* err_msg = uspl_error_code_to_str(err_code_usp);

            amxc_var_add_key(cstring_t, param_err, "param", param_name);
            amxc_var_add_key(uint32_t, param_err, "err_code", err_code_usp);
            amxc_var_add_key(cstring_t, param_err, "err_msg", err_msg);
        } else {
            amxc_var_set_key(updated_params, param_name, res, AMXC_VAR_FLAG_COPY);
        }
    }
}

static void amxb_usp_set_populate_success(const char* requested_path,
                                          amxc_llist_t* resp_list,
                                          amxc_var_t* set_resp) {
    amxc_var_t* result = NULL;
    amxc_var_t* success = NULL;
    amxc_var_t* updated_inst_results = NULL;

    amxc_var_new(&result);
    amxc_var_set_type(result, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, result, "requested_path", requested_path);
    amxc_var_add_key(uint32_t, result, "oper_status", USP__SET_RESP__UPDATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS);
    success = amxc_var_add_key(amxc_htable_t, result, "success", NULL);
    updated_inst_results = amxc_var_add_key(amxc_llist_t, success, "updated_inst_results", NULL);

    amxc_var_for_each(set_result, set_resp) {
        amxc_var_t* updated_inst = amxc_var_add(amxc_htable_t, updated_inst_results, NULL);
        const char* affected_path = amxc_var_key(set_result);
        amxc_var_t* updated_params = amxc_var_add_key(amxc_htable_t, updated_inst, "updated_params", NULL);
        amxc_var_t* param_errs = amxc_var_add_key(amxc_llist_t, updated_inst, "param_errs", NULL);
        amxc_var_add_key(cstring_t, updated_inst, "affected_path", affected_path);
        amxb_usp_set_add_params(updated_params, param_errs, set_result);
    }

    amxc_llist_append(resp_list, &result->lit);
}

static void amxb_usp_set_populate_error(const char* requested_path,
                                        amxc_llist_t* resp_list,
                                        uint32_t status) {
    int err_code = USP_ERR_INTERNAL_ERROR;
    const char* err_msg = NULL;
    amxc_var_t* result = NULL;
    amxc_var_t* failure = NULL;

    amxc_var_new(&result);
    amxc_var_set_type(result, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, result, "requested_path", requested_path);
    amxc_var_add_key(uint32_t, result, "oper_status", USP__SET_RESP__UPDATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE);
    failure = amxc_var_add_key(amxc_htable_t, result, "failure", NULL);

    err_code = uspl_amxd_status_to_usp_error(status);
    amxc_var_add_key(uint32_t, failure, "err_code", err_code);
    err_msg = uspl_error_code_to_str(err_code);
    amxc_var_add_key(cstring_t, failure, "err_msg", err_msg);

    amxc_llist_append(resp_list, &result->lit);
}

static amxd_status_t amxb_usp_invoke_set(amxb_usp_deferred_t* fcall,
                                         const char* target_path,
                                         amxc_var_t* parameters,
                                         bool allow_partial,
                                         const char* requested_path) {
    amxd_status_t status = amxd_status_ok;
    amxc_var_t* params = NULL;
    amxc_var_t* oparams = NULL;
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&args);
    amxc_var_init(&ret);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    params = amxc_var_add_key(amxc_htable_t, &args, "parameters", NULL);
    oparams = amxc_var_add_key(amxc_htable_t, &args, "oparameters", NULL);
    amxc_var_add_key(bool, &args, "allow_partial", allow_partial);
    amxc_var_add_key(uint32_t, &args, "access", AMXB_PUBLIC);

    amxc_var_for_each(entry, parameters) {
        bool required = GET_BOOL(entry, "required");
        if(required) {
            amxc_var_set_key(params,
                             GET_CHAR(entry, "param"),
                             GET_ARG(entry, "value"),
                             AMXC_VAR_FLAG_DEFAULT);
        } else {
            amxc_var_set_key(oparams,
                             GET_CHAR(entry, "param"),
                             GET_ARG(entry, "value"),
                             AMXC_VAR_FLAG_DEFAULT);
        }
    }

    status = amxb_usp_amx_call(fcall, target_path, requested_path, false, "_set", &args, &ret, amxb_usp_amx_call_done);
    if(status == amxd_status_deferred) {
        goto exit;
    }

    // Translate response
    amxb_usp_call_translate(&ret, AMXB_USP_TRANSLATE_PATHS);

    // Initialize rv if invoke failed
    if(status == amxd_status_ok) {
        amxb_usp_set_populate_success(requested_path, &fcall->resp_list, &ret);
    } else {
        amxb_usp_set_populate_error(requested_path, &fcall->resp_list, status);
    }

exit:
    amxc_var_clean(&ret);
    amxc_var_clean(&args);

    return status;
}

static amxd_status_t amxb_usp_set_translated(amxb_usp_deferred_t* fcall,
                                             const char* requested_path,
                                             amxc_var_t* parameters,
                                             bool allow_partial) {
    amxd_status_t status = amxd_status_ok;
    bool deferred = false;
    amxd_path_t path;
    const char* requested = NULL;
    const char* translated = NULL;

    amxd_path_init(&path, requested_path);

    // check if translation is need
    // if translation configuration is available, this will translate
    // the requested path into the internal path
    amxb_usp_translate_path(&path, &requested, &translated);

    status = amxb_usp_invoke_set(fcall, amxd_path_get(&path, AMXD_OBJECT_TERMINATE),
                                 parameters, allow_partial, requested_path);
    if(status == amxd_status_deferred) {
        deferred = true;
    }

    amxd_path_clean(&path);
    return deferred;
}

static void amxb_usp_set_populate_request(amxc_var_t* set_var,
                                          const char* object,
                                          const char* search_path,
                                          uint32_t flags,
                                          amxc_var_t* values,
                                          amxc_var_t* ovalues) {
    amxc_var_t* requests = NULL;
    amxc_var_t* request = NULL;
    amxc_var_t* params_list = NULL;
    amxc_string_t path;

    amxc_string_init(&path, 0);
    if(search_path != NULL) {
        amxc_string_setf(&path, "%s%s", object, search_path);
    } else {
        amxc_string_setf(&path, "%s", object);
    }


    if((flags & AMXB_FLAG_PARTIAL) == AMXB_FLAG_PARTIAL) {
        amxc_var_add_key(bool, set_var, "allow_partial", true);
    }
    requests = amxc_var_add_key(amxc_llist_t, set_var, "requests", NULL);
    request = amxc_var_add(amxc_htable_t, requests, NULL);
    amxc_var_add_key(cstring_t, request, "object_path", amxc_string_get(&path, 0));
    params_list = amxc_var_add_key(amxc_llist_t, request, "parameters", NULL);

    amxc_var_for_each(param, values) {
        const char* key = amxc_var_key(param);
        amxc_var_t* params_entry = amxc_var_add(amxc_htable_t, params_list, NULL);
        amxc_var_add_key(cstring_t, params_entry, "param", key);
        amxc_var_set_key(params_entry, "value", param, AMXC_VAR_FLAG_COPY);
        amxc_var_add_key(bool, params_entry, "required", true);
    }

    amxc_var_for_each(param, ovalues) {
        const char* key = amxc_var_key(param);
        amxc_var_t* params_entry = amxc_var_add(amxc_htable_t, params_list, NULL);
        amxc_var_add_key(cstring_t, params_entry, "param", key);
        amxc_var_set_key(params_entry, "value", param, AMXC_VAR_FLAG_COPY);
        amxc_var_add_key(bool, params_entry, "required", false);
    }

    amxc_string_clean(&path);
}

static amxd_status_t amxb_usp_set_resp_to_amx(amxc_var_t* usp_resp, amxc_var_t* ret) {
    amxd_status_t status = amxd_status_ok;
    uint32_t oper_status = GET_UINT32(usp_resp, "oper_status");

    if(oper_status == USP__SET_RESP__UPDATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS) {
        amxc_var_t* results = GETP_ARG(usp_resp, "success.updated_inst_results");
        amxc_var_t* set_resp = NULL;

        amxc_var_set_type(ret, AMXC_VAR_ID_LIST);
        set_resp = amxc_var_add(amxc_htable_t, ret, NULL);

        amxc_var_for_each(res, results) {
            const char* affected_path = GET_CHAR(res, "affected_path");
            amxc_var_t* updated_params = GET_ARG(res, "updated_params");
            amxc_var_t* updated_obj = amxc_var_add_key(amxc_htable_t, set_resp, affected_path, NULL);
            amxc_var_move(updated_obj, updated_params);
        }
    } else if(oper_status == USP__SET_RESP__UPDATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE) {
        uint32_t err_code = uspl_usp_error_to_amxd_status(GETP_UINT32(usp_resp, "failure.err_code"));
        status = err_code == 0 ? amxd_status_unknown_error : err_code;
    } else {
        // If none of the above, it could be an error message, so we can try to forward some information
        // from there to the caller
        status = amxb_usp_convert_error(ret, usp_resp);
    }

    return status;
}

void amxb_usp_set_deferred_resp(const amxc_var_t* const data,
                                amxb_usp_deferred_t* fcall,
                                amxb_usp_deferred_child_t* child) {
    amxc_var_t* retval = GET_ARG(data, "retval");
    uint32_t status = GET_UINT32(data, "status");

    if(status == amxd_status_ok) {
        amxb_usp_set_populate_success(child->requested_path, &fcall->resp_list, retval);
    } else {
        amxb_usp_set_populate_error(child->requested_path, &fcall->resp_list, status);
    }

    if(fcall->num_calls == 0) {
        amxb_usp_reply_deferred(fcall, uspl_set_resp_new);
        amxb_usp_deferred_delete(&fcall);
    }
}

int amxb_usp_handle_set(amxb_usp_t* ctx, uspl_rx_t* usp_rx) {
    int retval = -1;
    amxc_var_t set_var;
    amxc_var_t* requests = NULL;
    amxc_var_t* parameters = NULL;
    bool allow_partial = false;
    const char* path = NULL;
    bool deferred = false;
    amxb_usp_deferred_t* fcall = NULL;

    amxc_var_init(&set_var);

    when_null(ctx, exit);
    when_null(usp_rx, exit);

    ctx->stats.counter_rx_set++;

    retval = uspl_set_extract(usp_rx, &set_var);
    if(retval != 0) {
        retval = amxb_usp_reply_error(ctx, usp_rx, retval);
        goto exit;
    }

    allow_partial = GET_BOOL(&set_var, "allow_partial");
    requests = GET_ARG(&set_var, "requests");
    retval = amxb_usp_deferred_new(&fcall, ctx, usp_rx,
                                   amxc_llist_size(amxc_var_constcast(amxc_llist_t, requests)));
    when_failed(retval, exit);

    amxc_var_for_each(request, requests) {
        path = GET_CHAR(request, "object_path");
        parameters = GET_ARG(request, "parameters");
        deferred |= amxb_usp_set_translated(fcall, path, parameters, allow_partial);
    }

    if(deferred) {
        retval = 0;
        goto exit;
    } else {
        retval = amxb_usp_reply(ctx, usp_rx, &fcall->resp_list, uspl_set_resp_new);
        amxb_usp_deferred_delete(&fcall);
    }

exit:
    amxc_var_clean(&set_var);
    return retval;
}

int amxb_usp_set(void* const ctx,
                 const char* object,
                 const char* search_path,
                 uint32_t flags,
                 amxc_var_t* values,
                 amxc_var_t* ovalues,
                 UNUSED uint32_t access,
                 amxc_var_t* ret,
                 int timeout) {
    int retval = amxd_status_object_not_found;
    amxb_usp_t* amxb_usp = (amxb_usp_t*) ctx;
    amxc_var_t request;
    amxc_var_t usp_resp;
    char* msg_id = NULL;
    bool requires_device = GET_BOOL(amxb_usp_get_config(), AMXB_USP_COPT_REQUIRES_DEVICE);

    amxc_var_init(&usp_resp);
    amxc_var_init(&request);
    amxc_var_set_type(&request, AMXC_VAR_ID_HTABLE);

    if(requires_device) {
        when_false(amxb_usp_path_starts_with_device(object), exit);
    }

    amxb_usp_set_populate_request(&request, object, search_path, flags, values, ovalues);

    retval = amxb_usp_send_req(amxb_usp, &request, uspl_set_new, &msg_id);
    when_failed(retval, exit);

    retval = amxb_usp_poll_response(amxb_usp, msg_id, uspl_set_resp_extract, &usp_resp, false, timeout);
    when_failed(retval, exit);

    retval = amxb_usp_set_resp_to_amx(&usp_resp, ret);

exit:
    free(msg_id);
    amxc_var_clean(&usp_resp);
    amxc_var_clean(&request);
    return retval;
}
