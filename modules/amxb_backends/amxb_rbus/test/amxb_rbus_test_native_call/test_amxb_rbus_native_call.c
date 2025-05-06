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
#include "test_amxb_rbus_native_call.h"

static rbusHandle_t handle = NULL;
static pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int test_native_setup(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    cmocka_enable_check(true);

    assert_int_equal(rbus_checkStatus(), RBUS_ENABLED);

    printf("SETUP: Starting amxrt data model providers\n");
    system("amxrt ../common/odl/method_calls.odl &");
    printf("SETUP: Started ...\n");

    printf("SETUP: Connect to rbus\n");
    expect_string(__wrap_rbus_open, componentName, "rbus-unit-test");
    rc = rbus_open(&handle, "rbus-unit-test");
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("SETUP: Connected ...\n");

    return 0;
}

int test_native_teardown(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;

    printf("TEARDOWN: Stopping amxrt data model providers\n");
    system("killall amxrt");
    printf("TEARDOWN: Stopped ...\n");


    printf("TEARDOWN: Disconnect from rbus\n");
    rc = rbus_close(handle);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("TEARDOWN: Disconnected ...\n");

    return 0;
}

void test_native_rbus_call_method_synchronous(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    const char* method = "Device.TestObject.print_message()";
    rbusObject_t in_params = NULL;
    rbusObject_t out_params = NULL;
    rbusValue_t v = NULL;
    amxc_var_t result;

    amxc_var_init(&result);
    rbusObject_Init(&in_params, NULL);

    rbusValue_Init(&v);
    rbusValue_SetString(v, "Hello World");
    rbusObject_SetValue(in_params, "message", v);
    rbusValue_Release(v);

    // When calling an existing method with valid arguments
    printf("Calling method %s\n", method);
    fflush(stdout);
    rc = rbusMethod_Invoke(handle, method, in_params, &out_params);
    // the return code muse be RBUS_ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // the out_params must be set
    assert_non_null(out_params);

    // To check the content of the out params, convert it to an amxc_var_t
    // this is normally not done by a native rbus consumer.
    // but will make it easier in the test to check the content
    amxb_rbus_object_to_lvar(&result, out_params);
    amxc_var_dump(&result, STDOUT_FILENO);

    // an ambiorix data model provider will always return a list containing 3 items
    // 1. The return value (can be any type, see definition of the method)
    // 2. The out arguments, can be NULL or a table containing key-value pairs for each out argument
    // 3. The ambiorix data model engine return code
    assert_int_equal(amxc_var_type_of(&result), AMXC_VAR_ID_LIST);

    // the print_message returns the length of the message printed
    assert_int_equal(GETI_UINT32(&result, 0), strlen("Hello World"));
    // the print message has no out arguments
    assert_non_null((GETI_ARG(&result, 1))); // can be NULL variant or table variant
    assert_true(amxc_var_type_of(GETI_ARG(&result, 1)) == AMXC_VAR_ID_NULL ||
                amxc_var_type_of(GETI_ARG(&result, 1)) == AMXC_VAR_ID_HTABLE);
    // the libamxd return code is 0 when all went ok
    assert_non_null((GETI_ARG(&result, 2))); // is always 32-bit integer
    assert_int_equal(GETI_UINT32(&result, 2), 0);

    rbusObject_Release(in_params);
    rbusObject_Release(out_params);
    amxc_var_clean(&result);
}

