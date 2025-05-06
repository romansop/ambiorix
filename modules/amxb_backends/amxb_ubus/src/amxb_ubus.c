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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <amxc/amxc_string_split.h>

#include <amxb_be_ubus/amxb_be_ubus.h>

#include "amxb_ubus.h"
#include <amxb_ubus_version.h>

static const amxc_var_t* config_opts = NULL;
static char* log_output = NULL;

static void amxb_ubus_remove_sub(AMXB_UNUSED const char* key,
                                 amxc_htable_it_t* it) {
    amxb_ubus_sub_t* sub = amxc_htable_it_get_data(it, amxb_ubus_sub_t, it);
    amxp_timer_delete(&sub->reactivate);
    free(sub);
}

static int amxb_ubus_set_config(amxc_var_t* const configuration) {
    struct stat sb;
    amxc_var_t* uris = GET_ARG(configuration, "uris");
    amxc_var_t* log = GET_ARG(configuration, "log");

    free(log_output);
    log_output = NULL;
    config_opts = configuration;
    if(log != NULL) {
        const char* strlog = GET_CHAR(log, NULL);
        if(strlog != NULL) {
            log_output = strdup(strlog);
        }
    }

    if((configuration != NULL) && (uris == NULL)) {
        uris = amxc_var_add_key(amxc_llist_t, configuration, "uris", NULL);
    }

    if(stat("/var/run/ubus.sock", &sb) == 0) {
        amxc_var_add(cstring_t, uris, "ubus:/var/run/ubus.sock");
    }
    if(stat("/var/run/ubus/ubus.sock", &sb) == 0) {
        amxc_var_add(cstring_t, uris, "ubus:/var/run/ubus/ubus.sock");
    }

    return 0;
}

static void amxb_ubus_remove_connections(amxc_llist_it_t* lit) {
    amxb_bus_ctx_t* bus_ctx = amxc_llist_it_get_data(lit, amxb_bus_ctx_t, it);
    amxb_disconnect(bus_ctx);
}

static void amxb_ubus_reactivate_subscription(UNUSED amxp_timer_t* timer, void* priv) {
    amxb_ubus_sub_t* amxb_ubus_sub = (amxb_ubus_sub_t*) priv;
    amxc_htable_t* subscribers = NULL;
    amxb_ubus_t* amxb_ubus_ctx = NULL;
    const char* path = NULL;
    int ret = 0;

    subscribers = container_of(amxb_ubus_sub->it.ait->array, amxc_htable_t, table);
    amxb_ubus_ctx = container_of(subscribers, amxb_ubus_t, subscribers);
    path = amxc_htable_it_get_key(&amxb_ubus_sub->it);

    amxc_htable_it_take(&amxb_ubus_sub->it);
    ret = ubus_unregister_subscriber(amxb_ubus_ctx->ubus_ctx, &amxb_ubus_sub->sub);
    when_failed(ret, exit);
    amxb_ubus_subscribe(amxb_ubus_ctx, path);

exit:
    amxp_timer_delete(&amxb_ubus_sub->reactivate);
    amxc_htable_it_clean(&amxb_ubus_sub->it, NULL);
    free(amxb_ubus_sub);
}

static void amxb_ubus_watcher(UNUSED struct ubus_context* ctx,
                              struct ubus_event_handler* ev,
                              UNUSED const char* type,
                              struct blob_attr* msg) {
    amxb_ubus_t* amxb_ubus_ctx = container_of(ev, amxb_ubus_t, watcher);
    amxc_var_t watch_event;
    amxc_htable_it_t* it = NULL;
    const char* p = NULL;
    amxb_ubus_sub_t* amxb_ubus_sub = NULL;
    amxp_signal_t* signal = NULL;
    amxc_string_t signal_name;

    amxc_string_init(&signal_name, 0);
    amxc_var_init(&watch_event);
    amxc_var_set_type(&watch_event, AMXC_VAR_ID_HTABLE);
    amxb_ubus_parse_blob_table(&watch_event,
                               (struct blob_attr*) blob_data(msg),
                               blob_len(msg));

    p = GET_CHAR(&watch_event, "path");
    amxc_string_setf(&signal_name, "wait:%s.", p);
    signal = amxp_sigmngr_find_signal(NULL, amxc_string_get(&signal_name, 0));
    if(signal != NULL) {
        amxp_sigmngr_emit_signal(NULL, amxc_string_get(&signal_name, 0), NULL);
    }

    it = amxc_htable_get(&amxb_ubus_ctx->subscribers, p);
    when_null(it, exit);
    amxb_ubus_sub = amxc_container_of(it, amxb_ubus_sub_t, it);

    if(amxb_ubus_sub->reactivate == NULL) {
        amxp_timer_new(&amxb_ubus_sub->reactivate,
                       amxb_ubus_reactivate_subscription,
                       amxb_ubus_sub);
    }
    amxp_timer_start(amxb_ubus_sub->reactivate, 100);

exit:
    amxc_string_clean(&signal_name);
    amxc_var_clean(&watch_event);
}

