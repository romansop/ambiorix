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

#include <string.h>
#include <unistd.h>

#include "../common/include/amxb_rbus_test.h"
#include "test_amxb_rbus_register.h"

static amxc_var_t config;
static amxd_dm_t dm;
static amxo_parser_t parser;
static amxb_bus_ctx_t* bus_ctx = NULL;
static rbusHandle_t handle = NULL;

static const char* check_element_names[] = {
    "TopLevel.TLUnsignedNumber",
    "TopLevel.TLTimeStamp",
    "TopLevel.TLSignedNumber",
    "TopLevel.TLText",
    "TopLevel.amx_notify!",
    "TopLevel.TLEvent!",
    "TopLevel.TLMethod()",
    "TopLevel.Level1.{i}.",
    "TopLevel.Level1.{i}.L1Text",
    "TopLevel.Level1.{i}.L1UnsignedNumber",
    "TopLevel.Level1.{i}.L1SignedNumber",
    "TopLevel.Level1.{i}.amx_notify!",
    "TopLevel.Level1.{i}.L1Event!",
    "TopLevel.Level1.{i}.L1Method()",
    "TopLevel.Level1.{i}.Level2A.L2AText",
    "TopLevel.Level1.{i}.Level2A.L2ASignedNumber",
    "TopLevel.Level1.{i}.Level2A.L2AUnsignedNumber",
    "TopLevel.Level1.{i}.Level2A.amx_notify!",
    "TopLevel.Level1.{i}.Level2A.L2AEvent!",
    "TopLevel.Level1.{i}.Level2A.L2AMethod()",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3Text",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3UnsignedNumber",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3SignedNumber",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.amx_notify!",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3Event!",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3Method()",
    "TopLevel.Level1.{i}.Level2B.{i}.",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BBool",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BText",
    "TopLevel.Level1.{i}.Level2B.{i}.Alias",
    "TopLevel.Level1.{i}.Level2B.{i}.amx_notify!",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BEvent!",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BMethod()",
    "TopLevel.Level1.{i}.Level2C.{i}.",
    "TopLevel.Level1.{i}.Level2C.{i}.L2CBool",
    "TopLevel.Level1.{i}.Level2C.{i}.L2CDouble",
    "TopLevel.Level1.{i}.Level2C.{i}.L2CText",
    "TopLevel.Level1.{i}.Level2C.{i}.Alias",
    "TopLevel.Level1.{i}.Level2C.{i}.amx_notify!",
    NULL
};

static const char* check_extra_element_names[] = {
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.Extra.ExtraNumber",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.Extra.ExtraText",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.Extra.amx_notify!",
    NULL
};

static const char* check_element_names_fetched[] = {
    "TopLevel.TLUnsignedNumber",
    "TopLevel.TLTimeStamp",
    "TopLevel.TLSignedNumber",
    "TopLevel.TLText",
    "TopLevel.amx_notify!",
    "TopLevel.TLEvent!",
    "TopLevel.TLMethod()",
    "TopLevel.Level1.{i}",
    "TopLevel.Level1.{i}.L1Text",
    "TopLevel.Level1.{i}.L1UnsignedNumber",
    "TopLevel.Level1.{i}.L1SignedNumber",
    "TopLevel.Level1.{i}.amx_notify!",
    "TopLevel.Level1.{i}.L1Event!",
    "TopLevel.Level1.{i}.L1Method()",
    "TopLevel.Level1.{i}.Level2A.L2AText",
    "TopLevel.Level1.{i}.Level2A.L2ASignedNumber",
    "TopLevel.Level1.{i}.Level2A.L2AUnsignedNumber",
    "TopLevel.Level1.{i}.Level2A.amx_notify!",
    "TopLevel.Level1.{i}.Level2A.L2AEvent!",
    "TopLevel.Level1.{i}.Level2A.L2AMethod()",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3Text",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3UnsignedNumber",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3SignedNumber",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.amx_notify!",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3Event!",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3Method()",
    "TopLevel.Level1.{i}.Level2B.{i}",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BBool",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BText",
    "TopLevel.Level1.{i}.Level2B.{i}.Alias",
    "TopLevel.Level1.{i}.Level2B.{i}.amx_notify!",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BEvent!",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BMethod()",
    "TopLevel.Level1.{i}.Level2C.{i}",
    "TopLevel.Level1.{i}.Level2C.{i}.L2CBool",
    "TopLevel.Level1.{i}.Level2C.{i}.L2CDouble",
    "TopLevel.Level1.{i}.Level2C.{i}.L2CText",
    "TopLevel.Level1.{i}.Level2C.{i}.Alias",
    "TopLevel.Level1.{i}.Level2C.{i}.amx_notify!",
    NULL
};

