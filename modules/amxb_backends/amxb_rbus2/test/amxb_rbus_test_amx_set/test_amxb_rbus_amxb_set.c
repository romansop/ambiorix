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
#include "test_amxb_rbus_amxb_set.h"

static amxb_bus_ctx_t* bus_ctx = NULL;
static amxc_var_t config;

static int test_amx_get(const char* path, int32_t depth, const char* data_file) {
    int retval = 0;
    amxc_var_t data;
    amxc_var_init(&data);

    retval = amxb_get(bus_ctx, path, depth, &data, 10);
    if(retval == 0) {
        retval = check_amx_response(&data, data_file);
    } else {
        printf("amxb_get [%s] returned %d\n", path, retval);
    } retval = amxb_get(bus_ctx, path, depth, &data, 10);

    amxc_var_clean(&data);
    return retval;
}

static int test_amx_set(const char* path, amxc_var_t* parameters, const char* data_file) {
    int retval = 0;
    amxc_var_t data;
    amxc_var_init(&data);

    retval = amxb_set(bus_ctx, path, parameters, &data, 10);
    if(retval == 0) {
        retval = check_amx_response(&data, data_file);
    } else {
        printf("amxb_set [%s] returned %d\n", path, retval);
    }

    amxc_var_clean(&data);
    return retval;
}

static int test_amx_set_multi(const char* input_file, const char* verify_file, uint32_t flags) {
    int retval = 0;
    amxc_var_t ret;
    amxc_var_t* input = NULL;

    amxc_var_init(&ret);

    input = amxut_util_read_json_from_file(input_file);
    retval = amxb_set_multiple(bus_ctx, flags, input, &ret, 5);
    if(retval == 0) {
        retval = check_amx_response(&ret, verify_file);
    } else {
        printf("amxb_set_multiple returned %d\n", retval);
    }

    amxc_var_clean(&ret);
    amxc_var_delete(&input);
    return retval;
}

int test_amx_setup(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_init(&config);
    amxc_var_set_type(&config, AMXC_VAR_ID_HTABLE);

    assert_int_equal(amxb_be_load("../mod-amxb-test-rbus.so"), 0);
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    amxc_var_add_key(bool, rbus_config, "use-amx-calls", true);
    assert_int_equal(amxb_set_config(&config), 0);

    printf("SETUP: Starting amxrt data model providers\n");
    // Uses translation feature
    // Has support for amx-calls
    system("amxrt -D ../common/odl/softwaremodules_translated.odl");
    // Doesn't use translation feature
    // Has support for amx-calls
    system("amxrt -D ../common/odl/registration_test_amx_calls.odl");
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

void test_amxb_rbus_amxb_set_single_parameter(UNUSED void** state) {
    amxc_var_t params;

    amxc_var_init(&params);

    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "L1Text", "NewText");
    assert_int_equal(test_amx_set("TopLevel.Level1.1.", &params, "./verify_data/amxb_set_single_parameter_1.json"), 0);
    assert_int_equal(test_amx_get("TopLevel.Level1.1.L1Text", 0, "./verify_data/amxb_set_single_parameter_1.json"), 0);
    amxc_var_clean(&params);

    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "Enable", "false");
    assert_int_equal(test_amx_set("Device.SoftwareModules.ExecEnv.1.", &params, "./verify_data/amxb_set_single_parameter_2a.json"), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.1.Enable", 0, "./verify_data/amxb_set_single_parameter_2a.json"), 0);
    amxc_var_clean(&params);

    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "Enable", "false");
    assert_int_equal(test_amx_set("Device.SoftwareModules.ExecEnv.*.", &params, "./verify_data/amxb_set_single_parameter_2b.json"), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.1.Enable", 0, "./verify_data/amxb_set_single_parameter_2b.json"), 0);
    amxc_var_clean(&params);

    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "Enable", "true");
    assert_int_equal(test_amx_set("Device.SoftwareModules.ExecEnv.[Name=='TestEnv'].", &params, "./verify_data/amxb_set_single_parameter_3.json"), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.1.Enable", 0, "./verify_data/amxb_set_single_parameter_3.json"), 0);
    amxc_var_clean(&params);
}

