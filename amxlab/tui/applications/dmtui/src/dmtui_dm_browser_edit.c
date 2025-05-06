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

static void dmtui_dm_browser_stop_edit(const char* const sig_name,
                                       const amxc_var_t* const data,
                                       void* const priv);

static void dmtui_dm_browser_cancel_edit(const char* const sig_name,
                                         const amxc_var_t* const data,
                                         void* const priv);


static bool dmtui_dm_browser_node_is_function(UNUSED amxtui_tree_node_t* node, amxc_var_t* data) {
    amxc_var_t* arguments = GET_ARG(data, "arguments");

    return (arguments != NULL);
}

static void dmtui_dm_call_done(UNUSED const amxb_bus_ctx_t* bus_ctx,
                               amxb_request_t* req,
                               int status,
                               void* priv) {
    amxc_string_t message;
    dmtui_app_t* app = dmtui();
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(app->right_top);
    amxc_var_t* ret = NULL;

    amxc_string_init(&message, 0);
    if(req == NULL) {
        ret = (amxc_var_t*) priv;
    } else {
        ret = req->result;
    }

    if(status != 0) {
        amxc_string_setf(&message, "Invoke failed (%d).", status);
        amxtui_screen_show_message(amxc_string_get(&message, 0),
                                   NULL,
                                   amxtui_message_error,
                                   amxtui_allign_center);
    } else {
        amxtui_widget_set_title(app->right_top, "Return Data");
        amxtui_ctrl_vv_set_data(ctrl, ret);
        amxtui_widget_redraw(app->right_top);
    }
    amxc_string_clean(&message);
}

static int dmtui_dm_invoke_function(amxtui_tree_node_t* node,
                                    amxc_var_t* args) {
    int rv = 0;
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxc_var_t* node_data = amxtui_tree_node_get_data(node);
    amxc_string_t path;
    amxc_var_t ret;

    amxc_string_init(&path, 0);
    amxc_var_init(&ret);

    amxtui_tree_node_path(amxtui_tree_node_parent(node), &path, ".", false);

    bus_ctx = amxb_be_who_has(amxc_string_get(&path, 0));
    when_null(bus_ctx, exit);

    if(GETP_BOOL(node_data, "attributes.asynchronous")) {
        amxb_async_call(bus_ctx, amxc_string_get(&path, 0), GET_CHAR(node_data, "name"), args, dmtui_dm_call_done, NULL);
    } else {
        rv = amxb_call(bus_ctx, amxc_string_get(&path, 0), GET_CHAR(node_data, "name"), args, &ret, 20);
        dmtui_dm_call_done(NULL, NULL, rv, &ret);
    }

exit:
    amxc_string_clean(&path);
    amxc_var_clean(&ret);
    return rv;
}

static int dmtui_dm_apply_value(amxtui_tree_node_t* node,
                                amxc_var_t* params) {
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxc_var_t* node_data = amxtui_tree_node_get_data(node);
    amxc_string_t path;
    amxc_var_t ret;
    char* str_index = NULL;
    int rv = 0;
    amxc_string_t message;

    amxc_string_init(&path, 0);
    amxc_string_init(&message, 0);
    amxc_var_init(&ret);

    amxtui_tree_node_path(node, &path, ".", false);

    bus_ctx = amxb_be_who_has(amxc_string_get(&path, 0));
    when_null(bus_ctx, exit);
    if(amxtui_tree_node_has_children(node) &&
       (GET_UINT32(node_data, "type_id") == amxd_object_template)) {
        // add a new instance and set the parameters values
        rv = amxb_add(bus_ctx, amxc_string_get(&path, 0), 0, NULL, params, &ret, 20);
        amxc_string_setf(&message, "Add instance failed (%d).", rv);
    } else {
        // set multiple values in an existing (instance/singleton) object
        rv = amxb_set(bus_ctx, amxc_string_get(&path, 0), params, &ret, 20);
        amxc_string_setf(&message, "Set value(s) failed (%d).", rv);
    }

    dmtui_dm_browser_update_node(node, 0);
    str_index = amxc_var_dyncast(cstring_t, GETP_ARG(&ret, "0.index"));
    if(str_index != NULL) {
        node = amxtui_tree_node_get(node, str_index);
        dmtui_dm_browser_update_node(node, 0);
    }

    if(rv != 0) {
        amxtui_screen_show_message(amxc_string_get(&message, 0),
                                   NULL,
                                   amxtui_message_error,
                                   amxtui_allign_center);
    }
    free(str_index);

exit:
    amxc_string_clean(&message);
    amxc_string_clean(&path);
    amxc_var_clean(&ret);
    return rv;
}

