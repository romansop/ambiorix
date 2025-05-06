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

#include "dmtui.h"

static bool dmtui_screen_handle_ctrl_key(UNUSED amxtui_widget_t* widget,
                                         UNUSED amxtui_ctrl_t* ctrl,
                                         uint32_t ctrl_key) {
    dmtui_app_t* app = dmtui();
    bool handled = true;

    static bool right = false;
    static amxtui_widget_t* right_widget = NULL;
    if(right_widget == NULL) {
        right_widget = app->right_top;
    }

    switch(ctrl_key) {
    case amxt_key_shift_up:
    case amxt_key_ctrl_up:
        if(amxtui_widget_has_focus(app->bottom)) {
            amxtui_screen_set_focus(right? app->right_bottom:app->left);
            if(right) {
                right_widget = app->right_top;
            }
        } else if(amxtui_widget_has_focus(app->right_bottom)) {
            amxtui_screen_set_focus(app->right_top);
            right_widget = app->right_top;
        }
        break;
    case amxt_key_shift_down:
    case amxt_key_ctrl_down:
        if(amxtui_widget_has_focus(app->left) ||
           amxtui_widget_has_focus(app->right_bottom)) {
            amxtui_screen_set_focus(app->bottom);
        } else if(amxtui_widget_has_focus(app->right_top)) {
            amxtui_screen_set_focus(app->right_bottom);
            right_widget = app->right_bottom;
        }
        break;
    case amxt_key_shift_left:
    case amxt_key_ctrl_left:
        if(amxtui_widget_has_focus(app->right_bottom) ||
           amxtui_widget_has_focus(app->right_top)) {
            amxtui_screen_set_focus(app->left);
            right = false;
        }
        break;
    case amxt_key_shift_right:
    case amxt_key_ctrl_right:
        if(amxtui_widget_has_focus(app->left)) {
            amxtui_screen_set_focus(right_widget);
            right = true;
        }
        break;
    default:
        handled = false;
        break;
    }
    return handled;
}

static void dmtui_screen_slot_update(UNUSED const char* const sig_name,
                                     const amxc_var_t* const data,
                                     UNUSED void* const priv) {
    dmtui_app_t* app = dmtui();
    const char* str_path = NULL;
    amxd_path_t path;

    amxd_path_init(&path, NULL);

    when_false(amxc_var_type_of(data) == AMXC_VAR_ID_CSTRING, exit);

    str_path = GET_CHAR(data, NULL);
    amxd_path_setf(&path, false, "%s", str_path);
    when_false(amxd_path_get_type(&path) == amxd_path_object, exit);
    amxtui_ctrl_emit_text(&app->ctrl_main, "select_object", str_path);

exit:
    amxd_path_clean(&path);
    return;
}

static void dmtui_screen_slot_reset_right(UNUSED const char* const sig_name,
                                          UNUSED const amxc_var_t* const data,
                                          UNUSED void* const priv) {
    dmtui_app_t* app = dmtui();
    amxtui_widget_set_title(app->right_bottom, "Search Result");
    amxtui_widget_set_ctrl(app->right_bottom, app->ctrl_right_bottom);
    amxtui_widget_redraw(app->right_bottom);
}

static bool dmtui_start_game(dmtui_app_t* app, const char* const str_path) {
    bool handled = false;

    if(strcmp(str_path, "Jonathan") == 0) {
        amxtui_widget_set_ctrl(app->right_bottom, app->ctrl_snake);
        amxtui_snake_widget_start(app->right_bottom);
        amxtui_screen_set_focus(app->right_bottom);
        amxtui_widget_redraw(app->right_bottom);
        handled = true;
    }

    return handled;
}

static bool dmtui_add_objects(dmtui_app_t* app, const char* const str_path) {
    bool handled = false;

    if(str_path[0] == '+') {
        amxc_var_t objects;
        amxc_var_init(&objects);
        amxc_var_set(csv_string_t, &objects, str_path + 1);
        amxc_var_cast(&objects, AMXC_VAR_ID_LIST);
        dmtui_dm_browser_load(amxtui_widget_get_ctrl(app->left), &objects);
        amxc_var_clean(&objects);
        handled = true;
    }

    return handled;
}

