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

static void amxtui_dm_browser_select_node(dmtui_dm_browser_t* dmbctrl,
                                          amxtui_tree_node_t* node) {
    amxc_var_t* node_data = NULL;
    when_null(node, exit);
    node_data = amxtui_tree_node_get_data(node);
    if((node_data == NULL) ||
       amxtui_tree_node_has_children(node)) {
        dmtui_dm_browser_update_node(node, 0);
        node_data = amxtui_tree_node_get_data(node);
    }

    amxtui_ctrl_emit_data(&dmbctrl->ctrl,
                          CTRL_SIG_DM_OBJECT_INFO,
                          node_data);

exit:
    return;
}

static void dmtui_dm_browser_slot_sel_changed(UNUSED const char* const sig_name,
                                              const amxc_var_t* const data,
                                              void* const priv) {
    dmtui_dm_browser_t* dmbctrl = (dmtui_dm_browser_t*) priv;
    amxtui_tree_node_t* node = amxtui_ctrl_tb_get_node(&dmbctrl->ctrl, data);
    amxc_string_t path;

    amxc_string_init(&path, 0);
    amxtui_tree_node_path(node, &path, ".", false);
    amxtui_dm_browser_select_node(dmbctrl, node);
    amxtui_ctrl_trigger_text(&dmbctrl->ctrl,
                             CTRL_SIG_DM_SEL_CHANGED,
                             amxc_string_get(&path, 0));

    amxc_string_clean(&path);
}

static void dmtui_dm_browser_slot_expand_node(UNUSED const char* const sig_name,
                                              const amxc_var_t* const data,
                                              void* const priv) {
    dmtui_dm_browser_t* dmbctrl = (dmtui_dm_browser_t*) priv;
    amxtui_tree_node_t* node = amxtui_ctrl_tb_get_node(&dmbctrl->ctrl, data);
    amxc_var_t* node_data = amxtui_tree_node_get_data(node);

    if((node_data == NULL) ||
       (GET_UINT32(node_data, "type_id") == amxd_object_template)) {
        dmtui_dm_browser_update_node(node, 1);
    } else {
        dmtui_dm_browser_update_node(node, 0);
    }
}

static void amxtui_dm_browser_delete_instance(amxtui_widget_t* widget,
                                              amxtui_tree_node_t* node) {
    amxc_var_t* node_data = NULL;
    amxtui_tree_node_t* parent = amxtui_tree_node_parent(node);
    amxc_string_t path;
    amxc_var_t ret;
    amxb_bus_ctx_t* bus_ctx = NULL;
    int rv = 0;

    amxc_string_init(&path, 0);
    amxc_var_init(&ret);

    node_data = amxtui_tree_node_get_data(parent);
    when_true(GETP_BOOL(node_data, "attributes.read-only"), exit);
    node_data = amxtui_tree_node_get_data(node);
    amxtui_tree_node_path(parent, &path, ".", false);
    bus_ctx = amxb_be_who_has(amxc_string_get(&path, 0));
    when_null(bus_ctx, exit);
    rv = amxb_del(bus_ctx, amxc_string_get(&path, 0), GET_UINT32(node_data, "index"), NULL, &ret, 20);

    if(rv != 0) {
        amxtui_screen_show_message("Delete instance failed.",
                                   NULL,
                                   amxtui_message_error,
                                   amxtui_allign_center);
        goto exit;
    }

    amxtui_tb_widget_select_node(widget, parent);

    amxtui_tree_node_clear(parent);
    dmtui_dm_browser_update_node(parent, 1);

exit:
    amxc_string_clean(&path);
    amxc_var_clean(&ret);
    return;
}

static bool dmtui_dm_browser_handle_ctrl_key(amxtui_widget_t* widget,
                                             amxtui_ctrl_t* ctrl,
                                             uint32_t ctrl_key) {
    bool handled = false;
    amxtui_tree_node_t* node = NULL;
    dmtui_dm_browser_t* dmbctrl = NULL;
    amxc_var_t* node_data = NULL;

    node = amxtui_tb_widget_get_selected_node(widget);
    when_null(node, exit);
    node_data = amxtui_tree_node_get_data(node);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);

    switch(ctrl_key) {
    case amxt_key_right:
        if(!amxtui_tree_node_has_children(node)) {
            dmtui_dm_browser_follow_reference(widget, ctrl, node);
            handled = true;
        }
        break;
    case amxt_key_newline:
        dmtui_dm_browser_start_edit(widget, dmbctrl, node);
        handled = true;
        break;
    case amxt_key_insert:
        if(amxtui_tree_node_has_children(node) &&
           (GET_UINT32(node_data, "type_id") == amxd_object_template)) {
            dmtui_dm_browser_start_edit(widget, dmbctrl, node);
        }
        handled = true;
        break;
    case amxt_key_delete:
        if(amxtui_tree_node_has_children(node) &&
           (GET_UINT32(node_data, "type_id") == amxd_object_instance)) {
            amxtui_dm_browser_delete_instance(widget, node);
        }
        break;
    case amxt_key_backspace: {
        amxc_llist_it_t* it = amxc_lstack_pop(&dmbctrl->ostack);
        amxc_string_t* path = NULL;
        if(it != NULL) {
            path = amxc_string_from_llist_it(it);
            dmtui_dm_browser_select_referenced(widget, ctrl, amxc_string_get(path, 0), true);
        }
        amxc_string_delete(&path);
    }
    break;
    default:
        break;
    }
    when_true(handled, exit);

    ctrl = amxtui_ctrl_get_type(ctrl, "tree-box");
    handled = ctrl->handle_ctrl_key(widget, ctrl, ctrl_key);

