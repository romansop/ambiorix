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

#if !defined(__AMXTUI_DMTUI_BROWSER_H__)
#define __AMXTUI_DMTUI_BROWSER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <amxtui/amxtui_types.h>
#include <amxtui/ctrl/amxtui_ctrl_tree_box.h>

typedef struct _amxtui_ctrl_dm_browser {
    amxtui_widget_t* input_box;
    amxc_lstack_t ostack;
    amxtui_ctrl_t ctrl;
} dmtui_dm_browser_t;

#define CTRL_NAME        "dm-browser"

#define CTRL_SIG_DM_SEL_CHANGED  "dm:sel-changed"
#define CTRL_SIG_DM_OBJECT_INFO  "dm:object-info"
#define CTRL_SIG_DM_LOAD_DONE    "dm:load-done"

// node functions
void dmtui_dm_browser_discover_objects(dmtui_dm_browser_t* dmbctrl);

void dmtui_dm_browser_add_objects(dmtui_dm_browser_t* dmbctrl, amxc_var_t* objects);

bool dmtui_dm_browser_has_root(amxtui_ctrl_t* ctrl, amxd_path_t* path);

int dmtui_dm_browser_update_node(amxtui_tree_node_t* node, int level);

// edit functions
void dmtui_dm_browser_start_edit(amxtui_widget_t* widget,
                                 dmtui_dm_browser_t* dmbctrl,
                                 amxtui_tree_node_t* node);

// data model browser controller data functions
int dmtui_dm_browser_load(amxtui_ctrl_t* ctrl, amxc_var_t* objects);

// expects a string as data, which contains an object path
// the path will be selected in the tree
void dmtui_dm_browser_slot_select(const char* const sig_name,
                                  const amxc_var_t* const data,
                                  void* const priv);

// find node in dm browser tree and selects it.
// pointer to the node is returned
amxtui_tree_node_t* dmtui_dm_browser_select(amxtui_widget_t* widget,
                                            const char* path);

// follow reference.
// widget is the widget that must be upadated
// node is a parameter node, that contains the path that needs to be followed
void dmtui_dm_browser_follow_reference(amxtui_widget_t* widget,
                                       amxtui_ctrl_t* ctrl,
                                       amxtui_tree_node_t* node);

// select an object, using a reference path
void dmtui_dm_browser_select_referenced(amxtui_widget_t* widget,
                                        amxtui_ctrl_t* ctrl,
                                        const char* ref_path,
                                        bool sel_changed);

bool dmtui_dm_browser_is_cancel_button(const char* const sig_name,
                                       const amxc_var_t* const data);

int dmtui_dm_browser_widget_update(amxtui_widget_t* widget);

#ifdef __cplusplus
}
#endif

#endif // __AMXTUI_DMTUI_BROWSER_H__
