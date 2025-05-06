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

#include "busstats_main.h"
#include <stdlib.h>
#include <string.h>
#include <debug/sahtrace.h>
#include <debug/sahtrace_macros.h>
#include <amxb/amxb_stats.h>

#define ME "busstats"

static amxd_dm_t* s_dm;

#define GETBUSSTATS_RPC_FUNCTION_NAME "GetBusStats"
#define RESETBUSSTATS_RPC_FUNCTION_NAME "ResetBusStats"

/** @implements amxd_object_fn_t */
static amxd_status_t s_get_bus_stats (UNUSED amxd_object_t* object,
                                      UNUSED amxd_function_t* func,
                                      UNUSED amxc_var_t* args,
                                      amxc_var_t* ret) {

    amxc_var_set_type(ret, AMXC_VAR_ID_HTABLE);
    amxc_array_t* uris = amxb_list_uris();
    for(amxc_array_it_t* it = amxc_array_get_first(uris); it != NULL; it = amxc_array_it_get_next(it)) {
        const char* uri = amxc_array_it_get_data(it);
        amxb_bus_ctx_t* ctx = amxb_find_uri(uri);
        if(ctx == NULL) {
            SAH_TRACEZ_ERROR(ME, "No bus context found for %s", uri);
        }

        amxc_var_t* stats_for_connection = amxc_var_add_new_key(ret, uri);
        amxb_stats_get(ctx, stats_for_connection);
    }

    return amxd_status_ok;
}

static amxd_status_t s_reset_bus_stats (UNUSED amxd_object_t* object,
                                        UNUSED amxd_function_t* func,
                                        UNUSED amxc_var_t* args,
                                        UNUSED amxc_var_t* ret) {

    amxc_array_t* uris = amxb_list_uris();
    for(amxc_array_it_t* it = amxc_array_get_first(uris); it != NULL; it = amxc_array_it_get_next(it)) {
        const char* uri = amxc_array_it_get_data(it);
        amxb_bus_ctx_t* ctx = amxb_find_uri(uri);
        if(ctx == NULL) {
            SAH_TRACEZ_ERROR(ME, "No bus context found for %s", uri);
        }

        amxb_stats_reset(ctx);
    }

    return amxd_status_ok;
}

static void s_register (void) {
    amxd_object_for_each(child, it, amxd_dm_get_root(s_dm)) {
        amxd_status_t status = amxd_status_unknown_error;
        amxd_object_t* obj = amxc_container_of(it, amxd_object_t, it);
        amxd_function_t* func = NULL;

        when_null(obj, exit);
        status = amxd_function_new(&func,
                                   GETBUSSTATS_RPC_FUNCTION_NAME,
                                   AMXC_VAR_ID_NULL,
                                   s_get_bus_stats);
        if((status != amxd_status_ok) || (func == NULL)) {
            SAH_TRACEZ_ERROR(ME, "Error creating function %s", GETBUSSTATS_RPC_FUNCTION_NAME);
            continue;
        }
        amxd_function_set_attr(func, amxd_fattr_protected, true);
        status = amxd_object_add_function(obj, func);
        if((status != amxd_status_ok) || (func == NULL)) {
            SAH_TRACEZ_ERROR(ME, "Error adding function %s", GETBUSSTATS_RPC_FUNCTION_NAME);
        }

        status = amxd_function_new(&func,
                                   RESETBUSSTATS_RPC_FUNCTION_NAME,
                                   AMXC_VAR_ID_NULL,
                                   s_reset_bus_stats);
        if((status != amxd_status_ok) || (func == NULL)) {
            SAH_TRACEZ_ERROR(ME, "Error creating function %s", RESETBUSSTATS_RPC_FUNCTION_NAME);
            continue;
        }
        amxd_function_set_attr(func, amxd_fattr_protected, true);
        status = amxd_object_add_function(obj, func);
        if((status != amxd_status_ok) || (func == NULL)) {
            SAH_TRACEZ_ERROR(ME, "Error adding function %s", RESETBUSSTATS_RPC_FUNCTION_NAME);
        }
    }

exit:
    return;
}

void app_start(UNUSED const char* const sig_name,
               UNUSED const amxc_var_t* const data,
               UNUSED void* const priv) {
    s_register();

    return;
}

int _main(int reason,
          amxd_dm_t* dm,
          UNUSED amxo_parser_t* parser) {
    int retval = 0;
    switch(reason) {
    case 0: // START
        s_dm = dm;
        break;
    case 1: // STOP
        s_dm = NULL;
        break;
    }

    return retval;
}