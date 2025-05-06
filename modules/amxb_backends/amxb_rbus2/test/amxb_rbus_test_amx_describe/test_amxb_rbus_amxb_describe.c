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
#include "test_amxb_rbus_amxb_describe.h"

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
    // Uses translation feature
    // Has support for amx-calls
    system("amxrt -D -d ../common/odl/softwaremodules_translated.odl");
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

static void test_amxb_rbus_check_describe_parameters(amxc_var_t* data, expected_type_t* expected, int line) {
    amxc_var_t* parameters = GETP_ARG(data, "0.parameters");
    int index = 0;

    printf("check describe parameters called from line %d\n", line);
    fflush(stdout);

    assert_non_null(parameters);
    assert_int_equal(amxc_var_type_of(parameters), AMXC_VAR_ID_HTABLE);

    amxc_var_for_each(parameter, parameters) {
        printf("Verifying parameter [%s]\n", amxc_var_key(parameter));
        assert_non_null(GET_ARG(parameter, "type_id"));
        assert_non_null(GET_ARG(parameter, "type_name"));
        assert_non_null(GET_ARG(parameter, "value"));
        assert_non_null(GET_ARG(parameter, "name"));
        assert_non_null(GET_ARG(parameter, "attributes"));                                        // only check if it is available
        assert_int_equal(amxc_var_type_of(GET_ARG(parameter, "attributes")), AMXC_VAR_ID_HTABLE); // and is a table

        assert_string_equal(amxc_var_key(parameter), expected[index].name);
        assert_string_equal(GET_CHAR(parameter, "name"), expected[index].name);
        assert_int_equal(GET_UINT32(parameter, "type_id"), expected[index].type);
        assert_int_equal(amxc_var_type_of(GET_ARG(parameter, "value")), expected[index].type);
        assert_string_equal(GET_CHAR(parameter, "type_name"), amxc_var_type_name_of(GET_ARG(parameter, "value")));
        index++;
    }
}

static void test_amxb_rbus_check_describe_events(amxc_var_t* data, expected_type_t* expected, int line) {
    amxc_var_t* events = GETP_ARG(data, "0.events");
    int index = 0;

    printf("check describe events called from line %d\n", line);
    fflush(stdout);

    assert_non_null(events);
    assert_int_equal(amxc_var_type_of(events), AMXC_VAR_ID_HTABLE);

    amxc_var_for_each(event, events) {
        printf("Verifying event [%s]\n", amxc_var_key(event));

        assert_string_equal(amxc_var_key(event), expected[index].name);
        index++;
    }
}

static void test_amxb_rbus_check_describe_functions(amxc_var_t* data, expected_type_t* expected, int line) {
    amxc_var_t* functions = GETP_ARG(data, "0.functions");
    int index = 0;

    printf("check describe functions called from line %d\n", line);
    fflush(stdout);

    assert_non_null(functions);
    assert_int_equal(amxc_var_type_of(functions), AMXC_VAR_ID_HTABLE);

    amxc_var_for_each(function, functions) {
        printf("Verifying function [%s]\n", amxc_var_key(function));
        assert_string_equal(amxc_var_key(function), expected[index].name);
        assert_non_null(GET_ARG(function, "type_id"));                                           // only check if it is available
        assert_non_null(GET_ARG(function, "type_name"));                                         // only check if it is available
        assert_non_null(GET_ARG(function, "name"));
        assert_non_null(GET_ARG(function, "attributes"));                                        // only check if it is available
        assert_int_equal(amxc_var_type_of(GET_ARG(function, "attributes")), AMXC_VAR_ID_HTABLE); // and is a table

        assert_string_equal(GET_CHAR(function, "name"), expected[index].name);
        index++;
    }
}

