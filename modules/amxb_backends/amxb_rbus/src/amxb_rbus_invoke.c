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

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "amxb_rbus.h"
#include "amxb_rbus_ctrl.h"

static amxd_status_t amxb_rbus_convert_return_values(rbusObject_t out_params,
                                                     amxc_var_t* result,
                                                     rbusError_t bus_status) {
    amxd_status_t status = amxd_status_ok;

    if(bus_status == RBUS_ERROR_SUCCESS) {
        amxc_var_t* out_args = NULL;
        amxc_var_t* status_var = NULL;
        rbusProperty_t property = rbusObject_GetProperties(out_params);
        if(property != NULL) {
            amxb_rbus_object_to_var(result, out_params);
        } else {
            amxb_rbus_object_to_lvar(result, out_params);

            out_args = GETI_ARG(result, 1);
            status_var = GETI_ARG(result, 2);
            if((out_args != NULL) && (status_var != NULL)) {
                if((amxc_var_type_of(out_args) == AMXC_VAR_ID_HTABLE) &&
                   (amxc_var_type_of(status_var) == AMXC_VAR_ID_UINT32)) {
                    if(amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, out_args))) {
                        amxc_var_delete(&out_args);
                    }
                    status = (amxd_status_t) GET_UINT32(status_var, NULL);
                    amxc_var_delete(&status_var);
                }
            }
        }
    } else {
        status = amxb_rbus_translate_rbus2status(bus_status);
    }

    return status;
}

static void amxb_rbus_async_cleanup_handler(void* data) {
    amxb_rbus_item_t* item = (amxb_rbus_item_t*) data;
    amxb_rbus_t* ctx = NULL;

    if(item->it.llist != NULL) {
        ctx = amxc_container_of(item->it.llist, amxb_rbus_t, out);
    }

    if(ctx != NULL) {
        amxb_rbus_ctrl_write(ctx, item);
    }
}


static void* amxb_rbus_async_invoke_impl(void* data) {
    amxb_rbus_item_t* item = (amxb_rbus_item_t*) data;
    amxb_rbus_t* ctx = NULL;

    if(item->it.llist != NULL) {
        ctx = amxc_container_of(item->it.llist, amxb_rbus_t, out);
    }

    if(ctx != NULL) {
        pthread_cleanup_push(amxb_rbus_async_cleanup_handler, item);
        // apply the timeout - asynchronous calls should not have a timeout
        // but it seems that it is not possible using rbus
        // currently set the timeout to 1 hour
        amxb_rbus_set_timeout(3600);

        item->status = rbusMethod_Invoke(ctx->handle,
                                         amxc_string_get(&item->name, 0),
                                         item->in_params,
                                         &item->out_params);
        if(item->in_params != NULL) {
            rbusObject_Release(item->in_params);
            item->in_params = NULL;
        }
        pthread_cleanup_pop(1);
    }

    return NULL;
}

static void amxb_rbus_complete_async(UNUSED amxb_rbus_t* amxb_rbus_ctx,
                                     amxb_rbus_item_t* item) {
    amxb_request_t* request = (amxb_request_t*) item->priv;
    const amxb_bus_ctx_t* ctx = NULL;
    amxd_status_t status = amxd_status_ok;

    if(request != NULL) {
        ctx = amxb_request_get_ctx(request);
        request->bus_data = NULL;

        status = amxb_rbus_convert_return_values(item->out_params, request->result, item->status);
        request->bus_retval = status;

        if(request->done_fn != NULL) {
            request->done_fn(ctx, request, status, request->priv);
        }
    }
}

