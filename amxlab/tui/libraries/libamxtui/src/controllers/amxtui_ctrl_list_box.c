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
#include <amxtui/ctrl/amxtui_ctrl_list_box.h>

#define CTRL_NAME        "list-box"

#define WD_TOP_ITEM      "top"
#define WD_SELECTED_ITEM "selected"
#define WD_SELECTED_LINE "line"
#define WD_ALLIGN_ITEM   "allign"
#define WD_FILTER        "filter"

typedef struct _amxtui_ctrl_list_box {
    amxc_array_t items;
    amxtui_ctrl_t ctrl;
} amxtui_ctrl_list_box_t;

static amxc_string_t* amxtui_ctrl_lb_get(amxtui_ctrl_list_box_t* ctrl,
                                         uint32_t index) {
    amxc_string_t* txt =
        (amxc_string_t*) amxc_array_get_data_at(&ctrl->items, index);

    return txt;
}

static uint32_t amxtui_ctrl_lb_last_index(amxtui_ctrl_list_box_t* ctrl) {
    amxc_array_it_t* ait = amxc_array_get_last(&ctrl->items);
    return amxc_array_it_index(ait);
}

static void amxtui_ctrl_lb_free_item(amxc_array_it_t* it) {
    amxc_string_t* txt = (amxc_string_t*) amxc_array_it_get_data(it);
    amxc_string_delete(&txt);
}

static bool amxtui_ctrl_lb_is_valid(amxtui_ctrl_list_box_t* ctrl,
                                    uint32_t index,
                                    const char* filter) {
    amxc_string_t* txt = amxtui_ctrl_lb_get(ctrl, index);
    size_t filter_len = filter == NULL? 0:strlen(filter);
    return filter_len == 0 ||
           strncmp(amxc_string_get(txt, 0), filter, filter_len) == 0;
}

static uint32_t amxtui_ctrl_lb_first(amxtui_ctrl_list_box_t* ctrl,
                                     const char* filter) {
    uint32_t last_index = amxtui_ctrl_lb_last_index(ctrl);
    uint32_t current = 0;

    while(current < last_index &&
          !amxtui_ctrl_lb_is_valid(ctrl, current, filter)) {
        current++;
    }

    return current;
}

static uint32_t amxtui_ctrl_lb_last(amxtui_ctrl_list_box_t* ctrl,
                                    const char* filter) {
    uint32_t current = amxtui_ctrl_lb_last_index(ctrl);

    while(current > 0 &&
          !amxtui_ctrl_lb_is_valid(ctrl, current, filter)) {
        current--;
    }

    return current;
}

static uint32_t amxtui_ctrl_lb_next(amxtui_ctrl_list_box_t* ctrl,
                                    uint32_t index,
                                    const char* filter) {
    uint32_t last_index = amxtui_ctrl_lb_last_index(ctrl);
    uint32_t current = index + 1;

    if(current > last_index) {
        current = last_index;
    }
    while(current < last_index &&
          !amxtui_ctrl_lb_is_valid(ctrl, current, filter)) {
        current++;
    }

    if(!amxtui_ctrl_lb_is_valid(ctrl, current, filter)) {
        current = index;
    }
    return current;
}

static uint32_t amxtui_ctrl_lb_prev(amxtui_ctrl_list_box_t* ctrl,
                                    uint32_t index,
                                    const char* filter) {
    uint32_t current = index == 0? 0:index - 1;

    while(current > 0 &&
          !amxtui_ctrl_lb_is_valid(ctrl, current, filter)) {
        current--;
    }

    if(!amxtui_ctrl_lb_is_valid(ctrl, current, filter)) {
        current = index;
    }

    return current;
}

static uint32_t amxtui_ctrl_lb_page_up(amxtui_ctrl_list_box_t* ctrl,
                                       uint32_t index,
                                       const char* filter,
                                       int lines) {
    uint32_t current = index;

    index = amxtui_ctrl_lb_prev(ctrl, index, filter);
    while(current != index && lines > 0) {
        current = index;
        index = amxtui_ctrl_lb_prev(ctrl, index, filter);
        lines--;
    }

    return current;
}