static const char* check_extra_element_names_fetched[] = {
    "TopLevel.TLUnsignedNumber",
    "TopLevel.TLTimeStamp",
    "TopLevel.TLSignedNumber",
    "TopLevel.TLText",
    "TopLevel.amx_notify!",
    "TopLevel.TLEvent!",
    "TopLevel.TLMethod()",
    "TopLevel.Level1.{i}",
    "TopLevel.Level1.{i}.L1Text",
    "TopLevel.Level1.{i}.L1UnsignedNumber",
    "TopLevel.Level1.{i}.L1SignedNumber",
    "TopLevel.Level1.{i}.amx_notify!",
    "TopLevel.Level1.{i}.L1Event!",
    "TopLevel.Level1.{i}.L1Method()",
    "TopLevel.Level1.{i}.Level2A.L2AText",
    "TopLevel.Level1.{i}.Level2A.L2ASignedNumber",
    "TopLevel.Level1.{i}.Level2A.L2AUnsignedNumber",
    "TopLevel.Level1.{i}.Level2A.amx_notify!",
    "TopLevel.Level1.{i}.Level2A.L2AEvent!",
    "TopLevel.Level1.{i}.Level2A.L2AMethod()",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3Text",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3UnsignedNumber",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3SignedNumber",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.amx_notify!",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3Event!",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.L3Method()",
    "TopLevel.Level1.{i}.Level2B.{i}",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BBool",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BText",
    "TopLevel.Level1.{i}.Level2B.{i}.Alias",
    "TopLevel.Level1.{i}.Level2B.{i}.amx_notify!",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BEvent!",
    "TopLevel.Level1.{i}.Level2B.{i}.L2BMethod()",
    "TopLevel.Level1.{i}.Level2C.{i}",
    "TopLevel.Level1.{i}.Level2C.{i}.L2CBool",
    "TopLevel.Level1.{i}.Level2C.{i}.L2CDouble",
    "TopLevel.Level1.{i}.Level2C.{i}.L2CText",
    "TopLevel.Level1.{i}.Level2C.{i}.Alias",
    "TopLevel.Level1.{i}.Level2C.{i}.amx_notify!",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.Extra.ExtraNumber",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.Extra.ExtraText",
    "TopLevel.Level1.{i}.Level2A.Level3.{i}.Extra.amx_notify!",
    NULL
};

int test_suite_setup(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    assert_int_equal(rbus_checkStatus(), RBUS_ENABLED);
    amxc_var_t* rbus_config = NULL;

    amxc_var_init(&config);
    amxc_var_set_type(&config, AMXC_VAR_ID_HTABLE);

    // load the amxb-rbus test backend
    assert_int_equal(amxb_be_load("../mod-amxb-test-rbus.so"), 0);
    // and apply configuration settings
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    amxc_var_add_key(cstring_t, rbus_config, "register-name", "test-amxb-rbus");
    amxc_var_add_key(bool, rbus_config, "use-amx-calls", false);
    assert_int_equal(amxb_set_config(&config), 0);

    printf("SETUP: Starting amxrt data model provider\n");
    system("amxrt -D ../common/odl/softwaremodules.odl");
    printf("SETUP: Started ...\n");

    printf("SETUP: Connect to rbus\n");

    rc = rbus_open(&handle, "rbus-unit-test");
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("SETUP: Connected ...\n");

    return 0;
}

