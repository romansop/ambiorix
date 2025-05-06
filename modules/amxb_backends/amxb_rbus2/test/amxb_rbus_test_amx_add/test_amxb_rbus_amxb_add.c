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
#include "test_amxb_rbus_amxb_add.h"

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
    system("amxrt -D ../common/odl/add_test.odl");
    system("amxrt -D ../common/odl/softwaremodules_translated.odl");
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
    system("amxrt -D ../common/odl/add_test.odl");
    system("amxrt -D ../common/odl/softwaremodules_translated.odl");
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

void test_amxb_rbus_add_instance(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_add(bus_ctx, "TopLevel.Level1.", 0, NULL, NULL, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_var_type_of(GETI_ARG(&ret, 0)), AMXC_VAR_ID_HTABLE);

    // verify all items are returned
    assert_non_null(GETP_ARG(&ret, "0.index"));
    assert_non_null(GETP_ARG(&ret, "0.name"));
    assert_non_null(GETP_ARG(&ret, "0.path"));
    assert_non_null(GETP_ARG(&ret, "0.object"));
    assert_non_null(GETP_ARG(&ret, "0.parameters"));
    // expected index 1
    assert_int_equal(GETP_UINT32(&ret, "0.index"), 1);

    // Check the newly added row can be retrieved
    retval = amxb_get(bus_ctx, GETP_CHAR(&ret, "0.path"), 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_add_instance.json");
    assert_int_equal(retval, amxd_status_ok);

    // Check that translation is correctly done
    retval = amxb_add(bus_ctx, "Device.SoftwareModules.ExecEnv.", 0, NULL, NULL, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_var_type_of(GETI_ARG(&ret, 0)), AMXC_VAR_ID_HTABLE);

    // verify all items are returned
    assert_non_null(GETP_ARG(&ret, "0.index"));
    assert_non_null(GETP_ARG(&ret, "0.name"));
    assert_non_null(GETP_ARG(&ret, "0.path"));
    assert_non_null(GETP_ARG(&ret, "0.object"));
    assert_non_null(GETP_ARG(&ret, "0.parameters"));
    // expected index 3
    assert_int_equal(GETP_UINT32(&ret, "0.index"), 3);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_add_instance_request_index(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");
    const char* verify_data = "./verify_data/amxb_add_instance request_index.json";

    amxc_var_init(&ret);

    retval = amxb_add(bus_ctx, "TopLevel.Level1.", 5, NULL, NULL, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_var_type_of(GETI_ARG(&ret, 0)), AMXC_VAR_ID_HTABLE);

    // verify all items are returned
    assert_non_null(GETP_ARG(&ret, "0.index"));
    assert_non_null(GETP_ARG(&ret, "0.name"));
    assert_non_null(GETP_ARG(&ret, "0.path"));
    assert_non_null(GETP_ARG(&ret, "0.object"));
    assert_non_null(GETP_ARG(&ret, "0.parameters"));

    // When using rbus api's it will not be possible to request a specific index
    // It will use the automatic indexing
    if(GET_BOOL(amx_calls, NULL)) {
        // expected index 5
        assert_int_equal(GETP_UINT32(&ret, "0.index"), 5);
    } else {
        // when not using amx-calls the next index will be returned, in this case 2
        assert_int_equal(GETP_UINT32(&ret, "0.index"), 2);
        verify_data = "./verify_data/amxb_add_instance request_index rbus.json";
    }

    // Check the newly added row can be retrieved
    retval = amxb_get(bus_ctx, GETP_CHAR(&ret, "0.path"), 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, verify_data);
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_add_instance_and_set_param_values(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;
    amxc_var_t params;

    amxc_var_init(&ret);
    amxc_var_init(&params);
    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "L3Text", "Added instances");
    amxc_var_add_key(uint64_t, &params, "L3UnsignedNumber", 1000);
    amxc_var_add_key(int64_t, &params, "L3SignedNumber", -1000);

    retval = amxb_add(bus_ctx, "TopLevel.Level1.1.Level2A.Level3.", 0, NULL, &params, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_var_type_of(GETI_ARG(&ret, 0)), AMXC_VAR_ID_HTABLE);

    // verify all items are returned
    assert_non_null(GETP_ARG(&ret, "0.index"));
    assert_non_null(GETP_ARG(&ret, "0.name"));
    assert_non_null(GETP_ARG(&ret, "0.path"));
    assert_non_null(GETP_ARG(&ret, "0.object"));
    assert_non_null(GETP_ARG(&ret, "0.parameters"));
    // expected index 1
    assert_int_equal(GETP_UINT32(&ret, "0.index"), 1);

    // Check the newly added row can be retrieved
    retval = amxb_get(bus_ctx, GETP_CHAR(&ret, "0.path"), 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_add_instance with_params.json");
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
    amxc_var_clean(&params);
}

void test_amxb_rbus_add_instances_using_wildcard_path(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;
    amxc_var_t params;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    amxc_var_init(&ret);
    amxc_var_init(&params);
    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "L3Text", "Added instances");
    amxc_var_add_key(uint64_t, &params, "L3UnsignedNumber", 1000);
    amxc_var_add_key(int64_t, &params, "L3SignedNumber", -1000);


    retval = amxb_add(bus_ctx, "TopLevel.Level1.*.Level2A.Level3.", 0, NULL, &params, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    // expecting to get a list containing all added instances, in this case 2 as
    // two instances exist for TopLevel.Level1.
    assert_int_equal(amxc_var_type_of(GETI_ARG(&ret, 0)), AMXC_VAR_ID_LIST);

    if(GET_BOOL(amx_calls, NULL)) {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instance_wildcard_path_verify_add.json");
    } else {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instance_wildcard_path_verify_add_rbus.json");
    }
    assert_int_equal(retval, amxd_status_ok);

    // Check the newly added row can be retrieved
    retval = amxb_get(bus_ctx, "TopLevel.Level1.*.Level2A.Level3.*.", 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    if(GET_BOOL(amx_calls, NULL)) {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instances_using_wildcard_path.json");
    } else {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instances_using_wildcard_path_rbus.json");
    }
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
    amxc_var_clean(&params);
}

void test_amxb_rbus_add_instances_using_search_path(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;
    amxc_var_t params;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    amxc_var_init(&ret);
    amxc_var_init(&params);
    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "L3Text", "Added instances");
    amxc_var_add_key(uint64_t, &params, "L3UnsignedNumber", 1000);
    amxc_var_add_key(int64_t, &params, "L3SignedNumber", -1000);


    retval = amxb_add(bus_ctx, "TopLevel.Level1.[L1UnsignedNumber==0].Level2A.Level3.", 0, NULL, &params, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_var_type_of(GETI_ARG(&ret, 0)), AMXC_VAR_ID_LIST);

    if(GET_BOOL(amx_calls, NULL)) {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instance_search_path_verify_add.json");
    } else {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instance_search_path_verify_add_rbus.json");
    }
    assert_int_equal(retval, amxd_status_ok);

    // Check the newly added row can be retrieved
    retval = amxb_get(bus_ctx, "TopLevel.Level1.*.Level2A.Level3.*.", 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    if(GET_BOOL(amx_calls, NULL)) {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instances_using_search_path.json");
    } else {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instances_using_search_path_rbus.json");
    }
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
    amxc_var_clean(&params);
}

void test_amxb_rbus_add_instances_using_search_path_no_matches(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    amxc_var_init(&ret);

    retval = amxb_add(bus_ctx, "TopLevel.Level1.[L1UnsignedNumber>100].Level2A.Level3.", 0, NULL, NULL, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);

    // Check that no rows has been added
    retval = amxb_get(bus_ctx, "TopLevel.Level1.*.Level2A.Level3.*.", 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    if(GET_BOOL(amx_calls, NULL)) {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instances_using_search_path.json");
    } else {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instances_using_search_path_rbus.json");
    }
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_add_instance_with_invalid_values(UNUSED void** state) {
    int retval = 0;
    amxc_var_t params;
    amxc_var_t ret;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    amxc_var_init(&ret);
    amxc_var_init(&params);
    amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &params, "L3Text", "Added instances");
    amxc_var_add_key(uint64_t, &params, "L3UnsignedNumber", 200000);
    amxc_var_add_key(int64_t, &params, "L3SignedNumber", 20000);

    retval = amxb_add(bus_ctx, "TopLevel.Level1.*.Level2A.Level3.", 0, NULL, &params, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    // Check the no rows were added
    retval = amxb_get(bus_ctx, "TopLevel.Level1.*.Level2A.Level3.*.", 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    if(GET_BOOL(amx_calls, NULL)) {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instances_using_search_path.json");
    } else {
        retval = check_amx_response(&ret, "./verify_data/amxb_add_instances_using_search_path_rbus.json");
    }
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
    amxc_var_clean(&params);
}

void test_amxb_rbus_add_instance_with_alias(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_add(bus_ctx, "TopLevel.Level1.1.Level2B.", 0, "cpe-test-alias-a", NULL, &ret, 5);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(retval, amxd_status_ok);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_var_type_of(GETI_ARG(&ret, 0)), AMXC_VAR_ID_HTABLE);

    // verify all items are returned
    assert_non_null(GETP_ARG(&ret, "0.index"));
    assert_non_null(GETP_ARG(&ret, "0.name"));
    assert_non_null(GETP_ARG(&ret, "0.path"));
    assert_non_null(GETP_ARG(&ret, "0.object"));
    assert_non_null(GETP_ARG(&ret, "0.parameters"));
    // expected index 1
    assert_int_equal(GETP_UINT32(&ret, "0.index"), 1);
    // expected name cpe-test-alias-a
    assert_string_equal(GETP_CHAR(&ret, "0.name"), "cpe-test-alias-a");

    // Check the newly added row can be retrieved
    retval = amxb_get(bus_ctx, GETP_CHAR(&ret, "0.path"), 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_add_instance_with_alias.json");
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_add_instance_with_empty_alias(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");

    amxc_var_init(&ret);

    retval = amxb_add(bus_ctx, "TopLevel.Level1.1.Level2B.", 0, "", NULL, &ret, 5);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(retval, amxd_status_ok);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_var_type_of(GETI_ARG(&ret, 0)), AMXC_VAR_ID_HTABLE);

    // verify all items are returned
    assert_non_null(GETP_ARG(&ret, "0.index"));
    assert_non_null(GETP_ARG(&ret, "0.name"));
    assert_non_null(GETP_ARG(&ret, "0.path"));
    assert_non_null(GETP_ARG(&ret, "0.object"));
    assert_non_null(GETP_ARG(&ret, "0.parameters"));
    // expected index 2
    assert_int_equal(GETP_UINT32(&ret, "0.index"), 2);
    // expected name "2" when using RBUS-API or "" when using amx calls
    if(GET_BOOL(amx_calls, NULL)) {
        assert_string_equal(GETP_CHAR(&ret, "0.name"), "cpe-Level2B-2");
    } else {
        assert_string_equal(GETP_CHAR(&ret, "0.name"), "2");
    }

    amxc_var_clean(&ret);
}

void test_amxb_rbus_add_instance_with_alias_duplicate(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_add(bus_ctx, "TopLevel.Level1.1.Level2B.", 0, "cpe-test-alias-a", NULL, &ret, 5);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_not_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_add_instance_on_not_existing_table(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_add(bus_ctx, "TopLevel.NotExisting.", 0, NULL, NULL, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_add_instance_on_non_table(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_add(bus_ctx, "TopLevel.Level1.1.", 0, NULL, NULL, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    retval = amxb_add(bus_ctx, "TopLevel.Level1.1.Level2A.", 0, NULL, NULL, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}
