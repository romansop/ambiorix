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

static void amxb_usp_gi_add_inst(amxc_var_t* curr_insts, amxc_var_t* inst) {
    const char* inst_path = amxc_var_key(inst);
    amxc_var_t* entry = amxc_var_add(amxc_htable_t, curr_insts, NULL);
    amxc_var_t* keys = NULL;

    amxc_var_add_key(cstring_t, entry, "inst_path", inst_path);
    keys = amxc_var_add_key(amxc_htable_t, entry, "unique_keys", NULL);

    amxc_var_for_each(el, inst) {
        amxc_var_set_key(keys, amxc_var_key(el), el, AMXC_VAR_FLAG_COPY);
    }
}

static void amxb_usp_gi_resp_list_append(amxc_llist_t* resp_list,
                                         amxc_var_t* ret,
                                         const char* requested_path,
                                         int status) {
    int err_code = USP_ERR_INTERNAL_ERROR;
    amxc_var_t* response = NULL;
    amxc_var_t* curr_insts = NULL;

    amxc_var_new(&response);
    amxc_var_set_type(response, AMXC_VAR_ID_HTABLE);

    amxc_var_add_key(cstring_t, response, "requested_path", requested_path);
    err_code = uspl_amxd_status_to_usp_error(status);
    amxc_var_add_key(uint32_t, response, "err_code", err_code);
    if(err_code != USP_ERR_OK) {
        const char* err_msg = uspl_error_code_to_str(err_code);
        amxc_var_add_key(cstring_t, response, "err_msg", err_msg);
    }

    when_true(ret == NULL || amxc_var_type_of(ret) == AMXC_VAR_ID_NULL, exit);

    curr_insts = amxc_var_add_key(amxc_llist_t, response, "curr_insts", NULL);
    amxc_var_for_each(inst, ret) {
        amxb_usp_gi_add_inst(curr_insts, inst);
    }

exit:
    amxc_llist_append(resp_list, &response->lit);
}

static amxd_status_t amxb_usp_invoke_get_instances(amxb_usp_deferred_t* fcall,
                                                   const char* target_path,
                                                   bool first_level,
                                                   const char* requested_path) {
    amxd_status_t status = amxd_status_ok;
    amxc_var_t args;
    amxc_var_t ret;
    int depth = first_level ? 0 : INT32_MAX;

    amxc_var_init(&args);
    amxc_var_init(&ret);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(int32_t, &args, "depth", depth);
    amxc_var_add_key(uint32_t, &args, "access", AMXB_PUBLIC);

    status = amxb_usp_amx_call(fcall, target_path, requested_path, false, "_get_instances", &args, &ret, amxb_usp_amx_call_done);
    if(status == amxd_status_deferred) {
        goto exit;
    }

    // Translate response
    amxb_usp_call_translate(&ret, AMXB_USP_TRANSLATE_PATHS);

    amxb_usp_gi_resp_list_append(&fcall->resp_list, &ret, requested_path, status);

exit:
    amxc_var_clean(&args);
    amxc_var_clean(&ret);
    return status;
}

static amxd_status_t amxb_usp_get_instances_translated(amxb_usp_deferred_t* fcall,
                                                       const char* requested_path,
                                                       bool first_level) {
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

    status = amxb_usp_invoke_get_instances(fcall, amxd_path_get(&path, AMXD_OBJECT_TERMINATE),
                                           first_level, requested_path);
    if(status == amxd_status_deferred) {
        deferred = true;
    }

    amxd_path_clean(&path);
    return deferred;
}

static void amxb_usp_gi_populate_request(amxc_var_t* request, amxc_string_t* path, int32_t depth) {
    bool first_level = (depth > 0) ? false : true;
    amxc_var_t* obj_paths = NULL;

    amxc_var_set_type(request, AMXC_VAR_ID_HTABLE);
    obj_paths = amxc_var_add_key(amxc_llist_t, request, "obj_paths", NULL);
    amxc_var_add(cstring_t, obj_paths, amxc_string_get(path, 0));
    amxc_var_add_key(bool, request, "first_level_only", first_level);
}

