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
#include "test_amxb_usp_delete.h"
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

void test_amxb_usp_read_delete(UNUSED void** state) {
    amxc_var_t values;
    const char* req_path = "test_root.child";
    amxd_object_t* template = NULL;
    amxd_object_t* new_inst = NULL;

    // Add a new instance that will be deleted
    template = amxd_dm_findf(test_get_dm(), req_path);
    assert_non_null(template);

    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "Alias", "Dummy");

    assert_int_equal(amxd_object_add_instance(&new_inst, template, NULL, 0, &values), 0);

    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_delete);
    will_return(__wrap_imtp_connection_read_frame, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    assert_int_equal(amxb_usp_read(usp_ctx), 0);

    amxc_var_clean(&values);
}

void test_amxb_usp_handle_delete_error(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t delete_var;
    amxc_var_t* requests = NULL;

    amxc_var_init(&delete_var);
    amxc_var_set_type(&delete_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(bool, &delete_var, "allow_partial", false);
    requests = amxc_var_add_key(amxc_llist_t, &delete_var, "requests", NULL);
    amxc_var_add(cstring_t, requests, "test_root.child.[Alias == \"Dummy\"].");

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_delete_new(usp_tx, &delete_var), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    usp_rx->msg->body->msg_body_case = USP__BODY__MSG_BODY_ERROR;
    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    // Test sending error message in case of wrong USP message
    assert_int_equal(amxb_usp_handle_delete(usp_ctx, usp_rx), 0);

    amxc_var_clean(&delete_var);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_amxb_usp_handle_delete_invalid(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t delete_var;
    amxc_var_t* requests = NULL;

    // Test invalid input arguments
    assert_int_equal(amxb_usp_handle_delete(NULL, NULL), -1);
    assert_int_equal(amxb_usp_handle_delete(usp_ctx, NULL), -1);

    amxc_var_init(&delete_var);
    amxc_var_set_type(&delete_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(bool, &delete_var, "allow_partial", false);
    requests = amxc_var_add_key(amxc_llist_t, &delete_var, "requests", NULL);
    amxc_var_add(cstring_t, requests, "test_root.child.[Alias == \"Invalid\"]");

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_delete_new(usp_tx, &delete_var), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    // Test handling Delete in case the invoke delete fails
    assert_int_equal(amxb_usp_handle_delete(usp_ctx, usp_rx), 0);

    amxc_var_clean(&delete_var);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_amxb_usp_delete_request(UNUSED void** state) {
    amxc_var_t ret;
    int index = 0;
    int access = 0;
    const char* name = NULL;

    amxc_var_init(&ret);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_delete_resp);
    will_return(__wrap_imtp_connection_read_frame, 0);

    assert_int_equal(amxb_usp_delete(usp_ctx, "test_root.", "child.1.", index, name, access, &ret, 1), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_string_equal(GETP_CHAR(&ret, "0"), "test_root.child.1.");

    imtp_mock_free_msg_id();
    amxc_var_clean(&ret);
}

void test_e2e_proxy_delete(UNUSED void** state) {
    amxd_status_t status = amxd_status_ok;
    const char* object = "Device.MQTT.Client.1.";
    amxc_var_t ret;

    amxc_var_init(&ret);

    status = amxb_del(bus_ctx, object, 0, NULL, &ret, 5);
    assert_int_equal(status, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_true(test_verify_data(&ret, "0", "Device.MQTT.Client.1."));
    assert_true(test_verify_data(&ret, "1", "Device.MQTT.Client.1.Subscription."));
    assert_true(test_verify_data(&ret, "2", "Device.MQTT.Client.1.Stats."));
    assert_true(test_verify_data(&ret, "3", "Device.MQTT.Client.1.UserProperty."));

    amxc_var_clean(&ret);
}

void test_e2e_proxy_delete_search_path(UNUSED void** state) {
    amxd_status_t status = amxd_status_ok;
    const char* object = "Device.MQTT.Client.[BrokerPort == 1893].";
    amxc_var_t ret;

    amxc_var_init(&ret);

    status = amxb_del(bus_ctx, object, 0, NULL, &ret, 5);
    assert_int_equal(status, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_true(test_verify_data(&ret, "0", "Device.MQTT.Client.2."));
    assert_true(test_verify_data(&ret, "1", "Device.MQTT.Client.2.Subscription."));
    assert_true(test_verify_data(&ret, "2", "Device.MQTT.Client.2.Stats."));
    assert_true(test_verify_data(&ret, "3", "Device.MQTT.Client.2.UserProperty."));

    amxc_var_clean(&ret);
}

// TODO: figure out return status for delete with search path which has no result
void test_e2e_proxy_delete_search_path_no_result(UNUSED void** state) {
    amxd_status_t status = amxd_status_ok;
    const char* object = "Device.MQTT.Client.[BrokerPort == 1893].";
    amxc_var_t ret;

    amxc_var_init(&ret);

    status = amxb_del(bus_ctx, object, 0, NULL, &ret, 5);
    assert_int_not_equal(status, amxd_status_ok);

    amxc_var_clean(&ret);
}

void test_e2e_proxy_delete_invalid_path(UNUSED void** state) {
    amxd_status_t status = amxd_status_ok;
    const char* object = "Device.MQTT.Clients.1.";
    amxc_var_t ret;

    amxc_var_init(&ret);

    status = amxb_del(bus_ctx, object, 0, NULL, &ret, 5);
    assert_int_equal(status, amxd_status_object_not_found);

    amxc_var_clean(&ret);
}

void test_obuspa_delete_instance(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.History.2.Info.1.";
    const char* results_file = "./results/obuspa_delete_instance.json";
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(test_get_bus_ctx(), path, 0, NULL, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&ret);
}

void test_obuspa_delete_search_path(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.History.[From==\"Storyteller\"].";
    const char* results_file = "./results/obuspa_delete_search_path.json";
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(test_get_bus_ctx(), path, 0, NULL, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&ret);
}

void test_obuspa_delete_singleton(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(test_get_bus_ctx(), path, 0, NULL, &ret, 5);
    // TODO update tests when obuspa returns a valid error code
    //assert_int_equal(retval, amxd_status_not_a_template);
    assert_int_not_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&ret);
}

void test_obuspa_delete_invalid_object(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.Invalid.";
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(test_get_bus_ctx(), path, 0, NULL, &ret, 5);
    // TODO update tests when obuspa returns a valid error code
    assert_int_equal(retval, amxd_status_invalid_path);
    //assert_int_not_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&ret);
}