static uint32_t amxtui_ctrl_lb_page_down(amxtui_ctrl_list_box_t* ctrl,
                                         uint32_t index,
                                         const char* filter,
                                         int lines) {
    uint32_t current = index;

    index = amxtui_ctrl_lb_next(ctrl, index, filter);
    while(current != index && lines > 0) {
        current = index;
        index = amxtui_ctrl_lb_next(ctrl, index, filter);
        lines--;
    }

    return current;
}

static void amxtui_ctrl_lb_show(const amxtui_widget_t* widget,
                                amxtui_ctrl_t* ctrl) {
    amxtui_ctrl_list_box_t* lbctrl = NULL;
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* selected_line = GET_ARG(widget_data, WD_SELECTED_LINE);
    uint32_t selected = GET_UINT32(widget_data, WD_SELECTED_ITEM);
    amxtui_alligment_t allign = (amxtui_alligment_t) GET_UINT32(widget_data, WD_ALLIGN_ITEM);
    int line = GET_INT32(selected_line, NULL);
    const char* filter = GET_CHAR(widget_data, WD_FILTER);
    size_t height = amxtui_widget_height(widget) - 1;
    uint32_t index = selected;
    uint32_t current = 0;

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    lbctrl = amxc_container_of(ctrl, amxtui_ctrl_list_box_t, ctrl);

    if(amxc_array_is_empty(&lbctrl->items)) {
        return;
    }

    if(line >= (int) height) {
        line = height;
    }
    while(line > 0) {
        current = index;
        index = amxtui_ctrl_lb_prev(lbctrl, index, filter);
        if(index == current) {
            break;
        }
        line--;
    }
    line = 0;

    while(amxtui_ctrl_lb_is_valid(lbctrl, index, filter)) {
        amxc_string_t* txt = amxtui_ctrl_lb_get(lbctrl, index);
        amxtui_print_type_t type = amxtui_print_normal;
        int rv = 0;
        if(index == selected) {
            amxc_var_set(int32_t, selected_line, line);
            type = amxtui_print_selected;
        }
        rv = amxtui_widget_print(widget,
                                 line,
                                 allign,
                                 type,
                                 txt,
                                 0);
        if(rv != 0) {
            break;
        }
        line++;
        current = index;
        index = amxtui_ctrl_lb_next(lbctrl, current, filter);
        if(index == current) {
            break;
        }
    }

}

static bool amxtui_ctrl_lb_handle_ctrl_key(amxtui_widget_t* widget,
                                           amxtui_ctrl_t* ctrl,
                                           uint32_t ctrl_key) {
    amxtui_ctrl_list_box_t* lbctrl = NULL;
    bool handled = true;
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* selected_item = GET_ARG(widget_data, WD_SELECTED_ITEM);
    amxc_var_t* selected_line = GET_ARG(widget_data, WD_SELECTED_LINE);
    uint32_t selected = GET_UINT32(selected_item, NULL);
    int32_t line = GET_INT32(selected_line, NULL);
    const char* filter = GET_CHAR(widget_data, WD_FILTER);
    size_t height = amxtui_widget_height(widget);
    uint32_t current = selected;

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    lbctrl = amxc_container_of(ctrl, amxtui_ctrl_list_box_t, ctrl);

    switch(ctrl_key) {
    case amxt_key_newline: {
        amxc_string_t* txt = amxtui_ctrl_lb_get(lbctrl, selected);
        amxtui_ctrl_emit_text(&lbctrl->ctrl,
                              CTRL_SIG_LB_SELECT,
                              amxc_string_get(txt, 0));
    }
    break;
    case amxt_key_home:
        selected = amxtui_ctrl_lb_first(lbctrl, filter);
        break;
    case amxt_key_end:
        selected = amxtui_ctrl_lb_last(lbctrl, filter);
        line = height - 1;
        break;
    case amxt_key_down:
        selected = amxtui_ctrl_lb_next(lbctrl, current, filter);
        if(selected != current) {
            line++;
        }
        break;
    case amxt_key_up:
        selected = amxtui_ctrl_lb_prev(lbctrl, current, filter);
        if(selected != current) {
            line--;
        }
        break;
    case amxt_key_page_down:
        selected = amxtui_ctrl_lb_page_down(lbctrl,
                                            selected,
                                            filter,
                                            height);
        break;
    case amxt_key_page_up:
        selected = amxtui_ctrl_lb_page_up(lbctrl,
                                          selected,
                                          filter,
                                          height);
        break;
    default:
        handled = false;
        break;
    }

    amxc_var_set(uint32_t, selected_item, selected);
    amxc_var_set(int32_t, selected_line, line);

    if(selected != current) {
        amxc_string_t* txt = amxtui_ctrl_lb_get(lbctrl, selected);
        amxtui_ctrl_emit_text(&lbctrl->ctrl,
                              CTRL_SIG_LB_SEL_CHANGED,
                              amxc_string_get(txt, 0));
    }

    if(handled) {
        amxtui_widget_redraw(widget);
    }

    return handled;
}

