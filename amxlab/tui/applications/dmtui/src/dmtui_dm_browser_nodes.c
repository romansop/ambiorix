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

typedef struct _scan_data {
    dmtui_dm_browser_t* dmbctrl;
    int refs;
} scan_data_t;

static void dmtui_dm_browser_update_nodes(amxtui_tree_node_t* parent,
                                          amxc_var_t* data,
                                          int level) {
    const char* type_name = amxc_var_key(data);
    when_null(type_name, exit);

    amxc_var_for_each(item, data) {
        amxtui_tree_node_t* node = NULL;
        amxc_string_t fname;
        const char* name = NULL;
        bool children = false;

        amxc_string_init(&fname, 0);
        if((strcmp(type_name, "objects") == 0) ||
           (strcmp(type_name, "instances") == 0)) {
            name = GET_CHAR(item, NULL);
            children = true;
        } else if(strcmp(type_name, "parameters") == 0) {
            amxc_var_t* value = GET_ARG(item, "value");
            name = amxc_var_key(item);
            amxc_var_cast(value, GET_UINT32(item, "type_id"));
        } else if(strcmp(type_name, "functions") == 0) {
            name = amxc_var_key(item);
            amxc_string_setf(&fname, "%s()", name);
            name = amxc_string_get(&fname, 0);
            children = false;
        }
        node = amxtui_tree_node_get(parent, name);
        if(node == NULL) {
            node = amxtui_tree_node_add_node(parent, name, NULL);
            amxtui_tree_node_set_has_children(node, children);
        }
        if(children && (level > 0)) {
            dmtui_dm_browser_update_node(node, level - 1);
        }
        if(!children) {
            amxtui_tree_node_set_data(node, item);
        }
        amxc_string_clean(&fname);
    }

exit:
    return;
}

static void dmtui_dm_browser_get_meta_data(UNUSED const amxc_var_t* const data,
                                           void* const priv) {
    amxtui_tree_node_t* node = (amxtui_tree_node_t*) priv;
    dmtui_dm_browser_update_node(node, 0);
}

static void dmtui_dm_browser_add_root_object(dmtui_dm_browser_t* dmbctrl, amxd_path_t* path) {
    amxtui_tree_node_t* node = NULL;
    amxtui_tree_node_t* root = amxtui_ctrl_tb_root(&dmbctrl->ctrl);
    char* first = amxd_path_get_first(path, true);
    first[strlen(first) - 1] = 0;

    node = amxtui_tree_node_get(root, first);
    if(node == NULL) {
        node = amxtui_tree_node_add_node(root, first, NULL);
        amxtui_tree_node_set_has_children(node, true);
        amxp_sigmngr_deferred_call(NULL, dmtui_dm_browser_get_meta_data, NULL, node);
    }

    free(first);
}

static void dmtui_dm_browser_add_root(UNUSED const amxb_bus_ctx_t* bus_ctx,
                                      const amxc_var_t* const data,
                                      void* priv) {
    scan_data_t* scan_data = (scan_data_t*) priv;
    amxd_path_t path;

    if(data == NULL) { // data is NULL when all are available
        scan_data->refs--;
        if(scan_data->refs == 0) {
            amxtui_tree_node_sort(amxtui_ctrl_tb_root(&scan_data->dmbctrl->ctrl));
            amxtui_ctrl_emit_data(&scan_data->dmbctrl->ctrl, CTRL_SIG_REDRAW, NULL);
            amxtui_ctrl_emit_data(&scan_data->dmbctrl->ctrl, CTRL_SIG_DM_LOAD_DONE, NULL);
            free(scan_data);
            goto exit;
        }
    }

    amxd_path_init(&path, NULL);
    amxc_var_for_each(var, data) {
        const char* txt = GET_CHAR(var, NULL);
        if((txt == NULL) || (*txt == 0)) {
            continue;
        }
        amxd_path_setf(&path, false, "%s", txt);
        dmtui_dm_browser_add_root_object(scan_data->dmbctrl, &path);
    }

    amxd_path_clean(&path);

exit:
    return;
}

