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

#include <amxp/amxp_dir.h>

#include <amxtui/amxtui.h>
#include <amxtui/ctrl/amxtui_ctrl_tree_box.h>
#include <amxtui/ctrl/amxtui_ctrl_input_box.h>
#include <amxtui/ctrl/amxtui_ctrl_variant_viewer.h>

#define CTRL_NAME        "variant-viewer"

typedef struct _amxtui_ctrl_variant_viewer {
    amxc_var_t* data;
    amxtui_ctrl_t ctrl;
    amxtui_widget_t* input_box;
    bool editable;
    uint32_t edit_color;
} amxtui_ctrl_variant_viewer_t;

static void amxtui_vv_widget_cancel_edit(const char* const sig_name,
                                         const amxc_var_t* const data,
                                         void* const priv);

static void amxtui_vv_build_tree(amxtui_ctrl_t* ctrl,
                                 amxtui_tree_node_t* node,
                                 amxc_var_t* data) {
    amxc_string_t name;
    uint32_t index = 0;
    amxtui_tree_node_t* subnode = NULL;
    amxc_string_init(&name, 0);

    amxc_var_for_each(var, data) {
        const char* key = amxc_var_key(var);
        if(key == NULL) {
            amxc_string_setf(&name, "%d", index);
            key = amxc_string_get(&name, 0);
        }
        subnode = amxtui_ctrl_tb_add_node(ctrl, node, key, var);
        if((amxc_var_type_of(var) == AMXC_VAR_ID_LIST) ||
           (amxc_var_type_of(var) == AMXC_VAR_ID_HTABLE)) {
            amxtui_vv_build_tree(ctrl, subnode, var);
        }
        index++;
    }

    amxtui_tree_node_sort(node);
    amxc_string_clean(&name);
}

static void amxtui_ctrl_vv_sel_changed(UNUSED const char* const sig_name,
                                       const amxc_var_t* const data,
                                       void* const priv) {
    amxtui_ctrl_variant_viewer_t* vvctrl = (amxtui_ctrl_variant_viewer_t*) priv;
    amxtui_tree_node_t* node = amxtui_ctrl_tb_get_node(&vvctrl->ctrl, data);
    amxc_string_t path;

    amxc_string_init(&path, 0);
    amxtui_tree_node_path(node, &path, ".", true);

    amxtui_ctrl_trigger_text(&vvctrl->ctrl,
                             CTRL_SIG_VV_SEL_CHANGED,
                             amxc_string_get(&path, 0));

    amxc_string_clean(&path);
}

static void amxtui_vv_widget_stop_edit(UNUSED const char* const sig_name,
                                       const amxc_var_t* const data,
                                       void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(widget);
    amxtui_ctrl_variant_viewer_t* vvctrl = NULL;
    amxtui_tree_node_t* node = NULL;
    amxc_var_t* node_data = NULL;
    amxc_var_t tmp;
    uint32_t data_type = AMXC_VAR_ID_ANY;

    amxc_var_init(&tmp);
    node = amxtui_tb_widget_get_selected_node(widget);
    node_data = amxtui_tree_node_get_data(node);
    data_type = amxc_var_type_of(node_data);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    vvctrl = amxc_container_of(ctrl, amxtui_ctrl_variant_viewer_t, ctrl);

    amxtui_screen_hide(vvctrl->input_box);
    amxtui_screen_set_focus(widget);
    amxp_slot_disconnect_all(amxtui_vv_widget_stop_edit);
    amxp_slot_disconnect_all(amxtui_vv_widget_cancel_edit);

    amxc_var_copy(&tmp, data);
    if(amxc_var_cast(&tmp, data_type) != 0) {
        amxc_string_t msg;
        amxc_string_init(&msg, 0);
        amxc_string_setf(&msg,
                         "Invalid value - Can not convert to %s",
                         amxc_var_type_name_of(node_data));
        amxtui_screen_show_message(amxc_string_get(&msg, 0),
                                   NULL,
                                   amxtui_message_error,
                                   amxtui_allign_center);
        amxc_string_clean(&msg);
        goto exit;
    } else {
        amxc_var_move(node_data, &tmp);
    }

    amxtui_widget_redraw(widget);

exit:
    amxc_var_clean(&tmp);
    return;
}

static void amxtui_vv_widget_cancel_edit(UNUSED const char* const sig_name,
                                         UNUSED const amxc_var_t* const data,
                                         void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(widget);
    amxtui_ctrl_variant_viewer_t* vvctrl = NULL;

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    vvctrl = amxc_container_of(ctrl, amxtui_ctrl_variant_viewer_t, ctrl);

    amxtui_screen_hide(vvctrl->input_box);
    amxtui_screen_set_focus(widget);
    amxp_slot_disconnect_all(amxtui_vv_widget_stop_edit);
    amxp_slot_disconnect_all(amxtui_vv_widget_cancel_edit);

    amxtui_widget_redraw(widget);
}