static int amxb_rbus_get_longest_path(amxb_rbus_t* amxb_rbus_ctx,
                                      char** path,
                                      amxc_var_t* rel_path) {
    amxd_path_t longest_path;
    amxc_string_t new_rel_path;
    rbusElementInfo_t* elems = NULL;
    int retval = 0;

    amxd_path_init(&longest_path, *path);
    amxc_string_init(&new_rel_path, 0);
    amxc_string_set(&new_rel_path, GET_CHAR(rel_path, NULL));

    do {
        retval = rbusElementInfo_get(amxb_rbus_ctx->handle, amxd_path_get(&longest_path, AMXD_OBJECT_TERMINATE), 0, &elems);
        if(elems->type == RBUS_ELEMENT_TYPE_TABLE) {
            char* part = amxd_path_get_last(&longest_path, true);
            amxc_string_prepend(&new_rel_path, part, strlen(part));
            free(part);
            rbusElementInfo_free(amxb_rbus_ctx->handle, elems);
        } else {
            rbusElementInfo_free(amxb_rbus_ctx->handle, elems);
            break;
        }
    } while(retval == 0);

    amxc_var_set(cstring_t, rel_path, amxc_string_get(&new_rel_path, 0));
    amxc_string_clean(&new_rel_path);
    free(*path);
    *path = amxd_path_get_fixed_part(&longest_path, true);
    amxd_path_clean(&longest_path);

    return retval;
}

static int amxb_rbus_invoke_impl(amxb_rbus_t* amxb_rbus_ctx,
                                 const char* object,
                                 const char* method,
                                 amxc_var_t* args,
                                 amxb_request_t* request) {
    int ret = 0;
    rbusObject_t in_params = NULL;
    rbusObject_t out_params = NULL;
    amxc_string_t name;

    amxc_string_init(&name, 0);

    rbusObject_Init(&in_params, NULL);
    amxb_rbus_htvar_to_robject(args, in_params);
    amxc_var_set_type(request->result, AMXC_VAR_ID_LIST);
    amxc_string_setf(&name, "%s.%s()", object, method);

    ret = rbusMethod_Invoke(amxb_rbus_ctx->handle,
                            amxc_string_get(&name, 0),
                            in_params,
                            &out_params);
    rbusObject_Release(in_params);

    ret = amxb_rbus_convert_return_values(out_params, request->result, ret);

    rbusObject_Release(out_params);
    amxc_string_clean(&name);
    return ret;
}

int amxb_rbus_invoke_root(amxb_rbus_t* amxb_rbus_ctx,
                          const char* object,
                          const char* method,
                          amxc_var_t* args,
                          amxb_request_t* request) {
    int retval = 0;
    amxd_path_t path;
    char* root_obj = NULL;
    int len = 0;
    amxc_var_t* rel_path = GET_ARG(args, "rel_path");
    amxc_string_t full_rel_path;

    amxd_path_init(&path, NULL);
    amxd_path_setf(&path, true, "%s", object);

    root_obj = amxd_path_get_fixed_part(&path, true);
    amxc_string_init(&full_rel_path, 0);
    amxc_string_setf(&full_rel_path, "%s%s",
                     amxd_path_get(&path, AMXD_OBJECT_TERMINATE),
                     GET_CHAR(rel_path, NULL));
    amxc_var_set(cstring_t, rel_path, amxc_string_get(&full_rel_path, 0));
    amxc_string_clean(&full_rel_path);

    retval = amxb_rbus_get_longest_path(amxb_rbus_ctx, &root_obj, rel_path);
    when_failed(retval, exit);

    len = strlen(root_obj) - 1;
    if(root_obj[len] == '.') {
        root_obj[len] = 0;
    }

    retval = amxb_rbus_invoke_impl(amxb_rbus_ctx, root_obj, method, args, request);

exit:
    free(root_obj);
    amxd_path_clean(&path);

    return retval;
}

