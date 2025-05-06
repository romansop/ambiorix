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
#include "test_amxb_rbus_amxb_delete.h"

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

void test_amxb_rbus_delete_single_instance(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(bus_ctx, "TopLevel.Level1.1.Level2B.", 1, NULL, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 1);
    assert_string_equal(GETI_CHAR(&ret, 0), "TopLevel.Level1.1.Level2B.1.");
    retval = amxb_get(bus_ctx, "TopLevel.Level1.1.Level2B.1.", 0, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    retval = amxb_del(bus_ctx, "TopLevel.Level1.1.Level2B.", 0, "cpe-alias-2", &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 1);
    assert_string_equal(GETI_CHAR(&ret, 0), "TopLevel.Level1.1.Level2B.2.");
    retval = amxb_get(bus_ctx, "TopLevel.Level1.1.Level2B.2.", 0, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    retval = amxb_del(bus_ctx, "Device.SoftwareModules.ExecEnv.", 1, NULL, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 1);
    assert_string_equal(GETI_CHAR(&ret, 0), "Device.SoftwareModules.ExecEnv.1.");
    retval = amxb_get(bus_ctx, "Device.SoftwareModules.ExecEnv.1.", 0, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_delete_all_instances(UNUSED void** state) {
    int retval = 0;
    amxc_var_t verify;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&verify);

    retval = amxb_del(bus_ctx, "TopLevel.Level1.*.Level2B.*", 0, NULL, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 6);
    assert_string_equal(GETI_CHAR(&ret, 0), "TopLevel.Level1.1.Level2B.3.");
    assert_string_equal(GETI_CHAR(&ret, 1), "TopLevel.Level1.1.Level2B.4.");
    assert_string_equal(GETI_CHAR(&ret, 2), "TopLevel.Level1.2.Level2B.1.");
    assert_string_equal(GETI_CHAR(&ret, 3), "TopLevel.Level1.2.Level2B.2.");
    assert_string_equal(GETI_CHAR(&ret, 4), "TopLevel.Level1.2.Level2B.3.");
    assert_string_equal(GETI_CHAR(&ret, 5), "TopLevel.Level1.2.Level2B.4.");

    amxc_var_for_each(path, &ret) {
        retval = amxb_get(bus_ctx, GET_CHAR(path, NULL), 0, &verify, 5);
        assert_int_not_equal(retval, amxd_status_ok);
    }

    amxc_var_clean(&verify);
    amxc_var_clean(&ret);
}

void test_amxb_rbus_delete_no_instances_found(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(bus_ctx, "TopLevel.Level1.*.Level2B.*", 0, NULL, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    retval = amxb_del(bus_ctx, "TopLevel.Level1.*.Level2B.", 5, NULL, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    retval = amxb_del(bus_ctx, "TopLevel.Level1.*.Level2B.", 0, "cpe-alias-5", &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_delete_no_index_no_alias(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(bus_ctx, "TopLevel.Level1.*.Level2B.", 0, NULL, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    retval = amxb_del(bus_ctx, "TopLevel.Level1.1.Level2B.", 0, NULL, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_deletion_fails(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(bus_ctx, "TopLevel.Level1.2.Level2C.*", 0, NULL, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);
    // This will behave differently when using amx-calls or not.
    // When using RBus native calls, it is possible that some instances are deleted
    // This is currently not covered in the test.

    amxc_var_clean(&ret);
}

void test_amxb_rbus_all_removed_subobjects_are_returned(UNUSED void** state) {
    int retval = 0;
    amxc_var_t* amx_calls = GETP_ARG(&config, "rbus.use-amx-calls");
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(bus_ctx, "TopLevel.Level1.[L1UnsignedNumber>20].", 0, NULL, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);

    // RBus is not returning the table (multi-instance) objects.
    if(GET_BOOL(amx_calls, NULL)) {
        assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
        assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 8);
        assert_string_equal(GETI_CHAR(&ret, 0), "TopLevel.Level1.3.");
        assert_string_equal(GETI_CHAR(&ret, 1), "TopLevel.Level1.3.Level2A.");
        assert_string_equal(GETI_CHAR(&ret, 2), "TopLevel.Level1.3.Level2A.Level3.");
        assert_string_equal(GETI_CHAR(&ret, 3), "TopLevel.Level1.3.Level2A.Level3.1.");
        assert_string_equal(GETI_CHAR(&ret, 4), "TopLevel.Level1.3.Level2A.Level3.2.");
        assert_string_equal(GETI_CHAR(&ret, 5), "TopLevel.Level1.3.Level2A.Level3.3.");
        assert_string_equal(GETI_CHAR(&ret, 6), "TopLevel.Level1.3.Level2B.");
        assert_string_equal(GETI_CHAR(&ret, 7), "TopLevel.Level1.3.Level2C.");
    } else {
        assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
        assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 5);
        assert_string_equal(GETI_CHAR(&ret, 0), "TopLevel.Level1.3.");
        assert_string_equal(GETI_CHAR(&ret, 1), "TopLevel.Level1.3.Level2A.");
        assert_string_equal(GETI_CHAR(&ret, 2), "TopLevel.Level1.3.Level2A.Level3.1.");
        assert_string_equal(GETI_CHAR(&ret, 3), "TopLevel.Level1.3.Level2A.Level3.2.");
        assert_string_equal(GETI_CHAR(&ret, 4), "TopLevel.Level1.3.Level2A.Level3.3.");
    }

    amxc_var_clean(&ret);
}

void test_amxb_rbus_del_instance_on_not_existing_table(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(bus_ctx, "TopLevel.NotExisting.", 1, NULL, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_del_instance_on_non_table(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(bus_ctx, "TopLevel.Level1.1.Level2A.", 1, NULL, &ret, 5);
    assert_int_not_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}