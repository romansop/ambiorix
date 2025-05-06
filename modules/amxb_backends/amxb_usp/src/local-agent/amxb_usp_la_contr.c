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

#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxd/amxd_transaction.h>

#include "amxb_usp.h"
#include "amxb_usp_la.h"

static int amxb_usp_la_contr_del_subscriptions(UNUSED amxd_object_t* object,
                                               amxd_object_t* mobject,
                                               void* priv) {
    amxd_trans_t* trans = (amxd_trans_t*) priv;
    amxd_trans_del_inst(trans, amxd_object_get_index(mobject), NULL);
    return 0;
}

amxd_status_t amxb_usp_la_contr_add(amxb_usp_t* ctx, amxc_string_t* ctrl_path) {
    amxd_object_t* object = amxd_dm_findf(ctx->la_dm, "Device.LocalAgent.Controller.[EndpointID == '%s'].", ctx->eid);
    amxd_trans_t trans;
    amxd_status_t status = amxd_status_ok;
    const char* path = NULL;

    amxd_trans_init(&trans);

    // Do nothing on duplicate handshake
    when_not_null_status(object, exit, status = amxd_status_ok);

    object = amxd_dm_findf(ctx->la_dm, "Device.LocalAgent.Controller.");
    // Do nothing when LA DM is not supported
    when_null(object, exit);

    amxd_trans_set_attr(&trans, amxd_tattr_change_priv, true);
    amxd_trans_select_object(&trans, object);
    amxd_trans_add_inst(&trans, 0, NULL);
    amxd_trans_set_value(cstring_t, &trans, "EndpointID", ctx->eid);
    status = amxd_trans_apply(&trans, ctx->la_dm);
    when_failed(status, exit);

    path = GETP_CHAR(&trans.retvals, "1.path");
    if(path != NULL) {
        amxc_string_setf(ctrl_path, "%s", path);
    }

exit:
    amxd_trans_clean(&trans);
    return status;
}

amxd_status_t amxb_usp_la_contr_del(amxb_usp_t* ctx) {
    amxd_object_t* instance = amxd_dm_findf(ctx->la_dm, "Device.LocalAgent.Controller.[EndpointID == '%s'].", ctx->eid);
    uint32_t index = 0;
    amxd_object_t* parent = NULL;
    amxd_trans_t trans;
    amxd_status_t retval = amxd_status_ok;
    amxd_object_t* subscriptions = amxd_dm_findf(ctx->la_dm, "Device.LocalAgent.Subscription.");
    amxc_string_t filter;
    char* path = NULL;

    amxd_trans_init(&trans);
    amxc_string_init(&filter, 0);

    // Do nothing if not found
    when_null(instance, exit);

    index = amxd_object_get_index(instance);
    parent = amxd_object_get_parent(instance);
    path = amxd_object_get_path(instance, AMXD_OBJECT_INDEXED);
    amxc_string_setf(&filter, "[Recipient == '%s']", path);
    // remove controller
    amxd_trans_set_attr(&trans, amxd_tattr_change_priv, true);
    amxd_trans_select_object(&trans, parent);
    amxd_trans_del_inst(&trans, index, NULL);
    // remove all subscriptions for this controller
    amxd_trans_select_object(&trans, subscriptions);
    amxd_object_for_all(subscriptions, amxc_string_get(&filter, 0), amxb_usp_la_contr_del_subscriptions, &trans);
    retval = amxd_trans_apply(&trans, ctx->la_dm);

exit:
    free(path);
    amxc_string_clean(&filter);
    amxd_trans_clean(&trans);
    return retval;
}
