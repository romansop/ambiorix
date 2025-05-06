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

#define _GNU_SOURCE
#include <string.h>
#include <unistd.h>

#include <amxut/amxut_timer.h>

#include "../common/include/amxb_rbus_test.h"
#include "test_amxb_rbus_amxb_wait_for.h"

static amxb_bus_ctx_t* bus_ctx = NULL;
static amxc_var_t config;
static uint32_t count = 0;

static void amxb_rbus_objects_available(const char* const sig_name,
                                        const amxc_var_t* const data,
                                        void* const priv) {
    printf("WAIT DONE\n");
    assert_string_equal(sig_name, "wait:done");
    assert_true(amxc_var_is_null(data));
    check_expected_ptr(priv);
    count++;
}

int test_amx_setup(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_init(&config);
    amxc_var_set_type(&config, AMXC_VAR_ID_HTABLE);

    assert_int_equal(amxb_be_load("../mod-amxb-test-rbus.so"), 0);
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    amxc_var_add_key(bool, rbus_config, "use-amx-calls", false);
    assert_int_equal(amxb_set_config(&config), 0);

    printf("SETUP: Connect to rbus\n");
    assert_int_equal(amxb_connect(&bus_ctx, "rbus:"), 0);
    amxb_set_access(bus_ctx, AMXB_PUBLIC);
    printf("SETUP: Connected ...\n");

    amxp_sigmngr_add_signal(NULL, "wait:done");
    amxp_slot_connect(NULL, "wait:done", NULL, amxb_rbus_objects_available, &count);

    return 0;
}

int test_amx_teardown(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;

    printf("TEARDOWN: Stopping amxrt data model providers\n");
    system("killall amxrt");
    printf("TEARDOWN: Stopped ...\n");

    sleep(1);

    printf("TEARDOWN: Disconnect from rbus\n");
    amxb_free(&bus_ctx);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("TEARDOWN: Disconnected ...\n");

    amxb_be_remove_all();

    amxc_var_clean(&config);
    return 0;
}

void test_amx_setup_wait_for_already_available_object(UNUSED void** state) {
    system("amxrt -D ../common/odl/softwaremodules_translated.odl");
    sleep(1);

    expect_value(amxb_rbus_objects_available, priv, &count);
    assert_int_equal(amxb_wait_for_object("Device.SoftwareModules.ExecEnv."), 0);

    assert_int_not_equal(handle_events(), 0);
    assert_int_equal(count, 1);

    system("killall amxrt");
    sleep(1);
}

void test_amx_setup_wait_unavailable_object(UNUSED void** state) {
    count = 0;

    assert_int_equal(amxb_wait_for_object("Device.SoftwareModules.ExecEnv."), 0);
    assert_int_equal(handle_events(), 0);
    assert_int_equal(count, 0);

    system("amxrt -D ../common/odl/softwaremodules_translated.odl");
    sleep(1);

    expect_value(amxb_rbus_objects_available, priv, &count);
    amxut_timer_go_to_future_ms(2000);
    assert_int_equal(count, 1);

    system("killall amxrt");
    sleep(1);
}

void test_amx_setup_wait_multiple_unavailable_objects(UNUSED void** state) {
    count = 0;

    assert_int_equal(amxb_wait_for_object("Device.SoftwareModules.ExecEnv."), 0);
    assert_int_equal(amxb_wait_for_object("TopLevel.Level1.1.Level2A.Level3."), 0);
    assert_int_equal(handle_events(), 0);
    assert_int_equal(count, 0);

    system("amxrt -D ../common/odl/softwaremodules_translated.odl");
    sleep(1);

    amxut_timer_go_to_future_ms(2000);

    system("amxrt -D ../common/odl/registration_test.odl");
    sleep(1);

    expect_value(amxb_rbus_objects_available, priv, &count);
    amxut_timer_go_to_future_ms(2000);
    assert_int_equal(count, 1);

    system("killall amxrt");
    sleep(1);
}

