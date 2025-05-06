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

static void amxb_rbus_add_to_transaction(amxb_rbus_t* amxb_rbus_ctx,
                                         amxb_rbus_item_t* item) {

    rbus_condition_wait_t* cond_wait = (rbus_condition_wait_t*) item->priv;
    amxd_path_t path;
    amxd_object_t* object = NULL;
    amxd_param_t* param = NULL;
    amxc_var_t value;
    const char* requested = NULL;
    const char* translated = NULL;

    // amxb_rbus_open_transaction will create one if no transaction exists
    // for the given transaction_id or return the existing one.
    amxd_trans_t* transaction = amxb_rbus_open_transaction(amxb_rbus_ctx, &item->transaction_id);

    amxc_var_init(&value);
    amxd_path_init(&path, amxc_string_get(&item->name, 0));

    // check if translation is need
    // if translation configuration is available, this will translate
    // the requested path into the internal path
    amxb_rbus_translate_path(&path, &requested, &translated);

    cond_wait->status = RBUS_ERROR_ELEMENT_DOES_NOT_EXIST;
    object = amxd_dm_findf(amxb_rbus_ctx->dm, "%s", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
    // As it is possible that in the time between receiving the message and the
    // switch to the main thread, the object was removed.
    // Therefor it is possible the object was not found.
    when_null(object, exit);

    amxb_rbus_value_to_var(&value, cond_wait->value);
    param = amxd_object_get_param_def(object, amxd_path_get_param(&path));
    // it is possible that the parameter is not managed by the
    // data model engine, in that case it is not possible to
    // validate the value.
    // If the parameter is managed by the data model engine, a parameter
    // definition is found, and the new value can be validated.
    // if the new value is not valid, close the transaction and fail
    if(param != NULL) {
        cond_wait->status = RBUS_ERROR_INVALID_INPUT;
        when_failed(amxd_param_validate(param, &value), exit);
    }

    amxd_trans_select_object(transaction, object);
    amxd_trans_set_param(transaction, amxd_path_get_param(&path), &value);
    cond_wait->status = RBUS_ERROR_SUCCESS;

exit:
    amxd_path_clean(&path);
    amxc_var_clean(&value);
    return;
}

static void amxb_rbus_set_validate(amxb_rbus_t* amxb_rbus_ctx,
                                   amxb_rbus_item_t* item) {
    rbus_condition_wait_t* cond_wait = (rbus_condition_wait_t*) item->priv;
    pthread_mutex_lock(&cond_wait->lock);

    // validate the value and add it to the transaction.
    amxb_rbus_add_to_transaction(amxb_rbus_ctx, item);
    if(cond_wait->status != RBUS_ERROR_SUCCESS) {
        amxb_rbus_close_transaction(amxb_rbus_ctx, &item->transaction_id);
    }

    if(cond_wait->tid != amxb_rbus_ctx->tid) {
        pthread_cond_signal(&cond_wait->condition);
    }
    pthread_mutex_unlock(&cond_wait->lock);
}

static void amxb_rbus_set_apply(amxb_rbus_t* amxb_rbus_ctx,
                                amxb_rbus_item_t* item) {
    rbus_condition_wait_t* cond_wait = (rbus_condition_wait_t*) item->priv;
    amxd_trans_t* transaction = NULL;
    amxd_status_t status = amxd_status_ok;

    pthread_mutex_lock(&cond_wait->lock);
    amxb_rbus_add_to_transaction(amxb_rbus_ctx, item);
    when_false(cond_wait->status == RBUS_ERROR_SUCCESS, exit);

    transaction = amxb_rbus_get_transaction(amxb_rbus_ctx, &item->transaction_id);
    status = amxd_trans_apply(transaction, amxb_rbus_ctx->dm);
    cond_wait->status = amxb_rbus_translate_status2rbus(status);

exit:
    amxb_rbus_close_transaction(amxb_rbus_ctx, &item->transaction_id);
    if(cond_wait->tid != amxb_rbus_ctx->tid) {
        pthread_cond_signal(&cond_wait->condition);
    }
    pthread_mutex_unlock(&cond_wait->lock);
}

rbusError_t amxb_rbus_set_value(rbusHandle_t handle,
                                rbusProperty_t property,
                                rbusSetHandlerOptions_t* options) {

    const char* name = NULL;
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

    cond_wait.value = rbusProperty_GetValue(property);
    name = rbusProperty_GetName(property);

    // if options->commit is set to  false, only validate the new value
    // and add it to a transaction. If validation fails, do a cleanup
    // of the transaction and return an error.
    // If options->commit is set to true, add the parameter to the
    // transaction and apply the transaction.
    // Then the return value should be the result of the transaction.

    status = amxb_rbus_common_handler(handle,
                                      name,
                                      options->requestingComponent,
                                      options->sessionId,
                                      &cond_wait,
                                      options->commit? amxb_rbus_set_apply:amxb_rbus_set_validate);

    return status;
}