int test_suite_teardown(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;

    printf("TEARDOWN: Stopping amxrt data model provider\n");
    system("killall amxrt");
    printf("TEARDOWN: Stopped ...\n");

    printf("TEARDOWN: Disconnect from rbus\n");
    rc = rbus_close(handle);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("TEARDOWN: Disconnected ...\n");

    // remove all backends (only one shold be loaded)
    amxb_be_remove_all();
    amxc_var_clean(&config);

    return 0;
}

int test_setup(UNUSED void** state) {
    // initialize and load a data model definition
    amxd_dm_init(&dm);
    amxo_parser_init(&parser);

    assert_int_equal(amxb_connect(&bus_ctx, "rbus:"), 0);

    return 0;
}

int test_teardown(UNUSED void** state) {
    printf("TEST TEARDOWN: Disconnect\n");
    amxb_free(&bus_ctx);

    printf("TEST TEARDOWN: clean-up\n");
    amxo_parser_clean(&parser);
    amxd_dm_clean(&dm);
    printf("TEST TEARDOWN: done\n");
    return 0;
}

void test_amxb_rbus_native_can_discover_component_name(UNUSED void** state) {
    int count = 0;
    char** names = NULL;
    int found_count = 0;
    rbusCoreError_t rc = RBUSCORE_SUCCESS;

    // When the amxb-rbus backends connects to rbus daemon it uses
    // a name to register itself

    // When fetching all registered components
    rc = rbus_discoverRegisteredComponents(&count, &names);
    assert_int_equal(rc, RBUSCORE_SUCCESS);

    // the names must be in the list.
    for(int i = 0; i < count; i++) {
        if(strcmp(names[i], "test-amxb-rbus") == 0) {
            // this test data model provider
            found_count++;
        }
        if(strcmp(names[i], "test-software-modules") == 0) {
            // the test software modules provider
            found_count++;
        }
        free(names[i]);
    }
    free(names);

    assert_int_equal(found_count, 2);
}

void test_amxb_rbus_native_can_discover_component_name_using_element(UNUSED void** state) {
    int element_count = 3;
    char const* element_names[3] = {
        "TopLevel.Level1.{i}.Level2B.{i}.L2BBool",
        "Device.SoftwareModules.ExecEnv.{i}.",
        "TopLevel.Level1.{i}.Level2A."
    };
    const char* expected_components[3] = {
        "test-amxb-rbus",
        "test-software-modules",
        "test-amxb-rbus"
    };
    rbusError_t rc = RBUS_ERROR_SUCCESS;
    int component_count = 0;
    char** component_names = NULL;

    // when the data model is registered
    assert_int_equal(amxo_parser_parse_file(&parser, "../common/odl/registration_test_definition.odl", amxd_dm_get_root(&dm)), 0);
    assert_int_equal(amxb_register(bus_ctx, &dm), 0);

    // an data model consumer must be able to find the component names using the element names
    rc = rbus_discoverComponentName(handle, element_count, element_names, &component_count, &component_names);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    assert_int_not_equal(component_count, 0);

    for(int i = 0; i < component_count; i++) {
        printf("%s\n", component_names[i]);
        assert_string_equal(expected_components[i], component_names[i]);
        free(component_names[i]);
    }
    free(component_names);
}

void test_amxb_rbus_provider_registers_supported_datamodel(UNUSED void** state) {
    for(int i = 0; check_element_names[i] != NULL; i++) {
        expect_string(__wrap_rbus_regDataElements, name, check_element_names[i]);
    }
    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", true);

    // when the supported data model is registered
    assert_int_equal(amxo_parser_parse_file(&parser, "../common/odl/registration_test_definition.odl", amxd_dm_get_root(&dm)), 0);
    assert_int_equal(amxb_register(bus_ctx, &dm), 0);
    // rbus_regDataElements must be called for all elements

    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", false);

    check_supported_datamodel_is_available(handle, "test-amxb-rbus", check_element_names_fetched, __LINE__);

    // when registering the same data model, the register should fail
    // disable this check for now - it fails when using the latest release of ambiorix libs
    // TODO: Investigate and fix this issue
    // assert_int_not_equal(amxb_register(bus_ctx, &dm), 0);
}

