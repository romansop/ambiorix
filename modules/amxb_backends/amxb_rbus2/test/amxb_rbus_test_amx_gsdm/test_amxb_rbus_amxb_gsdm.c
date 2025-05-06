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
#include "test_amxb_rbus_amxb_gsdm.h"

static amxb_bus_ctx_t* bus_ctx = NULL;
static amxc_var_t config;

int test_amx_setup(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_init(&config);
    amxc_var_set_type(&config, AMXC_VAR_ID_HTABLE);

    assert_int_equal(amxb_be_load("../mod-amxb-test-rbus.so"), 0);
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    // Note: rbus backend gsdm implementation always uses native rbus api and doesn't support
    // amx calls.
    amxc_var_add_key(bool, rbus_config, "use-amx-calls", false);
    assert_int_equal(amxb_set_config(&config), 0);

    printf("SETUP: Starting amxrt data model providers\n");
    system("amxrt -D ../common/odl/delete_test.odl ../common/odl/test_event_data_model.odl");
    system("amxrt -D ../common/odl/softwaremodules_translated.odl ../common/odl/gsdm_test.odl");
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

void test_amxb_rbus_get_supported_data_model(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_get_supported(bus_ctx, "A.", AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_FUNCTIONS, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_full_root_1.json");
    assert_int_equal(retval, amxd_status_ok);

    retval = amxb_get_supported(bus_ctx, "Device.", AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_FUNCTIONS, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_full_root_2.json");
    assert_int_equal(retval, amxd_status_ok);

    retval = amxb_get_supported(bus_ctx, "TopLevel.Level1.{i}.", AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_FUNCTIONS, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_full_object.json");
    assert_int_equal(retval, amxd_status_ok);

    retval = amxb_get_supported(bus_ctx, "TopLevel.Level1.1.", AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_FUNCTIONS, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_full_object.json");
    assert_int_equal(retval, amxd_status_ok);

    retval = amxb_get_supported(bus_ctx, "TopLevel.", AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_FUNCTIONS, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_full_root_3.json");
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_get_supported_data_model_first_level_only(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_get_supported(bus_ctx, "Device.", AMXB_FLAG_FIRST_LVL | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_FUNCTIONS, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_first_level_root_1.json");
    assert_int_equal(retval, amxd_status_ok);

    retval = amxb_get_supported(bus_ctx, "TopLevel.Level1.{i}.", AMXB_FLAG_FIRST_LVL | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_FUNCTIONS, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_first_level_object.json");
    assert_int_equal(retval, amxd_status_ok);

    retval = amxb_get_supported(bus_ctx, "TopLevel.Level1.1.", AMXB_FLAG_FIRST_LVL | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_FUNCTIONS, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_first_level_object.json");
    assert_int_equal(retval, amxd_status_ok);

    retval = amxb_get_supported(bus_ctx, "TopLevel.", AMXB_FLAG_FIRST_LVL | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_FUNCTIONS, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_first_level_root_2.json");
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_get_supported_data_model_objects_only(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_get_supported(bus_ctx, "Device.", 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_full_objects_only_root_1.json");
    assert_int_equal(retval, amxd_status_ok);

    retval = amxb_get_supported(bus_ctx, "TopLevel.Level1.{i}.", 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_full_objects_only_object.json");
    assert_int_equal(retval, amxd_status_ok);

    retval = amxb_get_supported(bus_ctx, "TopLevel.Level1.1.", 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_full_objects_only_object.json");
    assert_int_equal(retval, amxd_status_ok);

    retval = amxb_get_supported(bus_ctx, "TopLevel.", 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_full_objects_only_root_2.json");
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_amxb_rbus_get_supported_data_model_on_not_provided_object(UNUSED void** state) {
    int retval = 0;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_get_supported(bus_ctx, "FakeDevice.", 0, &ret, 5);
    assert_int_equal(retval, amxd_status_ok);
    retval = check_amx_response(&ret, "./verify_data/amxb_get_supported_empty_response.json");
    assert_int_equal(retval, amxd_status_ok);

    amxc_var_clean(&ret);
}