static void dmtui_dm_browser_cancel_edit(UNUSED const char* const sig_name,
                                         UNUSED const amxc_var_t* const data,
                                         void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(widget);
    dmtui_dm_browser_t* dmbctrl = NULL;

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);

    amxtui_screen_hide(dmbctrl->input_box);
    amxtui_screen_set_focus(widget);
    amxtui_widget_remove_button(dmbctrl->input_box, "Apply");
    amxtui_widget_remove_button(dmbctrl->input_box, "Cancel");
    amxp_slot_disconnect_all(dmtui_dm_browser_stop_edit);
    amxp_slot_disconnect_all(dmtui_dm_browser_cancel_edit);

    amxtui_widget_redraw(widget);
}

static void dmtui_dm_browser_stop_edit(const char* const sig_name,
                                       const amxc_var_t* const data,
                                       void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(widget);
    dmtui_dm_browser_t* dmbctrl = NULL;
    amxtui_tree_node_t* node = amxtui_tb_widget_get_selected_node(widget);
    amxc_var_t* node_data = amxtui_tree_node_get_data(node);
    amxc_var_t* params = NULL;

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    dmbctrl = amxc_container_of(ctrl, dmtui_dm_browser_t, ctrl);

    amxtui_screen_hide(dmbctrl->input_box);
    amxtui_screen_set_focus(widget);
    amxtui_widget_remove_button(dmbctrl->input_box, "Apply");
    amxtui_widget_remove_button(dmbctrl->input_box, "Cancel");
    amxp_slot_disconnect_all(dmtui_dm_browser_stop_edit);
    amxp_slot_disconnect_all(dmtui_dm_browser_cancel_edit);

    when_true_status(dmtui_dm_browser_is_cancel_button(sig_name, data),
                     exit,
                     dmtui_dm_browser_cancel_edit(sig_name, data, priv));

    if(dmtui_dm_browser_node_is_function(node, node_data)) {
        params = amxtui_ctrl_vv_take_data(amxtui_widget_get_ctrl(dmbctrl->input_box));
        dmtui_dm_invoke_function(node, params);
    } else if(amxtui_tree_node_has_children(node)) {
        amxc_var_t* alias = NULL;
        const char* str_alias = NULL;
        // take data from vv ctrl
        params = amxtui_ctrl_vv_take_data(amxtui_widget_get_ctrl(dmbctrl->input_box));
        // Alias parameters are a special case
        // when not set, let the plug-in choose a value, but never set an empty
        // value for alias parameters
        alias = GET_ARG(params, "Alias");
        str_alias = GET_CHAR(alias, NULL);
        if((str_alias == NULL) || (*str_alias == 0)) {
            amxc_var_delete(&alias);
        }
        dmtui_dm_apply_value(node, params);
    } else {
        // take value from ib ctrl
        amxc_var_t* value = NULL;
        node = amxtui_tree_node_parent(node);
        amxc_var_new(&params);
        amxc_var_set_type(params, AMXC_VAR_ID_HTABLE);
        value = amxc_var_add_new_key(params, GET_CHAR(node_data, "name"));
        amxc_var_copy(value, data);
        if(amxc_var_cast(value, GET_UINT32(node_data, "type_id")) != 0) {
            amxc_string_t message;
            amxc_string_init(&message, 0);
            amxc_string_setf(&message,
                             "Invalid value - Can not convert to %s.",
                             GET_CHAR(node_data, "type_name"));
            amxtui_screen_show_message(amxc_string_get(&message, 0),
                                       NULL,
                                       amxtui_message_error,
                                       amxtui_allign_center);
            amxc_string_clean(&message);
            goto exit;
        }
        dmtui_dm_apply_value(node, params);
    }


exit:
    amxtui_widget_redraw(widget);
    amxc_var_delete(&params);
    return;
}

