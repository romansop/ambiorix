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

#include "../common/include/amxb_rbus_test.h"
#include "test_amxb_rbus_amxb_event.h"

static amxb_bus_ctx_t* bus_ctx = NULL;
static amxc_var_t config;

int test_amx_setup(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_init(&config);
    amxc_var_set_type(&config, AMXC_VAR_ID_HTABLE);

    assert_int_equal(amxb_be_load("../mod-amxb-test-rbus.so"), 0);
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    amxc_var_add_key(bool, rbus_config, "use-amx-calls", true);
    assert_int_equal(amxb_set_config(&config), 0);

    printf("SETUP: Starting amxrt data model providers\n");
    system("amxrt -D ../common/odl/test_event_data_model.odl");
    printf("SETUP: Started ...\n");

    sleep(1);

    printf("SETUP: Connect to rbus\n");
    assert_int_equal(amxb_connect(&bus_ctx, "rbus:"), 0);
    amxb_set_access(bus_ctx, AMXB_PUBLIC);
    printf("SETUP: Connected ...\n");

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

int test_amx_setup_no_amx_calls(UNUSED void** state) {
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");
    printf("Turn off amx calls\n");
    fflush(stdout);
    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer

    return 0;
}

int test_amx_teardown_no_amx_calls(UNUSED void** state) {
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");
    printf("Turn on amx calls\n");
    fflush(stdout);
    amxc_var_set(bool, amx_calls, true); // Turn amx-calls on for this test data model consumer

    return 0;
}

static void test_amxb_rbus_verify_event(UNUSED const char* const sig_name,
                                        const amxc_var_t* const data,
                                        UNUSED void* const priv) {
    const char* path = GET_CHAR(data, "path");
    const char* notification = GET_CHAR(data, "notification");

    printf("Event recieved\n");
    fflush(stdout);
    amxc_var_dump(data, STDOUT_FILENO);

    assert_non_null(path);
    assert_non_null(notification);

    check_expected(path);
    check_expected(notification);

}

static void test_amxb_rbus_changed(const char* text, uint32_t nr, bool check_event, bool unsubscribe) {
    int retval = 0;
    int i = 0;
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&values);
    amxc_var_init(&ret);

    // Subscribe for events
    retval = amxb_subscribe(bus_ctx, "Device.TestEvents.", NULL, test_amxb_rbus_verify_event, NULL);
    assert_int_equal(retval, 0);

    // Change a parameter
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "Text", text);
    amxc_var_add_key(uint32_t, &values, "Number", nr);
    retval = amxb_set(bus_ctx, "Device.TestEvents.", &values, &ret, 5);
    assert_int_equal(retval, 0);

    if(check_event) {
        // Check that a object-changed event is recieved
        expect_string(test_amxb_rbus_verify_event, path, "Device.TestEvents.");
        expect_string(test_amxb_rbus_verify_event, notification, "dm:object-changed");
    }
    for(i = 0; handle_events() == 0 && i < 5; i++) {
        sleep(1);
    }
    if(!check_event) {
        assert_int_equal(i, 5);
    }

    if(unsubscribe) {
        retval = amxb_unsubscribe(bus_ctx, "Device.TestEvents.", test_amxb_rbus_verify_event, NULL);
        assert_int_equal(retval, 0);
    }

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_amxb_rbus_object_changed_event(UNUSED void** state) {
    test_amxb_rbus_changed("Hello World", 101, true, true);
    test_amxb_rbus_changed("Hello World", 101, false, true);
    test_amxb_rbus_changed("Changed Text", 102, true, true);
    test_amxb_rbus_changed("Changed Text", 102, false, true);
}

