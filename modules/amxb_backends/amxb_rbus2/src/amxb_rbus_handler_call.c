/****************************************************************************
**
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
** SPDX-FileCopyrightText: Copyright (c) 2024 SoftAtHome
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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "amxb_rbus.h"
#include "amxb_rbus_ctrl.h"
#include "amxb_rbus_handlers.h"

typedef struct _call_data {
    rbusMethodAsyncHandle_t asynchandle;
    char* method;
} call_data_t;

static void amxb_rbus_send_call_reply(amxc_var_t* out, const char* method, rbusMethodAsyncHandle_t asynchandle) {
    rbusObject_t out_params = NULL;
    rbusObject_t children = NULL;
    uint32_t translation_function = amxb_rbus_call_needs_translation(method);

    // if translate option is set and the method called was one of the internal
    // metods (starting with _), the paths in the returned data structure
    // possibly needs translation.
    amxb_rbus_call_translate(out, translation_function);

    // always return an array containing 3 elements:
    // 1. the return value
    // 2. the out arguments (or an empty table if no out-arguments)
    // 3. the libamxd status code.

    rbusObject_Init(&out_params, NULL);
    rbusObject_Init(&children, NULL);

    amxb_rbus_lvar_to_robject(out, children);
    rbusObject_SetChildren(out_params, children);
    rbusObject_Release(children);

    // Altougth the documentation states that a NULL pointer can be passed
    // to this function for the out parameters, it should not be done as it
    // causes a memory leak. When a NULL pointer is passed for the out parameters
    // the function will allocated one, but doesn't free it.

    // use rbus error success, otherwhise rbus is removing all detailed
    // error information.
    rbusMethod_SendAsyncResponse(asynchandle, RBUS_ERROR_SUCCESS, out_params);
    rbusObject_Release(out_params);
}

static void amxb_rbus_exec_done(const amxc_var_t* const data,
                                void* const priv) {
    call_data_t* call_data = (call_data_t*) priv;
    rbusMethodAsyncHandle_t asynchandle = call_data->asynchandle;
    amxc_var_t out;
    amxc_var_t* ret = GET_ARG(data, "retval");
    amxc_var_t* args = GET_ARG(data, "args");
    uint32_t status = GET_UINT32(data, "status");

    amxc_var_init(&out);
    amxc_var_take_it(ret);
    amxc_var_take_it(args);

    amxc_var_set_type(&out, AMXC_VAR_ID_LIST);
    amxc_var_set_index(&out, -1, ret, AMXC_VAR_FLAG_DEFAULT);
    if(!amxc_var_is_null(args) && (amxc_var_type_of(args) == AMXC_VAR_ID_HTABLE)) {
        amxc_var_set_index(&out, -1, args, AMXC_VAR_FLAG_DEFAULT);
    } else {
        amxc_var_add(amxc_htable_t, &out, NULL);
    }
    amxc_var_add(uint32_t, &out, status);

    amxb_rbus_send_call_reply(&out, call_data->method, asynchandle);

    free(call_data->method);
    free(call_data);
    amxc_var_delete(&ret);
    amxc_var_delete(&args);
    amxc_var_clean(&out);
}

static void amxb_rbus_call_impl(amxb_rbus_t* amxb_rbus_ctx,
                                amxb_rbus_item_t* item) {
    rbusMethodAsyncHandle_t asynchandle = (rbusMethodAsyncHandle_t) item->priv;
    amxd_status_t status = amxd_status_object_not_found;
    amxd_path_t path;
    const char* requested = NULL;
    const char* translated = NULL;
    amxd_object_t* object = NULL;
    amxc_var_t* args = NULL;
    amxc_var_t* ret = NULL;
    amxc_var_t out;

    amxd_path_init(&path, amxc_string_get(&item->name, 0));
    amxc_var_init(&out);

    amxc_var_set_type(&out, AMXC_VAR_ID_LIST);
    ret = amxc_var_add_new(&out);
    args = amxc_var_add_new(&out);

    // Fetch in arguments - convert from rbus data structures to amx var
    amxb_rbus_object_to_var(args, item->in_params);
    // check if path translation is needed
    amxb_rbus_translate_path(&path, &requested, &translated);

    // As it is possible that in the time between receiving the message and the
    // switch to the main thread, the object was removed.
    // Therefor it is possible the object was not found.
    object = amxd_dm_findf(amxb_rbus_ctx->dm, "%s", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
    if(object != NULL) {
        // call the function
        status = amxd_object_invoke_function(object, amxd_path_get_param(&path), args, ret);

        if(status == amxd_status_deferred) {
            // the function will reply later.
            uint64_t call_id = amxc_var_constcast(uint64_t, ret);
            call_data_t* data = (call_data_t*) calloc(1, sizeof(call_data_t));
            data->method = strdup(amxd_path_get_param(&path));
            data->asynchandle = asynchandle;
            amxd_function_set_deferred_cb(call_id, amxb_rbus_exec_done, data);
            goto exit;
        }
    }
    amxc_var_add(uint32_t, &out, status);
    amxb_rbus_send_call_reply(&out, amxd_path_get_param(&path), asynchandle);

exit:
    amxd_path_clean(&path);
    amxc_var_clean(&out);
}

rbusError_t amxb_rbus_call_method(rbusHandle_t handle,
                                  char const* method_name,
                                  rbusObject_t in_params,
                                  UNUSED rbusObject_t out_params,
                                  rbusMethodAsyncHandle_t asyncHandle) {
    rbusError_t status = RBUS_ERROR_INVALID_CONTEXT;
    amxb_rbus_t* amxb_rbus_ctx = amxb_rbus_get_ctx(handle);
    amxb_rbus_item_t* rbus_item = NULL;

    when_null(amxb_rbus_ctx, exit);

    rbus_item = (amxb_rbus_item_t*) calloc(1, sizeof(amxb_rbus_item_t));
    status = RBUS_ERROR_OUT_OF_RESOURCES;
    when_null(rbus_item, exit);

    rbus_item->in_params = in_params;
    rbusObject_Retain(in_params);

    rbus_item->status = RBUS_ERROR_SUCCESS;
    amxc_string_init(&rbus_item->name, 0);
    amxc_string_set(&rbus_item->name, method_name);
    amxc_string_trimr(&rbus_item->name, ispunct);
    rbus_item->priv = asyncHandle;
    rbus_item->handler = amxb_rbus_call_impl;

    amxb_rbus_ctrl_write(amxb_rbus_ctx, rbus_item);
    status = RBUS_ERROR_ASYNC_RESPONSE;

exit:
    return status;
}
