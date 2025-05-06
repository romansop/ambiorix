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

#include "amxb_rbus.h"
#include "amxb_rbus_ctrl.h"
#include "amxb_rbus_handlers.h"

static void amxb_rbus_event_handler(UNUSED rbusHandle_t handle,
                                    rbusEvent_t const* event,
                                    rbusEventSubscription_t* subscription) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) subscription->userData;
    amxc_var_t notification;
    amxc_var_t* subs_path = NULL;

    amxc_var_init(&notification);
    amxb_rbus_object_to_var(&notification, event->data);
    subs_path = GET_ARG(&notification, "notification_path");

    if(!amxc_var_is_null(subs_path)) {
        amxc_var_take_it(subs_path);
        // This eventhandler can be called on any thread.
        // It is safe to emit signals from any thread context.
        // The event queue is protected with a mutex.
        amxp_sigmngr_emit_signal(amxb_rbus_ctx->sigmngr, GET_CHAR(subs_path, NULL), &notification);
    }

    amxc_var_delete(&subs_path);
    amxc_var_clean(&notification);

    return;
}

int amxb_rbus_subscribe(void* const ctx, const char* object) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    int ret = 0;
    amxc_string_t event_name;
    int status = RBUS_ERROR_SUCCESS;

    amxc_string_init(&event_name, 0);

    if(!amxc_htable_contains(&amxb_rbus_ctx->subscriptions, object)) {
        amxc_htable_it_t* subs_it = NULL;
        amxc_string_setf(&event_name, "%s.amx_notify!", object);
        status = rbusEvent_Subscribe(amxb_rbus_ctx->handle,
                                     amxc_string_get(&event_name, 0),
                                     amxb_rbus_event_handler,
                                     amxb_rbus_ctx,
                                     0);
        if(status != RBUS_ERROR_SUCCESS) {
            status = rbusEvent_Subscribe(amxb_rbus_ctx->handle,
                                         object,
                                         amxb_rbus_event_handler,
                                         amxb_rbus_ctx,
                                         0);
        }
        status = amxb_rbus_translate_rbus2status(status);
        if(status == amxd_status_ok) {
            subs_it = (amxc_htable_it_t*) calloc(1, sizeof(amxc_htable_it_t));
            amxc_htable_insert(&amxb_rbus_ctx->subscriptions, object, subs_it);
        }
    }

    amxc_string_clean(&event_name);
    return ret;
}

int amxb_rbus_unsubscribe(void* const ctx, const char* object) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    int ret = 0;
    amxc_string_t event_name;
    int status = RBUS_ERROR_SUCCESS;
    amxc_htable_it_t* subs_it = NULL;

    amxc_string_init(&event_name, 0);

    status = rbusEvent_Unsubscribe(amxb_rbus_ctx->handle, object);
    if(status != RBUS_ERROR_SUCCESS) {
        amxc_string_setf(&event_name, "%s.amx_notify!", object);
        status = rbusEvent_Unsubscribe(amxb_rbus_ctx->handle, amxc_string_get(&event_name, 0));
    }
    status = amxb_rbus_translate_rbus2status(status);
    if(status == amxd_status_ok) {
        subs_it = amxc_htable_get(&amxb_rbus_ctx->subscriptions, object);
        amxc_htable_it_take(subs_it);
        amxc_htable_it_clean(subs_it, NULL);
        free(subs_it);
    }

    amxc_string_clean(&event_name);
    return ret;
}
