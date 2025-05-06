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
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_transaction.h>
#include <amxb/amxb.h>
#include <amxo/amxo.h>

#include <amxut/amxut_util.h>
#include <amxut/amxut_verify.h>

#include "amxb_usp.h"
#include "test_amxb_usp_stats.h"
#include "test_amxb_usp_common.h"
#include "imtp_mock.h"

#define DATAMODEL_PATH "Device.Greeter."

static uint64_t s_get_rx_stat(const char* stat_name) {
    amxc_var_t ret;
    amxc_var_init(&ret);
    amxc_var_t args;
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    // Note: there's both usp:/tmp/broker_agent_path and usp:/tmp/broker_controller_path
    amxc_var_add_key(cstring_t, &args, "uri", "usp:/tmp/broker_controller_path");
    assert_int_equal(AMXB_STATUS_OK, amxb_call(test_get_bus_ctx(), DATAMODEL_PATH, "get_bus_stats_for", &args, &ret, 5));
    amxc_var_t* rx_operation = GETP_ARG(GETI_ARG(&ret, 0), "rx.operation");
    assert_non_null(rx_operation);
    amxc_var_t* stat = GETP_ARG(rx_operation, stat_name);
    assert_non_null(stat);

    // Looks like sending an UINT64 over USP ends up as an INT64. Currently not clear why.
    assert_true(AMXC_VAR_ID_INT64 == amxc_var_type_of(stat) || AMXC_VAR_ID_UINT64 == amxc_var_type_of(stat));
    uint64_t stat_uint64 = amxc_var_dyncast(uint64_t, stat);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
    return stat_uint64;
}

int test_usp_stats_setup(void** state) {
    return test_common_obuspa_setup(state);
}

int test_usp_stats_teardown(void** state) {
    return test_common_obuspa_teardown(state);
}

void test_usp_stats_invoke(UNUSED void** state) {
    // GIVEN a component running, that has received a number of method invocations
    uint64_t nb_calls_1 = s_get_rx_stat("invoke");

    // WHEN asking that component how many incoming method invocations it has received,
    //      by sending a method invocation to that component
    uint64_t nb_calls_2 = s_get_rx_stat("invoke");

    // THEN the number has increased by one (because we did one method invocation)
    assert_int_equal(nb_calls_2, nb_calls_1 + 1);
}

void test_usp_stats_invoke_twice(UNUSED void** state) {
    // GIVEN a component running, that has received a number of method invocations
    uint64_t nb_calls_1 = s_get_rx_stat("invoke");

    // WHEN performing a method invocation on that component, and then another method invocation
    //      for fetching the number of method invocations
    amxb_call(test_get_bus_ctx(), DATAMODEL_PATH, "test_out_args", NULL, NULL, 5);
    uint64_t nb_calls_2 = s_get_rx_stat("invoke");

    // THEN the component has received two more method invocations
    assert_int_equal(nb_calls_2, nb_calls_1 + 2);
}

void test_usp_stats_get(UNUSED void** state) {
    // GIVEN a component running, that has received a number of "get"s
    uint64_t nb_get_1 = s_get_rx_stat("get");

    // WHEN performing a "get"
    amxb_get(test_get_bus_ctx(), DATAMODEL_PATH, 1, NULL, 5);
    uint64_t nb_get_2 = s_get_rx_stat("get");

    // THEN the component has received more "get"s
    assert_true(nb_get_2 > nb_get_1);
}

void test_usp_stats_add(UNUSED void** state) {
    // GIVEN a component running, that has received a number of "add" operations
    uint64_t nb_add_1 = s_get_rx_stat("add");

    // WHEN performing an "add"
    amxb_add(test_get_bus_ctx(), DATAMODEL_PATH "History.", 0, NULL, NULL, NULL, 5);

    // THEN the component has received one more "add"
    uint64_t nb_add_2 = s_get_rx_stat("add");
    assert_int_equal(nb_add_2, nb_add_1 + 1);
}


void test_usp_stats_del(UNUSED void** state) {
    // GIVEN a component running with an intsance of a multi-instance object.
    uint32_t index = 2130;
    amxb_add(test_get_bus_ctx(), DATAMODEL_PATH "History.", index, NULL, NULL, NULL, 5);
    uint64_t nb_del_1 = s_get_rx_stat("del");

    // WHEN performing an "del"
    amxb_del(test_get_bus_ctx(), DATAMODEL_PATH "History.", index, NULL, NULL, 5);

    // THEN the component has received one more "del"
    uint64_t nb_del_2 = s_get_rx_stat("del");
    assert_int_equal(nb_del_2, nb_del_1 + 1);
}

void test_usp_stats_set(UNUSED void** state) {
    // GIVEN a component running
    uint64_t nb_set_1 = s_get_rx_stat("set");
    amxb_add(test_get_bus_ctx(), DATAMODEL_PATH "History.", 7777, NULL, NULL, NULL, 5);

    // WHEN performing an "set"
    amxc_var_t params;
    amxc_var_init(&params);
    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "From", __func__);
    amxb_set(test_get_bus_ctx(), DATAMODEL_PATH "History.7777.", &params, NULL, 5);

    // THEN the component has received one more "set"
    uint64_t nb_set_2 = s_get_rx_stat("set");
    assert_int_equal(nb_set_2, nb_set_1 + 1);

    amxc_var_clean(&params);
}

void test_usp_stats_get_instances(UNUSED void** state) {
    // GIVEN a component running
    uint64_t nb_get_instances_1 = s_get_rx_stat("get_instances");

    // WHEN performing an "get_instances"
    amxb_get_instances(test_get_bus_ctx(), DATAMODEL_PATH "History.", 10, NULL, 5);

    // THEN the component has received one more "get_instances"
    uint64_t nb_get_instances_2 = s_get_rx_stat("get_instances");
    assert_int_equal(nb_get_instances_2, nb_get_instances_1 + 1);
}
