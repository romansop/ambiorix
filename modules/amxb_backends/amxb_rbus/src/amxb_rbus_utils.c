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

#include <stdlib.h>
#include <string.h>

#include "amxb_rbus.h"
#include "amxb_rbus_ctrl.h"
#include "amxb_rbus_handlers.h"

void amxb_rbus_item_free(amxb_rbus_item_t* rbus_item) {
    if(rbus_item->tid != 0) {
        amxb_rbus_ctrl_wait(rbus_item);
    }

    amxc_llist_it_take(&rbus_item->it);

    if(rbus_item->in_params != NULL) {
        rbusObject_Release(rbus_item->in_params);
        rbus_item->in_params = NULL;
    }
    if(rbus_item->out_params != NULL) {
        rbusObject_Release(rbus_item->out_params);
        rbus_item->out_params = NULL;
    }

    amxc_string_clean(&rbus_item->name);
    amxc_string_clean(&rbus_item->transaction_id);
    free(rbus_item);
}

void amxb_rbus_clean_item(amxc_llist_it_t* it) {
    amxb_rbus_item_t* rbus_item = amxc_container_of(it, amxb_rbus_item_t, it);

    amxb_rbus_item_free(rbus_item);
}

bool amxb_rbus_remote_is_amx(amxb_rbus_t* rbus_ctx,
                             const char* object) {
    bool is_amx = false;
    uint32_t matches = 0;
    amxd_path_t path;
    rbusElementInfo_t* element = NULL;
    rbusElementInfo_t* elems = NULL;

    amxd_path_init(&path, object);

    rbusElementInfo_get(rbus_ctx->handle, amxd_path_get(&path, AMXD_OBJECT_TERMINATE), 1, &elems);
    when_null(elems, exit);
    if(elems->type == RBUS_ELEMENT_TYPE_TABLE) {
        char* part = amxd_path_get_last(&path, true);
        free(part);
        rbusElementInfo_free(rbus_ctx->handle, elems);
        rbusElementInfo_get(rbus_ctx->handle, amxd_path_get(&path, AMXD_OBJECT_TERMINATE), 1, &elems);
    }
    element = elems;
    while(element && matches < 3) {
        if(element->type == RBUS_ELEMENT_TYPE_METHOD) {
            int len = strlen(element->name);
            if(len > 8) {
                if(strncmp(element->name + len - 7, "_exec()", 7) == 0) {
                    matches++;
                } else if(strncmp(element->name + len - 7, "_list()", 7) == 0) {
                    matches++;
                } else if(strncmp(element->name + len - 6, "_get()", 6) == 0) {
                    matches++;
                }
            }
        }
        element = element->next;
    }

    rbusElementInfo_free(rbus_ctx->handle, elems);
    is_amx = (matches == 3);

exit:
    amxd_path_clean(&path);
    return is_amx;
}

amxd_trans_t* amxb_rbus_open_transaction(amxb_rbus_t* amxb_rbus_ctx, amxc_string_t* id) {
    rbus_session_t* session = NULL;
    amxd_trans_t* trans = amxb_rbus_get_transaction(amxb_rbus_ctx, id);
    when_not_null(trans, exit);

    session = (rbus_session_t*) calloc(1, sizeof(rbus_session_t));
    when_null(session, exit);

    amxd_trans_init(&session->transaction);
    amxc_htable_insert(&amxb_rbus_ctx->sessions, amxc_string_get(id, 0), &session->hit);
    trans = &session->transaction;

exit:
    return trans;
}

amxd_trans_t* amxb_rbus_get_transaction(amxb_rbus_t* amxb_rbus_ctx, amxc_string_t* id) {
    rbus_session_t* session = NULL;
    amxd_trans_t* trans = NULL;
    amxc_htable_it_t* hit = amxc_htable_get(&amxb_rbus_ctx->sessions, amxc_string_get(id, 0));
    when_null(hit, exit);

    session = amxc_container_of(hit, rbus_session_t, hit);
    trans = &session->transaction;

exit:
    return trans;
}

void amxb_rbus_close_transaction(amxb_rbus_t* amxb_rbus_ctx, amxc_string_t* id) {
    rbus_session_t* session = NULL;
    amxc_htable_it_t* hit = amxc_htable_get(&amxb_rbus_ctx->sessions, amxc_string_get(id, 0));
    when_null(hit, exit);

    session = amxc_container_of(hit, rbus_session_t, hit);
    amxd_trans_clean(&session->transaction);
    amxc_htable_it_clean(hit, NULL);
    free(session);

exit:
    return;
}

rbusError_t amxb_rbus_common_handler(rbusHandle_t handle,
                                     const char* name,
                                     const char* requester,
                                     uint32_t session,
                                     rbus_condition_wait_t* cond_wait,
                                     amxb_rbus_handler_t handler) {
    rbusError_t status = RBUS_ERROR_INVALID_CONTEXT;
    amxb_rbus_item_t* rbus_item = NULL;
    amxb_rbus_t* amxb_rbus_ctx = amxb_rbus_get_ctx(handle);
    when_null(amxb_rbus_ctx, exit);

    rbus_item = (amxb_rbus_item_t*) calloc(1, sizeof(amxb_rbus_item_t));
    status = RBUS_ERROR_OUT_OF_RESOURCES;
    when_null(rbus_item, exit);

    rbus_item->status = RBUS_ERROR_SUCCESS;
    amxc_string_init(&rbus_item->name, 0);
    amxc_string_init(&rbus_item->transaction_id, 0);
    amxc_string_set(&rbus_item->name, name);
    rbus_item->priv = cond_wait;
    rbus_item->handler = handler;

    if((requester != NULL) && (*requester != 0)) {
        amxc_string_setf(&rbus_item->transaction_id, "%s-%d", requester, session);
    }

    if(cond_wait->tid != amxb_rbus_ctx->tid) {
        pthread_mutex_lock(&cond_wait->lock);
        amxb_rbus_ctrl_write(amxb_rbus_ctx, rbus_item);
        pthread_cond_wait(&cond_wait->condition, &cond_wait->lock);
    } else {
        handler(amxb_rbus_ctx, rbus_item);
    }
    status = cond_wait->status;

exit:
    return status;
}