void test_amxb_rbus_amxb_set_multiple_parameters_same_object(UNUSED void** state) {
    amxc_var_t params;

    amxc_var_init(&params);

    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "L1Text", "NewText-multiple");
    amxc_var_add_key(cstring_t, &params, "Level2A.L2AText", "NewText-multiple");
    amxc_var_add_key(int16_t, &params, "L1SignedNumber", 100);
    amxc_var_add_key(int8_t, &params, "Level2A.L2ASignedNumber", -10);
    assert_int_equal(test_amx_set("TopLevel.Level1.1.", &params, "./verify_data/amxb_set_multiple_parameter_1.json"), 0);
    assert_int_equal(test_amx_get("TopLevel.Level1.1.", 1, "./verify_data/amxb_set_multiple_parameter_verify_1.json"), 0);
    amxc_var_clean(&params);

    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "L1Text", "NewText-multiple-2");
    amxc_var_add_key(cstring_t, &params, "Level2A.L2AText", "NewText-multiple-2");
    amxc_var_add_key(int16_t, &params, "L1SignedNumber", 110);
    amxc_var_add_key(int8_t, &params, "Level2A.L2ASignedNumber", -20);
    assert_int_equal(test_amx_set("TopLevel.Level1.[L1Text=='NewText-multiple'].", &params, "./verify_data/amxb_set_multiple_parameter_2.json"), 0);
    assert_int_equal(test_amx_get("TopLevel.Level1.1.", 1, "./verify_data/amxb_set_multiple_parameter_verify_2.json"), 0);
    amxc_var_clean(&params);

    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "Enable", "false");
    amxc_var_add_key(uint32_t, &params, "InitialRunLevel", 20);
    assert_int_equal(test_amx_set("Device.SoftwareModules.ExecEnv.1.", &params, "./verify_data/amxb_set_multiple_parameter_3.json"), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.1.", 0, "./verify_data/amxb_set_multiple_parameter_verify_3.json"), 0);
    amxc_var_clean(&params);

    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "Enable", "true");
    amxc_var_add_key(uint32_t, &params, "InitialRunLevel", 30);
    assert_int_equal(test_amx_set("Device.SoftwareModules.ExecEnv.[Name=='TestEnv'].", &params, "./verify_data/amxb_set_multiple_parameter_4.json"), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.1.", 0, "./verify_data/amxb_set_multiple_parameter_verify_4.json"), 0);
    amxc_var_clean(&params);
}