void test_amxb_rbus_describe_instance_full_using_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES;

    expected_type_t params[] = {
        {"CapabilityNumberOfEntries", AMXC_VAR_ID_UINT32, 0},
        {"Name", AMXC_VAR_ID_CSTRING, 0},
        {"Alias", AMXC_VAR_ID_CSTRING, 0},
        {"Version", AMXC_VAR_ID_CSTRING, 0},
        {"Vendor", AMXC_VAR_ID_CSTRING, 0},
        {"DeploymentUnitRef", AMXC_VAR_ID_CSTRING, 0},
    };
    expected_type_t events[] = {
    };
    expected_type_t functions[] = {
    };

    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.ExecEnvClass.1.", flags, &data, 5);
    assert_int_equal(retval, 0);
    test_amxb_rbus_check_describe_parameters(&data, params, __LINE__);
    assert_true(GETP_BOOL(&data, "0.parameters.Name.attributes.read-only"));
    test_amxb_rbus_check_describe_events(&data, events, __LINE__);
    test_amxb_rbus_check_describe_functions(&data, functions, __LINE__);
    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(GETP_UINT32(&data, "0.index"), 1);
    assert_string_equal(GETP_CHAR(&data, "0.name"), "cpe-ExecEnvClass-1");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.SoftwareModules.ExecEnvClass.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_instance);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "instance");
    assert_non_null(GETP_ARG(&data, "0.objects"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.objects")), AMXC_VAR_ID_LIST);
    assert_string_equal(GETP_CHAR(&data, "0.objects.0"), "Capability");
    assert_null(GETP_ARG(&data, "0.instances"));
    amxc_var_clean(&data);
}

void test_amxb_rbus_describe_instance_full_not_using_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES;

    expected_type_t params[] = {
        {"CapabilityNumberOfEntries", AMXC_VAR_ID_UINT32, 0},
        {"Name", AMXC_VAR_ID_CSTRING, 0},
        {"Alias", AMXC_VAR_ID_CSTRING, 0},
        {"Version", AMXC_VAR_ID_CSTRING, 0},
        {"Vendor", AMXC_VAR_ID_CSTRING, 0},
        {"DeploymentUnitRef", AMXC_VAR_ID_CSTRING, 0},
    };
    expected_type_t events[] = {
    };
    expected_type_t functions[] = {
    };

    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.ExecEnvClass.1.", flags, &data, 5);
    assert_int_equal(retval, 0);
    test_amxb_rbus_check_describe_parameters(&data, params, __LINE__);
    assert_true(GETP_BOOL(&data, "0.parameters.Name.attributes.read-only"));
    test_amxb_rbus_check_describe_events(&data, events, __LINE__);
    test_amxb_rbus_check_describe_functions(&data, functions, __LINE__);
    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(GETP_UINT32(&data, "0.index"), 1);
    assert_string_equal(GETP_CHAR(&data, "0.name"), "cpe-ExecEnvClass-1");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.SoftwareModules.ExecEnvClass.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_instance);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "instance");
    assert_non_null(GETP_ARG(&data, "0.objects"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.objects")), AMXC_VAR_ID_LIST);
    assert_string_equal(GETP_CHAR(&data, "0.objects.0"), "Capability");
    assert_null(GETP_ARG(&data, "0.instances"));
    amxc_var_clean(&data);
    amxc_var_set(bool, amx_calls, true); // Turn amx-calls on for this test data model consumer
}

void test_amxb_rbus_describe_instance_full(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES;

    ////////////////////////////////////////////////////////////////////////////
    expected_type_t params1[] = {
        {"L3Text", AMXC_VAR_ID_CSTRING, 0},
        {"L3UnsignedNumber", AMXC_VAR_ID_UINT64, 0},
        {"L3SignedNumber", AMXC_VAR_ID_INT64, 0},
    };
    expected_type_t events1[] = {
        {"L3Event!", AMXC_VAR_ID_NULL, 0},
    };
    expected_type_t functions1[] = {
        {"L3Method", -1, 0},
    };
    ////////////////////////////////////////////////////////////////////////////
    expected_type_t params2[] = {
        {"L1Text", AMXC_VAR_ID_CSTRING, 0},
        {"L1UnsignedNumber", AMXC_VAR_ID_UINT16, 0},
        {"L1SignedNumber", AMXC_VAR_ID_INT16, 0},
    };
    expected_type_t events2[] = {
        {"L1Event!", AMXC_VAR_ID_NULL, 0},
    };
    expected_type_t functions2[] = {
        {"L1Method", -1, 0},
    };

    ////////////////////////////////////////////////////////////////////////////
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "TopLevel.Level1.1.Level2A.Level3.1.", flags, &data, 5);
    assert_int_equal(retval, 0);
    test_amxb_rbus_check_describe_parameters(&data, params1, __LINE__);
    test_amxb_rbus_check_describe_events(&data, events1, __LINE__);
    test_amxb_rbus_check_describe_functions(&data, functions1, __LINE__);
    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(GETP_UINT32(&data, "0.index"), 1);
    assert_string_equal(GETP_CHAR(&data, "0.name"), "1");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "TopLevel.Level1.1.Level2A.Level3.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_instance);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "instance");
    assert_null(GETP_ARG(&data, "0.instances"));
    assert_null(GETP_ARG(&data, "0.objects"));
    amxc_var_clean(&data);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "TopLevel.Level1.1.", flags, &data, 5);
    assert_int_equal(retval, 0);
    test_amxb_rbus_check_describe_parameters(&data, params2, __LINE__);
    test_amxb_rbus_check_describe_events(&data, events2, __LINE__);
    test_amxb_rbus_check_describe_functions(&data, functions2, __LINE__);
    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(GETP_UINT32(&data, "0.index"), 1);
    assert_string_equal(GETP_CHAR(&data, "0.name"), "1");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "TopLevel.Level1.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_instance);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "instance");
    assert_null(GETP_ARG(&data, "0.instances"));
    assert_non_null(GETP_ARG(&data, "0.objects"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.objects")), AMXC_VAR_ID_LIST);
    assert_string_equal(GETP_CHAR(&data, "0.objects.0"), "Level2A");
    assert_string_equal(GETP_CHAR(&data, "0.objects.1"), "Level2B");
    assert_string_equal(GETP_CHAR(&data, "0.objects.2"), "Level2C");
    amxc_var_clean(&data);
    ////////////////////////////////////////////////////////////////////////////
}

void test_amxb_rbus_describe_instance_only_using_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = 0;

    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.ExecEnvClass.1.", flags, &data, 5);
    assert_int_equal(retval, 0);
    assert_null(GETP_ARG(&data, "0.parameters"));
    assert_null(GETP_ARG(&data, "0.events"));
    assert_null(GETP_ARG(&data, "0.functions"));
    assert_null(GETP_ARG(&data, "0.objects"));
    assert_null(GETP_ARG(&data, "0.instances"));

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(GETP_UINT32(&data, "0.index"), 1);
    assert_string_equal(GETP_CHAR(&data, "0.name"), "cpe-ExecEnvClass-1");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.SoftwareModules.ExecEnvClass.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_instance);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "instance");
    amxc_var_clean(&data);
}