void test_amxb_rbus_object_add_event(UNUSED void** state) {
    int retval = 0;
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&values);
    amxc_var_init(&ret);

    // Make sure no pending events are available
    handle_events();

    // Subscribe for events
    retval = amxb_subscribe(bus_ctx, "Device.TestEvents.Table.", NULL, test_amxb_rbus_verify_event, NULL);
    assert_int_equal(retval, 0);

    // Change a parameter
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    retval = amxb_add(bus_ctx, "Device.TestEvents.Table.", 0, NULL, &values, &ret, 5);
    assert_int_equal(retval, 0);

    // Check that a instance-added event is recieved
    expect_string(test_amxb_rbus_verify_event, path, "Device.TestEvents.Table.");
    expect_string(test_amxb_rbus_verify_event, notification, "dm:instance-added");
    for(int i = 0; handle_events() == 0 && i < 5; i++) {
        sleep(1);
    }

    retval = amxb_unsubscribe(bus_ctx, "Device.TestEvents.Table.", test_amxb_rbus_verify_event, NULL);
    assert_int_equal(retval, 0);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_amxb_rbus_object_delete_event(UNUSED void** state) {
    int retval = 0;
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&values);
    amxc_var_init(&ret);

    // Make sure no pending events are available
    handle_events();

    // Subscribe for events
    retval = amxb_subscribe(bus_ctx, "Device.TestEvents.Table.", NULL, test_amxb_rbus_verify_event, NULL);
    assert_int_equal(retval, 0);

    // Change a parameter
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    retval = amxb_del(bus_ctx, "Device.TestEvents.Table.", 1, NULL, &ret, 5);
    assert_int_equal(retval, 0);

    // This sleep is needed to make sure a thread context switch will happen
    // Otherwise it is possible that the event is not yet recieved, when
    // handeling the events.
    sleep(1);

    // Check that a instance-added event is recieved
    expect_string(test_amxb_rbus_verify_event, path, "Device.TestEvents.Table.");
    expect_string(test_amxb_rbus_verify_event, notification, "dm:instance-removed");
    handle_events();

    retval = amxb_unsubscribe(bus_ctx, "Device.TestEvents.Table.", test_amxb_rbus_verify_event, NULL);
    assert_int_equal(retval, 0);

    sleep(1);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_amxb_rbus_multiple_clients_can_subsribe(UNUSED void** state) {
    int retval = 0;
    int componentCnt = 0;
    char** pComponentNames = NULL;
    bool found = false;

    // Start a test client.
    // This test client takes a subscription and when an event is recieved
    // it stops itself.
    system("amxrt ../common/odl/test_client.odl &");
    sleep(1); // make sure it is started and connected to rbus.

    // Check if test client component is available
    retval = rbus_discoverRegisteredComponents(&componentCnt, &pComponentNames);
    assert_int_equal(retval, 0);
    for(int i = 0; i < componentCnt; i++) {
        if(strncmp(pComponentNames[i], "test-client", strlen("test-client")) == 0) {
            found = true;
        }
        free(pComponentNames[i]);
    }
    free(pComponentNames);
    assert_true(found);

    // generate an event
    test_amxb_rbus_changed("Hello World", 101, true, true);

    sleep(1); // make sure that the event can be recieved by the test-client and discconect from rbus.

    found = false;
    retval = rbus_discoverRegisteredComponents(&componentCnt, &pComponentNames);
    assert_int_equal(retval, 0);
    for(int i = 0; i < componentCnt; i++) {
        if(strncmp(pComponentNames[i], "test-client", strlen("test-client")) == 0) {
            found = true;
        }
        free(pComponentNames[i]);
    }
    free(pComponentNames);
    assert_false(found);

}

void test_amxb_rbus_subscriptions_are_removed_when_stopped(UNUSED void** state) {
    int retval = 0;

    // Make sure no pending events are available
    handle_events();

    // Subscribe for events
    retval = amxb_subscribe(bus_ctx, "Device.TestEvents.Table.", NULL, test_amxb_rbus_verify_event, NULL);
    assert_int_equal(retval, 0);

    // when the subscription is successfull and no unsubscribe is done,
    // the backend shoul clean-up the subscription when the connection is closed
    // this test is here to check taht stopping without unsubscribing doesn't
    // generate a memory leak
}
