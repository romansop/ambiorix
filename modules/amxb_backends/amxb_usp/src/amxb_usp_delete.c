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
#include <string.h>

#include <amxc/amxc.h>
#include <amxp/amxp_signal.h>
#include <amxd/amxd_function.h>
#include <amxd/amxd_action.h>
#include <amxd/amxd_types.h>
#include <amxd/amxd_path.h>

#include <usp/uspl.h>

#include "amxb_usp.h"

/* Extend the reply variant with additional information needed to build a USP response message */
static void amxb_usp_delete_extend(amxc_var_t* reply_data,
                                   const char* requested_path,
                                   amxd_status_t err_code,
                                   amxc_var_t* del_response) {
    amxc_var_t* result = NULL;

    amxc_var_set_type(reply_data, AMXC_VAR_ID_HTABLE);
    result = amxc_var_add_key(amxc_htable_t, reply_data, "result", NULL);
    amxc_var_add_key(cstring_t, result, "requested_path", requested_path);
    amxc_var_add_key(uint32_t, result, "err_code", err_code);

    if(err_code == amxd_status_ok) {
        const amxc_llist_t* rv_list = amxc_var_constcast(amxc_llist_t, del_response);
        amxc_var_add_key(amxc_llist_t, reply_data, "affected_paths", rv_list);
        amxc_var_add_key(amxc_llist_t, reply_data, "unaffected_paths", NULL);
    } else {
        const char* err_msg = uspl_error_code_to_str(uspl_amxd_status_to_usp_error(err_code));
        amxc_var_add_key(cstring_t, result, "err_msg", err_msg);
    }
}

static amxd_status_t amxb_usp_delete_invoke(amxb_usp_deferred_t* fcall,
                                            const char* target_path,
                                            const char* requested_path) {
    amxd_status_t status = amxd_status_ok;
    amxc_var_t args;
    amxc_var_t rv;
    amxc_var_t* ret = NULL;

    amxc_var_init(&args);
    amxc_var_init(&rv);

    amxc_var_new(&ret);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "access", AMXB_PUBLIC);

    status = amxb_usp_amx_call(fcall, target_path, requested_path, false, "_del", &args, &rv, amxb_usp_amx_call_done);
    if(status == amxd_status_deferred) {
        amxc_var_delete(&ret);
        goto exit;
    }

    // Translate response
    amxb_usp_call_translate(&rv, AMXB_USP_TRANSLATE_LIST);

    // Create reply variant with additional information needed to build a USP response message
    amxb_usp_delete_extend(ret, requested_path, status, &rv);
    amxc_llist_append(&fcall->resp_list, &ret->lit);

exit:
    amxc_var_clean(&rv);
    amxc_var_clean(&args);

    return status;
}

static amxd_status_t amxb_usp_delete_translated(amxb_usp_deferred_t* fcall,
                                                const char* requested_path) {
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

    status = amxb_usp_delete_invoke(fcall, amxd_path_get(&path, AMXD_OBJECT_TERMINATE), requested_path);
    if(status == amxd_status_deferred) {
        deferred = true;
    }

    amxd_path_clean(&path);
    return deferred;
}

static void amxb_usp_delete_populate_request(amxc_var_t* del_var,
                                             const char* object,
                                             const char* search_path) {
    amxc_string_t path;
    amxc_var_t* requests = NULL;

    amxc_string_init(&path, 0);
    if(search_path != NULL) {
        amxc_string_setf(&path, "%s%s", object, search_path);
    } else {
        amxc_string_setf(&path, "%s", object);
    }

    amxc_var_add_key(bool, del_var, "allow_partial", true);
    requests = amxc_var_add_key(amxc_llist_t, del_var, "requests", NULL);
    amxc_var_add(cstring_t, requests, amxc_string_get(&path, 0));

    amxc_string_clean(&path);
}