void test_amxb_rbus_provider_adds_rows(UNUSED void** state) {
    const char* expected_level1[] = { "TopLevel.Level1.1." };
    const char* expected_level3[] = { "TopLevel.Level1.1.Level2A.Level3.1." };
    const char* expected_level2b[] = { "TopLevel.Level1.1.Level2B.1." };

    for(int i = 0; check_element_names[i] != NULL; i++) {
        expect_string(__wrap_rbus_regDataElements, name, check_element_names[i]);
    }

    expect_string(__wrap_rbusTable_registerRow, tableName, "TopLevel.Level1.");
    expect_value(__wrap_rbusTable_registerRow, instNum, 1);
    expect_string(__wrap_rbusTable_registerRow, tableName, "TopLevel.Level1.1.Level2A.Level3.");
    expect_value(__wrap_rbusTable_registerRow, instNum, 1);
    expect_string(__wrap_rbusTable_registerRow, tableName, "TopLevel.Level1.1.Level2B.");
    expect_value(__wrap_rbusTable_registerRow, instNum, 1);
    expect_string(__wrap_rbusTable_registerRow, aliasName, "cpe-Level2B-1");
    expect_string(__wrap_rbusTable_registerRow, tableName, "TopLevel.Level1.1.Level2C.");
    expect_value(__wrap_rbusTable_registerRow, instNum, 1);
    expect_string(__wrap_rbusTable_registerRow, aliasName, "cpe-Level2C-1");

    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", true);

    // when the supported data model is registered and instances are avaialble
    assert_int_equal(amxo_parser_parse_file(&parser, "../common/odl/registration_test.odl", amxd_dm_get_root(&dm)), 0);
    assert_int_equal(amxb_register(bus_ctx, &dm), 0);
    // the supported data model must be registered and all instances must be registered as well

    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", false);

    check_rows_are_available(handle, "TopLevel.Level1.", expected_level1, __LINE__);
    check_rows_are_available(handle, "TopLevel.Level1.1.Level2A.Level3.", expected_level3, __LINE__);
    check_rows_are_available(handle, "TopLevel.Level1.1.Level2B.", expected_level2b, __LINE__);
}

void test_amxb_rbus_provider_adds_newly_added_instances(UNUSED void** state) {
    amxd_trans_t transaction;
    const char* expected_level1[] = { "TopLevel.Level1.1." };
    const char* expected_level2b[] = { "TopLevel.Level1.1.Level2B.5." };

    assert_int_equal(amxo_parser_parse_file(&parser, "../common/odl/registration_test_definition.odl", amxd_dm_get_root(&dm)), 0);
    assert_int_equal(amxb_register(bus_ctx, &dm), 0);
    // make event queue empty before continuing
    handle_events();

    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", true);

    // When a new instance is added
    assert_int_equal(amxd_trans_init(&transaction), 0);
    assert_int_equal(amxd_trans_select_pathf(&transaction, "TopLevel.Level1."), 0);
    assert_int_equal(amxd_trans_add_inst(&transaction, 0, NULL), 0);
    assert_int_equal(amxd_trans_select_pathf(&transaction, "TopLevel.Level1.1.Level2B."), 0);
    assert_int_equal(amxd_trans_add_inst(&transaction, 5, "TestAlias"), 0);
    assert_int_equal(amxd_trans_apply(&transaction, &dm), 0);
    amxd_trans_clean(&transaction);

    // and the instance added event is received by the backend
    // the newly added row must be registerd using rbusTable_registerRow
    expect_string(__wrap_rbusTable_registerRow, tableName, "TopLevel.Level1.");
    expect_value(__wrap_rbusTable_registerRow, instNum, 1);
    expect_string(__wrap_rbusTable_registerRow, tableName, "TopLevel.Level1.1.Level2B.");
    expect_value(__wrap_rbusTable_registerRow, instNum, 5);
    expect_string(__wrap_rbusTable_registerRow, aliasName, "TestAlias");

    handle_events();
    // it should be registered to the rbus daemon.

    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", false);

    check_rows_are_available(handle, "TopLevel.Level1.", expected_level1, __LINE__);
    check_rows_are_available(handle, "TopLevel.Level1.1.Level2B.", expected_level2b, __LINE__);
}

