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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>

#include "dmtui.h"

static dmtui_app_t app;

static void dmtui_read_input(UNUSED int fd, UNUSED void* priv) {
    amxtui_screen_read();
}

static void dmtui_set_access(void) {
    amxc_llist_t* connections = amxp_connection_get_connections();
    const char* access = GET_CHAR(&app.parser->config, "access");

    if(access == NULL) {
        access = "public";
    }

    amxc_llist_for_each(it, connections) {
        amxp_connection_t* con = amxc_llist_it_get_data(it, amxp_connection_t, it);
        amxb_bus_ctx_t* bus_ctx = (amxb_bus_ctx_t*) con->priv;
        if(con->type != AMXP_CONNECTION_BUS) {
            continue;
        }
        if(strcmp(access, "protected") == 0) {
            amxb_set_access(bus_ctx, AMXB_PROTECTED);
        } else {
            // default
            amxb_set_access(bus_ctx, AMXB_PUBLIC);
        }
    }

    return;
}

static void dmtui_init(void) {
    amxtui_ctrl_t* ctrl = NULL;
    amxc_var_t* objects = GET_ARG(&app.parser->config, "objects");

    amxtui_screen_init();
    // create the screen widgets
    dmtui_widgets_create(&app);
    // add a screen controller and connect everything together
    dmtui_create_screen_controller(&app);

    dmtui_set_access();

    ctrl = amxtui_widget_get_ctrl(app.left);
    dmtui_dm_browser_load(ctrl, objects);

    amxp_connection_add(fileno(stdin),
                        dmtui_read_input,
                        NULL,
                        AMXP_CONNECTION_CUSTOM,
                        NULL);

    amxtui_screen_set_focus(app.left);
    amxtui_screen_redraw();
}

static void dmtui_cleanup(void) {
    dmtui_widgets_delete(&app);
    dmtui_delete_screen_controller(&app);
    amxtui_screen_cleanup();
}

dmtui_app_t* dmtui(void) {
    return &app;
}

int _dmtui_main(int reason,
                amxd_dm_t* dm,
                amxo_parser_t* parser) {

    switch(reason) {
    case 0:     // START
        app.dm = dm;
        app.parser = parser;
        dmtui_init();
        break;
    case 1:     // STOP
        dmtui_cleanup();
        app.dm = NULL;
        app.parser = NULL;
        break;
    }

    return 0;
}