static void amxb_ubus_wait_watcher(UNUSED struct ubus_context* ctx,
                                   UNUSED struct ubus_event_handler* ev,
                                   UNUSED const char* type,
                                   struct blob_attr* msg) {
    amxc_var_t watch_event;
    amxc_string_t sig_name;
    amxp_signal_t* signal = NULL;

    amxc_var_init(&watch_event);
    amxc_var_set_type(&watch_event, AMXC_VAR_ID_HTABLE);
    amxb_ubus_parse_blob_table(&watch_event, (struct blob_attr*) blob_data(msg), blob_len(msg));

    amxc_string_init(&sig_name, 0);
    amxc_string_setf(&sig_name, "wait:%s.", GET_CHAR(&watch_event, "path"));
    signal = amxp_sigmngr_find_signal(NULL, amxc_string_get(&sig_name, 0));
    if(signal != NULL) {
        amxp_sigmngr_emit_signal(NULL, amxc_string_get(&sig_name, 0), NULL);
    }

    amxc_string_clean(&sig_name);
    amxc_var_clean(&watch_event);
}

static void amxb_ubus_config_changed(UNUSED const char* const sig_name,
                                     UNUSED const amxc_var_t* const data,
                                     void* const priv) {
    amxb_ubus_t* amxb_ubus_ctx = (amxb_ubus_t*) priv;
    if(GET_BOOL(config_opts, "watch-ubus-events")) {
        if(amxb_ubus_ctx->watcher.cb == NULL) {
            amxb_ubus_ctx->watcher.cb = amxb_ubus_watcher;
            ubus_register_event_handler(amxb_ubus_ctx->ubus_ctx,
                                        &amxb_ubus_ctx->watcher, "ubus.object.add");
        }
    } else {
        if(amxb_ubus_ctx->watcher.cb != NULL) {
            ubus_unregister_event_handler(amxb_ubus_ctx->ubus_ctx, &amxb_ubus_ctx->watcher);
            amxb_ubus_ctx->watcher.cb = NULL;
        }
    }
}

const amxc_var_t* amxb_ubus_get_config_option(const char* name) {
    return GET_ARG(config_opts, name);
}

const char* amxb_ubus_log_output(void) {
    return log_output;
}

void* PRIVATE amxb_ubus_connect(const char* host,
                                const char* port,
                                const char* path,
                                amxp_signal_mngr_t* sigmngr) {
    amxb_ubus_t* amxb_ubus_ctx = NULL;

    when_not_null(host, exit);
    when_not_null(port, exit);

    amxb_ubus_ctx = (amxb_ubus_t*) calloc(1, sizeof(amxb_ubus_t));
    when_null(amxb_ubus_ctx, exit);

    amxb_ubus_ctx->ubus_ctx = ubus_connect(path);
    if(amxb_ubus_ctx->ubus_ctx == NULL) {
        free(amxb_ubus_ctx);
        amxb_ubus_ctx = NULL;
        goto exit;
    }

    amxc_htable_init(&amxb_ubus_ctx->subscribers, 5);
    blob_buf_init(&amxb_ubus_ctx->b, 0);
    amxb_ubus_ctx->sigmngr = sigmngr;
    amxc_llist_init(&amxb_ubus_ctx->registered_objs);
    amxc_llist_init(&amxb_ubus_ctx->pending_reqs);

    if(GET_BOOL(config_opts, "watch-ubus-events")) {
        amxb_ubus_ctx->watcher.cb = amxb_ubus_watcher;
        ubus_register_event_handler(amxb_ubus_ctx->ubus_ctx,
                                    &amxb_ubus_ctx->watcher, "ubus.object.add");
    }
    amxp_slot_connect(NULL,
                      "config:changed",
                      NULL,
                      amxb_ubus_config_changed,
                      amxb_ubus_ctx);

exit:
    return amxb_ubus_ctx;
}

