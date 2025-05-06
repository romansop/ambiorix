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
#include <amxtui/ctrl/amxtui_ctrl_tree_box.h>

#define CTRL_NAME        "tree-box"

#define WD_SELECTED_LINE  "line"
#define WD_SELECTED_ITEM  "selected"
#define WD_EXPANDED_ITEMS "expanded"

#define NODE_IDENT  2

#define NODE(x) (amxtui_tree_node_t*) amxc_array_it_get_data(x)
#define NODE_AT(n, i) (amxtui_tree_node_t*) amxc_array_get_data_at(n, i)

struct _amxtui_tree_node {
    amxtui_tree_node_t* parent;     // the parent node
    uint32_t index;                 // index in parent array
    amxc_string_t txt;              // display text
    bool has_children;              // used for lazy loading
    amxc_var_t* data;               // node specific data - not displayed
    amxc_array_t nodes;             // array child nodes
};

typedef struct _amxtui_ctrl_tree_box {
    amxtui_tree_node_t top;
    amxtui_ctrl_t ctrl;
} amxtui_ctrl_tree_box_t;

static void amxtui_tree_clean_node(amxc_array_it_t* ait) {
    amxtui_tree_node_t* node = NODE(ait);
    amxc_array_clean(&node->nodes, amxtui_tree_clean_node);

    if(node->data != NULL) {
        if(amxc_var_get_parent(node->data) == NULL) {
            amxc_var_delete(&node->data);
        }
    }

    amxc_string_clean(&node->txt);
    free(node);
}

static amxtui_tree_node_t* amxtui_tree_get_node(amxc_array_t* nodes,
                                                int32_t level,
                                                const amxc_var_t* item) {
    amxtui_tree_node_t* node = NULL;
    amxc_array_it_t* ait = NULL;
    amxc_var_t* var_index = GETI_ARG(item, level);

    when_null(var_index, exit);
    ait = amxc_array_get_at(nodes, GET_UINT32(var_index, NULL));
    node = NODE(ait);
    node = amxtui_tree_get_node(&node->nodes, level + 1, item);
    if(node == NULL) {
        node = NODE(ait);
    }

exit:
    return node;
}

static int amxtui_tree_get_level(amxtui_tree_node_t* node) {
    int level = 0;
    while(node->parent != NULL && node->parent->parent != 0) {
        level++;
        node = node->parent;
    }

    return level;
}

static amxtui_tree_node_t* amxtui_tree_get_next(amxtui_tree_node_t* node,
                                                const amxtui_widget_t* widget) {
    uint32_t index = 0;
    amxtui_tree_node_t* current = NULL;
    when_null(node, exit);

    // if node has child nodes and is not collapsed go into the node
    if(!amxc_array_is_empty(&node->nodes)) {
        if(amxtui_tb_widget_node_is_collapsed(widget, node)) {
            // collapsed stay at the same level - next index
            index = node->index + 1;
        } else {
            // not collapsed go to next level - index = 0
            node = NODE_AT(&node->nodes, 0);
            index = 0;
        }
    } else {
        // no child nodes, stay at the same level - next index
        index = node->index + 1;
    }

    // check if index is in boundery
    current = node;
    while(index >= amxc_array_size(&node->parent->nodes)) {
        // index to high, go one level up - next index
        if(node->parent->parent == NULL) {
            node = current;
            goto exit;
        }
        node = node->parent;
        index = node->index + 1;
    }
    node = NODE_AT(&node->parent->nodes, index);

exit:
    return node;
}

static amxtui_tree_node_t* amxtui_tree_get_prev(amxtui_tree_node_t* node,
                                                const amxtui_widget_t* widget) {
    uint32_t index = 0;
    when_null(node, exit);

    index = node->index;
    if(index == 0) {
        when_null(node->parent->parent, exit);
        node = node->parent;
        index = node->index;
        node = NODE_AT(&node->parent->nodes, index);
    } else {
        index--;
        node = NODE_AT(&node->parent->nodes, index);
        while(!amxc_array_is_empty(&node->nodes) &&
              !amxtui_tb_widget_node_is_collapsed(widget, node)) {
            // not collapsed go to next level - index = nr items
            index = amxc_array_size(&node->nodes) - 1;
            node = NODE_AT(&node->nodes, index);
        }
    }

exit:
    return node;
}

