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

#include <string.h>

#include "amxb_dbus_version.h"
#include "amxb_dbus.h"
#include "amxb_dbus_methods.h"

static void amxb_dbus_fill_list(amxb_bus_ctx_t* bus_ctx,
                                const char* path,
                                amxc_var_t* data,
                                const char* type,
                                bool fetch_names,
                                amxc_var_t* retval) {
    amxc_string_t name;
    amxc_string_init(&name, 0);

    amxc_var_t* type_data = GETP_ARG(data, type);
    amxc_var_for_each(d, type_data) {
        if(amxc_var_type_of(type_data) == AMXC_VAR_ID_HTABLE) {
            amxc_string_setf(&name, "%s%s", path, amxc_var_key(d));
        } else {
            amxc_string_setf(&name, "%s%s", path, GET_CHAR(d, NULL));
        }
        amxc_var_add(cstring_t, retval, amxc_string_get(&name, 0));
        if(fetch_names) {
            amxc_var_t tmp;
            amxc_var_init(&tmp);
            amxb_describe(bus_ctx, amxc_string_get(&name, 0), 0, &tmp, 5);
            amxc_string_setf(&name, "%s%s", path, GETP_CHAR(&tmp, "0.name"));
            amxc_var_add(cstring_t, retval, amxc_string_get(&name, 0));
            amxc_var_clean(&tmp);
        }
    }

    amxc_string_clean(&name);
}

static int amxb_dbus_list_object_describe(amxb_bus_ctx_t* bus_ctx,
                                          const char* object,
                                          amxb_request_t* request,
                                          uint32_t flags) {
    int rv = 0;
    amxc_var_t tmp;
    amxc_string_t path;
    amxc_var_t* subojects = NULL;
    amxc_var_t output;

    amxc_string_init(&path, 0);
    amxc_var_init(&tmp);

    rv = amxb_describe(bus_ctx, object, flags, &tmp, 5);
    when_failed(rv, exit);

    amxc_var_init(&output);
    amxc_var_set_type(&output, AMXC_VAR_ID_LIST);
    amxc_var_add(cstring_t, &output, object);

    if(GETP_UINT32(&tmp, "0.type_id") != 2) {
        if((flags & AMXB_FLAG_PARAMETERS) != 0) {
            amxb_dbus_fill_list(bus_ctx, object, &tmp, "0.parameters", false, &output);
        }
        if((flags & AMXB_FLAG_FUNCTIONS) != 0) {
            amxb_dbus_fill_list(bus_ctx, object, &tmp, "0.functions", false, &output);
        }
        if((flags & AMXB_FLAG_OBJECTS) != 0) {
            if(flags & AMXB_FLAG_FIRST_LVL) {
                amxb_dbus_fill_list(bus_ctx, object, &tmp, "0.objects", false, &output);
            } else {
                subojects = GETP_ARG(&tmp, "0.objects");
            }
        }
    } else {
        if((flags & AMXB_FLAG_INSTANCES) != 0) {
            if(flags & AMXB_FLAG_FIRST_LVL) {
                amxb_dbus_fill_list(bus_ctx, object, &tmp, "0.instances", true, &output);
            } else {
                subojects = GETP_ARG(&tmp, "0.instances");
            }
        }
    }

    if(request->cb_fn) {
        request->cb_fn(bus_ctx, &output, request->priv);
    }

    // remove all items, they are not needed anymore
    amxc_var_clean(&output);

    amxc_var_for_each(subobj, subojects) {
        amxc_string_setf(&path, "%s%s.", object, GET_CHAR(subobj, NULL));
        amxb_dbus_list_object_describe(bus_ctx, amxc_string_get(&path, 0), request, flags);
    }

exit:
    amxc_var_clean(&tmp);
    amxc_string_clean(&path);
    return rv;
}

static int amxb_dbus_list_services(amxb_dbus_t* amxb_dbus_ctx, amxb_request_t* request) {
    int rv = -1;
    DBusMessage* msg = NULL;
    DBusPendingCall* pending = NULL;
    int status = amxd_status_ok;
    amxc_var_t services;
    amxc_var_t output;
    amxd_path_t path;
    const char* prefix = GET_CHAR(amxb_dbus_get_config_option("destination-prefix"), NULL);

    amxd_path_init(&path, NULL);
    amxc_var_init(&services);
    amxc_var_init(&output);
    amxc_var_set_type(&output, AMXC_VAR_ID_LIST);
    msg = dbus_message_new_method_call("org.freedesktop.DBus",
                                       "/org/freedesktop/DBus",
                                       "org.freedesktop.DBus",
                                       "ListNames");
    when_null(msg, exit);

    if(!dbus_connection_send_with_reply(amxb_dbus_ctx->dbus_handle, msg, &pending, -1)) {  // -1 is the default timeout
        goto exit;
    }

    dbus_connection_flush(amxb_dbus_ctx->dbus_handle);

    // free message
    dbus_message_unref(msg);

    // Block until we receive a reply
    dbus_pending_call_block(pending);
    msg = dbus_pending_call_steal_reply(pending);
    if(msg == NULL) {
        goto exit;
    }

    amxb_dbus_fetch_retval(msg, &services, NULL, &status);
    dbus_message_unref(msg);
    rv = status == 0? 0:status;
    when_failed(rv, exit);
    amxc_var_for_each(service, &services) {
        const char* name = GET_CHAR(service, NULL);
        if(name[0] == ':') {
            continue;
        }
        if(strncmp(name, prefix, strlen(prefix)) == 0) {
            amxd_path_setf(&path, true, "%s", name + strlen(prefix));
            amxc_var_add(cstring_t, &output, amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
        }
    }
    if(request->cb_fn) {
        request->cb_fn(amxb_request_get_ctx(request), &output, request->priv);
    }

exit:
    if(pending != NULL) {
        // free the pending message handle
        dbus_pending_call_unref(pending);
    }

    amxd_path_clean(&path);
    amxc_var_clean(&services);
    amxc_var_clean(&output);

    return rv;
}

int amxb_dbus_list(void* const ctx,
                   const char* object,
                   uint32_t flags,
                   UNUSED uint32_t access,
                   amxb_request_t* request) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) ctx;
    int rv = -1;

    if((object == NULL) || (*object == 0)) {
        rv = amxb_dbus_list_services(amxb_dbus_ctx, request);
        request->cb_fn(amxb_request_get_ctx(request), NULL, request->priv);
    } else {
        rv = amxb_dbus_list_object_describe(amxb_request_get_ctx(request), object, request, flags);
        request->cb_fn(amxb_request_get_ctx(request), NULL, request->priv);
    }

    amxb_close_request(&request);
    return rv;
}