int amxb_rbus_async_invoke(void* const ctx,
                           amxb_invoke_t* invoke_ctx,
                           amxc_var_t* args,
                           amxb_request_t* request) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    int ret = amxd_status_invalid_value;
    amxb_rbus_item_t* rbus_item = NULL;

    when_str_empty(invoke_ctx->object, exit);

    rbus_item = (amxb_rbus_item_t*) calloc(1, sizeof(amxb_rbus_item_t));
    when_null(rbus_item, exit);

    rbusObject_Init(&rbus_item->in_params, NULL);
    amxb_rbus_htvar_to_robject(args, rbus_item->in_params);
    rbus_item->priv = request;
    amxc_var_set_type(request->result, AMXC_VAR_ID_LIST);

    amxc_string_setf(&rbus_item->name, "%s.%s()",
                     invoke_ctx->object, invoke_ctx->method);
    rbus_item->handler = amxb_rbus_complete_async;
    request->bus_data = rbus_item;

    // The rbus API provides a function to do asynchronous RPC calls,
    // but it is not possible to pass private data to it, and it is not possible
    // to cancel or wait until the call finishes. To work around this
    // a thread is started that uses the synchronous API instead.
    //
    // The implementation of the rbus async call is also creating a new thread
    // and performing the call synchronous but with a custom timeout
    //
    // Use pthread_timedjoin_np to wait until the thread completes or timeout expires
    // Canceling the thread is not needed, the request can be canceled instead.
    ret = amxb_rbus_ctrl_start(amxb_rbus_ctx, rbus_item, amxb_rbus_async_invoke_impl);

    if(ret != 0) {
        request->bus_data = NULL;
        rbusObject_Release(rbus_item->in_params);
        amxc_string_clean(&rbus_item->name);
        free(rbus_item);
    }

exit:
    return ret;
}

int amxb_rbus_invoke(void* const ctx,
                     amxb_invoke_t* invoke_ctx,
                     amxc_var_t* args,
                     amxb_request_t* request,
                     int timeout) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    int ret = amxd_status_invalid_value;

    when_str_empty(invoke_ctx->object, exit);

    // apply the timeout
    amxb_rbus_set_timeout(timeout);
    ret = amxb_rbus_invoke_impl(amxb_rbus_ctx, invoke_ctx->object, invoke_ctx->method, args, request);

exit:
    return ret;
}

int amxb_rbus_close_request(UNUSED void* const ctx,
                            amxb_request_t* request) {
    amxb_rbus_item_t* rbus_item = (amxb_rbus_item_t*) request->bus_data;
    when_null(rbus_item, exit);

    rbus_item->priv = NULL;
    request->bus_data = NULL;
    // Set state to closed, the read handler will free the item and drop it.
    rbus_item->state = AMXB_RBUS_ITEM_CLOSED;
    // Keep this code in here, continue reading ...
    //if (rbus_item->tid != 0) {
    // When canceling the thread that executes the async call
    // an invalid read can occur in rhe rbus reader thread.
    //==1596911== Thread 2:
    //==1596911== Invalid read of size 4
    //==1596911==    at 0x4B734BB: rtConnection_ReaderThread (in /usr/local/lib/librtMessage.so.2.0.5)
    //==1596911==    by 0x4941EA6: start_thread (pthread_create.c:477)
    //==1596911==    by 0x4A57A2E: clone (clone.S:95)
    //==1596911==  Address 0x69d4b40 is on thread 4's stack
    //==1596911==  304 bytes below stack pointer
    // Either ignore this or don't cancel the thread.
    //pthread_cancel(rbus_item->tid);
    //}

exit:
    return 0;
}

int amxb_rbus_wait_request(UNUSED void* const ctx,
                           amxb_request_t* request,
                           int timeout) {
    struct timespec ts;
    int retval = 0;
    amxb_rbus_item_t* rbus_item = (amxb_rbus_item_t*) request->bus_data;

    when_null(rbus_item, exit);
    when_true(rbus_item->tid == 0, exit);
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout;

    retval = pthread_timedjoin_np(rbus_item->tid, NULL, &ts);
    if(retval == ETIMEDOUT) {
        retval = AMXB_ERROR_BUS_TIMEOUT;
    }

exit:
    return retval;
}