static void amxtui_tree_build_index(amxtui_tree_node_t* node,
                                    amxc_var_t* item) {
    if((node != NULL) && (node->parent != NULL)) {
        amxtui_tree_build_index(node->parent, item);
        amxc_var_add(uint32_t, item, node->index);
    }
}

static bool amxtui_node_in_tree(amxtui_tree_node_t* top,
                                amxtui_tree_node_t* node) {
    bool retval = false;
    if(node->parent == NULL) {
        retval = (top == node);
    } else {
        retval = amxtui_node_in_tree(top, node->parent);
    }

    return retval;
}

static int amxtui_tree_node_cmp(amxc_array_it_t* it1, amxc_array_it_t* it2) {
    amxtui_tree_node_t* n1 = NODE(it1);
    amxtui_tree_node_t* n2 = NODE(it2);

    return strcmp(amxc_string_get(&n1->txt, 0), amxc_string_get(&n2->txt, 0));
}

static void amxtui_tree_node_update_expanded(amxc_var_t* expanded_items,
                                             amxtui_tree_node_t* node,
                                             bool collapse) {
    amxc_string_t key;
    const amxc_htable_t* tei = amxc_var_constcast(amxc_htable_t,
                                                  expanded_items);
    amxc_string_init(&key, 0);
    when_null(node, exit);
    amxc_string_setf(&key, "%p", (void*) node);

    if(collapse) {
        amxc_var_t* expanded = GET_ARG(expanded_items,
                                       amxc_string_get(&key, 0));
        amxc_array_it_t* ait = amxc_array_get_first(&node->nodes);
        amxc_var_delete(&expanded);
        when_true(amxc_htable_is_empty(tei), exit);
        while(ait != NULL) {
            node = (amxtui_tree_node_t*) amxc_array_it_get_data(ait);
            amxtui_tree_node_update_expanded(expanded_items, node, collapse);
            ait = amxc_array_it_get_next(ait);
        }
    } else {
        if(node->parent->parent != NULL) {
            amxtui_tree_node_update_expanded(expanded_items,
                                             node->parent,
                                             collapse);
        }
        amxc_var_add_key(bool, expanded_items, amxc_string_get(&key, 0), true);
    }

exit:
    amxc_string_clean(&key);
}

static int amxtui_tree_node_collapse(amxtui_ctrl_tree_box_t* tbctrl,
                                     amxtui_widget_t* widget,
                                     amxtui_tree_node_t* node,
                                     bool collapse) {
    amxc_var_t node_index;
    int rv = -1;
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* expanded_items = GET_ARG(widget_data, WD_EXPANDED_ITEMS);

    amxc_var_init(&node_index);
    amxc_var_set_type(&node_index, AMXC_VAR_ID_LIST);

    when_true(amxtui_tb_widget_node_is_collapsed(widget, node) == collapse,
              exit);

    if(amxtui_tree_node_has_children(node)) {
        amxtui_tree_build_index(node, &node_index);
        amxtui_tree_node_update_expanded(expanded_items, node, collapse);
        amxtui_ctrl_trigger_data(&tbctrl->ctrl,
                                 collapse? CTRL_SIG_TB_COLLAPSE:CTRL_SIG_TB_EXPAND,
                                 &node_index);
    }
    rv = 0;

exit:
    amxc_var_clean(&node_index);
    return rv;
}

static int amxtui_tree_decoration(const amxtui_widget_t* widget,
                                  amxtui_tree_node_t* node,
                                  int line,
                                  int level) {
    int rv = 0;
    //int tree_lvl = 0;
    amxc_string_t txt;
    amxc_string_init(&txt, 10);

    if(!amxc_array_is_empty(&node->nodes) || node->has_children) {
        if(amxtui_tb_widget_node_is_collapsed(widget, node)) {
            amxc_string_set(&txt, "+");
        } else {
            amxc_string_set(&txt, "-");
        }

        rv = amxtui_widget_print_at(widget,
                                    line, level * NODE_IDENT,
                                    amxtui_print_focused, &txt, 0);
    }
    amxc_string_clean(&txt);

    return rv;
}