static amxd_status_t amxb_usp_get_instances_resp_to_amx(amxc_var_t* usp_ret, amxc_var_t* amx_ret) {
    amxd_status_t status = amxd_status_ok;
    amxc_var_for_each(entry, usp_ret) {
        amxc_var_t* err_code = GET_ARG(entry, "err_code");
        amxc_var_t* err_msg = GET_ARG(entry, "err_msg");
        amxc_var_t* requested_path = GET_ARG(entry, "requested_path");
        amxc_var_t* curr_insts = GET_ARG(entry, "curr_insts");
        amxc_var_t* gi_resp = NULL;

        status = uspl_usp_error_to_amxd_status(amxc_var_dyncast(uint32_t, err_code));
        amxc_var_delete(&err_code);
        amxc_var_delete(&err_msg);
        amxc_var_delete(&requested_path);

        amxc_var_set_type(amx_ret, AMXC_VAR_ID_LIST);
        gi_resp = amxc_var_add(amxc_htable_t, amx_ret, NULL);

        amxc_var_for_each(inst, curr_insts) {
            const char* inst_path = GET_CHAR(inst, "inst_path");
            amxc_var_t* unique_keys = GET_ARG(inst, "unique_keys");
            amxc_var_t* inst_obj = amxc_var_add_key(amxc_htable_t, gi_resp, inst_path, NULL);
            amxc_var_move(inst_obj, unique_keys);
        }
    }
    return status;
}

void amxb_usp_get_instances_deferred_resp(const amxc_var_t* const data,
                                          amxb_usp_deferred_t* fcall,
                                          amxb_usp_deferred_child_t* child) {
    amxc_var_t* retval = GET_ARG(data, "retval");

    amxb_usp_gi_resp_list_append(&fcall->resp_list, retval, child->requested_path, GET_UINT32(data, "status"));

    if(fcall->num_calls == 0) {
        amxb_usp_reply_deferred(fcall, uspl_get_instances_resp_new);
        amxb_usp_deferred_delete(&fcall);
    }
}

int amxb_usp_handle_get_instances(amxb_usp_t* ctx, uspl_rx_t* usp_rx) {
    int retval = -1;
    amxc_var_t request;
    amxc_var_t* obj_paths = NULL;
    bool first_level = false;
    bool deferred = false;
    amxb_usp_deferred_t* fcall = NULL;

    amxc_var_init(&request);

    when_null(ctx, exit);
    when_null(usp_rx, exit);

    ctx->stats.counter_rx_get_instances++;

    retval = uspl_get_instances_extract(usp_rx, &request);
    if(retval != 0) {
        retval = amxb_usp_reply_error(ctx, usp_rx, retval);
        goto exit;
    }

    first_level = GET_BOOL(&request, "first_level_only");
    obj_paths = GET_ARG(&request, "obj_paths");
    retval = amxb_usp_deferred_new(&fcall, ctx, usp_rx,
                                   amxc_llist_size(amxc_var_constcast(amxc_llist_t, obj_paths)));
    when_failed(retval, exit);

    amxc_var_for_each(var, obj_paths) {
        deferred |= amxb_usp_get_instances_translated(fcall, amxc_var_constcast(cstring_t, var), first_level);
    }

    if(deferred) {
        retval = 0;
        goto exit;
    } else {
        retval = amxb_usp_reply(ctx, usp_rx, &fcall->resp_list, uspl_get_instances_resp_new);
        amxb_usp_deferred_delete(&fcall);
    }

exit:
    amxc_var_clean(&request);
    return retval;
}

int amxb_usp_get_instances(void* const ctx,
                           const char* object,
                           const char* search_path,
                           int32_t depth,
                           UNUSED uint32_t access,
                           amxc_var_t* ret,
                           int timeout) {
    int retval = amxd_status_object_not_found;
    amxb_usp_t* amxb_usp = (amxb_usp_t*) ctx;
    amxc_var_t request;
    amxc_var_t usp_resp;
    amxc_string_t path;
    char* msg_id = NULL;
    bool requires_device = GET_BOOL(amxb_usp_get_config(), AMXB_USP_COPT_REQUIRES_DEVICE);

    amxc_string_init(&path, 0);
    amxc_var_init(&request);
    amxc_var_init(&usp_resp);

    if(requires_device) {
        when_false(amxb_usp_path_starts_with_device(object), exit);
    }

    if(search_path != NULL) {
        amxc_string_setf(&path, "%s%s", object, search_path);
    } else {
        amxc_string_setf(&path, "%s", object);
    }

    amxb_usp_gi_populate_request(&request, &path, depth);

    retval = amxb_usp_send_req(amxb_usp, &request, uspl_get_instances_new, &msg_id);
    when_failed(retval, exit);

    retval = amxb_usp_poll_response(amxb_usp, msg_id, uspl_get_instances_resp_extract, &usp_resp, true, timeout);
    when_failed(retval, exit);

    retval = amxb_usp_get_instances_resp_to_amx(&usp_resp, ret);

exit:
    free(msg_id);
    amxc_string_clean(&path);
    amxc_var_clean(&usp_resp);
    amxc_var_clean(&request);
    return retval;
}
