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
#include "test_amxb_rbus_amxb_call.h"

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
    system("amxrt -D ../common/odl/method_calls.odl");
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


    printf("TEARDOWN: Disconnect from rbus\n");
    amxb_free(&bus_ctx);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("TEARDOWN: Disconnected ...\n");

    amxb_be_remove_all();

    amxc_var_clean(&config);
    return 0;
}

void test_amxb_rbus_call_method_synchronous(UNUSED void** state) {
    int retval = 0;
    amxc_var_t args;
    amxc_var_t ret;

    amxc_var_init(&args);
    amxc_var_init(&ret);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "message", "Hello World");

    // When calling an existing method with valid arguments
    printf("Calling method print_message\n");
    fflush(stdout);
    retval = amxb_call(bus_ctx, "Device.TestObject.", "print_message", &args, &ret, 5);

    // the return code must be amxd_status_ok
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);

    // the print_message returns the length of the message printed
    assert_int_equal(GETI_UINT32(&ret, 0), strlen("Hello World"));

    amxc_var_clean(&ret);
    amxc_var_clean(&args);
}

void test_amxb_rbus_call_method_with_out_args(UNUSED void** state) {
    int retval = 0;
    amxc_var_t args;
    amxc_var_t ret;

    amxc_var_init(&args);
    amxc_var_init(&ret);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);

    // When calling an existing method with valid arguments
    printf("Calling method test_out_args\n");
    fflush(stdout);
    retval = amxb_call(bus_ctx, "Device.TestObject.", "test_out_args", &args, &ret, 5);

    // the return code must be amxd_status_ok
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_dump(&ret, STDOUT_FILENO);
    amxc_var_dump(&args, STDOUT_FILENO);

    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    retval = check_amx_response(GETI_ARG(&ret, 1), "./verify_data/verify_out_args.json");
    assert_int_equal(retval, 0);

    amxc_var_clean(&ret);
    amxc_var_clean(&args);
}

static void async_call_done(UNUSED const amxb_bus_ctx_t* bus_ctx,
                            UNUSED amxb_request_t* req,
                            int status,
                            UNUSED void* priv) {
    printf("CALLBACK CALLED (status = %d)\n", status);
    fflush(stdout);
    check_expected(status);
}

void test_amxb_rbus_call_method_asynchronous_and_wait(UNUSED void** state) {
    amxb_request_t* request = NULL;
    amxc_var_t args;
    int retval = 0;

    amxc_var_init(&args);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "message", "Hello World");

    // When calling an existing method with valid arguments
    printf("Calling method print_message\n");
    fflush(stdout);

    expect_value(async_call_done, status, amxd_status_ok);
    request = amxb_async_call(bus_ctx, "Device.TestObject.", "print_message", &args, async_call_done, NULL);
    assert_non_null(request);

    retval = amxb_wait_for_request(request, 20);
    assert_int_equal(retval, 0);
    while(amxb_read(bus_ctx) == 0) {
    }

    assert_int_equal(amxc_var_type_of(request->result), AMXC_VAR_ID_LIST);
    assert_int_equal(GETI_UINT32(request->result, 0), strlen("Hello World"));

    amxb_close_request(&request);

    amxc_var_clean(&args);
}

void test_amxb_rbus_call_method_asynchronous_and_wait_after_done(UNUSED void** state) {
    amxb_request_t* request = NULL;
    amxc_var_t args;
    int retval = 0;

    amxc_var_init(&args);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "message", "This is a message");

    // When calling an existing method with valid arguments
    printf("Calling method print_message\n");
    fflush(stdout);

    expect_value(async_call_done, status, amxd_status_ok);
    request = amxb_async_call(bus_ctx, "Device.TestObject.", "print_message", &args, async_call_done, NULL);
    assert_non_null(request);

    sleep(1);
    while(amxb_read(bus_ctx) == 0) {
    }

    assert_int_equal(amxc_var_type_of(request->result), AMXC_VAR_ID_LIST);
    assert_int_equal(GETI_UINT32(request->result, 0), strlen("This is a message"));

    retval = amxb_wait_for_request(request, 20);
    assert_int_equal(retval, 0);

    assert_int_equal(amxc_var_type_of(request->result), AMXC_VAR_ID_LIST);
    assert_int_equal(GETI_UINT32(request->result, 0), strlen("This is a message"));

    amxb_close_request(&request);

    amxc_var_clean(&args);
}

void test_amxb_rbus_call_method_asynchronous_and_close(UNUSED void** state) {
    amxb_request_t* request = NULL;
    amxc_var_t args;

    amxc_var_init(&args);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "message", "Hello World");

    // When calling an existing method with valid arguments
    printf("Calling method print_message\n");
    fflush(stdout);

    request = amxb_async_call(bus_ctx, "Device.TestObject.", "print_message", &args, async_call_done, NULL);
    assert_non_null(request);
    amxb_close_request(&request);

    while(amxb_read(bus_ctx) == 0) {
    }

    amxc_var_clean(&args);
}

