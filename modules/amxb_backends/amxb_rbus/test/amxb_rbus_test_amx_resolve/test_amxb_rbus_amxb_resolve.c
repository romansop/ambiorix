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
#include "test_amxb_rbus_amxb_resolve.h"

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
    system("amxrt -D ../common/odl/method_calls.odl");
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

void test_amxb_rbus_get_bus_ctx_using_who_has(UNUSED void** state) {
    amxc_var_t ret;
    amxb_bus_ctx_t* ctx = amxb_be_who_has("TopLevel.Level1.1.Level2A.Level3.1.");
    assert_ptr_equal(ctx, bus_ctx);
    // amxb_be_who_has will do partial matching if full path is not found
    // So using an not existing instance index will give back the bus context as well.
    ctx = amxb_be_who_has("TopLevel.Level1.99.Level2A.Level3.1.");
    assert_ptr_equal(ctx, bus_ctx);
    // Check if object doesn't exist
    amxc_var_init(&ret);
    assert_int_not_equal(amxb_get(ctx, "TopLevel.Level1.99.Level2A.Level3.1.", 0, &ret, 5), 0);
    amxc_var_clean(&ret);

    ctx = amxb_be_who_has("Device.TestObject.");
    assert_ptr_equal(ctx, bus_ctx);
    // Two providers start the object tree with device
    ctx = amxb_be_who_has("Device.");
    assert_ptr_equal(ctx, bus_ctx);

    // amxb_be_who_has will return NULL if no partial match is found
    ctx = amxb_be_who_has("Dummy.FakeObject.");
    assert_null(ctx);
    // or when an empty or NULL string is passed
    ctx = amxb_be_who_has(NULL);
    assert_null(ctx);
    ctx = amxb_be_who_has("");
    assert_null(ctx);
}

void test_amxb_rbus_amxb_can_resolve_wildcard_paths(UNUSED void** state) {
    amxc_var_t ret;
    amxd_path_t path;
    amxc_var_init(&ret);

    amxd_path_init(&path, "Device.SoftwareModules.ExecEnv.*.");
    assert_int_equal(amxb_resolve(bus_ctx, &path, &ret), 0);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 2);
    assert_string_equal(GETI_CHAR(&ret, 0), "Device.SoftwareModules.ExecEnv.1.");
    assert_string_equal(GETI_CHAR(&ret, 1), "Device.SoftwareModules.ExecEnv.2.");
    amxc_var_clean(&ret);

    amxd_path_setf(&path, false, "TopLevel.Level1.*.Level2A.Level3.*.");
    assert_int_equal(amxb_resolve(bus_ctx, &path, &ret), 0);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 1);
    assert_string_equal(GETI_CHAR(&ret, 0), "TopLevel.Level1.1.Level2A.Level3.1.");
    amxc_var_clean(&ret);

    // Wildcards on non-instances should not be allowed
    // RBus accepts a wildcard at any place in the path, but that is not
    // correct according to the USP sepcifications.
    // Extra checks need to be done in the amxb_resolve function to prevent
    // this.
    amxd_path_setf(&path, false, "TopLevel.*.1.Level2A.Level3.*.");
    assert_int_not_equal(amxb_resolve(bus_ctx, &path, &ret), 0);
    amxc_var_clean(&ret);

    amxd_path_setf(&path, false, "Device.SoftwareModules.*.1.");
    assert_int_not_equal(amxb_resolve(bus_ctx, &path, &ret), 0);
    amxc_var_clean(&ret);

    amxd_path_clean(&path);
}

void test_amxb_rbus_amxb_can_resolve_wildcard_paths_without_verify_types(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_t* skip_verify = NULL;
    amxc_var_t ret;
    amxd_path_t path;
    amxc_var_init(&ret);

    // Disable element type verification for wildcard and search paths
    rbus_config = GET_ARG(&config, "rbus");
    skip_verify = amxc_var_add_key(bool, rbus_config, "skip-verify-type", true);

    // When skip-verify-type is set non-standard wildcard paths are allowed
    amxd_path_init(&path, "TopLevel.*.1.Level2A.Level3.*.");
    assert_int_equal(amxb_resolve(bus_ctx, &path, &ret), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    amxc_var_clean(&ret);

    amxc_var_delete(&skip_verify);
    amxd_path_clean(&path);
}

void test_amxb_rbus_amxb_can_resolve_search_paths(UNUSED void** state) {
    amxc_var_t ret;
    amxd_path_t path;
    amxc_var_init(&ret);

    amxd_path_init(&path, "Device.SoftwareModules.ExecEnvClass.[Capability.1.SpecificationVersion=='1.0.1'].");
    assert_int_equal(amxb_resolve(bus_ctx, &path, &ret), 0);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 1);
    assert_string_equal(GETI_CHAR(&ret, 0), "Device.SoftwareModules.ExecEnvClass.1.");
    amxc_var_clean(&ret);

    amxd_path_setf(&path, false, "Device.SoftwareModules.ExecEnvClass.[Capability.1.SpecificationVersion=='2.0.0'].");
    assert_int_equal(amxb_resolve(bus_ctx, &path, &ret), 0);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_NULL); // nothing found
    amxc_var_clean(&ret);

    amxd_path_setf(&path, false, "TopLevel.Level1.*.Level2A.Level3.[L3Text==''].");
    assert_int_equal(amxb_resolve(bus_ctx, &path, &ret), 0);
    assert_int_equal(amxc_var_type_of(&ret), AMXC_VAR_ID_LIST);
    assert_int_equal(amxc_llist_size(amxc_var_constcast(amxc_llist_t, &ret)), 1);
    assert_string_equal(GETI_CHAR(&ret, 0), "TopLevel.Level1.1.Level2A.Level3.1.");
    amxc_var_clean(&ret);

    // Search expressions on non-instances should not be allowed
    amxd_path_setf(&path, false, "TopLevel.[Test>100].1.Level2A.Level3.*.");
    assert_int_not_equal(amxb_resolve(bus_ctx, &path, &ret), 0);
    amxc_var_clean(&ret);

    amxd_path_setf(&path, false, "Device.SoftwareModules.[Test>100].1.");
    assert_int_not_equal(amxb_resolve(bus_ctx, &path, &ret), 0);
    amxc_var_clean(&ret);

    amxd_path_clean(&path);
}
