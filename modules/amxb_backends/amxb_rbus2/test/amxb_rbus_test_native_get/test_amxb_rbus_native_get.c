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

#include "../common/include/amxb_rbus_test.h"
#include "test_amxb_rbus_native_get.h"

rbusHandle_t handle = NULL;

int test_native_setup(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    cmocka_enable_check(true); // in the unit test

    assert_int_equal(rbus_checkStatus(), RBUS_ENABLED);

    printf("SETUP: Starting amxrt data model provider\n");
    system("amxrt -D ../common/odl/softwaremodules.odl");
    printf("SETUP: Started ...\n");

    sleep(1);

    printf("SETUP: Connect to rbus\n");

    expect_string(__wrap_rbus_open, componentName, "rbus-unit-test");
    rc = rbus_open(&handle, "rbus-unit-test");
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("SETUP: Connected ...\n");

    return 0;
}

int test_native_translate_setup(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    cmocka_enable_check(true); // in the unit test

    assert_int_equal(rbus_checkStatus(), RBUS_ENABLED);

    printf("SETUP: Starting amxrt data model provider\n");
    system("amxrt -D ../common/odl/softwaremodules_translated.odl");
    printf("SETUP: Started ...\n");

    sleep(1);

    printf("SETUP: Connect to rbus\n");

    expect_string(__wrap_rbus_open, componentName, "rbus-unit-test");
    rc = rbus_open(&handle, "rbus-unit-test");
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("SETUP: Connected ...\n");

    return 0;
}

int test_native_teardown(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;

    printf("TEARDOWN: Stopping amxrt data model provider\n");
    system("killall amxrt");
    printf("TEARDOWN: Stopped ...\n");


    printf("TEARDOWN: Disconnect from rbus\n");
    rc = rbus_close(handle);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("TEARDOWN: Disconnected ...\n");

    return 0;
}

void test_native_rbus_get_string_value(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    char* str_val = NULL;
    rbusValue_t value = NULL;

    // When calling rbus_get on a string parameter that exists
    rc = rbus_get(handle, "Device.SoftwareModules.ExecEnvClass.1.Capability.1.Specification", &value);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // The type must be a string
    assert_int_equal(rbusValue_GetType(value), RBUS_STRING);
    // The value should be the current value of the parameter
    assert_string_equal(rbusValue_GetString(value, NULL), "TestSpecification");
    rbusValue_Release(value);

    // When calling rbus_getStr on a string parameter that exists
    rc = rbus_getStr(handle, "Device.SoftwareModules.ExecEnvClass.1.Capability.1.Specification", &str_val);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // The value should be the current value of the parameter
    assert_string_equal(str_val, "TestSpecification");
    free(str_val);
    str_val = NULL;

    // When calling rbus_getStr on a parameter that exists and is not a string must fail
    rc = rbus_getStr(handle, "Device.SoftwareModules.ExecEnv.2.Enable", &str_val);
    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
    // The str_val pointer should still be NULL
    assert_null(str_val);
}