void test_amxb_rbus_call_method_asynchronous_no_callback(UNUSED void** state) {
    amxb_request_t* request = NULL;
    amxc_var_t args;
    int retval = 0;

    amxc_var_init(&args);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "message", "Hello World");

    // When calling an existing method with valid arguments
    printf("Calling method print_message\n");
    fflush(stdout);

    request = amxb_async_call(bus_ctx, "Device.TestObject.", "print_message", &args, NULL, NULL);
    assert_non_null(request);

    retval = amxb_wait_for_request(request, 10);
    assert_int_equal(retval, 0);
    while(amxb_read(bus_ctx) == 0) {
    }

    assert_int_equal(amxc_var_type_of(request->result), AMXC_VAR_ID_LIST);
    assert_int_equal(GETI_UINT32(request->result, 0), strlen("Hello World"));

    amxb_close_request(&request);

    amxc_var_clean(&args);
}

void test_amxb_rbus_call_method_asynchronous_and_pthread_create_fails(UNUSED void** state) {
    amxb_request_t* request = NULL;
    amxc_var_t args;

    amxc_var_init(&args);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "message", "Hello World");

    // When calling an existing method with valid arguments
    printf("Calling method print_message\n");
    fflush(stdout);

    cmocka_rbus_backend_enable_mock("../mod-amxb-test-rbus.so", true);
    will_return(__wrap_pthread_create, -1);
    request = amxb_async_call(bus_ctx, "Device.TestObject.", "print_message", &args, async_call_done, NULL);
    assert_null(request);
    cmocka_rbus_backend_enable_mock("../mod-amxb-test-rbus.so", false);

    amxc_var_clean(&args);
}

void test_amxb_rbus_call_non_existing_method(UNUSED void** state) {
    amxc_var_t args;
    amxc_var_t ret;
    int retval = 0;
    amxb_request_t* request = NULL;

    amxc_var_init(&args);
    amxc_var_init(&ret);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "message", "Hello World");

    retval = amxb_call(bus_ctx, "Device.TestObject.", "NoneExisting", &args, &ret, 10);
    assert_int_not_equal(retval, 0);

    retval = amxb_call(bus_ctx, "", "NoneExisting", &args, &ret, 10);
    assert_int_not_equal(retval, 0);

    retval = amxb_call(bus_ctx, "Device.NonExisting.", "NoneExisting", &args, &ret, 10);
    assert_int_not_equal(retval, 0);

    expect_value(async_call_done, status, amxd_status_object_not_found);
    request = amxb_async_call(bus_ctx, "Device.TestObject.", "NoneExisting", &args, async_call_done, NULL);
    assert_non_null(request);
    retval = amxb_wait_for_request(request, 120);
    assert_int_equal(retval, 0);
    while(amxb_read(bus_ctx) == 0) {
    }
    assert_int_not_equal(request->bus_retval, 0);
    amxb_close_request(&request);

    request = amxb_async_call(bus_ctx, "", "NoneExisting", &args, async_call_done, NULL);
    assert_null(request);

    expect_value(async_call_done, status, amxd_status_object_not_found);
    request = amxb_async_call(bus_ctx, "Device.NonExisting.", "NoneExisting", &args, async_call_done, NULL);
    assert_non_null(request);
    retval = amxb_wait_for_request(request, 120);
    assert_int_equal(retval, 0);
    while(amxb_read(bus_ctx) == 0) {
    }
    assert_int_not_equal(request->bus_retval, 0);
    amxb_close_request(&request);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_amxb_rbus_call_method_asynchronous_and_wait_timeout(UNUSED void** state) {
    amxb_request_t* request = NULL;
    amxc_var_t args;
    int retval = 0;

    amxc_var_init(&args);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "message", "Hello World");
    amxc_var_add_key(uint32_t, &args, "time", 15);

    // When calling an existing method with valid arguments
    printf("Calling method print_message deferred, will respond in 15 seconds\n");
    fflush(stdout);

    request = amxb_async_call(bus_ctx, "Device.TestObject.", "print_message_deferred", &args, async_call_done, NULL);
    assert_non_null(request);

    printf("Waiting on the request for 10 seconds - should timeout\n");
    fflush(stdout);
    retval = amxb_wait_for_request(request, 10);
    assert_int_not_equal(retval, 0);
    amxb_close_request(&request);
    printf("Waiting for 6 seconds - the async thread should get the reply, but not caal the callback\n");
    fflush(stdout);
    while(amxb_read(bus_ctx) == 0) {
    }

    amxc_var_clean(&args);
}

