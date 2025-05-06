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
#include <string.h>
#include <sys/sysinfo.h>
#include <poll.h>

#include <uspi/uspi_prefix.h>

#include "amxb_usp.h"
#include "amxb_usp_la.h"
#include "amxb_usp_version.h"

#define DEFAULT_LA_DM "/etc/amx/modules/usp/local-agent.odl"

static amxc_var_t* config_opts = NULL;
static const char* sig_eid = "usp:eid-added";
static amxc_htable_t ctx_table;
static amxd_dm_t la_dm;
static amxo_parser_t la_parser;

static void sigpipe_ignore(UNUSED const char* const sig_name,
                           UNUSED const amxc_var_t* const data,
                           UNUSED void* const priv) {
}

CONSTRUCTOR static void amxb_usp_init(void) {
    amxp_sigmngr_add_signal(NULL, sig_eid);
    amxp_syssig_enable(SIGPIPE, true);
    amxp_slot_connect(NULL, strsignal(SIGPIPE), NULL, sigpipe_ignore, NULL);
    amxc_htable_init(&ctx_table, 0);
    amxd_dm_init(&la_dm);
    amxo_parser_init(&la_parser);
}

DESTRUCTOR static void amxb_usp_clean(void) {
    amxp_signal_t* sig = amxp_sigmngr_find_signal(NULL, sig_eid);

    amxp_sigmngr_remove_signal(NULL, sig_eid);
    amxp_signal_delete(&sig);
    amxc_htable_clean(&ctx_table, NULL);
    amxo_parser_clean(&la_parser);
    amxd_dm_clean(&la_dm);
}

static void amxb_usp_handshake_received(UNUSED const char* const sig_name,
                                        const amxc_var_t* const data,
                                        void* const priv) {
    amxb_usp_t* ctx = (amxb_usp_t*) priv;
    const char* eid = GET_CHAR(data, "EndpointID");

    when_str_empty(eid, exit);
    when_not_null(ctx->eid, exit);

    ctx->eid = strdup(eid);

exit:
    return;
}

static void amxb_usp_eid_slot_connect(amxb_usp_t* ctx) {
    amxc_string_t expr;

    amxc_string_init(&expr, 0);

    amxc_string_setf(&expr, "fd == %d", imtp_connection_get_fd(ctx->icon));
    amxp_slot_connect(NULL, sig_eid, amxc_string_get(&expr, 0), amxb_usp_handshake_received, ctx);

    amxc_string_clean(&expr);
}

static void amxb_usp_remove_subscriptions(UNUSED const char* key,
                                          amxc_htable_it_t* it) {
    amxc_var_t* sub = amxc_var_from_htable_it(it);
    amxc_var_delete(&sub);
}

static void amxb_usp_remove_server_subs(UNUSED const char* key,
                                        amxc_htable_it_t* it) {
    usp_subscription_t* sub = amxc_htable_it_get_data(it, usp_subscription_t, hit);

    amxp_slot_disconnect_with_priv(&sub->ctx->dm->sigmngr,
                                   amxb_usp_send_notification,
                                   &sub->hit);
    free(sub);
}

static uint32_t amxb_usp_get_capabilities(UNUSED void* const ctx) {
    amxc_var_t* caps = GET_ARG(config_opts, "caps");
    uint32_t retval = 0;

    // Default behaviour
    if(caps == NULL) {
        retval = AMXB_BE_DISCOVER_DESCRIBE | AMXB_BE_DISCOVER_RESOLVE | AMXB_BE_DISCOVER;
        goto exit;
    }

    if(GET_BOOL(caps, "describe")) {
        retval |= AMXB_BE_DISCOVER_DESCRIBE;
    }
    if(GET_BOOL(caps, "resolve")) {
        retval |= AMXB_BE_DISCOVER_RESOLVE;
    }
    if(GET_BOOL(caps, "has")) {
        retval |= AMXB_BE_DISCOVER;
    }

exit:
    return retval;
}

static void amxb_usp_set_random_eid(amxc_string_t* eid) {
    int32_t a = 0;
    struct sysinfo info;

    sysinfo(&info);
    srandom(info.uptime);
    a = random();

    amxc_string_setf(eid, "proto::usp-%d", a);
}

