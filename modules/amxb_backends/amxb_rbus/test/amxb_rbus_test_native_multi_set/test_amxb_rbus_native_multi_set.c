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
#include "test_amxb_rbus_native_multi_set.h"

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

void test_native_rbus_multi_set_using_setMulti_on_one_provider_same_object(UNUSED void** state) {
    rbusProperty_t properties = NULL;
    rbusProperty_t next = NULL;
    rbusValue_t rbus_value = NULL;
    int retval = -1;

    // Build properties to set
    rbusValue_Init(&rbus_value);
    rbusValue_SetString(rbus_value, "TestText");
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.L2AText", rbus_value);
    rbusValue_Release(rbus_value);
    properties = next;
    rbusValue_Init(&rbus_value);
    rbusValue_SetInt8(rbus_value, -100);
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.L2ASignedNumber", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);
    rbusValue_Init(&rbus_value);
    rbusValue_SetInt8(rbus_value, 100);
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.L2AUnsignedNumber", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);

    // When setting multiple parameters at once with rbus_setMulti of the same object
    // and all parameters contain valid values
    retval = rbus_setMulti(handle, 3, properties, NULL);
    rbusProperty_Release(properties);

    // the return value should be RBUS_ERROR_SUCCESS
    assert_int_equal(retval, RBUS_ERROR_SUCCESS);

    // and the values must be applied
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.L2AText", "TestText"), 0);
    assert_int_equal(check_rbus_value_int8(handle, "TopLevel.Level1.1.Level2A.L2ASignedNumber", -100), 0);
    assert_int_equal(check_rbus_value_uint8(handle, "TopLevel.Level1.1.Level2A.L2AUnsignedNumber", 100), 0);
}

void test_native_rbus_multi_set_using_setMulti_on_one_provider_same_object_invalid_value(UNUSED void** state) {
    rbusProperty_t properties = NULL;
    rbusProperty_t next = NULL;
    rbusValue_t rbus_value = NULL;
    int retval = -1;

    // Build properties to set
    rbusValue_Init(&rbus_value);
    rbusValue_SetString(rbus_value, "TestText2");
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.L2AText", rbus_value);
    rbusValue_Release(rbus_value);
    properties = next;
    rbusValue_Init(&rbus_value);
    rbusValue_SetInt8(rbus_value, -105); // This is an invalid value
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.L2ASignedNumber", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);
    rbusValue_Init(&rbus_value);
    rbusValue_SetInt8(rbus_value, 105);
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.L2AUnsignedNumber", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);

    // When setting multiple parameters at once with rbus_setMulti of the same object
    // and one of the parameters contain an invalid value
    retval = rbus_setMulti(handle, 3, properties, NULL);
    rbusProperty_Release(properties);

    // the return value should not be RBUS_ERROR_SUCCESS
    assert_int_not_equal(retval, RBUS_ERROR_SUCCESS);

    // and the values must not be applied - they should stll be the same as in the previous test
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.L2AText", "TestText"), 0);
    assert_int_equal(check_rbus_value_int8(handle, "TopLevel.Level1.1.Level2A.L2ASignedNumber", -100), 0);
    assert_int_equal(check_rbus_value_uint8(handle, "TopLevel.Level1.1.Level2A.L2AUnsignedNumber", 100), 0);
}

void test_native_rbus_multi_set_using_setMulti_on_one_provider_different_objects(UNUSED void** state) {
    rbusProperty_t properties = NULL;
    rbusProperty_t next = NULL;
    rbusValue_t rbus_value = NULL;
    int retval = -1;

    // Build properties to set
    rbusValue_Init(&rbus_value);
    rbusValue_SetString(rbus_value, "TestValue");
    rbusProperty_Init(&next, "TopLevel.TLText", rbus_value);
    rbusValue_Release(rbus_value);
    properties = next;
    rbusValue_Init(&rbus_value);
    rbusValue_SetString(rbus_value, "TestValue");
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);
    rbusValue_Init(&rbus_value);
    rbusValue_SetString(rbus_value, "TestValue");
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2B.1.L2BText", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);

    // When setting multiple parameters at once with rbus_setMulti on different objects
    // and all parameters contain valid values
    retval = rbus_setMulti(handle, 3, properties, NULL);
    rbusProperty_Release(properties);

    // the return value should be RBUS_ERROR_SUCCESS
    assert_int_equal(retval, RBUS_ERROR_SUCCESS);

    // and the values must be applied
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.TLText", "TestValue"), 0);
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", "TestValue"), 0);
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2B.1.L2BText", "TestValue"), 0);
}

