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
#include "test_amxb_usp_get_supported_dm.h"
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

int test_e2e_setup(UNUSED void** state) {
    assert_int_equal(system("amxrt -D ../amxb_usp_common/odl/test_config.odl ../amxb_usp_common/odl/tr181-mqtt_definition.odl"), 0);
    assert_int_equal(amxb_be_load("../mod-amxb-test-usp.so"), 0);
    assert_int_equal(amxb_connect(&bus_ctx, "usp:/tmp/test.sock"), 0);

    sleep(1);

    return 0;
}

int test_e2e_teardown(UNUSED void** state) {
    amxb_disconnect(bus_ctx);
    amxb_free(&bus_ctx);

    assert_int_equal(system("killall amxrt"), 0);
    amxb_be_remove_all();
    return 0;
}

int test_e2e_transl_setup(UNUSED void** state) {
    return test_common_e2e_transl_setup(&bus_ctx);
}

int test_e2e_transl_teardown(UNUSED void** state) {
    return test_common_e2e_transl_teardown(&bus_ctx);
}

void test_amxb_usp_read_get_supported_dm(UNUSED void** state) {
    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_get_supported_dm);
    will_return(__wrap_imtp_connection_read_frame, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    assert_int_equal(amxb_usp_read(usp_ctx), 0);
}

