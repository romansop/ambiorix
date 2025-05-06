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
** to be necessarily infringed. The tent license shall not apply to any other
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

#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <ctype.h>

#include <amxc/amxc.h>
#include <amxp/amxp_signal.h>
#include <amxd/amxd_types.h>
#include <amxd/amxd_path.h>
#include <amxd/amxd_transaction.h>
#include <amxd/amxd_action.h>

#include <usp/uspl.h>
#include <uspi/uspi_subscription.h>

#include "amxb_usp.h"
#include "amxb_usp_la.h"

typedef bool (* send_event_check_fn_t)(const char* event_name,
                                       amxd_path_t* path,
                                       const amxc_llist_t* resolved,
                                       const amxc_var_t* event,
                                       amxc_var_t* event_list);

typedef struct _event_data {
    const char* event;
    const amxc_var_t* data;
    uspi_notify_type_t type;
    send_event_check_fn_t check_fn;
} event_data_t;

static void amxb_usp_add_changed_event(amxc_var_t* event_list, const char* path, amxc_var_t* parameter) {
    amxc_var_t* e = amxc_var_add(amxc_htable_t, event_list, NULL);
    amxc_var_t* f = amxc_var_add_new_key(e, "from");
    amxc_var_t* t = amxc_var_add_new_key(e, "to");
    amxc_var_add_key(cstring_t, e, "path", path);
    amxc_var_add_key(cstring_t, e, "param_name", amxc_var_key(parameter));
    amxc_var_copy(f, GET_ARG(parameter, "from"));
    amxc_var_copy(t, GET_ARG(parameter, "to"));
}

static void amxb_usp_build_changed_event_list(const amxc_var_t* event,
                                              const char* param_name,
                                              amxc_var_t* event_list) {
    const char* path = GET_CHAR(event, "path");
    if((param_name == NULL) || (*param_name == 0)) {
        amxc_var_t* parameters = GET_ARG(event, "parameters");
        amxc_var_for_each(parameter, parameters) {
            amxb_usp_add_changed_event(event_list, path, parameter);
        }
    } else {
        amxc_var_t* parameter = GET_ARG(event, "parameters");
        parameter = GET_ARG(parameter, param_name);
        amxb_usp_add_changed_event(event_list, path, parameter);
    }
}

static void amxb_usp_add_resolved_paths(amxc_string_t* expr, const amxc_llist_t* resolved) {
    const char* sep = "";
    amxc_llist_for_each(it, resolved) {
        amxc_string_t* p = amxc_container_of(it, amxc_string_t, it);
        amxc_string_appendf(expr, "%s'%s'", sep, amxc_string_get(p, 0));
        sep = ",";
    }
}

// can be used to verify if object-changed must be send
static bool amxb_usp_la_need_send_change_event(UNUSED const char* event_name,
                                               amxd_path_t* path,
                                               const amxc_llist_t* resolved,
                                               const amxc_var_t* event,
                                               amxc_var_t* event_list) {
    amxc_string_t expression;
    bool add_event = false;
    amxp_expr_t e;
    const char* param = amxd_path_get_param(path);

    amxc_string_init(&expression, 0);

    when_true(amxc_llist_is_empty(resolved), exit);

    if(param != NULL) {
        amxc_string_setf(&expression, "contains('parameters.%s') && path in [", param);
        amxb_usp_add_resolved_paths(&expression, resolved);
        amxc_string_append(&expression, "]", 1);
    } else {
        amxc_string_setf(&expression, "path starts with [");
        amxb_usp_add_resolved_paths(&expression, resolved);
        amxc_string_append(&expression, "]", 1);
    }
    amxp_expr_init(&e, amxc_string_get(&expression, 0));
    add_event = amxp_expr_eval_var(&e, event, NULL);
    amxp_expr_clean(&e);

    if(add_event) {
        amxb_usp_build_changed_event_list(event, param, event_list);
    }

    amxc_string_clean(&expression);

exit:
    // return false as all references must be checked.
    return false;
}