int PRIVATE amxb_ubus_disconnect(void* ctx) {
    amxb_ubus_t* amxb_ubus_ctx = (amxb_ubus_t*) ctx;

    when_null(amxb_ubus_ctx, exit);
    when_null(amxb_ubus_ctx->ubus_ctx, exit);

    amxp_slot_disconnect_with_priv(NULL,
                                   amxb_ubus_config_changed,
                                   amxb_ubus_ctx);

    amxc_htable_clean(&amxb_ubus_ctx->subscribers, amxb_ubus_remove_sub);
    amxb_ubus_cancel_requests(amxb_ubus_ctx);
    ubus_free(amxb_ubus_ctx->ubus_ctx);
    blob_buf_free(&amxb_ubus_ctx->b);
    amxb_ubus_ctx->ubus_ctx = NULL;

exit:
    return 0;
}

int PRIVATE amxb_ubus_get_fd(void* ctx) {
    amxb_ubus_t* amxb_ubus_ctx = (amxb_ubus_t*) ctx;
    int fd = -1;

    when_null(amxb_ubus_ctx, exit);
    when_null(amxb_ubus_ctx->ubus_ctx, exit);

    fd = amxb_ubus_ctx->ubus_ctx->sock.fd;

exit:
    return fd;
}

int PRIVATE amxb_ubus_read(void* ctx) {
    amxb_ubus_t* amxb_ubus_ctx = (amxb_ubus_t*) ctx;
    struct ubus_context* ubus_ctx = NULL;
    int retval = -1;

    when_null(amxb_ubus_ctx, exit);
    when_null(amxb_ubus_ctx->ubus_ctx, exit);

    ubus_ctx = amxb_ubus_ctx->ubus_ctx;
    ubus_ctx->sock.cb(&ubus_ctx->sock, ULOOP_READ);

    if(ubus_ctx->sock.eof) {
        retval = -1;
    } else {
        retval = 0;
    }

exit:
    return retval;
}

void PRIVATE amxb_ubus_free(void* ctx) {
    amxb_ubus_t* amxb_ubus_ctx = (amxb_ubus_t*) ctx;

    when_null(amxb_ubus_ctx, exit);
    amxc_llist_clean(&amxb_ubus_ctx->registered_objs, amxb_ubus_obj_it_free);
    free(amxb_ubus_ctx);

exit:
    return;
}

static void amxb_ubus_wait_for_done(UNUSED const char* const sig_name,
                                    UNUSED const amxc_var_t* const d,
                                    void* const priv) {
    amxb_ubus_t* amxb_ubus_ctx = (amxb_ubus_t*) priv;
    if(amxb_ubus_ctx->wait_watcher.cb != NULL) {
        ubus_unregister_event_handler(amxb_ubus_ctx->ubus_ctx, &amxb_ubus_ctx->wait_watcher);
        amxb_ubus_ctx->wait_watcher.cb = NULL;
        amxp_slot_disconnect_with_priv(NULL, amxb_ubus_wait_for_done, amxb_ubus_ctx);
    }
}

static int amxb_ubus_wait_for(void* const ctx, UNUSED const char* object) {
    amxb_ubus_t* amxb_ubus_ctx = (amxb_ubus_t*) ctx;
    int retval = -1;

    when_null(amxb_ubus_ctx, exit);
    if(amxb_ubus_ctx->wait_watcher.cb == NULL) {
        amxb_ubus_ctx->wait_watcher.cb = amxb_ubus_wait_watcher;
        ubus_register_event_handler(amxb_ubus_ctx->ubus_ctx,
                                    &amxb_ubus_ctx->wait_watcher, "ubus.object.add");
        amxp_slot_connect(NULL, "wait:done", NULL, amxb_ubus_wait_for_done, amxb_ubus_ctx);
    }

    retval = 0;

exit:
    return retval;
}

static uint32_t amxb_ubus_capabilities(UNUSED void* ctx) {
    return AMXB_BE_DISCOVER | AMXB_BE_DISCOVER_RESOLVE;
}