void test_amxb_rbus_describe_instance_only_not_using_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");
    uint32_t flags = 0;

    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.ExecEnvClass.1.", flags, &data, 5);
    amxc_var_dump(&data, STDOUT_FILENO);
    assert_int_equal(retval, 0);
    assert_null(GETP_ARG(&data, "0.parameters"));
    assert_null(GETP_ARG(&data, "0.events"));
    assert_null(GETP_ARG(&data, "0.functions"));
    assert_null(GETP_ARG(&data, "0.objects"));
    assert_null(GETP_ARG(&data, "0.instances"));

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(GETP_UINT32(&data, "0.index"), 1);
    assert_string_equal(GETP_CHAR(&data, "0.name"), "cpe-ExecEnvClass-1");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.SoftwareModules.ExecEnvClass.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_instance);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "instance");
    amxc_var_clean(&data);
    amxc_var_set(bool, amx_calls, true); // Turn amx-calls on for this test data model consumer
}

void test_amxb_rbus_describe_instance_only(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = 0;

    ////////////////////////////////////////////////////////////////////////////
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "TopLevel.Level1.1.Level2A.Level3.1.", flags, &data, 5);
    amxc_var_dump(&data, STDOUT_FILENO);
    assert_int_equal(retval, 0);
    assert_int_equal(retval, 0);
    assert_null(GETP_ARG(&data, "0.parameters"));
    assert_null(GETP_ARG(&data, "0.events"));
    assert_null(GETP_ARG(&data, "0.functions"));
    assert_null(GETP_ARG(&data, "0.objects"));
    assert_null(GETP_ARG(&data, "0.instances"));

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(GETP_UINT32(&data, "0.index"), 1);
    assert_string_equal(GETP_CHAR(&data, "0.name"), "1");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "TopLevel.Level1.1.Level2A.Level3.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_instance);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "instance");
    amxc_var_clean(&data);
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "TopLevel.Level1.1.", flags, &data, 5);
    assert_int_equal(retval, 0);
    assert_null(GETP_ARG(&data, "0.parameters"));
    assert_null(GETP_ARG(&data, "0.events"));
    assert_null(GETP_ARG(&data, "0.functions"));
    assert_null(GETP_ARG(&data, "0.objects"));
    assert_null(GETP_ARG(&data, "0.instances"));

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(GETP_UINT32(&data, "0.index"), 1);
    assert_string_equal(GETP_CHAR(&data, "0.name"), "1");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "TopLevel.Level1.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_instance);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "instance");
    amxc_var_clean(&data);
    ////////////////////////////////////////////////////////////////////////////
}