void test_amxb_usp_handle_get_supported_dm_error(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t request;
    amxc_var_t* flags;
    amxc_var_t* paths;

    amxc_var_init(&request);
    amxc_var_set_type(&request, AMXC_VAR_ID_HTABLE);
    flags = amxc_var_add_key(amxc_htable_t, &request, "flags", NULL);
    amxc_var_add_key(bool, flags, "first_level_only", true);
    amxc_var_add_key(bool, flags, "return_commands", true);
    amxc_var_add_key(bool, flags, "return_params", true);
    paths = amxc_var_add_key(amxc_llist_t, &request, "paths", NULL);
    amxc_var_add(cstring_t, paths, "test_root.");

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_get_supported_dm_new(usp_tx, &request), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    usp_rx->msg->body->msg_body_case = USP__BODY__MSG_BODY_ERROR;
    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    // Test sending error message in case of wrong USP message
    assert_int_equal(amxb_usp_handle_get_supported_dm(usp_ctx, usp_rx), 0);

    amxc_var_clean(&request);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_amxb_usp_handle_get_supported_dm_invalid(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t request;
    amxc_var_t* flags;
    amxc_var_t* paths;

    // Test invalid input arguments
    assert_int_equal(amxb_usp_handle_get_supported_dm(NULL, NULL), -1);
    assert_int_equal(amxb_usp_handle_get_supported_dm(usp_ctx, NULL), -1);

    amxc_var_init(&request);
    amxc_var_set_type(&request, AMXC_VAR_ID_HTABLE);
    flags = amxc_var_add_key(amxc_htable_t, &request, "flags", NULL);
    amxc_var_add_key(bool, flags, "first_level_only", true);
    amxc_var_add_key(bool, flags, "return_commands", true);
    amxc_var_add_key(bool, flags, "return_params", true);
    paths = amxc_var_add_key(amxc_llist_t, &request, "paths", NULL);
    amxc_var_add(cstring_t, paths, "Invalid.Path.");

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_get_supported_dm_new(usp_tx, &request), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    // Test handling gsdm in case the invoke gsdm fails
    assert_int_equal(amxb_usp_handle_get_supported_dm(usp_ctx, usp_rx), 0);

    amxc_var_clean(&request);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_amxb_usp_get_supported_dm_request(UNUSED void** state) {
    amxc_var_t ret;
    uint32_t flags = 0;
    amxc_var_t* ret_0 = NULL;
    amxc_var_t* object = NULL;
    amxc_var_t* supported_res = NULL;

    amxc_var_init(&ret);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_get_supported_dm_resp);
    will_return(__wrap_imtp_connection_read_frame, 0);

    assert_int_equal(amxb_usp_get_supported(usp_ctx, "test_root.", NULL, flags, &ret, 1), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    ret_0 = GETI_ARG(&ret, 0);
    assert_non_null(ret_0);
    assert_null(GET_ARG(ret_0, "result"));
    object = GET_ARG(ret_0, "test_root.");
    assert_non_null(object);
    assert_true(test_verify_data(object, "access", "0"));
    assert_true(test_verify_data(object, "is_multi_instance", "false"));
    supported_res = GET_ARG(object, "supported_commands");
    assert_non_null(supported_res);
    supported_res = GET_ARG(object, "supported_events");
    assert_non_null(supported_res);
    supported_res = GET_ARG(object, "supported_params");
    assert_non_null(supported_res);

    imtp_mock_free_msg_id();
    amxc_var_clean(&ret);
}

void test_gsdm_e2e(UNUSED void** state) {
    const char* object = "MQTT.";
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS;
    amxc_var_t ret;
    amxc_var_t* entry = NULL;

    amxc_var_init(&ret);
    amxb_get_supported(bus_ctx, object, flags, &ret, 5);
    amxc_var_dump(&ret, STDOUT_FILENO);

    entry = GETP_ARG(&ret, "0.'MQTT.'");
    assert_non_null(entry);
    test_verify_data(entry, "supported_params.0.param_name", "ClientNumberOfEntries");

    entry = GETP_ARG(&ret, "0.'MQTT.Capabilities.'");
    assert_non_null(entry);
    entry = GETP_ARG(&ret, "0.'MQTT.Client.{i}.'");
    assert_non_null(entry);
    test_verify_data(entry, "supported_commands.0.command_name", "ForceReconnect()");
    entry = GETP_ARG(&ret, "0.'MQTT.Client.{i}.Subscription.{i}.'");
    assert_non_null(entry);
    entry = GETP_ARG(&ret, "0.'MQTT.Client.{i}.UserProperty.{i}.'");
    assert_non_null(entry);

    amxc_var_clean(&ret);
}

void test_e2e_transl_gsdm(UNUSED void** state) {
    const char* object = "Device.MQTT.";
    uint32_t flags = AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS;
    amxc_var_t ret;
    amxc_var_t* entry = NULL;

    amxc_var_init(&ret);
    amxb_get_supported(bus_ctx, object, flags, &ret, 5);
    amxc_var_dump(&ret, STDOUT_FILENO);

    entry = GETP_ARG(&ret, "0.'Device.MQTT.'");
    assert_non_null(entry);
    test_verify_data(entry, "supported_params.0.param_name", "ClientNumberOfEntries");

    entry = GETP_ARG(&ret, "0.'Device.MQTT.Capabilities.'");
    assert_non_null(entry);
    entry = GETP_ARG(&ret, "0.'Device.MQTT.Client.{i}.'");
    assert_non_null(entry);
    test_verify_data(entry, "supported_commands.0.command_name", "ForceReconnect()");
    entry = GETP_ARG(&ret, "0.'Device.MQTT.Client.{i}.Subscription.{i}.'");
    assert_non_null(entry);
    entry = GETP_ARG(&ret, "0.'Device.MQTT.Client.{i}.UserProperty.{i}.'");
    assert_non_null(entry);

    amxc_var_clean(&ret);
}

void test_obuspa_gsdm_all(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    const char* results_file = "./results/obuspa_gsdm_all.json";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_supported(test_get_bus_ctx(), path, AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&ret);
}

void test_obuspa_gsdm_all_functions_events(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    const char* results_file = "./results/obuspa_gsdm_all_functions_events.json";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_supported(test_get_bus_ctx(), path, AMXB_FLAG_FUNCTIONS | AMXB_FLAG_EVENTS, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&ret);
}

// Get everything at first level + immediate child objects with no details
void test_obuspa_gsdm_first_level(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    const char* results_file = "./results/obuspa_gsdm_first_level.json";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_supported(test_get_bus_ctx(), path, AMXB_FLAG_FUNCTIONS | AMXB_FLAG_PARAMETERS | AMXB_FLAG_EVENTS | AMXB_FLAG_FIRST_LVL, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&ret);
}

void test_obuspa_gsdm_first_level_params(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    const char* results_file = "./results/obuspa_gsdm_first_level_params.json";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_supported(test_get_bus_ctx(), path, AMXB_FLAG_PARAMETERS | AMXB_FLAG_FIRST_LVL, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&ret);
}

void test_obuspa_gsdm_invalid_object(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeters.";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_supported(test_get_bus_ctx(), path, 0, &ret, 5);
    assert_int_equal(retval, amxd_status_invalid_path);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&ret);
}

void test_obuspa_gsdm_invalid_sub_object(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.Invalid.";
    amxc_var_t ret;

    amxc_var_init(&ret);
    retval = amxb_get_supported(test_get_bus_ctx(), path, 0, &ret, 5);
    assert_int_equal(retval, amxd_status_invalid_path);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&ret);
}