static uint32_t amxb_usp_del_resp_to_amx(amxc_var_t* usp_resp, amxc_var_t* ret) {
    uint32_t status = uspl_usp_error_to_amxd_status(GETP_UINT32(usp_resp, "result.err_code"));
    if(status == amxd_status_ok) {
        amxc_var_t* affected_paths = GET_ARG(usp_resp, "affected_paths");
        amxc_var_move(ret, affected_paths);
    }
    return status;
}

void amxb_usp_delete_deferred_resp(const amxc_var_t* const data,
                                   amxb_usp_deferred_t* fcall,
                                   amxb_usp_deferred_child_t* child) {
    amxc_var_t* ret = NULL;
    amxc_var_t* retval = GET_ARG(data, "retval");

    amxc_var_new(&ret);

    amxb_usp_delete_extend(ret, child->requested_path, GET_UINT32(data, "status"), retval);
    amxc_llist_append(&fcall->resp_list, &ret->lit);

    if(fcall->num_calls == 0) {
        amxb_usp_reply_deferred(fcall, uspl_delete_resp_new);
        amxb_usp_deferred_delete(&fcall);
    }
}

int amxb_usp_handle_delete(amxb_usp_t* ctx, uspl_rx_t* usp_rx) {
    int retval = -1;
    amxc_var_t del_var;
    amxc_var_t* requests = NULL;
    bool deferred = false;
    amxb_usp_deferred_t* fcall = NULL;

    amxc_var_init(&del_var);

    when_null(ctx, exit);
    when_null(usp_rx, exit);

    ctx->stats.counter_rx_del++;

    retval = uspl_delete_extract(usp_rx, &del_var);
    if(retval != 0) {
        retval = amxb_usp_reply_error(ctx, usp_rx, retval);
        goto exit;
    }

    requests = GET_ARG(&del_var, "requests");
    retval = amxb_usp_deferred_new(&fcall, ctx, usp_rx,
                                   amxc_llist_size(amxc_var_constcast(amxc_llist_t, requests)));
    when_failed(retval, exit);

    amxc_var_for_each(entry, requests) {
        const char* path = amxc_var_constcast(cstring_t, entry);
        deferred |= amxb_usp_delete_translated(fcall, path);
    }

    if(deferred) {
        retval = 0;
        goto exit;
    } else {
        retval = amxb_usp_reply(ctx, usp_rx, &fcall->resp_list, uspl_delete_resp_new);
        amxb_usp_deferred_delete(&fcall);
    }

exit:
    amxc_var_clean(&del_var);
    return retval;
}

int amxb_usp_delete(void* const ctx,
                    const char* object,
                    const char* search_path,
                    UNUSED uint32_t index,
                    UNUSED const char* name,
                    UNUSED uint32_t access,
                    amxc_var_t* ret,
                    int timeout) {
    int retval = amxd_status_object_not_found;
    amxb_usp_t* amxb_usp = (amxb_usp_t*) ctx;
    amxc_var_t del_var;
    amxc_var_t usp_resp;
    char* msg_id = NULL;
    bool requires_device = GET_BOOL(amxb_usp_get_config(), AMXB_USP_COPT_REQUIRES_DEVICE);

    amxc_var_init(&usp_resp);
    amxc_var_init(&del_var);
    amxc_var_set_type(&del_var, AMXC_VAR_ID_HTABLE);

    if(requires_device) {
        when_false(amxb_usp_path_starts_with_device(object), exit);
    }

    amxb_usp_delete_populate_request(&del_var, object, search_path);

    retval = amxb_usp_send_req(amxb_usp, &del_var, uspl_delete_new, &msg_id);
    when_failed(retval, exit);

    retval = amxb_usp_poll_response(amxb_usp, msg_id, uspl_delete_resp_extract, &usp_resp, false, timeout);
    when_failed(retval, exit);

    retval = amxb_usp_del_resp_to_amx(&usp_resp, ret);

exit:
    free(msg_id);
    amxc_var_clean(&usp_resp);
    amxc_var_clean(&del_var);
    return retval;
}