void test_native_rbus_call_deferred_method_synchronous(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    const char* method = "Device.TestObject.print_message_deferred()";
    rbusObject_t in_params = NULL;
    rbusObject_t out_params = NULL;
    rbusValue_t v = NULL;
    amxc_var_t result;

    amxc_var_init(&result);
    rbusObject_Init(&in_params, NULL);

    rbusValue_Init(&v);
    rbusValue_SetString(v, "Hello World (deffered version)");
    rbusObject_SetValue(in_params, "message", v);
    rbusValue_Release(v);

    // When calling an existing method with valid arguments
    printf("Calling method %s\n", method);
    fflush(stdout);
    rc = rbusMethod_Invoke(handle, method, in_params, &out_params);
    // the return code muse be RBUS_ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // the out_params must be set
    assert_non_null(out_params);

    // To check the content of the out params, convert it to an amxc_var_t
    // this is normally not done by a native rbus consumer.
    // but will make it easier in the test to check the content
    amxb_rbus_object_to_lvar(&result, out_params);
    amxc_var_dump(&result, STDOUT_FILENO);

    // an ambiorix data model provider will always return a list containing 3 items
    // 1. The return value (can be any type, see definition of the method)
    // 2. The out arguments, can be NULL or a table containing key-value pairs for each out argument
    // 3. The ambiorix data model engine return code
    assert_int_equal(amxc_var_type_of(&result), AMXC_VAR_ID_LIST);

    // the print_message returns the length of the message printed
    assert_int_equal(GETI_UINT32(&result, 0), strlen("Hello World (deffered version)"));
    // the print message has no out arguments
    assert_non_null((GETI_ARG(&result, 1))); // can be NULL variant or table variant
    assert_true(amxc_var_type_of(GETI_ARG(&result, 1)) == AMXC_VAR_ID_NULL ||
                amxc_var_type_of(GETI_ARG(&result, 1)) == AMXC_VAR_ID_HTABLE);
    // the libamxd return code is 0 when all went ok
    assert_non_null((GETI_ARG(&result, 2))); // is always 32-bit integer
    assert_int_equal(GETI_UINT32(&result, 2), 0);

    rbusObject_Release(in_params);
    rbusObject_Release(out_params);
    amxc_var_clean(&result);
}

void test_native_rbus_call_method_synchronous_get_out_args(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    const char* method = "Device.TestObject.test_out_args()";
    rbusObject_t in_params = NULL;
    rbusObject_t out_params = NULL;
    amxc_var_t result;

    amxc_var_init(&result);
    rbusObject_Init(&in_params, NULL);

    // When calling an existing method without arguments
    printf("Calling method %s\n", method);
    fflush(stdout);
    rc = rbusMethod_Invoke(handle, method, in_params, &out_params);
    // the return code muse be RBUS_ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // the out_params must be set
    assert_non_null(out_params);

    // To check the content of the out params, convert it to an amxc_var_t
    // this is normally not done by a native rbus consumer.
    // but will make it easier in the test to check the content
    amxb_rbus_object_to_lvar(&result, out_params);
    amxc_var_dump(&result, STDOUT_FILENO);

    // the test_out_args returns 0 as return value
    assert_int_equal(GETI_UINT32(&result, 0), 0);
    // the test_out_args  has out arguments
    assert_non_null((GETI_ARG(&result, 1))); // must be a table variant
    assert_true(amxc_var_type_of(GETI_ARG(&result, 1)) == AMXC_VAR_ID_HTABLE);
    // the table must contain a message
    assert_non_null(GETP_ARG(&result, "1.message"));
    assert_string_equal(GETP_CHAR(&result, "1.message"), "Hello World");
    // the table must contain another table (object)
    assert_non_null(GETP_ARG(&result, "1.data"));
    assert_non_null(GETP_ARG(&result, "1.data.'HostObject.1.'"));
    assert_non_null(GETP_ARG(&result, "1.data.'HostObject.1.'.'EnvVariable.1.'"));
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.Destination"), "testDestination");
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.Options"), "Option1,Option2");
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.Source"), "testSource");
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.'EnvVariable.1.'.Key"), "testKey");
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.'EnvVariable.1.'.ModuleVersion"), "testVersion");
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.'EnvVariable.1.'.Value"), "testValue");

    rbusObject_Release(in_params);
    rbusObject_Release(out_params);
    amxc_var_clean(&result);
}

