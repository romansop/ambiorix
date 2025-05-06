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

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_transaction.h>
#include <amxb/amxb.h>
#include <amxo/amxo.h>

#include <amxut/amxut_util.h>
#include <amxut/amxut_verify.h>

#include "amxb_usp.h"
#include "test_amxb_usp_get_instances.h"
#include "test_amxb_usp_common.h"
#include "imtp_mock.h"

#define UNUSED __attribute__((unused))

static amxb_usp_t* usp_ctx = NULL;
static amxb_bus_ctx_t* bus_ctx = NULL;

int test_dm_setup(UNUSED void** state) {
    return test_common_setup(&usp_ctx);
}

int test_dm_teardown(UNUSED void** state) {
    return test_common_teardown(&usp_ctx);
}

int test_e2e_transl_setup(UNUSED void** state) {
    return test_common_e2e_transl_setup(&bus_ctx);
}

int test_e2e_transl_teardown(UNUSED void** state) {
    return test_common_e2e_transl_teardown(&bus_ctx);
}

void test_amxb_usp_read_get_instances(UNUSED void** state) {
    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_get_instances);
    will_return(__wrap_imtp_connection_read_frame, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    assert_int_equal(amxb_usp_read(usp_ctx), 0);
}

void test_amxb_usp_handle_get_instances_error(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t request;
    amxc_var_t* obj_paths = NULL;

    amxc_var_init(&request);
    amxc_var_set_type(&request, AMXC_VAR_ID_HTABLE);
    obj_paths = amxc_var_add_key(amxc_llist_t, &request, "obj_paths", NULL);
    amxc_var_add(cstring_t, obj_paths, "test_root.child.");

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_get_instances_new(usp_tx, &request), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    usp_rx->msg->body->msg_body_case = USP__BODY__MSG_BODY_ERROR;
    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    // Test sending error message in case of wrong USP message
    assert_int_equal(amxb_usp_handle_get_instances(usp_ctx, usp_rx), 0);

    amxc_var_clean(&request);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_amxb_usp_handle_get_instances_invalid(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t request;
    amxc_var_t* obj_paths = NULL;

    // Test invalid input arguments
    assert_int_equal(amxb_usp_handle_get_instances(NULL, NULL), -1);
    assert_int_equal(amxb_usp_handle_get_instances(usp_ctx, NULL), -1);

    amxc_var_init(&request);
    amxc_var_set_type(&request, AMXC_VAR_ID_HTABLE);
    obj_paths = amxc_var_add_key(amxc_llist_t, &request, "obj_paths", NULL);
    amxc_var_add(cstring_t, obj_paths, "Invalid.Path.");

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_get_instances_new(usp_tx, &request), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    // Test handling GetInst in case the invoke GetInst fails
    assert_int_equal(amxb_usp_handle_get_instances(usp_ctx, usp_rx), 0);

    amxc_var_clean(&request);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_amxb_usp_get_instances_request(UNUSED void** state) {
    amxc_var_t ret;
    int depth = 0;
    int access = 0;
    amxc_var_t* result = NULL;

    amxc_var_init(&ret);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_get_instances_resp);
    will_return(__wrap_imtp_connection_read_frame, 0);

    assert_int_equal(amxb_usp_get_instances(usp_ctx, "test_root.", "child.", depth, access, &ret, 1), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    result = GETP_ARG(&ret, "0.'test_root.child.1.'");
    assert_non_null(result);
    amxc_var_dump(result, STDOUT_FILENO);

    assert_true(test_verify_data(result, "Alias", "One"));

    imtp_mock_free_msg_id();
    amxc_var_clean(&ret);
}

void test_e2e_transl_gi(UNUSED void** state) {
    const char* object = "Device.MQTT.Client.";
    amxc_var_t ret;
    amxc_var_t* entry = NULL;

    amxc_var_init(&ret);
    amxb_get_instances(bus_ctx, object, UINT32_MAX, &ret, 5);
    amxc_var_dump(&ret, STDOUT_FILENO);

    entry = GETP_ARG(&ret, "0.'Device.MQTT.Client.1.'");
    assert_non_null(entry);
    assert_true(test_verify_data(entry, "Alias", "test-client-1"));

    entry = GETP_ARG(&ret, "0.'Device.MQTT.Client.2.'");
    assert_non_null(entry);
    assert_true(test_verify_data(entry, "Alias", "test-client-2"));

    amxc_var_clean(&ret);
}

void test_obuspa_get_instances(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.History.";
    const char* results_file = "./results/obuspa_get_instances.json";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_instances(test_get_bus_ctx(), path, INT32_MAX, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&ret);
}

void test_obuspa_get_instances_depth_0(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.History.";
    const char* results_file = "./results/obuspa_get_instances_depth_0.json";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_instances(test_get_bus_ctx(), path, 0, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&ret);
}

void test_obuspa_get_instances_search_path(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.History.[From==\"Storyteller\"].Info.";
    const char* results_file = "./results/obuspa_get_instances_search_path.json";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_instances(test_get_bus_ctx(), path, 0, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&ret);
}

void test_obuspa_get_instances_singleton(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_instances(test_get_bus_ctx(), path, 0, &ret, 5);
    // Currently returns 0, but should return:
    // 7018 | Object is not a table | Add, GetInstances | This error indicates that the requested Path Name is not a Multi-Instance Object.
    // https://usp.technology/specification/#sec:getinstances
    // https://usp.technology/specification/#sec:error-codes
    //assert_int_equal(retval, amxd_status_not_a_template);
    printf("amxb_get_instances returns %d, expected %d\n", retval, amxd_status_not_a_template);
    fflush(stdout);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&ret);
}

void test_obuspa_get_instances_invalid(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeters.";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_instances(test_get_bus_ctx(), path, 0, &ret, 5);
    assert_int_equal(retval, amxd_status_invalid_path);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&ret);
}

void test_obuspa_get_instances_invalid_sub_object(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.Invalid.";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_instances(test_get_bus_ctx(), path, 0, &ret, 5);
    assert_int_equal(retval, amxd_status_invalid_path);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&ret);
}

void test_obuspa_get_instances_invalid_parameter_path(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.State";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_instances(test_get_bus_ctx(), path, 0, &ret, 5);
    // Requires changes to lib_usp
    //assert_int_equal(retval, amxd_status_invalid_path);
    assert_int_not_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&ret);
}
