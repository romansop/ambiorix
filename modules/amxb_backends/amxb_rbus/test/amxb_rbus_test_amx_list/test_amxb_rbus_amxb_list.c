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
#include "test_amxb_rbus_amxb_list.h"

static amxb_bus_ctx_t* bus_ctx = NULL;
static amxc_var_t config;
static const uint32_t flags_all = (AMXB_FLAG_EVENTS |
                                   AMXB_FLAG_FUNCTIONS |
                                   AMXB_FLAG_PARAMETERS |
                                   AMXB_FLAG_OBJECTS |
                                   AMXB_FLAG_INSTANCES);

static void amxb_rbus_verify_list(const amxb_bus_ctx_t* ctx,
                                  const amxc_var_t* const data,
                                  void* priv) {
    amxc_var_t* items = (amxc_var_t*) priv;
    assert_ptr_equal(ctx, bus_ctx);
    amxc_var_for_each(item, data) {
        amxc_var_add(cstring_t, items, GET_CHAR(item, NULL));
    }
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
    system("amxrt -D ../common/odl/delete_test.odl");
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
    system("amxrt -D ../common/odl/delete_test.odl");
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

void test_amxb_rbus_list_root(UNUSED void** state) {
    amxc_var_t ret;
    const char* items[] = { "TopLevel.", "Device." };
    int nr_of_items = sizeof(items) / sizeof(const char*);

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    assert_int_equal(amxb_list(bus_ctx, "", flags_all, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items[i], GETI_CHAR(&ret, i));
    }

    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);
    assert_int_equal(amxb_list(bus_ctx, NULL, flags_all, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items[i], GETI_CHAR(&ret, i));
    }

    amxc_var_clean(&ret);
}

void test_amxb_rbus_list_root_object(UNUSED void** state) {
    amxc_var_t ret;
    const char* items[] = { "Device.SoftwareModules.", "Device.TestDescribe." };
    int nr_of_items = sizeof(items) / sizeof(const char*);

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    assert_int_equal(amxb_list(bus_ctx, "Device.", flags_all, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items[i], GETI_CHAR(&ret, i));
    }
    amxc_var_clean(&ret);
}


void test_amxb_rbus_list_object(UNUSED void** state) {
    amxc_var_t ret;
    const char* items[] = { "Device.SoftwareModules.ExecEnvClassNumberOfEntries",
        "Device.SoftwareModules.amx_notify!",
        "Device.SoftwareModules.ExecEnvClass.",
        "Device.SoftwareModules.ExecEnv.", };
    int nr_of_items = sizeof(items) / sizeof(const char*);

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    assert_int_equal(amxb_list(bus_ctx, "Device.SoftwareModules.", flags_all, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items[i], GETI_CHAR(&ret, i));
    }

    amxc_var_clean(&ret);
}

void test_amxb_rbus_list_instances(UNUSED void** state) {
    amxc_var_t ret;
    const char* items[] = { "TopLevel.Level1.1.Level2B.1.",
        "TopLevel.Level1.1.Level2B.2.",
        "TopLevel.Level1.1.Level2B.3.",
        "TopLevel.Level1.1.Level2B.4.", };
    int nr_of_items = sizeof(items) / sizeof(const char*);

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    assert_int_equal(amxb_list(bus_ctx, "TopLevel.Level1.1.Level2B.", flags_all, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items[i], GETI_CHAR(&ret, i));
    }

    amxc_var_clean(&ret);
}

void test_amxb_rbus_list_instance_items(UNUSED void** state) {
    amxc_var_t ret;
    const char* items[] = { "TopLevel.Level1.1.Level2B.1.L2BBool",
        "TopLevel.Level1.1.Level2B.1.L2BText",
        "TopLevel.Level1.1.Level2B.1.Alias",
        "TopLevel.Level1.1.Level2B.1.amx_notify!",
        "TopLevel.Level1.1.Level2B.1.L2BEvent!",
        "TopLevel.Level1.1.Level2B.1.L2BMethod()", };
    int nr_of_items = sizeof(items) / sizeof(const char*);

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    assert_int_equal(amxb_list(bus_ctx, "TopLevel.Level1.1.Level2B.1.", flags_all, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items[i], GETI_CHAR(&ret, i));
    }

    amxc_var_clean(&ret);
}

