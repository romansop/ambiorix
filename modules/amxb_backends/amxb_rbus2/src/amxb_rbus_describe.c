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

typedef void (* amxb_rbus_add_info_t) (amxb_rbus_t* amxb_rbus_ctx,
                                       amxd_path_t* full_path,
                                       uint32_t flags,
                                       uint32_t access,
                                       amxd_object_type_t type,
                                       rbusElementInfo_t* element,
                                       amxc_var_t* object_data);

static int amxb_rbus_call_describe(amxb_rbus_t* amxb_rbus_ctx,
                                   const char* object,
                                   const char* rel_path,
                                   uint32_t flags,
                                   uint32_t access,
                                   amxc_var_t* values) {
    int retval = -1;
    amxc_var_t args;
    amxb_request_t* request = NULL;

    amxc_var_init(&args);
    when_failed(amxb_request_new(&request), exit);

    request->result = values;

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "rel_path", rel_path);
    amxc_var_add_key(bool, &args, "functions", (flags & AMXB_FLAG_FUNCTIONS) != 0);
    amxc_var_add_key(bool, &args, "parameters", (flags & AMXB_FLAG_PARAMETERS) != 0);
    amxc_var_add_key(bool, &args, "objects", (flags & AMXB_FLAG_OBJECTS) != 0);
    amxc_var_add_key(bool, &args, "instances", (flags & AMXB_FLAG_INSTANCES) != 0);
    amxc_var_add_key(bool, &args, "events", (flags & AMXB_FLAG_EVENTS) != 0);
    amxc_var_add_key(uint32_t, &args, "access", access);

    retval = amxb_rbus_invoke_root(amxb_rbus_ctx, object, "_describe", &args, request);

exit:
    request->result = NULL;
    amxb_close_request(&request);
    amxc_var_clean(&args);
    return retval;
}

static void amxb_rbus_add_sub_name(amxd_path_t* full_path,
                                   uint32_t flags,
                                   rbusElementInfo_t* element,
                                   amxc_var_t* object_data) {
    amxc_var_t* sub_list_instances = GET_ARG(object_data, "instances");
    amxc_var_t* sub_list_objects = GET_ARG(object_data, "objects");
    const char* requested_path = amxd_path_get(full_path, AMXD_OBJECT_TERMINATE);
    amxc_string_t name;

    amxc_string_init(&name, 0);
    amxc_string_set(&name, element->name);
    if((sub_list_instances == NULL) && ((flags & AMXB_FLAG_INSTANCES) != 0)) {
        // If the list was not added yet, add an empty one
        if(GET_UINT32(object_data, "type_id") == amxd_object_template) {
            sub_list_instances = amxc_var_add_key(amxc_llist_t, object_data, "instances", NULL);
        }
    }
    if((sub_list_objects == NULL) && ((flags & AMXB_FLAG_OBJECTS) != 0)) {
        // If the list was not added yet, add an empty one
        if(GET_UINT32(object_data, "type_id") != amxd_object_template) {
            sub_list_objects = amxc_var_add_key(amxc_llist_t, object_data, "objects", NULL);
        }
    }

    // The provided element should be one up in the hierarchy.
    // Removing the requested path and stripping any dots on the left and right
    // will provide the instance index or sub-object name.
    amxc_string_remove_at(&name, 0, strlen(requested_path));
    amxc_string_trim(&name, isdot);

    // If the correct flag is not set, the list used will be NULL and the
    // amxc_var_add will not do anything.
    if(amxc_string_is_numeric(&name)) {
        amxc_var_add(cstring_t, sub_list_instances, amxc_string_get(&name, 0));
    } else {
        amxc_var_add(cstring_t, sub_list_objects, amxc_string_get(&name, 0));
    }

    amxc_string_clean(&name);
}