void dmtui_dm_browser_discover_objects(dmtui_dm_browser_t* dmbctrl) {
    scan_data_t* scan_data = NULL;
    amxc_llist_t* connections = amxp_connection_get_connections();

    scan_data = (scan_data_t*) calloc(1, sizeof(scan_data_t));
    scan_data->dmbctrl = dmbctrl;

    amxtui_ctrl_tb_clear_nodes(&dmbctrl->ctrl, NULL);

    // increase reference counter before calling amxb_list.
    scan_data->refs++;

    amxc_llist_for_each(it, connections) {
        amxp_connection_t* con = amxc_llist_it_get_data(it, amxp_connection_t, it);
        amxb_bus_ctx_t* bus_ctx = (amxb_bus_ctx_t*) con->priv;
        if(con->type != AMXP_CONNECTION_BUS) {
            continue;
        }
        scan_data->refs++;
        amxb_list(bus_ctx,
                  "",
                  AMXB_FLAG_OBJECTS |
                  AMXB_FLAG_INSTANCES |
                  AMXB_FLAG_FIRST_LVL,
                  dmtui_dm_browser_add_root,
                  scan_data);
    }

    scan_data->refs--;

    if(scan_data->refs == 0) {
        amxtui_tree_node_sort(amxtui_ctrl_tb_root(&scan_data->dmbctrl->ctrl));
        amxtui_ctrl_emit_data(&scan_data->dmbctrl->ctrl, CTRL_SIG_REDRAW, NULL);
        amxtui_ctrl_emit_data(&scan_data->dmbctrl->ctrl, CTRL_SIG_DM_LOAD_DONE, NULL);
        free(scan_data);
    }
}

void dmtui_dm_browser_add_objects(dmtui_dm_browser_t* dmbctrl, amxc_var_t* objects) {
    amxd_path_t path;

    amxd_path_init(&path, NULL);
    amxc_var_for_each(var, objects) {
        const char* txt = GET_CHAR(var, NULL);
        if((txt == NULL) || (*txt == 0)) {
            continue;
        }
        amxd_path_setf(&path, false, "%s", txt);
        dmtui_dm_browser_add_root_object(dmbctrl, &path);
    }

    amxd_path_clean(&path);
}

int dmtui_dm_browser_update_node(amxtui_tree_node_t* node, int level) {
    amxc_string_t path;
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxc_var_t tmp;
    amxc_var_t* node_data = amxtui_tree_node_get_data(node);
    int rv = 0;

    amxc_string_init(&path, 0);
    amxtui_tree_node_path(node, &path, ".", false);
    amxc_var_init(&tmp);

    when_false(amxtui_tree_node_has_children(node), exit);
    bus_ctx = amxb_be_who_has(amxc_string_get(&path, 0));
    when_null_status(bus_ctx, exit, amxtui_tree_node_delete(&node));
    rv = amxb_describe(bus_ctx,
                       amxc_string_get(&path, 0),
                       AMXB_FLAG_PARAMETERS |
                       AMXB_FLAG_OBJECTS |
                       AMXB_FLAG_INSTANCES |
                       AMXB_FLAG_FUNCTIONS,
                       &tmp,
                       10);
    when_failed(rv, exit);

    amxc_var_delete(&node_data);
    amxtui_tree_node_set_data(node, NULL);
    node_data = GETI_ARG(&tmp, 0);
    amxc_var_take_it(node_data);
    amxtui_tree_node_set_data(node, node_data);

    if(GET_UINT32(node_data, "type_id") == amxd_object_template) {
        dmtui_dm_browser_update_nodes(node,
                                      GET_ARG(node_data, "instances"),
                                      level);
    } else {
        dmtui_dm_browser_update_nodes(node,
                                      GET_ARG(node_data, "objects"),
                                      level);
        dmtui_dm_browser_update_nodes(node,
                                      GET_ARG(node_data, "parameters"),
                                      level);
        dmtui_dm_browser_update_nodes(node,
                                      GET_ARG(node_data, "functions"),
                                      level);
    }

exit:
    amxc_var_clean(&tmp);
    amxc_string_clean(&path);
    return rv;
}