static bool dmtui_dm_browser_edit_object(dmtui_dm_browser_t* dmbctrl,
                                         amxc_var_t* data) {
    amxtui_ctrl_t* vvctrl = NULL;
    amxc_var_t params;
    uint32_t object_type = GET_UINT32(data, "type_id");
    bool editing = false;

    when_true((object_type == amxd_object_template) &&
              GETP_BOOL(data, "attributes.read-only"), exit);

    amxc_var_init(&params);
    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_for_each(param, GET_ARG(data, "parameters")) {
        amxc_var_t* value = GET_ARG(param, "value");
        amxc_var_t* input = NULL;
        bool is_read_only = GETP_BOOL(param, "attributes.read-only");

        if(is_read_only && (object_type != amxd_object_template)) {
            continue;
        }
        input = amxc_var_add_new_key(&params, amxc_var_key(param));
        amxc_var_copy(input, value);
        amxc_var_cast(input, GET_UINT32(param, "type_id"));
    }

    if(amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, &params))) {
        goto exit;
    }

    // creating a controller will also take a reference
    amxtui_ctrl_new(&vvctrl, "variant-viewer");
    // setting a controller to a widget, will take a reference
    amxtui_widget_set_ctrl(dmbctrl->input_box, vvctrl);
    // set parameters
    amxtui_ctrl_vv_set_data(vvctrl, &params);
    amxtui_ctrl_vv_set_edit_color(vvctrl, AMXTUI_COLOR_BLUE_WHITE);

    if(object_type == amxd_object_template) {
        amxtui_widget_set_title(dmbctrl->input_box, "Add instance");
    } else {
        amxtui_widget_set_title(dmbctrl->input_box, "Set values");
    }
    amxtui_widget_set_box(dmbctrl->input_box, true);
    amxtui_widget_set_width(dmbctrl->input_box, 75, amxtui_percentage);
    amxtui_widget_set_height(dmbctrl->input_box, 10, amxtui_absolute);
    amxtui_widget_set_pos(dmbctrl->input_box,
                          amxtui_allign_center, amxtui_alligned,
                          amxtui_allign_center, amxtui_alligned);

    // will remove a reference
    amxtui_ctrl_delete(&vvctrl);
    editing = true;

exit:
    amxc_var_clean(&params);
    if(!editing) {
        amxtui_screen_hide(dmbctrl->input_box);
    }
    return editing;
}

static bool dmtui_dm_browser_set_arguments(dmtui_dm_browser_t* dmbctrl,
                                           amxtui_tree_node_t* node,
                                           amxc_var_t* data) {
    amxtui_ctrl_t* vvctrl = NULL;
    amxc_var_t args;
    bool editing = false;
    amxc_string_t title;

    amxc_string_init(&title, 0);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_for_each(arg, GET_ARG(data, "arguments")) {
        amxc_var_t* input = NULL;
        amxc_var_t* default_val = GET_ARG(arg, "default");
        bool is_in_arg = GETP_BOOL(arg, "attributes.in");
        if(is_in_arg) {
            input = amxc_var_add_new_key(&args, GET_CHAR(arg, "name"));
            if(default_val) {
                amxc_var_convert(input, default_val, GET_UINT32(arg, "type_id"));
            } else {
                amxc_var_set_type(input, GET_UINT32(arg, "type_id"));
            }
        }
    }

    if(amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, &args))) {
        dmtui_dm_invoke_function(node, &args);
        goto exit;
    }

    // creating a controller will also take a reference
    amxtui_ctrl_new(&vvctrl, "variant-viewer");
    // setting a controller to a widget, will take a reference
    amxtui_widget_set_ctrl(dmbctrl->input_box, vvctrl);
    // set parameters
    amxtui_ctrl_vv_set_data(vvctrl, &args);
    amxtui_ctrl_vv_set_edit_color(vvctrl, AMXTUI_COLOR_BLUE_WHITE);

    amxc_string_setf(&title, "Invoke %s()", GET_CHAR(data, "name"));
    amxtui_widget_set_title(dmbctrl->input_box, amxc_string_get(&title, 0));

    amxtui_widget_set_box(dmbctrl->input_box, true);
    amxtui_widget_set_width(dmbctrl->input_box, 75, amxtui_percentage);
    amxtui_widget_set_height(dmbctrl->input_box, 10, amxtui_absolute);
    amxtui_widget_set_pos(dmbctrl->input_box,
                          amxtui_allign_center, amxtui_alligned,
                          amxtui_allign_center, amxtui_alligned);

    // will remove a reference
    amxtui_ctrl_delete(&vvctrl);
    editing = true;

