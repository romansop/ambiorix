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

#include <imtp/imtp_connection.h>

#include <usp/uspl.h>
#include <uspi/uspi_prefix.h>

#include "amxb_usp.h"
#include "amxb_usp_la.h"

void amxb_usp_send_notification(const char* const sig_name,
                                const amxc_var_t* const data,
                                void* const priv) {
    amxc_htable_it_t* hit = (amxc_htable_it_t*) priv;
    usp_subscription_t* sub = amxc_htable_it_get_data(hit, usp_subscription_t, hit);
    amxb_usp_t* ctx = sub->ctx;
    amxc_var_t notify_var;
    amxc_var_t* event_var = NULL;
    char* from_id = amxb_usp_get_from_id();
    char* to_id = amxb_usp_get_to_id(ctx);
    uspl_tx_t* usp_tx = NULL;
    const amxc_htable_t* data_table = amxc_var_constcast(amxc_htable_t, data);

    amxc_var_init(&notify_var);
    amxc_var_set_type(&notify_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &notify_var, "subscription_id", "Unused in backed"); // ??
    amxc_var_add_key(bool, &notify_var, "send_resp", false);
    amxc_var_add_key(uint32_t, &notify_var, "notification_case", USP__NOTIFY__NOTIFICATION_EVENT);
    event_var = amxc_var_add_key(amxc_htable_t, &notify_var, "event", data_table);
    amxc_var_add_key(cstring_t, event_var, "event_name", sig_name);

    uspl_tx_new(&usp_tx, from_id, to_id);
    uspl_notify_new(usp_tx, &notify_var);

    amxb_usp_build_and_send_tlv(ctx, usp_tx);

    uspl_tx_delete(&usp_tx);

    free(to_id);
    free(from_id);
    amxc_var_clean(&notify_var);
}

static void amxb_usp_handle_subscription(amxb_usp_t* ctx,
                                         const char* tlv_value) {
    amxc_string_t expression;
    usp_subscription_t* sub = (usp_subscription_t*) calloc(1, sizeof(usp_subscription_t));

    when_null(sub, exit);
    sub->ctx = ctx;

    amxc_string_init(&expression, 0);
    amxc_string_setf(&expression, "path starts with \"%s\"", tlv_value);

    amxc_htable_insert(&ctx->server_subs, tlv_value, &sub->hit);

    // Pass expression to function?
    amxp_slot_connect_filtered(&ctx->dm->sigmngr,
                               ".*",
                               amxc_string_get(&expression, 0),
                               amxb_usp_send_notification,
                               &sub->hit);
    amxc_string_clean(&expression);

exit:
    return;
}

static void amxb_usp_handle_unsubscription(amxb_usp_t* ctx,
                                           const char* tlv_value) {
    amxc_htable_it_t* hit = amxc_htable_take(&ctx->server_subs, tlv_value);
    usp_subscription_t* sub = NULL;

    when_null(hit, exit);

    sub = amxc_htable_it_get_data(hit, usp_subscription_t, hit);

    amxp_slot_disconnect_with_priv(&ctx->dm->sigmngr,
                                   amxb_usp_send_notification,
                                   &sub->hit);
    amxc_htable_it_clean(hit, NULL);
    free(sub);

exit:
    return;
}

static void amxb_usp_set_eid(amxb_usp_t* ctx, const imtp_tlv_t* tlv_handshake) {
    when_not_null(ctx->eid, exit);

    ctx->eid = (char*) calloc(1, tlv_handshake->length + 1);
    when_null(ctx->eid, exit);
    strncpy(ctx->eid, (char*) tlv_handshake->value + tlv_handshake->offset, tlv_handshake->length);
exit:
    return;
}

static amxd_status_t amxb_usp_handle_handshake(amxb_usp_t* ctx, const imtp_tlv_t* tlv_handshake) {
    amxc_string_t contr_path;
    amxd_status_t status = amxd_status_ok;

    amxc_string_init(&contr_path, 0);
    amxb_usp_set_eid(ctx, tlv_handshake);
    status = amxb_usp_la_contr_add(ctx, &contr_path);
    when_failed(status, exit);

    uspi_prefix_strip_device(&contr_path);
    amxc_string_trimr(&contr_path, amxb_usp_is_dot);
    amxb_usp_ctx_insert(ctx, amxc_string_get(&contr_path, 0));
    ctx->contr_path = amxc_string_take_buffer(&contr_path);

exit:
    amxc_string_clean(&contr_path);
    return status;
}

int amxb_usp_handle_read(amxb_usp_t* ctx) {
    int retval = -1;
    imtp_frame_t* frame = NULL;
    const imtp_tlv_t* tlv_pbuf = NULL;
    const imtp_tlv_t* tlv_sub = NULL;
    const imtp_tlv_t* tlv_unsub = NULL;
    const imtp_tlv_t* tlv_handshake = NULL;
    const imtp_tlv_t* tlv_error = NULL;

    when_null(ctx, exit);
    when_null(ctx->icon, exit);

    retval = imtp_connection_read_frame(ctx->icon, &frame);
    when_failed(retval, exit);

    tlv_pbuf = imtp_frame_get_first_tlv(frame, imtp_tlv_type_protobuf_bytes);
    tlv_sub = imtp_frame_get_first_tlv(frame, imtp_tlv_type_subscribe);
    tlv_unsub = imtp_frame_get_first_tlv(frame, imtp_tlv_type_unsubscribe);
    tlv_handshake = imtp_frame_get_first_tlv(frame, imtp_tlv_type_handshake);
    tlv_error = imtp_frame_get_first_tlv(frame, imtp_tlv_type_error);

    retval = 0;
    if(tlv_pbuf != NULL) {
        uspl_rx_t* usp_rx = uspl_msghandler_unpack_protobuf((unsigned char*) tlv_pbuf->value + tlv_pbuf->offset,
                                                            tlv_pbuf->length);
        // Don't check return value. Even if handling message fails, the read function should not
        // return a non-zero value, because this will close the connection
        amxb_usp_handle_protobuf(ctx, usp_rx);
        uspl_rx_delete(&usp_rx);
    } else if(tlv_sub != NULL) {
        amxb_usp_handle_subscription(ctx,
                                     (char*) tlv_sub->value + tlv_sub->offset);
    } else if(tlv_unsub != NULL) {
        amxb_usp_handle_unsubscription(ctx, (char*) tlv_unsub->value + tlv_unsub->offset);
    } else if(tlv_handshake != NULL) {
        retval = amxb_usp_handle_handshake(ctx, tlv_handshake) == 0 ? 0 : -1;
    } else if(tlv_error != NULL) {
        printf("Received error message: %s\n", (char*) tlv_error->value + tlv_error->offset);
        retval = 0;
    } else {
        retval = -1;
    }

exit:
    imtp_frame_delete(&frame);
    return retval;
}