void test_amxb_rbus_describe_table_full_using_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES;

    expected_type_t params[] = {
        {"InitialExecutionUnitRunLevel", AMXC_VAR_ID_INT32, 0},
        {"CurrentRunLevel", AMXC_VAR_ID_INT32, 0},
        {"Status", AMXC_VAR_ID_CSTRING, 0},
        {"Enable", AMXC_VAR_ID_BOOL, 0},
        {"Name", AMXC_VAR_ID_CSTRING, 0},
        {"Alias", AMXC_VAR_ID_CSTRING, 0},
        {"InitialRunLevel", AMXC_VAR_ID_UINT32, 0},
        {"Type", AMXC_VAR_ID_CSTRING, 0},
    };
    expected_type_t events[] = {
    };
    expected_type_t functions[] = {
    };

    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.ExecEnv.", flags, &data, 5);
    assert_int_equal(retval, 0);
    test_amxb_rbus_check_describe_parameters(&data, params, __LINE__);
    test_amxb_rbus_check_describe_events(&data, events, __LINE__);
    test_amxb_rbus_check_describe_functions(&data, functions, __LINE__);
    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "ExecEnv");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.SoftwareModules.ExecEnv.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_template);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "template");
    assert_non_null(GETP_ARG(&data, "0.instances"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.instances")), AMXC_VAR_ID_LIST);
    assert_string_equal(GETP_CHAR(&data, "0.instances.0"), "1");
    assert_string_equal(GETP_CHAR(&data, "0.instances.1"), "2");
    assert_null(GETP_ARG(&data, "0.objects"));
    amxc_var_clean(&data);
}

void test_amxb_rbus_describe_table_full_without_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.ExecEnv.", flags, &data, 5);
    assert_int_equal(retval, 0);
    // No details can be provided for tables using rbus api
    //test_amxb_rbus_check_describe_parameters(&data, params, __LINE__);
    //test_amxb_rbus_check_describe_events(&data, events, __LINE__);
    //test_amxb_rbus_check_describe_functions(&data, functions, __LINE__);

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "ExecEnv");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.SoftwareModules.ExecEnv.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_template);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "template");
    assert_non_null(GETP_ARG(&data, "0.instances"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.instances")), AMXC_VAR_ID_LIST);
    assert_string_equal(GETP_CHAR(&data, "0.instances.0"), "1");
    assert_string_equal(GETP_CHAR(&data, "0.instances.1"), "2");
    assert_null(GETP_ARG(&data, "0.objects"));
    amxc_var_clean(&data);
    amxc_var_set(bool, amx_calls, true); // Turn amx-calls on for this test data model consumer
}

void test_amxb_rbus_describe_table_full(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES;

    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "TopLevel.Level1.1.Level2A.Level3.", flags, &data, 5);
    assert_int_equal(retval, 0);
    // No details can be provided for tables using rbus api
    //test_amxb_rbus_check_describe_parameters(&data, params, __LINE__);
    //test_amxb_rbus_check_describe_events(&data, events, __LINE__);
    //test_amxb_rbus_check_describe_functions(&data, functions, __LINE__);

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "Level3");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "TopLevel.Level1.1.Level2A.Level3.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_template);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "template");
    assert_non_null(GETP_ARG(&data, "0.instances"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.instances")), AMXC_VAR_ID_LIST);
    assert_string_equal(GETP_CHAR(&data, "0.instances.0"), "1");
    assert_null(GETP_ARG(&data, "0.objects"));
    amxc_var_clean(&data);
}

void test_amxb_rbus_describe_table_only_using_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = 0;

    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.ExecEnv.", flags, &data, 5);
    assert_int_equal(retval, 0);
    assert_null(GETP_ARG(&data, "0.parameters"));
    assert_null(GETP_ARG(&data, "0.events"));
    assert_null(GETP_ARG(&data, "0.functions"));
    assert_null(GETP_ARG(&data, "0.objects"));
    assert_null(GETP_ARG(&data, "0.instances"));

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "ExecEnv");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.SoftwareModules.ExecEnv.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_template);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "template");
    amxc_var_clean(&data);
}

