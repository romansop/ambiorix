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
#include "test_amxb_rbus_native_single_set.h"

rbusHandle_t handle = NULL;

int test_native_setup(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    cmocka_enable_check(true);

    assert_int_equal(rbus_checkStatus(), RBUS_ENABLED);

    printf("SETUP: Starting amxrt data model providers\n");
    system("amxrt -D ../common/odl/softwaremodules.odl");
    system("amxrt -D ../common/odl/registration_test.odl");
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

int test_native_translate_setup(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    cmocka_enable_check(true);

    assert_int_equal(rbus_checkStatus(), RBUS_ENABLED);

    printf("SETUP: Starting amxrt data model providers\n");
    system("amxrt -D ../common/odl/softwaremodules_translated.odl");
    system("amxrt -D ../common/odl/registration_test.odl");
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

void test_native_rbus_single_set_string_value(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    rbusValue_t value = NULL;

    rbusValue_Init(&value);
    assert_true(rbusValue_SetFromString(value, RBUS_STRING, "ChangedVendor"));

    // When calling rbus_set on a string parameter that exists
    rc = rbus_set(handle, "Device.SoftwareModules.ExecEnvClass.1.Vendor", value, NULL);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must be set
    assert_int_equal(check_rbus_value_str(handle, "Device.SoftwareModules.ExecEnvClass.1.Vendor", "ChangedVendor"), 0);
    rbusValue_Release(value);

    // When calling rbus_setStr on a string parameter that exists
    rc = rbus_setStr(handle, "Device.SoftwareModules.ExecEnvClass.1.Vendor", "AnotherValue");
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must be set
    assert_int_equal(check_rbus_value_str(handle, "Device.SoftwareModules.ExecEnvClass.1.Vendor", "AnotherValue"), 0);

    // When calling rbus_setStr on a none string parameter that exists and the value can be converted to the correct type
    rc = rbus_setStr(handle, "Device.SoftwareModules.ExecEnv.1.InitialRunLevel", "99");
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // and the value must be set
    assert_int_equal(check_rbus_value_uint32(handle, "Device.SoftwareModules.ExecEnv.1.InitialRunLevel", 99), 0);

    // When calling rbus_setStr on a none string parameter that exists and the value can not be converted
    rc = rbus_setStr(handle, "Device.SoftwareModules.ExecEnv.1.InitialRunLevel", "AnotherValue");
    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);

    // When calling rbus_setStr on a not existing parameter
    rc = rbus_setStr(handle, "Device.SoftwareModules.ExecEnv.1.NotExisting", "SomeValue");
    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);

    // When calling rbus_setStr on a non-mutable key parameter
    rc = rbus_setStr(handle, "Device.SoftwareModules.ExecEnv.1.Alias", "ChangedAlias");
    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);

    // When calling rbus_setStr on a read-only parameter
    rc = rbus_setStr(handle, "Device.SoftwareModules.ExecEnvClass.1.Name", "ChangedName");
    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_single_set_bool_value(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    rbusValue_t value = NULL;

    rbusValue_Init(&value);
    assert_true(rbusValue_SetFromString(value, RBUS_BOOLEAN, "true"));

    // When calling rbus_set on a bool parameter that exists
    rc = rbus_set(handle, "TopLevel.Level1.1.Level2B.1.L2BBool", value, NULL);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must be set
    assert_int_equal(check_rbus_value_bool(handle, "TopLevel.Level1.1.Level2B.1.L2BBool", true), 0);
    rbusValue_Release(value);

    // When calling rbus_setBoolean on a bool parameter that exists
    rc = rbus_setBoolean(handle, "TopLevel.Level1.1.Level2B.1.L2BBool", false);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must be set
    assert_int_equal(check_rbus_value_bool(handle, "TopLevel.Level1.1.Level2B.1.L2BBool", false), 0);

    // When calling rbus_setint on a none int32 parameter that exists and the value can be converted to the correct type
    rc = rbus_setBoolean(handle, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", true);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must be set
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", "true"), 0);

    // When calling rbus_setint on a not existing parameter
    rc = rbus_setBoolean(handle, "Device.SoftwareModules.ExecEnv.1.NotExisting", true);
    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_single_set_int_value(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    rbusValue_t value = NULL;

    rbusValue_Init(&value);
    assert_true(rbusValue_SetFromString(value, RBUS_INT32, "123"));

    // When calling rbus_set on a int32 parameter that exists
    rc = rbus_set(handle, "TopLevel.TLSignedNumber", value, NULL);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must be set
    assert_int_equal(check_rbus_value_int32(handle, "TopLevel.TLSignedNumber", 123), 0);
    rbusValue_Release(value);

    // When calling rbus_setUint on a int32 parameter that exists
    rc = rbus_setInt(handle, "TopLevel.TLSignedNumber", -666);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must be set
    assert_int_equal(check_rbus_value_int32(handle, "TopLevel.TLSignedNumber", -666), 0);

    // When calling rbus_setint on a none int32 parameter that exists and the value can be converted to the correct type
    rc = rbus_setInt(handle, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", -99);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must be set
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", "-99"), 0);

    // When calling rbus_setint on a not existing parameter
    rc = rbus_setInt(handle, "Device.SoftwareModules.ExecEnv.1.NotExisting", -123);
    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_single_set_uint_value(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    rbusValue_t value = NULL;

    rbusValue_Init(&value);
    assert_true(rbusValue_SetFromString(value, RBUS_UINT32, "123"));

    // When calling rbus_set on a uint32 parameter that exists
    rc = rbus_set(handle, "TopLevel.TLUnsignedNumber", value, NULL);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must be set
    assert_int_equal(check_rbus_value_uint32(handle, "TopLevel.TLUnsignedNumber", 123), 0);
    rbusValue_Release(value);

    // When calling rbus_setUint on a uint32 parameter that exists
    rc = rbus_setUInt(handle, "TopLevel.TLUnsignedNumber", 666);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must be set
    assert_int_equal(check_rbus_value_uint32(handle, "TopLevel.TLUnsignedNumber", 666), 0);

    // When calling rbus_setUint on a none uint32 parameter that exists and the value can be converted to the correct type
    rc = rbus_setUInt(handle, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", 1025);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must be set
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", "1025"), 0);

    // When calling rbus_setUInt on a not existing parameter
    rc = rbus_setUInt(handle, "Device.SoftwareModules.ExecEnv.1.NotExisting", 123);
    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
}
