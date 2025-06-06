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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <signal.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>

#include <amxd/amxd_dm.h>

#include <amxb/amxb.h>

#include "test_amxb_ubus_stats.h"
#include <amxb/amxb_stats.h>

static amxb_bus_ctx_t* bus_ctx = NULL;

static const char* test_amxb_ubus_get_socket(void) {
    struct stat sb;
    if(stat("/var/run/ubus.sock", &sb) == 0) {
        return "ubus:/var/run/ubus.sock";
    }
    if(stat("/var/run/ubus/ubus.sock", &sb) == 0) {
        return "ubus:/var/run/ubus/ubus.sock";
    }
    return NULL;
}


static void test_event_cb(UNUSED const char* const sig_name,
                          UNUSED const amxc_var_t* const data,
                          UNUSED void* const priv) {
    printf("Event recieved %s\n", sig_name);
}


static uint64_t s_get_rx_stat(const char* stat_name) {
    amxc_var_t ret;
    amxc_var_init(&ret);
    amxc_var_t args;
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "uri", "ubus:");
    assert_int_equal(AMXB_STATUS_OK, amxb_call(bus_ctx, "TestBusStats.", "get_bus_stats_for", &args, &ret, 5));
    amxc_var_t* operation_rx = GETP_ARG(GETI_ARG(&ret, 0), "rx.operation");
    assert_non_null(operation_rx);
    amxc_var_t* stat = GETP_ARG(operation_rx, stat_name);
    assert_non_null(stat);

    // ubus does not fully support uint64, so accept int64.
    assert_true(AMXC_VAR_ID_UINT64 == amxc_var_type_of(stat) || AMXC_VAR_ID_INT64 == amxc_var_type_of(stat));
    uint64_t stat_uint64 = amxc_var_dyncast(uint64_t, stat);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
    return stat_uint64;
}



int test_amxb_ubus_stats_setup(UNUSED void** state) {
    amxc_string_t txt;
    sigset_t mask;
    const char* ubus_sock = test_amxb_ubus_get_socket();
    printf("Ubus socket = %s\n", ubus_sock);

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);

    sigprocmask(SIG_BLOCK, &mask, NULL);


    amxc_string_init(&txt, 0);

    amxp_sigmngr_add_signal(NULL, "config:changed");

    amxc_string_reset(&txt);
    amxc_string_setf(&txt, "amxrt --no-auto-detect -u ubus: -B ../mod-amxb-test-ubus.so -A ../test_data/bus_stats.odl &");
    system(amxc_string_get(&txt, 0));

    amxc_string_clean(&txt);

    assert_int_equal(amxb_be_load("../mod-amxb-test-ubus.so"), 0);
    assert_int_equal(amxb_connect(&bus_ctx, ubus_sock), 0);

    sleep(1);

    return 0;
}

int test_amxb_ubus_stats_teardown(UNUSED void** state) {
    amxb_disconnect(bus_ctx);
    amxb_free(&bus_ctx);

    system("killall amxrt");

    amxb_be_remove_all();

    return 0;
}

void test_ubus_stats_invoke(UNUSED void** state) {
    // GIVEN a component running, that has received a number of method invocations
    uint64_t nb_calls_1 = s_get_rx_stat("invoke");

    // WHEN asking that component how many incoming method invocations it has received,
    //      by sending a method invocation to that component
    uint64_t nb_calls_2 = s_get_rx_stat("invoke");

    // THEN the number has increased by one (because we did one method invocation)
    assert_int_equal(nb_calls_2, nb_calls_1 + 1);
}

void test_ubus_stats_invoke_twice(UNUSED void** state) {
    // GIVEN a component running, that has received a number of method invocations
    uint64_t nb_calls_1 = s_get_rx_stat("invoke");

    // WHEN performing a method invocation on that component, and then another method invocation
    //      for fetching the number of method invocations
    amxb_call(bus_ctx, "TestBusStats.", "unimplemented_method", NULL, NULL, 5);
    uint64_t nb_calls_2 = s_get_rx_stat("invoke");

    // THEN the component has received two more method invocations
    assert_int_equal(nb_calls_2, nb_calls_1 + 2);
}

void test_ubus_stats_invoke_get(UNUSED void** state) {
    // GIVEN a component running, that has received a number of method invocations
    uint64_t nb_calls_1 = s_get_rx_stat("invoke");

    // WHEN performing a "get"
    amxb_get(bus_ctx, "TestBusStats.", 1, NULL, 5);
    uint64_t nb_calls_2 = s_get_rx_stat("invoke");

    // THEN the component has received two more method invocations (one for "get" which
    //      results in a "_get()" invocation, and one for get invocation to get the stats)
    assert_int_equal(nb_calls_2, nb_calls_1 + 2);
}

void test_ubus_stats_invoke_subscribe_unsubscribe(UNUSED void** state) {
    // GIVEN a component running, that has received a number of method invocations
    uint64_t nb_calls_1 = s_get_rx_stat("invoke");

    // WHEN performing a "subscribe" and an "unsubscribe"
    assert_int_equal(amxb_subscribe(bus_ctx, "TestBusStats.", NULL, test_event_cb, NULL), AMXB_STATUS_OK);
    assert_int_equal(amxb_unsubscribe(bus_ctx, "TestBusStats.", test_event_cb, NULL), AMXB_STATUS_OK);

    // THEN the component has received two more method invocations.
    uint64_t nb_calls_2 = s_get_rx_stat("invoke");
    assert_int_equal(nb_calls_2, nb_calls_1 + 2);
}