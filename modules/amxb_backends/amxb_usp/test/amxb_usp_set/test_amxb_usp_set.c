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
#include "test_amxb_usp_set.h"
#include "test_amxb_usp_common.h"
#include "imtp_mock.h"

#define UNUSED __attribute__((unused))

static amxb_usp_t* usp_ctx = NULL;
static amxb_bus_ctx_t* bus_ctx = NULL;

static void fill_set_var(amxc_var_t* set_var) {
    amxc_var_t* requests = NULL;
    amxc_var_t* request = NULL;
    amxc_var_t* params_list = NULL;
    amxc_var_t* params_entry = NULL;

    amxc_var_set_type(set_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(bool, set_var, "allow_partial", false);
    requests = amxc_var_add_key(amxc_llist_t, set_var, "requests", NULL);
    request = amxc_var_add(amxc_htable_t, requests, NULL);
    amxc_var_add_key(cstring_t, request, "object_path", "test_root.child.1.");
    params_list = amxc_var_add_key(amxc_llist_t, request, "parameters", NULL);
    params_entry = amxc_var_add(amxc_htable_t, params_list, NULL);
    amxc_var_add_key(cstring_t, params_entry, "param", "Name");
    amxc_var_add_key(cstring_t, params_entry, "value", "Bob");
    amxc_var_add_key(bool, params_entry, "required", true);
}

static void fill_set_var_optional(amxc_var_t* set_var) {
    amxc_var_t* requests = NULL;
    amxc_var_t* request = NULL;
    amxc_var_t* params_list = NULL;
    amxc_var_t* params_entry = NULL;

    amxc_var_set_type(set_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(bool, set_var, "allow_partial", false);
    requests = amxc_var_add_key(amxc_llist_t, set_var, "requests", NULL);
    request = amxc_var_add(amxc_htable_t, requests, NULL);
    amxc_var_add_key(cstring_t, request, "object_path", "test_root.child.1.");
    params_list = amxc_var_add_key(amxc_llist_t, request, "parameters", NULL);

    params_entry = amxc_var_add(amxc_htable_t, params_list, NULL);
    amxc_var_add_key(cstring_t, params_entry, "param", "Name");
    amxc_var_add_key(cstring_t, params_entry, "value", "Cat");
    amxc_var_add_key(bool, params_entry, "required", true);

    params_entry = amxc_var_add(amxc_htable_t, params_list, NULL);
    amxc_var_add_key(cstring_t, params_entry, "param", "Alias");
    amxc_var_add_key(cstring_t, params_entry, "value", "Cat");
    amxc_var_add_key(bool, params_entry, "required", false);
}

int test_dm_setup(UNUSED void** state) {
    return test_common_setup(&usp_ctx);
}

int test_dm_teardown(UNUSED void** state) {
    return test_common_teardown(&usp_ctx);
}

int test_e2e_proxy_setup(UNUSED void** state) {
    return test_common_e2e_proxy_setup(&bus_ctx);
}

int test_e2e_proxy_teardown(UNUSED void** state) {
    return test_common_e2e_proxy_teardown(&bus_ctx);
}

int test_e2e_transl_setup(UNUSED void** state) {
    return test_common_e2e_transl_setup(&bus_ctx);
}

int test_e2e_transl_teardown(UNUSED void** state) {
    return test_common_e2e_transl_teardown(&bus_ctx);
}

void test_amxb_usp_read_set(UNUSED void** state) {
    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_set);
    will_return(__wrap_imtp_connection_read_frame, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    assert_int_equal(amxb_usp_read(usp_ctx), 0);
}

void test_amxb_usp_handle_set_error(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t request;

    amxc_var_init(&request);
    fill_set_var(&request);

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_set_new(usp_tx, &request), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    usp_rx->msg->body->msg_body_case = USP__BODY__MSG_BODY_ERROR;
    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    // Test sending error message in case of wrong USP message
    assert_int_equal(amxb_usp_handle_set(usp_ctx, usp_rx), 0);

    amxc_var_clean(&request);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_amxb_usp_handle_set_invalid(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t request;

    // Test invalid input arguments
    assert_int_equal(amxb_usp_handle_set(NULL, NULL), -1);
    assert_int_equal(amxb_usp_handle_set(usp_ctx, NULL), -1);

    amxc_var_init(&request);
    fill_set_var(&request);

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_set_new(usp_tx, &request), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    // Test handling Set in case the invoke Set fails
    assert_int_equal(amxb_usp_handle_set(usp_ctx, usp_rx), 0);

    amxc_var_clean(&request);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_amxb_usp_set_request(UNUSED void** state) {
    amxc_var_t ret;
    amxc_var_t values;
    int access = 0;
    amxc_var_t* result = NULL;

    amxc_var_init(&ret);
    amxc_var_init(&values);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_set_resp);
    will_return(__wrap_imtp_connection_read_frame, 0);

    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "Name", "Super dummy");

    assert_int_equal(amxb_usp_set(usp_ctx, "test_root.", "child.1.", 0, &values, NULL, access, &ret, 1), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    result = GETP_ARG(&ret, "0.'test_root.child.1.'");
    assert_non_null(result);

    assert_true(test_verify_data(result, "Name", "Super dummy"));

    amxc_var_clean(&ret);
    amxc_var_clean(&values);
}

void test_amxb_usp_handle_set_optional_params(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t request;

    amxc_var_init(&request);
    fill_set_var_optional(&request);

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_set_new(usp_tx, &request), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    assert_int_equal(amxb_usp_handle_set(usp_ctx, usp_rx), 0);

    imtp_mock_free_msg_id();
    amxc_var_clean(&request);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_e2e_proxy_set_params(UNUSED void** state) {
    const char* object = "Device.MQTT.Client.1.";
    amxc_var_t* check = NULL;
    amxc_var_t args;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(bool, &args, "Enable", true);

    assert_int_equal(amxb_set(bus_ctx, object, &args, &ret, 5), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    check = GETP_ARG(&ret, "0.0");
    assert_string_equal(amxc_var_key(check), object);
    check = GET_ARG(check, "Enable");
    assert_int_equal(amxc_var_type_of(check), AMXC_VAR_ID_BOOL);
    assert_true(amxc_var_dyncast(bool, check));

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_proxy_set_search_path(UNUSED void** state) {
    const char* object = "Device.MQTT.Client.[BrokerPort == 1883].";
    amxc_var_t* check = NULL;
    amxc_var_t args;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "TransportProtocol", "TLS");

    assert_int_equal(amxb_set(bus_ctx, object, &args, &ret, 5), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    check = GETP_ARG(&ret, "0.0");
    assert_string_equal(amxc_var_key(check), "Device.MQTT.Client.1.");
    check = GET_ARG(check, "TransportProtocol");
    assert_int_equal(amxc_var_type_of(check), AMXC_VAR_ID_CSTRING);
    assert_string_equal(amxc_var_constcast(cstring_t, check), "TLS");

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_proxy_set_invalid_object(UNUSED void** state) {
    const char* object = "Device.MQTT.Invalid.";
    amxd_status_t status;
    amxc_var_t args;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "TransportProtocol", "TLS");

    status = amxb_set(bus_ctx, object, &args, &ret, 10);
    assert_int_equal(status, amxd_status_object_not_found);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_proxy_set_invalid_param(UNUSED void** state) {
    const char* object = "Device.MQTT.";
    amxd_status_t status;
    amxc_var_t args;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "Invalid", "Oof");

    status = amxb_set(bus_ctx, object, &args, &ret, 10);
    assert_int_equal(status, amxd_status_invalid_path);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_obuspa_set_param(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    const char* results_file = "./results/obuspa_set_param.json";
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "State", "Start");

    retval = amxb_set(test_get_bus_ctx(), path, &values, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_obuspa_set_param_read_only(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &values, "MaxHistory", 99);

    retval = amxb_set(test_get_bus_ctx(), path, &values, &ret, 5);
    assert_int_equal(retval, amxd_status_read_only);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_obuspa_set_param_invalid_value(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "State", "Invalid");

    retval = amxb_set(test_get_bus_ctx(), path, &values, &ret, 5);
    assert_int_equal(retval, amxd_status_invalid_value);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_obuspa_set_search_path(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.History.[Retain==true].Info.*.";
    const char* results_file = "./results/obuspa_set_search_path.json";
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "Text", "Koffiekoeken");
    amxc_var_add_key(int32_t, &values, "SignedNumber", -999);
    amxc_var_add_key(bool, &values, "Disabled", true);

    retval = amxb_set(test_get_bus_ctx(), path, &values, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_obuspa_set_search_path_complex(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.History.[Retain==true].Info.[Alias==\"cpe-Info-1\"&&Number==0&&Flags==\"\"].";
    const char* results_file = "./results/obuspa_set_search_path_complex.json";
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "Text", "Koffiekoeken");
    amxc_var_add_key(int32_t, &values, "SignedNumber", -999);
    amxc_var_add_key(bool, &values, "Disabled", true);

    retval = amxb_set(test_get_bus_ctx(), path, &values, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_obuspa_set_search_path_no_match(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.History.[Retain==false].Info.[Alias==\"cpe-Info-1\"&&Number==0&&Flags==\"\"].";
    const char* results_file = "./results/obuspa_set_search_path_no_match.json";
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "Text", "Koffiekoeken");
    amxc_var_add_key(int32_t, &values, "SignedNumber", -999);
    amxc_var_add_key(bool, &values, "Disabled", true);

    retval = amxb_set(test_get_bus_ctx(), path, &values, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_obuspa_set_invalid_object(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeters.";
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "Text", "Koffiekoeken");

    retval = amxb_set(test_get_bus_ctx(), path, &values, &ret, 5);
    assert_int_equal(retval, amxd_status_invalid_path);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_obuspa_set_invalid_object_partial_true(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeters.";
    amxc_var_t* req_path = NULL;
    amxc_var_t* params = NULL;
    amxc_var_t req_paths;
    amxc_var_t ret;

    amxc_var_init(&ret);
    amxc_var_init(&req_paths);
    amxc_var_set_type(&req_paths, AMXC_VAR_ID_LIST);
    req_path = amxc_var_add(amxc_htable_t, &req_paths, NULL);
    amxc_var_add_key(cstring_t, req_path, "path", path);
    params = amxc_var_add_key(amxc_htable_t, req_path, "parameters", NULL);
    amxc_var_add_key(cstring_t, params, "Param", "Value");

    retval = amxb_set_multiple(test_get_bus_ctx(), AMXB_FLAG_PARTIAL, &req_paths, &ret, 5);
    // allow_partial == true will always return 0
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&req_paths);
    amxc_var_clean(&ret);
}