void test_native_rbus_get_bool_value(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    rbusValue_t value = NULL;
    bool bool_val = false;

    // When calling rbus_get on a boolean parameter that exists
    rc = rbus_get(handle, "Device.SoftwareModules.ExecEnv.1.Enable", &value);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // The type must be a boolean
    assert_int_equal(rbusValue_GetType(value), RBUS_BOOLEAN);
    // The value should be the current value of the parameter
    assert_true(rbusValue_GetBoolean(value));
    rbusValue_Release(value);

    // When calling rbus_get on a boolean parameter that exists
    rc = rbus_get(handle, "Device.SoftwareModules.ExecEnv.2.Enable", &value);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // The type must be a boolean
    assert_int_equal(rbusValue_GetType(value), RBUS_BOOLEAN);
    // The value should be the current value of the parameter
    assert_false(rbusValue_GetBoolean(value));
    rbusValue_Release(value);

    // When calling rbus_getBoolean on a string parameter that exists
    rc = rbus_getBoolean(handle, "Device.SoftwareModules.ExecEnv.1.Enable", &bool_val);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // The value should be the current value of the parameter
    assert_true(bool_val);

    // When calling rbus_getBoolean on a string parameter that exists
    rc = rbus_getBoolean(handle, "Device.SoftwareModules.ExecEnv.2.Enable", &bool_val);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // The value should be the current value of the parameter
    assert_false(bool_val);

    // When calling rbus_getBoolean on a parameter that exists and is not a boolean must fail
    rc = rbus_getBoolean(handle, "Device.SoftwareModules.ExecEnv.1.Name", &bool_val);
    // The return value must not be RBUS_ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_get_int_value(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    int32_t int_val = 0;
    rbusValue_t value = NULL;

    // When calling rbus_get on an integer parameter that exists
    rc = rbus_get(handle, "Device.SoftwareModules.ExecEnv.1.InitialExecutionUnitRunLevel", &value);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // The type must be a integer
    assert_int_equal(rbusValue_GetType(value), RBUS_INT32);
    // The value should be the current value of the parameter
    assert_int_equal(rbusValue_GetInt32(value), 100);
    rbusValue_Release(value);

    // When calling rbus_getInt on an integer parameter that exists
    rc = rbus_getInt(handle, "Device.SoftwareModules.ExecEnv.1.InitialExecutionUnitRunLevel", &int_val);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // The value should be the current value of the parameter
    assert_int_equal(int_val, 100);

    // When calling rbus_getInt on a parameter that exists and is not an integer must fail
    rc = rbus_getInt(handle, "Device.SoftwareModules.ExecEnv.1.InitialRunLevel", &int_val);
    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_get_uint_value(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    uint32_t uint_val = 0;
    rbusValue_t value = NULL;

    // When calling rbus_get on an unsigned integer parameter that exists
    rc = rbus_get(handle, "Device.SoftwareModules.ExecEnv.1.InitialRunLevel", &value);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // The type must be a unsigned integer
    assert_int_equal(rbusValue_GetType(value), RBUS_UINT32);
    // The value should be the current value of the parameter
    assert_int_equal(rbusValue_GetUInt32(value), 10);
    rbusValue_Release(value);

    // When calling rbus_getInt on an integer parameter that exists
    rc = rbus_getUint(handle, "Device.SoftwareModules.ExecEnv.1.InitialRunLevel", &uint_val);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // The value should be the current value of the parameter
    assert_int_equal(uint_val, 10);

    // When calling rbus_getInt on a parameter that exists and is not an unsigned integer must fail
    rc = rbus_getUint(handle, "Device.SoftwareModules.ExecEnv.1.InitialExecutionUnitRunLevel", &uint_val);
    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_getext_partial_path(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    int num_vals = 0;
    rbusProperty_t output_vals = NULL;
    rbusProperty_t next = NULL;
    const char* partial_path[1] = {"Device.SoftwareModules.ExecEnvClass.1."};
    expected_type_t expected[] = {
        { "Device.SoftwareModules.ExecEnvClass.1.CapabilityNumberOfEntries", RBUS_UINT32, 0 },
        { "Device.SoftwareModules.ExecEnvClass.1.Name", RBUS_STRING, 0 },
        { "Device.SoftwareModules.ExecEnvClass.1.Version", RBUS_STRING, 0 },
        { "Device.SoftwareModules.ExecEnvClass.1.Alias", RBUS_STRING, 0 },
        { "Device.SoftwareModules.ExecEnvClass.1.DeploymentUnitRef", RBUS_STRING, 0 },
        { "Device.SoftwareModules.ExecEnvClass.1.Vendor", RBUS_STRING, 0 },
        { "Device.SoftwareModules.ExecEnvClass.1.Capability.1.SpecificationVersion", RBUS_STRING, 0 },
        { "Device.SoftwareModules.ExecEnvClass.1.Capability.1.Specification", RBUS_STRING, 0 },
        { "Device.SoftwareModules.ExecEnvClass.1.Capability.1.SpecificationURI", RBUS_STRING, 0 },
    };

    // When calling rbus_getex on a partial path that exists
    rc = rbus_getExt(handle, 1, partial_path, &num_vals, &output_vals);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // All available parameters under the partial path must be returned
    assert_int_equal(num_vals, 9);
    next = output_vals;
    for(int i = 0; i < num_vals; i++) {
        rbusValue_t val = rbusProperty_GetValue(next);
        const char* name = rbusProperty_GetName(next);
        // the names and types must match the expected
        assert_string_equal(name, expected[i].name);
        assert_int_equal(rbusValue_GetType(val), expected[i].type);
        next = rbusProperty_GetNext(next);
    }
    rbusProperty_Release(output_vals);
}