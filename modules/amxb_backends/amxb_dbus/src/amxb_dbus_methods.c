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

int isdot(int c) {
    if(c == '.') {
        return 1;
    }

    return 0;
}

void amxd_dbus_dbus_args_to_amx_args(amxc_var_t* dbus_args,
                                     const char* arg_names[],
                                     amxc_var_t* fn_args) {
    amxc_var_t* arg = NULL;

    amxc_var_set_type(fn_args, AMXC_VAR_ID_HTABLE);
    for(int i = 0; arg_names[i] != NULL; i++) {
        arg = amxc_var_get_first(dbus_args);
        if(arg == NULL) {
            break;
        }
        amxc_var_take_it(arg);
        if(amxc_var_is_null(arg)) {
            amxc_var_delete(&arg);
        } else {
            amxc_var_set_key(fn_args, arg_names[i], arg, AMXC_VAR_FLAG_DEFAULT);
        }
    }
}

void amxd_dbus_amx_out_to_dbus_args(DBusMessage* reply,
                                    amxc_var_t* rv,
                                    amxc_var_t* out,
                                    int status) {
    DBusMessageIter dbus_ret;
    amxc_var_t var_status;
    amxc_var_init(&var_status);
    amxc_var_set(int32_t, &var_status, status);

    if(rv != NULL) {
        dbus_message_iter_init_append(reply, &dbus_ret);
        amxb_var_to_dbus(rv, &dbus_ret);
    }

    if(out != NULL) {
        dbus_message_iter_init_append(reply, &dbus_ret);
        amxb_var_to_dbus(out, &dbus_ret);
    }

    dbus_message_iter_init_append(reply, &dbus_ret);
    amxb_var_to_dbus(&var_status, &dbus_ret);

    amxc_var_clean(&var_status);
}

void amxb_dbus_fetch_retval(DBusMessage* reply,
                            amxc_var_t* rv,
                            amxc_var_t* out,
                            int* status) {
    DBusMessageIter dbus_args;
    amxc_var_t var_status;
    amxc_var_init(&var_status);

    if(dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
        if(status != NULL) {
            *status = amxd_status_unknown_error;
        }
        goto exit;
    }


    dbus_message_iter_init(reply, &dbus_args);
    if(rv != NULL) {
        amxb_dbus_to_var(rv, &dbus_args);
    }

    dbus_message_iter_next(&dbus_args);
    if(out != NULL) {
        amxb_dbus_to_var(out, &dbus_args);
    }

    dbus_message_iter_next(&dbus_args);
    amxb_dbus_to_var(&var_status, &dbus_args);
    if(status != NULL) {
        *status = GET_INT32(&var_status, NULL);
    }

exit:
    amxc_var_clean(&var_status);
}

DBusMessage* amxb_dbus_call(amxb_dbus_t* amxb_dbus_ctx,
                            const char* object,
                            const char* method,
                            amxc_var_t* args,
                            int timeout) {
    DBusMessage* msg = NULL;
    DBusMessageIter dbus_args;
    DBusPendingCall* pending = NULL;
    amxc_string_t destination;
    const char* prefix = GET_CHAR(amxb_dbus_get_config_option("destination-prefix"), NULL);
    const char* interface = GET_CHAR(amxb_dbus_get_config_option("dm-interface"), NULL);

    amxc_string_init(&destination, 0);
    amxc_string_setf(&destination, "%s%s", prefix, object);
    amxc_string_trimr(&destination, isdot);
    msg = dbus_message_new_method_call(amxc_string_get(&destination, 0), "/", interface, method);
    when_null(msg, exit);

    amxc_var_for_each(arg, args) {
        dbus_message_iter_init_append(msg, &dbus_args);
        amxb_var_to_dbus(arg, &dbus_args);
    }

    if(!dbus_connection_send_with_reply(amxb_dbus_ctx->dbus_handle, msg, &pending, timeout * 1000)) {  // -1 is the default timeout
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

exit:
    if(pending != NULL) {
        // free the pending message handle
        dbus_pending_call_unref(pending);
    }

    amxc_string_clean(&destination);

    return msg;
}