void test_amxb_rbus_amxb_set_multi_partial_allowed(UNUSED void** state) {
    assert_int_equal(test_amx_set_multi("./input_data/amxb_set_multi_objects_1.json", "./verify_data/amxb_set_multi_objects_1.json", AMXB_FLAG_PARTIAL), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.*.", 0, "./verify_data/amxb_set_multi_objects_verify_1a.json"), 0);
    assert_int_equal(test_amx_get("TopLevel.Level1.1.Level2A.Level3.1.", 0, "./verify_data/amxb_set_multi_objects_verify_1b.json"), 0);

    assert_int_equal(test_amx_set_multi("./input_data/amxb_set_multi_objects_2.json", "./verify_data/amxb_set_multi_objects_2.json", AMXB_FLAG_PARTIAL), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.*.", 0, "./verify_data/amxb_set_multi_objects_verify_2a.json"), 0);
    assert_int_equal(test_amx_get("TopLevel.Level1.1.Level2A.Level3.1.", 0, "./verify_data/amxb_set_multi_objects_verify_2b.json"), 0);
}

void test_amxb_rbus_amxb_set_multi_partial_allowed_optional_params(UNUSED void** state) {
    amxc_var_t params;
    amxc_var_t data;

    assert_int_equal(test_amx_set_multi("./input_data/amxb_set_multi_objects_optional_value_1.json", "./verify_data/amxb_set_multi_objects_optional_value_1.json", AMXB_FLAG_PARTIAL), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.*.", 0, "./verify_data/amxb_set_multi_objects_optional_value_verify_1a.json"), 0);
    assert_int_equal(test_amx_get("TopLevel.Level1.1.Level2A.", 0, "./verify_data/amxb_set_multi_objects_optional_value_verify_1b.json"), 0);

    assert_int_equal(test_amx_set_multi("./input_data/amxb_set_multi_objects_optional_value_2.json", "./verify_data/amxb_set_multi_objects_optional_value_2.json", AMXB_FLAG_PARTIAL), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.*.", 0, "./verify_data/amxb_set_multi_objects_optional_value_verify_2a.json"), 0);
    assert_int_equal(test_amx_get("TopLevel.Level1.1.Level2A.", 0, "./verify_data/amxb_set_multi_objects_optional_value_verify_2b.json"), 0);

    // reset value so the test can be repeated
    amxc_var_init(&params);
    amxc_var_init(&data);
    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(int8_t, &params, "L2ASignedNumber", -20);
    assert_int_equal(amxb_set(bus_ctx, "TopLevel.Level1.1.Level2A.", &params, &data, 10), 0);
    amxc_var_clean(&params);
    amxc_var_clean(&data);
}

void test_amxb_rbus_amxb_set_without_values(UNUSED void** state) {
    amxc_var_t params;
    amxc_var_t data;

    amxc_var_init(&data);
    amxc_var_init(&params);
    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxb_set(bus_ctx, "TopLevel.Level1.1.Level2A.", &params, &data, 10), 0);
    amxc_var_dump(&data, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(GETI_ARG(&data, 0)), AMXC_VAR_ID_NULL);

    assert_int_equal(amxb_set(bus_ctx, "TopLevel.Level1.1.Level2A.", NULL, &data, 10), 0);
    amxc_var_dump(&data, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(GETI_ARG(&data, 0)), AMXC_VAR_ID_NULL);

    amxc_var_clean(&params);
    amxc_var_clean(&data);
}

void test_amxb_rbus_amxb_set_non_existing(UNUSED void** state) {
    amxc_var_t params;
    amxc_var_t data;

    amxc_var_init(&data);
    amxc_var_init(&params);
    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(int8_t, &params, "NonExisting", -20);
    assert_int_not_equal(amxb_set(bus_ctx, "TopLevel.Level1.1.NonExisting.", &params, &data, 10), 0);
    assert_int_not_equal(amxb_set(bus_ctx, "TopLevel.Level1.1.Level2A.", &params, &data, 10), 0);

    amxc_var_clean(&params);
    amxc_var_clean(&data);
}

void test_amxb_rbus_amxb_set_multi_no_partial_allowed_valid_values(UNUSED void** state) {
    assert_int_equal(test_amx_set_multi("./input_data/amxb_set_multi_objects_1.json", "./verify_data/amxb_set_multi_objects_1.json", 0), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.*.", 0, "./verify_data/amxb_set_multi_objects_verify_1a.json"), 0);
    assert_int_equal(test_amx_get("TopLevel.Level1.1.Level2A.Level3.1.", 0, "./verify_data/amxb_set_multi_objects_verify_1b.json"), 0);

    assert_int_equal(test_amx_set_multi("./input_data/amxb_set_multi_objects_2.json", "./verify_data/amxb_set_multi_objects_2.json", 0), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.*.", 0, "./verify_data/amxb_set_multi_objects_verify_2a.json"), 0);
    assert_int_equal(test_amx_get("TopLevel.Level1.1.Level2A.Level3.1.", 0, "./verify_data/amxb_set_multi_objects_verify_2b.json"), 0);
}

void test_amxb_rbus_amxb_set_multi_no_partial_allowed_invalid_values(UNUSED void** state) {
    int retval = 0;
    amxc_var_t original_swm;
    amxc_var_t original_testobj;
    amxc_var_t verify_swm;
    amxc_var_t verify_testobj;
    amxc_var_t data;
    amxc_var_t* input = amxut_util_read_json_from_file("./input_data/amxb_set_multi_objects_3.json");

    amxc_var_init(&original_swm);
    amxc_var_init(&original_testobj);
    amxc_var_init(&verify_swm);
    amxc_var_init(&verify_testobj);
    amxc_var_init(&data);

    retval = amxb_get(bus_ctx, "Device.SoftwareModules.ExecEnv.*.", 0, &original_swm, 10);
    assert_int_equal(retval, 0);
    retval = amxb_get(bus_ctx, "TopLevel.Level1.1.Level2A.", 0, &original_testobj, 10);
    assert_int_equal(retval, 0);

    assert_int_not_equal(amxb_set_multiple(bus_ctx, 0, input, &data, 5), 0);

    retval = amxb_get(bus_ctx, "Device.SoftwareModules.ExecEnv.*.", 0, &verify_swm, 10);
    assert_int_equal(retval, 0);
    retval = amxb_get(bus_ctx, "TopLevel.Level1.1.Level2A.", 0, &verify_testobj, 10);
    assert_int_equal(retval, 0);

    amxc_var_compare(&original_swm, &verify_swm, &retval);
    assert_int_equal(retval, 0);
    amxc_var_compare(&original_testobj, &verify_testobj, &retval);
    assert_int_equal(retval, 0);

    amxc_var_clean(&data);
    amxc_var_clean(&original_swm);
    amxc_var_clean(&original_testobj);
    amxc_var_clean(&verify_swm);
    amxc_var_clean(&verify_testobj);
    amxc_var_delete(&input);
}

void test_amxb_rbus_amxb_set_multi_no_partial_allowed_optional_params(UNUSED void** state) {
    amxc_var_t params;
    amxc_var_t data;

    assert_int_equal(test_amx_set_multi("./input_data/amxb_set_multi_objects_optional_value_1.json", "./verify_data/amxb_set_multi_objects_optional_value_1.json", AMXB_FLAG_PARTIAL), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.*.", 0, "./verify_data/amxb_set_multi_objects_optional_value_verify_1a.json"), 0);
    assert_int_equal(test_amx_get("TopLevel.Level1.1.Level2A.", 0, "./verify_data/amxb_set_multi_objects_optional_value_verify_1b.json"), 0);

    assert_int_equal(test_amx_set_multi("./input_data/amxb_set_multi_objects_optional_value_2.json", "./verify_data/amxb_set_multi_objects_optional_value_2.json", AMXB_FLAG_PARTIAL), 0);
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.*.", 0, "./verify_data/amxb_set_multi_objects_optional_value_verify_2a.json"), 0);
    assert_int_equal(test_amx_get("TopLevel.Level1.1.Level2A.", 0, "./verify_data/amxb_set_multi_objects_optional_value_verify_2b.json"), 0);

    // reset value so the test can be repeated
    amxc_var_init(&params);
    amxc_var_init(&data);
    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(int8_t, &params, "L2ASignedNumber", -20);
    assert_int_equal(amxb_set(bus_ctx, "TopLevel.Level1.1.Level2A.", &params, &data, 10), 0);
    amxc_var_clean(&params);
    amxc_var_clean(&data);
}