static bool amxtui_ctrl_lb_handle_key(UNUSED amxtui_widget_t* widget,
                                      amxtui_ctrl_t* ctrl,
                                      char key) {
    bool handled = false;
    amxtui_ctrl_list_box_t* lbctrl = NULL;

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    lbctrl = amxc_container_of(ctrl, amxtui_ctrl_list_box_t, ctrl);

    if(key == 0x1b) {
        amxtui_ctrl_emit_data(&lbctrl->ctrl, CTRL_SIG_LB_CANCEL, NULL);
        handled = true;
    }
    when_true(handled, exit);

exit:
    return handled;
}

static amxc_var_t* amxtui_ctrl_lb_get_data(UNUSED const amxtui_widget_t* widget,
                                           UNUSED amxtui_ctrl_t* ctrl) {
    amxc_var_t* widget_data = NULL;

    amxc_var_new(&widget_data);
    amxc_var_set_type(widget_data, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, widget_data, WD_SELECTED_ITEM, 0);
    amxc_var_add_key(int32_t, widget_data, WD_SELECTED_LINE, 0);
    amxc_var_add_key(cstring_t, widget_data, WD_FILTER, "");
    amxc_var_add_key(uint32_t, widget_data, WD_ALLIGN_ITEM, amxtui_allign_center);

    return widget_data;
}

static int amxtui_ctrl_new_list_box(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_list_box_t* lbctrl = NULL;
    when_null(ctrl, exit);

    lbctrl = (amxtui_ctrl_list_box_t*) calloc(1, sizeof(amxtui_ctrl_list_box_t));
    amxc_array_init(&lbctrl->items, 10);

    amxtui_ctrl_init(&lbctrl->ctrl,
                     amxtui_ctrl_lb_show,
                     amxtui_ctrl_lb_handle_ctrl_key,
                     amxtui_ctrl_lb_handle_key,
                     amxtui_ctrl_lb_get_data);

    amxtui_ctrl_add_signal(&lbctrl->ctrl, CTRL_SIG_LB_SELECT);
    amxtui_ctrl_add_signal(&lbctrl->ctrl, CTRL_SIG_LB_CANCEL);
    amxtui_ctrl_add_signal(&lbctrl->ctrl, CTRL_SIG_LB_SEL_CHANGED);

    *ctrl = &lbctrl->ctrl;
exit:
    return 0;
}

static int amxtui_ctrl_delete_list_box(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_list_box_t* lbctrl = NULL;
    when_null(ctrl, exit);
    when_null(*ctrl, exit);
    when_false(strcmp(amxtui_ctrl_type_name(*ctrl), CTRL_NAME) == 0, exit);

    lbctrl = amxc_container_of((*ctrl), amxtui_ctrl_list_box_t, ctrl);
    amxtui_ctrl_clean(&lbctrl->ctrl);
    amxc_array_clean(&lbctrl->items, amxtui_ctrl_lb_free_item);
    free(lbctrl);

exit:
    return 0;
}

static amxtui_ctrl_type_t amxtui_ctrl_list_box = {
    .hit = { NULL, NULL, NULL },
    .ctor = amxtui_ctrl_new_list_box,
    .dtor = amxtui_ctrl_delete_list_box
};

CONSTRUCTOR_LVL(102) static void amxtui_ctrl_types_init(void) {
    amxtui_ctrl_type_register(&amxtui_ctrl_list_box, CTRL_NAME);
}