static void amxtui_tree_data(const amxtui_widget_t* widget,
                             amxtui_tree_node_t* node,
                             int line,
                             int col) {
    char* data_txt = NULL;
    amxtui_print_type_t type = amxtui_print_data;
    amxc_var_t* value = NULL;
    amxc_string_t tmp;
    amxc_string_init(&tmp, 0);

    when_null(node->data, exit);
    value = node->data;

    if(amxc_var_type_of(node->data) == AMXC_VAR_ID_HTABLE) {
        value = GET_ARG(node->data, "value");
        if(value == NULL) {
            value = GET_ARG(node->data, "name");
            data_txt = amxc_var_dyncast(cstring_t, value);
            if((data_txt != NULL) &&
               (strncmp(data_txt, amxc_string_get(&node->txt, 0), strlen(data_txt)) == 0)) {
                free(data_txt);
                data_txt = NULL;
                goto exit;
            }
            free(data_txt);
            data_txt = NULL;
        }
        when_null(value, exit);
    }

    when_true(amxc_var_type_of(value) == AMXC_VAR_ID_LIST, exit);
    when_true(amxc_var_type_of(value) == AMXC_VAR_ID_HTABLE, exit);
    when_true(amxc_var_type_of(value) == AMXC_VAR_ID_NULL, exit);

    data_txt = amxc_var_dyncast(cstring_t, value);
    amxc_string_set(&tmp, ":");
    amxtui_widget_print_at(widget, line, col, type, &tmp, 0);
    amxc_string_set(&tmp, data_txt);
    amxtui_widget_print_at(widget, line, col + 3, type, &tmp, 0);

exit:
    free(data_txt);
    amxc_string_clean(&tmp);
}

static int amxtui_tree_data_longest_label(amxtui_tree_node_t* node) {
    int len = 0;
    amxc_array_it_t* ait = amxc_array_get_first(&node->nodes);

    while(ait != NULL) {
        amxtui_tree_node_t* n = NODE(ait);
        int txt_len = (int) amxc_string_text_length(&n->txt);
        if(txt_len > len) {
            len = txt_len;
        }
        ait = amxc_array_it_get_next(ait);
    }

    return len;
}

static int amxtui_tree_show(const amxtui_widget_t* widget,
                            amxtui_tree_node_t* top,
                            amxtui_tree_node_t* select,
                            int* selected_line) {
    int rv = 0;
    amxc_string_t txt;
    amxtui_tree_node_t* node = top;
    amxtui_tree_node_t* current = NULL;
    int line = 0;
    int col = 0;
    int longest_label = 0;
    int level = amxtui_tree_get_level(top);

    when_null(node, exit);

    amxc_string_init(&txt, 0);
    while(rv == 0 && node != NULL) {
        amxtui_print_type_t type = amxtui_print_normal;
        if(node == select) {
            type = amxtui_print_selected;
            *selected_line = line;
        }
        rv = amxtui_tree_decoration(widget, node, line, level);
        if(rv != 0) {
            break;
        }
        col = (level + 1) * NODE_IDENT;
        amxtui_widget_print_at(widget, line, col, type, &node->txt, 0);

        if(node->data != NULL) {
            if(longest_label == 0) {
                longest_label = amxtui_tree_data_longest_label(node->parent);
            }
            col += (longest_label + 1);
            amxtui_tree_data(widget, node, line, col);
        }

        line++;
        current = node;
        node = amxtui_tree_get_next(current, widget);
        if((node == NULL) || (node == current)) {
            break;
        }
        if(node->parent != current->parent) {
            longest_label = 0;
        }
        level = amxtui_tree_get_level(node);
    }

    amxc_string_clean(&txt);

exit:
    return rv;
}

static void amxtui_ctrl_tb_show(const amxtui_widget_t* widget,
                                amxtui_ctrl_t* ctrl) {
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    amxtui_tree_node_t* selected = NULL;
    amxtui_tree_node_t* top = NULL;
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* selected_item = GET_ARG(widget_data, WD_SELECTED_ITEM);
    amxc_var_t* selected_line = GET_ARG(widget_data, WD_SELECTED_LINE);
    int line = GET_INT32(selected_line, NULL);
    size_t height = amxtui_widget_height(widget);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);
    selected = amxtui_tree_get_node(&tbctrl->top.nodes, 0, selected_item);
    top = selected;
    if(line >= (int) height - 1) {
        line = height - 1;
    }
    while(line > 0) {
        top = amxtui_tree_get_prev(top, widget);
        line--;
    }
    when_null(top, exit);

    amxtui_tree_show(widget, top, selected, &line);
    amxc_var_set(int32_t, selected_line, line);
exit:
    return;
}