void test_amxb_rbus_describe_table_only_without_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = 0;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.ExecEnv.", flags, &data, 5);
    assert_int_equal(retval, 0);
    assert_null(GETP_ARG(&data, "0.parameters"));
    assert_null(GETP_ARG(&data, "0.events"));
    assert_null(GETP_ARG(&data, "0.functions"));
    assert_null(GETP_ARG(&data, "0.objects"));
    assert_null(GETP_ARG(&data, "0.instances"));

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "ExecEnv");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.SoftwareModules.ExecEnv.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_template);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "template");
    amxc_var_clean(&data);
    amxc_var_set(bool, amx_calls, true); // Turn amx-calls on for this test data model consumer
}

void test_amxb_rbus_describe_table_only(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = 0;

    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "TopLevel.Level1.1.Level2A.Level3.", flags, &data, 5);
    assert_int_equal(retval, 0);
    assert_null(GETP_ARG(&data, "0.parameters"));
    assert_null(GETP_ARG(&data, "0.events"));
    assert_null(GETP_ARG(&data, "0.functions"));
    assert_null(GETP_ARG(&data, "0.objects"));
    assert_null(GETP_ARG(&data, "0.instances"));

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "Level3");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "TopLevel.Level1.1.Level2A.Level3.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_template);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "template");
    amxc_var_clean(&data);
}

