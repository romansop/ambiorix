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

#include "amxb_usp.h"
#include "test_amxb_usp_common.h"
#include "test_amxb_usp_la_subs.h"
#include "imtp_mock.h"

#define UNUSED __attribute__((unused))

static amxb_bus_ctx_t* bus_ctx = NULL;
static amxc_var_t config;

static void read_notification(amxc_var_t* notification) {
    uint32_t count = 1024;
    uint8_t* buffer = (uint8_t*) calloc(1, count);
    imtp_frame_t* frame = NULL;
    const imtp_tlv_t* tlv_pbuf = NULL;
    uspl_rx_t* usp_rx = NULL;

    // Need read_raw, because client side of backend does not create subscription on instance add
    // so there will be no signal emitted during amxb_usp_handle_notify
    assert_true(amxb_read_raw(bus_ctx, buffer, count - 1) >= 0);

    assert_int_equal(imtp_frame_parse(&frame, buffer), 0);
    tlv_pbuf = imtp_frame_get_first_tlv(frame, imtp_tlv_type_protobuf_bytes);
    assert_non_null(tlv_pbuf);

    usp_rx = uspl_msghandler_unpack_protobuf(tlv_pbuf->value + tlv_pbuf->offset, tlv_pbuf->length);
    assert_non_null(usp_rx);

    assert_int_equal(uspl_notify_extract(usp_rx, notification), 0);

    uspl_rx_delete(&usp_rx);
    imtp_frame_delete(&frame);
}

static void handle_notification(UNUSED const char* const sig_name,
                                const amxc_var_t* const data,
                                UNUSED void* const priv) {
    amxc_var_dump(data, STDOUT_FILENO);
    //test_write_json_to_file(data, "./verify_data/object_deletion.json");
    check_expected(data);
}

int test_e2e_setup(UNUSED void** state) {
    amxc_var_init(&config);

    // Launch MQTT data model with LocalAgent data model
    assert_int_equal(system("amxrt ../amxb_usp_common/odl/tr181-mqtt_translations.odl ../amxb_usp_common/odl/tr181-mqtt_definition.odl ../amxb_usp_common/odl/tr181-mqtt_defaults.odl -D"), 0);
    sleep(1);

    // Load backend and connect to proxy socket
    assert_int_equal(amxb_be_load("../mod-amxb-test-usp.so"), 0);

    // Make sure config is set correctly for EID exchange
    amxc_var_set_type(&config, AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxb_set_config(&config), 0);
    assert_int_equal(amxb_connect(&bus_ctx, "usp:/tmp/test.sock"), 0);
    sleep(1);

    // Read handshake
    assert_int_equal(amxb_read(bus_ctx), 0);

    return 0;
}

int test_e2e_teardown(UNUSED void** state) {
    amxc_var_clean(&config);
    amxb_disconnect(bus_ctx);
    amxb_free(&bus_ctx);

    assert_int_equal(system("killall amxrt"), 0);

    amxb_be_remove_all();
    return 0;
}

