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

static int cstr_path_get_depth(const char* cstr_path) {
    int depth = 0;
    amxd_path_t path;

    amxd_path_init(&path, NULL);
    amxd_path_setf(&path, true, "%s", cstr_path);
    depth = amxd_path_get_depth(&path);
    amxd_path_clean(&path);

    return depth;
}

/**
 *  Get the specified object, using the specified attributes
 *  and returns its path by index.
 */
static char* amxb_pcb_get_path_by_index_ex(amxb_pcb_t* pcb_ctx, const char* object, uint32_t attributes) {
    pcb_t* pcb = amxb_pcb_ctx();
    request_t* req = NULL;
    reply_t* reply = NULL;
    reply_item_t* item = NULL;
    char* pathi = NULL;

    req = request_create_getObject(object, 0, attributes | request_no_object_caching);
    if(amxb_pcb_fetch_object(pcb, pcb_ctx->peer, req, NULL, NULL, NULL) != 0) {
        goto error;
    }
    reply = request_reply(req);
    reply_for_each_item(item, reply) {
        object_t* obj = reply_item_object(item);
        if(obj != NULL) {
            pathi = object_pathChar(obj, path_attr_default);
            break;
        }
    }

error:
    amxb_pcb_request_destroy(req);
    return pathi;
}

/**
 *  Get the specified object, trying first using path key notation
 *  and then using path index notation.
 *
 *  Returns the object path by index.
 */
static char* amxb_pcb_get_path_by_index(amxb_pcb_t* pcb_ctx, const char* object) {
    char* pathi = NULL;

    pathi = amxb_pcb_get_path_by_index_ex(pcb_ctx, object, request_common_path_key_notation);
    if(pathi == NULL) {
        pathi = amxb_pcb_get_path_by_index_ex(pcb_ctx, object, request_common_default);
    }

    return pathi;
}

/**
 * The common implementation of amxb_pcb_subscribe() and amxb_pcb_subscribe_v2()
 *
 * If a subscription already exists on the same object, we increase by 1
 * its reference count on it and we resend the request if we need to
 * subscribe to new events or a larger depth.
 */
static int amxb_pcb_subscribe_impl(void* const ctx,
                                   const char* object,
                                   uint32_t depth,
                                   uint32_t attributes) {

    int retval = -1;
    pcb_t* pcb_ctx = amxb_pcb_ctx();
    amxb_pcb_t* amxb_pcb = (amxb_pcb_t*) ctx;
    peer_info_t* peer = amxb_pcb->peer;
    amxc_htable_it_t* it = NULL;
    request_t* sub_req = NULL;
    amxb_pcb_sub_t* amxb_pcb_sub = NULL;
    char* path = NULL;

    amxb_pcb_log("Subscribing object: (%s)", object);
    it = amxc_htable_get(&amxb_pcb->subscriptions, object);

    if(it != NULL) {
        // subscription already exists: do we need to increase the request depth
        // or add new request attributes ?
        amxb_pcb_sub = amxc_htable_it_get_data(it, amxb_pcb_sub_t, it);
        const char* req_path = request_path(amxb_pcb_sub->sub_req);
        uint32_t req_depth = request_depth(amxb_pcb_sub->sub_req);
        uint32_t req_attributes = request_attributes(amxb_pcb_sub->sub_req);

        amxb_pcb_log("Subscribe path: (%s)", req_path);

        if((req_depth < depth) || ((attributes & req_attributes) != attributes)) {
            depth = (depth >= req_depth) ? depth : req_depth;
            attributes |= req_attributes;
            sub_req = request_create_getObject(req_path, depth, attributes);
            amxb_pcb_log("==> (%s:%d)", __FILE__, __LINE__);
            amxb_pcb_log_pcb_request("Send request", sub_req);
            when_false(pcb_sendRequest(pcb_ctx, peer, sub_req), exit);
            request_setData(sub_req, amxb_pcb_sub);
            request_setReplyItemHandler(sub_req, amxb_pcb_notification);
            request_destroy(amxb_pcb_sub->sub_req);
            amxb_pcb_sub->sub_req = sub_req;
        }
        amxb_pcb_sub->reference++;
    } else {
        // create new subscription
        path = amxb_pcb_get_path_by_index(amxb_pcb, object);
        when_null(path, exit);
        amxb_pcb_log("Subscribe path: (%s)", path);
        sub_req = request_create_getObject(path, depth, attributes);
        amxb_pcb_log("==> (%s:%d)", __FILE__, __LINE__);
        amxb_pcb_log_pcb_request("Send request", sub_req);
        when_false(pcb_sendRequest(pcb_ctx, peer, sub_req), exit);

        amxb_pcb_sub = (amxb_pcb_sub_t*) calloc(1, sizeof(amxb_pcb_sub_t));
        amxc_htable_insert(&amxb_pcb->subscriptions,
                           object,
                           &amxb_pcb_sub->it);

        amxb_pcb_sub->amxb_pcb = (amxb_pcb_t*) ctx;
        amxb_pcb_sub->sub_req = sub_req;
        amxb_pcb_sub->reference = 1;

        request_setData(sub_req, amxb_pcb_sub);
        request_setReplyItemHandler(sub_req, amxb_pcb_notification);
    }
    amxb_pcb_log("Subscriber address (sub): (%p)", (void*) amxb_pcb_sub);

    retval = 0;

exit:
    free(path);
    if(retval != 0) {
        if(sub_req != NULL) {
            request_destroy(sub_req);
        }
    }

    return retval;
}

