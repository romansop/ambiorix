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

#if !defined(__AMXTUI_CTRL_TREE_BOX_H__)
#define __AMXTUI_CTRL_TREE_BOX_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <amxtui/amxtui_types.h>

#define CTRL_SIG_TB_EXPAND       "tb:expand"
#define CTRL_SIG_TB_COLLAPSE     "tb:collapse"
#define CTRL_SIG_TB_SEL_CHANGED  "tb:sel-changed"

typedef struct _amxtui_tree_node amxtui_tree_node_t;

// tree box controller functions
amxtui_tree_node_t* amxtui_ctrl_tb_add_node(amxtui_ctrl_t* ctrl,
                                            amxtui_tree_node_t* parent,
                                            const char* txt,
                                            amxc_var_t* data);

int amxtui_ctrl_tb_clear_nodes(amxtui_ctrl_t* ctrl,
                               amxtui_tree_node_t* parent);

amxtui_tree_node_t* amxtui_ctrl_tb_root(amxtui_ctrl_t* ctrl);

amxtui_tree_node_t* amxtui_ctrl_tb_get_node(amxtui_ctrl_t* ctrl,
                                            const amxc_var_t* item);

amxtui_tree_node_t* amxtui_ctrl_tb_find_node(amxtui_ctrl_t* ctrl,
                                             amxc_llist_t* parts);

// tree box widget functions
int amxtui_tb_widget_select_node(amxtui_widget_t* widget,
                                 amxtui_tree_node_t* node);

const amxc_var_t* amxtui_tb_widget_get_selected_node_index(amxtui_widget_t* widget);

amxtui_tree_node_t* amxtui_tb_widget_get_selected_node(amxtui_widget_t* widget);

int amxtui_tb_widget_get_selected_line(amxtui_widget_t* widget);

int amxtui_tb_widget_get_selected_data_col(amxtui_widget_t* widget);

bool amxtui_tb_widget_node_is_collapsed(const amxtui_widget_t* widget,
                                        amxtui_tree_node_t* node);

int amxtui_tb_widget_collapse_node(amxtui_widget_t* widget,
                                   amxtui_tree_node_t* node);

int amxtui_tb_widget_expand_node(amxtui_widget_t* widget,
                                 amxtui_tree_node_t* node);

int amxtui_tb_widget_select_path(amxtui_widget_t* widget,
                                 const char* path,
                                 char sep);

// tree node functions
void amxtui_tree_node_delete(amxtui_tree_node_t** node);

void amxtui_tree_node_clear(amxtui_tree_node_t* node);

bool amxtui_tree_node_has_children(amxtui_tree_node_t* node);

void amxtui_tree_node_set_has_children(amxtui_tree_node_t* node,
                                       bool has_children);

void amxtui_tree_node_path(amxtui_tree_node_t* node,
                           amxc_string_t* path,
                           const char* sep,
                           bool quote);

void amxtui_tree_node_sort(amxtui_tree_node_t* node);

amxtui_tree_node_t* amxtui_tree_node_find(amxtui_tree_node_t* node,
                                          amxc_llist_t* parts);

amxtui_tree_node_t* amxtui_tree_node_get(amxtui_tree_node_t* node,
                                         const char* name);

amxtui_tree_node_t* amxtui_tree_node_parent(amxtui_tree_node_t* node);

int amxtui_tree_node_set_data(amxtui_tree_node_t* node,
                              amxc_var_t* var);

amxc_var_t* amxtui_tree_node_get_data(amxtui_tree_node_t* node);

amxtui_tree_node_t* amxtui_tree_node_add_node(amxtui_tree_node_t* parent,
                                              const char* txt,
                                              amxc_var_t* data);

#ifdef __cplusplus
}
#endif

#endif // _AMXG_CTRL_TREE_BOX_H__