void test_amxb_rbus_describe_singelton_full_using_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES;

    expected_type_t params[] = {
        {"Enable", AMXC_VAR_ID_BOOL, 0},
        {"InitialExecutionUnitRunLevel", AMXC_VAR_ID_INT32, 0},
        {"CurrentRunLevel", AMXC_VAR_ID_INT32, 0},
        {"Name", AMXC_VAR_ID_CSTRING, 0},
        {"Status", AMXC_VAR_ID_CSTRING, 0},
        {"InitialRunLevel", AMXC_VAR_ID_UINT32, 0},
        {"Type", AMXC_VAR_ID_CSTRING, 0},
        {"Alias", AMXC_VAR_ID_CSTRING, 0},
    };
    expected_type_t events[] = {
    };
    expected_type_t functions[] = {
    };

    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.TestDescribe.", flags, &data, 5);
    assert_int_equal(retval, 0);
    test_amxb_rbus_check_describe_parameters(&data, params, __LINE__);
    test_amxb_rbus_check_describe_events(&data, events, __LINE__);
    test_amxb_rbus_check_describe_functions(&data, functions, __LINE__);

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "TestDescribe");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.TestDescribe.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_singleton);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "singleton");
    assert_null(GETP_ARG(&data, "0.instances"));
    assert_non_null(GETP_ARG(&data, "0.objects"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.objects")), AMXC_VAR_ID_LIST);
    amxc_var_clean(&data);
}

void test_amxb_rbus_describe_singelton_full_without_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    expected_type_t params[] = {
        {"Enable", AMXC_VAR_ID_BOOL, 0},
        {"InitialExecutionUnitRunLevel", AMXC_VAR_ID_INT32, 0},
        {"CurrentRunLevel", AMXC_VAR_ID_INT32, 0},
        {"Name", AMXC_VAR_ID_CSTRING, 0},
        {"Status", AMXC_VAR_ID_CSTRING, 0},
        {"InitialRunLevel", AMXC_VAR_ID_UINT32, 0},
        {"Type", AMXC_VAR_ID_CSTRING, 0},
        {"Alias", AMXC_VAR_ID_CSTRING, 0},
    };
    expected_type_t events[] = {
    };
    expected_type_t functions[] = {
    };

    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.TestDescribe.", flags, &data, 5);
    assert_int_equal(retval, 0);
    test_amxb_rbus_check_describe_parameters(&data, params, __LINE__);
    test_amxb_rbus_check_describe_events(&data, events, __LINE__);
    test_amxb_rbus_check_describe_functions(&data, functions, __LINE__);

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "TestDescribe");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.TestDescribe.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_singleton);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "singleton");
    assert_null(GETP_ARG(&data, "0.instances"));
    assert_null(GETP_ARG(&data, "0.objects")); // TODO: different with amx-calls, needs fixing?
    amxc_var_clean(&data);
    amxc_var_set(bool, amx_calls, true);       // Turn amx-calls on for this test data model consumer
}

void test_amxb_rbus_describe_singleton_full(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES;

    expected_type_t params[] = {
        {"L2AText", AMXC_VAR_ID_CSTRING, 0},
        {"L2ASignedNumber", AMXC_VAR_ID_INT8, 0},
        {"L2AUnsignedNumber", AMXC_VAR_ID_UINT8, 0},
    };
    expected_type_t events[] = {
        {"L2AEvent!", AMXC_VAR_ID_NULL, 0},
    };
    expected_type_t functions[] = {
        {"L2AMethod", -1, 0},
    };

    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "TopLevel.Level1.1.Level2A.", flags, &data, 5);
    assert_int_equal(retval, 0);
    test_amxb_rbus_check_describe_parameters(&data, params, __LINE__);
    test_amxb_rbus_check_describe_events(&data, events, __LINE__);
    test_amxb_rbus_check_describe_functions(&data, functions, __LINE__);
    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "Level2A");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "TopLevel.Level1.1.Level2A.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_singleton);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "singleton");
    assert_null(GETP_ARG(&data, "0.instances"));
    assert_non_null(GETP_ARG(&data, "0.objects"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.objects")), AMXC_VAR_ID_LIST);
    assert_string_equal(GETP_CHAR(&data, "0.objects.0"), "Level3");

    amxc_var_clean(&data);
}

void test_amxb_rbus_describe_singelton_object_only_with_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = 0;

    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.TestDescribe.", flags, &data, 5);
    assert_int_equal(retval, 0);
    assert_null(GETP_ARG(&data, "0.parameters"));
    assert_null(GETP_ARG(&data, "0.events"));
    assert_null(GETP_ARG(&data, "0.functions"));
    assert_null(GETP_ARG(&data, "0.objects"));
    assert_null(GETP_ARG(&data, "0.instances"));

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "TestDescribe");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.TestDescribe.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_singleton);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "singleton");
    amxc_var_clean(&data);
}

