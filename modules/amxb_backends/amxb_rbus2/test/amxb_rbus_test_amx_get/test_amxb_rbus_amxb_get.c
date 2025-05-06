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
#include "test_amxb_rbus_amxb_get.h"

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
    }

    amxc_var_clean(&data);
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
    // Doesn't support amx-calls
    system("amxrt -D ../common/odl/registration_test.odl");
    printf("SETUP: Started ...\n");

    sleep(1);

    printf("SETUP: Connect to rbus\n");
    assert_int_equal(amxb_connect(&bus_ctx, "rbus:"), 0);
    amxb_set_access(bus_ctx, AMXB_PUBLIC);
    printf("SETUP: Connected ...\n");

    return 0;
}

int test_amx_setup_full_amx_calls(UNUSED void** state) {
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

int test_amx_setup_no_amx_calls(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_init(&config);
    amxc_var_set_type(&config, AMXC_VAR_ID_HTABLE);

    assert_int_equal(amxb_be_load("../mod-amxb-test-rbus.so"), 0);
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    amxc_var_add_key(bool, rbus_config, "use-amx-calls", false);
    assert_int_equal(amxb_set_config(&config), 0);

    printf("SETUP: Starting amxrt data model providers\n");
    // Uses translation feature
    system("amxrt -D ../common/odl/softwaremodules_translated.odl");
    // Doesn't use translation feature
    system("amxrt -D ../common/odl/registration_test.odl");
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

void test_amxb_rbus_amxb_get_all(UNUSED void** state) {
    int retval = 0;
    amxc_var_t data;

    assert_int_equal(test_amx_get("Device.", -1, "./verify_data/amxb_get_all_1.json"), 0);

    amxc_var_init(&data);
    retval = amxb_get(bus_ctx, "TopLevel.", -1, &data, 10);
    assert_int_equal(retval, 0);
    retval = check_amx_response(&data, "./verify_data/amxb_get_all_2.json");
    assert_int_equal(retval, 0);
    amxc_var_clean(&data);
}

void test_amxb_rbus_amxb_get_with_depth(UNUSED void** state) {
    assert_int_equal(test_amx_get("Device.", 2, "./verify_data/amxb_get_with_depth.json"), 0);
}

void test_amxb_rbus_amxb_get_specific_recursive(UNUSED void** state) {
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    assert_int_equal(test_amx_get("Device.SoftwareModules.", -1, "./verify_data/amxb_get_specific_recursive.json"), 0);
    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    assert_int_equal(test_amx_get("Device.SoftwareModules.", -1, "./verify_data/amxb_get_specific_recursive.json"), 0);
    amxc_var_set(bool, amx_calls, true);  // Turn amx-calls on for this test data model consumer
}

void test_amxb_rbus_amxb_get_specific_recursive_wild_card(UNUSED void** state) {
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.*.", -1, "./verify_data/amxb_get_specific_recursive_wild_card_1.json"), 0);
    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.*.", -1, "./verify_data/amxb_get_specific_recursive_wild_card_1.json"), 0);
    amxc_var_set(bool, amx_calls, true);  // Turn amx-calls on for this test data model consumer
    assert_int_equal(test_amx_get("TopLevel.Level1.*.", -1, "./verify_data/amxb_get_specific_recursive_wild_card_2.json"), 0);
}

void test_amxb_rbus_amxb_get_specific_recursive_wild_card_sub_objects(UNUSED void** state) {
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnvClass.*.Capability.", -1, "./verify_data/amxb_get_specific_recursive_wild_card_sub_object_1.json"), 0);
    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnvClass.*.Capability.", -1, "./verify_data/amxb_get_specific_recursive_wild_card_sub_object_1.json"), 0);
    amxc_var_set(bool, amx_calls, true);  // Turn amx-calls on for this test data model consumer
    assert_int_equal(test_amx_get("TopLevel.Level1.*.Level2A.Level3.*.", -1, "./verify_data/amxb_get_specific_recursive_wild_card_sub_object_2.json"), 0);
}

void test_amxb_rbus_amxb_get_specific_recursive_search_path(UNUSED void** state) {
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.[Enable==false].", -1, "./verify_data/amxb_get_specific_recursive_search_path_1.json"), 0);
    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.[Enable==false].", -1, "./verify_data/amxb_get_specific_recursive_search_path_1.json"), 0);
    amxc_var_set(bool, amx_calls, true);  // Turn amx-calls on for this test data model consumer
    assert_int_equal(test_amx_get("TopLevel.Level1.[Level2A.L2AText!=''].Level2A.Level3.*.", -1, "./verify_data/amxb_get_specific_recursive_search_path_2.json"), 0);
}

void test_amxb_rbus_amxb_get_specific_parameter(UNUSED void** state) {
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.1.Status", -1, "./verify_data/amxb_get_specific_parameter_1.json"), 0);
    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnv.1.Status", -1, "./verify_data/amxb_get_specific_parameter_1.json"), 0);
    amxc_var_set(bool, amx_calls, true);  // Turn amx-calls on for this test data model consumer
    assert_int_equal(test_amx_get("TopLevel.Level1.[Level2A.L2AText!=''].Level2A.Level3.*.L3SignedNumber", -1, "./verify_data/amxb_get_specific_parameter_2.json"), 0);
}

void test_amxb_rbus_amxb_get_non_existing_must_fail(UNUSED void** state) {
    int retval = 0;
    amxc_var_t data;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    amxc_var_init(&data);
    // Will use amx-calls
    retval = amxb_get(bus_ctx, "Device.SoftwareModules.NonExisting.", 1, &data, 10);
    assert_int_not_equal(retval, 0);
    amxc_var_clean(&data);

    amxc_var_init(&data);
    // Will use amx-calls
    retval = amxb_get(bus_ctx, "Device.SoftwareModules.ExecEnv.1.FakeParam", 0, &data, 10);
    assert_int_not_equal(retval, 0);
    amxc_var_clean(&data);

    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    amxc_var_init(&data);
    retval = amxb_get(bus_ctx, "Device.SoftwareModules.NonExisting.", 1, &data, 10);
    assert_int_not_equal(retval, 0);
    amxc_var_clean(&data);
    amxc_var_set(bool, amx_calls, true); // Turn amx-calls on for this test data model consumer

    amxc_var_init(&data);
    // Can never use amx-calls
    retval = amxb_get(bus_ctx, "TopLevel.Level1.99.Level2A.NonExisting.*.", 1, &data, 10);
    assert_int_not_equal(retval, 0);
    amxc_var_clean(&data);

    amxc_var_init(&data);
    // Can never use amx-calls
    retval = amxb_get(bus_ctx, "TopLevel.Level1.1.FakeParam", 0, &data, 10);
    assert_int_not_equal(retval, 0);
    amxc_var_clean(&data);

    amxc_var_init(&data);
    // Can never use amx-calls
    retval = amxb_get(bus_ctx, "TopLevel.Level1.99.", 1, &data, 10);
    assert_int_not_equal(retval, 0);
    amxc_var_clean(&data);
}

void test_amxb_rbus_amxb_get_empty_result(UNUSED void** state) {
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnvClass.[Name=='RBusEnvClass'].", -1, "./verify_data/empty_result.json"), 0);
    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    assert_int_equal(test_amx_get("Device.SoftwareModules.ExecEnvClass.[Name=='RBusEnvClass'].", -1, "./verify_data/empty_result.json"), 0);
    amxc_var_set(bool, amx_calls, true);  // Turn amx-calls on for this test data model consumer
}

void test_amxb_rbus_amxb_get_multiple(UNUSED void** state) {
    amxc_var_t paths;
    amxc_var_t ret;
    int retval = 0;

    amxc_var_init(&paths);
    amxc_var_init(&ret);
    amxc_var_set_type(&paths, AMXC_VAR_ID_LIST);
    amxc_var_add(cstring_t, &paths, "Device.SoftwareModules.ExecEnvClass.[Name=='RBusEnvClass'].");
    amxc_var_add(cstring_t, &paths, "Device.SoftwareModules.ExecEnvClass.*.Capability.");
    amxc_var_add(cstring_t, &paths, "TopLevel.Level1.[Level2A.L2AText==''].Level2A.Level3.*.");
    amxc_var_add(cstring_t, &paths, "TopLevel.Level1.1.Level2A.L2AUnsignedNumber");

    retval = amxb_get_multiple(bus_ctx, &paths, 0, &ret, 5);
    assert_int_equal(retval, 0);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_multiple_1.json");
    assert_int_equal(retval, 0);
    amxc_var_clean(&paths);
    amxc_var_clean(&ret);

    amxc_var_set_type(&paths, AMXC_VAR_ID_LIST);
    amxc_var_add(cstring_t, &paths, "Device.SoftwareModules.ExecEnvClass.[Name=='TestEnvClass'].");
    amxc_var_add(cstring_t, &paths, "Device.SoftwareModules.NoneExisting.*.");
    amxc_var_add(cstring_t, &paths, "TopLevel.Level1.[Level2A.L2AText==''].Level2A.Level3.*.FakeParam");
    amxc_var_add(cstring_t, &paths, "TopLevel.Level1.*.Level2A.");
    retval = amxb_get_multiple(bus_ctx, &paths, -1, &ret, 5);
    assert_int_equal(retval, 0);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_multiple_2.json");
    assert_int_equal(retval, 0);
    amxc_var_clean(&paths);
    amxc_var_clean(&ret);
}

void test_amxb_rbus_amxb_get_reference_following(UNUSED void** state) {
    int retval = 0;
    amxc_var_t data;

    amxc_var_init(&data);

    // Launch a data model provider with a paramter containing a reference
    // Doesn't use translation feature
    // Doesn't support amx-calls
    system("amxrt -D ../common/odl/reference_param.odl");
    sleep(1);

    retval = amxb_get(bus_ctx, "Device.TestObject.Reference", -1, &data, 10);
    assert_int_equal(retval, 0);
    retval = check_amx_response(&data, "./verify_data/amxb_get_reference_following_1.json");
    assert_int_equal(retval, 0);
    amxc_var_clean(&data);

    retval = amxb_get(bus_ctx, "Device.TestObject.Reference+.", -1, &data, 10);
    assert_int_equal(retval, 0);
    retval = check_amx_response(&data, "./verify_data/amxb_get_reference_following_2.json");
    assert_int_equal(retval, 0);
    amxc_var_clean(&data);

    retval = amxb_get(bus_ctx, "Device.TestObject.References", -1, &data, 10);
    assert_int_equal(retval, 0);
    retval = check_amx_response(&data, "./verify_data/amxb_get_reference_following_3.json");
    assert_int_equal(retval, 0);
    amxc_var_clean(&data);

    retval = amxb_get(bus_ctx, "Device.TestObject.References#1+.", -1, &data, 10);
    assert_int_equal(retval, 0);
    retval = check_amx_response(&data, "./verify_data/amxb_get_reference_following_2.json");
    assert_int_equal(retval, 0);
    amxc_var_clean(&data);

    retval = amxb_get(bus_ctx, "Device.TestObject.References#2+.", -1, &data, 10);
    assert_int_equal(retval, 0);
    retval = check_amx_response(&data, "./verify_data/amxb_get_reference_following_4.json");
    assert_int_equal(retval, 0);
    amxc_var_clean(&data);
}
