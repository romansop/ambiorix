/****************************************************************************
**
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
** SPDX-FileCopyrightText: Copyright (c) 2024 SoftAtHome
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "amxb_rbus.h"
#include "amxb_rbus_ctrl.h"
#include "amxb_rbus_handlers.h"

typedef rbusError_t (* amxb_rbus_reg_func_t) (rbusHandle_t handle,
                                              int numDataElements,
                                              rbusDataElement_t* elements);

static amxd_status_t amxb_rbus_object_destroy(amxd_object_t* const object,
                                              amxd_param_t* const param,
                                              amxd_action_t reason,
                                              const amxc_var_t* const args,
                                              amxc_var_t* const retval,
                                              void* priv);

static bool amxb_rbus_is_instantiated(amxd_object_t* const object) {
    bool instantiated = false;

    amxd_object_t* parent = object;
    while(parent != NULL) {
        if(amxd_object_get_type(parent) == amxd_object_instance) {
            instantiated = true;
            break;
        }
        parent = amxd_object_get_parent(parent);
    }

    return instantiated;
}

static int amxb_rbus_register_object_funcs(amxb_rbus_t* amxb_rbus_ctx,
                                           amxd_object_t* object,
                                           const char* path,
                                           rbusDataElement_t* element,
                                           amxb_rbus_reg_func_t fn) {
    amxc_var_t functions;
    amxc_string_t full_name;
    int len = 0;
    const amxc_var_t* check_amx = amxb_rbus_get_config_option("use-amx-calls");
    int retval = 0;

    amxc_var_init(&functions);
    amxc_string_init(&full_name, 0);

    element->type = RBUS_ELEMENT_TYPE_METHOD;
    element->cbTable.eventSubHandler = NULL;
    element->cbTable.methodHandler = amxb_rbus_call_method;
    element->cbTable.getHandler = NULL;
    element->cbTable.setHandler = NULL;
    element->cbTable.tableAddRowHandler = NULL;
    element->cbTable.tableRemoveRowHandler = NULL;

    // if option use-amx-calls is set to true, also register all protected methods.
    // This includes all internal ambiorix data model methods, starting with _
    if(GET_BOOL(check_amx, NULL)) {
        amxd_object_list_functions(object, &functions, amxd_dm_access_protected);
    } else {
        amxd_object_list_functions(object, &functions, amxd_dm_access_public);
    }

    // loop over all the data model methods and register them.
    // when one of the registered methods are called by a data model consumer
    // the function amxb_rbus_call_method is called to invoke the implementation
    amxc_var_for_each(function, &functions) {
        amxc_string_setf(&full_name, "%s%s()", path, GET_CHAR(function, NULL));
        len = amxc_string_buffer_length(&full_name);
        element->name = amxc_string_take_buffer(&full_name);
        retval = fn(amxb_rbus_ctx->handle, 1, element);
        amxc_string_push_buffer(&full_name, element->name, len);
        when_failed(retval, exit);
    }

exit:
    amxc_string_clean(&full_name);
    amxc_var_clean(&functions);
    return retval;
}

static int amxb_rbus_register_object_params(amxb_rbus_t* amxb_rbus_ctx,
                                            amxc_var_t* object_meta,
                                            const char* path,
                                            rbusDataElement_t* element,
                                            amxb_rbus_reg_func_t fn) {
    amxc_var_t* params = NULL;
    amxc_string_t full_name;
    int len = 0;
    int retval = 0;

    amxc_string_init(&full_name, 0);

    element->type = RBUS_ELEMENT_TYPE_PROPERTY;
    element->cbTable.eventSubHandler = NULL;
    element->cbTable.getHandler = amxb_rbus_get_value;
    element->cbTable.tableAddRowHandler = NULL;
    element->cbTable.tableRemoveRowHandler = NULL;
    element->cbTable.methodHandler = NULL;

    // When the parameter is marked as read-only, the setHandler must be set to NULL.
    //
    // Parameters can be defined as protected, private or public.
    // Currently only register the public parameters
    params = GET_ARG(object_meta, "parameters");

    amxc_var_for_each(param, params) {
        const char* name = amxc_var_key(param);
        if(GETP_BOOL(param, "attributes.protected") ||
           GETP_BOOL(param, "attributes.private")) {
            continue;
        }
        if(GETP_BOOL(param, "attributes.read-only")) {
            element->cbTable.setHandler = NULL;
        } else {
            element->cbTable.setHandler = amxb_rbus_set_value;
        }
        amxc_string_setf(&full_name, "%s%s", path, name);
        len = amxc_string_buffer_length(&full_name);
        element->name = amxc_string_take_buffer(&full_name);
        retval = fn(amxb_rbus_ctx->handle, 1, element);
        amxc_string_push_buffer(&full_name, element->name, len);
        when_failed(retval, exit);
    }

exit:
    amxc_string_clean(&full_name);
    return retval;
}

static int amxb_rbus_register_object_events(amxb_rbus_t* amxb_rbus_ctx,
                                            amxc_var_t* object_meta,
                                            const char* path,
                                            rbusDataElement_t* element,
                                            amxb_rbus_reg_func_t fn) {
    amxc_var_t* events;
    amxc_string_t full_name;
    int len = 0;
    int retval = 0;

    amxc_string_init(&full_name, 0);

    element->type = RBUS_ELEMENT_TYPE_PROPERTY;
    element->cbTable.eventSubHandler = amxb_rbus_subscribe_handler;
    element->cbTable.methodHandler = NULL;
    element->cbTable.getHandler = NULL;
    element->cbTable.setHandler = NULL;
    element->cbTable.tableAddRowHandler = NULL;
    element->cbTable.tableRemoveRowHandler = NULL;

    // add amx_notify! event, this event is used to encapsulate the default
    // ambiorix data model events.
    amxc_string_setf(&full_name, "%samx_notify!", path);
    element->type = RBUS_ELEMENT_TYPE_EVENT;
    len = amxc_string_buffer_length(&full_name);
    element->name = amxc_string_take_buffer(&full_name);
    retval = fn(amxb_rbus_ctx->handle, 1, element);
    amxc_string_push_buffer(&full_name, element->name, len);
    when_failed(retval, exit);

    events = GET_ARG(object_meta, "events");

    // register the custom events which are defined in the data model
    amxc_var_for_each(event, events) {
        const char* name = amxc_var_key(event);
        amxc_string_setf(&full_name, "%s%s", path, name);
        len = amxc_string_buffer_length(&full_name);
        element->name = amxc_string_take_buffer(&full_name);
        retval = fn(amxb_rbus_ctx->handle, 1, element);
        amxc_string_push_buffer(&full_name, element->name, len);
        when_failed(retval, exit);
    }

exit:
    amxc_string_clean(&full_name);
    return retval;
}

static void amxb_rbus_register_row(amxb_rbus_t* amxb_rbus_ctx,
                                   amxd_object_t* const object) {
    const char* object_name = amxd_object_get_name(object, AMXD_OBJECT_NAMED);
    char* table_path = amxd_object_get_path(amxd_object_get_parent(object), AMXD_OBJECT_INDEXED | AMXD_OBJECT_TERMINATE);
    uint32_t index = amxd_object_get_index(object);

    table_path = amxb_rbus_translate_register_path(table_path);
    if(isdigit(object_name[0]) != 0) {
        object_name = NULL;
    }
    rbusTable_registerRow(amxb_rbus_ctx->handle, table_path, index, object_name);
    free(table_path);
}

static int amxb_rbus_register_table(amxb_rbus_t* amxb_rbus_ctx,
                                    amxc_var_t* object_meta,
                                    char* path,
                                    rbusDataElement_t* element,
                                    amxb_rbus_reg_func_t fn) {
    int retval = 0;
    element->name = path;
    element->type = RBUS_ELEMENT_TYPE_TABLE;
    element->cbTable.methodHandler = NULL;
    element->cbTable.getHandler = NULL;
    element->cbTable.setHandler = NULL;
    if(!GETP_BOOL(object_meta, "attributes.read-only")) {
        element->cbTable.tableAddRowHandler = amxb_rbus_add_row;
        element->cbTable.tableRemoveRowHandler = amxb_rbus_remove_row;
    } else {
        element->cbTable.tableAddRowHandler = NULL;
        element->cbTable.tableRemoveRowHandler = NULL;
    }
    element->cbTable.eventSubHandler = amxb_rbus_row_subscribe_handler;
    retval = fn(amxb_rbus_ctx->handle, 1, element);

    return retval;
}

static void amxb_rbus_register_object_impl(amxb_rbus_t* amxb_rbus_ctx,
                                           amxd_object_t* const object,
                                           bool reg) {
    char* path = NULL;
    const amxc_var_t* skip = amxb_rbus_get_config_option("skip-register");
    rbusDataElement_t element;
    amxc_var_t object_meta;
    amxb_rbus_reg_func_t fn = reg? rbus_regDataElements:rbus_unregDataElements;

    amxc_var_init(&object_meta);
    memset(&element, 0, sizeof(rbusDataElement_t));
    when_true(object->type == amxd_object_root, exit);

    path = amxd_object_get_path(object, AMXD_OBJECT_SUPPORTED | AMXD_OBJECT_TERMINATE);
    if((skip != NULL) && (amxc_var_type_of(skip) == AMXC_VAR_ID_HTABLE)) {
        when_true(GET_BOOL(skip, path), exit);
    }

    if(amxb_rbus_is_instantiated(object)) {
        if((amxd_object_get_type(object) == amxd_object_instance) && reg) {
            amxb_rbus_register_row(amxb_rbus_ctx, object);
        }
    } else {
        path = amxb_rbus_translate_register_path(path);
        amxd_object_describe(object, &object_meta, AMXD_OBJECT_PARAM | AMXD_OBJECT_EVENT, amxd_dm_access_protected);
        if(amxd_object_get_type(object) == amxd_object_template) {
            when_failed(amxb_rbus_register_table(amxb_rbus_ctx, &object_meta, path, &element, fn), exit);
        }
        when_failed(amxb_rbus_register_object_params(amxb_rbus_ctx, &object_meta, path, &element, fn), exit);
        when_failed(amxb_rbus_register_object_events(amxb_rbus_ctx, &object_meta, path, &element, fn), exit);
        when_failed(amxb_rbus_register_object_funcs(amxb_rbus_ctx, object, path, &element, fn), exit);
        if(reg) {
            // add destroy handler
            // the destroy handler is used to unregister the data elements
            amxd_object_add_action_cb(object, action_object_destroy, amxb_rbus_object_destroy, amxb_rbus_ctx);
        }
    }

exit:
    amxc_var_clean(&object_meta);
    free(path);
}

static amxd_status_t amxb_rbus_object_destroy(amxd_object_t* const object,
                                              UNUSED amxd_param_t* const param,
                                              amxd_action_t reason,
                                              UNUSED const amxc_var_t* const args,
                                              UNUSED amxc_var_t* const retval,
                                              UNUSED void* priv) {
    amxd_status_t status = amxd_status_ok;
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) priv;

    when_false_status(reason == action_object_destroy, exit, status = amxd_status_function_not_implemented);
    when_true(amxb_rbus_is_instantiated(object), exit);

    amxb_rbus_register_object_impl(amxb_rbus_ctx, object, false);

exit:
    return status;
}

static void amxb_rbus_register_object(amxd_object_t* const object,
                                      AMXB_UNUSED int32_t depth,
                                      void* priv) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) priv;
    amxb_rbus_register_object_impl(amxb_rbus_ctx, object, true);
}

static void amxb_rbus_unregister_object(amxd_object_t* const object,
                                        AMXB_UNUSED int32_t depth,
                                        void* priv) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) priv;
    amxb_rbus_register_object_impl(amxb_rbus_ctx, object, false);

    // remove destroy handler
    amxd_object_remove_action_cb(object, action_object_destroy, amxb_rbus_object_destroy);
}

static bool amxb_rbus_filter_object(amxd_object_t* const object,
                                    AMXB_UNUSED int32_t depth,
                                    AMXB_UNUSED void* priv) {
    bool retval = true;
    when_true(amxd_object_get_type(object) == amxd_object_root, exit);

    // skip private object.
    // private objects can only be used by the application that provides
    // the data model. Private objects are never accessible by other
    // applications
    if(amxd_object_is_attr_set(object, amxd_oattr_private)) {
        retval = false;
        goto exit;
    }

    // Should protected objects be skipped as well?
    // Protected objects can be registered, but as it is not possible to known
    // through rbus if it is a protected object or not, it will become a public
    // object.
    // Protected objects can be access by the provider of the object and its
    // friends. Friends are considered all applications and services running
    // on the same target. Other applications or services that are external,
    // that includes the ones running in a container, should not be able to
    // access these objects.
    //
    // Remove following code to enable registration of protected objects
    if(amxd_object_is_attr_set(object, amxd_oattr_protected)) {
        retval = false;
        goto exit;
    }

    // Private and Protected flags are not related to Access Control Lists.
    // ACLs will check if certain role can access the public data elements.

exit:
    return retval;
}

static void amxb_rbus_register_inst_add(UNUSED const char* const sig_name,
                                        const amxc_var_t* const data,
                                        void* const priv) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) priv;
    amxd_object_t* object = amxd_dm_signal_get_object(amxb_rbus_ctx->dm, data);
    uint32_t index = GET_UINT32(data, "index");
    char* path = NULL;
    amxc_var_t* ignore = NULL;

    object = amxd_object_get_instance(object, NULL, index);
    path = amxd_object_get_path(object, AMXD_OBJECT_TERMINATE | AMXD_OBJECT_INDEXED);
    ignore = GET_ARG(&amxb_rbus_ctx->ignore, path);

    if((ignore == NULL) && (object != NULL)) {
        amxb_rbus_register_row(amxb_rbus_ctx, object);
    }

    free(path);
    amxc_var_delete(&ignore);
}

static void amxb_rbus_register_inst_remove(UNUSED const char* const sig_name,
                                           const amxc_var_t* const data,
                                           void* const priv) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) priv;
    const char* object_path = GET_CHAR(data, "path");
    amxc_string_t full_path;
    amxc_var_t* ignore = NULL;
    uint32_t index = amxc_var_constcast(uint32_t, amxc_var_get_key(data, "index", 0));

    amxc_string_init(&full_path, 0);
    amxc_string_setf(&full_path, "%s%d.", object_path, index);
    ignore = GET_ARG(&amxb_rbus_ctx->ignore, amxc_string_get(&full_path, 0));

    if(ignore == NULL) {
        char* path = NULL;
        path = amxc_string_take_buffer(&full_path);
        path = amxb_rbus_translate_register_path(path);
        rbusTable_unregisterRow(amxb_rbus_ctx->handle, path);
        free(path);
    }

    amxc_var_delete(&ignore);
    amxc_string_clean(&full_path);
}

static void amxb_rbus_register_add_object(UNUSED const char* const sig_name,
                                          const amxc_var_t* const data,
                                          void* const priv) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) priv;
    amxd_object_t* object = amxd_dm_signal_get_object(amxb_rbus_ctx->dm, data);
    char* path = NULL;

    when_null(object, exit);
    // Do not register this object when initial registration is not done yet
    when_false(amxb_rbus_ctx->register_done, exit);
    // Do not register private objects
    when_true(amxd_object_is_attr_set(object, amxd_oattr_private), exit);
    // Do not register protected objects
    when_true(amxd_object_is_attr_set(object, amxd_oattr_protected), exit);
    // Do not register this object if it is in the instantiated data model
    when_true(amxb_rbus_is_instantiated(object), exit);
    path = amxd_object_get_path(object, AMXD_OBJECT_INDEXED | AMXD_OBJECT_TERMINATE);
    path = amxb_rbus_translate_register_path(path);
    // Do not register if already available
    when_true(amxd_object_has_action_cb(object, action_object_destroy, amxb_rbus_object_destroy), exit);
    amxb_rbus_register_object(object, 0, (void*) amxb_rbus_ctx);

exit:
    free(path);
    return;
}

static void amxb_rbus_register_dm(UNUSED const char* const sig_name,
                                  UNUSED const amxc_var_t* const data,
                                  void* const priv) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) priv;
    amxd_dm_t* dm = amxb_rbus_ctx->dm;

    // Loop over the full data model (top-down),
    amxd_object_hierarchy_walk(amxd_dm_get_root(dm),
                               amxd_direction_down,
                               amxb_rbus_filter_object,
                               amxb_rbus_register_object,
                               INT32_MAX,
                               amxb_rbus_ctx);

    amxp_slot_disconnect_with_priv(&dm->sigmngr, amxb_rbus_register_dm, amxb_rbus_ctx);

    // make sure the newly created instances are registered to rbus using rbusTable_registerRow
    amxp_slot_connect(&dm->sigmngr,
                      "dm:instance-added",
                      NULL,
                      amxb_rbus_register_inst_add,
                      amxb_rbus_ctx);
    // make sure the removed instances are unregisterd from rbus using rbusTable_unregisterRow
    amxp_slot_connect(&dm->sigmngr,
                      "dm:instance-removed",
                      NULL,
                      amxb_rbus_register_inst_remove,
                      amxb_rbus_ctx);

    // When extra object definitions are loaded or added in the supported data model
    // (multi-instance objects or singleton objects), they will needed to be
    // registered to rbus as well.
    amxp_slot_connect(&dm->sigmngr, "dm:object-added", NULL, amxb_rbus_register_add_object, amxb_rbus_ctx);

    // When object definitions are removed from the supported data model
    // (multi-instance objects or singleton objects), they will current not be
    // unregistered from rbus.
    // A destroy handler which is added to each object that is registered to rbus.
    // A destroy handler will be called just before the memory is freed.
    // Using an event handler is not possible. By the time the event
    // handler is called the object will be removed from memory, so all needed
    // information will be removed.

    // mark registration as done
    amxb_rbus_ctx->register_done = true;
}

int amxb_rbus_register(void* const ctx, amxd_dm_t* const dm) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    int status = 0;
    const amxc_var_t* cfg_ros = amxb_rbus_get_config_option("register-on-start-event");

    when_not_null(amxb_rbus_ctx->dm, exit);

    amxb_rbus_ctx->dm = dm;

    if(amxc_var_dyncast(bool, cfg_ros)) {
        amxp_slot_connect(&dm->sigmngr,
                          "app:start",
                          NULL,
                          amxb_rbus_register_dm,
                          amxb_rbus_ctx);
    } else {
        amxb_rbus_register_dm(NULL, NULL, ctx);
    }

    status = RBUS_ERROR_SUCCESS;

exit:
    return status;
}

void amxb_rbus_unregister(amxb_rbus_t* amxb_rbus_ctx) {
    when_null(amxb_rbus_ctx->dm, exit);

    // Loop over the full data model (top-don),
    amxd_object_hierarchy_walk(amxd_dm_get_root(amxb_rbus_ctx->dm),
                               amxd_direction_down,
                               amxb_rbus_filter_object,
                               amxb_rbus_unregister_object,
                               INT32_MAX,
                               amxb_rbus_ctx);

exit:
    return;
}