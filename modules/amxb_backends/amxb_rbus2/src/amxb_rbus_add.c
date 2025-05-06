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

static int amxb_rbus_call_add(amxb_rbus_t* amxb_rbus_ctx,
                              const char* object,
                              const char* search_path,
                              uint32_t index,
                              const char* name,
                              amxc_var_t* values,
                              uint32_t access,
                              amxc_var_t* ret) {
    int retval = -1;
    amxc_var_t args;
    amxc_var_t* params = NULL;
    amxb_request_t* request = NULL;

    amxc_var_init(&args);
    when_failed(amxb_request_new(&request), exit);

    request->result = ret;

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "rel_path", search_path);
    amxc_var_add_key(uint32_t, &args, "index", index);
    amxc_var_add_key(cstring_t, &args, "name", name);
    params = amxc_var_add_key(amxc_htable_t, &args, "parameters", NULL);
    amxc_var_move(params, values);
    amxc_var_add_key(uint32_t, &args, "access", access);

    retval = amxb_rbus_invoke_root(amxb_rbus_ctx, object, "_add", &args, request);

exit:
    request->result = NULL;
    amxb_close_request(&request);
    amxc_var_clean(&args);
    return retval;
}

static void amxb_rbus_remove_entries(amxc_var_t* resolved,
                                     uint32_t depth) {
    amxc_var_for_each(entry, resolved) {
        const char* path = amxc_var_key(entry);
        const char* dot = strchr(path, '.');
        uint32_t count = 0;
        while(dot != NULL) {
            count++;
            dot = strchr(dot + 1, '.');
        }
        if(count != depth) {
            amxc_var_delete(&entry);
        }
    }
}

static int amxb_rbus_add_set_values(amxb_rbus_t* amxb_rbus_ctx,
                                    const char* path,
                                    amxc_var_t* parameters) {
    int retval = 0;
    amxc_var_t data;
    amxc_var_init(&data);
    when_true(amxc_var_is_null(parameters), exit);

    retval = amxb_rbus_set_values(amxb_rbus_ctx, true, path, parameters, &data);

exit:
    amxc_var_clean(&data);
    return retval;
}

static int amxb_rbus_check_resolved(amxb_rbus_t* amxb_rbus_ctx,
                                    const char* table,
                                    const char* alias,
                                    amxc_var_t* parameters,
                                    amxc_var_t* resolved_table,
                                    amxc_var_t* ret) {
    int retval = 0;
    amxc_string_t full_path;
    amxc_string_init(&full_path, 0);

    amxc_var_for_each(item, resolved_table) {
        const char* path = amxc_var_key(item);
        amxc_var_t* instance = NULL;
        uint32_t index = 0;
        amxc_string_setf(&full_path, "%s%s", path, table);
        retval = rbusTable_addRow(amxb_rbus_ctx->handle, amxc_string_get(&full_path, 0), alias, &index);
        retval = amxb_rbus_translate_rbus2status(retval);
        if(retval != 0) {
            continue;
        }
        // instance added - create variant containing all the data
        instance = amxc_var_add(amxc_htable_t, ret, NULL);
        amxc_var_add_key(uint32_t, instance, "index", index);
        if((alias != NULL) && (*alias != 0)) {
            amxc_var_add_key(cstring_t, instance, "name", alias);
            amxc_string_setf(&full_path, "%s%s%s.", path, table, alias);
            amxc_var_add_key(cstring_t, instance, "object", amxc_string_get(&full_path, 0));
        } else {
            amxc_var_t* name = amxc_var_add_key(uint32_t, instance, "name", index);
            amxc_var_cast(name, AMXC_VAR_ID_CSTRING);
            amxc_string_setf(&full_path, "%s%s%d.", path, table, index);
            amxc_var_add_key(cstring_t, instance, "object", amxc_string_get(&full_path, 0));
        }
        amxc_string_setf(&full_path, "%s%s%d.", path, table, index);
        amxc_var_add_key(cstring_t, instance, "path", amxc_string_get(&full_path, 0));
        amxc_var_add_key(amxc_htable_t, instance, "parameters", NULL);

        // As it is not possible to push parameter values together with the addrow
        // the parameter values are set with a separate call.
        retval = amxb_rbus_add_set_values(amxb_rbus_ctx, amxc_string_get(&full_path, 0), parameters);
        if(retval != 0) {
            rbusTable_removeRow(amxb_rbus_ctx->handle, amxc_string_get(&full_path, 0));
            amxc_var_delete(&instance);
        }

        // TODO ?  Fetch "Alias" parameter and add it to the repsone
        //         RBus doesn't have any knowledge of key parameters.
        //         USP requires that all key parameters are returned when a new instance is created.
        //         As a work-arround fetch the alias parameter of the instance,
        //         if it exists, use that as a key parameter, otherwise leave the key parameter response empty.
        // Another option is to leave the key-parameters table empty.
    }

    amxc_string_clean(&full_path);
    return retval;
}

int amxb_rbus_add(void* const ctx,
                  const char* object,
                  const char* search_path,
                  uint32_t index,
                  const char* name,
                  amxc_var_t* values,
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
        retval = amxb_rbus_call_add(amxb_rbus_ctx, object, search_path, index, name, values, access, ret);
    } else {
        amxd_path_t path;
        amxc_var_t* result = NULL;
        char* table = NULL;
        size_t entries = 0;
        amxd_path_init(&path, 0);
        // resolving should be done on a non-table
        // assume that the last part of the path is a multi-instance object (table),
        // so remove it and then resolve
        // if search_path is empty then the last part of the object must be removed.
        // if search_path is not empty then the last part of the search_path must be removed.
        if((search_path != NULL) && (*search_path != 0)) {
            amxd_path_setf(&path, true, "%s", search_path);
            search_path = amxd_path_get(&path, AMXD_OBJECT_TERMINATE);
        } else {
            amxd_path_setf(&path, true, "%s", object);
            object = amxd_path_get(&path, AMXD_OBJECT_TERMINATE);
        }
        table = amxd_path_get_last(&path, true);
        retval = amxb_rbus_resolve(amxb_rbus_ctx, object, search_path, 1, &resolved_table);
        when_failed(retval, exit);
        // Check the resolved table and filter out all sub-objects of the given path.
        if((search_path != NULL) && (*search_path != 0)) {
            amxd_path_prepend(&path, object);
        }
        amxb_rbus_remove_entries(&resolved_table, amxd_path_get_depth(&path));
        entries = amxc_htable_size(amxc_var_constcast(amxc_htable_t, &resolved_table));
        // Index can not be passed using rbus api, so no need to add it as argument
        if(entries == 0) {
            if(amxd_path_is_search_path(&path)) {
                retval = 0;
            } else {
                retval = amxd_status_invalid_type;
            }
        } else if(entries > 1) {
            result = amxc_var_add(amxc_llist_t, ret, NULL);
            retval = amxb_rbus_check_resolved(amxb_rbus_ctx, table, name, values, &resolved_table, result);
        } else {
            result = ret;
            retval = amxb_rbus_check_resolved(amxb_rbus_ctx, table, name, values, &resolved_table, result);
        }
        free(table);
        amxd_path_clean(&path);
    }

exit:
    amxc_var_clean(&resolved_table);
    return retval;
}