/**
 * Subscribe to the specified events on object, using the specified depth.
 */
int amxb_pcb_subscribe_v2(void* const ctx,
                          amxd_path_t* path,
                          int32_t depth,
                          uint32_t event_types) {

    int retval = -1;
    char* obj_path = NULL;
    int attributes = request_getObject_parameters |
        request_getObject_children |
        request_getObject_instances |
        /*request_notify_only |*/              // This provides problems with pcb-mapper
        AMXB_PCB_AMX_EVENTING |
        request_no_object_caching |
        request_notify_no_updates;

    // get the base object path
    obj_path = amxd_path_get_fixed_part(path, false);
    if(*obj_path != 0) {
        obj_path[strlen(obj_path) - 1] = 0;
    }

    // we need to increase the request depth for a search path
    if(depth >= 0) {
        depth += amxd_path_get_depth(path)
            - cstr_path_get_depth(obj_path);
    }

    if(event_types & AMXB_BE_EVENT_TYPE_CHANGE) {
        // be notified of parameter value even when an object instance is created
        attributes |= request_notify_values_changed | request_notify_object_added;
    }
    if(event_types & AMXB_BE_EVENT_TYPE_ADD) {
        attributes |= request_notify_object_added;
    }
    if(event_types & AMXB_BE_EVENT_TYPE_DEL) {
        attributes |= request_notify_object_deleted;
    }
    if(event_types & AMXB_BE_EVENT_TYPE_EVENT) {
        attributes |= request_notify_custom;
    }

    retval = amxb_pcb_subscribe_impl(ctx, obj_path, depth, attributes);
    free(obj_path);

    return retval;
}

/**
 * Unsubscribe from object subscription.
 *
 * When a backend implement subscribe_v2, unsubscribe() is expected to be called as many time than subcribe() was called.
 * => it is mandatory to use subscription reference counting.
 */
int amxb_pcb_unsubscribe(void* const ctx,
                         const char* object) {

    amxb_pcb_t* amxb_pcb = (amxb_pcb_t*) ctx;
    amxb_pcb_sub_t* amxb_pcb_sub = NULL;
    amxc_htable_it_t* it = NULL;

    int ret = -1;

    it = amxc_htable_get(&amxb_pcb->subscriptions, object);
    when_null(it, exit);
    amxb_pcb_sub = amxc_htable_it_get_data(it, amxb_pcb_sub_t, it);
    amxb_pcb_sub_drop(amxb_pcb_sub);
    ret = 0;

exit:
    return ret;
}

/**
 * Drop reference count of pcb subscribtion.
 *
 * The object is destroyed when reference count reaches 0.
 */
void amxb_pcb_sub_drop(amxb_pcb_sub_t* amxb_pcb_sub) {
    request_t* sub_req = NULL;

    amxb_pcb_sub->reference--;
    if(amxb_pcb_sub->reference == 0) {
        sub_req = amxb_pcb_sub->sub_req;
        amxb_pcb_sub->sub_req = NULL;
        request_setData(sub_req, NULL);
        request_setReplyItemHandler(sub_req, NULL);
        request_destroy(sub_req);
        peer_flush(amxb_pcb_sub->amxb_pcb->peer);

        amxc_htable_it_clean(&amxb_pcb_sub->it, NULL);
        free(amxb_pcb_sub);
    }
}
