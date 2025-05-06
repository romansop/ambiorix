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

#include <amxtui/amxtui.h>
#include "amxtui_priv.h"

int amxtui_ctrl_new(amxtui_ctrl_t** ctrl, const char* type_name) {
    int rv = -1;
    amxtui_ctrl_type_t* type = NULL;
    when_null(ctrl, exit);

    type = amxtui_ctrl_type_get(type_name);
    when_null(type, exit);

    if(type->ctor != NULL) {
        type->ctor(ctrl);
    }
    when_null(*ctrl, exit);

    (*ctrl)->ctrl_type = type;
    (*ctrl)->ref = 1;

    rv = 0;

exit:
    return rv;
}

int amxtui_ctrl_delete(amxtui_ctrl_t** ctrl) {
    int rv = -1;
    when_null(ctrl, exit);
    when_null((*ctrl), exit);

    (*ctrl)->ref--;
    if((*ctrl)->ref == 0) {
        if(((*ctrl)->ctrl_type != NULL) && ((*ctrl)->ctrl_type->dtor != NULL)) {
            (*ctrl)->ctrl_type->dtor(ctrl);
        }
    }

    rv = 0;
    *ctrl = NULL;

exit:
    return rv;
}

int amxtui_ctrl_init(amxtui_ctrl_t* ctrl,
                     amxtui_show_fn_t show_fn,
                     amxtui_ctrl_key_fn_t ctrl_key_fn,
                     amxtui_key_fn_t key_fn,
                     amxtui_get_data_fn_t data_fn) {
    when_null(ctrl, exit);

    amxp_sigmngr_init(&ctrl->sigmngr);
    amxp_sigmngr_add_signal(&ctrl->sigmngr, CTRL_SIG_REDRAW);
    amxp_sigmngr_add_signal(&ctrl->sigmngr, CTRL_SIG_RESET);
    amxp_sigmngr_add_signal(&ctrl->sigmngr, CTRL_SIG_BUTTON);

    ctrl->show_content = show_fn;
    ctrl->handle_ctrl_key = ctrl_key_fn;
    ctrl->handle_key = key_fn;
    ctrl->get_data = data_fn;

exit:
    return 0;
}

int amxtui_ctrl_clean(amxtui_ctrl_t* ctrl) {
    when_null(ctrl, exit);

    amxp_sigmngr_clean(&ctrl->sigmngr);

exit:
    return 0;
}

int amxtui_ctrl_inherits(amxtui_ctrl_t* ctrl, const char* type_name) {
    when_null(ctrl, exit);

    amxtui_ctrl_new(&ctrl->base, type_name);

    amxp_sigmngr_init(&ctrl->sigmngr);
    amxp_sigmngr_add_signal(&ctrl->sigmngr, CTRL_SIG_REDRAW);
    amxp_sigmngr_add_signal(&ctrl->sigmngr, CTRL_SIG_RESET);
    amxp_sigmngr_add_signal(&ctrl->sigmngr, CTRL_SIG_BUTTON);

    ctrl->show_content = ctrl->base->show_content;
    ctrl->handle_ctrl_key = ctrl->base->handle_ctrl_key;
    ctrl->handle_key = ctrl->base->handle_key;
    ctrl->get_data = ctrl->base->get_data;

exit:
    return 0;
}

int amxtui_ctrl_take_reference(amxtui_ctrl_t* ctrl) {
    when_null(ctrl, exit);

    ctrl->ref++;

exit:
    return 0;
}

const char* amxtui_ctrl_type_name(const amxtui_ctrl_t* ctrl) {
    const char* name = NULL;
    when_null(ctrl, exit);
    when_null(ctrl->ctrl_type, exit);

    name = amxc_htable_it_get_key(&ctrl->ctrl_type->hit);

exit:
    return name;
}

bool amxtui_ctrl_is_type(const amxtui_ctrl_t* ctrl, const char* type_name) {
    bool is_type = false;
    const char* name = amxtui_ctrl_type_name(ctrl);
    when_null(name, exit);
    when_null(type_name, exit);

    if(ctrl->base) {
        is_type = amxtui_ctrl_is_type(ctrl->base, type_name);
        when_true(is_type, exit);
    }
    is_type = (strcmp(type_name, name) == 0);

exit:
    return is_type;
}

amxtui_ctrl_t* amxtui_ctrl_get_type(const amxtui_ctrl_t* ctrl, const char* type_name) {
    amxtui_ctrl_t* type_ctrl = (amxtui_ctrl_t*) ctrl;
    const char* name = amxtui_ctrl_type_name(ctrl);
    when_null(name, exit);
    when_null(type_name, exit);

    when_true(strcmp(type_name, name) == 0, exit);
    type_ctrl = amxtui_ctrl_get_type(ctrl->base, type_name);

exit:
    return type_ctrl;
}

int amxtui_ctrl_add_signal(amxtui_ctrl_t* ctrl, const char* signal) {
    when_null(ctrl, exit);

    amxp_sigmngr_add_signal(&ctrl->sigmngr, signal);

exit:
    return 0;
}

int amxtui_ctrl_emit_text(amxtui_ctrl_t* ctrl, const char* signal, const char* txt) {
    amxc_var_t data;
    when_null(ctrl, exit);

    amxc_var_init(&data);
    amxc_var_set(cstring_t, &data, txt);
    amxp_sigmngr_emit_signal(&ctrl->sigmngr, signal, &data);
    amxc_var_clean(&data);

exit:
    return 0;
}

int amxtui_ctrl_trigger_text(amxtui_ctrl_t* ctrl, const char* signal, const char* txt) {
    amxc_var_t data;
    when_null(ctrl, exit);

    amxc_var_init(&data);
    amxc_var_set(cstring_t, &data, txt);
    amxp_sigmngr_trigger_signal(&ctrl->sigmngr, signal, &data);
    amxc_var_clean(&data);

exit:
    return 0;
}

int amxtui_ctrl_emit_data(amxtui_ctrl_t* ctrl, const char* signal, const amxc_var_t* data) {
    when_null(ctrl, exit);

    amxp_sigmngr_emit_signal(&ctrl->sigmngr, signal, data);

exit:
    return 0;
}

int amxtui_ctrl_trigger_data(amxtui_ctrl_t* ctrl, const char* signal, const amxc_var_t* data) {
    when_null(ctrl, exit);

    amxp_sigmngr_trigger_signal(&ctrl->sigmngr, signal, data);

exit:
    return 0;
}