static bool amxtui_ctrl_tb_handle_ctrl_key(amxtui_widget_t* widget,
                                           UNUSED amxtui_ctrl_t* ctrl,
                                           uint32_t ctrl_key) {
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    bool handled = true;
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* selected_item = GET_ARG(widget_data, WD_SELECTED_ITEM);
    amxc_var_t* selected_line = GET_ARG(widget_data, WD_SELECTED_LINE);
    amxtui_tree_node_t* selected_node = NULL;
    amxtui_tree_node_t* current = NULL;
    int line = GET_INT32(selected_line, NULL);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);
    selected_node = amxtui_tree_get_node(&tbctrl->top.nodes, 0, selected_item);
    current = selected_node;

    switch(ctrl_key) {
    case amxt_key_newline:
        break;
    case amxt_key_tab:
        amxtui_ctrl_emit_data(&tbctrl->ctrl,
                              CTRL_SIG_TB_SEL_CHANGED,
                              selected_item);
        break;
    case amxt_key_left:
        if(selected_node->parent->parent != NULL) {
            selected_node = selected_node->parent;
            amxtui_tree_node_collapse(tbctrl, widget, selected_node, true);
            line--;
        } else {
            amxtui_tree_node_collapse(tbctrl, widget, selected_node, true);
        }
        break;
    case amxt_key_right:
        amxtui_tree_node_collapse(tbctrl, widget, selected_node, false);
        if(!amxc_array_is_empty(&selected_node->nodes)) {
            selected_node = NODE_AT(&selected_node->nodes, 0);
            line++;
        }
        break;
    case amxt_key_home:
        selected_node = NODE_AT(&tbctrl->top.nodes, 0);
        line = 0;
        break;
    case amxt_key_end: {
        amxtui_tree_node_t* tmp = selected_node;
        line = amxtui_widget_height(widget) - 1;
        selected_node = amxtui_tree_get_next(tmp, widget);
        while(selected_node != tmp) {
            tmp = selected_node;
            selected_node = amxtui_tree_get_next(tmp, widget);
        }
    }
    break;
    case amxt_key_down: {
        amxtui_tree_node_t* tmp = selected_node;
        selected_node = amxtui_tree_get_next(tmp, widget);
        if(tmp != selected_node) {
            line++;
        }
    }
    break;
    case amxt_key_up: {
        amxtui_tree_node_t* tmp = selected_node;
        selected_node = amxtui_tree_get_prev(tmp, widget);
        if(tmp != selected_node) {
            line--;
        }
    }
    break;
    case amxt_key_page_down: {
        int nlines = amxtui_widget_height(widget) - 1;
        amxtui_tree_node_t* tmp = selected_node;
        selected_node = amxtui_tree_get_next(tmp, widget);
        while(nlines > 0 && selected_node != tmp) {
            nlines--;
            tmp = selected_node;
            selected_node = amxtui_tree_get_next(tmp, widget);
        }
    }
    break;
    case amxt_key_page_up: {
        int nlines = amxtui_widget_height(widget) - 1;
        amxtui_tree_node_t* tmp = selected_node;
        selected_node = amxtui_tree_get_prev(tmp, widget);
        while(nlines > 0 && selected_node != tmp) {
            nlines--;
            tmp = selected_node;
            selected_node = amxtui_tree_get_prev(tmp, widget);
        }
    }
    break;
    default:
        handled = false;
        break;
    }

    amxc_var_set_type(selected_item, AMXC_VAR_ID_LIST);
    amxtui_tree_build_index(selected_node, selected_item);
    amxc_var_set(int32_t, selected_line, line);

    if(selected_node != current) {
        amxtui_ctrl_emit_data(&tbctrl->ctrl,
                              CTRL_SIG_TB_SEL_CHANGED,
                              selected_item);
    }

    if(handled) {
        amxtui_widget_redraw(widget);
    }

    return handled;
}

static bool amxtui_ctrl_tb_handle_key(amxtui_widget_t* widget,
                                      amxtui_ctrl_t* ctrl,
                                      char key) {
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* selected_item = GET_ARG(widget_data, WD_SELECTED_ITEM);
    amxtui_tree_node_t* selected_node = NULL;
    bool handled = false;

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);
    selected_node = amxtui_tree_get_node(&tbctrl->top.nodes, 0, selected_item);

    if(key == ' ') {
        bool collapsed = amxtui_tb_widget_node_is_collapsed(widget,
                                                            selected_node);
        collapsed = collapsed? false:true;
        amxtui_tree_node_collapse(tbctrl, widget, selected_node, collapsed);
        handled = true;
    }

    return handled;
}

