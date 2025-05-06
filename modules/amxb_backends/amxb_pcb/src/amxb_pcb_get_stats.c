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

#include "amxb_pcb.h"

/** @implements @ref amxb_be_stats_t */
int amxb_pcb_get_stats(void* const ctx,
                       amxc_var_t* stats) {
    int retval = -1;
    amxc_var_t* rx_stats = NULL;
    amxc_var_t* operation_rx_stats = NULL;
    amxb_pcb_t* amxb_pcb_ctx = (amxb_pcb_t*) ctx;
    when_null(amxb_pcb_ctx, exit);
    when_null(stats, exit);
    rx_stats = amxc_var_add_new_key(stats, "rx");
    amxc_var_set_type(rx_stats, AMXC_VAR_ID_HTABLE);
    operation_rx_stats = amxc_var_add_new_key(rx_stats, "operation");
    amxc_var_set_type(operation_rx_stats, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint64_t, operation_rx_stats, "invoke", amxb_pcb_ctx->stats.counter_rx_invoke);
    amxc_var_add_key(uint64_t, operation_rx_stats, "get", amxb_pcb_ctx->stats.counter_rx_get);
    amxc_var_add_key(uint64_t, operation_rx_stats, "set", amxb_pcb_ctx->stats.counter_rx_set);
    amxc_var_add_key(uint64_t, operation_rx_stats, "add", amxb_pcb_ctx->stats.counter_rx_add);
    amxc_var_add_key(uint64_t, operation_rx_stats, "del", amxb_pcb_ctx->stats.counter_rx_del);
    amxc_var_add_key(uint64_t, operation_rx_stats, "close_request", amxb_pcb_ctx->stats.counter_rx_close_request);

    retval = 0;
exit:
    return retval;
}