void test_native_rbus_call_deferred_method_synchronous_get_out_args(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    const char* method = "Device.TestObject.test_out_args_deferred()";
    rbusObject_t in_params = NULL;
    rbusObject_t out_params = NULL;
    amxc_var_t result;

    amxc_var_init(&result);
    rbusObject_Init(&in_params, NULL);

    // When calling an existing method without arguments
    printf("Calling method %s\n", method);
    fflush(stdout);
    rc = rbusMethod_Invoke(handle, method, in_params, &out_params);
    // the return code muse be RBUS_ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // the out_params must be set
    assert_non_null(out_params);

    // To check the content of the out params, convert it to an amxc_var_t
    // this is normally not done by a native rbus consumer.
    // but will make it easier in the test to check the content
    amxb_rbus_object_to_lvar(&result, out_params);
    amxc_var_dump(&result, STDOUT_FILENO);

    // the test_out_args returns 0 as return value
    assert_int_equal(GETI_UINT32(&result, 0), 0);
    // the test_out_args  has out arguments
    assert_non_null((GETI_ARG(&result, 1))); // must be a table variant
    assert_true(amxc_var_type_of(GETI_ARG(&result, 1)) == AMXC_VAR_ID_HTABLE);
    // the table must contain a message
    assert_non_null(GETP_ARG(&result, "1.message"));
    assert_string_equal(GETP_CHAR(&result, "1.message"), "Hello World");
    // the table must contain another table (object)
    assert_non_null(GETP_ARG(&result, "1.data"));
    assert_non_null(GETP_ARG(&result, "1.data.'HostObject.1.'"));
    assert_non_null(GETP_ARG(&result, "1.data.'HostObject.1.'.'EnvVariable.1.'"));
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.Destination"), "testDestination");
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.Options"), "Option1,Option2");
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.Source"), "testSource");
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.'EnvVariable.1.'.Key"), "testKey");
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.'EnvVariable.1.'.ModuleVersion"), "testVersion");
    assert_string_equal(GETP_CHAR(&result, "1.data.'HostObject.1.'.'EnvVariable.1.'.Value"), "testValue");

    rbusObject_Release(in_params);
    rbusObject_Release(out_params);
    amxc_var_clean(&result);
}

static void async_print_message_done_callback(UNUSED rbusHandle_t handle,
                                              UNUSED char const* methodName,
                                              rbusError_t error,
                                              rbusObject_t params) {
    amxc_var_t result;
    amxc_var_init(&result);

    // the return code muse be RBUS_ERROR_SUCCESS
    assert_int_equal(error, RBUS_ERROR_SUCCESS);

    // the out_params must be set
    assert_non_null(params);

    // To check the content of the out params, convert it to an amxc_var_t
    // this is normally not done by a native rbus consumer.
    // but will make it easier in the test to check the content
    amxb_rbus_object_to_lvar(&result, params);
    amxc_var_dump(&result, STDOUT_FILENO);

    // an ambiorix data model provider will always return a list containing 3 items
    // 1. The return value (can be any type, see definition of the method)
    // 2. The out arguments, can be NULL or a table containing key-value pairs for each out argument
    // 3. The ambiorix data model engine return code
    assert_int_equal(amxc_var_type_of(&result), AMXC_VAR_ID_LIST);

    // the print_message returns the length of the message printed
    assert_int_equal(GETI_UINT32(&result, 0), strlen("Hello World"));
    // the print message has no out arguments
    assert_non_null((GETI_ARG(&result, 1))); // can be NULL variant or table variant
    assert_true(amxc_var_type_of(GETI_ARG(&result, 1)) == AMXC_VAR_ID_NULL ||
                amxc_var_type_of(GETI_ARG(&result, 1)) == AMXC_VAR_ID_HTABLE);
    // the libamxd return code is 0 when all went ok
    assert_non_null((GETI_ARG(&result, 2))); // is always 32-bit integer
    assert_int_equal(GETI_UINT32(&result, 2), 0);

    pthread_cond_signal(&condition);
    amxc_var_clean(&result);
}