void test_native_rbus_multi_set_using_setMulti_on_one_provider_different_objects_invalid_value(UNUSED void** state) {
    rbusProperty_t properties = NULL;
    rbusProperty_t next = NULL;
    rbusValue_t rbus_value = NULL;
    int retval = -1;

    // Build properties to set
    rbusValue_Init(&rbus_value);
    rbusValue_SetString(rbus_value, "TestValue2");
    rbusProperty_Init(&next, "TopLevel.TLText", rbus_value);
    rbusValue_Release(rbus_value);
    properties = next;
    rbusValue_Init(&rbus_value);
    rbusValue_SetInt8(rbus_value, -105);
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.L2ASignedNumber", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);
    rbusValue_Init(&rbus_value);
    rbusValue_SetString(rbus_value, "TestValue2");
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2B.1.L2BText", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);

    // When setting multiple parameters at once with rbus_setMulti on different objects
    // and one parameter contains an invalid value
    retval = rbus_setMulti(handle, 3, properties, NULL);
    rbusProperty_Release(properties);

    // the return value should not be RBUS_ERROR_SUCCESS
    assert_int_not_equal(retval, RBUS_ERROR_SUCCESS);

    // and the values must be applied
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.TLText", "TestValue"), 0);
    assert_int_equal(check_rbus_value_int8(handle, "TopLevel.Level1.1.Level2A.L2ASignedNumber", -100), 0);
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2B.1.L2BText", "TestValue"), 0);
}

void test_native_rbus_multi_set_using_setMulti_on_more_providers(UNUSED void** state) {
    rbusProperty_t properties = NULL;
    rbusProperty_t next = NULL;
    rbusValue_t rbus_value = NULL;

    int retval = -1;

    // Build properties to set
    rbusValue_Init(&rbus_value);
    rbusValue_SetString(rbus_value, "TestText3");
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.L2AText", rbus_value);
    rbusValue_Release(rbus_value);
    properties = next;
    rbusValue_Init(&rbus_value);
    rbusValue_SetInt8(rbus_value, -99);
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.L2ASignedNumber", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);
    rbusValue_Init(&rbus_value);
    rbusValue_SetString(rbus_value, "TestText3");
    rbusProperty_Init(&next, "Device.SoftwareModules.ExecEnvClass.1.Vendor", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);

    // When setting multiple parameters at once with rbus_setMulti of the same object
    // and all parameters contain valid values
    retval = rbus_setMulti(handle, 3, properties, NULL);
    rbusProperty_Release(properties);

    // the return value should be RBUS_ERROR_SUCCESS
    assert_int_equal(retval, RBUS_ERROR_SUCCESS);

    // and the values must be applied
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.L2AText", "TestText3"), 0);
    assert_int_equal(check_rbus_value_int8(handle, "TopLevel.Level1.1.Level2A.L2ASignedNumber", -99), 0);
    assert_int_equal(check_rbus_value_str(handle, "Device.SoftwareModules.ExecEnvClass.1.Vendor", "TestText3"), 0);
}