static amxc_var_t* amxtui_ctrl_tb_get_data(UNUSED const amxtui_widget_t* widget,
                                           UNUSED amxtui_ctrl_t* ctrl) {
    amxc_var_t* widget_data = NULL;
    amxc_var_t* selected = NULL;

    amxc_var_new(&widget_data);
    amxc_var_set_type(widget_data, AMXC_VAR_ID_HTABLE);
    selected = amxc_var_add_key(amxc_llist_t, widget_data, WD_SELECTED_ITEM, 0);
    amxc_var_add(uint32_t, selected, 0);
    amxc_var_add_key(int32_t, widget_data, WD_SELECTED_LINE, 0);
    amxc_var_add_key(amxc_htable_t, widget_data, WD_EXPANDED_ITEMS, 0);

    return widget_data;
}

static int amxtui_ctrl_new_tree_box(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    when_null(ctrl, exit);

    tbctrl = (amxtui_ctrl_tree_box_t*) calloc(1, sizeof(amxtui_ctrl_tree_box_t));
    amxtui_ctrl_init(&tbctrl->ctrl,
                     amxtui_ctrl_tb_show,
                     amxtui_ctrl_tb_handle_ctrl_key,
                     amxtui_ctrl_tb_handle_key,
                     amxtui_ctrl_tb_get_data);

    amxtui_ctrl_add_signal(&tbctrl->ctrl, CTRL_SIG_TB_EXPAND);
    amxtui_ctrl_add_signal(&tbctrl->ctrl, CTRL_SIG_TB_COLLAPSE);
    amxtui_ctrl_add_signal(&tbctrl->ctrl, CTRL_SIG_TB_SEL_CHANGED);

    amxc_array_init(&tbctrl->top.nodes, 5);

    *ctrl = &tbctrl->ctrl;

exit:
    return 0;
}

static int amxtui_ctrl_delete_tree_box(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    when_null(ctrl, exit);
    when_null(*ctrl, exit);
    when_false(strcmp(amxtui_ctrl_type_name(*ctrl), CTRL_NAME) == 0, exit);

    tbctrl = amxc_container_of((*ctrl), amxtui_ctrl_tree_box_t, ctrl);
    amxtui_ctrl_clean(&tbctrl->ctrl);
    amxc_array_clean(&tbctrl->top.nodes, amxtui_tree_clean_node);
    free(tbctrl);

exit:
    return 0;
}

static amxtui_ctrl_type_t amxtui_ctrl_tree_box = {
    .hit = { NULL, NULL, NULL },
    .ctor = amxtui_ctrl_new_tree_box,
    .dtor = amxtui_ctrl_delete_tree_box
};

CONSTRUCTOR_LVL(102) static void amxtui_ctrl_types_init(void) {
    amxtui_ctrl_type_register(&amxtui_ctrl_tree_box, CTRL_NAME);
}

amxtui_tree_node_t* amxtui_ctrl_tb_add_node(amxtui_ctrl_t* ctrl,
                                            amxtui_tree_node_t* parent,
                                            const char* txt,
                                            amxc_var_t* data) {
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    amxtui_tree_node_t* node = NULL;

    when_null(ctrl, exit);
    when_null(txt, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);

    if(parent == NULL) {
        parent = &tbctrl->top;
    }

    node = amxtui_tree_node_add_node(parent, txt, data);

exit:
    return node;
}

int amxtui_ctrl_tb_clear_nodes(amxtui_ctrl_t* ctrl,
                               amxtui_tree_node_t* parent) {
    int rv = -1;
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);

    if(parent == NULL) {
        parent = &tbctrl->top;
    }

    amxc_array_clean(&parent->nodes, amxtui_tree_clean_node);
    rv = 0;

exit:
    return rv;
}

amxtui_tree_node_t* amxtui_ctrl_tb_root(amxtui_ctrl_t* ctrl) {
    amxtui_tree_node_t* node = NULL;
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);

    node = &tbctrl->top;

exit:
    return node;
}

amxtui_tree_node_t* amxtui_ctrl_tb_get_node(amxtui_ctrl_t* ctrl,
                                            const amxc_var_t* item) {
    amxtui_tree_node_t* node = NULL;
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);
    node = amxtui_tree_get_node(&tbctrl->top.nodes, 0, item);