void test_amxb_rbus_describe_singelton_object_only_without_amx_calls(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = 0;

    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.TestDescribe.", flags, &data, 5);
    assert_int_equal(retval, 0);
    assert_null(GETP_ARG(&data, "0.parameters"));
    assert_null(GETP_ARG(&data, "0.events"));
    assert_null(GETP_ARG(&data, "0.functions"));
    assert_null(GETP_ARG(&data, "0.objects"));
    assert_null(GETP_ARG(&data, "0.instances"));

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "TestDescribe");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "Device.TestDescribe.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_singleton);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "singleton");
    amxc_var_clean(&data);
}

void test_amxb_rbus_describe_object_only(UNUSED void** state) {
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = 0;

    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "TopLevel.Level1.1.Level2A.", flags, &data, 5);
    assert_int_equal(retval, 0);
    assert_null(GETP_ARG(&data, "0.parameters"));
    assert_null(GETP_ARG(&data, "0.events"));
    assert_null(GETP_ARG(&data, "0.functions"));
    assert_null(GETP_ARG(&data, "0.objects"));
    assert_null(GETP_ARG(&data, "0.instances"));

    assert_non_null(GETP_ARG(&data, "0.attributes"));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0.attributes")), AMXC_VAR_ID_HTABLE);
    assert_null(GETP_UINT32(&data, "0.index"));
    assert_string_equal(GETP_CHAR(&data, "0.name"), "Level2A");
    assert_string_equal(GETP_CHAR(&data, "0.path"), "TopLevel.Level1.1.Level2A.");
    assert_int_equal(GETP_UINT32(&data, "0.type_id"), amxd_object_singleton);
    assert_string_equal(GETP_CHAR(&data, "0.type_name"), "singleton");

    amxc_var_clean(&data);
}

void test_amxb_rbus_describe_on_none_existing_returns_empty_table(UNUSED void** state) {
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");
    amxc_var_t data;
    uint32_t retval = 0;
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES;

    // Describe a non existing instance using amx-call
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.ExecEnv.99.", flags, &data, 5);
    assert_int_not_equal(retval, 0);
    assert_non_null(GETI_ARG(&data, 0));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxc_htable_size(amxc_var_constcast(amxc_htable_t, GETP_ARG(&data, "0"))), 0);
    amxc_var_clean(&data);

    // Describe a non existing instance using RBus API
    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.ExecEnv.99.", flags, &data, 5);
    assert_int_not_equal(retval, 0);
    assert_non_null(GETI_ARG(&data, 0));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxc_htable_size(amxc_var_constcast(amxc_htable_t, GETP_ARG(&data, "0"))), 0);
    amxc_var_clean(&data);
    amxc_var_set(bool, amx_calls, true); // Turn amx-calls on for this test data model consumer

    // Describe a non existing object using amx-call
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.FakeObject.", flags, &data, 5);
    assert_int_not_equal(retval, 0);
    assert_non_null(GETI_ARG(&data, 0));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxc_htable_size(amxc_var_constcast(amxc_htable_t, GETP_ARG(&data, "0"))), 0);
    amxc_var_clean(&data);

    // Describe a non existing object using RBus api
    amxc_var_set(bool, amx_calls, false); // Turn amx-calls off for this test data model consumer
    amxc_var_init(&data);
    retval = amxb_describe(bus_ctx, "Device.SoftwareModules.FakeObject.", flags, &data, 5);
    assert_int_not_equal(retval, 0);
    assert_non_null(GETI_ARG(&data, 0));
    assert_int_equal(amxc_var_type_of(GETP_ARG(&data, "0")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxc_htable_size(amxc_var_constcast(amxc_htable_t, GETP_ARG(&data, "0"))), 0);
    amxc_var_clean(&data);
    amxc_var_set(bool, amx_calls, true); // Turn amx-calls on for this test data model consumer
}