void test_native_rbus_multi_set_using_setMulti_on_more_providers_invalid_value(UNUSED void** state) {
    rbusProperty_t properties = NULL;
    rbusProperty_t next = NULL;
    rbusValue_t rbus_value = NULL;
    int retval = -1;

    // Build properties to set
    rbusValue_Init(&rbus_value);
    rbusValue_SetString(rbus_value, "TestText4");
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.L2AText", rbus_value);
    rbusValue_Release(rbus_value);
    properties = next;
    rbusValue_Init(&rbus_value);
    rbusValue_SetString(rbus_value, "TestText4");
    rbusProperty_Init(&next, "Device.SoftwareModules.ExecEnvClass.1.Vendor", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);
    rbusValue_Init(&rbus_value);
    rbusValue_SetInt8(rbus_value, -105);
    rbusProperty_Init(&next, "TopLevel.Level1.1.Level2A.L2ASignedNumber", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);
    rbusValue_Init(&rbus_value);
    rbusValue_SetBoolean(rbus_value, false);
    rbusProperty_Init(&next, "Device.SoftwareModules.ExecEnv.1.Enable", rbus_value);
    rbusProperty_Append(properties, next);
    rbusProperty_Release(next);
    rbusValue_Release(rbus_value);

    // When setting multiple parameters at once with rbus_setMulti of the same object
    // and one parameter contain an invalid value
    retval = rbus_setMulti(handle, 4, properties, NULL);
    rbusProperty_Release(properties);

    // the expected return value should not be RBUS_ERROR_SUCCESS
    // but it seems that rbus is not returning an error in this case
    // As one of the providers are accepting all values and can apply them,
    // the return seems to be RBUS_ERROR_SUCCESS
    printf("Expected failure or at least some info about which parameters failed\n");
    printf("rbus_setMulti returned %d\n", retval);
    printf("Following parameter is not set due to invalid value: TopLevel.Level1.1.Level2A.L2ASignedNumber");
    assert_int_equal(retval, RBUS_ERROR_SUCCESS);

    // and the values must not be applied - same as previous test for the provider
    // for which one of the values are invalid
    // for all other providers the values are changed
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.L2AText", "TestText3"), 0);
    assert_int_equal(check_rbus_value_int8(handle, "TopLevel.Level1.1.Level2A.L2ASignedNumber", -99), 0);
    assert_int_equal(check_rbus_value_str(handle, "Device.SoftwareModules.ExecEnvClass.1.Vendor", "TestText4"), 0);
    assert_int_equal(check_rbus_value_bool(handle, "Device.SoftwareModules.ExecEnv.1.Enable", false), 0);

    // NOTE:
    // If rbus_setMulti is done on elements provide by different providers,
    // the set can be partially applied. Is this the expected behaviour?
    // In the USP specification a partial set can be done, but must be indicate that
    // a partial set is wanted, by default a set on mutiple elements are expected
    // to work as an atomic operation (all succeed or nothing has been changed when failed).
    // In the case of rbus it is possible that it partially fails but the return value
    // is not indicating an error. So basically there is no guarantee that when
    // setting values and the return value of rbus_setMulti is RBUS_ERROR_SUCCESS
    // that all values are set. The only way to check that is to read them back.
}

void test_native_rbus_multiple_single_sets_with_commit(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    rbusValue_t value = NULL;
    rbusSetOptions_t opts = { false, 0};

    rbusValue_Init(&value);
    assert_true(rbusValue_SetFromString(value, RBUS_STRING, "TestText5"));
    // When calling rbus_set on a string parameter that exists with commit option false
    rc = rbus_set(handle, "TopLevel.Level1.1.Level2A.L2AText", value, &opts);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must not be set
    assert_int_not_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.L2AText", "TestText5"), 0);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    assert_true(rbusValue_SetFromString(value, RBUS_STRING, "TestText5"));
    // When calling rbus_set on a string parameter that exists with commit option false
    rc = rbus_set(handle, "TopLevel.TLText", value, &opts);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must not be set
    assert_int_not_equal(check_rbus_value_str(handle, "TopLevel.TLText", "TestText5"), 0);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    assert_true(rbusValue_SetFromString(value, RBUS_STRING, "TestText5"));
    // When calling rbus_set on a string parameter that exists with commit option true
    opts.commit = true;
    rc = rbus_set(handle, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", value, &opts);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And all the value must  be set
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.L2AText", "TestText5"), 0);
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.TLText", "TestText5"), 0);
    assert_int_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", "TestText5"), 0);
    rbusValue_Release(value);
}

void test_native_rbus_multiple_single_sets_without_commit(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    rbusValue_t value = NULL;
    rbusSetOptions_t opts = { false, 0};

    rbusValue_Init(&value);
    assert_true(rbusValue_SetFromString(value, RBUS_STRING, "TestText6"));
    // When calling rbus_set on a string parameter that exists with commit option false
    rc = rbus_set(handle, "TopLevel.Level1.1.Level2A.L2AText", value, &opts);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must not be set
    assert_int_not_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.L2AText", "TestText6"), 0);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    assert_true(rbusValue_SetFromString(value, RBUS_STRING, "TestText6"));
    // When calling rbus_set on a string parameter that exists with commit option false
    rc = rbus_set(handle, "TopLevel.TLText", value, &opts);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must not be set
    assert_int_not_equal(check_rbus_value_str(handle, "TopLevel.TLText", "TestText6"), 0);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    assert_true(rbusValue_SetFromString(value, RBUS_STRING, "TestText6"));
    // When calling rbus_set on a string parameter that exists with commit option false
    rc = rbus_set(handle, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", value, &opts);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // And the value must not be set
    assert_int_not_equal(check_rbus_value_str(handle, "TopLevel.Level1.1.Level2A.Level3.1.L3Text", "TestText6"), 0);
    rbusValue_Release(value);

    // NOTE: the open session is kept until a set is done with the commit set to true.
    // Stopping now will call the clean sessions when the amxb-rbus backend is unloaded.
}
