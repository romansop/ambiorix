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

#include <unistd.h>

#include <rbus.h>

#include "../common/include/amxb_rbus_test.h"
#include "test_amxb_rbus_native_subscribe.h"

static rbusHandle_t handle = NULL;
static uint32_t event_counter = 0;

int test_native_setup(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    cmocka_enable_check(true);

    assert_int_equal(rbus_checkStatus(), RBUS_ENABLED);

    printf("SETUP: Starting amxrt data model providers\n");
    system("amxrt -D ../common/odl/test_event_data_model.odl ../common/odl/test_event_sub_object.odl");
    printf("SETUP: Started ...\n");

    sleep(1);
    system("find /var/run/ -name \"*.pid\"");

    printf("SETUP: Connect to rbus\n");
    expect_string(__wrap_rbus_open, componentName, "rbus-unit-test");
    rc = rbus_open(&handle, "rbus-unit-test");
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("SETUP: Connected ...\n");

    return 0;
}

int test_native_teardown(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;

    system("ps -aux | grep amxrt");

    printf("TEARDOWN: Stopping amxrt data model providers\n");
    system("killall amxrt");
    printf("TEARDOWN: Stopped ...\n");


    printf("TEARDOWN: Disconnect from rbus\n");
    rc = rbus_close(handle);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("TEARDOWN: Disconnected ...\n");

    return 0;
}

static void amxb_rbus_test_event_handler(rbusHandle_t h,
                                         rbusEvent_t const* event,
                                         rbusEventSubscription_t* subscription) {
    amxc_var_t notification;
    int retval = 0;
    const char* expected_event = (const char*) subscription->userData;

    printf("Event recieved\n");
    assert_ptr_equal(h, handle);

    // convert to a amxc variant for easy checking
    amxc_var_init(&notification);
    amxb_rbus_object_to_var(&notification, event->data);

    retval = check_amx_response(&notification, expected_event);
    assert_int_equal(retval, 0);

    // increase event counter
    event_counter++;

    amxc_var_clean(&notification);
}

void test_native_rbus_can_subscribe_on_specific_event(UNUSED void** state) {
    rbusError_t status = RBUS_ERROR_SUCCESS;

    status = rbusEvent_Subscribe(handle,
                                 "Device.TestEvents.TestEvent!",
                                 amxb_rbus_test_event_handler,
                                 "./verify_data/single_specific_event.json",
                                 0);
    assert_int_equal(status, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_specific_event_is_received(UNUSED void** state) {
    rbusError_t status = RBUS_ERROR_SUCCESS;
    rbusObject_t in_params = NULL;
    rbusObject_t out_params = NULL;
    rbusValue_t v = NULL;

    rbusObject_Init(&in_params, NULL);

    rbusValue_Init(&v);
    rbusValue_SetString(v, "Hello World");
    rbusObject_SetValue(in_params, "message", v);
    rbusValue_Release(v);

    event_counter = 0;
    status = rbusMethod_Invoke(handle, "Device.TestEvents.test_send_event()", in_params, &out_params);

    assert_int_equal(status, RBUS_ERROR_SUCCESS);
    assert_non_null(out_params);

    rbusObject_Release(in_params);
    rbusObject_Release(out_params);

    // wait a moment so the event can be handled
    sleep(1);

    assert_int_equal(event_counter, 1);
}

void test_native_rbus_no_event_from_subobject_is_received(UNUSED void** state) {
    rbusError_t status = RBUS_ERROR_SUCCESS;
    rbusObject_t in_params = NULL;
    rbusObject_t out_params = NULL;
    rbusValue_t v = NULL;

    rbusObject_Init(&in_params, NULL);

    rbusValue_Init(&v);
    rbusValue_SetString(v, "Hello World");
    rbusObject_SetValue(in_params, "message", v);
    rbusValue_Release(v);

    event_counter = 0;
    status = rbusMethod_Invoke(handle, "Device.TestEvents.SubObject.test_send_event()", in_params, &out_params);

    assert_int_equal(status, RBUS_ERROR_SUCCESS);
    assert_non_null(out_params);

    rbusObject_Release(in_params);
    rbusObject_Release(out_params);

    // wait a moment so the event can be handled
    sleep(1);

    assert_int_equal(event_counter, 0);
}

void test_native_rbus_check_no_unrelated_events_are_received(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    uint32_t index = 0;

    event_counter = 0;
    rc = rbusTable_addRow(handle, "Device.TestEvents.Table.", NULL, &index);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);

    // wait a moment and check no event is received
    sleep(1);

    assert_int_equal(event_counter, 0);
}

void test_native_rbus_can_subscribe_on_ambiorix_event(UNUSED void** state) {
    rbusError_t status = RBUS_ERROR_SUCCESS;

    status = rbusEvent_Subscribe(handle,
                                 "Device.TestEvents.amx_notify!",
                                 amxb_rbus_test_event_handler,
                                 "./verify_data/single_specific_event.json",
                                 0);
    assert_int_equal(status, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_all_events_are_received(UNUSED void** state) {
    rbusError_t status = RBUS_ERROR_SUCCESS;
    rbusObject_t in_params = NULL;
    rbusObject_t out_params = NULL;
    rbusValue_t v = NULL;

    rbusObject_Init(&in_params, NULL);

    rbusValue_Init(&v);
    rbusValue_SetString(v, "Hello World");
    rbusObject_SetValue(in_params, "message", v);
    rbusValue_Release(v);

    event_counter = 0;
    status = rbusMethod_Invoke(handle, "Device.TestEvents.test_send_event()", in_params, &out_params);

    assert_int_equal(status, RBUS_ERROR_SUCCESS);
    assert_non_null(out_params);

    rbusObject_Release(in_params);
    rbusObject_Release(out_params);

    // wait a moment so the event can be handled
    sleep(1);

    assert_int_equal(event_counter, 2);
}


void test_native_rbus_can_unsubscribe(UNUSED void** state) {
    rbusError_t status = RBUS_ERROR_SUCCESS;
    status = rbusEvent_Unsubscribe(handle, "Device.TestEvents.TestEvent!");
    assert_int_equal(status, RBUS_ERROR_SUCCESS);
    status = rbusEvent_Unsubscribe(handle, "Device.TestEvents.amx_notify!");
    assert_int_equal(status, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_no_event_received_after_unsubscribe(UNUSED void** state) {
    rbusError_t status = RBUS_ERROR_SUCCESS;
    rbusObject_t in_params = NULL;
    rbusObject_t out_params = NULL;
    rbusValue_t v = NULL;

    rbusObject_Init(&in_params, NULL);

    rbusValue_Init(&v);
    rbusValue_SetString(v, "Hello World");
    rbusObject_SetValue(in_params, "message", v);
    rbusValue_Release(v);

    event_counter = 0;
    status = rbusMethod_Invoke(handle, "Device.TestEvents.test_send_event()", in_params, &out_params);

    assert_int_equal(status, RBUS_ERROR_SUCCESS);
    assert_non_null(out_params);

    rbusObject_Release(in_params);
    rbusObject_Release(out_params);

    // wait a moment and check no event is received
    sleep(1);

    assert_int_equal(event_counter, 0);
}
