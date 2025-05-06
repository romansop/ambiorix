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

static void dmtui_dm_browser_stop_select(const char* const sig_name,
                                         const amxc_var_t* const data,
                                         void* const priv);

static void dmtui_dm_browser_cancel_select(UNUSED const char* const sig_name,
                                           UNUSED const amxc_var_t* const data,
                                           void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(widget);
    dmtui_dm_browser_t* dmbctrl = NULL;

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);

    amxtui_screen_hide(dmbctrl->input_box);
    amxtui_screen_set_focus(widget);
    amxtui_widget_remove_button(dmbctrl->input_box, "Cancel");
    amxp_slot_disconnect_all(dmtui_dm_browser_stop_select);
    amxp_slot_disconnect_all(dmtui_dm_browser_cancel_select);

    amxtui_widget_redraw(widget);
}

static void dmtui_dm_browser_stop_select(UNUSED const char* const sig_name,
                                         UNUSED const amxc_var_t* const data,
                                         void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(widget);
    dmtui_dm_browser_t* dmbctrl = NULL;
    amxc_string_t* path = NULL;
    amxtui_tree_node_t* selected = amxtui_tb_widget_get_selected_node(widget);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);

    amxtui_screen_hide(dmbctrl->input_box);
    amxtui_screen_set_focus(widget);
    amxtui_widget_remove_button(dmbctrl->input_box, "Cancel");
    amxp_slot_disconnect_all(dmtui_dm_browser_stop_select);
    amxp_slot_disconnect_all(dmtui_dm_browser_cancel_select);

    when_true_status(dmtui_dm_browser_is_cancel_button(sig_name, data),
                     exit,
                     dmtui_dm_browser_cancel_select(sig_name, data, priv));

    dmtui_dm_browser_select_referenced(widget, &dmbctrl->ctrl, GET_CHAR(data, NULL), true);

    amxc_string_new(&path, 0);
    amxtui_tree_node_path(selected, path, ".", false);
    amxc_lstack_push(&dmbctrl->ostack, &path->it);

exit:
    amxtui_widget_redraw(widget);
}

static const char* dmtui_dm_browser_select_reference(amxtui_widget_t* widget,
                                                     dmtui_dm_browser_t* dmbctrl,
                                                     amxtui_tree_node_t* node) {
    amxc_var_t* node_data = amxtui_tree_node_get_data(node);
    uint32_t ref_type = GET_UINT32(node_data, "type_id");
    const char* ref_path = GET_CHAR(node_data, "value");
    amxtui_ctrl_t* lbctrl = NULL;
    amxc_var_t refs;
    amxc_var_init(&refs);

    if((ref_type == AMXC_VAR_ID_CSV_STRING) ||
       (ref_type == AMXC_VAR_ID_SSV_STRING)) {
        amxc_var_copy(&refs, GET_ARG(node_data, "value"));
        amxc_var_cast(&refs, AMXC_VAR_ID_LIST);
        when_true(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &refs)) < 2, exit)
    } else {
        goto exit;
    }

    ref_path = NULL;
    // add edit box widget to this widget
    amxtui_widget_add_widget(widget, dmbctrl->input_box);
    amxtui_screen_show(dmbctrl->input_box);
    amxtui_widget_add_button(dmbctrl->input_box, "Cancel");

    // creating a controller will also take a reference
    amxtui_ctrl_new(&lbctrl, "list-box");
    // setting a controller to a widget, will take a reference
    amxtui_widget_set_ctrl(dmbctrl->input_box, lbctrl);
    // fill list box
    amxc_var_for_each(ref, &refs) {
        amxtui_ctrl_lb_add(lbctrl, GET_CHAR(ref, NULL));
    }

    amxtui_widget_set_box(dmbctrl->input_box, true);
    amxtui_widget_set_width(dmbctrl->input_box, 75, amxtui_percentage);
    amxtui_widget_set_height(dmbctrl->input_box, 10, amxtui_absolute);
    amxtui_widget_set_pos(dmbctrl->input_box,
                          amxtui_allign_center, amxtui_alligned,
                          amxtui_allign_center, amxtui_alligned);


    amxtui_widget_connect(dmbctrl->input_box,
                          widget,
                          CTRL_SIG_BUTTON,
                          dmtui_dm_browser_stop_select);
    amxtui_widget_connect(dmbctrl->input_box,
                          widget,
                          CTRL_SIG_LB_CANCEL,
                          dmtui_dm_browser_cancel_select);
    amxtui_widget_connect(dmbctrl->input_box,
                          widget,
                          CTRL_SIG_LB_SELECT,
                          dmtui_dm_browser_stop_select);

    amxtui_screen_set_focus(dmbctrl->input_box);

