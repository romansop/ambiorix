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

#include "amxb_pcb.h"

#include "amxb_pcb_serialize.h"

#include <amxc/amxc.h>

#include <amxa/amxa_merger.h>
#include <amxa/amxa_permissions.h>
#include <amxa/amxa_resolver.h>
#include <amxa/amxa_validator.h>

#include <usermngt/usermngt.h>

static void amxb_pcb_amx_subscribe_done(UNUSED const amxc_var_t* const data,
                                        void* const priv) {
    amxb_pcb_deferred_t* fcall = (amxb_pcb_deferred_t*) priv;
    amxb_pcb_deferred_t* main_fcall = NULL;

    if(fcall->it.llist != NULL) {
        main_fcall = amxc_container_of(fcall->it.llist, amxb_pcb_deferred_t, calls);
    }

    amxb_pcb_log("Done fcall (ID = %" PRIu64 ") - Request = %p (%s:%d) ", fcall->call_id, (void*) fcall->req, __FILE__, __LINE__);
    amxc_llist_it_take(&fcall->it);

    amxb_pcb_handle_subscription(fcall->req, NULL, fcall->amxb_pcb, main_fcall == NULL? fcall:main_fcall);
    if((main_fcall != NULL) && amxc_llist_is_empty(&main_fcall->calls)) {
        amxb_pcb_log("Free fcall %p, request = %p (%s:%d)", (void*) main_fcall, (void*) main_fcall->req, __FILE__, __LINE__);
        amxb_pcb_fcall_delete(&main_fcall);
    }
    amxb_pcb_log("Free fcall %p, request = %p (%s:%d)", (void*) fcall, (void*) fcall->req, __FILE__, __LINE__);
    amxb_pcb_fcall_delete(&fcall);
}

static void amxb_pcb_amx_subscribe_get_done(const amxc_var_t* const data,
                                            void* const priv) {
    amxb_pcb_deferred_t* fcall = (amxb_pcb_deferred_t*) priv;
    amxb_pcb_deferred_t* main_fcall = fcall;
    amxd_status_t status = (amxd_status_t) GET_UINT32(data, "status");
    amxc_var_t* ret = GET_ARG(data, "retval");
    amxd_object_t* object = NULL;

    when_null(fcall, exit);
    when_null(fcall->amxb_pcb, exit);
    object = amxb_pcb_find_object(fcall->amxb_pcb, fcall->req, NULL);

    if(fcall->it.llist != NULL) {
        main_fcall = amxc_container_of(fcall->it.llist, amxb_pcb_deferred_t, calls);
    }

    amxb_pcb_log("Done fcall (ID = %" PRIu64 ")  Status = %d - %p (%s:%d) ", fcall->call_id, status, (void*) fcall->req, __FILE__, __LINE__);

    if(status == amxd_status_ok) {
        amxc_var_t args;
        amxc_var_t rv;
        amxb_pcb_deferred_t* sub_fcall = NULL;

        amxc_var_init(&rv);
        amxc_var_init(&args);
        amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
        amxb_pcb_reply_objects(fcall, object, ret);
        // Do not close the request if sub-calls are still in flight
        // as amxb_pcb_reply_objects needs to fetch extra information about the objects
        // to be able to create a valid pcb reply
        amxb_pcb_fcall_new(&sub_fcall, fcall->amxb_pcb, fcall->req, &fcall->calls);
        when_null(sub_fcall, exit);
        amxb_pcb_log("New fcall allocated %p, request = %p (%s:%d)", (void*) sub_fcall, (void*) fcall->req, __FILE__, __LINE__);

        amxb_pcb_log("New fcall for request %p (%s:%d)", (void*) sub_fcall->req, __FILE__, __LINE__);
        status = amxb_pcb_amx_call(sub_fcall, object, "_subscribe", &args, &rv, amxb_pcb_amx_subscribe_done);
        if(status != amxd_status_deferred) {
            amxb_pcb_log("Call is done, status = %d, request = %p (%s:%d)", status, (void*) sub_fcall->req, __FILE__, __LINE__);
            amxb_pcb_handle_subscription(fcall->req, NULL, fcall->amxb_pcb, main_fcall);
            amxb_pcb_log("Free fcall %p, request = %p (%s:%d)", (void*) sub_fcall, (void*) sub_fcall->req, __FILE__, __LINE__);
            amxb_pcb_fcall_delete(&sub_fcall);
            if(amxc_llist_is_empty(&fcall->calls)) {
                amxb_pcb_log("Free fcall %p, request = %p (%s:%d)", (void*) fcall, (void*) fcall->req, __FILE__, __LINE__);
                amxb_pcb_fcall_delete(&fcall);
            }
        } else {
            amxb_pcb_log("Call is deferred (ID  = %" PRIu64 "), request = %p (%s:%d)", sub_fcall->call_id, (void*) sub_fcall->req, __FILE__, __LINE__);
        }
        amxc_var_clean(&args);
        amxc_var_clean(&rv);
    } else {
        amxb_amxd_status_to_pcb_error(status, fcall->amxb_pcb->peer, fcall->req, NULL);
        amxb_pcb_serialize_reply_end(fcall->amxb_pcb->peer, fcall->req);
        amxb_pcb_set_request_done(fcall->amxb_pcb->peer, fcall->req);
        request_setData(fcall->req, NULL);
        request_setDestroyHandler(fcall->req, NULL);
        amxb_pcb_log("Destroying request: %p (%s:%d)", (void*) fcall->req, __FILE__, __LINE__);
        request_destroy(fcall->req);
        amxb_pcb_log("Free fcall %p, request = %p (%s:%d)", (void*) fcall, (void*) fcall->req, __FILE__, __LINE__);
        amxb_pcb_fcall_delete(&fcall);
    }

exit:
    return;
}

