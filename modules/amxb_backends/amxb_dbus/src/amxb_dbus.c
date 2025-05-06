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

static amxc_var_t* config_opts = NULL;

static void amxb_dbus_remove_sub(UNUSED const char* key,
                                 amxc_htable_it_t* it) {
    amxb_dbus_sub_t* sub = amxc_htable_it_get_data(it, amxb_dbus_sub_t, it);
    free(sub);
}

static dbus_bool_t amxb_dbus_watch_add(DBusWatch* watch, void* data) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) data;

    int flags = dbus_watch_get_flags(watch);
    if(flags & DBUS_WATCH_READABLE) {
        if(amxb_dbus_ctx->watch == NULL) {
            amxb_dbus_ctx->watch = watch;
        }
    }

    return TRUE;
}

static void amxb_dbus_watch_remove(DBusWatch* watch, void* data) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) data;

    if(amxb_dbus_ctx->watch == watch) {
        amxb_dbus_ctx->watch = NULL;
    }
}

static void amxb_dbus_watch_toggle(DBusWatch* watch, void* data) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) data;

    if(dbus_watch_get_enabled(watch) && (amxb_dbus_ctx->watch == NULL)) {
        amxb_dbus_ctx->watch = watch;
    }
    if(!dbus_watch_get_enabled(watch) && (amxb_dbus_ctx->watch == watch)) {
        amxb_dbus_ctx->watch = NULL;
    }
}

static int amxb_dbus_set_config(amxc_var_t* const configuration) {
    amxc_var_t* option = NULL;
    amxc_var_t* uris = GET_ARG(configuration, "uris");
    config_opts = configuration;

    option = GET_ARG(config_opts, "dm-interface");
    if(option == NULL) {
        amxc_var_add_key(cstring_t, config_opts, "dm-interface", AMX_DM_INTERFACE);
    }

    option = GET_ARG(config_opts, "destination-prefix");
    if(option == NULL) {
        amxc_var_add_key(cstring_t, config_opts, "destination-prefix", AMX_DEST_PREFIX);
    }

    if((config_opts != NULL) && (uris == NULL)) {
        uris = amxc_var_add_key(amxc_llist_t, config_opts, "uris", NULL);
    }

    amxc_var_add(cstring_t, uris, "dbus:");

    return 0;
}

const amxc_var_t* amxb_dbus_get_config_option(const char* name) {
    return GET_ARG(config_opts, name);
}

const amxc_var_t* amxb_dbus_get_config(void) {
    return config_opts;
}

static void* amxb_dbus_connect(const char* host,
                               const char* port,
                               const char* path,
                               amxp_signal_mngr_t* sigmngr) {
    amxb_dbus_t* amxb_dbus_ctx = NULL;
    DBusError err;

    dbus_error_init(&err);

    when_not_null(host, exit);
    when_not_null(port, exit);

    amxb_dbus_ctx = (amxb_dbus_t*) calloc(1, sizeof(amxb_dbus_t));
    when_null(amxb_dbus_ctx, exit);

    amxb_dbus_ctx->sigmngr = sigmngr;
    amxc_htable_init(&amxb_dbus_ctx->subscribers, 5);

    if((path == NULL) || (*path == 0)) {
        amxb_dbus_ctx->dbus_handle = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    } else if(strcmp(path, "/system") == 0) {
        amxb_dbus_ctx->dbus_handle = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    } else if(strcmp(path, "/session") == 0) {
        amxb_dbus_ctx->dbus_handle = dbus_bus_get(DBUS_BUS_SESSION, &err);
    }

    when_null(amxb_dbus_ctx->dbus_handle, exit);

    if(!dbus_connection_set_watch_functions(amxb_dbus_ctx->dbus_handle,
                                            amxb_dbus_watch_add,
                                            amxb_dbus_watch_remove,
                                            amxb_dbus_watch_toggle,
                                            amxb_dbus_ctx,
                                            NULL)) {
        dbus_connection_unref(amxb_dbus_ctx->dbus_handle);
        amxb_dbus_ctx->dbus_handle = NULL;
    }

    dbus_connection_set_exit_on_disconnect(amxb_dbus_ctx->dbus_handle, FALSE);

    if(!dbus_connection_add_filter(amxb_dbus_ctx->dbus_handle, amxb_dbus_handle_msg, amxb_dbus_ctx, NULL)) {
        dbus_connection_unref(amxb_dbus_ctx->dbus_handle);
        amxb_dbus_ctx->dbus_handle = NULL;
        goto exit;
    }

    dbus_bus_add_match(amxb_dbus_ctx->dbus_handle, "type='signal',interface='" DBUS_INTERFACE_DBUS  "'", &err);
    if(dbus_error_is_set(&err)) {
        goto exit;
    }

exit:
    if(amxb_dbus_ctx->dbus_handle == NULL) {
        free(amxb_dbus_ctx);
        amxb_dbus_ctx = NULL;
    }
    dbus_error_free(&err);
    return amxb_dbus_ctx;
}