exit:
    amxtui_ctrl_delete(&lbctrl);
    amxc_var_clean(&refs);
    return ref_path;
}

void dmtui_dm_browser_follow_reference(amxtui_widget_t* widget,
                                       amxtui_ctrl_t* ctrl,
                                       amxtui_tree_node_t* node) {
    dmtui_dm_browser_t* dmbctrl = NULL;
    const char* ref_path = NULL;
    amxc_string_t* path = NULL;
    amxtui_tree_node_t* selected = amxtui_tb_widget_get_selected_node(widget);

    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);
    ref_path = dmtui_dm_browser_select_reference(widget, dmbctrl, node);
    when_null(ref_path, exit);

    dmtui_dm_browser_select_referenced(widget, ctrl, ref_path, true);

    amxc_string_new(&path, 0);
    amxtui_tree_node_path(selected, path, ".", false);
    amxc_lstack_push(&dmbctrl->ostack, &path->it);

exit:
    return;
}

void dmtui_dm_browser_select_referenced(amxtui_widget_t* widget,
                                        amxtui_ctrl_t* ctrl,
                                        const char* ref_path,
                                        bool sel_changed) {
    amxc_llist_t parts;
    amxc_string_t path;
    dmtui_dm_browser_t* dmbctrl = NULL;
    const char* sep = "";
    amxtui_tree_node_t* ref_node = NULL;

    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);
    ref_node = amxtui_ctrl_tb_root(&dmbctrl->ctrl);

    amxc_llist_init(&parts);
    amxc_string_init(&path, 0);
    amxc_string_set(&path, ref_path);
    amxc_string_split_to_llist(&path, &parts, '.');

    amxc_string_reset(&path);
    amxc_llist_for_each(it, &parts) {
        amxc_string_t* part = amxc_string_from_llist_it(it);
        amxtui_tree_node_t* tmp = NULL;
        if(amxc_string_is_empty(part)) {
            break;
        }
        tmp = amxtui_tree_node_get(ref_node, amxc_string_get(part, 0));
        if(tmp == NULL) {
            break;
        }
        ref_node = tmp;
        amxc_string_appendf(&path, "%s%s", sep, amxc_string_get(part, 0));
        when_failed(dmtui_dm_browser_update_node(ref_node, 0), exit);
        amxtui_tb_widget_expand_node(widget, ref_node);
        amxtui_tb_widget_select_node(widget, ref_node);
        amxtui_widget_redraw(widget);
        sep = ".";
        amxc_string_delete(&part);
    }

    if(sel_changed) {
        amxtui_ctrl_emit_text(&dmbctrl->ctrl,
                              CTRL_SIG_DM_SEL_CHANGED,
                              amxc_string_get(&path, 0));
    }
    amxtui_ctrl_emit_data(&dmbctrl->ctrl,
                          CTRL_SIG_DM_OBJECT_INFO,
                          amxtui_tree_node_get_data(ref_node));

exit:
    amxc_llist_clean(&parts, amxc_string_list_it_free);
    amxc_string_clean(&path);
}
