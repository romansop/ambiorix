/****************************************************************************
**
** Copyright (c) 2020 SoftAtHome
**
** Redistribution and use in source and binary forms, with or
** without modification, are permitted provided that the following
** conditions are met:
**
** 1. Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** 2. Redistributions in binary form must reproduce the above
** copyright notice, this list of conditions and the following
** disclaimer in the documentation and/or other materials provided
** with the distribution.
**
** Subject to the terms and conditions of this license, each
** copyright holder and contributor hereby grants to those receiving
** rights under this license a perpetual, worldwide, non-exclusive,
** no-charge, royalty-free, irrevocable (except for failure to
** satisfy the conditions of this license) patent license to make,
** have made, use, offer to sell, sell, import, and otherwise
** transfer this software, where such license applies only to those
** patent claims, already acquired or hereafter acquired, licensable
** by such copyright holder or contributor that are necessarily
** infringed by:
**
** (a) their Contribution(s) (the licensed copyrights of copyright
** holders and non-copyrightable additions of contributors, in
** source or binary form) alone; or
**
** (b) combination of their Contribution(s) with the work of
** authorship to which such Contribution(s) was added by such
** copyright holder or contributor, if, at the time the Contribution
** is added, such addition causes such combination to be necessarily
** infringed. The patent license shall not apply to any other
** combinations which include the Contribution.
**
** Except as expressly stated above, no rights or licenses from any
** copyright holder or contributor is granted under this license,
** whether expressly, by implication, estoppel or otherwise.
**
** DISCLAIMER
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
** CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
** USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
** AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
** ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "amxb_dbus_version.h"
#include "amxb_dbus.h"
#include "amxb_dbus_methods.h"

typedef struct _amxb_dbus_method {
    const char* interface;
    const char* dbus_method;
    amxb_dbus_func_t fn;
    const char* amx_method;
    const char** amx_args;
} amxb_dbus_method_t;

static void amxb_dbus_send(const amxc_var_t* const data, void* const priv) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) priv;
    const amxc_var_t* interface = amxb_dbus_get_config_option("dm-interface");
    DBusMessageIter dbus_args;
    DBusMessage* msg = dbus_message_new_signal("/", GET_CHAR(interface, NULL), "dmevent");

    dbus_message_iter_init_append(msg, &dbus_args);
    amxb_var_to_dbus(data, &dbus_args);

    if(amxb_dbus_ctx->dbus_handle) {
        if(!dbus_connection_send(amxb_dbus_ctx->dbus_handle, msg, NULL)) {
            fprintf(stderr, "Out Of Memory!\n");
        }
        dbus_connection_flush(amxb_dbus_ctx->dbus_handle);
    }
    dbus_message_unref(msg);
}

static void amxb_dbus_send_signal(const char* const sig_name,
                                  const amxc_var_t* const data,
                                  void* const priv) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) priv;
    amxc_var_t dbus_event_data;

    amxc_var_init(&dbus_event_data);
    if((data != NULL) && (amxc_var_type_of(data) != AMXC_VAR_ID_NULL)) {
        amxc_var_copy(&dbus_event_data, data);
    } else {
        amxc_var_set_type(&dbus_event_data, AMXC_VAR_ID_HTABLE);
    }
    amxc_var_add_key(cstring_t, &dbus_event_data, "notification", sig_name);

    if(strncmp(sig_name, "app:", 4) == 0) {
        amxb_dbus_send(&dbus_event_data, amxb_dbus_ctx);
    } else {
        amxp_sigmngr_deferred_call(amxb_dbus_ctx->sigmngr, amxb_dbus_send, &dbus_event_data, amxb_dbus_ctx);
    }

    amxc_var_clean(&dbus_event_data);
}

static DBusHandlerResult amxb_dbus_dm_common_handler(amxb_dbus_t* amxb_dbus_ctx,
                                                     DBusMessage* message,
                                                     const char* amx_method,
                                                     const char* amx_args[],
                                                     amxc_var_t* args) {
    DBusHandlerResult result = DBUS_HANDLER_RESULT_HANDLED;
    amxd_status_t status = amxd_status_ok;
    DBusMessage* reply = NULL;
    amxd_dm_t* dm = amxb_dbus_ctx->dm;
    amxd_object_t* root = amxd_dm_get_root(dm);
    amxc_var_t ret;
    amxc_var_t fn_args;
    amxc_var_t* fn_ret = NULL;

    amxc_var_init(&ret);
    amxc_var_init(&fn_args);

    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    if(!(reply = dbus_message_new_method_return(message))) {
        goto exit;
    }

    amxd_dbus_dbus_args_to_amx_args(args, amx_args, &fn_args);
    fn_ret = amxc_var_add_new(&ret);
    status = amxd_object_invoke_function(root, amx_method, &fn_args, fn_ret);

    amxd_dbus_amx_out_to_dbus_args(reply, fn_ret, &fn_args, status);

exit:
    if(reply == NULL) {
        result = DBUS_HANDLER_RESULT_NEED_MEMORY;
    } else {
        if(!dbus_connection_send(amxb_dbus_ctx->dbus_handle, reply, NULL)) {
            result = DBUS_HANDLER_RESULT_NEED_MEMORY;
        }
        dbus_message_unref(reply);
    }
    amxc_var_clean(&ret);
    amxc_var_clean(&fn_args);
    return result;
}

static const char* get_args[] = {
    "rel_path",
    "parameters",
    "depth",
    "access",
    "filter",
    NULL
};

static const char* set_args[] = {
    "rel_path",
    "parameters",
    "oparameters",
    "access",
    "allow_partial",
    NULL
};

static const char* add_args[] = {
    "rel_path",
    "parameters",
    "index",
    "name",
    "access",
    NULL
};

static const char* del_args[] = {
    "rel_path",
    "index",
    "name",
    "access",
    NULL
};

static const char* gsdm_args[] = {
    "rel_path",
    "parameters",
    "functions",
    "events",
    "first_level_only",
    NULL
};

static const char* describe_args[] = {
    "rel_path",
    "parameters",
    "functions",
    "objects",
    "instances",
    "exists",
    "events",
    "access",
    NULL
};

static const char* execute_args[] = {
    "rel_path",
    "method",
    "args",
    NULL
};

static amxb_dbus_method_t dispatch[] = {
    { DBUS_INTERFACE_INTROSPECTABLE, "Introspect", amxb_dbus_introspect, NULL, NULL },
    { "", "get", amxb_dbus_dm_common_handler, "_get", get_args },
    { "", "set", amxb_dbus_dm_common_handler, "_set", set_args },
    { "", "add", amxb_dbus_dm_common_handler, "_add", add_args },
    { "", "del", amxb_dbus_dm_common_handler, "_del", del_args },
    { "", "gsdm", amxb_dbus_dm_common_handler, "_get_supported", gsdm_args },
    { "", "describe", amxb_dbus_dm_common_handler, "_describe", describe_args },
    { "", "execute", amxb_dbus_dm_common_handler, "_exec", execute_args },
    { NULL, NULL, NULL }
};

static DBusHandlerResult amxb_dbus_dispatch_call(amxb_dbus_t* amxb_dbus_ctx, DBusMessage* message) {
    amxc_var_t args;
    amxc_var_t* arg = NULL;
    DBusMessageIter iter;
    DBusHandlerResult rv = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_LIST);

    dbus_message_iter_init(message, &iter);
    while(dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_INVALID) {
        arg = amxc_var_add_new(&args);
        amxb_dbus_to_var(arg, &iter);
        dbus_message_iter_next(&iter);
    }

    for(int i = 0; dispatch[i].interface != NULL; i++) {
        const char* interface = dispatch[i].interface;
        if(*interface == 0) {
            interface = GET_CHAR(amxb_dbus_get_config_option("dm-interface"), NULL);
        }
        if(dbus_message_is_method_call(message, interface, dispatch[i].dbus_method)) {
            rv = dispatch[i].fn(amxb_dbus_ctx, message, dispatch[i].amx_method, dispatch[i].amx_args, &args);
            break;
        }
    }

    amxc_var_clean(&args);
    return rv;
}

static void amxb_dbus_handle_wait(const char* name) {
    amxc_string_t signal_name;
    const char* prefix = GET_CHAR(amxb_dbus_get_config_option("destination-prefix"), NULL);

    amxc_string_init(&signal_name, 0);
    when_false(strncmp(name, prefix, strlen(prefix)) == 0, exit);

    amxc_string_setf(&signal_name, "wait:%s.", name + strlen(prefix));
    amxp_sigmngr_emit_signal(NULL, amxc_string_get(&signal_name, 0), NULL);

exit:
    amxc_string_clean(&signal_name);
}

static void amxb_dbus_handle_signal(amxb_dbus_t* amxb_dbus_ctx,
                                    DBusMessage* message) {
    amxc_var_t args;
    amxc_var_t* arg = NULL;
    DBusMessageIter iter;
    const char* member = dbus_message_get_member(message);
    const char* path = NULL;
    const char* object = NULL;
    amxc_var_t* notification = NULL;

    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_LIST);

    dbus_message_iter_init(message, &iter);
    while(dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_INVALID) {
        arg = amxc_var_add_new(&args);
        amxb_dbus_to_var(arg, &iter);
        dbus_message_iter_next(&iter);
    }

    if(strcmp(member, "NameOwnerChanged") == 0) {
        amxc_var_for_each(service, &args) {
            const char* name = GET_CHAR(service, NULL);
            if(name[0] == ':') {
                continue;
            }
            amxb_dbus_handle_wait(name);
        }
    }
    when_false(strcmp(member, "dmevent") == 0, exit);

    notification = GETI_ARG(&args, 0);
    path = GET_CHAR(notification, "path");
    object = GET_CHAR(notification, "object");

    amxc_htable_for_each(it, &amxb_dbus_ctx->subscribers) {
        const char* key = amxc_htable_it_get_key(it);
        size_t len = strlen(key);
        if((path != NULL) && (strncmp(path, key, len) == 0)) {
            amxp_sigmngr_emit_signal(amxb_dbus_ctx->sigmngr, key, notification);
        } else if((object != NULL) && (strncmp(path, key, len) == 0)) {
            amxp_sigmngr_emit_signal(amxb_dbus_ctx->sigmngr, key, notification);
        }
    }

exit:
    amxc_var_clean(&args);
}

DBusHandlerResult amxb_dbus_handle_msg(UNUSED DBusConnection* connection,
                                       DBusMessage* message,
                                       void* user_data) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) user_data;
    DBusHandlerResult retval = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    switch(dbus_message_get_type(message)) {
    case DBUS_MESSAGE_TYPE_METHOD_CALL:
        retval = amxb_dbus_dispatch_call(amxb_dbus_ctx, message);
        break;
    case DBUS_MESSAGE_TYPE_SIGNAL:
        amxb_dbus_handle_signal(amxb_dbus_ctx, message);
        retval = DBUS_HANDLER_RESULT_HANDLED;
        break;
    default:
        // Message not handled
        break;
    }

    return retval;
}

int amxb_dbus_register(void* const ctx, amxd_dm_t* const dm) {
    int status = -1;
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) ctx;
    amxc_string_t path;
    DBusError err;
    int rv = 0;
    const amxc_var_t* prefix = NULL;
    amxc_string_t name;
    amxc_string_init(&name, 0);

    dbus_error_init(&err);
    amxc_string_init(&path, 0);

    when_not_null(amxb_dbus_ctx->dm, exit);
    amxb_dbus_ctx->dm = dm;
    prefix = amxb_dbus_get_config_option("destination-prefix");

    amxd_object_for_each(child, it, amxd_dm_get_root(dm)) {
        amxd_object_t* obj = amxc_container_of(it, amxd_object_t, it);
        amxc_string_setf(&name, "%s%s", GET_CHAR(prefix, NULL), amxd_object_get_name(obj, 0));
        rv = dbus_bus_request_name(amxb_dbus_ctx->dbus_handle, amxc_string_get(&name, 0), DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
        if(rv != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
            fprintf(stderr, "Failed to request name on bus: %s\n", err.message);
            goto exit;
        }
    }

    dbus_watch_handle(amxb_dbus_ctx->watch, DBUS_WATCH_READABLE | DBUS_WATCH_WRITABLE);
    status = dbus_connection_get_dispatch_status(amxb_dbus_ctx->dbus_handle);
    while(status == DBUS_DISPATCH_DATA_REMAINS) {
        status = dbus_connection_dispatch(amxb_dbus_ctx->dbus_handle);
    }

    amxp_slot_connect_filtered(&dm->sigmngr, ".*", NULL, amxb_dbus_send_signal, amxb_dbus_ctx);

    status = 0;

exit:
    amxc_string_clean(&name);
    amxc_string_clean(&path);
    dbus_error_free(&err);
    return status;
}