exit:
    return handled;
}

static int dmtui_dm_browser_new(amxtui_ctrl_t** ctrl) {
    dmtui_dm_browser_t* dmbctrl = NULL;
    when_null(ctrl, exit);

    dmbctrl = (dmtui_dm_browser_t*) calloc(1, sizeof(dmtui_dm_browser_t));
    amxtui_ctrl_inherits(&dmbctrl->ctrl, "tree-box");
    dmbctrl->ctrl.handle_ctrl_key = dmtui_dm_browser_handle_ctrl_key;
    amxc_lstack_init(&dmbctrl->ostack);

    // create the helper widget, leave the controller empty
    // depending on the need a new controller is created and set
    // the width and height will be changed on the need
    amxtui_widget_new(&dmbctrl->input_box,
                      NULL,
                      1,
                      amxtui_absolute,
                      1,
                      amxtui_absolute);

    amxtui_widget_set_modal(dmbctrl->input_box, true);
    amxtui_screen_hide(dmbctrl->input_box);
    amxtui_widget_set_color(dmbctrl->input_box, amxtui_print_normal, AMXTUI_COLOR_BLACK_CYAN);
    amxtui_widget_set_color(dmbctrl->input_box, amxtui_print_focused, AMXTUI_COLOR_BLACK_CYAN);
    amxtui_widget_set_color(dmbctrl->input_box, amxtui_print_data, AMXTUI_COLOR_MAGENTA_CYAN);

    amxtui_ctrl_add_signal(&dmbctrl->ctrl, CTRL_SIG_DM_SEL_CHANGED);
    amxtui_ctrl_add_signal(&dmbctrl->ctrl, CTRL_SIG_DM_OBJECT_INFO);
    amxtui_ctrl_add_signal(&dmbctrl->ctrl, CTRL_SIG_DM_LOAD_DONE);

    // connect to tree-box signals
    amxp_slot_connect(&dmbctrl->ctrl.base->sigmngr,
                      CTRL_SIG_TB_SEL_CHANGED,
                      NULL,
                      dmtui_dm_browser_slot_sel_changed,
                      dmbctrl);
    amxp_slot_connect(&dmbctrl->ctrl.base->sigmngr,
                      CTRL_SIG_TB_EXPAND,
                      NULL,
                      dmtui_dm_browser_slot_expand_node,
                      dmbctrl);

    *ctrl = &dmbctrl->ctrl;

exit:
    return 0;
}

static int dmtui_dm_browser_delete(amxtui_ctrl_t** ctrl) {
    dmtui_dm_browser_t* dmbctrl = NULL;
    when_null(ctrl, exit);
    when_null(*ctrl, exit);
    when_false(strcmp(amxtui_ctrl_type_name(*ctrl), CTRL_NAME) == 0, exit);

    dmbctrl = amxc_container_of((*ctrl), dmtui_dm_browser_t, ctrl);
    amxp_slot_disconnect_with_priv(&dmbctrl->ctrl.base->sigmngr, NULL, dmbctrl);
    amxtui_widget_delete(&dmbctrl->input_box);
    amxc_lstack_clean(&dmbctrl->ostack, amxc_string_list_it_free);

    amxtui_ctrl_delete(&dmbctrl->ctrl.base);
    amxtui_ctrl_clean(&dmbctrl->ctrl);
    free(dmbctrl);

exit:
    return 0;
}

static amxtui_ctrl_type_t amxtui_ctrl_dm_browser = {
    .hit = { NULL, NULL, NULL },
    .ctor = dmtui_dm_browser_new,
    .dtor = dmtui_dm_browser_delete
};

CONSTRUCTOR_LVL(102) static void amxtui_ctrl_types_init(void) {
    amxtui_ctrl_type_register(&amxtui_ctrl_dm_browser, CTRL_NAME);
}

