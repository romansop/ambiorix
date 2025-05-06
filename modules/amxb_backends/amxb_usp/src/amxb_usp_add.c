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
#include <sys/sysinfo.h>

#include <amxc/amxc.h>
#include <amxp/amxp_signal.h>
#include <amxd/amxd_function.h>
#include <amxd/amxd_action.h>
#include <amxd/amxd_types.h>
#include <amxd/amxd_path.h>
#include <amxd/amxd_transaction.h>

#include <usp/uspl.h>
#include <uspi/uspi_subscription.h>

#include "amxb_usp.h"

static void amxd_usp_add_resp_handle_list(amxc_llist_t* resp_list,
                                          amxc_var_t* retval,
                                          const char* requested_path) {
    amxc_var_for_each(rv, retval) {
        amxc_var_t* ret = NULL;
        amxc_var_t* result = NULL;

        amxc_var_new(&ret);
        amxc_var_move(ret, rv);
        result = amxc_var_add_key(amxc_htable_t, ret, "result", NULL);
        amxc_var_add_key(cstring_t, result, "requested_path", requested_path);
        amxc_var_add_key(uint32_t, result, "err_code", 0);
        amxc_llist_append(resp_list, &ret->lit);
    }
}

static int amxb_usp_invoke_add(amxb_usp_deferred_t* fcall,
                               const char* target_path,
                               amxc_var_t* parameters,
                               const char* requested_path) {
    amxd_status_t status = amxd_status_ok;
    amxc_var_t args;
    amxc_var_t* ret = NULL;
    amxc_var_t* result = NULL;
    amxc_var_t* params = NULL;

    amxc_var_init(&args);

    amxc_var_new(&ret);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "access", AMXB_PUBLIC);
    params = amxc_var_add_key(amxc_htable_t, &args, "parameters", NULL);

    amxc_var_for_each(param, parameters) {
        amxc_var_add_key(cstring_t, params, GET_CHAR(param, "param"), GET_CHAR(param, "value"));
    }

    status = amxb_usp_amx_call(fcall, target_path, requested_path, false, "_add", &args, ret, amxb_usp_amx_call_done);
    if(status == amxd_status_deferred) {
        amxc_var_delete(&ret);
        goto exit;
    }

    // Initialize rv if invoke failed
    if(status != amxd_status_ok) {
        amxc_var_set_type(ret, AMXC_VAR_ID_HTABLE);
    }

    // Translate response
    amxb_usp_call_translate(ret, AMXB_USP_TRANSLATE_MULTIPLE);

    if(amxc_var_type_of(ret) == AMXC_VAR_ID_LIST) {
        amxd_usp_add_resp_handle_list(&fcall->resp_list, ret, requested_path);
        amxc_var_delete(&ret);
        goto exit;
    }

    result = amxc_var_add_key(amxc_htable_t, ret, "result", NULL);
    amxc_var_add_key(uint32_t, result, "err_code", status);
    amxc_var_add_key(cstring_t, result, "requested_path", requested_path);

    amxc_llist_append(&fcall->resp_list, &ret->lit);

exit:
    amxc_var_clean(&args);

    return status;
}

static amxd_status_t amxb_usp_add_translated(amxb_usp_deferred_t* fcall,
                                             const char* requested_path,
                                             amxc_var_t* parameters) {
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

    status = amxb_usp_invoke_add(fcall, amxd_path_get(&path, AMXD_OBJECT_TERMINATE), parameters, requested_path);
    if(status == amxd_status_deferred) {
        deferred = true;
    }

    amxd_path_clean(&path);
    return deferred;
}

