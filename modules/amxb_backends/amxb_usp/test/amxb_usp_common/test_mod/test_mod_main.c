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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <amxc/amxc_macros.h>
#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>
#include <amxd/amxd_object_event.h>
#include <amxd/amxd_function.h>
#include <amxd/amxd_transaction.h>
#include <amxo/amxo.h>
#include <amxb/amxb_stats.h>

typedef struct _test_mod {
    amxd_dm_t* dm;
    amxo_parser_t* parser;
} test_mod_t;

typedef struct _deferred_data {
    uint64_t call_id;
    amxp_timer_t* timer;
    amxc_var_t data;
} deferred_data_t;

static test_mod_t app;
static amxp_timer_t* update_timer = NULL;

static void print_message_impl(UNUSED amxp_timer_t* timer, void* priv) {
    amxc_var_t ret;
    deferred_data_t* data = (deferred_data_t*) priv;
    const char* message = GET_CHAR(&data->data, "message");

    amxc_var_init(&ret);
    printf("%s\n", message);
    amxc_var_set(uint32_t, &ret, strlen(message));

    amxd_function_deferred_done(data->call_id, amxd_status_ok, NULL, &ret);
    amxc_var_clean(&data->data);
    amxc_var_clean(&ret);
    amxp_timer_delete(&data->timer);
    free(data);
}

static void print_message_deferred_cancel(UNUSED uint64_t call_id, void* priv) {
    deferred_data_t* data = (deferred_data_t*) priv;
    amxc_var_clean(&data->data);
    amxp_timer_delete(&data->timer);
    free(data);
}

static void test_out_args_impl(UNUSED amxp_timer_t* timer, void* priv) {
    amxc_var_t ret;
    amxc_var_t out;
    amxc_var_t* tmp = NULL;
    deferred_data_t* data = (deferred_data_t*) priv;

    amxc_var_init(&ret);
    amxc_var_init(&out);
    amxc_var_set(uint32_t, &ret, 0);
    amxc_var_set_type(&out, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &out, "message", "Hello World");
    tmp = amxc_var_add_key(amxc_htable_t, &out, "data", NULL);
    tmp = amxc_var_add_key(amxc_htable_t, tmp, "HostObject.1.", NULL);
    amxc_var_add_key(cstring_t, tmp, "Source", "testSource");
    amxc_var_add_key(cstring_t, tmp, "Destination", "testDestination");
    amxc_var_add_key(cstring_t, tmp, "Options", "Option1,Option2");
    tmp = amxc_var_add_key(amxc_htable_t, tmp, "EnvVariable.1.", NULL);
    amxc_var_add_key(cstring_t, tmp, "Key", "testKey");
    amxc_var_add_key(cstring_t, tmp, "Value", "testValue");
    amxc_var_add_key(cstring_t, tmp, "ModuleVersion", "testVersion");

    amxd_function_deferred_done(data->call_id, amxd_status_ok, &out, &ret);
    amxc_var_clean(&data->data);
    amxc_var_clean(&ret);
    amxc_var_clean(&out);
    amxp_timer_delete(&data->timer);
    free(data);
}

static void test_out_args_deferred_cancel(UNUSED uint64_t call_id, void* priv) {
    deferred_data_t* data = (deferred_data_t*) priv;
    amxc_var_clean(&data->data);
    amxp_timer_delete(&data->timer);
    free(data);
}

amxd_status_t _print_message(UNUSED amxd_object_t* object,
                             UNUSED amxd_function_t* func,
                             amxc_var_t* args,
                             amxc_var_t* ret) {
    const char* message = GET_CHAR(args, "message");

    printf("%s\n", message);
    amxc_var_set(uint32_t, ret, strlen(message));

    return amxd_status_ok;
}

amxd_status_t _print_message_deferred(UNUSED amxd_object_t* object,
                                      UNUSED amxd_function_t* func,
                                      amxc_var_t* args,
                                      amxc_var_t* ret) {
    uint32_t time = GET_UINT32(args, "time");
    deferred_data_t* data = (deferred_data_t*) calloc(1, sizeof(deferred_data_t));

    amxc_var_move(&data->data, args);
    amxd_function_defer(func, &data->call_id, ret, print_message_deferred_cancel, data);

    amxp_timer_new(&data->timer, print_message_impl, data);
    amxp_timer_start(data->timer, time * 1000);

    return amxd_status_deferred;
}

amxd_status_t _test_out_args(UNUSED amxd_object_t* object,
                             UNUSED amxd_function_t* func,
                             amxc_var_t* args,
                             amxc_var_t* ret) {
    amxc_var_t* tmp = NULL;

    amxc_var_set(uint32_t, ret, 0);
    amxc_var_set_type(args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, args, "message", "Hello World");
    tmp = amxc_var_add_key(amxc_htable_t, args, "data", NULL);
    tmp = amxc_var_add_key(amxc_htable_t, tmp, "HostObject.1.", NULL);
    amxc_var_add_key(cstring_t, tmp, "Source", "testSource");
    amxc_var_add_key(cstring_t, tmp, "Destination", "testDestination");
    amxc_var_add_key(cstring_t, tmp, "Options", "Option1,Option2");
    tmp = amxc_var_add_key(amxc_htable_t, tmp, "EnvVariable.1.", NULL);
    amxc_var_add_key(cstring_t, tmp, "Key", "testKey");
    amxc_var_add_key(cstring_t, tmp, "Value", "testValue");
    amxc_var_add_key(cstring_t, tmp, "ModuleVersion", "testVersion");

    return amxd_status_ok;
}