bool amxb_pcb_get_object_v2(peer_info_t* peer,
                            UNUSED datamodel_t* datamodel,
                            request_t* req) {
    bool retval = false;
    amxb_pcb_t* amxb_pcb = NULL;
    amxb_bus_ctx_t* amxb_bus_ctx = NULL;
    amxc_var_t args;
    amxc_var_init(&args);

    amxb_bus_ctx = amxb_pcb_find_peer(peer);
    when_null(amxb_bus_ctx, exit);
    amxb_pcb = (amxb_pcb_t*) amxb_bus_ctx->bus_ctx;
    when_null(amxb_pcb, exit);
    amxb_pcb->stats.counter_rx_get++;

    amxb_pcb_log("<== (%s:%d)", __FILE__, __LINE__);
    amxb_pcb_log_pcb_request("Received request", req);

    if((request_attributes(req) & request_notify_all) != 0) {
        amxb_pcb_log("Subscription request (%p)", (void*) req);
        if((request_attributes(req) & (request_notify_only | AMXB_PCB_AMX_EVENTING)) == 0) {
            amxb_pcb_build_get_args(&args, req);
            amxb_pcb_log("Call _get (%p) (%s:%d)", (void*) req, __FILE__, __LINE__);
            amxb_pcb_log_variant("Arguments = ", &args);
            retval = amxb_pcb_handler_common(peer, req, &args, "_get", amxb_pcb_amx_subscribe_get_done);
        } else {
            amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
            amxb_pcb_log("Call _subscribe (%p) (%s:%d)", (void*) req, __FILE__, __LINE__);
            retval = amxb_pcb_handler_common(peer, req, &args, "_subscribe", amxb_pcb_amx_subscribe_done);
        }
    } else {
        amxb_pcb_build_get_args(&args, req);
        amxb_pcb_log("Call _get (%p) (%s:%d)", (void*) req, __FILE__, __LINE__);
        amxb_pcb_log_variant("Arguments = ", &args);
        retval = amxb_pcb_handler_common(peer, req, &args, "_get", NULL);
        when_false(retval, exit);
    }

exit:
    amxc_var_clean(&args);
    return retval;
}

bool amxb_pcb_set_object_v2(peer_info_t* peer,
                            UNUSED datamodel_t* datamodel,
                            request_t* req) {
    bool retval = false;
    amxb_pcb_t* amxb_pcb = NULL;
    amxb_bus_ctx_t* amxb_bus_ctx = NULL;
    amxc_var_t args;
    amxc_var_init(&args);

    amxb_bus_ctx = amxb_pcb_find_peer(peer);
    when_null(amxb_bus_ctx, exit);
    amxb_pcb = (amxb_pcb_t*) amxb_bus_ctx->bus_ctx;
    when_null(amxb_pcb, exit);
    amxb_pcb->stats.counter_rx_set++;

    amxb_pcb_log("<== (%s:%d)", __FILE__, __LINE__);
    amxb_pcb_log_pcb_request("Received request", req);

    amxb_pcb_build_set_args(&args, req);
    request_setAttributes(req, request_attributes(req) |
                          request_getObject_parameters |
                          request_getObject_instances);
    amxb_pcb_log("Call _set (%p) (%s:%d)", (void*) req, __FILE__, __LINE__);
    amxb_pcb_log_variant("Arguments = ", &args);
    retval = amxb_pcb_handler_common(peer, req, &args, "_set", NULL);

exit:
    amxc_var_clean(&args);
    return retval;
}