static void amxb_usp_add_populate_request(amxc_var_t* add_var,
                                          const char* object,
                                          const char* search_path,
                                          amxc_var_t* values) {
    amxc_var_t* requests = NULL;
    amxc_var_t* request = NULL;
    const amxc_htable_t* value_table = NULL;
    amxc_string_t path;

    amxc_string_init(&path, 0);
    if(search_path != NULL) {
        amxc_string_setf(&path, "%s%s", object, search_path);
    } else {
        amxc_string_setf(&path, "%s", object);
    }

    amxc_var_set_type(add_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(bool, add_var, "allow_partial", true);
    requests = amxc_var_add_key(amxc_llist_t, add_var, "requests", NULL);
    request = amxc_var_add(amxc_htable_t, requests, NULL);
    amxc_var_add_key(cstring_t, request, "object_path", amxc_string_get(&path, 0));

    value_table = amxc_var_constcast(amxc_htable_t, values);
    if(amxc_htable_size(value_table) > 0) {
        amxc_var_t* params = amxc_var_add_key(amxc_llist_t, request, "parameters", NULL);
        amxc_htable_iterate(it, value_table) {
            amxc_var_t* entry = amxc_var_from_htable_it(it);
            amxc_var_t* param = amxc_var_add(amxc_htable_t, params, NULL);

            amxc_var_cast(entry, AMXC_VAR_ID_CSTRING);
            amxc_var_add_key(cstring_t, param, "param", amxc_htable_it_get_key(it));
            amxc_var_add_key(cstring_t, param, "value", amxc_var_constcast(cstring_t, entry));
            amxc_var_add_key(bool, param, "required", true);
        }
    }

    amxc_string_clean(&path);
}

void amxb_usp_add_deferred_resp(const amxc_var_t* const data,
                                amxb_usp_deferred_t* fcall,
                                amxb_usp_deferred_child_t* child) {
    amxc_var_t* ret = NULL;
    amxc_var_t* result = NULL;
    amxc_var_t* retval = GET_ARG(data, "retval");

    amxc_var_new(&ret);

    if(amxc_var_type_of(retval) == AMXC_VAR_ID_HTABLE) {
        amxc_var_move(ret, retval);
    } else if(amxc_var_type_of(retval) == AMXC_VAR_ID_LIST) {
        amxd_usp_add_resp_handle_list(&fcall->resp_list, retval, child->requested_path);
        amxc_var_delete(&ret);
        goto exit;
    } else {
        amxc_var_set_type(ret, AMXC_VAR_ID_HTABLE);
    }
    result = amxc_var_add_key(amxc_htable_t, ret, "result", NULL);
    amxc_var_add_key(cstring_t, result, "requested_path", child->requested_path);
    amxc_var_add_key(uint32_t, result, "err_code", GET_UINT32(data, "status"));
    amxc_llist_append(&fcall->resp_list, &ret->lit);

exit:
    if(fcall->num_calls == 0) {
        amxb_usp_reply_deferred(fcall, uspl_add_resp_new);
        amxb_usp_deferred_delete(&fcall);
    }
}

int amxb_usp_handle_add(amxb_usp_t* ctx, uspl_rx_t* usp_rx) {
    int retval = -1;
    amxc_var_t add_var;
    amxc_var_t* requests = NULL;
    bool deferred = false;
    amxb_usp_deferred_t* fcall = NULL;

    amxc_var_init(&add_var);

    when_null(ctx, exit);
    when_null(usp_rx, exit);

    ctx->stats.counter_rx_add++;

    retval = uspl_add_extract(usp_rx, &add_var);
    if(retval != 0) {
        retval = amxb_usp_reply_error(ctx, usp_rx, retval);
        goto exit;
    }

    requests = GET_ARG(&add_var, "requests");
    retval = amxb_usp_deferred_new(&fcall, ctx, usp_rx,
                                   amxc_llist_size(amxc_var_constcast(amxc_llist_t, requests)));
    when_failed(retval, exit);

    amxc_var_for_each(request, requests) {
        const char* path = amxc_var_constcast(cstring_t, GET_ARG(request, "object_path"));
        deferred |= amxb_usp_add_translated(fcall, path, GET_ARG(request, "parameters"));
    }

    if(deferred) {
        retval = 0;
        goto exit;
    } else {
        retval = amxb_usp_reply(ctx, usp_rx, &fcall->resp_list, uspl_add_resp_new);
        amxb_usp_deferred_delete(&fcall);
    }

exit:
    amxc_var_clean(&add_var);
    return retval;
}

int amxb_usp_add(void* const ctx,
                 const char* object,
                 const char* search_path,
                 UNUSED uint32_t index,
                 UNUSED const char* name,
                 amxc_var_t* values,
                 UNUSED uint32_t access,
                 amxc_var_t* ret,
                 int timeout) {
    int retval = amxd_status_object_not_found;
    amxb_usp_t* amxb_usp = (amxb_usp_t*) ctx;
    amxc_var_t add_var;
    char* msg_id = NULL;
    bool requires_device = GET_BOOL(amxb_usp_get_config(), AMXB_USP_COPT_REQUIRES_DEVICE);

    amxc_var_init(&add_var);

    if(requires_device) {
        when_false(amxb_usp_path_starts_with_device(object), exit);
    }

    amxb_usp_add_populate_request(&add_var, object, search_path, values);

    retval = amxb_usp_send_req(amxb_usp, &add_var, uspl_add_new, &msg_id);
    when_failed(retval, exit);

    retval = amxb_usp_poll_response(amxb_usp, msg_id, uspl_add_resp_extract, ret, true, timeout);
    when_failed(retval, exit);

    retval = amxb_usp_filter_ret(ret);

exit:
    free(msg_id);
    amxc_var_clean(&add_var);
    return retval;
}