static void amxtui_vv_widget_start_edit(amxtui_widget_t* widget,
                                        amxtui_ctrl_variant_viewer_t* vvctrl,
                                        amxtui_tree_node_t* node) {
    int line = amxtui_tb_widget_get_selected_line(widget);
    int col = amxtui_tb_widget_get_selected_data_col(widget);
    amxc_var_t* data = amxtui_tree_node_get_data(node);
    char* txt = amxc_var_dyncast(cstring_t, data);
    amxtui_ctrl_t* ibctrl = amxtui_widget_get_ctrl(vvctrl->input_box);

    amxtui_widget_add_widget(widget, vvctrl->input_box);
    amxtui_widget_set_width(vvctrl->input_box, 0, amxtui_maximized);
    amxtui_widget_set_pos(vvctrl->input_box,
                          col,
                          amxtui_absolute,
                          line,
                          amxtui_absolute);
    amxtui_ctrl_ib_set_text(ibctrl, txt);
    amxtui_widget_set_color(vvctrl->input_box, amxtui_print_normal, vvctrl->edit_color);
    amxtui_widget_set_color(vvctrl->input_box, amxtui_print_focused, vvctrl->edit_color);
    amxtui_screen_show(vvctrl->input_box);
    amxtui_screen_set_focus(vvctrl->input_box);

    amxtui_widget_connect(vvctrl->input_box,
                          widget,
                          CTRL_SIG_IB_SELECT,
                          amxtui_vv_widget_stop_edit);
    amxtui_widget_connect(vvctrl->input_box,
                          widget,
                          CTRL_SIG_IB_CANCEL,
                          amxtui_vv_widget_cancel_edit);
    free(txt);
}

