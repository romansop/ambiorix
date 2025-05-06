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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <inttypes.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include "../mocks/mocks.h"

#include <amxtui/amxtui_widget.h>
#include <amxtui/amxtui_controller.h>
#include <amxtui/ctrl/amxtui_ctrl_variant_viewer.h>
#include <amxtui/ctrl/amxtui_ctrl_tree_box.h>

#include "../../include_priv/amxtui_priv.h"

#include "test_amxtui_ctrl_variant_viewer.h"

amxtui_ctrl_t ctrl_screen;

static void test_amxtui_ctrl_vv_set_data(amxtui_ctrl_t* ctrl) {
    amxc_var_t data;
    amxc_var_t* sub = NULL;
    const amxc_var_t* ctrl_data = NULL;
    int result = 0;

    amxc_var_init(&data);
    amxc_var_set_type(&data, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(bool, &data, "Field0", "Hello");
    amxc_var_add_key(cstring_t, &data, "Field1", "Hello");
    amxc_var_add_key(cstring_t, &data, "Field2", "World");
    amxc_var_add_key(uint32_t, &data, "Field3", 100);
    sub = amxc_var_add_key(amxc_llist_t, &data, "Field4", NULL);
    amxc_var_add(cstring_t, sub, "A");
    amxc_var_add(cstring_t, sub, "B");
    sub = amxc_var_add_key(amxc_htable_t, &data, "Field5", NULL);
    amxc_var_add_key(uint32_t, sub, "S1", 1);
    amxc_var_add_key(uint32_t, sub, "S2", 2);

    assert_int_equal(amxtui_ctrl_vv_set_data(ctrl, &data), 0);
    ctrl_data = amxtui_ctrl_vv_get_data(ctrl);
    assert_non_null(ctrl_data);
    assert_int_equal(amxc_var_compare(ctrl_data, &data, &result), 0);
    assert_int_equal(result, 0);

    amxc_var_clean(&data);
}

static void test_check_current_node(amxtui_widget_t* widget, const char* name, const char* value) {
    amxtui_tree_node_t* node = NULL;
    amxc_var_t* sub = NULL;

    node = amxtui_tb_widget_get_selected_node(widget);
    assert_non_null(node);
    sub = amxtui_tree_node_get_data(node);
    assert_non_null(sub);
    if(name != NULL) {
        assert_string_equal(name, amxc_var_key(sub));
    }
    assert_string_equal(value, GET_CHAR(sub, NULL));
}

static void test_check_current_node_int(amxtui_widget_t* widget, const char* name, uint32_t value) {
    amxtui_tree_node_t* node = NULL;
    amxc_var_t* sub = NULL;

    node = amxtui_tb_widget_get_selected_node(widget);
    assert_non_null(node);
    sub = amxtui_tree_node_get_data(node);
    assert_non_null(sub);
    assert_string_equal(name, amxc_var_key(sub));
    assert_int_equal(GET_UINT32(sub, NULL), value);
}

static void test_check_current_node_type(amxtui_widget_t* widget, const char* name, uint32_t type) {
    amxtui_tree_node_t* node = NULL;
    amxc_var_t* sub = NULL;

    node = amxtui_tb_widget_get_selected_node(widget);
    assert_non_null(node);
    sub = amxtui_tree_node_get_data(node);
    assert_non_null(sub);
    assert_string_equal(name, amxc_var_key(sub));
    assert_int_equal(amxc_var_type_of(sub), type);
}

void test_amxtui_ctrl_variant_viewer_create(UNUSED void** state) {
    amxtui_widget_t* widget1 = NULL;
    amxtui_widget_t* widget2 = NULL;
    amxtui_ctrl_t* ctrl = NULL;

    set_terminal_size(80, 50);
    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget1,
                      "variant-viewer",
                      100, amxtui_percentage,
                      100, amxtui_percentage);
    amxtui_widget_new(&widget2,
                      NULL,
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget1), 0);
    assert_int_equal(amxtui_screen_add_widget(widget2), 0);
    amxtui_screen_redraw();

    ctrl = amxtui_widget_get_ctrl(widget1);
    amxtui_widget_set_ctrl(widget2, ctrl);
    assert_non_null(ctrl);
    assert_true(amxtui_ctrl_is_type(ctrl, "variant-viewer"));
    assert_true(amxtui_ctrl_is_type(ctrl, "tree-box"));

    assert_int_equal(amxtui_widget_delete(&widget1), 0);
    amxtui_screen_redraw();

    ctrl = amxtui_widget_get_ctrl(widget2);
    assert_non_null(ctrl);
    assert_true(amxtui_ctrl_is_type(ctrl, "variant-viewer"));
    assert_true(amxtui_ctrl_is_type(ctrl, "tree-box"));

    assert_int_equal(amxtui_widget_delete(&widget2), 0);

    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_ctrl_variant_viewer_set_data(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;
    amxtui_ctrl_t* ctrl = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget,
                      "variant-viewer",
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);
    ctrl = amxtui_widget_get_ctrl(widget);
    assert_non_null(ctrl);
    test_amxtui_ctrl_vv_set_data(ctrl);
    amxtui_screen_redraw();

    test_key_down();
    test_check_current_node(widget, "Field1", "Hello");
    test_key_down();
    test_check_current_node(widget, "Field2", "World");
    test_key_enter();
    test_key_enter();
    test_check_current_node(widget, "Field2", "World");
    test_key_down();
    test_check_current_node_int(widget, "Field3", 100);
    test_key_toggle();
    test_check_current_node_int(widget, "Field3", 100);
    test_key_left();
    test_check_current_node_int(widget, "Field3", 100);
    test_key_down();
    test_check_current_node_type(widget, "Field4", AMXC_VAR_ID_LIST);
    test_key_toggle();
    test_check_current_node_type(widget, "Field4", AMXC_VAR_ID_LIST);
    test_key_down();
    test_check_current_node(widget, NULL, "A");
    test_key_down();
    test_check_current_node(widget, NULL, "B");
    test_key_left();
    test_check_current_node_type(widget, "Field4", AMXC_VAR_ID_LIST);
    test_key_down();
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);
    test_key_toggle();
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);
    test_key_down();
    test_check_current_node_int(widget, "S1", 1);
    test_key_down();
    test_check_current_node_int(widget, "S2", 2);
    test_key_up();
    test_check_current_node_int(widget, "S1", 1);
    test_key_up();
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);
    test_key_left();
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);
    test_key_right();
    test_check_current_node_int(widget, "S1", 1);
    test_key_home();
    test_check_current_node_type(widget, "Field0", AMXC_VAR_ID_BOOL);
    test_key_end();
    test_check_current_node_type(widget, "S2", AMXC_VAR_ID_UINT32);
    test_key_page_up();
    test_check_current_node_type(widget, "Field0", AMXC_VAR_ID_BOOL);
    test_key_page_down();
    test_check_current_node_type(widget, "S2", AMXC_VAR_ID_UINT32);
    test_key_up();
    test_key_up();
    test_key_up();
    test_key_right();
    test_key_down();
    test_key_down();
    test_key_up();
    test_key_up();
    test_key_up();
    test_key_up();
    test_key_up();
    test_check_current_node(widget, "Field2", "World");

    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_ctrl_variant_viewer_set_data_using_slot(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;
    amxtui_ctrl_t* ctrl = NULL;
    amxc_var_t* data = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget,
                      "variant-viewer",
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);
    ctrl = amxtui_widget_get_ctrl(widget);
    assert_non_null(ctrl);
    test_amxtui_ctrl_vv_set_data(ctrl);
    data = amxtui_ctrl_vv_take_data(ctrl);
    amxtui_vv_widget_slot_set_data(NULL, data, widget);
    amxc_var_delete(&data);

    amxtui_screen_redraw();

    test_key_down();
    test_check_current_node(widget, "Field1", "Hello");
    test_key_down();
    test_check_current_node(widget, "Field2", "World");
    test_key_enter();
    test_key_enter();
    test_check_current_node(widget, "Field2", "World");

    test_key_down();
    test_check_current_node_int(widget, "Field3", 100);
    test_key_down();
    test_check_current_node_type(widget, "Field4", AMXC_VAR_ID_LIST);
    test_key_toggle();
    test_check_current_node_type(widget, "Field4", AMXC_VAR_ID_LIST);
    test_key_down();
    test_check_current_node(widget, NULL, "A");
    test_key_down();
    test_check_current_node(widget, NULL, "B");
    test_key_left();
    test_check_current_node_type(widget, "Field4", AMXC_VAR_ID_LIST);
    test_key_down();
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);
    test_key_toggle();
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);
    test_key_down();
    test_check_current_node_int(widget, "S1", 1);
    test_key_down();
    test_check_current_node_int(widget, "S2", 2);
    test_key_up();
    test_check_current_node_int(widget, "S1", 1);
    test_key_up();
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);
    test_key_left();
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);
    test_key_right();
    test_check_current_node_int(widget, "S1", 1);
    test_key_home();
    test_check_current_node_type(widget, "Field0", AMXC_VAR_ID_BOOL);
    test_key_end();
    test_check_current_node_type(widget, "S2", AMXC_VAR_ID_UINT32);

    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_ctrl_variant_viewer_can_edit(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;
    amxtui_ctrl_t* ctrl = NULL;
    amxc_var_t* data = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget,
                      "variant-viewer",
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);
    ctrl = amxtui_widget_get_ctrl(widget);
    assert_non_null(ctrl);
    test_amxtui_ctrl_vv_set_data(ctrl);
    amxtui_screen_redraw();

    test_key_down();
    test_key_down();
    test_check_current_node(widget, "Field2", "World");
    test_key_enter();
    test_key_backspace();
    test_key_backspace();
    test_key_backspace();
    test_key_backspace();
    test_key_backspace();
    test_key("B");
    test_key("e");
    test_key("l");
    test_key("g");
    test_key("i");
    test_key("u");
    test_key("m");
    test_key_enter();
    test_check_current_node(widget, "Field2", "Belgium");

    data = amxtui_ctrl_vv_take_data(ctrl);
    amxc_var_dump(data, STDOUT_FILENO);
    amxtui_screen_redraw();
    assert_string_equal(GET_CHAR(data, "Field2"), "Belgium");

    amxc_var_delete(&data);

    amxc_var_delete(&data);
    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_ctrl_variant_viewer_can_disable_edit(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;
    amxtui_ctrl_t* ctrl = NULL;
    amxc_var_t* data = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget,
                      "variant-viewer",
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);
    ctrl = amxtui_widget_get_ctrl(widget);
    assert_non_null(ctrl);
    test_amxtui_ctrl_vv_set_data(ctrl);
    amxtui_screen_redraw();
    assert_int_equal(amxtui_ctrl_vv_editable(ctrl, false), 0);

    test_key_down();
    test_key_down();
    test_check_current_node(widget, "Field2", "World");
    test_key_enter();
    test_key_backspace();
    test_key_backspace();
    test_key_backspace();
    test_key_backspace();
    test_key_backspace();
    test_key("B");
    test_key("e");
    test_key("l");
    test_key("g");
    test_key("i");
    test_key("u");
    test_key("m");
    test_key_enter();
    test_check_current_node(widget, "Field2", "World");

    data = amxtui_ctrl_vv_take_data(ctrl);
    amxc_var_dump(data, STDOUT_FILENO);
    amxtui_screen_redraw();
    assert_string_equal(GET_CHAR(data, "Field2"), "World");

    amxc_var_delete(&data);
    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_ctrl_variant_viewer_edit_wrong_value(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;
    amxtui_ctrl_t* ctrl = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget,
                      "variant-viewer",
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);
    ctrl = amxtui_widget_get_ctrl(widget);
    assert_non_null(ctrl);
    test_amxtui_ctrl_vv_set_data(ctrl);
    amxtui_screen_redraw();

    test_key_enter();
    test_key_backspace();
    test_key_backspace();
    test_key_backspace();
    test_key_enter();
    test_key_enter();

    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_ctrl_variant_viewer_can_select(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;
    amxtui_ctrl_t* ctrl = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget,
                      "variant-viewer",
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);
    ctrl = amxtui_widget_get_ctrl(widget);
    assert_non_null(ctrl);
    test_amxtui_ctrl_vv_set_data(ctrl);
    amxtui_screen_redraw();

    assert_int_equal(amxtui_vv_widget_select(widget, "Field5"), 0);
    while(amxp_signal_read() == 0) {
        printf(".");
    }
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);

    test_key_toggle();
    test_key_down();
    test_check_current_node_int(widget, "S1", 1);
    test_key_down();
    test_check_current_node_int(widget, "S2", 2);

    test_key_left();
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxtui_vv_widget_select(widget, "Field5.S2"), 0);
    while(amxp_signal_read() == 0) {
        printf(".");
    }
    test_check_current_node_int(widget, "S2", 2);

    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_ctrl_variant_viewer_can_select_using_slot(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;
    amxtui_ctrl_t* ctrl = NULL;
    amxc_var_t path;

    amxc_var_init(&path);
    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget,
                      "variant-viewer",
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);
    ctrl = amxtui_widget_get_ctrl(widget);
    assert_non_null(ctrl);
    test_amxtui_ctrl_vv_set_data(ctrl);
    amxtui_screen_redraw();

    amxc_var_set(cstring_t, &path, "Field5");
    amxtui_vv_widget_slot_select(NULL, &path, widget);
    while(amxp_signal_read() == 0) {
        printf(".");
    }
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);

    test_key_toggle();
    test_key_down();
    test_check_current_node_int(widget, "S1", 1);
    test_key_down();
    test_check_current_node_int(widget, "S2", 2);

    test_key_left();
    test_check_current_node_type(widget, "Field5", AMXC_VAR_ID_HTABLE);
    amxc_var_set(cstring_t, &path, "Field5.S2");
    amxtui_vv_widget_slot_select(NULL, &path, widget);
    while(amxp_signal_read() == 0) {
        printf(".");
    }
    test_check_current_node_int(widget, "S2", 2);

    amxc_var_set(uint32_t, &path, 100);
    amxtui_vv_widget_slot_select(NULL, &path, widget);
    while(amxp_signal_read() == 0) {
        printf(".");
    }
    test_check_current_node_int(widget, "S2", 2);

    amxc_var_clean(&path);
    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_ctrl_variant_viewer_can_set_edit_color(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;
    amxtui_ctrl_t* ctrl = NULL;
    amxc_var_t path;

    amxc_var_init(&path);
    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget,
                      "variant-viewer",
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);
    ctrl = amxtui_widget_get_ctrl(widget);
    assert_non_null(ctrl);

    assert_int_equal(amxtui_ctrl_vv_set_edit_color(ctrl, AMXTUI_COLOR_MAGENTA_CYAN), 0);
    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_ctrl_check_node_has_children(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;
    amxtui_ctrl_t* ctrl = NULL;
    amxtui_tree_node_t* node = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget,
                      "variant-viewer",
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);
    ctrl = amxtui_widget_get_ctrl(widget);
    assert_non_null(ctrl);
    test_amxtui_ctrl_vv_set_data(ctrl);
    amxtui_screen_redraw();

    assert_int_equal(amxtui_vv_widget_select(widget, "Field5"), 0);
    while(amxp_signal_read() == 0) {
        printf(".");
    }

    node = amxtui_tb_widget_get_selected_node(widget);
    assert_true(amxtui_tree_node_has_children(node));

    assert_int_equal(amxtui_vv_widget_select(widget, "Field4"), 0);
    while(amxp_signal_read() == 0) {
        printf(".");
    }

    node = amxtui_tb_widget_get_selected_node(widget);
    assert_true(amxtui_tree_node_has_children(node));

    assert_int_equal(amxtui_vv_widget_select(widget, "Field2"), 0);
    while(amxp_signal_read() == 0) {
        printf(".");
    }

    node = amxtui_tb_widget_get_selected_node(widget);
    assert_false(amxtui_tree_node_has_children(node));

    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_ctrl_check_node_hierarchy(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;
    amxtui_ctrl_t* ctrl = NULL;
    amxtui_tree_node_t* node = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget,
                      "variant-viewer",
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);
    ctrl = amxtui_widget_get_ctrl(widget);
    assert_non_null(ctrl);
    test_amxtui_ctrl_vv_set_data(ctrl);
    amxtui_screen_redraw();

    assert_int_equal(amxtui_vv_widget_select(widget, "Field5"), 0);
    while(amxp_signal_read() == 0) {
        printf(".");
    }

    node = amxtui_tb_widget_get_selected_node(widget);
    node = amxtui_tree_node_get(node, "S2");
    assert_non_null(node);
    node = amxtui_tree_node_parent(node);
    assert_non_null(node);

    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}