exit:
    amxc_var_clean(&args);
    if(!editing) {
        amxtui_screen_hide(dmbctrl->input_box);
    }

    amxc_string_clean(&title);
    return editing;
}

static bool dmtui_dm_browser_edit_value(dmtui_dm_browser_t* dmbctrl,
                                        amxc_var_t* data,
                                        int line,
                                        int col) {
    amxtui_ctrl_t* ibctrl = NULL;
    amxc_var_t* value = GET_ARG(data, "value");
    char* txt = amxc_var_dyncast(cstring_t, value);
    bool editing = false;

    when_true(GETP_BOOL(data, "attributes.read-only"), exit);

    // creating a controller will also take a reference
    amxtui_ctrl_new(&ibctrl, "input-box");
    // setting a controller to a widget, will take a reference
    amxtui_widget_set_ctrl(dmbctrl->input_box, ibctrl);

    amxtui_widget_set_box(dmbctrl->input_box, false);
    amxtui_widget_set_width(dmbctrl->input_box, 0, amxtui_maximized);
    amxtui_widget_set_height(dmbctrl->input_box, 1, amxtui_absolute);
    amxtui_widget_set_pos(dmbctrl->input_box,
                          col, amxtui_absolute,
                          line, amxtui_absolute);

    amxtui_ctrl_ib_set_text(ibctrl, txt);

    // will remove a reference
    amxtui_ctrl_delete(&ibctrl);
    editing = true;

exit:
    if(!editing) {
        amxtui_screen_hide(dmbctrl->input_box);
    }
    free(txt);
    return editing;
}

void dmtui_dm_browser_start_edit(amxtui_widget_t* widget,
                                 dmtui_dm_browser_t* dmbctrl,
                                 amxtui_tree_node_t* node) {
    int line = amxtui_tb_widget_get_selected_line(widget);
    int col = amxtui_tb_widget_get_selected_data_col(widget);
    amxc_var_t* data = amxtui_tree_node_get_data(node);

    // add edit box widget to this widget
    amxtui_widget_add_widget(widget, dmbctrl->input_box);
    amxtui_screen_show(dmbctrl->input_box);
    amxtui_widget_add_button(dmbctrl->input_box, "Apply");
    amxtui_widget_add_button(dmbctrl->input_box, "Cancel");

    if(dmtui_dm_browser_node_is_function(node, data)) {
        when_false(dmtui_dm_browser_set_arguments(dmbctrl, node, data), exit);
    } else if(amxtui_tree_node_has_children(node)) {
        when_false(dmtui_dm_browser_edit_object(dmbctrl, data), exit);
    } else {
        when_false(dmtui_dm_browser_edit_value(dmbctrl, data, line, col), exit);
    }

    amxtui_widget_connect(dmbctrl->input_box,
                          widget,
                          CTRL_SIG_IB_SELECT,
                          dmtui_dm_browser_stop_edit);
    amxtui_widget_connect(dmbctrl->input_box,
                          widget,
                          CTRL_SIG_BUTTON,
                          dmtui_dm_browser_stop_edit);
    amxtui_widget_connect(dmbctrl->input_box,
                          widget,
                          CTRL_SIG_IB_CANCEL,
                          dmtui_dm_browser_cancel_edit);
    amxtui_widget_connect(dmbctrl->input_box,
                          widget,
                          CTRL_SIG_VV_CANCEL,
                          dmtui_dm_browser_cancel_edit);

    amxtui_screen_set_focus(dmbctrl->input_box);

exit:
    return;
}