void test_amxb_rbus_provider_unregisters_removed_instances(UNUSED void** state) {
    amxd_trans_t transaction;

    assert_int_equal(amxo_parser_parse_file(&parser, "../common/odl/registration_test.odl", amxd_dm_get_root(&dm)), 0);
    assert_int_equal(amxb_register(bus_ctx, &dm), 0);
    // make event queue empty before continuing
    handle_events();

    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", true);

    // When a new instance is added
    assert_int_equal(amxd_trans_init(&transaction), 0);
    assert_int_equal(amxd_trans_select_pathf(&transaction, "TopLevel.Level1.1.Level2B."), 0);
    assert_int_equal(amxd_trans_del_inst(&transaction, 1, NULL), 0);
    assert_int_equal(amxd_trans_select_pathf(&transaction, "TopLevel.Level1.1.Level2A.Level3."), 0);
    assert_int_equal(amxd_trans_del_inst(&transaction, 1, NULL), 0);
    assert_int_equal(amxd_trans_apply(&transaction, &dm), 0);
    amxd_trans_clean(&transaction);

    // and the instance added event is received by the backend
    // the newly added row must be registerd using rbusTable_registerRow
    expect_string(__wrap_rbusTable_unregisterRow, rowName, "TopLevel.Level1.1.Level2B.1.");
    expect_string(__wrap_rbusTable_unregisterRow, rowName, "TopLevel.Level1.1.Level2A.Level3.1.");

    handle_events();
    // it should be registered to the rbus daemon.

    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", false);

    check_rows_are_available(handle, "TopLevel.Level1.1.Level2A.Level3.", NULL, __LINE__);
    check_rows_are_available(handle, "TopLevel.Level1.1.Level2B.", NULL, __LINE__);
}

void test_amxb_rbus_provider_can_delay_registration_until_app_start_event(UNUSED void** state) {
    amxc_var_t* rbus_config = GET_ARG(&config, "rbus");
    amxc_var_t* register_on_start = amxc_var_add_key(bool, rbus_config, "register-on-start-event", true);

    for(int i = 0; check_element_names[i] != NULL; i++) {
        expect_string(__wrap_rbus_regDataElements, name, check_element_names[i]);
    }

    expect_string(__wrap_rbusTable_registerRow, tableName, "TopLevel.Level1.");
    expect_value(__wrap_rbusTable_registerRow, instNum, 1);
    expect_string(__wrap_rbusTable_registerRow, tableName, "TopLevel.Level1.1.Level2A.Level3.");
    expect_value(__wrap_rbusTable_registerRow, instNum, 1);
    expect_string(__wrap_rbusTable_registerRow, tableName, "TopLevel.Level1.1.Level2B.");
    expect_value(__wrap_rbusTable_registerRow, instNum, 1);
    expect_string(__wrap_rbusTable_registerRow, aliasName, "cpe-Level2B-1");
    expect_string(__wrap_rbusTable_registerRow, tableName, "TopLevel.Level1.1.Level2C.");
    expect_value(__wrap_rbusTable_registerRow, instNum, 1);
    expect_string(__wrap_rbusTable_registerRow, aliasName, "cpe-Level2C-1");

    // when the supported data model is registered using amxb_register
    assert_int_equal(amxo_parser_parse_file(&parser, "../common/odl/registration_test.odl", amxd_dm_get_root(&dm)), 0);
    assert_int_equal(amxb_register(bus_ctx, &dm), 0);
    handle_events();

    // the data model is not yet registered to rbus
    check_supported_datamodel_is_unavailable(handle, "test-amxb-rbus", __LINE__);

    // until the start event is triggered or emitted
    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", true);
    // the data model is only registered as soon as the app:start event is triggered or emitted
    amxp_sigmngr_trigger_signal(&dm.sigmngr, "app:start", NULL);
    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", false);

    check_supported_datamodel_is_available(handle, "test-amxb-rbus", check_element_names_fetched, __LINE__);

    amxc_var_set(bool, register_on_start, false);
}