static void dmtui_screen_slot_select(UNUSED const char* const sig_name,
                                     const amxc_var_t* const data,
                                     UNUSED void* const priv) {
    dmtui_app_t* app = dmtui();
    const char* str_path = NULL;
    amxc_var_t paths;
    amxd_path_t path;
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxtui_ctrl_t* ctrl = NULL;

    amxd_path_init(&path, NULL);
    amxc_var_init(&paths);

    when_false(amxc_var_type_of(data) == AMXC_VAR_ID_CSTRING, exit);

    str_path = GET_CHAR(data, NULL);
    when_str_empty(str_path, exit);

    when_true(dmtui_start_game(app, str_path), exit);
    when_true(dmtui_add_objects(app, str_path), exit);

    amxd_path_setf(&path, false, "%s", str_path);
    when_false(amxd_path_get_type(&path) == amxd_path_search, exit);
    when_false(dmtui_dm_browser_has_root(amxtui_widget_get_ctrl(app->left), &path), exit);
    bus_ctx = amxb_be_who_has(str_path);
    when_null(bus_ctx, exit);

    amxb_resolve(bus_ctx, &path, &paths);
    ctrl = amxtui_widget_get_ctrl(app->right_bottom);
    amxtui_ctrl_lb_clear(ctrl);
    amxc_var_for_each(p, &paths) {
        amxtui_ctrl_lb_add(ctrl, GET_CHAR(p, NULL));
    }
    amxtui_lb_widget_update(app->right_bottom);
    amxtui_screen_set_focus(app->right_bottom);

exit:
    amxc_var_clean(&paths);
    amxd_path_clean(&path);
    return;
}

static void dmtui_screen_slot_update_dm_tree(UNUSED const char* const sig_name,
                                             const amxc_var_t* const data,
                                             void* const priv) {
    const char* str_path = NULL;
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(widget);

    when_false(amxc_var_type_of(data) == AMXC_VAR_ID_CSTRING, exit);

    str_path = GET_CHAR(data, NULL);
    dmtui_dm_browser_select_referenced(widget, ctrl, str_path, true);

exit:
    return;
}

static void dmtui_screen_slot_dm_loaded(UNUSED const char* const sig_name,
                                        UNUSED const amxc_var_t* const data,
                                        UNUSED void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    dmtui_dm_browser_widget_update(widget);
}

static void dmtui_screen_slot_expand_attrs(UNUSED const char* const sig_name,
                                           UNUSED const amxc_var_t* const data,
                                           UNUSED void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(widget);
    amxtui_tree_node_t* node = amxtui_ctrl_tb_root(ctrl);
    amxtui_widget_set_title(widget, "Meta Information");
    node = amxtui_tree_node_get(node, "attributes");
    amxtui_tb_widget_expand_node(widget, node);
    amxtui_widget_redraw(widget);
}

void dmtui_create_screen_controller(dmtui_app_t* app) {
    amxtui_ctrl_init(&app->ctrl_main,
                     NULL,
                     dmtui_screen_handle_ctrl_key,
                     NULL,
                     NULL);
    amxtui_screen_set_controller(&app->ctrl_main);

    amxp_sigmngr_add_signal(&app->ctrl_main.sigmngr, "select_object");

    amxtui_widget_connect(app->bottom, app->left, CTRL_SIG_IB_UPDATE, dmtui_screen_slot_update);
    amxtui_widget_connect(app->bottom, app->left, CTRL_SIG_IB_SELECT, dmtui_screen_slot_select);
    amxtui_widget_connect(app->right_bottom, app->left, CTRL_SIG_LB_SEL_CHANGED, dmtui_screen_slot_update_dm_tree);
    amxp_slot_connect(&app->ctrl_main.sigmngr, "select_object", NULL, dmtui_dm_browser_slot_select, app->left);
    amxp_slot_connect(&app->ctrl_snake->sigmngr, CTRL_SIG_S_GAME_OVER, NULL, dmtui_screen_slot_reset_right, NULL);
    amxtui_widget_connect(app->left, app->bottom, CTRL_SIG_DM_SEL_CHANGED, amxtui_ib_widget_slot_set_text);
    amxtui_widget_connect(app->left, app->right_top, CTRL_SIG_DM_OBJECT_INFO, amxtui_vv_widget_slot_set_data);
    amxtui_widget_connect(app->left, app->left, CTRL_SIG_DM_LOAD_DONE, dmtui_screen_slot_dm_loaded);
    amxtui_widget_connect(app->left, app->right_top, CTRL_SIG_DM_OBJECT_INFO, dmtui_screen_slot_expand_attrs);
}

void dmtui_delete_screen_controller(dmtui_app_t* app) {
    amxtui_ctrl_clean(&app->ctrl_main);
}