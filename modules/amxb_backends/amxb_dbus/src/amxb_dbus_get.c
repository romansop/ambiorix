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

static bool amxb_dbus_destination_exists(amxb_dbus_t* amxb_dbus_ctx,
                                         const char* dest) {
    bool exists = false;
    DBusMessage* msg = NULL;
    DBusMessageIter dbus_args;
    DBusPendingCall* pending = NULL;
    amxc_var_t var_dest;
    int status = amxd_status_ok;

    amxc_var_init(&var_dest);
    amxc_var_set(cstring_t, &var_dest, dest);

    msg = dbus_message_new_method_call("org.freedesktop.DBus",
                                       "/org/freedesktop/DBus",
                                       "org.freedesktop.DBus",
                                       "GetNameOwner");
    when_null(msg, exit);

    dbus_message_iter_init_append(msg, &dbus_args);
    amxb_var_to_dbus(&var_dest, &dbus_args);

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

    amxb_dbus_fetch_retval(msg, NULL, NULL, &status);
    exists = (status == amxd_status_ok);
    dbus_message_unref(msg);

exit:
    if(pending != NULL) {
        // free the pending message handle
        dbus_pending_call_unref(pending);
    }
    amxc_var_clean(&var_dest);

    return exists;
}

int amxb_dbus_get(void* const ctx,
                  const char* object,
                  const char* search_path,
                  int32_t depth,
                  uint32_t access,
                  amxc_var_t* ret,
                  int timeout) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) ctx;
    DBusMessage* msg = NULL;
    char* dest = NULL;
    amxc_var_t args;
    amxc_var_t* l = NULL;
    amxc_var_t* rv = NULL;
    int status = -1;
    amxd_path_t full_path;

    amxc_var_init(&args);

    amxd_path_init(&full_path, NULL);
    amxd_path_setf(&full_path, false, "%s%s", object, search_path == NULL? "":search_path);
    dest = amxd_path_get_first(&full_path, false);

    if(amxb_dbus_destination_exists(amxb_dbus_ctx, amxd_path_get(&full_path, 0))) {
        amxc_var_set_type(ret, AMXC_VAR_ID_LIST);
        rv = amxc_var_add(amxc_htable_t, ret, NULL);
        amxc_var_add_key(amxc_htable_t, rv, amxd_path_get(&full_path, AMXD_OBJECT_TERMINATE), NULL);
        status = 0;
        goto exit;
    }

    // build dbus args
    amxc_var_set_type(&args, AMXC_VAR_ID_LIST);
    amxc_var_add(cstring_t, &args, amxd_path_get(&full_path, AMXD_OBJECT_TERMINATE));
    l = amxc_var_add(amxc_llist_t, &args, NULL);
    if(amxd_path_get_param(&full_path) != NULL) {
        amxc_var_add(cstring_t, l, amxd_path_get_param(&full_path));
    }
    amxc_var_add(int32_t, &args, depth);
    amxc_var_add(uint32_t, &args, access);
    amxc_var_add(cstring_t, &args, "");

    msg = amxb_dbus_call(amxb_dbus_ctx, dest, "get", &args, timeout);
    when_null(msg, exit);

    amxc_var_set_type(ret, AMXC_VAR_ID_LIST);
    rv = amxc_var_add_new(ret);
    amxb_dbus_fetch_retval(msg, rv, NULL, &status);

exit:
    if(msg != NULL) {
        dbus_message_unref(msg);
    }

    free(dest);
    amxc_var_clean(&args);
    amxd_path_clean(&full_path);

    return status;
}