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

#include "test_amxb_pcb_stats.h"
#include <amxb/amxb_stats.h>

static amxb_bus_ctx_t* bus_ctx = NULL;

static uint64_t s_get_rx_stat(const char* stat_name) {
    amxc_var_t ret;
    amxc_var_init(&ret);
    amxc_var_t args;
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "uri", "pcb:/tmp/test.sock");
    assert_int_equal(AMXB_STATUS_OK, amxb_call(bus_ctx, "TestBusStats.", "get_bus_stats_for", &args, &ret, 5));
    amxc_var_t* rx_operation = GETP_ARG(GETI_ARG(&ret, 0), "rx.operation");
    assert_non_null(rx_operation);
    amxc_var_t* stat = GETP_ARG(rx_operation, stat_name);
    assert_non_null(stat);

    assert_true(AMXC_VAR_ID_UINT64 == amxc_var_type_of(stat));
    uint64_t stat_uint64 = amxc_var_dyncast(uint64_t, stat);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
    return stat_uint64;
}

static void test_generic_setup() {
    amxc_string_t txt;
    amxc_string_init(&txt, 0);

    amxc_string_setf(&txt, "pcb_sysbus -n test_bus -I /tmp/test.sock");
    system(amxc_string_get(&txt, 0));
    sleep(1);
    amxc_string_clean(&txt);
}

static void test_generic_teardown() {
    amxc_string_t txt;
    amxc_string_init(&txt, 0);

    amxc_string_setf(&txt, "killall test_bus");
    system(amxc_string_get(&txt, 0));


    unlink("/tmp/test.sock");

    amxc_string_clean(&txt);
}

int test_amxrt_pcb_stats_amx_setup(UNUSED void** state) {
    amxc_string_t txt;

    test_generic_setup();

    amxc_string_init(&txt, 0);

    unlink("test_config.odl");
    int fd = open("test_config.odl", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH);
    assert_int_not_equal(fd, -1);
    if(use_async_handlers()) {
        amxc_string_setf(&txt, "%%config { %%global backends = ['../mod-amxb-test-pcb.so']; %%global uris = [ 'pcb:/tmp/test.sock' ];  %%global pcb.use-async-handlers = true; }\n");
    } else {
        amxc_string_setf(&txt, "%%config { %%global backends = ['../mod-amxb-test-pcb.so']; %%global uris = [ 'pcb:/tmp/test.sock' ];  %%global pcb.use-async-handlers = false; }\n");
    }
    write(fd, amxc_string_get(&txt, 0), amxc_string_text_length(&txt));
    close(fd);

    amxc_string_reset(&txt);
    amxc_string_setf(&txt, "amxrt  -A -D test_config.odl ../test_data/bus_stats.odl");
    system(amxc_string_get(&txt, 0));

    amxc_string_clean(&txt);

    assert_int_equal(amxb_be_load("../mod-amxb-test-pcb.so"), 0);
    assert_int_equal(amxb_connect(&bus_ctx, "pcb:/tmp/test.sock"), 0);

    sleep(1);

    return 0;
}

int test_amxrt_pcb_stats_amx_teardown(UNUSED void** state) {
    amxb_disconnect(bus_ctx);
    amxb_free(&bus_ctx);

    system("killall amxrt");

    amxb_be_remove_all();
    unlink("test_config.odl");
    test_generic_teardown();

    return 0;
}

void test_pcb_stats_invoke(UNUSED void** state) {
    // GIVEN a component running, that has received a number of method invocations
    uint64_t nb_calls_1 = s_get_rx_stat("invoke");

    // WHEN asking that component how many incoming method invocations it has received,
    //      by sending a method invocation to that component
    uint64_t nb_calls_2 = s_get_rx_stat("invoke");

    // THEN the number has increased by one (because we did one method invocation)
    assert_int_equal(nb_calls_2, nb_calls_1 + 1);
}

void test_pcb_stats_invoke_twice(UNUSED void** state) {
    // GIVEN a component running, that has received a number of method invocations
    uint64_t nb_calls_1 = s_get_rx_stat("invoke");

    // WHEN performing a method invocation on that component, and then another method invocation
    //      for fetching the number of method invocations
    system("pcb_cli pcb://ipc:[/tmp/test.sock] 'TestBusStats.unimplemented_method()'");
    uint64_t nb_calls_2 = s_get_rx_stat("invoke");

    // THEN the component has received two more method invocations
    assert_int_equal(nb_calls_2, nb_calls_1 + 2);
}

void test_pcb_stats_get(UNUSED void** state) {
    // GIVEN a component running, that has received a number of "get"s
    uint64_t nb_get_1 = s_get_rx_stat("get");

    // WHEN performing a "get"
    system("pcb_cli pcb://ipc:[/tmp/test.sock] TestBusStats.?");
    uint64_t nb_get_2 = s_get_rx_stat("get");

    // THEN the component has received one more "get" (plus one extra that is caused by `s_get_rx_stat`)
    assert_int_equal(nb_get_2, nb_get_1 + 1 + 1);
}

void test_pcb_stats_add(UNUSED void** state) {
    // GIVEN a component running, that has received a number of "add" operations
    uint64_t nb_add_1 = s_get_rx_stat("add");

    // WHEN performing an "add"
    system("pcb_cli pcb://ipc:[/tmp/test.sock] TestBusStats.MultiInstance.+");

    // THEN the component has received one more "add"
    uint64_t nb_add_2 = s_get_rx_stat("add");
    assert_int_equal(nb_add_2, nb_add_1 + 1);
}


void test_pcb_stats_del(UNUSED void** state) {
    // GIVEN a component running
    uint64_t nb_del_1 = s_get_rx_stat("del");

    // WHEN performing an "del"
    system("pcb_cli pcb://ipc:[/tmp/test.sock] TestBusStats.MultiInstance.12345.-");

    // THEN the component has received one more "del"
    uint64_t nb_del_2 = s_get_rx_stat("del");
    assert_int_equal(nb_del_2, nb_del_1 + 1);
}

void test_pcb_stats_set(UNUSED void** state) {
    // GIVEN a component running
    uint64_t nb_set_1 = s_get_rx_stat("set");

    // WHEN performing an "set"
    system("pcb_cli pcb://ipc:[/tmp/test.sock] TestBusStats.MultiInstance.123.Text='hello'");

    // THEN the component has received one more "set"
    uint64_t nb_set_2 = s_get_rx_stat("set");
    assert_int_equal(nb_set_2, nb_set_1 + 1);
}

void test_pcb_stats_close_request(UNUSED void** state) {
    // GIVEN a component running
    uint64_t nb_close_request_1 = s_get_rx_stat("close_request");

    // WHEN performing an "set"
    system("pcb_cli pcb://ipc:[/tmp/test.sock] 'TestBusStats.?&' & PCB_CLI_PID=$! ; sleep 0.5 ; kill $PCB_CLI_PID");

    // THEN the component has received one more "close_request"
    uint64_t nb_close_request_2 = s_get_rx_stat("close_request");
    assert_int_equal(nb_close_request_2, nb_close_request_1 + 1);
}