void test_amxb_rbus_list_instance_items_protected_access(UNUSED void** state) {
    amxc_var_t ret;
    const char* items[] = { "TopLevel.Level1.1.Level2B.1.L2BBool",
        "TopLevel.Level1.1.Level2B.1.L2BText",
        "TopLevel.Level1.1.Level2B.1.Alias",
        "TopLevel.Level1.1.Level2B.1.amx_notify!",
        "TopLevel.Level1.1.Level2B.1.L2BEvent!",
        "TopLevel.Level1.1.Level2B.1.L2BMethod()",
        "TopLevel.Level1.1.Level2B.1._list()",
        "TopLevel.Level1.1.Level2B.1._describe()",
        "TopLevel.Level1.1.Level2B.1._get()",
        "TopLevel.Level1.1.Level2B.1._get_instances()",
        "TopLevel.Level1.1.Level2B.1._get_supported()",
        "TopLevel.Level1.1.Level2B.1._set()",
        "TopLevel.Level1.1.Level2B.1._add()",
        "TopLevel.Level1.1.Level2B.1._del()",
        "TopLevel.Level1.1.Level2B.1._exec()"};
    int nr_of_items = sizeof(items) / sizeof(const char*);

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    amxb_set_access(bus_ctx, AMXB_PROTECTED);
    assert_int_equal(amxb_list(bus_ctx, "TopLevel.Level1.1.Level2B.1.", flags_all, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    amxb_set_access(bus_ctx, AMXB_PUBLIC);

    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items[i], GETI_CHAR(&ret, i));
    }

    amxc_var_clean(&ret);
}

void test_amxb_rbus_list_objects_instances_only(UNUSED void** state) {
    amxc_var_t ret;
    const char* items1[] = { "TopLevel.Level1.1.Level2A.Level3." };
    const char* items2[] = { "TopLevel.Level1.1.Level2A.Level3.1.",
        "TopLevel.Level1.1.Level2A.Level3.2.",
        "TopLevel.Level1.1.Level2A.Level3.3."};
    int nr_of_items = sizeof(items1) / sizeof(const char*);

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    assert_int_equal(amxb_list(bus_ctx, "TopLevel.Level1.1.Level2A.", AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items1[i], GETI_CHAR(&ret, i));
    }

    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    nr_of_items = sizeof(items2) / sizeof(const char*);
    assert_int_equal(amxb_list(bus_ctx, "TopLevel.Level1.1.Level2A.Level3.", AMXB_FLAG_OBJECTS | AMXB_FLAG_INSTANCES, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items2[i], GETI_CHAR(&ret, i));
    }

    amxc_var_clean(&ret);
}

void test_amxb_rbus_list_events_only(UNUSED void** state) {
    amxc_var_t ret;
    const char* items[] = { "TopLevel.Level1.1.Level2B.1.amx_notify!",
        "TopLevel.Level1.1.Level2B.1.L2BEvent!", };
    int nr_of_items = sizeof(items) / sizeof(const char*);

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    assert_int_equal(amxb_list(bus_ctx, "TopLevel.Level1.1.Level2B.1.", AMXB_FLAG_EVENTS, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items[i], GETI_CHAR(&ret, i));
    }

    amxc_var_clean(&ret);
}

void test_amxb_rbus_list_parameters_only(UNUSED void** state) {
    amxc_var_t ret;
    const char* items1[] = { "TopLevel.Level1.1.Level2B.2.L2BBool",
        "TopLevel.Level1.1.Level2B.2.L2BText",
        "TopLevel.Level1.1.Level2B.2.Alias", };
    const char* items2[] = { "TopLevel.Level1.1.Level2A.L2AText",
        "TopLevel.Level1.1.Level2A.L2ASignedNumber",
        "TopLevel.Level1.1.Level2A.L2AUnsignedNumber", };
    int nr_of_items = sizeof(items1) / sizeof(const char*);

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    assert_int_equal(amxb_list(bus_ctx, "TopLevel.Level1.1.Level2B.2.", AMXB_FLAG_PARAMETERS, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items1[i], GETI_CHAR(&ret, i));
    }

    nr_of_items = sizeof(items2) / sizeof(const char*);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);
    assert_int_equal(amxb_list(bus_ctx, "TopLevel.Level1.1.Level2A.", AMXB_FLAG_PARAMETERS, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items2[i], GETI_CHAR(&ret, i));
    }

    amxc_var_clean(&ret);
}

void test_amxb_rbus_list_methods_only(UNUSED void** state) {
    amxc_var_t ret;
    const char* items[] = { "TopLevel.Level1.1.Level2B.1.L2BMethod()", };
    int nr_of_items = sizeof(items) / sizeof(const char*);

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    assert_int_equal(amxb_list(bus_ctx, "TopLevel.Level1.1.Level2B.1.", AMXB_FLAG_FUNCTIONS, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), nr_of_items);
    for(int i = 0; i < nr_of_items; i++) {
        assert_string_equal(items[i], GETI_CHAR(&ret, i));
    }

    amxc_var_clean(&ret);
}

void test_amxb_rbus_list_non_existing_object(UNUSED void** state) {
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    assert_int_equal(amxb_list(bus_ctx, "Device.SoftwareModules.NonExisting.", flags_all, amxb_rbus_verify_list, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 0);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_list_no_callback(UNUSED void** state) {
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_LIST);

    assert_int_equal(amxb_list(bus_ctx, "Device.", flags_all, NULL, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 0);

    assert_int_equal(amxb_list(bus_ctx, "", flags_all, NULL, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 0);

    amxc_var_clean(&ret);
}