// can be used to verify if instance-add must be send
static bool amxb_usp_la_need_send_add_event(UNUSED const char* event_name,
                                            amxd_path_t* path,
                                            const amxc_llist_t* resolved,
                                            const amxc_var_t* event,
                                            UNUSED amxc_var_t* event_list) {
    bool retval = false;
    amxc_string_t expression;
    amxp_expr_t e;
    const char* param = amxd_path_get_param(path);

    when_not_null(param, exit);

    amxc_string_init(&expression, 0);
    amxc_string_setf(&expression, "path in [");
    amxb_usp_add_resolved_paths(&expression, resolved);
    amxc_string_append(&expression, "]", 1);
    amxp_expr_init(&e, amxc_string_get(&expression, 0));
    retval = amxp_expr_eval_var(&e, event, NULL);
    amxp_expr_clean(&e);

    amxc_string_clean(&expression);

exit:
    return retval;
}

// can be used to verify if instance-del must be send
static bool amxb_usp_la_need_send_del_event(UNUSED const char* event_name,
                                            amxd_path_t* path,
                                            const amxc_llist_t* resolved,
                                            const amxc_var_t* event,
                                            UNUSED amxc_var_t* event_list) {
    bool retval = false;
    const char* param = amxd_path_get_param(path);
    amxc_string_t full_path;

    when_not_null(param, exit);

    amxc_string_init(&full_path, 0);
    amxc_string_setf(&full_path, "%s%d.", GET_CHAR(event, "path"), GET_UINT32(event, "index"));
    amxc_llist_for_each(it, resolved) {
        amxc_var_t* p = amxc_container_of(it, amxc_var_t, lit);
        if(strcmp(GET_CHAR(p, NULL), amxc_string_get(&full_path, 0)) == 0) {
            retval = true;
            amxc_var_delete(&p);
            break;
        }
    }
    amxc_string_clean(&full_path);

exit:
    return retval;
}

static bool amxb_usp_la_need_send_event(const char* event_name,
                                        amxd_path_t* path,
                                        const amxc_llist_t* resolved,
                                        const amxc_var_t* event,
                                        UNUSED amxc_var_t* event_list) {
    amxc_string_t expression;
    bool retval = false;
    amxp_expr_t e;
    const char* param = amxd_path_get_param(path);

    if(param != NULL) {
        retval = (strcmp(event_name, param) == 0);
        when_false(retval, exit);
    }

    amxc_string_init(&expression, 0);
    amxc_string_setf(&expression, "path starts with [");
    amxb_usp_add_resolved_paths(&expression, resolved);
    amxc_string_append(&expression, "]", 1);
    amxp_expr_init(&e, amxc_string_get(&expression, 0));
    retval = amxp_expr_eval_var(&e, event, NULL);
    amxp_expr_clean(&e);
    amxc_string_clean(&expression);

exit:
    return retval;
}

static bool amxb_usp_la_need_send_operation_complete(UNUSED const char* event_name,
                                                     amxd_path_t* path,
                                                     const amxc_llist_t* resolved,
                                                     const amxc_var_t* event,
                                                     UNUSED amxc_var_t* event_list) {
    amxc_string_t expression;
    bool retval = false;
    amxp_expr_t e;
    const char* param = amxd_path_get_param(path);

    amxc_string_init(&expression, 0);
    amxc_string_setf(&expression, "path starts with [");
    amxb_usp_add_resolved_paths(&expression, resolved);
    amxc_string_append(&expression, "]", 1);
    if(param != NULL) {
        amxc_string_appendf(&expression, " && data.cmd == '%s'", param);
    }
    amxp_expr_init(&e, amxc_string_get(&expression, 0));
    retval = amxp_expr_eval_var(&e, event, NULL);
    amxp_expr_clean(&e);
    amxc_string_clean(&expression);

    return retval;
}

static int amxb_usp_la_send_usp_event(amxb_usp_t* ctx,
                                      const amxc_var_t* const data,
                                      uspi_notify_type_t type,
                                      const char* notification,
                                      amxd_object_t* object) {
    int retval = -1;
    uspl_tx_t* usp_tx = NULL;
    char* from_id = amxb_usp_get_from_id();
    char* to_id = amxb_usp_get_to_id(ctx);
    const char* id = NULL;
    amxc_var_t data_copy;

    amxc_var_init(&data_copy);

    when_str_empty(from_id, exit);
    when_str_empty(to_id, exit);

    retval = uspl_tx_new(&usp_tx, from_id, to_id);
    when_failed(retval, exit);

    id = amxc_var_constcast(cstring_t, amxd_object_get_param_value(object, "ID"));
    when_null(id, exit);

    // Need copy to modify constant variant
    amxc_var_copy(&data_copy, data);
    amxb_usp_call_translate(&data_copy, AMXB_USP_TRANSLATE_DATA);
    amxc_var_add_key(cstring_t, &data_copy, "notification", notification);

    retval = uspi_subscription_build_notify_v2(&data_copy, type, id, usp_tx, false);
    when_failed(retval, exit);

    retval = amxb_usp_build_and_send_tlv(ctx, usp_tx);
    when_failed(retval, exit);

exit:
    free(from_id);
    free(to_id);
    amxc_var_clean(&data_copy);
    uspl_tx_delete(&usp_tx);
    return retval;
}

