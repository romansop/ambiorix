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

typedef struct _rbus_subscriber {
    char* event_name;
    uint32_t ref_count;
    amxc_htable_it_t it;
} rbus_subscriber_t;

static void amxb_rbus_send_notification(const char* const sig_name,
                                        const amxc_var_t* const data,
                                        void* const priv) {
    rbus_subscriber_t* subscriber = (rbus_subscriber_t*) priv;
    amxc_htable_t* ptable = amxc_container_of(subscriber->it.ait->array, amxc_htable_t, table);
    amxb_rbus_t* amxb_rbus_ctx = amxc_container_of(ptable, amxb_rbus_t, subscribers);
    rbusEvent_t event;
    rbusObject_t rbus_data = NULL;
    rbusValue_t value;

    amxb_rbus_translate_data(data);

    rbusObject_Init(&rbus_data, NULL);
    amxb_rbus_htvar_to_robject(data, rbus_data);

    rbusValue_Init(&value);
    rbusValue_SetString(value, sig_name);
    rbusObject_SetValue(rbus_data, "notification", value);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    rbusValue_SetString(value, amxc_htable_it_get_key(&subscriber->it));
    rbusObject_SetValue(rbus_data, "notification_path", value);
    rbusValue_Release(value);

    event.name = subscriber->event_name;
    event.data = rbus_data;
    event.type = RBUS_EVENT_GENERAL;

    rbusEvent_Publish(amxb_rbus_ctx->handle, &event);
    rbusObject_Release(rbus_data);

    return;
}

static void amxb_rbus_subscribe_impl(amxb_rbus_t* amxb_rbus_ctx,
                                     amxb_rbus_item_t* item) {
    amxd_path_t path;
    amxd_object_t* object = NULL;
    char* obj_path = NULL;
    char* sub_key = NULL;
    amxc_string_t expression;
    amxc_htable_it_t* it = NULL;
    rbus_subscriber_t* subscriber = NULL;
    const char* requested = NULL;
    const char* translated = NULL;

    amxc_string_init(&expression, 0);
    amxd_path_init(&path, amxc_string_get(&item->name, 0));
    sub_key = strdup(amxd_path_get(&path, 0));

    amxb_rbus_translate_path(&path, &requested, &translated);
    object = amxd_dm_findf(amxb_rbus_ctx->dm, "%s", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
    when_null(object, exit);

    obj_path = amxd_object_get_path(object, AMXD_OBJECT_INDEXED);
    it = amxc_htable_get(&amxb_rbus_ctx->subscribers, sub_key);

    if(it != NULL) {
        subscriber = amxc_container_of(it, rbus_subscriber_t, it);
        subscriber->ref_count++;
        goto exit;
    }

    subscriber = (rbus_subscriber_t*) calloc(1, sizeof(rbus_subscriber_t));
    subscriber->event_name = amxc_string_take_buffer(&item->name);
    subscriber->ref_count = 1;
    if(amxd_path_get_param(&path) == NULL) {
        subscriber->event_name[strlen(subscriber->event_name) - 1] = 0;
    }

    amxc_string_appendf(&expression, "path starts with \"%s.\"", obj_path);
    amxc_htable_insert(&amxb_rbus_ctx->subscribers, sub_key, &subscriber->it);

    amxp_slot_connect_filtered(&amxb_rbus_ctx->dm->sigmngr,
                               ".*",
                               amxc_string_get(&expression, 0),
                               amxb_rbus_send_notification,
                               subscriber);


exit:
    free(obj_path);
    free(sub_key);
    amxd_path_clean(&path);
    amxc_string_clean(&expression);
}

static void amxb_rbus_unsubscribe_impl(amxb_rbus_t* amxb_rbus_ctx,
                                       amxb_rbus_item_t* item) {
    amxd_path_t path;
    amxc_htable_it_t* it = NULL;
    rbus_subscriber_t* subscriber = NULL;
    const char* requested = NULL;
    const char* translated = NULL;

    amxd_path_init(&path, amxc_string_get(&item->name, 0));
    amxb_rbus_translate_path(&path, &requested, &translated);

    it = amxc_htable_get(&amxb_rbus_ctx->subscribers, amxd_path_get(&path, 0));
    if(it != NULL) {
        subscriber = amxc_container_of(it, rbus_subscriber_t, it);
        subscriber->ref_count--;
        if(subscriber->ref_count == 0) {
            amxp_slot_disconnect_with_priv(&amxb_rbus_ctx->dm->sigmngr,
                                           amxb_rbus_send_notification,
                                           subscriber);

            amxc_htable_it_take(it);
            amxc_htable_it_clean(it, NULL);
            free(subscriber->event_name);
            free(subscriber);
        }
    }

    amxd_path_clean(&path);
}

rbusError_t amxb_rbus_row_subscribe_handler(rbusHandle_t handle,
                                            rbusEventSubAction_t action,
                                            char const* event_name,
                                            rbusFilter_t filter,
                                            int32_t interval,
                                            bool* auto_publish) {
    amxc_string_t amx_event_name;
    rbusError_t status = RBUS_ERROR_SUCCESS;
    amxc_string_init(&amx_event_name, 0);
    amxc_string_setf(&amx_event_name, "%s.", event_name);

    status = amxb_rbus_subscribe_handler(handle,
                                         action,
                                         amxc_string_get(&amx_event_name, 0),
                                         filter,
                                         interval,
                                         auto_publish);

    amxc_string_clean(&amx_event_name);

    return status;
}

rbusError_t amxb_rbus_subscribe_handler(rbusHandle_t handle,
                                        rbusEventSubAction_t action,
                                        char const* event_name,
                                        UNUSED rbusFilter_t filter,
                                        UNUSED int32_t interval,
                                        bool* auto_publish) {
    rbusError_t status = RBUS_ERROR_INVALID_CONTEXT;
    amxb_rbus_t* amxb_rbus_ctx = amxb_rbus_get_ctx(handle);
    amxb_rbus_item_t* rbus_item = NULL;

    *auto_publish = true;
    when_null(amxb_rbus_ctx, exit);

    rbus_item = (amxb_rbus_item_t*) calloc(1, sizeof(amxb_rbus_item_t));
    when_null(rbus_item, exit);

    amxc_string_init(&rbus_item->name, 0);
    amxc_string_set(&rbus_item->name, event_name);
    if(action == RBUS_EVENT_ACTION_SUBSCRIBE) {
        rbus_item->handler = amxb_rbus_subscribe_impl;
    } else {
        rbus_item->handler = amxb_rbus_unsubscribe_impl;

    }
    amxb_rbus_ctrl_write(amxb_rbus_ctx, rbus_item);

    status = RBUS_ERROR_SUCCESS;
exit:

    return status;
}

void amxb_rbus_remove_subs(amxb_rbus_t* amxb_rbus_ctx) {
    if(amxb_rbus_ctx->dm != NULL) {
        amxc_htable_for_each(it, &amxb_rbus_ctx->subscribers) {
            rbus_subscriber_t* subscriber = amxc_container_of(it, rbus_subscriber_t, it);
            amxp_slot_disconnect_with_priv(&amxb_rbus_ctx->dm->sigmngr,
                                           amxb_rbus_send_notification,
                                           subscriber);
            amxc_htable_it_take(it);
            amxc_htable_it_clean(it, NULL);
            free(subscriber->event_name);
            free(subscriber);
        }
    }
    amxc_htable_for_each(it, &amxb_rbus_ctx->subscriptions) {
        amxc_htable_it_take(it);
        amxc_htable_it_clean(it, NULL);
        free(it);
    }
}