void test_e2e_add_subs_value_change(UNUSED void** state) {
    amxd_status_t status = amxd_status_ok;
    const char* object = "Device.LocalAgent.Subscription.";
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "Alias", "mqtt-vc");
    amxc_var_add_key(bool, &args, "Enable", true);
    amxc_var_add_key(cstring_t, &args, "ID", "mqtt-vc");
    amxc_var_add_key(cstring_t, &args, "NotifType", "ValueChange");
    amxc_var_add_key(cstring_t, &args, "ReferenceList", "Device.MQTT.Client.");

    status = amxb_add(bus_ctx, object, 0, NULL, &args, &ret, 5);
    assert_int_equal(status, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_true(test_verify_data(&ret, "0.index", "1"));
    assert_true(test_verify_data(&ret, "0.parameters.Alias", "mqtt-vc"));
    assert_true(test_verify_data(&ret, "0.parameters.ID", "mqtt-vc"));
    assert_true(test_verify_data(&ret, "0.parameters.Recipient", "Device.LocalAgent.Controller.1"));
    // only key parameters are returned
    assert_null(GETP_ARG(&ret, "0.parameters.NotifType"));

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_get_value_change_notification(UNUSED void** state) {
    amxd_status_t status = amxd_status_ok;
    const char* object = "Device.MQTT.Client.1.";
    amxc_var_t ret;
    amxc_var_t args;
    amxc_var_t notification;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_init(&notification);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "BrokerAddress", "NewAddress");

    status = amxb_set(bus_ctx, object, &args, &ret, 5);
    assert_int_equal(status, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);

    // Give other process time to send notification
    sleep(1);
    read_notification(&notification);
    amxc_var_dump(&notification, STDOUT_FILENO);

    assert_true(test_verify_data(&notification, "subscription_id", "mqtt-vc"));
    assert_true(test_verify_data(&notification, "value_change.param_path", "Device.MQTT.Client.1.BrokerAddress"));
    assert_true(test_verify_data(&notification, "value_change.param_value", "NewAddress"));

    amxc_var_clean(&notification);
    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_del_subs_value_change(UNUSED void** state) {
    amxd_status_t status = amxd_status_ok;
    const char* object = "Device.LocalAgent.Subscription.1.";
    uint32_t count = 1024;
    uint8_t buffer[1024] = {};
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);

    status = amxb_del(bus_ctx, object, 0, NULL, &ret, 5);
    assert_int_equal(status, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_true(test_verify_data(&ret, "0", object));

    // Make sure no event is sent after subscription has been deleted
    amxc_var_add_key(cstring_t, &args, "BrokerAddress", "NewNewAddress");
    status = amxb_set(bus_ctx, "Device.MQTT.Client.1.", &args, &ret, 5);
    assert_int_equal(status, amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);

    // Give other side time to send notification if there was any
    sleep(1);

    // Then check that indeed nothing was sent
    assert_false(amxb_read_raw(bus_ctx, buffer, count - 1) >= 0);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_subscribe_v2_value_change(UNUSED void** state) {
    const char* object = "Device.MQTT.Client.1.";
    amxc_var_t args;
    amxc_var_t ret;
    amxc_var_t* expected_variant = amxut_util_read_json_from_file("./verify_data/value_change_single.json");

    amxc_var_init(&ret);
    amxc_var_init(&args);

    assert_int_equal(amxb_subscribe(bus_ctx, object, NULL, handle_notification, NULL), 0);
    usleep(100);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "ClientID", "Darth Vader");
    assert_int_equal(amxb_set(bus_ctx, object, &args, &ret, 5), 0);
    usleep(100);

    // Read notification
    assert_int_equal(amxb_read(bus_ctx), 0);

    // Handle callback function
    expect_check(handle_notification, data, test_variant_equal_check, expected_variant);
    handle_events();

    assert_int_equal(amxb_unsubscribe(bus_ctx, object, handle_notification, NULL), 0);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_subscribe_v2_value_change_search_path(UNUSED void** state) {
    const char* object = "Device.MQTT.Client.*.";
    amxc_var_t args;
    amxc_var_t ret;
    amxc_var_t* expected_variant_1 = amxut_util_read_json_from_file("./verify_data/value_change_search_path_1.json");
    amxc_var_t* expected_variant_2 = amxut_util_read_json_from_file("./verify_data/value_change_search_path_2.json");

    amxc_var_init(&ret);
    amxc_var_init(&args);

    assert_int_equal(amxb_subscribe(bus_ctx, object, NULL, handle_notification, NULL), 0);
    usleep(100);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "BrokerAddress", "Death Star Satellite");
    assert_int_equal(amxb_set(bus_ctx, object, &args, &ret, 5), 0);
    usleep(100);

    // Read notifications
    assert_int_equal(amxb_read(bus_ctx), 0);
    assert_int_equal(amxb_read(bus_ctx), 0);

    // Handle callback function
    expect_check(handle_notification, data, test_variant_equal_check, expected_variant_1);
    expect_check(handle_notification, data, test_variant_equal_check, expected_variant_2);
    handle_events();

    assert_int_equal(amxb_unsubscribe(bus_ctx, object, handle_notification, NULL), 0);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_subscribe_v2_value_change_change_multiple(UNUSED void** state) {
    const char* object = "Device.MQTT.Client.1.";
    amxc_var_t args;
    amxc_var_t ret;
    amxc_var_t* expected_variant_1 = amxut_util_read_json_from_file("./verify_data/value_change_multiple_1.json");
    amxc_var_t* expected_variant_2 = amxut_util_read_json_from_file("./verify_data/value_change_multiple_2.json");

    amxc_var_init(&ret);
    amxc_var_init(&args);

    assert_int_equal(amxb_subscribe(bus_ctx, object, NULL, handle_notification, NULL), 0);
    usleep(100);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(bool, &args, "Enable", true);
    amxc_var_add_key(uint32_t, &args, "KeepAliveTime", 20);
    assert_int_equal(amxb_set(bus_ctx, object, &args, &ret, 5), 0);
    usleep(100);

    // Read notifications
    assert_int_equal(amxb_read(bus_ctx), 0);
    assert_int_equal(amxb_read(bus_ctx), 0);

    // Handle callback function
    expect_check(handle_notification, data, test_variant_equal_check, expected_variant_1);
    expect_check(handle_notification, data, test_variant_equal_check, expected_variant_2);
    handle_events();

    assert_int_equal(amxb_unsubscribe(bus_ctx, object, handle_notification, NULL), 0);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

//void test_e2e_subscribe_v2_value_change_with_filter(UNUSED void** state) {

void test_e2e_subscribe_v2_object_creation(UNUSED void** state) {
    const char* object = "Device.MQTT.Client.";
    amxc_var_t args;
    amxc_var_t ret;
    amxc_var_t* expected_variant = amxut_util_read_json_from_file("./verify_data/object_creation.json");

    amxc_var_init(&ret);
    amxc_var_init(&args);

    assert_int_equal(amxb_subscribe(bus_ctx, object, NULL, handle_notification, NULL), 0);
    usleep(100);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "ClientID", "Luke Skywalker");
    assert_int_equal(amxb_add(bus_ctx, object, 0, NULL, &args, &ret, 5), 0);
    usleep(100);

    // Read notification
    assert_int_equal(amxb_read(bus_ctx), 0);

    // Handle callback function
    expect_check(handle_notification, data, test_variant_equal_check, expected_variant);
    handle_events();

    assert_int_equal(amxb_unsubscribe(bus_ctx, object, handle_notification, NULL), 0);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

// Test depends on client added in test_e2e_subscribe_v2_object_creation
void test_e2e_subscribe_v2_object_deletion(UNUSED void** state) {
    const char* object = "Device.MQTT.Client.*.";
    amxc_var_t ret;
    amxc_var_t args;
    //amxc_var_t* expected_variant = amxut_util_read_json_from_file("./verify_data/object_deletion.json");

    amxc_var_init(&ret);
    amxc_var_init(&args);

    // Add an instance that will be deleted in this test
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "ClientID", "Anakin Skywalker");
    assert_int_equal(amxb_add(bus_ctx, "Device.MQTT.Client.", 0, NULL, &args, &ret, 5), 0);
    usleep(100);

    // Add subscription
    assert_int_equal(amxb_subscribe(bus_ctx, object, NULL, handle_notification, NULL), 0);
    usleep(100);

    // Get clients -> number 3 is still there
    //assert_int_equal(amxb_get(bus_ctx, "Device.MQTT.Client.", 5, &ret, 5), 0);
    //amxc_var_dump(&ret, 1);

    assert_int_equal(amxb_del(bus_ctx, "Device.MQTT.Client.[ClientID == 'Anakin Skywalker'].", 0, NULL, &ret, 5), 0);
    usleep(100);

    // Get clients -> number 3 is gone
    //assert_int_equal(amxb_get(bus_ctx, "Device.MQTT.Client.", 5, &ret, 5), 0);
    //amxc_var_dump(&ret, 1);

    // Read notification -> notification is never sent
    //assert_int_equal(amxb_read(bus_ctx), 0);

    // Handle callback function
    //expect_check(handle_notification, data, test_variant_equal_check, expected_variant);
    handle_events();

    assert_int_equal(amxb_unsubscribe(bus_ctx, object, handle_notification, NULL), 0);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}
