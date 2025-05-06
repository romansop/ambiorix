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

#include <string.h>
#include <stdlib.h>

#include "amxb_usp.h"

static int amxb_usp_subscribe_send(amxb_usp_t* ctx, const char* object, imtp_tlv_type_t type) {
    int retval = -1;
    imtp_frame_t* frame = NULL;
    imtp_tlv_t* tlv_sub = NULL;
    amxc_string_t* dotted_object = amxb_usp_add_dot(object);
    uint32_t len = amxc_string_text_length(dotted_object);

    retval = imtp_frame_new(&frame);
    when_failed(retval, exit);

    retval = imtp_tlv_new(&tlv_sub,
                          type,
                          len + 1,
                          amxc_string_take_buffer(dotted_object),
                          0,
                          IMTP_TLV_TAKE);
    when_failed(retval, exit);

    retval = imtp_frame_tlv_add(frame, tlv_sub);
    when_failed(retval, exit);

    retval = imtp_connection_write_frame(ctx->icon, frame);
    when_failed(retval, exit);

exit:
    amxc_string_delete(&dotted_object);
    imtp_frame_delete(&frame);
    return retval;
}

static int amxb_usp_subscription_add(void* const ctx, const char* path, uint32_t event_type, amxc_var_t* sub) {
    int retval = 0;
    amxc_var_t subs_params;
    amxc_var_t ret;
    amxc_var_t* notif_type = NULL;

    when_true(event_type == 0, exit);

    amxc_var_init(&ret);
    amxc_var_init(&subs_params);
    amxc_var_set_type(&subs_params, AMXC_VAR_ID_HTABLE);

    amxc_var_add_key(cstring_t, &subs_params, "ReferenceList", path);
    amxc_var_add_key(bool, &subs_params, "Enable", true);
    notif_type = amxc_var_add_key(cstring_t, &subs_params, "NotifType", "");
    switch(event_type) {
    case AMXB_BE_EVENT_TYPE_CHANGE:
        amxc_var_set(cstring_t, notif_type, "ValueChange");
        break;
    case AMXB_BE_EVENT_TYPE_ADD:
        amxc_var_set(cstring_t, notif_type, "ObjectCreation");
        break;
    case AMXB_BE_EVENT_TYPE_DEL:
        amxc_var_set(cstring_t, notif_type, "ObjectDeletion");
        break;
    case AMXB_BE_EVENT_TYPE_EVENT:
        amxc_var_set(cstring_t, notif_type, "Event");
        break;
    case AMXB_BE_EVENT_TYPE_COMPL:
        amxc_var_set(cstring_t, notif_type, "OperationComplete");
        break;
    }
    retval = amxb_usp_add(ctx, "Device.LocalAgent.Subscription.", NULL, 0, NULL, &subs_params, AMXB_PUBLIC, &ret, 10);
    if(retval == amxd_status_ok) {
        amxc_var_add(uint32_t, sub, GETP_UINT32(&ret, "0.index"));
    }

    amxc_var_clean(&subs_params);
    amxc_var_clean(&ret);

exit:
    return retval;
}

int amxb_usp_subscribe(void* const ctx,
                       const char* object) {
    amxb_usp_t* amxb_usp = (amxb_usp_t*) ctx;
    amxc_htable_it_t* it = NULL;
    amxc_var_t* amxb_usp_sub = NULL;
    int retval = 0;

    when_str_empty(object, exit);

    it = amxc_htable_get(&amxb_usp->subscriptions, object);
    when_not_null(it, exit);

    amxc_var_new(&amxb_usp_sub);
    amxc_htable_insert(&amxb_usp->subscriptions,
                       object,
                       &amxb_usp_sub->hit);

    retval = amxb_usp_subscribe_send(amxb_usp, object, imtp_tlv_type_subscribe);
    when_failed(retval, exit);

exit:
    return retval;
}