exit:
    return node;
}

amxtui_tree_node_t* amxtui_ctrl_tb_find_node(amxtui_ctrl_t* ctrl,
                                             amxc_llist_t* parts) {
    amxtui_tree_node_t* node = NULL;
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);

    node = amxtui_tree_node_find(&tbctrl->top, parts);

exit:
    return node;
}

int amxtui_tb_widget_select_node(amxtui_widget_t* widget,
                                 amxtui_tree_node_t* node) {
    int rv = -1;
    amxtui_ctrl_t* ctrl = NULL;
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    int line = 0;

    // fetch widget data
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* selected_item = GET_ARG(widget_data, WD_SELECTED_ITEM);
    amxc_var_t* selected_line = GET_ARG(widget_data, WD_SELECTED_LINE);

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);
    when_false(amxtui_node_in_tree(&tbctrl->top, node), exit);

    amxc_var_set_type(selected_item, AMXC_VAR_ID_LIST);
    amxtui_tree_build_index(node, selected_item);
    line = amxtui_tree_get_level(node);
    amxc_var_set(int32_t, selected_line, line);

    rv = 0;
exit:
    return rv;
}

const amxc_var_t* amxtui_tb_widget_get_selected_node_index(amxtui_widget_t* widget) {
    amxc_var_t* item_index = NULL;

    // fetch widget data
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    item_index = GET_ARG(widget_data, WD_SELECTED_ITEM);

    return item_index;
}

amxtui_tree_node_t* amxtui_tb_widget_get_selected_node(amxtui_widget_t* widget) {
    amxtui_tree_node_t* node = NULL;
    amxc_var_t* item_index = NULL;
    amxtui_ctrl_t* ctrl = NULL;

    // fetch widget data
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    item_index = GET_ARG(widget_data, WD_SELECTED_ITEM);

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);
    node = amxtui_ctrl_tb_get_node(ctrl, item_index);

exit:
    return node;

}

int amxtui_tb_widget_get_selected_line(amxtui_widget_t* widget) {
    int line = -1;
    amxtui_ctrl_t* ctrl = NULL;

    // fetch widget data
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* selected_line = GET_ARG(widget_data, WD_SELECTED_LINE);

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);
    line = GET_INT32(selected_line, NULL);

exit:
    return line;
}

int amxtui_tb_widget_get_selected_data_col(amxtui_widget_t* widget) {
    int col = -1;
    amxtui_ctrl_t* ctrl = NULL;
    amxtui_tree_node_t* node = NULL;
    int label_length = 0;
    int level = 0;

    // fetch widget data
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* selected_item = GET_ARG(widget_data, WD_SELECTED_ITEM);

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);
    node = amxtui_ctrl_tb_get_node(ctrl, selected_item);
    label_length = amxtui_tree_data_longest_label(node->parent);
    level = amxtui_tree_get_level(node);
    col = ((level + 1) * NODE_IDENT) + label_length + 4;

exit:
    return col;
}

bool amxtui_tb_widget_node_is_collapsed(const amxtui_widget_t* widget,
                                        amxtui_tree_node_t* node) {
    bool collapsed = true;
    amxc_string_t key;

    // fetch widget data
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* expanded_items = GET_ARG(widget_data, WD_EXPANDED_ITEMS);

    amxc_string_init(&key, 0);

    when_null(widget, exit);
    when_null(node, exit);

    amxc_string_setf(&key, "%p", (void*) node);

    if(GET_ARG(expanded_items, amxc_string_get(&key, 0))) {
        collapsed = false;
    }

exit:
    amxc_string_clean(&key);
    return collapsed;
}

int amxtui_tb_widget_collapse_node(amxtui_widget_t* widget,
                                   amxtui_tree_node_t* node) {
    amxtui_ctrl_t* ctrl = NULL;
    int rv = -1;
    amxtui_ctrl_tree_box_t* tbctrl = NULL;

    ctrl = amxtui_widget_get_ctrl(widget);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);
    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);
    when_false(amxtui_node_in_tree(&tbctrl->top, node), exit);

    rv = amxtui_tree_node_collapse(tbctrl, widget, node, true);

exit:
    return rv;
}