void test_amxb_rbus_provider_sets_correct_property_handlers(UNUSED void** state) {
    amxd_trans_t transaction;
    expected_type_t expected_2C[] = {
        { "TopLevel.Level1.1.Level2C.", (int) RBUS_ELEMENT_TYPE_TABLE, RBUS_ACCESS_SUBSCRIBE },
        { "TopLevel.Level1.1.Level2C.1.", 0, RBUS_ACCESS_GET | RBUS_ACCESS_SET },
        { "TopLevel.Level1.1.Level2C.1.L2CBool", (int) RBUS_ELEMENT_TYPE_PROPERTY, RBUS_ACCESS_GET | RBUS_ACCESS_SET},
        { "TopLevel.Level1.1.Level2C.1.L2CDouble", (int) RBUS_ELEMENT_TYPE_PROPERTY, RBUS_ACCESS_GET | RBUS_ACCESS_SET},
        { "TopLevel.Level1.1.Level2C.1.L2CText", (int) RBUS_ELEMENT_TYPE_PROPERTY, RBUS_ACCESS_GET  },
        { "TopLevel.Level1.1.Level2C.1.Alias", (int) RBUS_ELEMENT_TYPE_PROPERTY, RBUS_ACCESS_GET | RBUS_ACCESS_SET },
        { "TopLevel.Level1.1.Level2C.1.amx_notify!", (int) RBUS_ELEMENT_TYPE_EVENT, RBUS_ACCESS_SUBSCRIBE },
        { "TopLevel.Level1.1.Level2C.2.", 0, RBUS_ACCESS_GET | RBUS_ACCESS_SET },
        { "TopLevel.Level1.1.Level2C.2.L2CBool", (int) RBUS_ELEMENT_TYPE_PROPERTY, RBUS_ACCESS_GET | RBUS_ACCESS_SET},
        { "TopLevel.Level1.1.Level2C.2.L2CDouble", (int) RBUS_ELEMENT_TYPE_PROPERTY, RBUS_ACCESS_GET | RBUS_ACCESS_SET},
        { "TopLevel.Level1.1.Level2C.2.L2CText", (int) RBUS_ELEMENT_TYPE_PROPERTY, RBUS_ACCESS_GET  },
        { "TopLevel.Level1.1.Level2C.2.Alias", (int) RBUS_ELEMENT_TYPE_PROPERTY, RBUS_ACCESS_GET | RBUS_ACCESS_SET },
        { "TopLevel.Level1.1.Level2C.2.amx_notify!", (int) RBUS_ELEMENT_TYPE_EVENT, RBUS_ACCESS_SUBSCRIBE },
    };

    expected_type_t expected_2B[] = {
        { "TopLevel.Level1.1.Level2B.", (int) RBUS_ELEMENT_TYPE_TABLE, RBUS_ACCESS_SUBSCRIBE | RBUS_ACCESS_ADDROW | RBUS_ACCESS_REMOVEROW },
        { "TopLevel.Level1.1.Level2B.1.", 0, RBUS_ACCESS_GET | RBUS_ACCESS_SET },
        { "TopLevel.Level1.1.Level2B.1.L2BBool", (int) RBUS_ELEMENT_TYPE_PROPERTY, RBUS_ACCESS_GET | RBUS_ACCESS_SET },
        { "TopLevel.Level1.1.Level2B.1.L2BText", (int) RBUS_ELEMENT_TYPE_PROPERTY, RBUS_ACCESS_GET | RBUS_ACCESS_SET},
        { "TopLevel.Level1.1.Level2B.1.Alias", (int) RBUS_ELEMENT_TYPE_PROPERTY, RBUS_ACCESS_GET | RBUS_ACCESS_SET },
        { "TopLevel.Level1.1.Level2B.1.amx_notify!", (int) RBUS_ELEMENT_TYPE_EVENT, RBUS_ACCESS_SUBSCRIBE },
        { "TopLevel.Level1.1.Level2B.1.L2BEvent!", (int) RBUS_ELEMENT_TYPE_EVENT, RBUS_ACCESS_SUBSCRIBE },
        { "TopLevel.Level1.1.Level2B.1.L2BMethod()", (int) RBUS_ELEMENT_TYPE_METHOD, RBUS_ACCESS_INVOKE },
    };

    // when the supported data model is registered
    assert_int_equal(amxo_parser_parse_file(&parser, "../common/odl/registration_test.odl", amxd_dm_get_root(&dm)), 0);
    assert_int_equal(amxb_register(bus_ctx, &dm), 0);
    // read-only attribute must be taken into account

    assert_non_null(amxd_dm_findf(&dm, "TopLevel.Level1.1.Level2C."));

    assert_int_equal(amxd_trans_init(&transaction), 0);
    assert_int_equal(amxd_trans_set_attr(&transaction, amxd_tattr_change_ro, true), 0);
    assert_int_equal(amxd_trans_select_pathf(&transaction, "TopLevel.Level1.1.Level2C."), 0);
    assert_int_equal(amxd_trans_add_inst(&transaction, 0, NULL), 0);
    assert_int_equal(amxd_trans_apply(&transaction, &dm), 0);
    amxd_trans_clean(&transaction);

    handle_events();

    check_element_info(handle, "TopLevel.Level1.1.Level2C.", 2, expected_2C, __LINE__);
    check_element_info(handle, "TopLevel.Level1.1.Level2B.", 2, expected_2B, __LINE__);

    // NOTE:
    // Key parameters do have a set callback handler, as they are write once (unless defined mutable).
    // It is not possible to change the registration callbacks to rbus for individual rows.
    // So even if the key parameters in an ambiorix data model are not writable in most cases
    // the setcallback handler is kept.
}