int amxtui_ctrl_lb_add(amxtui_ctrl_t* ctrl, const char* txt) {
    int rv = -1;
    amxtui_ctrl_list_box_t* lbctrl = NULL;
    amxc_string_t* str = NULL;
    when_null(ctrl, exit);
    when_null(txt, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    lbctrl = amxc_container_of(ctrl, amxtui_ctrl_list_box_t, ctrl);
    amxc_string_new(&str, 0);
    amxc_string_set(str, txt);

    amxc_array_append_data(&lbctrl->items, str);
    rv = 0;
exit:
    return rv;
}

int amxtui_ctrl_lb_clear(amxtui_ctrl_t* ctrl) {
    int rv = -1;
    amxtui_ctrl_list_box_t* lbctrl = NULL;
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    lbctrl = amxc_container_of(ctrl, amxtui_ctrl_list_box_t, ctrl);
    amxc_array_clean(&lbctrl->items, amxtui_ctrl_lb_free_item);
    rv = 0;

exit:
    return rv;
}

void amxtui_lb_widget_slot_set_filter(UNUSED const char* const sig_name,
                                      const amxc_var_t* const data,
                                      void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    const char* filter = NULL;
    when_false(amxc_var_type_of(data) == AMXC_VAR_ID_CSTRING, exit);

    filter = GET_CHAR(data, NULL);

    if(amxtui_lb_widget_set_filter(widget, filter) == 0) {
        amxtui_widget_redraw(widget);
    }

exit:
    return;
}

int amxtui_lb_widget_set_filter(amxtui_widget_t* widget,
                                const char* filter) {
    int rv = -1;
    amxtui_ctrl_t* ctrl = NULL;
    amxtui_ctrl_list_box_t* lbctrl = NULL;
    uint32_t selected = 0;

    // fetch widget data
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* item_filter = GET_ARG(widget_data, WD_FILTER);
    amxc_var_t* selected_line = GET_ARG(widget_data, WD_SELECTED_LINE);
    amxc_var_t* selected_item = GET_ARG(widget_data, WD_SELECTED_ITEM);

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    lbctrl = amxc_container_of(ctrl, amxtui_ctrl_list_box_t, ctrl);

    selected = amxtui_ctrl_lb_first(lbctrl, filter);

    // update widget data
    amxc_var_set(cstring_t, item_filter, filter == NULL? "":filter);
    amxc_var_set(uint32_t, selected_item, selected);
    amxc_var_set(int32_t, selected_line, 0);

    rv = 0;

exit:
    return rv;
}

int amxtui_lb_widget_allign(amxtui_widget_t* widget,
                            amxtui_alligment_t allign) {
    int rv = -1;
    amxtui_ctrl_t* ctrl = NULL;

    // fetch widget data
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* allign_item = GET_ARG(widget_data, WD_ALLIGN_ITEM);

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);
    when_false(allign == amxtui_allign_center ||
               allign == amxtui_allign_left ||
               allign == amxtui_allign_right, exit);

    // update widget data
    amxc_var_set(uint32_t, allign_item, allign);

    rv = 0;

exit:
    return rv;
}

int amxtui_lb_widget_update(amxtui_widget_t* widget) {
    int rv = -1;
    amxtui_ctrl_t* ctrl = NULL;
    amxc_string_t* txt = NULL;
    amxtui_ctrl_list_box_t* lbctrl = NULL;
    uint32_t selected = 0;

    // fetch widget data
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    selected = GET_UINT32(widget_data, WD_SELECTED_ITEM);

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    lbctrl = amxc_container_of(ctrl, amxtui_ctrl_list_box_t, ctrl);

    when_true(amxc_array_is_empty(&lbctrl->items), exit);
    if(selected > amxc_array_size(&lbctrl->items)) {
        selected = 0;
    }
    txt = amxtui_ctrl_lb_get(lbctrl, selected);
    amxtui_ctrl_emit_text(&lbctrl->ctrl,
                          CTRL_SIG_LB_SEL_CHANGED,
                          amxc_string_get(txt, 0));

    rv = 0;
exit:
    return rv;
}