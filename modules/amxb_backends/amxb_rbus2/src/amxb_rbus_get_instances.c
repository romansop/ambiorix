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

#include "amxb_rbus.h"

static bool amxb_rbus_is_table(amxb_rbus_t* amxb_rbus_ctx, const char* path) {
    rbusElementInfo_t* elems = NULL;
    bool retval = true;
    const amxc_var_t* skip_verify = amxb_rbus_get_config_option("skip-verify-type");

    // Is data element type verification disabled?
    // when "skip-verify-type" is set to true do not check if the element is a table
    when_true(GET_BOOL(skip_verify, NULL), exit);

    rbusElementInfo_get(amxb_rbus_ctx->handle, path, 1, &elems);
    when_null(elems, exit);

    retval = (elems->type == RBUS_ELEMENT_TYPE_TABLE);
    rbusElementInfo_free(amxb_rbus_ctx->handle, elems);

exit:
    return retval;
}

static int amxb_rbus_filter_instances(amxb_rbus_t* amxb_rbus_ctx, uint32_t path_depth, int32_t depth, amxc_var_t* resolved_table) {
    int retval = amxd_status_invalid_path;
    amxc_array_t* paths = NULL;
    amxd_path_t obj_path;
    amxd_path_init(&obj_path, NULL);

    // check if the first path is a table, if not return an error
    paths = amxc_htable_get_sorted_keys(amxc_var_constcast(amxc_htable_t, resolved_table));
    if(paths != NULL) {
        const char* path = (const char*) amxc_array_get_data_at(paths, 0);
        amxd_path_setf(&obj_path, false, "%s", path);
        if(!amxd_path_is_instance_path(&obj_path) && !amxb_rbus_is_table(amxb_rbus_ctx, path)) {
            amxc_var_clean(resolved_table);
            goto exit;
        }
    }

    retval = amxd_status_ok;
    amxc_var_for_each(object, resolved_table) {
        const char* path = amxc_var_key(object);

        // check if it is an instance (must end with a number)
        amxd_path_setf(&obj_path, false, "%s", path);
        if(!amxd_path_is_instance_path(&obj_path)) {
            amxc_var_delete(&object);
            continue;
        }

        // if depth is to high remove object
        if((depth < INT32_MAX) && (depth > 1)) {
            printf("MAX DEPTH = %d - current depth = %d [%s]\n", depth, amxd_path_get_depth(&obj_path), amxd_path_get(&obj_path, AMXD_OBJECT_TERMINATE));
            if(amxd_path_get_depth(&obj_path) > path_depth + depth) {
                amxc_var_delete(&object);
                continue;
            }
        }

        // Remove all parameters except "Alias"
        // As rbus can not provide key parameters only
        // it is assumed here that the Alias parameter is a key
        // Other keys will be removed as no indication is given
        // that it is a key.
        amxc_var_for_each(param, object) {
            const char* param_name = amxc_var_key(param);
            if(strcmp(param_name, "Alias") != 0) {
                amxc_var_delete(&param);
            }
        }
    }

exit:
    amxc_array_delete(&paths, NULL);
    amxd_path_clean(&obj_path);
    return retval;
}

// The object path must end with '.'
int amxb_rbus_get_instances(void* const ctx,
                            const char* object,
                            const char* search_path,
                            int32_t depth,
                            uint32_t access,
                            amxc_var_t* ret,
                            int timeout) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    const amxc_var_t* check_amx = amxb_rbus_get_config_option("use-amx-calls");
    int retval = -1;
    uint32_t path_depth = 0;
    amxd_path_t full_path;

    // apply the timeout
    amxb_rbus_set_timeout(timeout);

    amxd_path_init(&full_path, NULL);
    if((search_path != NULL) && (*search_path != 0)) {
        amxd_path_setf(&full_path, true, "%s%s", object, search_path);
    } else {
        amxd_path_setf(&full_path, true, "%s", object);
    }
    // check if the path provided is not an instance path
    if(amxd_path_is_instance_path(&full_path)) {
        retval = amxd_status_invalid_path;
        goto exit;
    }
    path_depth = amxd_path_get_depth(&full_path);

    if(GET_BOOL(check_amx, NULL) && amxb_rbus_remote_is_amx(amxb_rbus_ctx, object)) {
        retval = amxb_rbus_call_get(amxb_rbus_ctx, "_get_instances", object, search_path, depth, access, ret);
    } else {
        amxc_var_t* data = NULL;
        int32_t resolve_depth = 0;
        amxc_var_set_type(ret, AMXC_VAR_ID_LIST);
        data = amxc_var_add(amxc_htable_t, ret, NULL);
        resolve_depth = (depth < INT32_MAX && depth >= 0)? depth + 1:depth;

        retval = amxb_rbus_resolve(amxb_rbus_ctx, object, search_path, resolve_depth, data);
        when_failed(retval, exit);
        retval = amxb_rbus_filter_instances(amxb_rbus_ctx, path_depth, depth, data);
    }

exit:
    amxd_path_clean(&full_path);
    return retval;
}