bool amxb_pcb_add_instance_v2(peer_info_t* peer,
                              UNUSED datamodel_t* datamodel,
                              request_t* req) {
    bool retval = false;
    amxb_pcb_t* amxb_pcb = NULL;
    amxb_bus_ctx_t* amxb_bus_ctx = NULL;
    amxc_var_t args;
    amxc_var_init(&args);

    amxb_bus_ctx = amxb_pcb_find_peer(peer);
    when_null(amxb_bus_ctx, exit);
    amxb_pcb = (amxb_pcb_t*) amxb_bus_ctx->bus_ctx;
    when_null(amxb_pcb, exit);
    amxb_pcb->stats.counter_rx_add++;

    amxb_pcb_log("<== (%s:%d)", __FILE__, __LINE__);
    amxb_pcb_log_pcb_request("Received request", req);

    amxb_pcb_build_add_instance_args(&args, req);
    request_setAttributes(req, request_attributes(req) |
                          request_getObject_parameters |
                          request_getObject_instances);
    amxb_pcb_log("Call _add (%p) (%s:%d)", (void*) req, __FILE__, __LINE__);
    amxb_pcb_log_variant("Arguments = ", &args);
    retval = amxb_pcb_handler_common(peer, req, &args, "_add", NULL);

exit:
    amxc_var_clean(&args);
    return retval;
}

bool amxb_pcb_del_instance_v2(peer_info_t* peer,
                              UNUSED datamodel_t* datamodel,
                              request_t* req) {
    bool retval = false;
    amxb_pcb_t* amxb_pcb = NULL;
    amxb_bus_ctx_t* amxb_bus_ctx = NULL;
    amxc_var_t args;
    amxc_var_init(&args);

    amxb_bus_ctx = amxb_pcb_find_peer(peer);
    when_null(amxb_bus_ctx, exit);
    amxb_pcb = (amxb_pcb_t*) amxb_bus_ctx->bus_ctx;
    when_null(amxb_pcb, exit);
    amxb_pcb->stats.counter_rx_del++;

    amxb_pcb_log("<== (%s:%d)", __FILE__, __LINE__);
    amxb_pcb_log_pcb_request("Received request", req);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxb_pcb_log("Call _del (%p) (%s:%d)", (void*) req, __FILE__, __LINE__);
    amxb_pcb_log_variant("Arguments = ", &args);
    retval = amxb_pcb_handler_common(peer, req, &args, "_del", NULL);

exit:
    amxc_var_clean(&args);
    return retval;
}

bool amxb_pcb_execute_v2(peer_info_t* peer,
                         UNUSED datamodel_t* datamodel,
                         request_t* req) {
    bool retval = false;
    llist_iterator_t* it = request_firstParameter(req);
    amxb_pcb_t* amxb_pcb = NULL;
    amxb_bus_ctx_t* amxb_bus_ctx = NULL;
    amxc_var_t args;
    amxc_var_init(&args);

    amxb_bus_ctx = amxb_pcb_find_peer(peer);
    when_null(amxb_bus_ctx, exit);
    amxb_pcb = (amxb_pcb_t*) amxb_bus_ctx->bus_ctx;
    when_null(amxb_pcb, exit);
    amxb_pcb->stats.counter_rx_invoke++;

    amxb_pcb_log("<== (%s:%d)", __FILE__, __LINE__);
    amxb_pcb_log_pcb_request("Received request", req);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    if(((request_attributes(req) & request_function_args_by_name) == 0) &&
       (it != NULL)) {
        // troubles in paradise.
        // function arguments are passed in order, need to find the
        // argument names before translation can be done
        retval = amxb_pcb_fetch_function_def(peer, req);
    } else {
        amxb_pcb_build_exec_args(&args, req, NULL);
        amxb_pcb_log("Call _exec (%p) (%s:%d)", (void*) req, __FILE__, __LINE__);
        amxb_pcb_log_variant("Arguments = ", &args);
        retval = amxb_pcb_handler_common(peer, req, &args, "_exec", NULL);
    }

exit:
    amxc_var_clean(&args);
    return retval;
}