static void amxb_rbus_get_row_info(amxb_rbus_t* amxb_rbus_ctx,
                                   amxd_path_t* full_path,
                                   amxc_var_t* object_data) {
    char* part = amxd_path_get_last(full_path, true);
    rbusRowName_t* rows = NULL;
    rbusRowName_t* row = NULL;
    // The function "rbusElementInfo_get" doesn't provide the instance number
    // and alias (object name) if available. There for an extra request is done
    // to get the row information of the parent object.
    int retval = rbusTable_getRowNames(amxb_rbus_ctx->handle, amxd_path_get(full_path, AMXD_OBJECT_TERMINATE), &rows);
    amxc_var_add_key(cstring_t, object_data, "path", amxd_path_get(full_path, AMXD_OBJECT_TERMINATE));
    // re-add the index to the full path
    amxd_path_append(full_path, part, true);
    free(part);

    if(retval == 0) {
        for(row = rows; row != NULL; row = row->next) {
            if(strcmp(row->name, amxd_path_get(full_path, AMXD_OBJECT_TERMINATE)) == 0) {
                // The matching row is found, add the information the the object info
                // structure (composite variant)
                if(row->alias != NULL) {
                    amxc_var_add_key(cstring_t, object_data, "name", row->alias);
                } else {
                    amxc_string_t index_name;
                    amxc_string_init(&index_name, 0);
                    amxc_string_setf(&index_name, "%d", row->instNum);
                    amxc_var_add_key(cstring_t, object_data, "name", amxc_string_get(&index_name, 0));
                    amxc_string_clean(&index_name);
                }
                amxc_var_add_key(uint32_t, object_data, "index", row->instNum);
                // stop the loop, found what was needed.
                break;
            }
        }
    }

    if(rows != NULL) {
        rbusTable_freeRowNames(amxb_rbus_ctx->handle, rows);
    }
}

static void amxb_rbus_add_object_name(amxd_path_t* full_path, amxc_var_t* object_data) {
    char* last = amxd_path_get_last(full_path, AMXD_OBJECT_TERMINATE);
    amxc_string_t name;

    amxc_string_init(&name, 0);
    amxc_string_set(&name, last);
    amxc_string_trim(&name, isdot);

    // The name of the object must be added as well.
    // Take the last part of the full path as the object name.
    amxc_var_add_key(cstring_t, object_data, "name", amxc_string_get(&name, 0));
    amxd_path_append(full_path, amxc_string_get(&name, 0), true);
    free(last);
    amxc_string_clean(&name);
}

static void amxb_rbus_add_parameter(amxb_rbus_t* amxb_rbus_ctx,
                                    UNUSED amxd_path_t* full_path,
                                    uint32_t flags,
                                    UNUSED uint32_t access,
                                    UNUSED amxd_object_type_t type,
                                    rbusElementInfo_t* element,
                                    amxc_var_t* object_data) {
    amxc_var_t* parameters = GET_ARG(object_data, "parameters");
    amxc_var_t* parameter = NULL;
    rbusValue_t value = NULL;
    amxc_var_t* var_helper = NULL;
    const char* name = NULL;

    when_true((flags & AMXB_FLAG_PARAMETERS) == 0, exit);
    // If the list was not added yet, add an empty one
    if(parameters == NULL) {
        parameters = amxc_var_add_key(amxc_htable_t, object_data, "parameters", NULL);
    }

    name = strrchr(element->name, '.') + 1;
    parameter = amxc_var_add_key(amxc_htable_t, parameters, name, NULL);
    rbus_get(amxb_rbus_ctx->handle, element->name, &value);
    amxc_var_add_key(cstring_t, parameter, "name", name);
    var_helper = amxc_var_add_new_key(parameter, "value");
    amxb_rbus_value_to_var(var_helper, value);
    amxc_var_add_key(uint32_t, parameter, "type_id", amxc_var_type_of(var_helper));
    amxc_var_add_key(cstring_t, parameter, "type_name", amxc_var_type_name_of(var_helper));
    var_helper = amxc_var_add_key(amxc_htable_t, parameter, "attributes", NULL);
    amxc_var_add_key(bool, var_helper, "read-only", (element->access & RBUS_ACCESS_SET) == 0);
    rbusValue_Release(value);

exit:
    return;
}

static void amxb_rbus_add_method(UNUSED amxb_rbus_t* amxb_rbus_ctx,
                                 UNUSED amxd_path_t* full_path,
                                 uint32_t flags,
                                 uint32_t access,
                                 UNUSED amxd_object_type_t type,
                                 rbusElementInfo_t* element,
                                 amxc_var_t* object_data) {
    amxc_var_t* functions = GET_ARG(object_data, "functions");
    amxc_var_t* function = NULL;
    const char* name = NULL;
    amxc_string_t stripped_name;

    amxc_string_init(&stripped_name, 0);
    when_true((flags & AMXB_FLAG_FUNCTIONS) == 0, exit);

    if(functions == NULL) {
        functions = amxc_var_add_key(amxc_htable_t, object_data, "functions", NULL);
    }

    name = strrchr(element->name, '.') + 1;
    // The internal data model functions (the ones starting with a "_"), can
    // be added to the list of methods, for now just drop these.

    // One option is to check if the query access was set to "protected" and
    // only drop it it is not protected.
    if(access != amxd_dm_access_protected) {
        if(name[0] == '_') {
            goto exit;
        }
    }

    amxc_string_set(&stripped_name, name);
    amxc_string_trimr(&stripped_name, isbraces);

    function = amxc_var_add_key(amxc_htable_t, functions, amxc_string_get(&stripped_name, 0), NULL);
    amxc_var_add_key(cstring_t, function, "name", amxc_string_get(&stripped_name, 0));
    amxc_var_add_key(uint32_t, function, "type_id", amxc_var_type_of(0));
    amxc_var_add_key(cstring_t, function, "type_name", amxc_var_get_type_name_from_id(0));
    amxc_var_add_key(amxc_htable_t, function, "attributes", NULL);
    // Note that it is not possible to known the arguements of the RPC method or the
    // return type. RBus doesn't provide that information.

exit:
    amxc_string_clean(&stripped_name);
    return;
}