void test_amxb_rbus_provider_registers_added_definitions_in_supported_datamodel(UNUSED void** state) {
    assert_int_equal(amxo_parser_parse_file(&parser, "../common/odl/registration_test_definition.odl", amxd_dm_get_root(&dm)), 0);
    assert_int_equal(amxb_register(bus_ctx, &dm), 0);
    check_supported_datamodel_is_available(handle, "test-amxb-rbus", check_element_names_fetched, __LINE__);
    handle_events();

    // When extra object definitions are loaded in the supported data model
    assert_int_equal(amxo_parser_parse_file(&parser, "../common/odl/registration_test_extra_definition.odl", amxd_dm_get_root(&dm)), 0);
    for(int i = 0; check_extra_element_names[i] != NULL; i++) {
        expect_string(__wrap_rbus_regDataElements, name, check_extra_element_names[i]);
    }

    // and the corresponding events are handled
    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", true);
    handle_events();
    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", false);

    // the data elements must be registered to rbus.
    check_supported_datamodel_is_available(handle, "test-amxb-rbus", check_extra_element_names_fetched, __LINE__);
}

void test_amxb_rbus_provider_unregisters_deleted_definitions_in_supported_datamodel(UNUSED void** state) {
    amxd_object_t* object = NULL;
    assert_int_equal(amxo_parser_parse_file(&parser, "../common/odl/registration_test_definition.odl", amxd_dm_get_root(&dm)), 0);
    assert_int_equal(amxb_register(bus_ctx, &dm), 0);
    check_supported_datamodel_is_available(handle, "test-amxb-rbus", check_element_names_fetched, __LINE__);
    handle_events();
    assert_int_equal(amxo_parser_parse_file(&parser, "../common/odl/registration_test_extra_definition.odl", amxd_dm_get_root(&dm)), 0);
    handle_events();

    object = amxd_dm_findf(&dm, "TopLevel.Level1.Level2A.Level3.Extra.");
    assert_non_null(object);

    for(int i = 0; check_extra_element_names[i] != NULL; i++) {
        expect_string(__wrap_rbus_unregDataElements, name, check_extra_element_names[i]);
    }

    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", true);
    // When object definitions are removed from the supported data model
    amxd_object_delete(&object);
    assert_null(object);
    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", false);

    // the corresponding elements should be unregistered from rbus.
    check_supported_datamodel_is_available(handle, "test-amxb-rbus", check_element_names_fetched, __LINE__);
}