void test_native_rbus_call_method_asynchronous(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    const char* method = "Device.TestObject.print_message()";
    rbusObject_t in_params = NULL;
    rbusValue_t v = NULL;

    rbusObject_Init(&in_params, NULL);

    rbusValue_Init(&v);
    rbusValue_SetString(v, "Hello World");
    rbusObject_SetValue(in_params, "message", v);
    rbusValue_Release(v);

    // When calling an existing method with valid arguments
    printf("Calling method %s\n", method);
    fflush(stdout);
    rc = rbusMethod_InvokeAsync(handle, method, in_params, async_print_message_done_callback, 0);
    // the return code muse be RBUS_ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);

    rbusObject_Release(in_params);

    // rbus will call the callback on a different thread
    // just wait until the condition is triggered.
    pthread_cond_wait(&condition, &lock);
}

void test_native_rbus_call_deferred_method_asynchronous(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    const char* method = "Device.TestObject.print_message_deferred()";
    rbusObject_t in_params = NULL;
    rbusValue_t v = NULL;

    rbusObject_Init(&in_params, NULL);

    rbusValue_Init(&v);
    rbusValue_SetString(v, "Hello World");
    rbusObject_SetValue(in_params, "message", v);
    rbusValue_Release(v);

    // When calling an existing method with valid arguments
    printf("Calling method %s\n", method);
    fflush(stdout);
    rc = rbusMethod_InvokeAsync(handle, method, in_params, async_print_message_done_callback, 0);
    // the return code muse be RBUS_ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);

    rbusObject_Release(in_params);

    // rbus will call the callback on a different thread
    // just wait until the condition is triggered.
    pthread_cond_wait(&condition, &lock);
}

void test_native_rbus_call_start_timer(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    const char* method = "Device.TestObject.test_start_timer()";
    rbusObject_t in_params = NULL;
    rbusObject_t out_params = NULL;
    rbusValue_t v = NULL;
    uint32_t uint_val = 0;

    rbusObject_Init(&in_params, NULL);

    rbusValue_Init(&v);
    rbusValue_SetUInt32(v, 200);
    rbusObject_SetValue(in_params, "ms_interval", v);
    rbusValue_Release(v);

    // When calling an existing method with valid arguments
    printf("Calling method %s\n", method);
    fflush(stdout);
    rc = rbusMethod_Invoke(handle, method, in_params, &out_params);
    // the return code muse be RBUS_ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // the out_params must be set
    assert_non_null(out_params);
    rbusObject_Release(in_params);
    rbusObject_Release(out_params);

    sleep(1);

    rbus_getUint(handle, "Device.TestObject.Counter", &uint_val);
    printf("Counter = %d\n", uint_val);
    fflush(stdout);
    assert_true(uint_val >= 4 && uint_val <= 6);
}

void test_native_rbus_reset_counter(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    const char* method = "Device.TestObject.test_reset_counter()";
    rbusObject_t in_params = NULL;
    rbusObject_t out_params = NULL;
    uint32_t uint_val = 0;

    for(int i = 0; i < 100; i++) {
        usleep(10000);
        rbusObject_Init(&in_params, NULL);
        // When calling an existing method with valid arguments
        printf("Calling method %s\n", method);
        fflush(stdout);
        rc = rbusMethod_Invoke(handle, method, in_params, &out_params);
        // the return code muse be RBUS_ERROR_SUCCESS
        assert_int_equal(rc, RBUS_ERROR_SUCCESS);
        // the out_params must be set
        assert_non_null(out_params);
        rbusObject_Release(in_params);
        rbusObject_Release(out_params);
    }

    rbus_getUint(handle, "Device.TestObject.Counter", &uint_val);
    printf("Counter = %d\n", uint_val);
    fflush(stdout);
    assert_true(uint_val == 0 || uint_val == 1);

    sleep(1);

    rbus_getUint(handle, "Device.TestObject.Counter", &uint_val);
    printf("Counter = %d\n", uint_val);
    fflush(stdout);
    assert_true(uint_val >= 4 && uint_val <= 6);
}
