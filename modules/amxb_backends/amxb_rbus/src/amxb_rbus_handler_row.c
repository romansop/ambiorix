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

#include <stdlib.h>
#include <ctype.h>

#include "amxb_rbus.h"
#include "amxb_rbus_ctrl.h"
#include "amxb_rbus_handlers.h"

static void amxb_rbus_add_row_impl(amxb_rbus_t* amxb_rbus_ctx,
                                   amxb_rbus_item_t* item) {
    rbus_condition_wait_t* cond_wait = (rbus_condition_wait_t*) item->priv;
    amxd_path_t path;
    amxd_object_t* object = NULL;
    amxd_trans_t transaction;
    const char* requested = NULL;
    const char* translated = NULL;
    amxd_status_t status = amxd_status_ok;

    pthread_mutex_lock(&cond_wait->lock);
    amxd_trans_init(&transaction);
    amxd_path_init(&path, amxc_string_get(&item->name, 0));
    cond_wait->status = RBUS_ERROR_INVALID_INPUT;

    // check if translation is need
    // if translation configuration is available, this will translate
    // the requested path into the internal path
    amxb_rbus_translate_path(&path, &requested, &translated);

    cond_wait->status = RBUS_ERROR_ELEMENT_DOES_NOT_EXIST;
    object = amxd_dm_findf(amxb_rbus_ctx->dm, "%s", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));

    if(object != NULL) {
        amxd_trans_select_object(&transaction, object);
        amxd_trans_add_inst(&transaction, 0, cond_wait->alias);
        status = amxd_trans_apply(&transaction, amxd_object_get_dm(object));
        cond_wait->status = amxb_rbus_translate_status2rbus(status);
        if(status == amxd_status_ok) {
            char* p = amxd_object_get_path(transaction.current, AMXD_OBJECT_INDEXED | AMXD_OBJECT_TERMINATE);
            cond_wait->index = amxd_object_get_index(transaction.current);
            amxc_var_add_key(bool, &amxb_rbus_ctx->ignore, p, true);
            free(p);
        }
    }

    if(cond_wait->tid != amxb_rbus_ctx->tid) {
        pthread_cond_signal(&cond_wait->condition);
    }
    amxd_trans_clean(&transaction);
    amxd_path_clean(&path);
    pthread_mutex_unlock(&cond_wait->lock);
}

static void amxb_rbus_add_ignore(amxd_object_t* const object, UNUSED int32_t depth, void* priv) {
    amxc_var_t* paths = (amxc_var_t*) priv;
    char* p = amxd_object_get_path(object, AMXD_OBJECT_TERMINATE | AMXD_OBJECT_INDEXED);
    amxc_var_add(cstring_t, paths, p);
    free(p);
}

static void amxb_rbus_remove_row_impl(amxb_rbus_t* amxb_rbus_ctx,
                                      amxb_rbus_item_t* item) {
    rbus_condition_wait_t* cond_wait = (rbus_condition_wait_t*) item->priv;
    amxd_path_t path;
    amxd_object_t* object = NULL;
    amxd_trans_t transaction;
    uint32_t index = 0;
    const char* requested = NULL;
    const char* translated = NULL;
    amxd_status_t status = amxd_status_ok;

    pthread_mutex_lock(&cond_wait->lock);
    amxd_trans_init(&transaction);
    amxd_path_init(&path, NULL);
    amxd_path_setf(&path, true, "%s", amxc_string_get(&item->name, 0));
    cond_wait->status = RBUS_ERROR_INVALID_INPUT;

    // check if translation is need
    // if translation configuration is available, this will translate
    // the requested path into the internal path
    amxb_rbus_translate_path(&path, &requested, &translated);

    cond_wait->status = RBUS_ERROR_ELEMENT_DOES_NOT_EXIST;
    object = amxd_dm_findf(amxb_rbus_ctx->dm, "%s", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));

    if(object != NULL) {
        cond_wait->status = RBUS_ERROR_INVALID_INPUT;
        if(amxd_object_get_type(object) == amxd_object_instance) {
            amxc_var_t paths;
            amxc_var_init(&paths);
            amxc_var_set_type(&paths, AMXC_VAR_ID_LIST);

            amxd_object_hierarchy_walk(object, amxd_direction_down, NULL, amxb_rbus_add_ignore, INT32_MAX, &paths);
            index = amxd_object_get_index(object);

            amxd_trans_select_object(&transaction, amxd_object_get_parent(object));
            amxd_trans_del_inst(&transaction, index, NULL);
            status = amxd_trans_apply(&transaction, amxd_object_get_dm(object));
            cond_wait->status = amxb_rbus_translate_status2rbus(status);

            if(status == amxd_status_ok) {
                // make sure that the instance delete event is ignored, otherwhise a
                // segmentation fault occurs in librbus
                amxc_var_for_each(p, &paths) {
                    amxc_var_add_key(bool, &amxb_rbus_ctx->ignore, GET_CHAR(p, NULL), true);
                }
            }

            amxc_var_clean(&paths);
        }
    }

    if(cond_wait->tid != amxb_rbus_ctx->tid) {
        pthread_cond_signal(&cond_wait->condition);
    }
    amxd_trans_clean(&transaction);
    amxd_path_clean(&path);
    pthread_mutex_unlock(&cond_wait->lock);
}

rbusError_t amxb_rbus_add_row(rbusHandle_t handle,
                              char const* table_name,
                              char const* alias,
                              uint32_t* index) {
    rbusError_t status = RBUS_ERROR_INVALID_CONTEXT;

    rbus_condition_wait_t cond_wait = {
        .condition = PTHREAD_COND_INITIALIZER,
        .lock = PTHREAD_MUTEX_INITIALIZER,
        .value = NULL,
        .property = NULL,
        .alias = alias,
        .index = 0,
        .tid = pthread_self(),
    };

    status = amxb_rbus_common_handler(handle,
                                      table_name,
                                      NULL,
                                      0,
                                      &cond_wait,
                                      amxb_rbus_add_row_impl);
    *index = cond_wait.index;

    return status;
}

rbusError_t amxb_rbus_remove_row(rbusHandle_t handle, char const* row_name) {
    rbusError_t status = RBUS_ERROR_INVALID_CONTEXT;
    rbus_condition_wait_t cond_wait = {
        .condition = PTHREAD_COND_INITIALIZER,
        .lock = PTHREAD_MUTEX_INITIALIZER,
        .value = NULL,
        .property = NULL,
        .alias = NULL,
        .index = 0,
        .tid = pthread_self(),
    };

    status = amxb_rbus_common_handler(handle,
                                      row_name,
                                      NULL,
                                      0,
                                      &cond_wait,
                                      amxb_rbus_remove_row_impl);
    return status;
}