int amxb_usp_subscribe_v2(void* const ctx,
                          amxd_path_t* path,
                          UNUSED int32_t depth,
                          uint32_t event_types) {
    amxb_usp_t* amxb_usp = (amxb_usp_t*) ctx;
    amxc_htable_it_t* it = NULL;
    amxc_var_t* amxb_usp_sub = NULL;
    amxc_string_t full_path;
    bool legacy_subscribe = amxc_var_constcast(bool, amxb_usp_get_config_option("legacy-subscribe"));
    char* object = amxd_path_get_fixed_part(path, false);
    const char* param = amxd_path_get_param(path);
    int retval = 0;

    amxc_string_init(&full_path, 0);

    if(legacy_subscribe) {
        // Pass on path without terminating dot
        retval = amxb_usp_subscribe(ctx, amxd_path_get(path, 0));
        goto exit;
    }

    when_str_empty(object, exit);
    if(*object != 0) {
        object[strlen(object) - 1] = 0;
    }

    it = amxc_htable_get(&amxb_usp->subscriptions, object);
    when_not_null(it, exit);

    amxc_var_new(&amxb_usp_sub);
    amxc_var_set_type(amxb_usp_sub, AMXC_VAR_ID_LIST);
    amxc_htable_insert(&amxb_usp->subscriptions,
                       object,
                       &amxb_usp_sub->hit);

    amxc_string_setf(&full_path, "%s%s", amxd_path_get(path, AMXD_OBJECT_TERMINATE), param == NULL ? "" : param);
    amxb_usp_subscription_add(ctx, amxc_string_get(&full_path, 0), event_types & AMXB_BE_EVENT_TYPE_CHANGE, amxb_usp_sub);
    amxb_usp_subscription_add(ctx, amxc_string_get(&full_path, 0), event_types & AMXB_BE_EVENT_TYPE_ADD, amxb_usp_sub);
    amxb_usp_subscription_add(ctx, amxc_string_get(&full_path, 0), event_types & AMXB_BE_EVENT_TYPE_DEL, amxb_usp_sub);
    amxb_usp_subscription_add(ctx, amxc_string_get(&full_path, 0), event_types & AMXB_BE_EVENT_TYPE_EVENT, amxb_usp_sub);
    amxb_usp_subscription_add(ctx, amxc_string_get(&full_path, 0), event_types & AMXB_BE_EVENT_TYPE_COMPL, amxb_usp_sub);

exit:
    free(object);
    amxc_string_clean(&full_path);
    return retval;
}

int amxb_usp_unsubscribe(void* const ctx,
                         const char* object) {

    amxb_usp_t* amxb_usp = (amxb_usp_t*) ctx;
    amxc_var_t* sub = NULL;
    amxc_htable_it_t* it = NULL;
    int retval = -1;

    it = amxc_htable_get(&amxb_usp->subscriptions, object);
    when_null(it, exit);
    sub = amxc_var_from_htable_it(it);

    if(amxc_var_type_of(sub) == AMXC_VAR_ID_LIST) {
        amxc_string_t sub_path;
        amxc_var_t ret;
        amxc_var_init(&ret);
        amxc_string_init(&sub_path, 0);
        amxc_var_for_each(index, sub) {
            amxc_string_setf(&sub_path, "Device.LocalAgent.Subscription.%d.", GET_UINT32(index, 0));
            amxb_usp_delete(ctx, amxc_string_get(&sub_path, 0), NULL, 0, NULL, AMXB_PUBLIC, &ret, 10);
        }
        amxc_string_clean(&sub_path);
        amxc_var_clean(&ret);

        amxc_htable_it_clean(it, NULL);
        amxc_var_delete(&sub);
        retval = 0;
    } else {
        amxc_htable_it_clean(it, NULL);
        amxc_var_delete(&sub);

        retval = amxb_usp_subscribe_send(amxb_usp, object, imtp_tlv_type_unsubscribe);
    }

exit:
    return retval;
}
