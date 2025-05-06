/****************************************************************************
**
** Copyright (c) 2020 SoftAtHome
**
** Redistribution and use in source and binary forms, with or
** without modification, are permitted provided that the following
** conditions are met:
**
** 1. Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** 2. Redistributions in binary form must reproduce the above
** copyright notice, this list of conditions and the following
** disclaimer in the documentation and/or other materials provided
** with the distribution.
**
** Subject to the terms and conditions of this license, each
** copyright holder and contributor hereby grants to those receiving
** rights under this license a perpetual, worldwide, non-exclusive,
** no-charge, royalty-free, irrevocable (except for failure to
** satisfy the conditions of this license) patent license to make,
** have made, use, offer to sell, sell, import, and otherwise
** transfer this software, where such license applies only to those
** patent claims, already acquired or hereafter acquired, licensable
** by such copyright holder or contributor that are necessarily
** infringed by:
**
** (a) their Contribution(s) (the licensed copyrights of copyright
** holders and non-copyrightable additions of contributors, in
** source or binary form) alone; or
**
** (b) combination of their Contribution(s) with the work of
** authorship to which such Contribution(s) was added by such
** copyright holder or contributor, if, at the time the Contribution
** is added, such addition causes such combination to be necessarily
** infringed. The patent license shall not apply to any other
** combinations which include the Contribution.
**
** Except as expressly stated above, no rights or licenses from any
** copyright holder or contributor is granted under this license,
** whether expressly, by implication, estoppel or otherwise.
**
** DISCLAIMER
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
** CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
** USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
** AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
** ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>

#include "amxb_dbus_version.h"
#include "amxb_dbus.h"
#include "amxb_dbus_methods.h"

typedef struct _amxb_dbus_request {
    amxb_request_t* request;
    DBusPendingCall* pending;
    bool convert;
} amxb_dbus_request_t;

static void amxb_dbus_free_request(void* user_data) {
    amxb_dbus_request_t* r = (amxb_dbus_request_t*) user_data;
    if(r->request != NULL) {
        r->request->bus_data = NULL;
    }
    free(r);
}

static void amxb_dbus_build_reply_var(DBusMessage* reply, amxc_var_t* result) {
    DBusMessageIter dbus_args;
    amxc_var_t* item = NULL;

    amxc_var_set_type(result, AMXC_VAR_ID_LIST);
    dbus_message_iter_init(reply, &dbus_args);
    while(dbus_message_iter_get_arg_type(&dbus_args) != DBUS_TYPE_INVALID) {
        item = amxc_var_add_new(result);
        amxb_dbus_to_var(item, &dbus_args);
        dbus_message_iter_next(&dbus_args);
    }
}

static void amxb_dbus_method_done(DBusPendingCall* pending, void* user_data) {
    amxb_dbus_request_t* r = (amxb_dbus_request_t*) user_data;
    DBusMessage* reply = dbus_pending_call_steal_reply(pending);
    int32_t status = 0;
    const amxb_bus_ctx_t* ctx = NULL;

    if(dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
        status = amxd_status_unknown_error;
        r->request->bus_retval = amxd_status_unknown_error;
        amxc_var_clean(r->request->result);
    } else {
        amxb_dbus_build_reply_var(reply, r->request->result);
        if(r->convert) {
            amxc_var_t* var = GETI_ARG(r->request->result, 2);
            status = GET_INT32(var, NULL);
            amxc_var_delete(&var);
            var = GETI_ARG(r->request->result, 1);
            if(amxc_var_is_null(var)) {
                amxc_var_delete(&var);
            }
        }
    }
    ctx = amxb_request_get_ctx(r->request);
    if((status == 0) && (r->request->cb_fn != NULL)) {
        r->request->cb_fn(ctx, r->request->result, r->request->priv);
    }

    if(r->request->done_fn != NULL) {
        r->request->done_fn(ctx, r->request, status, r->request->priv);
    }

    r->request = NULL;

    dbus_message_unref(reply);
}

static DBusPendingCall* amxb_dbus_invoke_impl(amxb_dbus_t* amxb_dbus_ctx,
                                              const char* dest,
                                              const char* dbus_object,
                                              amxb_invoke_t* invoke_ctx,
                                              amxc_var_t* args,
                                              int timeout) {
    DBusPendingCall* pending = NULL;
    DBusMessageIter dbus_args;
    DBusMessage* msg = NULL;

    if(timeout != -1) {
        timeout *= 1000;
    }
    msg = dbus_message_new_method_call(dest, dbus_object, invoke_ctx->interface, invoke_ctx->method);
    when_null(msg, exit);

    amxc_var_for_each(arg, GET_ARG(args, "args")) {
        dbus_message_iter_init_append(msg, &dbus_args);
        amxb_var_to_dbus(arg, &dbus_args);
    }

    if(!dbus_connection_send_with_reply(amxb_dbus_ctx->dbus_handle, msg, &pending, timeout)) {
        goto exit;
    }

    dbus_connection_flush(amxb_dbus_ctx->dbus_handle);

exit:
    if(msg != NULL) {
        dbus_message_unref(msg);
    }
    return pending;
}

static char* amxb_dbus_get_destination(amxd_path_t* full_path) {
    char* dest = NULL;
    amxc_string_t real_dest;
    const char* prefix = GET_CHAR(amxb_dbus_get_config_option("destination-prefix"), NULL);
    char* root_obj = amxd_path_get_first(full_path, false);

    amxc_string_init(&real_dest, 0);
    amxc_string_setf(&real_dest, "%s%s", prefix, root_obj);
    amxc_string_trimr(&real_dest, isdot);
    dest = amxc_string_take_buffer(&real_dest);
    amxc_string_clean(&real_dest);

    free(root_obj);
    return dest;
}

static void amxb_dbus_convert_to_execute(amxb_invoke_t* invoke_ctx,
                                         amxd_path_t* full_path,
                                         amxc_var_t* args,
                                         amxc_var_t** converted_args) {
    const char* dm_interface = GET_CHAR(amxb_dbus_get_config_option("dm-interface"), NULL);
    amxc_var_t* dbus_args = NULL;
    invoke_ctx->interface = strdup(dm_interface);
    amxc_var_new(converted_args);
    amxc_var_set_type(*converted_args, AMXC_VAR_ID_HTABLE);
    dbus_args = amxc_var_add_key(amxc_llist_t, *converted_args, "args", NULL);
    amxc_var_add(cstring_t, dbus_args, amxd_path_get(full_path, AMXD_OBJECT_TERMINATE));
    amxc_var_add(cstring_t, dbus_args, invoke_ctx->method);
    amxc_var_set_index(dbus_args, -1, args, AMXC_VAR_FLAG_DEFAULT);
    free(invoke_ctx->method);
    invoke_ctx->method = strdup("execute");
}

int amxb_dbus_async_invoke(void* const ctx,
                           amxb_invoke_t* invoke_ctx,
                           amxc_var_t* args,
                           amxb_request_t* request) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) ctx;
    amxc_var_t* interface = NULL;
    amxc_var_t* object = NULL;
    const char* dbus_object = NULL;
    amxb_dbus_request_t* r = NULL;
    char* dest = NULL;
    int status = -1;
    amxd_path_t full_path;
    amxc_var_t* converted_args = NULL;

    interface = GET_ARG(args, "interface");
    amxc_var_take_it(interface);
    object = GET_ARG(args, "object");
    amxc_var_take_it(object);
    dbus_object = GET_CHAR(object, NULL) == NULL? "/":GET_CHAR(object, NULL);

    amxd_path_init(&full_path, NULL);
    amxd_path_setf(&full_path, true, "%s", invoke_ctx->object);

    r = (amxb_dbus_request_t*) calloc(1, sizeof(amxb_dbus_request_t));

    if(interface != NULL) {
        if(invoke_ctx->interface == NULL) {
            invoke_ctx->interface = strdup(GET_CHAR(interface, NULL));
        }
    } else {
        if(invoke_ctx->interface == NULL) {
            dest = amxb_dbus_get_destination(&full_path);
            amxb_dbus_convert_to_execute(invoke_ctx, &full_path, args, &converted_args);
            r->convert = true;
        }
    }

    r->pending = amxb_dbus_invoke_impl(amxb_dbus_ctx,
                                       dest == NULL? amxd_path_get(&full_path, 0):dest,
                                       dbus_object,
                                       invoke_ctx,
                                       converted_args == NULL? args:converted_args,
                                       -1);
    request->bus_data = r;
    r->request = request;
    dbus_pending_call_set_notify(r->pending, amxb_dbus_method_done, r, amxb_dbus_free_request);

    status = 0;

    amxc_var_take_it(args);
    amxc_var_delete(&converted_args);
    amxc_var_delete(&object);
    amxc_var_delete(&interface);
    free(dest);
    amxd_path_clean(&full_path);

    return status;
}

int amxb_dbus_invoke(void* const ctx,
                     amxb_invoke_t* invoke_ctx,
                     amxc_var_t* args,
                     amxb_request_t* request,
                     int timeout) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) ctx;
    amxc_var_t* interface = NULL;
    amxc_var_t* object = NULL;
    const char* dbus_object = NULL;
    amxb_dbus_request_t r;
    char* dest = NULL;
    int status = -1;
    amxd_path_t full_path;
    amxc_var_t* converted_args = NULL;

    interface = GET_ARG(args, "interface");
    amxc_var_take_it(interface);
    object = GET_ARG(args, "object");
    amxc_var_take_it(object);
    dbus_object = GET_CHAR(object, NULL) == NULL? "/":GET_CHAR(object, NULL);

    amxd_path_init(&full_path, NULL);
    amxd_path_setf(&full_path, true, "%s", invoke_ctx->object);

    if(interface != NULL) {
        if(invoke_ctx->interface == NULL) {
            invoke_ctx->interface = strdup(GET_CHAR(interface, NULL));
        }
    } else {
        if(invoke_ctx->interface == NULL) {
            dest = amxb_dbus_get_destination(&full_path);
            amxb_dbus_convert_to_execute(invoke_ctx, &full_path, args, &converted_args);
            r.convert = true;
        }
    }

    r.pending = amxb_dbus_invoke_impl(amxb_dbus_ctx,
                                      dest == NULL? amxd_path_get(&full_path, 0):dest,
                                      dbus_object,
                                      invoke_ctx,
                                      args,
                                      timeout);
    request->bus_data = &r;
    r.request = request;
    dbus_pending_call_block(r.pending);
    amxb_dbus_method_done(r.pending, &r);
    dbus_pending_call_unref(r.pending);

    status = request->bus_retval;

    amxc_var_take_it(args);
    amxc_var_delete(&object);
    amxc_var_delete(&interface);
    amxc_var_delete(&converted_args);
    free(dest);
    amxd_path_clean(&full_path);

    return status;
}

int amxb_dbus_close_request(UNUSED void* const ctx, amxb_request_t* request) {
    amxb_dbus_request_t* r = (amxb_dbus_request_t*) request->bus_data;

    if((r != NULL) && (r->pending != NULL)) {
        dbus_pending_call_cancel(r->pending);
        dbus_pending_call_unref(r->pending);
        request->bus_data = NULL;
    }
    return 0;

}