static bool amxb_ubus_has(void* ctx, const char* object) {
    amxb_ubus_t* amxb_ubus_ctx = (amxb_ubus_t*) ctx;
    int retval = -1;
    bool has_object = false;
    uint32_t id = 0;
    when_null(object, exit);

    retval = ubus_lookup_id(amxb_ubus_ctx->ubus_ctx, object, &id);

    when_failed(retval, exit);

    has_object = (id != 0);

exit:
    return has_object;
}

/** @implements @ref amxb_be_stats_t */
static int amxb_ubus_get_stats(void* const ctx,
                               amxc_var_t* stats) {
    int retval = -1;
    amxc_var_t* rx_stats = NULL;
    amxc_var_t* operation_rx_stats = NULL;
    amxb_ubus_t* amxb_ubus_ctx = (amxb_ubus_t*) ctx;
    when_null(amxb_ubus_ctx, exit);
    when_null(stats, exit);
    rx_stats = amxc_var_add_new_key(stats, "rx");
    amxc_var_set_type(rx_stats, AMXC_VAR_ID_HTABLE);
    operation_rx_stats = amxc_var_add_new_key(rx_stats, "operation");
    amxc_var_set_type(operation_rx_stats, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint64_t, operation_rx_stats, "invoke", amxb_ubus_ctx->stats.counter_rx_invoke);

    retval = 0;
exit:
    return retval;
}

static int amxb_ubus_reset_stats(void* const ctx) {
    amxb_ubus_t* amxb_ubus_ctx = (amxb_ubus_t*) ctx;
    when_null(amxb_ubus_ctx, exit);

    amxb_ubus_ctx->stats.counter_rx_invoke = 0;

exit:
    return 0;
}

static amxb_be_funcs_t amxb_ubus_impl = {
    .it = { .ait = NULL, .key = NULL, .next = NULL },
    .handle = NULL,
    .connections = { .head = NULL, .tail = NULL },
    .name = "ubus",
    .size = sizeof(amxb_be_funcs_t),
    .connect = amxb_ubus_connect,
    .disconnect = amxb_ubus_disconnect,
    .get_fd = amxb_ubus_get_fd,
    .read = amxb_ubus_read,
    .new_invoke = NULL,
    .free_invoke = NULL,
    .invoke = amxb_ubus_invoke,
    .async_invoke = amxb_ubus_async_invoke,
    .close_request = amxb_ubus_close_request,
    .wait_request = amxb_ubus_wait_request,
    .subscribe = amxb_ubus_subscribe,
    .unsubscribe = amxb_ubus_unsubscribe,
    .free = amxb_ubus_free,
    .register_dm = amxb_ubus_register,
    .get = NULL,
    .set = NULL,
    .add = NULL,
    .del = NULL,
    .get_supported = NULL,
    .set_config = amxb_ubus_set_config,
    .describe = NULL,
    .list = amxb_ubus_list,
    .listen = NULL,
    .accept = NULL,
    .read_raw = NULL,
    .wait_for = amxb_ubus_wait_for,
    .capabilities = amxb_ubus_capabilities,
    .has = amxb_ubus_has,
    .get_instances = NULL,
    .get_filtered = NULL,
    .get_stats = amxb_ubus_get_stats,
    .reset_stats = amxb_ubus_reset_stats,
};

static amxb_version_t sup_min_lib_version = {
    .major = 4,
    .minor = 11,
    .build = 0
};

static amxb_version_t sup_max_lib_version = {
    .major = 4,
    .minor = -1,
    .build = -1
};

static amxb_version_t ubus_be_version = {
    .major = AMXB_UBUS_VERSION_MAJOR,
    .minor = AMXB_UBUS_VERSION_MINOR,
    .build = AMXB_UBUS_VERSION_BUILD,
};

amxb_be_info_t amxb_ubus_be_info = {
    .min_supported = &sup_min_lib_version,
    .max_supported = &sup_max_lib_version,
    .be_version = &ubus_be_version,
    .name = "ubus",
    .description = "AMXB Backend for UBUS (Openwrt/Prplwrt)",
    .funcs = &amxb_ubus_impl,
};

amxb_be_info_t* amxb_be_info(void) {
    return &amxb_ubus_be_info;
}

int amxb_be_ubus_init(void) {
    return amxb_be_register(&amxb_ubus_impl);
}

int amxb_be_ubus_clean(void) {
    amxc_llist_clean(&amxb_ubus_impl.connections, amxb_ubus_remove_connections);
    free(log_output);
    log_output = NULL;
    return amxb_be_unregister(&amxb_ubus_impl);
}