int amxtui_tb_widget_expand_node(amxtui_widget_t* widget,
                                 amxtui_tree_node_t* node) {
    amxtui_ctrl_t* ctrl = NULL;
    int rv = -1;
    amxtui_ctrl_tree_box_t* tbctrl = NULL;
    when_null(node, exit);

    ctrl = amxtui_widget_get_ctrl(widget);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);
    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);
    when_false(amxtui_node_in_tree(&tbctrl->top, node), exit);

    rv = amxtui_tree_node_collapse(tbctrl, widget, node, false);

exit:
    return rv;
}

static int isquote(int c) {
    int rv = 0;
    if((c == '\'') || (c == '"')) {
        rv = 1;
    }
    return rv;
}

int amxtui_tb_widget_select_path(amxtui_widget_t* widget,
                                 const char* path,
                                 char sep) {
    amxtui_ctrl_t* ctrl = NULL;
    int rv = -1;
    amxtui_tree_node_t* selected = NULL;
    amxc_llist_t parts;
    amxc_string_t str_path;

    amxc_string_init(&str_path, 0);
    amxc_string_set(&str_path, path);
    amxc_llist_init(&parts);

    ctrl = amxtui_widget_get_ctrl(widget);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    amxc_string_split_to_llist(&str_path, &parts, sep);
    amxc_llist_for_each(it, &parts) {
        amxc_string_t* part = amxc_string_from_llist_it(it);
        amxc_string_trim(part, isquote);
        if(amxc_string_text_length(part) == 0) {
            amxc_string_delete(&part);
        }
    }
    selected = amxtui_ctrl_tb_find_node(ctrl, &parts);
    if(selected != NULL) {
        amxtui_tb_widget_select_node(widget, selected);
        if(path[amxc_string_text_length(&str_path) - 1] == sep) {
            amxtui_ctrl_tree_box_t* tbctrl = NULL;
            amxc_var_t node_index;

            tbctrl = amxc_container_of(ctrl, amxtui_ctrl_tree_box_t, ctrl);
            amxc_var_init(&node_index);
            amxtui_tree_build_index(selected, &node_index);
            amxtui_tb_widget_expand_node(widget, selected);
            amxtui_ctrl_trigger_data(&tbctrl->ctrl,
                                     CTRL_SIG_TB_EXPAND,
                                     &node_index);
            amxc_var_clean(&node_index);
        }
    }

    rv = 0;

exit:
    amxc_string_clean(&str_path);
    amxc_llist_clean(&parts, amxc_string_list_it_free);
    return rv;
}

void amxtui_tree_node_delete(amxtui_tree_node_t** node) {
    when_null(node, exit);
    when_null(*node, exit);

    amxc_array_clean(&(*node)->nodes, amxtui_tree_clean_node);

    if((*node)->data != NULL) {
        if(amxc_var_get_parent((*node)->data) == NULL) {
            amxc_var_delete(&(*node)->data);
        }
    }

    amxc_string_clean(&(*node)->txt);
    if((*node)->parent != NULL) {
        uint32_t index = (*node)->index;
        amxc_array_t* nodes = &(*node)->parent->nodes;
        amxc_array_it_t* ait = amxc_array_get_at(nodes, index);
        amxc_array_it_take_data(ait);
        while(amxc_array_get_data_at(nodes, index + 1) != NULL) {
            amxc_array_it_t* it1 = amxc_array_get_at(nodes, index);
            amxc_array_it_t* it2 = amxc_array_get_at(nodes, index + 1);
            amxtui_tree_node_t* n = (amxtui_tree_node_t*) amxc_array_it_get_data(it2);
            n->index = index;
            amxc_array_it_swap(it1, it2);
            index++;
        }
    }
    free((*node));
    *node = NULL;

exit:
    return;
}

void amxtui_tree_node_clear(amxtui_tree_node_t* node) {
    when_null(node, exit);

    amxc_array_clean(&node->nodes, amxtui_tree_clean_node);

exit:
    return;
}

bool amxtui_tree_node_has_children(amxtui_tree_node_t* node) {
    bool has_children = false;

    when_null(node, exit);
    has_children = (!amxc_array_is_empty(&node->nodes) || node->has_children);

exit:
    return has_children;
}

void amxtui_tree_node_set_has_children(amxtui_tree_node_t* node,
                                       bool has_children) {
    when_null(node, exit);

    node->has_children = has_children;
exit:
    return;
}