static int amxb_dbus_disconnect(void* ctx) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) ctx;

    amxc_htable_clean(&amxb_dbus_ctx->subscribers, amxb_dbus_remove_sub);

    when_null(ctx, exit);
    when_null(amxb_dbus_ctx->watch, exit);

    dbus_connection_set_watch_functions(amxb_dbus_ctx->dbus_handle,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL);

    dbus_connection_unref(amxb_dbus_ctx->dbus_handle);
    amxb_dbus_ctx->dbus_handle = NULL;

exit:
    return 0;
}

static int amxb_dbus_get_fd(void* ctx) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) ctx;
    int flags = 0;
    int fd = -1;

    if(amxb_dbus_ctx->watch != NULL) {
        flags = dbus_watch_get_flags(amxb_dbus_ctx->watch);
        if((flags & DBUS_WATCH_READABLE) == DBUS_WATCH_READABLE) {
            fd = dbus_watch_get_unix_fd(amxb_dbus_ctx->watch);
        }
    }

    return fd;
}

static int amxb_dbus_read(void* ctx) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) ctx;
    DBusDispatchStatus status = 0;

    when_null(amxb_dbus_ctx->watch, exit);

    when_false(dbus_watch_handle(amxb_dbus_ctx->watch, DBUS_WATCH_READABLE | DBUS_WATCH_WRITABLE), exit);

    status = dbus_connection_get_dispatch_status(amxb_dbus_ctx->dbus_handle);
    while(status == DBUS_DISPATCH_DATA_REMAINS) {
        status = dbus_connection_dispatch(amxb_dbus_ctx->dbus_handle);
    }

exit:
    return 0;
}

static void amxb_dbus_free(void* ctx) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) ctx;
    free(amxb_dbus_ctx);
}

// This method must be available to make the wait-for work.
// Nothing is done here, when new services are launched an event is recieved anyway.
static int amxb_dbus_wait_for(UNUSED void* const ctx, UNUSED const char* object) {
    return 0;
}

static amxb_be_funcs_t amxb_dbus_impl = {
    .it = { .ait = NULL, .key = NULL, .next = NULL },
    .handle = NULL,
    .connections = { .head = NULL, .tail = NULL },
    .name = "dbus",
    .size = sizeof(amxb_be_funcs_t),
    .connect = amxb_dbus_connect,
    .disconnect = amxb_dbus_disconnect,
    .get_fd = amxb_dbus_get_fd,
    .read = amxb_dbus_read,
    .new_invoke = NULL,
    .free_invoke = NULL,
    .invoke = amxb_dbus_invoke,
    .async_invoke = amxb_dbus_async_invoke,
    .close_request = amxb_dbus_close_request,
    .wait_request = NULL,
    .subscribe = amxb_dbus_subscribe,
    .unsubscribe = NULL,
    .free = amxb_dbus_free,
    .register_dm = amxb_dbus_register,
    .get = amxb_dbus_get,
    .set = amxb_dbus_set,
    .add = amxb_dbus_add,
    .del = amxb_dbus_del,
    .get_supported = amxb_dbus_gsdm,
    .set_config = amxb_dbus_set_config,
    .describe = amxb_dbus_describe,
    .list = amxb_dbus_list,
    .listen = NULL,
    .accept = NULL,
    .read_raw = NULL,
    .wait_for = amxb_dbus_wait_for,
    .capabilities = NULL,
    .has = NULL,
    .get_instances = NULL,
};

static amxb_version_t sup_min_lib_version = {
    .major = 4,
    .minor = 2,
    .build = 0
};

static amxb_version_t sup_max_lib_version = {
    .major = 4,
    .minor = -1,
    .build = -1
};

static amxb_version_t dbus_be_version = {
    .major = AMXB_DBUS_VERSION_MAJOR,
    .minor = AMXB_DBUS_VERSION_MINOR,
    .build = AMXB_DBUS_VERSION_BUILD,
};

amxb_be_info_t amxb_dbus_be_info = {
    .min_supported = &sup_min_lib_version,
    .max_supported = &sup_max_lib_version,
    .be_version = &dbus_be_version,
    .name = "dbus",
    .description = "AMXB Backend for DBUS",
    .funcs = &amxb_dbus_impl,
};

amxb_be_info_t* amxb_be_info(void) {
    return &amxb_dbus_be_info;
}


DESTRUCTOR static void amxb_dbus_quit(void) {
    dbus_shutdown();
}
