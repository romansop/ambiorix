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

#include "amxb_rbus.h"

static int amxb_rbus_call_del(amxb_rbus_t* amxb_rbus_ctx,
                              const char* object,
                              const char* search_path,
                              uint32_t index,
                              const char* name,
                              uint32_t access,
                              amxc_var_t* ret) {
    int retval = -1;
    amxc_var_t args;
    amxb_request_t* request = NULL;
    amxc_var_t* result = NULL;

    amxc_var_init(&args);
    when_failed(amxb_request_new(&request), exit);

    request->result = ret;

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "rel_path", search_path);
    amxc_var_add_key(uint32_t, &args, "index", index);
    amxc_var_add_key(cstring_t, &args, "name", name);
    amxc_var_add_key(uint32_t, &args, "access", access);

    retval = amxb_rbus_invoke_root(amxb_rbus_ctx, object, "_del", &args, request);
    when_failed(retval, exit);
    result = GETI_ARG(ret, 0);

    amxc_var_take_it(result);
    amxc_var_move(ret, result);
    amxc_var_delete(&result);

exit:
    request->result = NULL;
    amxb_close_request(&request);
    amxc_var_clean(&args);
    return retval;
}

static void amxb_rbus_collect_deleted(amxb_rbus_t* amxb_rbus_ctx,
                                      const char* path,
                                      amxc_var_t* ret) {
    amxc_var_t collection;
    const amxc_htable_t* paths = NULL;
    amxc_array_t* sorted_paths = NULL;
    uint32_t nr_paths = 0;

    amxc_var_init(&collection);
    amxc_var_set_type(&collection, AMXC_VAR_ID_HTABLE);

    amxb_rbus_resolve(amxb_rbus_ctx, path, NULL, INT32_MAX, &collection);

    paths = amxc_var_constcast(amxc_htable_t, &collection);
    sorted_paths = amxc_htable_get_sorted_keys(paths);
    nr_paths = amxc_array_size(sorted_paths);

    for(uint32_t index = 0; index < nr_paths; index++) {
        const char* item_path = (const char*) amxc_array_get_data_at(sorted_paths, index);
        amxc_var_add(cstring_t, ret, item_path);
    }

    amxc_array_delete(&sorted_paths, NULL);
    amxc_var_clean(&collection);
}

static int amxb_rbus_send_del_request(amxb_rbus_t* amxb_rbus_ctx,
                                      const char* path,
                                      amxc_var_t* ret) {
    int retval = 0;
    amxc_var_t del_objs;

    amxc_var_init(&del_objs);
    amxc_var_set_type(&del_objs, AMXC_VAR_ID_LIST);

    amxb_rbus_collect_deleted(amxb_rbus_ctx, path, &del_objs);

    retval = rbusTable_removeRow(amxb_rbus_ctx->handle, path);
    retval = amxb_rbus_translate_rbus2status(retval);

    if(retval == 0) {
        amxc_var_for_each(var, (&del_objs)) {
            amxc_llist_append(&ret->data.vl, &var->lit);
        }
    }

    amxc_var_clean(&del_objs);
    return retval;
}

static int amxb_rbus_check_resolved(amxb_rbus_t* amxb_rbus_ctx,
                                    amxc_var_t* resolved_table,
                                    amxc_var_t* ret) {
    int retval = 0;
    const amxc_htable_t* paths = amxc_var_constcast(amxc_htable_t, resolved_table);
    amxc_array_t* sorted_paths = amxc_htable_get_sorted_keys(paths);
    uint32_t nr_paths = amxc_array_size(sorted_paths);

    for(uint32_t index = 0; index < nr_paths; index++) {
        const char* path = (const char*) amxc_array_get_data_at(sorted_paths, index);
        retval = amxb_rbus_send_del_request(amxb_rbus_ctx, path, ret);
        if(retval != 0) {
            // If multiple instances needs to be deleted and one of them fails
            // stop deleting and leave the loop.
            // The instances that have been deleted are in the list.
            // When using amx-calls and there is one failure, no instances are
            // deleted. With RBus each instance has to be deleted individually
            // no transaction like behavior is available. So just stop deleting
            // at the first failure, it will not be possible to revert the
            // already deleted objecs.
            break;
        }
    }

    amxc_array_delete(&sorted_paths, NULL);
    return retval;
}

int amxb_rbus_del(void* const ctx,
                  const char* object,
                  const char* search_path,
                  uint32_t index,
                  const char* name,
                  uint32_t access,
                  amxc_var_t* ret,
                  int timeout) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    const amxc_var_t* check_amx = amxb_rbus_get_config_option("use-amx-calls");
    amxc_var_t resolved_table;
    int retval = -1;

    amxc_var_init(&resolved_table);
    amxc_var_set_type(&resolved_table, AMXC_VAR_ID_HTABLE);
    amxc_var_set_type(ret, AMXC_VAR_ID_LIST);

    // apply the timeout
    amxb_rbus_set_timeout(timeout);

    if(GET_BOOL(check_amx, NULL) && amxb_rbus_remote_is_amx(amxb_rbus_ctx, object)) {
        retval = amxb_rbus_call_del(amxb_rbus_ctx, object, search_path, index, name, access, ret);
    } else {
        amxc_string_t full_search_path;
        amxc_string_init(&full_search_path, 0);
        if((search_path != NULL) && (*search_path != 0)) {
            if(index != 0) {
                amxc_string_setf(&full_search_path, "%s.%d.", search_path, index);
            } else if(name != NULL) {
                amxc_string_setf(&full_search_path, "%s.[%s].", search_path, name);
            } else {
                amxc_string_setf(&full_search_path, "%s", search_path);
            }
        } else {
            if(index != 0) {
                amxc_string_setf(&full_search_path, "%d.", index);
            } else if(name != NULL) {
                amxc_string_setf(&full_search_path, "[Alias == '%s'].", name);
            }
        }
        retval = amxb_rbus_resolve(amxb_rbus_ctx, object, amxc_string_get(&full_search_path, 0), 0, &resolved_table);
        amxc_string_clean(&full_search_path);
        if(amxc_htable_is_empty(amxc_var_constcast(amxc_htable_t, &resolved_table))) {
            retval = amxd_status_object_not_found;
        }
        when_failed(retval, exit);
        // The resolved_table should contain all instances that needs to be deleted.
        retval = amxb_rbus_check_resolved(amxb_rbus_ctx, &resolved_table, ret);
    }

exit:
    amxc_var_clean(&resolved_table);
    return retval;
}