static void amxb_rbus_add_event(UNUSED amxb_rbus_t* amxb_rbus_ctx,
                                UNUSED amxd_path_t* full_path,
                                UNUSED uint32_t flags,
                                UNUSED uint32_t access,
                                UNUSED amxd_object_type_t type,
                                rbusElementInfo_t* element,
                                amxc_var_t* object_data) {
    amxc_var_t* events = GET_ARG(object_data, "events");
    const char* name = NULL;

    when_true((flags & AMXB_FLAG_EVENTS) == 0, exit);

    if(events == NULL) {
        events = amxc_var_add_key(amxc_htable_t, object_data, "events", NULL);
    }

    name = strrchr(element->name, '.') + 1;
    // Always drop the "amx_notify!" event.
    // This event is mainly used to encapsulate the amx data model events.
    // And is not immediatly part of the public data model api.
    if(strcmp(name, "amx_notify!") == 0) {
        goto exit;
    }

    // Note that it is not poissble to add any event parameters that could be
    // available in the event as Rbus doesn't provide that information.
    amxc_var_add_key(amxc_llist_t, events, name, NULL);

exit:
    return;
}

static void amxb_rbus_add_object_info(amxb_rbus_t* amxb_rbus_ctx,
                                      amxd_path_t* full_path,
                                      uint32_t flags,
                                      UNUSED uint32_t access,
                                      amxd_object_type_t type,
                                      rbusElementInfo_t* element,
                                      amxc_var_t* object_data) {
    const char* requested_path = amxd_path_get(full_path, AMXD_OBJECT_TERMINATE);
    amxc_var_add_key(uint32_t, object_data, "type_id", type);
    amxc_var_add_key(amxc_htable_t, object_data, "attributes", NULL);

    if(strcmp(requested_path, element->name) == 0) {
        // Add the requested object information
        switch(type) {
        case amxd_object_instance:
            amxc_var_add_key(cstring_t, object_data, "type_name", "instance");
            amxb_rbus_get_row_info(amxb_rbus_ctx, full_path, object_data);
            break;
        case amxd_object_template:
            amxc_var_add_key(cstring_t, object_data, "path", element->name);
            amxc_var_add_key(cstring_t, object_data, "type_name", "template");
            amxb_rbus_add_object_name(full_path, object_data);
            break;
        default:
        case amxd_object_singleton:
            amxc_var_add_key(cstring_t, object_data, "path", element->name);
            amxc_var_add_key(cstring_t, object_data, "type_name", "singleton");
            amxb_rbus_add_object_name(full_path, object_data);
            break;
        }
    } else if(((flags & (AMXB_FLAG_INSTANCES | AMXB_FLAG_OBJECTS)) != 0)) {
        amxb_rbus_add_sub_name(full_path, flags, element, object_data);
    }
}

