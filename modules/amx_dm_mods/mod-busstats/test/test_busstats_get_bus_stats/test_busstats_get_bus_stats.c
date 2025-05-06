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
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include <amxc/amxc_macros.h>
#include <amxc/amxc.h>
#include <amxp/amxp.h>

#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>
#include <amxd/amxd_transaction.h>

#include <amxo/amxo.h>
#include <amxp/amxp_dir.h>
#include <amxb/amxb_stats.h>

#include "test_busstats_get_bus_stats.h"

static amxb_bus_ctx_t* bus_ctx1 = NULL;
static amxb_bus_ctx_t* bus_ctx2 = NULL;

#define PCB_SOCKET1 "/tmp/test_bus1_sock"
#define PCB_NAME1 "test_bus1"
#define PCB_SOCKET2 "/tmp/test_bus2_sock"
#define PCB_NAME2 "test_bus2"

// /** @implements amxp_dir_match_fn_t */
static int s_load_backend(const char* name, UNUSED void* priv) {
    assert_int_equal(0, amxb_be_load(name));
    return 0;
}

int test_busstat_get_bus_stats_setup(UNUSED void** state) {
    system("killall examplecomponent");
    system("killall " PCB_NAME1);
    system("killall " PCB_NAME2);

    assert_int_equal(0, system("pcb_sysbus -n " PCB_NAME1 " -I " PCB_SOCKET1));
    assert_int_equal(0, system("pcb_sysbus -n " PCB_NAME2 " -I " PCB_SOCKET2));

    amxp_dir_scan("/usr/bin/mods/amxb", "d_name matches '.*mod-amxb-pcb\\.so'", false, s_load_backend, NULL);

    sleep(2);
    system("ps aux | grep test_bus");

    assert_int_equal(amxb_connect(&bus_ctx1, "pcb:" PCB_SOCKET1), 0);
    assert_int_equal(amxb_connect(&bus_ctx2, "pcb:" PCB_SOCKET2), 0);

    system("./examplecomponent -A -I ../../odl/"
           " -L ./"
           " -B /usr/bin/mods/amxb/*mod-amxb-pcb.so"
           " -u pcb:" PCB_SOCKET1
           " -u pcb:" PCB_SOCKET2
           " ./examplecomponent.odl &");

    return 0;
}

int test_busstat_get_bus_stats_teardown(UNUSED void** state) {
    amxb_disconnect(bus_ctx1);
    amxb_free(&bus_ctx1);
    amxb_disconnect(bus_ctx2);
    amxb_free(&bus_ctx2);

    system("killall examplecomponent");

    system("killall " PCB_NAME1);
    system("killall " PCB_NAME2);

    unlink(PCB_SOCKET1);
    unlink(PCB_SOCKET2);

    amxb_be_remove_all();

    return 0;
}

static amxc_var_t* s_get_bus_stat(amxb_bus_ctx_t* bus_ctx) {
    amxc_var_t ret;
    amxc_var_init(&ret);
    assert_int_equal(AMXB_STATUS_OK, amxb_call(bus_ctx, "ExampleComponent.", "GetBusStats", NULL, &ret, 5));
    amxc_var_t* stats = amxc_var_get_index(&ret, 0, AMXC_VAR_FLAG_COPY);
    amxc_var_clean(&ret);
    return stats;
}

void test_busstats_get_bus_stats(UNUSED void** state) {
    // GIVEN a component that received some bus operations:
    // 5 "add"s:
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] ExampleComponent.MultiInstance.+");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] ExampleComponent.MultiInstance.+");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] ExampleComponent.MultiInstance.+");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] ExampleComponent.MultiInstance.+");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] ExampleComponent.MultiInstance.+");
    // 3 "del"s:
    system("pcb_cli pcb://ipc:[" PCB_SOCKET2 "] ExampleComponent.MultiInstance.1-");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET2 "] ExampleComponent.MultiInstance.2-");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET2 "] ExampleComponent.MultiInstance.3-");
    // 2 "set"s:
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] AnotherRootObject.Parameter3=hi");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] AnotherRootObject.Parameter3=lo");
    // 4 "get"s:
    system("pcb_cli pcb://ipc:[" PCB_SOCKET2 "] AnotherRootObject.?");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET2 "] ThirdRootObject.?");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET2 "] ExampleComponent.MultiInstance.4.?");
    // 6 "invoke"s
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] 'ExampleComponent.function()'");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] 'ExampleComponent.function()'");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] 'ExampleComponent.function()'");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] 'ExampleComponent.function()'");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] 'ExampleComponent.function()'");
    system("pcb_cli pcb://ipc:[" PCB_SOCKET1 "] 'ExampleComponent.function()'");

    // WHEN retrieving the bus statistics of that component
    amxc_var_t* bus_stats = s_get_bus_stat(bus_ctx2);

    // THEN the number of operations is correctly reported, taking into account on which bus connection it happened
    assert_int_equal(GETP_UINT32(bus_stats, "pcb:" PCB_SOCKET1 ".rx.operation.add"), 5);
    assert_int_equal(GETP_UINT32(bus_stats, "pcb:" PCB_SOCKET2 ".rx.operation.del"), 3);
    assert_int_equal(GETP_UINT32(bus_stats, "pcb:" PCB_SOCKET1 ".rx.operation.set"), 2);
    assert_int_equal(GETP_UINT32(bus_stats, "pcb:" PCB_SOCKET2 ".rx.operation.get"), 4);
    assert_int_equal(GETP_UINT32(bus_stats, "pcb:" PCB_SOCKET1 ".rx.operation.invoke"), 6);

    // (the sent operations are also counted - their values are tested in the tests of libamxb)
    assert_non_null(GETP_ARG(bus_stats, "pcb:" PCB_SOCKET1 ".tx.operation.add"));
    assert_non_null(GETP_ARG(bus_stats, "pcb:" PCB_SOCKET2 ".tx.operation.del"));
    assert_non_null(GETP_ARG(bus_stats, "pcb:" PCB_SOCKET1 ".tx.operation.set"));


    amxc_var_delete(&bus_stats);
}