DESTRUCTOR_LVL(102) static void amxtui_ctrl_types_remove(void) {
    amxc_htable_it_clean(&amxtui_ctrl_dm_browser.hit, NULL);
}

int dmtui_dm_browser_load(amxtui_ctrl_t* ctrl, amxc_var_t* objects) {
    int rv = -1;
    dmtui_dm_browser_t* dmbctrl = NULL;

    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);

    if((objects == NULL) || (amxc_var_type_of(objects) != AMXC_VAR_ID_LIST)) {
        dmtui_dm_browser_discover_objects(dmbctrl);
    } else {
        dmtui_dm_browser_add_objects(dmbctrl, objects);
        amxtui_tree_node_sort(amxtui_ctrl_tb_root(&dmbctrl->ctrl));
        amxtui_ctrl_emit_data(&dmbctrl->ctrl, CTRL_SIG_REDRAW, NULL);
        amxtui_ctrl_emit_data(&dmbctrl->ctrl, CTRL_SIG_DM_LOAD_DONE, NULL);
    }

    rv = 0;
exit:
    return rv;
}

bool dmtui_dm_browser_has_root(amxtui_ctrl_t* ctrl, amxd_path_t* path) {
    bool rv = false;
    amxtui_tree_node_t* node = NULL;
    dmtui_dm_browser_t* dmbctrl = NULL;
    char* name = NULL;

    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);
    node = amxtui_ctrl_tb_root(&dmbctrl->ctrl);
    name = amxd_path_get_first(path, false);
    when_str_empty(name, exit);
    name[strlen(name) - 1] = 0;

    node = amxtui_tree_node_get(node, name);

    rv = node == NULL? false:true;
exit:
    free(name);
    return rv;
}

void dmtui_dm_browser_slot_select(UNUSED const char* const sig_name,
                                  const amxc_var_t* const data,
                                  void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    amxtui_tree_node_t* node = NULL;
    amxtui_tree_node_t* selected_node = NULL;
    const char* path = NULL;
    amxc_string_t sel_path;
    amxtui_ctrl_t* ctrl = NULL;
    dmtui_dm_browser_t* dmbctrl = NULL;

    amxc_string_init(&sel_path, 0);
    when_false(amxc_var_type_of(data) == AMXC_VAR_ID_CSTRING, exit);
    when_null(widget, exit);

    ctrl = amxtui_widget_get_ctrl(widget);
    path = GET_CHAR(data, NULL);
    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);

    selected_node = amxtui_tb_widget_get_selected_node(widget);
    amxtui_tree_node_path(selected_node, &sel_path, ".", false);
    if(strncmp(amxc_string_get(&sel_path, 0), path, amxc_string_text_length(&sel_path)) != 0) {
        dmtui_dm_browser_select_referenced(widget, ctrl, path, false);
    }
    node = dmtui_dm_browser_select(widget, path);
    amxtui_dm_browser_select_node(dmbctrl, node);

    amxtui_widget_redraw(widget);

exit:
    amxc_string_clean(&sel_path);
    return;
}

amxtui_tree_node_t* dmtui_dm_browser_select(amxtui_widget_t* widget,
                                            const char* path) {
    amxtui_ctrl_t* ctrl = NULL;
    dmtui_dm_browser_t* dmbctrl = NULL;
    amxtui_tree_node_t* selected = NULL;

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    when_failed(amxtui_tb_widget_select_path(widget, path, '.'), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);

    selected = amxtui_tb_widget_get_selected_node(widget);
    amxtui_ctrl_emit_data(&dmbctrl->ctrl,
                          CTRL_SIG_DM_OBJECT_INFO,
                          amxtui_tree_node_get_data(selected));

exit:
    return selected;
}

bool dmtui_dm_browser_is_cancel_button(const char* const sig_name,
                                       const amxc_var_t* const data) {
    bool cancel = false;
    if(strcmp(sig_name, CTRL_SIG_BUTTON) == 0) {
        // if cancel button selected
        if(strcmp(GET_CHAR(data, NULL), "Cancel") == 0) {
            cancel = true;
        }
    }

    return cancel;
}

int dmtui_dm_browser_widget_update(amxtui_widget_t* widget) {
    int rv = -1;
    amxtui_ctrl_t* ctrl = NULL;
    dmtui_dm_browser_t* dmbctrl = NULL;
    amxtui_tree_node_t* selected = NULL;

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);

    selected = amxtui_tb_widget_get_selected_node(widget);
    dmtui_dm_browser_update_node(selected, 0);
    amxtui_ctrl_emit_data(&dmbctrl->ctrl,
                          CTRL_SIG_DM_OBJECT_INFO,
                          amxtui_tree_node_get_data(selected));

    rv = 0;
exit:
    return rv;
}