static bool amxtui_ctrl_vv_handle_ctrl_key(amxtui_widget_t* widget,
                                           amxtui_ctrl_t* ctrl,
                                           uint32_t ctrl_key) {
    bool handled = false;
    amxtui_ctrl_variant_viewer_t* vvctrl = NULL;
    amxtui_tree_node_t* node = NULL;

    node = amxtui_tb_widget_get_selected_node(widget);
    when_null(node, exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    vvctrl = amxc_container_of(ctrl, amxtui_ctrl_variant_viewer_t, ctrl);

    if(!amxtui_tree_node_has_children(node)) {
        switch(ctrl_key) {
        case amxt_key_newline:
            if(vvctrl->editable) {
                amxtui_vv_widget_start_edit(widget, vvctrl, node);
                handled = true;
            }
            break;
        default:
            break;
        }
    }
    when_true(handled, exit);

    ctrl = amxtui_ctrl_get_type(ctrl, "tree-box");
    handled = ctrl->handle_ctrl_key(widget, ctrl, ctrl_key);

exit:
    return handled;
}

static bool amxtui_ctrl_vv_handle_key(amxtui_widget_t* widget,
                                      amxtui_ctrl_t* ctrl,
                                      char key) {
    bool handled = false;
    amxtui_ctrl_variant_viewer_t* vvctrl = NULL;

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    vvctrl = amxc_container_of(ctrl, amxtui_ctrl_variant_viewer_t, ctrl);

    if(key == 0x1b) {
        amxtui_ctrl_emit_data(&vvctrl->ctrl, CTRL_SIG_VV_CANCEL, NULL);
        handled = true;
    }
    when_true(handled, exit);

    ctrl = amxtui_ctrl_get_type(ctrl, "tree-box");
    handled = ctrl->handle_key(widget, ctrl, key);

exit:
    return handled;
}

static int amxtui_ctrl_new_variant_viewer(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_variant_viewer_t* vvctrl = NULL;
    when_null(ctrl, exit);

    vvctrl = (amxtui_ctrl_variant_viewer_t*)
        calloc(1, sizeof(amxtui_ctrl_variant_viewer_t));
    amxtui_ctrl_inherits(&vvctrl->ctrl, "tree-box");
    vvctrl->ctrl.handle_ctrl_key = amxtui_ctrl_vv_handle_ctrl_key;
    vvctrl->ctrl.handle_key = amxtui_ctrl_vv_handle_key;
    vvctrl->editable = true;

    amxc_var_new(&vvctrl->data);
    amxtui_widget_new(&vvctrl->input_box,
                      "input-box",
                      10,
                      amxtui_absolute,
                      1,
                      amxtui_absolute);
    amxtui_widget_set_box(vvctrl->input_box, false);
    amxtui_widget_set_modal(vvctrl->input_box, true);
    vvctrl->edit_color = AMXTUI_COLOR_BLACK_CYAN;

    amxtui_ctrl_add_signal(&vvctrl->ctrl, CTRL_SIG_VV_SEL_CHANGED);
    amxtui_ctrl_add_signal(&vvctrl->ctrl, CTRL_SIG_VV_CANCEL);

    amxp_slot_connect(&vvctrl->ctrl.base->sigmngr,
                      CTRL_SIG_TB_SEL_CHANGED,
                      NULL,
                      amxtui_ctrl_vv_sel_changed,
                      vvctrl);
    *ctrl = &vvctrl->ctrl;

exit:
    return 0;
}

static int amxtui_ctrl_delete_variant_viewer(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_variant_viewer_t* vvctrl = NULL;
    when_null(ctrl, exit);
    when_null(*ctrl, exit);
    when_false(strcmp(amxtui_ctrl_type_name(*ctrl), CTRL_NAME) == 0, exit);

    vvctrl = amxc_container_of((*ctrl), amxtui_ctrl_variant_viewer_t, ctrl);
    amxp_slot_disconnect_with_priv(&vvctrl->ctrl.base->sigmngr, NULL, vvctrl);
    amxtui_widget_delete(&vvctrl->input_box);
    amxtui_ctrl_delete(&vvctrl->ctrl.base);
    amxtui_ctrl_clean(&vvctrl->ctrl);
    amxc_var_delete(&vvctrl->data);
    free(vvctrl);

exit:
    return 0;
}

static amxtui_ctrl_type_t amxtui_ctrl_variant_viewer = {
    .hit = { NULL, NULL, NULL },
    .ctor = amxtui_ctrl_new_variant_viewer,
    .dtor = amxtui_ctrl_delete_variant_viewer
};

CONSTRUCTOR_LVL(102) static void amxtui_ctrl_types_init(void) {
    amxtui_ctrl_type_register(&amxtui_ctrl_variant_viewer, CTRL_NAME);
}

int amxtui_ctrl_vv_editable(const amxtui_ctrl_t* ctrl, bool editable) {
    int rv = -1;
    amxtui_ctrl_variant_viewer_t* vvctrl = NULL;
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    vvctrl = amxc_container_of(ctrl, amxtui_ctrl_variant_viewer_t, ctrl);
    vvctrl->editable = editable;
    rv = 0;

exit:
    return rv;
}

const amxc_var_t* amxtui_ctrl_vv_get_data(const amxtui_ctrl_t* ctrl) {
    amxtui_ctrl_variant_viewer_t* vvctrl = NULL;
    amxc_var_t* data = NULL;
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    vvctrl = amxc_container_of(ctrl, amxtui_ctrl_variant_viewer_t, ctrl);
    data = vvctrl->data;

exit:
    return data;
}

amxc_var_t* amxtui_ctrl_vv_take_data(const amxtui_ctrl_t* ctrl) {
    amxtui_ctrl_variant_viewer_t* vvctrl = NULL;
    amxc_var_t* data = NULL;
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    vvctrl = amxc_container_of(ctrl, amxtui_ctrl_variant_viewer_t, ctrl);
    data = vvctrl->data;
    vvctrl->data = NULL;
    amxtui_ctrl_tb_clear_nodes(&vvctrl->ctrl,
                               amxtui_ctrl_tb_root(&vvctrl->ctrl));

exit:
    return data;
}

void amxtui_vv_widget_slot_set_data(UNUSED const char* const sig_name,
                                    const amxc_var_t* const data,
                                    void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(widget);

    if(amxtui_ctrl_vv_set_data(ctrl, data) == 0) {
        amxtui_ctrl_trigger_data(ctrl, CTRL_SIG_RESET, NULL);
    }

    return;
}

int amxtui_ctrl_vv_set_data(const amxtui_ctrl_t* ctrl,
                            const amxc_var_t* data) {
    int rv = -1;
    amxtui_ctrl_variant_viewer_t* vvctrl = NULL;
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    vvctrl = amxc_container_of(ctrl, amxtui_ctrl_variant_viewer_t, ctrl);
    amxtui_ctrl_tb_clear_nodes(&vvctrl->ctrl,
                               amxtui_ctrl_tb_root(&vvctrl->ctrl));
    amxc_var_delete(&vvctrl->data);

    if(data != NULL) {
        amxc_var_new(&vvctrl->data);
        amxc_var_copy(vvctrl->data, data);
        amxtui_vv_build_tree(&vvctrl->ctrl,
                             amxtui_ctrl_tb_root(&vvctrl->ctrl),
                             vvctrl->data);
    }

    rv = 0;
exit:
    return rv;
}

int amxtui_ctrl_vv_set_edit_color(const amxtui_ctrl_t* ctrl,
                                  uint32_t color) {
    int rv = -1;
    amxtui_ctrl_variant_viewer_t* vvctrl = NULL;
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    vvctrl = amxc_container_of(ctrl, amxtui_ctrl_variant_viewer_t, ctrl);

    vvctrl->edit_color = color;

    rv = 0;
exit:
    return rv;
}

void amxtui_vv_widget_slot_select(UNUSED const char* const sig_name,
                                  const amxc_var_t* const data,
                                  void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    const char* path = NULL;

    when_false(amxc_var_type_of(data) == AMXC_VAR_ID_CSTRING, exit);

    path = GET_CHAR(data, NULL);

    if(amxtui_vv_widget_select(widget, path) == 0) {
        amxtui_widget_redraw(widget);
    }

exit:
    return;
}

int amxtui_vv_widget_select(amxtui_widget_t* widget,
                            const char* path) {
    int rv = -1;
    amxtui_ctrl_t* ctrl = NULL;
    amxc_string_t p;

    amxc_string_init(&p, 0);

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    amxc_string_set(&p, path);
    rv = amxtui_tb_widget_select_path(widget, p.buffer, '.');

exit:
    amxc_string_clean(&p);
    return rv;
}