amxd_status_t _test_out_args_deferred(UNUSED amxd_object_t* object,
                                      UNUSED amxd_function_t* func,
                                      amxc_var_t* args,
                                      amxc_var_t* ret) {
    uint32_t time = GET_UINT32(args, "time");
    deferred_data_t* data = (deferred_data_t*) calloc(1, sizeof(deferred_data_t));

    amxc_var_move(&data->data, args);
    amxd_function_defer(func, &data->call_id, ret, test_out_args_deferred_cancel, data);

    amxp_timer_new(&data->timer, test_out_args_impl, data);
    amxp_timer_start(data->timer, time * 1000);

    return amxd_status_deferred;
}

static void test_update_counter(UNUSED amxp_timer_t* timer, UNUSED void* priv) {
    amxd_object_t* object = amxd_dm_findf(app.dm, "Device.TestObject.");
    uint32_t counter = amxd_object_get_value(uint32_t, object, "Counter", NULL);
    amxd_trans_t transaction;

    printf("Update counter (current value = %d)\n", counter);
    amxd_trans_init(&transaction);
    amxd_trans_select_pathf(&transaction, "Device.TestObject.");
    amxd_trans_set_value(uint32_t, &transaction, "Counter", ++counter);
    amxd_trans_apply(&transaction, app.dm);
    amxd_trans_clean(&transaction);

    counter = amxd_object_get_value(uint32_t, object, "Counter", NULL);
    printf("counter %d\n", counter);
    fflush(stdout);
}

amxd_status_t _test_start_timer(UNUSED amxd_object_t* object,
                                UNUSED amxd_function_t* func,
                                amxc_var_t* args,
                                UNUSED amxc_var_t* ret) {
    uint32_t interval = GET_UINT32(args, "ms_interval");
    amxd_status_t status = amxd_status_ok;

    if(update_timer == NULL) {
        printf("Start timer\n");
        fflush(stdout);
        amxp_timer_new(&update_timer, test_update_counter, NULL);
        amxp_timer_set_interval(update_timer, interval);
        amxp_timer_start(update_timer, 0);
    } else {
        status = amxd_status_invalid_action;
    }

    return status;
}

amxd_status_t _test_stop_timer(UNUSED amxd_object_t* object,
                               UNUSED amxd_function_t* func,
                               UNUSED amxc_var_t* args,
                               UNUSED amxc_var_t* ret) {
    amxd_status_t status = amxd_status_ok;

    if(update_timer != NULL) {
        amxp_timer_delete(&update_timer);
    } else {
        status = amxd_status_invalid_action;
    }

    return status;
}

amxd_status_t _test_reset_counter(UNUSED amxd_object_t* object,
                                  UNUSED amxd_function_t* func,
                                  UNUSED amxc_var_t* args,
                                  UNUSED amxc_var_t* ret) {
    amxd_trans_t transaction;

    amxd_trans_init(&transaction);
    amxd_trans_select_pathf(&transaction, "Device.TestObject.");
    amxd_trans_set_value(uint32_t, &transaction, "Counter", 0);
    amxd_trans_apply(&transaction, app.dm);
    amxd_trans_clean(&transaction);

    return amxd_status_ok;
}

amxd_status_t _test_mod_always_fail_del_inst(amxd_object_t* const object,
                                             UNUSED amxd_param_t* const p,
                                             amxd_action_t reason,
                                             UNUSED const amxc_var_t* const args,
                                             UNUSED amxc_var_t* const retval,
                                             UNUSED void* priv) {
    amxd_status_t status = amxd_status_unknown_error;

    when_null(object, exit);
    when_true_status(reason != action_object_del_inst,
                     exit,
                     status = amxd_status_function_not_implemented);

exit:
    return status;
}

amxd_status_t _send_event(amxd_object_t* object,
                          UNUSED amxd_function_t* func,
                          amxc_var_t* args,
                          UNUSED amxc_var_t* ret) {
    amxc_var_t event_params;
    amxc_var_t* data = NULL;

    amxc_var_init(&event_params);
    amxc_var_set_type(&event_params, AMXC_VAR_ID_HTABLE);
    data = amxc_var_add_key(amxc_htable_t, &event_params, "data", NULL);
    amxc_var_move(data, args);

    amxd_object_send_signal(object, "MyEvent!", &event_params, false);

    amxc_var_clean(&event_params);
    return amxd_status_ok;
}

amxd_status_t _get_bus_stats_for(UNUSED amxd_object_t* object,
                                 UNUSED amxd_function_t* func,
                                 amxc_var_t* args,
                                 amxc_var_t* ret) {

    const char* uri = GET_CHAR(args, "uri");
    amxb_bus_ctx_t* ctx = amxb_find_uri(uri);
    if(ctx == NULL) {
        printf("No bus found with given URI");
        return amxd_status_unknown_error;
    } else {
        amxb_stats_get(ctx, ret);
        return amxd_status_ok;
    }
}

int _test_mod_main(int reason, amxd_dm_t* dm, amxo_parser_t* parser) {
    switch(reason) {
    case 0:     // START
        printf("TEST MOD START\n");
        app.dm = dm;
        app.parser = parser;
        break;
    case 1:     // STOP
        printf("TEST MOD STOP\n");
        amxp_timer_delete(&update_timer);
        app.dm = NULL;
        app.parser = NULL;
        break;
    }

    return 0;
}