void amxtui_tree_node_path(amxtui_tree_node_t* node,
                           amxc_string_t* path,
                           const char* sep,
                           bool quoted) {
    const char* quote = "";
    when_null(node, exit);
    when_null(path, exit);

    if(quoted) {
        quote = "'";
    }
    if(node->parent->parent != NULL) {
        amxtui_tree_node_path(node->parent, path, sep, quoted);
    }

    if((sep != 0) && !amxc_string_is_empty(path)) {
        amxc_string_appendf(path,
                            "%s%s%s%s",
                            sep,
                            quote,
                            amxc_string_get(&node->txt, 0),
                            quote);
    } else {
        amxc_string_appendf(path,
                            "%s%s%s",
                            quote,
                            amxc_string_get(&node->txt, 0),
                            quote);
    }

exit:
    return;
}

void amxtui_tree_node_sort(amxtui_tree_node_t* node) {
    amxc_array_it_t* ait = NULL;

    when_null(node, exit);

    amxc_array_sort(&node->nodes, amxtui_tree_node_cmp);
    ait = amxc_array_get_first(&node->nodes);
    while(ait != NULL) {
        amxtui_tree_node_t* n = NODE(ait);
        n->index = amxc_array_it_index(ait);
        ait = amxc_array_it_get_next(ait);
    }
exit:
    return;
}

amxtui_tree_node_t* amxtui_tree_node_find(amxtui_tree_node_t* node,
                                          amxc_llist_t* parts) {
    amxtui_tree_node_t* n = NULL;
    amxc_string_t* part = NULL;
    amxc_array_it_t* ait = NULL;

    when_null(node, exit);
    when_null(parts, exit);
    when_true(amxc_llist_is_empty(parts), exit);

    part = amxc_string_from_llist_it(amxc_llist_take_first(parts));
    ait = amxc_array_get_first(&node->nodes);
    while(ait) {
        int len = amxc_string_text_length(part);
        const char* p = amxc_string_get(part, 0);
        const char* np = NULL;
        n = NODE(ait);
        np = amxc_string_get(&n->txt, 0);
        if(strncmp(np, p, len) == 0) {
            if(amxc_llist_is_empty(parts)) {
                break;
            } else {
                n = amxtui_tree_node_find(n, parts);
                break;
            }
        }
        n = NULL;
        ait = amxc_array_it_get_next(ait);
    }

    amxc_string_delete(&part);

exit:
    return n;
}

amxtui_tree_node_t* amxtui_tree_node_get(amxtui_tree_node_t* node,
                                         const char* name) {
    amxtui_tree_node_t* n = NULL;
    amxc_array_it_t* ait = NULL;

    when_null(node, exit);
    when_null(name, exit);

    ait = amxc_array_get_first(&node->nodes);
    while(ait) {
        n = NODE(ait);
        if(strcmp(amxc_string_get(&n->txt, 0), name) == 0) {
            break;
        }
        ait = amxc_array_it_get_next(ait);
        n = NULL;
    }

exit:
    return n;
}

amxtui_tree_node_t* amxtui_tree_node_parent(amxtui_tree_node_t* node) {
    amxtui_tree_node_t* p = NULL;

    when_null(node, exit);
    p = node->parent;

exit:
    return p;
}

int amxtui_tree_node_set_data(amxtui_tree_node_t* node,
                              amxc_var_t* var) {
    int rv = -1;
    when_null(node, exit);

    node->data = var;
    rv = 0;

exit:
    return rv;
}

amxc_var_t* amxtui_tree_node_get_data(amxtui_tree_node_t* node) {
    amxc_var_t* data = NULL;
    when_null(node, exit);

    data = node->data;

exit:
    return data;
}

amxtui_tree_node_t* amxtui_tree_node_add_node(amxtui_tree_node_t* parent,
                                              const char* txt,
                                              amxc_var_t* data) {
    amxtui_tree_node_t* node = NULL;
    amxc_array_it_t* ait = NULL;

    when_null(parent, exit);
    when_str_empty(txt, exit);

    node = (amxtui_tree_node_t*) calloc(1, sizeof(amxtui_tree_node_t));
    amxc_array_init(&node->nodes, 5);
    amxc_string_init(&node->txt, 0);
    amxc_string_set(&node->txt, txt);
    node->data = data;

    node->parent = parent;
    ait = amxc_array_append_data(&parent->nodes, node);
    node->index = amxc_array_it_index(ait);

exit:
    return node;
}