static int amxb_rbus_collect_info(amxb_rbus_t* amxb_rbus_ctx,
                                  const char* object,
                                  const char* rel_path,
                                  uint32_t access,
                                  uint32_t flags,
                                  amxc_var_t* values) {
    int retval = -1;
    const uint32_t bitmask = (AMXB_FLAG_EVENTS |
                              AMXB_FLAG_FUNCTIONS |
                              AMXB_FLAG_INSTANCES |
                              AMXB_FLAG_OBJECTS |
                              AMXB_FLAG_PARAMETERS);
    amxd_object_type_t parent_type = amxd_object_singleton;
    amxd_path_t full_path;
    char* part = NULL;
    rbusElementInfo_t* element = NULL;
    rbusElementInfo_t* elems = NULL;
    amxd_object_type_t type = amxd_object_singleton;
    amxb_rbus_add_info_t info_fn[5] = {
        amxb_rbus_add_object_info,                                // 0 = instance or object
        amxb_rbus_add_parameter,                                  // RBUS_ELEMENT_TYPE_PROPERTY
        amxb_rbus_add_object_info,                                // RBUS_ELEMENT_TYPE_TABLE
        amxb_rbus_add_event,                                      // RBUS_ELEMENT_TYPE_EVENT
        amxb_rbus_add_method                                      // RBUS_ELEMENT_TYPE_METHOD
    };

    amxd_path_init(&full_path, object);
    amxd_path_append(&full_path, rel_path, true);

    // To know what kind of object the current object is, it is needed to retrieve
    // the parent object.
    // If the parent object is a rbus table, then it is assumed that the current
    // object will be an rbus row. Table object is translated to multi-instance
    // object and a row is translated to an instance object.
    part = amxd_path_get_last(&full_path, true);
    rbusElementInfo_get(amxb_rbus_ctx->handle, amxd_path_get(&full_path, AMXD_OBJECT_TERMINATE), 0, &element);
    if(element != NULL) {
        if(element->type == RBUS_ELEMENT_TYPE_TABLE) {
            parent_type = amxd_object_template;
        }
        rbusElementInfo_free(amxb_rbus_ctx->handle, element);
    }
    amxd_path_append(&full_path, part, true);
    free(part);

    // retrieve the element info for the requested path.
    retval = rbusElementInfo_get(amxb_rbus_ctx->handle, amxd_path_get(&full_path, AMXD_OBJECT_TERMINATE), 0, &element);
    retval = amxb_rbus_translate_rbus2status(retval);
    when_failed(retval, exit);
    when_null_status(element, exit, retval = amxd_status_object_not_found);
    type = (parent_type == amxd_object_template)? amxd_object_instance:amxd_object_singleton;
    if(element->type == RBUS_ELEMENT_TYPE_TABLE) {
        type = amxd_object_template;
    }
    amxb_rbus_add_object_info(amxb_rbus_ctx, &full_path, flags, access, type, element, values);
    if(element->type == RBUS_ELEMENT_TYPE_TABLE) {
        parent_type = amxd_object_template;
    }
    rbusElementInfo_free(amxb_rbus_ctx->handle, element);

    when_true((flags & bitmask) == 0, exit);
    retval = rbusElementInfo_get(amxb_rbus_ctx->handle, amxd_path_get(&full_path, AMXD_OBJECT_TERMINATE), -1, &elems);
    when_null_status(element, exit, retval = amxd_status_object_not_found);

    // Loop over all elements and call the appropiate function to fill the
    // object info structure (= a composite amxc_variant)
    for(element = elems; element != NULL; element = element->next) {
        type = (parent_type == amxd_object_template)? amxd_object_instance:amxd_object_singleton;
        // Check if the element type is in the expected range.
        // There is no emum value available for type 0 !
        if((element->type >= 0) && (element->type <= RBUS_ELEMENT_TYPE_METHOD)) {
            if(element->type == RBUS_ELEMENT_TYPE_TABLE) {
                type = amxd_object_template;
            }
            // Call the correct function
            info_fn[element->type](amxb_rbus_ctx, &full_path, flags, access, type, element, values);
        }
    }

exit:
    if(elems != NULL) {
        rbusElementInfo_free(amxb_rbus_ctx->handle, elems);
    }
    amxd_path_clean(&full_path);
    return retval;
}

int amxb_rbus_describe(void* const ctx,
                       const char* object,
                       const char* rel_path,
                       uint32_t flags,
                       uint32_t access,
                       amxc_var_t* ret,
                       int timeout) {
    // The describe doesn't support search, wildcard paths or parameter paths.
    // object + rel_path = full_path
    int retval = -1;
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    // Always check if it is an ambiorix provided data model, this to be able
    // to get all information.
    // So ignore the setting "use-amx_calls" here
    const amxc_var_t* check_amx = amxb_rbus_get_config_option("use-amx-calls");

    amxc_var_set_type(ret, AMXC_VAR_ID_LIST);

    // apply the timeout
    amxb_rbus_set_timeout(timeout);

    // When the data model provider that provides this object is an ambiorix based
    // provider, it is better to call "_describe" function on the object.
    // It is not possible to retrieve the same meta-information for the object
    // using rbus API.
    if(GET_BOOL(check_amx, NULL) && amxb_rbus_remote_is_amx(amxb_rbus_ctx, object)) {
        retval = amxb_rbus_call_describe(amxb_rbus_ctx, object, rel_path, flags, access, ret);
    } else {
        amxc_var_t* description = amxc_var_add(amxc_htable_t, ret, NULL);
        retval = amxb_rbus_collect_info(amxb_rbus_ctx, object, rel_path, access, flags, description);
    }

    return retval;
}
