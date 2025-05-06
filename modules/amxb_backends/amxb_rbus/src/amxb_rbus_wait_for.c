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

#include "amxb_rbus.h"

static amxc_llist_t wait_objects;
static amxp_timer_t* check_timer = NULL;

static void amxb_rbus_check_objects(UNUSED amxp_timer_t* timer, void* priv) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) priv;
    rbusElementInfo_t* element = NULL;
    amxc_string_t sig_name;
    amxc_string_init(&sig_name, 0);

    amxc_llist_for_each(it, &wait_objects) {
        amxc_string_t* path = amxc_string_from_llist_it(it);
        rbusElementInfo_get(amxb_rbus_ctx->handle, amxc_string_get(path, 0), 0, &element);
        if(element == NULL) {
            continue;
        }
        amxc_string_setf(&sig_name, "wait:%s", amxc_string_get(path, 0));
        amxp_sigmngr_emit_signal(NULL, amxc_string_get(&sig_name, 0), NULL);
        amxc_string_delete(&path);
        rbusElementInfo_free(amxb_rbus_ctx->handle, element);
    }

    amxc_string_clean(&sig_name);
}

static void amxb_rbus_wait_for_done(UNUSED const char* const sig_name,
                                    UNUSED const amxc_var_t* const d,
                                    void* const priv) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) priv;
    amxp_slot_disconnect_with_priv(NULL, amxb_rbus_wait_for_done, amxb_rbus_ctx);
    amxp_timer_delete(&check_timer);
    amxc_llist_clean(&wait_objects, amxc_string_list_it_free);
}

int amxb_rbus_wait_for(void* const ctx, const char* object) {
    int retval = -1;
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    amxc_string_t* path = NULL;

    amxc_string_new(&path, 0);
    amxc_string_set(path, object);
    amxc_string_trimr(path, isdot);
    amxc_string_append(path, ".", 1);

    amxc_llist_append(&wait_objects, &path->it);

    amxp_slot_connect(NULL, "wait:done", NULL, amxb_rbus_wait_for_done, amxb_rbus_ctx);
    amxp_slot_connect(NULL, "wait:cancel", NULL, amxb_rbus_wait_for_done, amxb_rbus_ctx);

    if(check_timer == NULL) {
        amxp_timer_new(&check_timer, amxb_rbus_check_objects, amxb_rbus_ctx);
        amxp_timer_set_interval(check_timer, 2500);
        amxp_timer_start(check_timer, 1500);
    }

    retval = 0;

    return retval;
}

CONSTRUCTOR static void amxb_rbus_wait_for_init(void) {
    amxc_llist_init(&wait_objects);
}

DESTRUCTOR static void amxb_rbus_wait_for_clean(void) {
    amxp_timer_delete(&check_timer);
    amxc_llist_clean(&wait_objects, amxc_string_list_it_free);
}