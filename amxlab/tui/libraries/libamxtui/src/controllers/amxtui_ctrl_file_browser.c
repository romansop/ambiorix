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
#include <amxtui/ctrl/amxtui_ctrl_file_browser.h>

#define CTRL_NAME        "file-browser"

typedef struct _amxtui_ctrl_file_browser {
    amxc_string_t dir;
    amxtui_ctrl_t ctrl;
} amxtui_ctrl_file_browser_t;

typedef struct _scan_data {
    amxtui_ctrl_file_browser_t* fbctrl;
    amxtui_tree_node_t* node;
} scan_data_t;

static int amxtui_ctrl_fb_fill_node(const char* name, void* priv) {
    scan_data_t* data = (scan_data_t*) priv;
    const char* base_name = strrchr(name, '/') + 1;
    amxtui_tree_node_t* node = amxtui_ctrl_tb_add_node(&data->fbctrl->ctrl,
                                                       data->node,
                                                       base_name,
                                                       NULL);

    amxtui_tree_node_set_has_children(node, !amxp_dir_is_empty(name));

    return 0;
}

static void amxtui_ctrl_fb_expand_node(UNUSED const char* const sig_name,
                                       const amxc_var_t* const data,
                                       void* const priv) {
    amxtui_ctrl_file_browser_t* fbctrl = (amxtui_ctrl_file_browser_t*) priv;
    amxtui_tree_node_t* node = amxtui_ctrl_tb_get_node(&fbctrl->ctrl, data);
    amxc_string_t path;
    scan_data_t scan_data;

    amxtui_tree_node_clear(node);

    amxc_string_init(&path, 0);

    amxtui_tree_node_path(node, &path, "/", false);
    amxc_string_prepend(&path, "/", 1);

    scan_data.fbctrl = fbctrl;
    scan_data.node = node;
    amxp_dir_scan(amxc_string_get(&path, 0),
                  NULL,
                  false,
                  amxtui_ctrl_fb_fill_node,
                  &scan_data);
    amxtui_tree_node_sort(node);

    amxc_string_clean(&path);
    amxtui_ctrl_trigger_data(&fbctrl->ctrl, CTRL_SIG_REDRAW, NULL);
}

static void amxtui_ctrl_fb_sel_changed(UNUSED const char* const sig_name,
                                       const amxc_var_t* const data,
                                       void* const priv) {
    amxtui_ctrl_file_browser_t* fbctrl = (amxtui_ctrl_file_browser_t*) priv;
    amxtui_tree_node_t* node = amxtui_ctrl_tb_get_node(&fbctrl->ctrl, data);
    amxc_string_t path;

    amxc_string_init(&path, 0);
    amxtui_tree_node_path(node, &path, "/", false);
    amxc_string_prepend(&path, "/", 1);

    amxtui_ctrl_trigger_text(&fbctrl->ctrl, CTRL_SIG_FB_SEL_CHANGED, amxc_string_get(&path, 0));

    amxc_string_clean(&path);
}

static int amxtui_ctrl_new_file_browser(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_file_browser_t* fbctrl = NULL;
    when_null(ctrl, exit);

    fbctrl = (amxtui_ctrl_file_browser_t*) calloc(1, sizeof(amxtui_ctrl_file_browser_t));
    amxtui_ctrl_inherits(&fbctrl->ctrl, "tree-box");
    amxc_string_init(&fbctrl->dir, 0);
    amxc_string_set(&fbctrl->dir, "/");

    amxtui_ctrl_add_signal(&fbctrl->ctrl, CTRL_SIG_FB_SEL_CHANGED);

    amxp_slot_connect(&fbctrl->ctrl.base->sigmngr,
                      CTRL_SIG_TB_EXPAND,
                      NULL,
                      amxtui_ctrl_fb_expand_node,
                      fbctrl);
    amxp_slot_connect(&fbctrl->ctrl.base->sigmngr,
                      CTRL_SIG_TB_SEL_CHANGED,
                      NULL,
                      amxtui_ctrl_fb_sel_changed,
                      fbctrl);

    *ctrl = &fbctrl->ctrl;

exit:
    return 0;
}

static int amxtui_ctrl_delete_file_browser(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_file_browser_t* fbctrl = NULL;
    when_null(ctrl, exit);
    when_null(*ctrl, exit);
    when_false(strcmp(amxtui_ctrl_type_name(*ctrl), CTRL_NAME) == 0, exit);

    fbctrl = amxc_container_of((*ctrl), amxtui_ctrl_file_browser_t, ctrl);
    amxp_slot_disconnect_with_priv(&fbctrl->ctrl.base->sigmngr, NULL, fbctrl);

    amxtui_ctrl_delete(&fbctrl->ctrl.base);
    amxtui_ctrl_clean(&fbctrl->ctrl);
    amxc_string_clean(&fbctrl->dir);
    free(fbctrl);

exit:
    return 0;
}

static amxtui_ctrl_type_t amxtui_ctrl_file_browser = {
    .hit = { NULL, NULL, NULL },
    .ctor = amxtui_ctrl_new_file_browser,
    .dtor = amxtui_ctrl_delete_file_browser
};

CONSTRUCTOR_LVL(102) static void amxtui_ctrl_types_init(void) {
    amxtui_ctrl_type_register(&amxtui_ctrl_file_browser, CTRL_NAME);
}


int amxtui_ctrl_fb_scan(amxtui_ctrl_t* ctrl, const char* dir) {
    int rv = -1;
    amxtui_ctrl_file_browser_t* fbctrl = NULL;
    scan_data_t scan_data;

    when_null(ctrl, exit);
    when_null(dir, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    fbctrl = amxc_container_of(ctrl, amxtui_ctrl_file_browser_t, ctrl);
    amxc_string_set(&fbctrl->dir, "/");

    scan_data.fbctrl = fbctrl;
    scan_data.node = amxtui_ctrl_tb_root(ctrl); // uses top node
    amxp_dir_scan(amxc_string_get(&fbctrl->dir, 0),
                  NULL,
                  false,
                  amxtui_ctrl_fb_fill_node, &scan_data);
    amxtui_tree_node_sort(amxtui_ctrl_tb_root(&fbctrl->ctrl));

    rv = 0;
exit:
    return rv;
}

void amxtui_fb_widget_slot_select(UNUSED const char* const sig_name,
                                  const amxc_var_t* const data,
                                  void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    const char* path = NULL;

    when_false(amxc_var_type_of(data) == AMXC_VAR_ID_CSTRING, exit);

    path = GET_CHAR(data, NULL);

    if(amxtui_fb_widget_select(widget, path) == 0) {
        amxtui_widget_redraw(widget);
    }

exit:
    return;
}

int amxtui_fb_widget_select(amxtui_widget_t* widget,
                            const char* path) {
    int rv = -1;
    amxtui_ctrl_t* ctrl = NULL;

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    rv = amxtui_tb_widget_select_path(widget, path, '/');

exit:
    return rv;
}