static int amxb_usp_la_send_event(UNUSED amxd_object_t* object,
                                  amxd_object_t* mobject,
                                  void* priv) {
    amxb_usp_t* ctx = (amxb_usp_t*) mobject->priv;
    bool send_event = false;
    amxc_var_t references;
    event_data_t* event_data = (event_data_t*) priv;
    amxc_var_t event_list;
    amxd_path_t path;
    amxc_llist_t resolved;

    amxc_var_init(&references);
    amxc_var_init(&event_list);
    amxd_path_init(&path, NULL);
    amxc_llist_init(&resolved);
    when_null(ctx, exit);

    amxc_var_set_type(&event_list, AMXC_VAR_ID_LIST);
    if(event_data->type != notify_object_deletion) {
        amxc_var_convert(&references, amxd_object_get_param_value(mobject, "ReferenceList"), AMXC_VAR_ID_LIST);

        amxc_var_for_each(ref, &references) {
            const char* requested = NULL;
            const char* translated = NULL;

            amxc_llist_clean(&resolved, amxc_string_list_it_free);
            amxd_path_setf(&path, false, "%s", GET_CHAR(ref, NULL));
            amxb_usp_translate_path(&path, &requested, &translated);

            amxd_dm_resolve_pathf(ctx->dm, &resolved, "%s", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
            if(amxc_llist_is_empty(&resolved)) {
                amxd_dm_resolve_pathf(ctx->la_dm, &resolved, "%s", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
            }
            if(amxc_llist_is_empty(&resolved)) {
                continue;
            }
            send_event = event_data->check_fn(event_data->event, &path, &resolved, event_data->data, &event_list);
            if(send_event) {
                break;
            }
        }
    } else {
        amxd_param_t* param_def = amxd_object_get_param_def(mobject, "ReferenceList");
        amxc_var_t* list = (amxc_var_t*) param_def->priv;
        if(list != NULL) {
            const amxc_llist_t* l = amxc_var_constcast(amxc_llist_t, list);
            send_event = event_data->check_fn(event_data->event, &path, l, event_data->data, &event_list);
        }
    }
    send_event |= !amxc_llist_is_empty(amxc_var_constcast(amxc_llist_t, &event_list));
    when_false(send_event, exit);

    if(event_data->type == notify_value_changed) {
        amxc_var_for_each(event, &event_list) {
            amxb_usp_la_send_usp_event(ctx, event, event_data->type, event_data->event, mobject);
        }
    } else {
        amxb_usp_la_send_usp_event(ctx, event_data->data, event_data->type, event_data->event, mobject);
    }

exit:
    amxd_path_clean(&path);
    amxc_llist_clean(&resolved, amxc_string_list_it_free);
    amxc_var_clean(&event_list);
    amxc_var_clean(&references);
    return 0;
}

static int amxb_usp_la_update_list(UNUSED amxd_object_t* object,
                                   amxd_object_t* mobject,
                                   void* priv) {
    amxd_param_t* param_def = amxd_object_get_param_def(mobject, "ReferenceList");
    amxc_var_t references;
    amxd_path_t path;
    amxc_llist_t resolved;
    const char* data = (const char*) priv;
    amxb_usp_t* ctx = NULL;
    amxc_var_t* delete_list = NULL;

    when_null(param_def, exit);
    ctx = (amxb_usp_t*) mobject->priv;
    when_null(ctx, exit);

    amxc_var_init(&references);
    amxd_path_init(&path, NULL);
    amxc_llist_init(&resolved);

    amxc_var_convert(&references, &param_def->value, AMXC_VAR_ID_LIST);

    delete_list = (amxc_var_t*) param_def->priv;
    if(delete_list == NULL) {
        amxc_var_new(&delete_list);
        amxc_var_set_type(delete_list, AMXC_VAR_ID_LIST);
        param_def->priv = delete_list;
    }

    amxc_var_for_each(ref, &references) {
        const char* requested = NULL;
        const char* translated = NULL;
        bool matching = false;

        amxc_llist_clean(&resolved, amxc_string_list_it_free);
        amxd_path_setf(&path, false, "%s", GET_CHAR(ref, NULL));
        amxb_usp_translate_path(&path, &requested, &translated);
        amxd_dm_resolve_pathf(ctx->dm, &resolved, "%s", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
        if(amxc_llist_is_empty(&resolved)) {
            amxd_dm_resolve_pathf(ctx->la_dm, &resolved, "%s", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
        }
        amxc_llist_for_each(it, &resolved) {
            amxc_string_t* p = amxc_container_of(it, amxc_string_t, it);
            if(strcmp(data, amxc_string_get(p, 0)) == 0) {
                amxc_var_add(cstring_t, delete_list, data);
                matching = true;
                break;
            }
        }
        if(matching) {
            break;
        }
    }

    amxd_path_clean(&path);
    amxc_llist_clean(&resolved, amxc_string_list_it_free);
    amxc_var_clean(&references);

exit:
    return 0;
}

static void amxb_usp_la_update_deletion_list(UNUSED const char* const sig_name,
                                             const amxc_var_t* const data,
                                             UNUSED void* const priv) {
    const char* filter = "[NotifType == 'ObjectDeletion'].";
    amxd_dm_t* la_dm = amxb_usp_get_la();
    amxd_object_t* la_subscriptions = amxd_dm_findf(la_dm, "Device.LocalAgent.Subscription.");
    const char* path = GET_CHAR(data, NULL);
    amxd_object_for_all(la_subscriptions, filter, amxb_usp_la_update_list, (void*) path);
}

static void amxb_usp_la_dispatch_events(const char* const sig_name,
                                        const amxc_var_t* const data,
                                        UNUSED void* const priv) {
    amxd_dm_t* la_dm = amxb_usp_get_la();
    amxd_object_t* la_subscriptions = amxd_dm_findf(la_dm, "Device.LocalAgent.Subscription.");
    event_data_t event_data;
    const char* filter = "";

    event_data.event = sig_name;
    event_data.data = data;
    event_data.check_fn = NULL;

    if(strcmp(sig_name, "dm:object-changed") == 0) {
        filter = "[Enable == true && NotifType == 'ValueChange'].";
        event_data.type = notify_value_changed;
        event_data.check_fn = amxb_usp_la_need_send_change_event;
    } else if(strcmp(sig_name, "dm:instance-added") == 0) {
        filter = "[Enable == true && NotifType == 'ObjectCreation'].";
        event_data.type = notify_object_creation;
        event_data.check_fn = amxb_usp_la_need_send_add_event;
    } else if(strcmp(sig_name, "dm:instance-removed") == 0) {
        filter = "[Enable == true && NotifType == 'ObjectDeletion'].";
        event_data.type = notify_object_deletion;
        event_data.check_fn = amxb_usp_la_need_send_del_event;
    } else if(strcmp(sig_name, "dm:operation-complete") == 0) {
        filter = "[Enable == true && NotifType == 'OperationComplete'].";
        event_data.type = notify_operation_complete;
        event_data.check_fn = amxb_usp_la_need_send_operation_complete;
    } else if(sig_name[strlen(sig_name) - 1] == '!') {
        filter = "[Enable == true && NotifType == 'Event'].";
        event_data.type = notify_event;
        event_data.check_fn = amxb_usp_la_need_send_event;
    }

    if(event_data.check_fn != NULL) {
        amxd_object_for_all(la_subscriptions, filter, amxb_usp_la_send_event, (void*) &event_data);
    }
}

static char* amxb_usp_la_subs_create_id(amxd_object_t* object, uint32_t index) {
    uint32_t new_index = index == 0 ? (object->last_index + 1) : index;
    char* id = NULL;
    amxc_string_t default_value;
    amxc_string_init(&default_value, 0);
    amxc_string_setf(&default_value, "cpe-ID-%d", new_index);

    id = amxc_string_take_buffer(&default_value);
    amxc_string_clean(&default_value);

    return id;
}

static bool amxb_usp_la_subs_reference_matches_type(amxd_path_t* path, const char* type) {
    bool retval = true;
    const char* param = amxd_path_get_param(path);
    uint32_t len = 0;

    when_null(param, exit);
    len = strlen(param);

    if(param[len - 1] == '!') {
        // this is an event path
        retval = (strcmp(type, "Event") == 0);
    } else if((param[len - 2] == '(') && (param[len - 1] == ')')) {
        // this is a command path
        retval = (strcmp(type, "OperationComplete") == 0);
    } else {
        // a parameter path
        retval = (strcmp(type, "ValueChange") == 0);
    }

exit:
    return retval;
}

static bool amxb_usp_la_subs_reference_is_valid(amxd_dm_t* dm, amxc_var_t* ref, const char* type) {
    amxd_object_t* object = NULL;
    char* sup_path = NULL;
    bool retval = false;
    amxd_path_t path;
    const char* requested = NULL;
    const char* translated = NULL;

    amxd_path_init(&path, GET_CHAR(ref, NULL));
    amxb_usp_translate_path(&path, &requested, &translated);

    // check if reference path is valid
    when_false(amxd_path_is_valid(&path), exit);

    // is it referencing something in the supported data model
    sup_path = amxd_path_get_supported_path(&path);
    object = amxd_dm_findf(dm, "%s", sup_path);
    when_null(object, exit);

    // if it is a parameter, event or command path check it is matching with the given type
    //   parameter path => must be ValueChange
    //   event path => must be Event
    //   command path => must be OperationComplete
    when_false(amxb_usp_la_subs_reference_matches_type(&path, type), exit);

    if((strcmp(type, "ObjectDeletion") == 0) ||
       (strcmp(type, "ObjectCreation") == 0)) {
        // should poing to a multi-instance or instance object
        when_false(amxd_object_get_type(object) == amxd_object_template, exit);
        // should not be referencing a parameter, event or command
        when_not_null(amxd_path_get_param(&path), exit);
    }

    if(strcmp(type, "ObjectDeletion") == 0) {
        // make sure it is pointing to instance(s)
        char* last = amxd_path_get_last(&path, true);
        if((last[0] != '*') && (last[0] != '[') && (isdigit(last[0]) == 0)) {
            amxc_var_t all;
            amxc_var_init(&all);
            amxc_var_set(cstring_t, &all, "*.");
            amxc_var_add_value(ref, &all);
            amxc_var_clean(&all);
        }
        free(last);
    }

    retval = true;

exit:
    amxd_path_clean(&path);
    free(sup_path);
    return retval;
}

static bool amxb_usp_la_subs_are_references_valid(amxb_usp_t* ctx,
                                                  amxc_var_t* type,
                                                  amxc_var_t* ref_list) {
    bool valid = true;

    amxc_var_cast(ref_list, AMXC_VAR_ID_CSV_STRING);
    amxc_var_cast(ref_list, AMXC_VAR_ID_LIST);

    amxc_var_for_each(ref, ref_list) {
        const char* ref_str = GET_CHAR(ref, NULL);
        amxd_dm_t* dm = NULL;
        if((ref_str == NULL) || (*ref_str == 0)) {
            // empty string
            amxc_var_delete(&ref);
            continue;
        }
        if(strlen(ref_str) > 256) {
            // too long
            amxc_var_delete(&ref);
            continue;
        }
        if(strncmp(ref_str, "Device.LocalAgent.", 18) == 0) {
            dm = ctx->la_dm;
        } else {
            dm = ctx->dm;
        }

        if(!amxb_usp_la_subs_reference_is_valid(dm, ref, GET_CHAR(type, NULL))) {
            // invalid reference
            amxc_var_delete(&ref);
            continue;
        }
    }

    // if all invalid references are removed and the list is empty
    // make the add instance fail
    if(amxc_llist_is_empty(amxc_var_constcast(amxc_llist_t, ref_list))) {
        valid = false;
    }

    // cast the remaining list back to a string with comma separated values
    amxc_var_cast(ref_list, AMXC_VAR_ID_CSV_STRING);
    return valid;
}

amxd_status_t amxb_usp_la_subs_add_inst(amxd_object_t* const object,
                                        amxd_param_t* const p,
                                        amxd_action_t reason,
                                        const amxc_var_t* const args,
                                        amxc_var_t* const retval,
                                        void* priv) {
    amxd_status_t status = amxd_status_unknown_error;
    amxd_object_t* la_root = amxd_object_get_root(object);
    amxc_var_t* params = GET_ARG(args, "parameters");
    amxc_var_t* recipient = GET_ARG(params, "Recipient");
    amxc_var_t* type = GET_ARG(params, "NotifType");
    amxc_var_t* refs = GET_ARG(params, "ReferenceList");
    amxc_var_t* id = GET_ARG(params, "ID");
    const char* id_str = NULL;
    amxb_usp_t* ctx = (amxb_usp_t*) la_root->priv;
    amxd_object_t* la_controller = NULL;
    amxd_object_t* instance = NULL;
    char* path = NULL;

    when_null_status(ctx, exit, status = amxd_status_permission_denied);

    la_controller = amxd_object_findf(la_root, "Device.LocalAgent.Controller.[EndpointID == '%s'].", ctx->eid);
    path = amxd_object_get_path(la_controller, AMXD_OBJECT_INDEXED);

    // no registered controller found - return error
    when_null_status(la_controller, exit, status = amxd_status_permission_denied);

    // set or overwrite the Recipient
    if(recipient == NULL) {
        recipient = amxc_var_add_new_key(params, "Recipient");
    }
    amxc_var_set(cstring_t, recipient, path);

    // if no id was given, add one
    if(id == NULL) {
        id = amxc_var_add_new_key(params, "ID");
    }
    id_str = GET_CHAR(id, NULL);
    // if the id is an empty string, create one
    if((id_str == NULL) || (*id_str == 0)) {
        uint32_t index = GET_UINT32(args, "index") == 0 ? GET_UINT32(retval, "index") : GET_UINT32(args, "index");
        amxc_var_push(cstring_t, id, amxb_usp_la_subs_create_id(object, index));
    }

    status = amxd_status_invalid_value;
    // Is a value provided for NotifType?
    when_null(type, exit);
    // Is a reference list provided?
    when_null(refs, exit);
    // Verify reference list
    when_false(amxb_usp_la_subs_are_references_valid(ctx, type, refs), exit);

    // call default implementation - reason code is checked in default implementation
    status = amxd_action_object_add_inst(object, p, reason, args, retval, priv);
    when_failed(status, exit);

    instance = amxd_object_get_instance(object, NULL, GET_UINT32(retval, "index"));
    instance->priv = ctx;

exit:
    free(path);
    return status;
}

amxd_status_t amxb_usp_la_subs_remove_matching(UNUSED amxd_object_t* const object,
                                               amxd_param_t* const p,
                                               amxd_action_t reason,
                                               UNUSED const amxc_var_t* const args,
                                               UNUSED amxc_var_t* const retval,
                                               UNUSED void* priv) {
    amxc_var_t* list = NULL;
    amxd_status_t status = amxd_status_function_not_implemented;

    when_false(reason == action_param_destroy, exit);
    when_null(p, exit);

    status = amxd_status_ok;
    list = (amxc_var_t*) p->priv;
    amxc_var_delete(&list);

exit:
    return status;
}

void amxb_usp_la_subscription_added(UNUSED const char* const sig_name,
                                    const amxc_var_t* const data,
                                    UNUSED void* const priv) {
    amxd_object_t* subs = amxd_dm_signal_get_object(amxb_usp_get_la(), data);
    amxd_object_t* subs_inst = amxd_object_get_instance(subs, NULL, GET_UINT32(data, "index"));
    amxb_usp_t* ctx = NULL;

    when_null(subs_inst, exit);
    ctx = (amxb_usp_t*) subs_inst->priv;
    when_null(ctx, exit);
    when_null(ctx->dm, exit);

    amxp_sigmngr_add_signal(NULL, "dm:deletion-start");
    // no need to check if the function is allready connected.
    // slot functions without private data or expression can only be added once
    amxp_slot_connect(&ctx->dm->sigmngr, "*", NULL, amxb_usp_la_dispatch_events, NULL);
    amxp_slot_connect(&ctx->la_dm->sigmngr, "*", NULL, amxb_usp_la_dispatch_events, NULL);
    amxp_slot_connect(NULL, "dm:deletion-start", NULL, amxb_usp_la_update_deletion_list, NULL);

exit:
    return;
}
