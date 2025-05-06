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

#include "amxb/amxb_stats.h"
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxd/amxd_dm.h>
#include <amxb/amxb_be_intf.h>
#include <amxb/amxb.h>
#include "amxb_priv.h"

static void s_append_tx_stats(amxb_bus_ctx_t* bus_ctx, amxc_var_t* stats) {
    amxc_var_t* operation_tx = NULL;
    amxc_var_t* tx = amxc_var_add_new_key(stats, "tx");
    amxc_var_set_type(tx, AMXC_VAR_ID_HTABLE);
    operation_tx = amxc_var_add_new_key(tx, "operation");
    amxc_var_set_type(operation_tx, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint64_t, operation_tx, "invoke", bus_ctx->stats->counter_tx_invoke);
    amxc_var_add_key(uint64_t, operation_tx, "async_invoke", bus_ctx->stats->counter_tx_async_invoke);
    amxc_var_add_key(uint64_t, operation_tx, "add", bus_ctx->stats->counter_tx_add);
    amxc_var_add_key(uint64_t, operation_tx, "get", bus_ctx->stats->counter_tx_get);
    amxc_var_add_key(uint64_t, operation_tx, "get_filtered", bus_ctx->stats->counter_tx_get_filtered);
    amxc_var_add_key(uint64_t, operation_tx, "set", bus_ctx->stats->counter_tx_set);
    amxc_var_add_key(uint64_t, operation_tx, "del", bus_ctx->stats->counter_tx_del);
    amxc_var_add_key(uint64_t, operation_tx, "describe", bus_ctx->stats->counter_tx_describe);
    amxc_var_add_key(uint64_t, operation_tx, "list", bus_ctx->stats->counter_tx_list);
    amxc_var_add_key(uint64_t, operation_tx, "get_instances", bus_ctx->stats->counter_tx_get_instances);
    amxc_var_add_key(uint64_t, operation_tx, "has", bus_ctx->stats->counter_tx_has);
    amxc_var_add_key(uint64_t, operation_tx, "subscribe", bus_ctx->stats->counter_tx_subscribe);
    amxc_var_add_key(uint64_t, operation_tx, "get_supported", bus_ctx->stats->counter_tx_get_supported);
    amxc_var_add_key(uint64_t, operation_tx, "unsubscribe", bus_ctx->stats->counter_tx_unsubscribe);
    amxc_var_add_key(uint64_t, operation_tx, "wait_for", bus_ctx->stats->counter_tx_wait_for);
}

static void s_reset_tx_stats(amxb_bus_ctx_t* bus_ctx) {
    bus_ctx->stats->counter_tx_invoke = 0;
    bus_ctx->stats->counter_tx_async_invoke = 0;
    bus_ctx->stats->counter_tx_add = 0;
    bus_ctx->stats->counter_tx_get = 0;
    bus_ctx->stats->counter_tx_get_filtered = 0;
    bus_ctx->stats->counter_tx_set = 0;
    bus_ctx->stats->counter_tx_del = 0;
    bus_ctx->stats->counter_tx_describe = 0;
    bus_ctx->stats->counter_tx_list = 0;
    bus_ctx->stats->counter_tx_get_instances = 0;
    bus_ctx->stats->counter_tx_has = 0;
    bus_ctx->stats->counter_tx_subscribe = 0;
    bus_ctx->stats->counter_tx_get_supported = 0;
    bus_ctx->stats->counter_tx_unsubscribe = 0;
    bus_ctx->stats->counter_tx_wait_for = 0;
}

void amxb_stats_new(amxb_stats_t** stats) {
    when_null(stats, exit);
    *stats = (amxb_stats_t*) calloc(1, sizeof(amxb_stats_t));

exit:
    return;
}

void amxb_stats_delete(amxb_stats_t** stats) {
    when_null(stats, exit);
    when_null(*stats, exit);
    free(*stats);
    *stats = NULL;

exit:
    return;
}

void amxb_stats_get(amxb_bus_ctx_t* bus_ctx, amxc_var_t* var) {
    const amxb_be_funcs_t* fns = NULL;
    when_null(bus_ctx, exit);
    when_null(var, exit);
    when_null(bus_ctx->stats, exit);
    fns = bus_ctx->bus_fn;
    amxc_var_set_type(var, AMXC_VAR_ID_HTABLE);

    if(amxb_is_valid_be_func(fns, get, fns->get_stats)) {
        fns->get_stats(bus_ctx->bus_ctx, var);
    }

    s_append_tx_stats(bus_ctx, var);

exit:
    return;
}

void amxb_stats_reset(amxb_bus_ctx_t* bus_ctx) {
    const amxb_be_funcs_t* fns = NULL;
    when_null(bus_ctx, exit);
    when_null(bus_ctx->stats, exit);
    fns = bus_ctx->bus_fn;

    if(amxb_is_valid_be_func(fns, get, fns->reset_stats)) {
        fns->reset_stats(bus_ctx->bus_ctx);
    }

    s_reset_tx_stats(bus_ctx);

exit:
    return;
}