static void amxb_usp_config_set_eid(void) {
    amxc_var_t* eid = GET_ARG(config_opts, "EndpointID");
    amxc_var_t* parent_opts = amxc_var_get_parent(config_opts);
    const char* name = NULL;
    amxc_string_t eid_generated;

    amxc_string_init(&eid_generated, 0);

    when_not_null(eid, exit);

    name = GET_CHAR(parent_opts, "name");
    if((name != NULL) && (*name != 0)) {
        amxc_string_setf(&eid_generated, "proto::%s", name);
    } else {
        amxb_usp_set_random_eid(&eid_generated);
    }

    amxc_var_add_key(cstring_t, config_opts, "EndpointID", amxc_string_get(&eid_generated, 0));

exit:
    amxc_string_clean(&eid_generated);
    return;
}

static void amxb_usp_config_set_defaults(void) {
    amxc_var_t* requires_device = GET_ARG(config_opts, AMXB_USP_COPT_REQUIRES_DEVICE);

    // If not defined in odl already, pick default value based on config option
    if(requires_device == NULL) {
        amxc_var_add_key(bool, config_opts, AMXB_USP_COPT_REQUIRES_DEVICE, AMXB_USP_CVAL_REQUIRES_DEVICE);
    }
}

static int amxb_usp_send_handshake(amxb_usp_t* usp_ctx) {
    int retval = -1;
    const char* eid = GET_CHAR(config_opts, "EndpointID");
    imtp_frame_t* frame = NULL;
    imtp_tlv_t* tlv_handshake = NULL;

    when_str_empty(eid, exit);

    imtp_frame_new(&frame);
    imtp_tlv_new(&tlv_handshake, imtp_tlv_type_handshake, strlen(eid),
                 (char*) eid, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(frame, tlv_handshake);

    retval = imtp_connection_write_frame(usp_ctx->icon, frame);

exit:
    imtp_frame_delete(&frame);
    return retval;
}

static bool amxb_usp_list_contains(amxc_var_t* list, const char* value) {
    bool duplicate = false;

    amxc_var_for_each(elem, list) {
        if(strcmp(GET_CHAR(elem, NULL), value) == 0) {
            duplicate = true;
            break;
        }
    }

    return duplicate;
}

static void amxb_usp_add_default_uris(amxc_var_t* uris, amxc_var_t* data_uris, bool connect_retry) {
    struct stat sb;
    const char* default_agent_uri = "usp:/var/run/usp/broker_agent_path";
    const char* default_contr_uri = "usp:/var/run/usp/broker_controller_path";

    // If connect retry is needed, make sure default uris are added, because
    // the sockets may not be detected yet on startup depending on startup
    // order
    if(!amxb_usp_list_contains(data_uris, default_agent_uri)) {
        if(connect_retry || (stat("/var/run/usp/broker_agent_path", &sb) == 0)) {
            amxc_var_add(cstring_t, data_uris, default_agent_uri);
        }
    }

    if(!amxb_usp_list_contains(uris, default_contr_uri)) {
        if(connect_retry || (stat("/var/run/usp/broker_controller_path", &sb) == 0)) {
            amxc_var_add(cstring_t, uris, default_contr_uri);
        }
    }
}

static int amxb_usp_ctx_new(amxb_usp_t** usp_ctx, imtp_connection_t* icon, amxp_signal_mngr_t* sigmngr) {
    int retval = -1;

    *usp_ctx = (amxb_usp_t*) calloc(1, sizeof(amxb_usp_t));
    when_null(*usp_ctx, exit);

    amxc_htable_init(&(*usp_ctx)->subscriptions, 5);
    amxc_htable_init(&(*usp_ctx)->server_subs, 5);
    amxc_htable_init(&(*usp_ctx)->operate_requests, 5);
    amxc_llist_init(&(*usp_ctx)->registrations);
    (*usp_ctx)->icon = icon;
    (*usp_ctx)->sigmngr = sigmngr;
    (*usp_ctx)->la_dm = &la_dm;

    retval = 0;

exit:
    return retval;
}

static void amxb_usp_ctx_delete(amxb_usp_t** usp_ctx) {
    when_null(usp_ctx, exit);
    when_null(*usp_ctx, exit);

    imtp_connection_delete(&(*usp_ctx)->icon);
    (*usp_ctx)->icon = NULL;
    amxc_htable_clean(&(*usp_ctx)->subscriptions, amxb_usp_remove_subscriptions);
    amxc_htable_clean(&(*usp_ctx)->server_subs, amxb_usp_remove_server_subs);
    amxc_htable_clean(&(*usp_ctx)->operate_requests, amxb_usp_request_free);
    amxc_llist_clean(&(*usp_ctx)->registrations, amxc_string_list_it_free);
    amxp_timer_delete(&(*usp_ctx)->register_retry);

exit:
    return;
}

amxd_dm_t* amxb_usp_get_la(void) {
    return &la_dm;
}

int amxb_usp_set_config(amxc_var_t* const configuration) {
    config_opts = configuration;
    amxc_var_t* uris = GET_ARG(configuration, "uris");
    amxc_var_t* data_uris = GET_ARG(configuration, "data-uris");
    amxc_var_t* la_dm_def = GET_ARG(configuration, "local-agent-dm");
    amxc_var_t* translate = GET_ARG(configuration, "translate");
    bool connect_retry = GET_BOOL(configuration, "connect-retry");

    when_null(configuration, exit);

    amxb_usp_config_set_defaults();

    if((uris == NULL)) {
        uris = amxc_var_add_key(amxc_llist_t, configuration, "uris", NULL);
    }
    if((data_uris == NULL)) {
        data_uris = amxc_var_add_key(amxc_llist_t, configuration, "data-uris", NULL);
    }

    amxb_usp_add_default_uris(uris, data_uris, connect_retry);

    if(la_dm_def == NULL) {
        struct stat statbuf;
        if(stat(DEFAULT_LA_DM, &statbuf) == 0) {
            la_dm_def = amxc_var_add_key(cstring_t, configuration, "local-agent-dm", DEFAULT_LA_DM);
        }
    }
    if(la_dm_def != NULL) {
        amxo_resolver_ftab_add(&la_parser, "la_create_subscription", AMXO_FUNC(amxb_usp_la_subs_add_inst));
        amxo_resolver_ftab_add(&la_parser, "la_cleanup_matching", AMXO_FUNC(amxb_usp_la_subs_remove_matching));
        amxo_resolver_ftab_add(&la_parser, "la_subscription_added", AMXO_FUNC(amxb_usp_la_subscription_added));
        amxo_resolver_ftab_add(&la_parser, "la_request_remove", AMXO_FUNC(amxb_usp_la_remove_request));
        amxo_resolver_ftab_add(&la_parser, "la_threshold_changed", AMXO_FUNC(amxb_usp_la_threshold_changed));
        amxo_resolver_ftab_add(&la_parser, "la_threshold_create", AMXO_FUNC(amxb_usp_la_threshold_create));
        amxo_resolver_ftab_add(&la_parser, "la_threshold_added", AMXO_FUNC(amxb_usp_la_threshold_added));
        amxo_resolver_ftab_add(&la_parser, "la_threshold_instance_is_valid", AMXO_FUNC(amxb_usp_la_threshold_instance_is_valid));
        amxo_resolver_ftab_add(&la_parser, "la_threshold_instance_cleanup", AMXO_FUNC(amxb_usp_la_threshold_instance_cleanup));
        amxo_parser_parse_file(&la_parser, GET_CHAR(la_dm_def, NULL), amxd_dm_get_root(&la_dm));
    }

    if(translate != NULL) {
        amxb_usp_translate_set_paths();
    }

exit:
    return 0;
}

amxc_var_t* amxb_usp_get_config_option(const char* name) {
    return GET_ARG(config_opts, name);
}

amxc_var_t* amxb_usp_get_config(void) {
    return config_opts;
}

void* amxb_usp_connect(const char* host,
                       const char* port,
                       const char* path,
                       amxp_signal_mngr_t* sigmngr) {
    amxb_usp_t* amxb_usp_ctx = NULL;
    imtp_connection_t* icon = NULL;
    int retval = -1;

    when_str_empty(path, exit);
    when_not_null(host, exit);
    when_not_null(port, exit);

    retval = imtp_connection_connect(&icon, NULL, path);
    when_failed(retval, exit);

    retval = amxb_usp_ctx_new(&amxb_usp_ctx, icon, sigmngr);
    when_failed(retval, exit);

    amxb_usp_config_set_eid();
    amxb_usp_send_handshake(amxb_usp_ctx);
    amxb_usp_eid_slot_connect(amxb_usp_ctx);

exit:
    if(retval < 0) {
        free(amxb_usp_ctx);
        amxb_usp_ctx = NULL;
    }
    return amxb_usp_ctx;
}

void* amxb_usp_listen(const char* host,
                      const char* port,
                      const char* path,
                      amxp_signal_mngr_t* sigmngr) {
    amxb_usp_t* amxb_usp_ctx = NULL;
    imtp_connection_t* icon = NULL;
    int retval = -1;

    when_str_empty(path, exit);
    when_not_null(host, exit);
    when_not_null(port, exit);

    amxb_usp_ctx = (amxb_usp_t*) calloc(1, sizeof(amxb_usp_t));
    when_null(amxb_usp_ctx, exit);

    retval = imtp_connection_listen(&icon, path, NULL);
    if(retval < 0) {
        free(amxb_usp_ctx);
        amxb_usp_ctx = NULL;
        goto exit;
    }

    amxb_usp_ctx->la_dm = &la_dm;
    amxb_usp_ctx->icon = icon;
    amxb_usp_ctx->sigmngr = sigmngr;

    amxb_usp_config_set_eid();

exit:
    return amxb_usp_ctx;
}

void* amxb_usp_accept(void* const ctx,
                      amxp_signal_mngr_t* sigmngr) {
    amxb_usp_t* amxb_usp_listen = (amxb_usp_t*) ctx;
    amxb_usp_t* amxb_usp_accepted = NULL;
    imtp_connection_t* con_new = NULL;
    int retval = -1;
    int fd_new = -1;

    fd_new = imtp_connection_accept(amxb_usp_listen->icon);
    if(fd_new < 0) {
        goto exit;
    }
    con_new = imtp_connection_get_con(fd_new);

    retval = amxb_usp_ctx_new(&amxb_usp_accepted, con_new, sigmngr);
    when_failed(retval, exit);

    amxb_usp_accepted->dm = amxb_usp_listen->dm;
    amxb_usp_send_handshake(amxb_usp_accepted);
    amxb_usp_eid_slot_connect(amxb_usp_accepted);

exit:
    return amxb_usp_accepted;
}

int amxb_usp_disconnect(void* ctx) {
    amxb_usp_t* amxb_usp_ctx = (amxb_usp_t*) ctx;
    int retval = -1;

    when_null(amxb_usp_ctx, exit);

    amxb_usp_ctx_remove(amxb_usp_ctx);
    amxb_usp_la_contr_del(amxb_usp_ctx);
    free(amxb_usp_ctx->contr_path);
    free(amxb_usp_ctx->eid);
    amxp_slot_disconnect_with_priv(NULL, amxb_usp_handshake_received, amxb_usp_ctx);

    amxb_usp_ctx_delete(&amxb_usp_ctx);

    retval = 0;
exit:
    return retval;
}

void amxb_usp_free(void* ctx) {
    amxb_usp_t* amxb_usp_ctx = (amxb_usp_t*) ctx;

    free(amxb_usp_ctx);

    return;
}

int amxb_usp_get_fd(void* ctx) {
    amxb_usp_t* amxb_usp_ctx = (amxb_usp_t*) ctx;
    int fd = -1;

    when_null(amxb_usp_ctx, exit);
    when_null(amxb_usp_ctx->icon, exit);

    fd = imtp_connection_get_fd(amxb_usp_ctx->icon);
exit:
    return fd;
}

int amxb_usp_read(void* ctx) {
    amxb_usp_t* amxb_usp_ctx = (amxb_usp_t*) ctx;
    int retval = -1;

    when_null(amxb_usp_ctx, exit);
    when_null(amxb_usp_ctx->icon, exit);

    // When 1 is returned, the message is partially read and buffered in ctx->icon->buffer
    // Continue and read rest next time
    retval = amxb_usp_handle_read(amxb_usp_ctx);
    if(retval == 1) {
        retval = 0;
    }

exit:
    return retval;
}

int amxb_usp_read_raw(void* ctx, void* buf, size_t count) {
    amxb_usp_t* amxb_usp_ctx = (amxb_usp_t*) ctx;
    int retval = -1;

    when_null(amxb_usp_ctx, exit);
    when_null(amxb_usp_ctx->icon, exit);
    when_null(buf, exit);

    retval = read(imtp_connection_get_fd(amxb_usp_ctx->icon), buf, count);

exit:
    return retval;
}

void amxb_usp_ctx_insert(amxb_usp_t* ctx, const char* contr_path) {
    when_null(ctx, exit);
    when_str_empty(ctx->eid, exit);
    amxc_htable_insert(&ctx_table, contr_path, &ctx->hit);
exit:
    return;
}

void amxb_usp_ctx_remove(amxb_usp_t* ctx) {
    amxc_htable_it_t* hit = NULL;

    when_null(ctx, exit);
    when_str_empty(ctx->contr_path, exit);

    hit = amxc_htable_take(&ctx_table, ctx->contr_path);
    amxc_htable_it_clean(hit, NULL);
exit:
    return;
}

amxb_usp_t* amxb_usp_ctx_get(const char* contr_path) {
    amxb_usp_t* retval = NULL;
    amxc_htable_it_t* hit = amxc_htable_get(&ctx_table, contr_path);

    when_null(hit, exit);

    retval = amxc_htable_it_get_data(hit, amxb_usp_t, hit);
exit:
    return retval;
}

bool amxb_usp_has(void* const ctx,
                  const char* object) {
    int retval = -1;
    bool has_object = false;
    amxc_var_t ret;
    amxc_string_t* dotted_object = NULL;
    bool requires_device = GET_BOOL(config_opts, AMXB_USP_COPT_REQUIRES_DEVICE);

    amxc_var_init(&ret);

    when_null(ctx, exit);
    when_str_empty(object, exit);

    if(requires_device) {
        when_false(amxb_usp_path_starts_with_device(object), exit);
    }

    dotted_object = amxb_usp_add_dot(object);
    retval = amxb_usp_get(ctx, amxc_string_get(dotted_object, 0), NULL, 0, 0, &ret, 5);
    when_failed(retval, exit);

    // If the response is not empty, the object was found
    if(GETP_ARG(&ret, "0.0") != NULL) {
        has_object = true;
    }

exit:
    amxc_string_delete(&dotted_object);
    amxc_var_clean(&ret);
    return has_object;
}

static amxb_be_funcs_t amxb_usp_impl = {
    .it = { .ait = NULL, .key = NULL, .next = NULL },
    .handle = NULL,
    .connections = { .head = NULL, .tail = NULL },
    .name = "usp",
    .size = sizeof(amxb_be_funcs_t),
    .connect = amxb_usp_connect,
    .disconnect = amxb_usp_disconnect,
    .get_fd = amxb_usp_get_fd,
    .read = amxb_usp_read,
    .new_invoke = NULL,
    .free_invoke = NULL,
    .invoke = amxb_usp_invoke,
    .async_invoke = amxb_usp_async_invoke,
    .close_request = amxb_usp_close_request,
    .wait_request = amxb_usp_wait_for_request,
    .subscribe = NULL,
    .unsubscribe = amxb_usp_unsubscribe,
    .free = amxb_usp_free,
    .register_dm = amxb_usp_register,
    .get = amxb_usp_get,
    .set = amxb_usp_set,
    .add = amxb_usp_add,
    .del = amxb_usp_delete,
    .get_supported = amxb_usp_get_supported,
    .set_config = amxb_usp_set_config,
    .describe = NULL,
    .list = NULL,
    .listen = amxb_usp_listen,
    .accept = amxb_usp_accept,
    .read_raw = amxb_usp_read_raw,
    .wait_for = NULL,
    .has = amxb_usp_has,
    .capabilities = amxb_usp_get_capabilities,
    .get_instances = amxb_usp_get_instances,
    .get_filtered = NULL,
    .subscribe_v2 = amxb_usp_subscribe_v2,
    .get_stats = amxb_usp_get_stats,
    .reset_stats = amxb_usp_reset_stats,
};

static amxb_version_t sup_min_lib_version = {
    .major = 4,
    .minor = 10,
    .build = 0
};

static amxb_version_t sup_max_lib_version = {
    .major = 4,
    .minor = -1,
    .build = -1
};

static amxb_version_t usp_be_version = {
    .major = AMXB_USP_VERSION_MAJOR,
    .minor = AMXB_USP_VERSION_MINOR,
    .build = AMXB_USP_VERSION_BUILD,
};

amxb_be_info_t amxb_usp_be_info = {
    .min_supported = &sup_min_lib_version,
    .max_supported = &sup_max_lib_version,
    .be_version = &usp_be_version,
    .name = "usp",
    .description = "AMXB Backend for USP",
    .funcs = &amxb_usp_impl,
};

amxb_be_info_t* amxb_be_info(void) {
    return &amxb_usp_be_info;
}
