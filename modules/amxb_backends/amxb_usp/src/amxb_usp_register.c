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

#include "amxb_usp.h"

static int amxb_usp_register_roots(amxb_usp_t* ctx) {
    int retval = -1;
    amxd_object_t* root = amxd_dm_get_root(ctx->dm);
    amxc_var_t ret;
    amxc_var_t args;
    amxc_var_t reg_request;
    amxc_var_t* objects = NULL;
    amxc_var_t* reg_paths = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxd_path_t obj_path;
    char* from_id = amxb_usp_get_from_id();
    char* to_id = amxb_usp_get_to_id(ctx);

    amxd_path_init(&obj_path, NULL);
    amxc_var_init(&reg_request);
    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_set_type(&reg_request, AMXC_VAR_ID_HTABLE);

    amxd_object_invoke_function(root, "_list", &args, &ret);
    objects = GET_ARG(&ret, "objects");
    reg_paths = amxc_var_add_key(amxc_llist_t, &reg_request, "reg_paths", NULL);

    amxc_var_for_each(path, objects) {
        amxc_var_t* reg_path = amxc_var_add(amxc_htable_t, reg_paths, NULL);

        amxd_path_setf(&obj_path, true, "%s", amxc_var_constcast(cstring_t, path));
        amxb_usp_translate_register_path(&obj_path);
        amxc_var_add_key(cstring_t, reg_path, "path", amxd_path_get(&obj_path, AMXD_OBJECT_TERMINATE));
    }

    uspl_tx_new(&usp_tx, from_id, to_id);
    when_failed(uspl_register_new(usp_tx, &reg_request), exit);
    retval = amxb_usp_build_and_send_tlv(ctx, usp_tx);

exit:
    free(to_id);
    free(from_id);
    amxd_path_clean(&obj_path);
    uspl_tx_delete(&usp_tx);
    amxc_var_clean(&reg_request);
    amxc_var_clean(&ret);
    amxc_var_clean(&args);
    return retval;
}

static int amxb_usp_register_custom(amxb_usp_t* ctx, amxc_var_t* registrations) {
    int retval = -1;
    amxc_var_t reg_request;
    amxc_var_t* reg_paths = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_string_t path;
    char* from_id = amxb_usp_get_from_id();
    char* to_id = amxb_usp_get_to_id(ctx);

    amxc_string_init(&path, 0);
    amxc_var_init(&reg_request);
    amxc_var_set_type(&reg_request, AMXC_VAR_ID_HTABLE);

    amxc_var_add_key(bool, &reg_request, "allow_partial", true);
    reg_paths = amxc_var_add_key(amxc_llist_t, &reg_request, "reg_paths", NULL);

    amxc_var_for_each(reg, registrations) {
        amxc_var_t* reg_path = amxc_var_add(amxc_htable_t, reg_paths, NULL);
        amxc_var_t* p = NULL;

        amxc_string_setf(&path, "%s", amxc_var_constcast(cstring_t, reg));
        p = amxc_var_add_new_key(reg_path, "path");
        amxc_var_push(cstring_t, p, amxc_string_take_buffer(&path));
    }

    uspl_tx_new(&usp_tx, from_id, to_id);
    when_failed(uspl_register_new(usp_tx, &reg_request), exit);
    retval = amxb_usp_build_and_send_tlv(ctx, usp_tx);

exit:
    free(to_id);
    free(from_id);
    amxc_string_clean(&path);
    uspl_tx_delete(&usp_tx);
    amxc_var_clean(&reg_request);
    return retval;
}

static uint32_t amxb_usp_get_register_retry_time(void) {
    amxc_var_t* config_opts = amxb_usp_get_config();
    return GET_UINT32(config_opts, "register-retry-time") == 0 ? 10000 : \
           GET_UINT32(config_opts, "register-retry-time");
}

static void amxb_usp_register_retry_cb(amxp_timer_t* timer, void* priv) {
    int retval = -1;
    amxc_var_t* config_opts = amxb_usp_get_config();
    amxc_var_t* registrations = GET_ARG(config_opts, "registrations");
    amxb_usp_t* amxb_usp = (amxb_usp_t*) priv;

    if(registrations == NULL) {
        retval = amxb_usp_register_roots(amxb_usp);
    } else {
        retval = amxb_usp_register_custom(amxb_usp, registrations);
    }

    if(retval != 0) {
        amxp_timer_start(timer, amxb_usp_get_register_retry_time());
    } else {
        amxp_timer_delete(&timer);
        amxb_usp->register_retry = NULL;
    }
}

static void amxb_usp_register_retry_start(amxb_usp_t* amxb_usp) {
    amxp_timer_delete(&amxb_usp->register_retry);
    amxp_timer_new(&amxb_usp->register_retry, amxb_usp_register_retry_cb, amxb_usp);
    amxp_timer_start(amxb_usp->register_retry, amxb_usp_get_register_retry_time());
}

int PRIVATE amxb_usp_register(void* const ctx,
                              amxd_dm_t* const dm) {
    int retval = -1;
    int flags = 0;
    amxb_usp_t* amxb_usp = NULL;
    amxc_var_t* config_opts = amxb_usp_get_config();
    amxc_var_t* registrations = GET_ARG(config_opts, "registrations");
    bool register_retry = GET_BOOL(config_opts, "register-retry");

    when_null(ctx, exit);
    when_null(dm, exit);

    amxb_usp = (amxb_usp_t*) ctx;
    amxb_usp->dm = dm;
    if(dm != NULL) {
        amxp_sigmngr_add_signal(&dm->sigmngr, "dm:operation-complete");
    }

    // Don't try to send register message on listen context
    flags = imtp_connection_get_flags(amxb_usp->icon);
    if((flags & IMTP_LISTEN) == IMTP_LISTEN) {
        retval = 0;
        goto exit;
    }

    if(registrations == NULL) {
        retval = amxb_usp_register_roots(amxb_usp);
    } else {
        retval = amxb_usp_register_custom(amxb_usp, registrations);
    }

    if((retval != 0) && register_retry) {
        amxb_usp_register_retry_start(amxb_usp);
    }

    retval = 0;
exit:

    return retval;
}

int amxb_usp_handle_register(amxb_usp_t* ctx, uspl_rx_t* usp_data) {
    int retval = -1;
    amxc_var_t reg_request;
    amxc_var_t* reg_paths = NULL;
    amxc_string_t* path = NULL;

    amxc_var_init(&reg_request);
    amxc_string_new(&path, 0);

    when_null(ctx, exit);
    when_null(usp_data, exit);

    retval = uspl_register_extract(usp_data, &reg_request);
    when_failed(retval, exit);

    reg_paths = GET_ARG(&reg_request, "reg_paths");
    amxc_var_for_each(reg_path, reg_paths) {
        amxc_string_setf(path, "%s", GET_CHAR(reg_path, "path"));
        amxc_llist_append(&ctx->registrations, &path->it);
    }

exit:
    amxc_var_clean(&reg_request);

    return retval;
}
