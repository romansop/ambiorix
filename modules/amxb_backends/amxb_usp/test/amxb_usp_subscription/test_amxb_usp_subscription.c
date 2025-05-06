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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>
#include <unistd.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_transaction.h>
#include <amxb/amxb.h>
#include <amxo/amxo.h>

#include <amxut/amxut_util.h>
#include <amxut/amxut_verify.h>

#include "amxb_usp.h"
#include "test_amxb_usp_subscription.h"
#include "test_amxb_usp_common.h"
#include "imtp_mock.h"

#define UNUSED __attribute__((unused))

static amxb_bus_ctx_t* bus_ctx = NULL;
static amxc_var_t events;
static amxc_var_t* config = NULL;

static void event_handler(const char* const sig_name,
                          const amxc_var_t* const data,
                          void* const priv) {
    int* event_count = (int*) priv;
    amxc_var_t* event = NULL;
    printf("Event received = %s\n", sig_name);
    fflush(stdout);
    amxc_var_dump(data, STDOUT_FILENO);

    check_expected(sig_name);

    event = amxc_var_add_new(&events);
    amxc_var_copy(event, data);

    (*event_count)++;
}

static void reset_events() {
    amxc_var_clean(&events);
    amxc_var_set_type(&events, AMXC_VAR_ID_LIST);
}

int test_e2e_setup(UNUSED void** state) {
    amxc_var_t* usp_config = NULL;

    amxc_var_init(&events);
    amxc_var_new(&config);
    amxc_var_set_type(&events, AMXC_VAR_ID_LIST);
    amxc_var_set_type(config, AMXC_VAR_ID_HTABLE);
    usp_config = amxc_var_add_key(amxc_htable_t, config, "usp", NULL);
    amxc_var_add_key(bool, usp_config, "legacy-subscribe", true);
    amxc_var_add_key(bool, usp_config, "requires-device-prefix", false);

    assert_int_equal(system("amxrt -D ../amxb_usp_common/odl/test_config.odl ../amxb_usp_common/odl/tr181-mqtt_definition.odl"), 0);

    assert_int_equal(amxb_be_load("../mod-amxb-test-usp.so"), 0);
    amxb_set_config(config);
    assert_int_equal(amxb_connect(&bus_ctx, "usp:/tmp/test.sock"), 0);
    usleep(100);
    assert_int_equal(amxb_read(bus_ctx), 0); // Read handshake

    return 0;
}

int test_e2e_teardown(UNUSED void** state) {
    amxb_disconnect(bus_ctx);
    amxb_free(&bus_ctx);

    assert_int_equal(system("killall amxrt"), 0);

    amxb_be_remove_all();
    amxc_var_delete(&config);
    amxc_var_clean(&events);
    return 0;
}

void test_e2e_subscribe_instance_added(UNUSED void** state) {
    amxc_var_t values;
    amxc_var_t ret;
    int event_count = 0;
    amxc_var_t* event = NULL;
    const char* broker_address = "broker.hivemq.com";
    const char* topic = "topic-test";

    amxc_var_init(&ret);
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);

    assert_int_equal(amxb_subscribe(bus_ctx, "MQTT.", NULL, event_handler, &event_count), 0);
    usleep(100);

    amxc_var_add_key(cstring_t, &values, "BrokerAddress", broker_address);
    assert_int_equal(amxb_add(bus_ctx, "MQTT.Client.", 1, "client-test", &values, &ret, 5), 0);

    expect_string_count(event_handler, sig_name, "MQTT", 5);
    handle_e2e_events(bus_ctx);
    assert_int_equal(event_count, 5);

    event = GETI_ARG(&events, 0);
    assert_true(test_verify_data(event, "eobject", "MQTT."));
    assert_true(test_verify_data(event, "notification", "dm:object-changed"));
    assert_true(test_verify_data(event, "parameters.ClientNumberOfEntries.from", "0"));
    assert_true(test_verify_data(event, "parameters.ClientNumberOfEntries.to", "1"));
    assert_true(test_verify_data(event, "object", "MQTT."));
    assert_true(test_verify_data(event, "path", "MQTT."));

    event = GETI_ARG(&events, 1);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client."));
    assert_true(test_verify_data(event, "index", "1"));
    assert_true(test_verify_data(event, "keys.Alias", "cpe-Client-1")); // Alias is not correctly passed yet
    assert_true(test_verify_data(event, "name", "cpe-Client-1"));
    assert_true(test_verify_data(event, "notification", "dm:instance-added"));
    assert_true(test_verify_data(event, "object", "MQTT.Client."));
    assert_true(test_verify_data(event, "path", "MQTT.Client."));
    assert_true(test_verify_data(event, "parameters.BrokerAddress", broker_address));

    event = GETI_ARG(&events, 2);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client.[cpe-Client-1].Subscription."));
    assert_true(test_verify_data(event, "notification", "dm:object-added"));
    assert_true(test_verify_data(event, "object", "MQTT.Client.cpe-Client-1.Subscription."));
    assert_true(test_verify_data(event, "path", "MQTT.Client.1.Subscription."));

    event = GETI_ARG(&events, 3);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client.[cpe-Client-1].Stats."));
    assert_true(test_verify_data(event, "notification", "dm:object-added"));
    assert_true(test_verify_data(event, "object", "MQTT.Client.cpe-Client-1.Stats."));
    assert_true(test_verify_data(event, "path", "MQTT.Client.1.Stats."));

    event = GETI_ARG(&events, 4);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client.[cpe-Client-1].UserProperty."));
    assert_true(test_verify_data(event, "notification", "dm:object-added"));
    assert_true(test_verify_data(event, "object", "MQTT.Client.cpe-Client-1.UserProperty."));
    assert_true(test_verify_data(event, "path", "MQTT.Client.1.UserProperty."));
    event_count = 0;
    reset_events();

    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "Topic", topic);
    assert_int_equal(amxb_add(bus_ctx, "MQTT.Client.1.Subscription.", 1, "sub-1", &values, &ret, 5), 0);

    expect_string_count(event_handler, sig_name, "MQTT", 2);
    handle_e2e_events(bus_ctx);
    assert_int_equal(event_count, 2);

    event = GETI_ARG(&events, 0);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client.[cpe-Client-1]."));
    assert_true(test_verify_data(event, "notification", "dm:object-changed"));
    assert_true(test_verify_data(event, "parameters.SubscriptionNumberOfEntries.from", "0"));
    assert_true(test_verify_data(event, "parameters.SubscriptionNumberOfEntries.to", "1"));
    assert_true(test_verify_data(event, "object", "MQTT.Client.cpe-Client-1."));
    assert_true(test_verify_data(event, "path", "MQTT.Client.1."));

    event = GETI_ARG(&events, 1);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client.[cpe-Client-1].Subscription."));
    assert_true(test_verify_data(event, "index", "1"));
    assert_true(test_verify_data(event, "keys.Alias", "cpe-Subscription-1")); // Alias is not correctly passed yet
    assert_true(test_verify_data(event, "name", "cpe-Subscription-1"));
    assert_true(test_verify_data(event, "notification", "dm:instance-added"));
    assert_true(test_verify_data(event, "object", "MQTT.Client.cpe-Client-1.Subscription."));
    assert_true(test_verify_data(event, "path", "MQTT.Client.1.Subscription."));
    assert_true(test_verify_data(event, "parameters.Topic", topic));
    event_count = 0;
    reset_events();

    assert_int_equal(amxb_unsubscribe(bus_ctx, "MQTT.", event_handler, &event_count), 0);
    usleep(100);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_e2e_subscribe_object_changed(UNUSED void** state) {
    amxc_var_t values;
    amxc_var_t ret;
    int event_count = 0;
    amxc_var_t* event = NULL;
    const char* broker_address_old = "broker.hivemq.com";
    const char* broker_address_new = "test.mosquitto.org";

    amxc_var_init(&ret);
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);

    assert_int_equal(amxb_subscribe(bus_ctx, "MQTT.", NULL, event_handler, &event_count), 0);
    usleep(100);

    amxc_var_add_key(cstring_t, &values, "BrokerAddress", broker_address_new);
    amxc_var_add_key(uint32_t, &values, "BrokerPort", 8000);
    assert_int_equal(amxb_set(bus_ctx, "MQTT.Client.1.", &values, &ret, 5), 0);

    expect_string_count(event_handler, sig_name, "MQTT", 1);
    handle_e2e_events(bus_ctx);
    assert_int_equal(event_count, 1);

    event = GETI_ARG(&events, 0);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client.[cpe-Client-1]."));
    assert_true(test_verify_data(event, "notification", "dm:object-changed"));
    assert_true(test_verify_data(event, "object", "MQTT.Client.cpe-Client-1."));
    assert_true(test_verify_data(event, "path", "MQTT.Client.1."));
    assert_true(test_verify_data(event, "parameters.BrokerAddress.from", broker_address_old));
    assert_true(test_verify_data(event, "parameters.BrokerAddress.to", broker_address_new));
    assert_true(test_verify_data(event, "parameters.BrokerPort.from", "1883"));
    assert_true(test_verify_data(event, "parameters.BrokerPort.to", "8000"));
    event_count = 0;
    reset_events();

    assert_int_equal(amxb_unsubscribe(bus_ctx, "MQTT.", event_handler, &event_count), 0);
    usleep(100);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}

void test_e2e_subscribe_instance_removed(UNUSED void** state) {
    amxc_var_t ret;
    int event_count = 0;
    amxc_var_t* event = NULL;

    amxc_var_init(&ret);

    assert_int_equal(amxb_subscribe(bus_ctx, "MQTT.", NULL, event_handler, &event_count), 0);
    usleep(100);

    assert_int_equal(amxb_del(bus_ctx, "MQTT.Client.1.", 0, NULL, &ret, 5), 0);

    expect_string_count(event_handler, sig_name, "MQTT", 7);
    handle_e2e_events(bus_ctx);
    assert_int_equal(event_count, 7);

    event = GETI_ARG(&events, 0);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client.[cpe-Client-1].Subscription."));
    assert_true(test_verify_data(event, "index", "1"));
    assert_true(test_verify_data(event, "keys.Alias", "cpe-Subscription-1"));
    assert_true(test_verify_data(event, "name", "cpe-Subscription-1"));
    assert_true(test_verify_data(event, "notification", "dm:instance-removed"));
    assert_true(test_verify_data(event, "object", "MQTT.Client.cpe-Client-1.Subscription."));
    assert_true(test_verify_data(event, "path", "MQTT.Client.1.Subscription."));
    assert_true(test_verify_data(event, "parameters.Topic", "topic-test"));

    event = GETI_ARG(&events, 1);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client.[cpe-Client-1].Subscription."));
    assert_true(test_verify_data(event, "notification", "dm:object-removed"));
    assert_true(test_verify_data(event, "object", "MQTT.Client.cpe-Client-1.Subscription."));
    assert_true(test_verify_data(event, "path", "MQTT.Client.1.Subscription."));

    event = GETI_ARG(&events, 2);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client.[cpe-Client-1].Stats."));
    assert_true(test_verify_data(event, "notification", "dm:object-removed"));
    assert_true(test_verify_data(event, "object", "MQTT.Client.cpe-Client-1.Stats."));
    assert_true(test_verify_data(event, "path", "MQTT.Client.1.Stats."));

    event = GETI_ARG(&events, 3);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client.[cpe-Client-1].UserProperty."));
    assert_true(test_verify_data(event, "notification", "dm:object-removed"));
    assert_true(test_verify_data(event, "object", "MQTT.Client.cpe-Client-1.UserProperty."));
    assert_true(test_verify_data(event, "path", "MQTT.Client.1.UserProperty."));

    event = GETI_ARG(&events, 4);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client."));
    assert_true(test_verify_data(event, "index", "1"));
    assert_true(test_verify_data(event, "keys.Alias", "cpe-Client-1"));
    assert_true(test_verify_data(event, "name", "cpe-Client-1"));
    assert_true(test_verify_data(event, "notification", "dm:instance-removed"));
    assert_true(test_verify_data(event, "object", "MQTT.Client."));
    assert_true(test_verify_data(event, "path", "MQTT.Client."));
    assert_true(test_verify_data(event, "parameters.BrokerAddress", "test.mosquitto.org"));

    event = GETI_ARG(&events, 5);
    assert_true(test_verify_data(event, "eobject", "MQTT.Client.[cpe-Client-1]."));
    assert_true(test_verify_data(event, "notification", "dm:object-changed"));
    assert_true(test_verify_data(event, "parameters.SubscriptionNumberOfEntries.from", "1"));
    assert_true(test_verify_data(event, "parameters.SubscriptionNumberOfEntries.to", "0"));
    assert_true(test_verify_data(event, "object", "MQTT.Client.cpe-Client-1."));
    assert_true(test_verify_data(event, "path", "MQTT.Client.1."));

    event = GETI_ARG(&events, 6);
    assert_true(test_verify_data(event, "eobject", "MQTT."));
    assert_true(test_verify_data(event, "notification", "dm:object-changed"));
    assert_true(test_verify_data(event, "parameters.ClientNumberOfEntries.from", "1"));
    assert_true(test_verify_data(event, "parameters.ClientNumberOfEntries.to", "0"));
    assert_true(test_verify_data(event, "object", "MQTT."));
    assert_true(test_verify_data(event, "path", "MQTT."));

    event_count = 0;
    reset_events();

    assert_int_equal(amxb_unsubscribe(bus_ctx, "MQTT.", event_handler, &event_count), 0);
    usleep(100);

    amxc_var_clean(&ret);
}

/************************* OBUSPA tests *************************/

static void test_add_event_sub(const char* ref_list) {
    int retval = -1;
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&values);
    amxc_var_init(&ret);

    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "ReferenceList", ref_list);
    amxc_var_add_key(cstring_t, &values, "NotifType", "Event");
    amxc_var_add_key(bool, &values, "Enable", true);

    retval = amxb_add(test_get_bus_ctx(), "Device.LocalAgent.Subscription.", 0, NULL, &values, &ret, 5);
    assert_int_equal(retval, 0);

    amxc_var_clean(&ret);
    amxc_var_clean(&values);
}

static void test_del_all_subs(void) {
    int retval = -1;
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_del(test_get_bus_ctx(), "Device.LocalAgent.Subscription.*.", 0, NULL, &ret, 5);
    assert_int_equal(retval, 0);

    amxc_var_clean(&ret);
}

static void test_handle_notification(UNUSED const char* const sig_name,
                                     const amxc_var_t* const data,
                                     UNUSED void* const priv) {
    amxc_var_t* cmd_key = GET_ARG(data, "cmd_key");

    amxc_var_dump(data, STDOUT_FILENO);
    //amxut_util_write_to_json_file(data, "./results/obuspa_event.json");

    // Don't check cmd_key, because it will change between tests and we want to resuse the same
    // expected variant (we don't care about the command key anyway)
    amxc_var_delete(&cmd_key);
    check_expected(data);
}

void test_obuspa_subscribe_value_change(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    amxb_bus_ctx_t* ctx = test_get_bus_ctx();
    amxc_var_t* expected_notification = amxut_util_read_json_from_file("./results/obuspa_value_change.json");
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);

    retval = amxb_subscribe_ex(ctx, path, INT32_MAX, AMXB_BE_EVENT_TYPE_CHANGE, NULL, test_handle_notification, NULL);
    assert_int_equal(retval, 0);

    amxc_var_add_key(cstring_t, &args, "State", "Start");

    retval = amxb_set(ctx, path, &args, &ret, 5);
    assert_int_equal(retval, 0);

    sleep(1);
    expect_check(test_handle_notification, data, amxut_verify_variant_equal_check, expected_notification);
    handle_e2e_events(ctx);

    amxb_unsubscribe(ctx, path, test_handle_notification, NULL);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_obuspa_subscribe_object_creation(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.History.";
    amxb_bus_ctx_t* ctx = test_get_bus_ctx();
    amxc_var_t* expected_notification = amxut_util_read_json_from_file("./results/obuspa_object_creation.json");
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);

    retval = amxb_subscribe_ex(ctx, path, INT32_MAX, AMXB_BE_EVENT_TYPE_ADD, NULL, test_handle_notification, NULL);
    assert_int_equal(retval, 0);

    retval = amxb_add(ctx, path, 0, NULL, &args, &ret, 5);
    assert_int_equal(retval, 0);

    sleep(1);
    expect_check(test_handle_notification, data, amxut_verify_variant_equal_check, expected_notification);
    handle_e2e_events(ctx);

    amxb_unsubscribe(ctx, path, test_handle_notification, NULL);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_obuspa_subscribe_object_deletion(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.History.1.";
    amxb_bus_ctx_t* ctx = test_get_bus_ctx();
    amxc_var_t* expected_notification = amxut_util_read_json_from_file("./results/obuspa_object_deletion.json");
    amxc_var_t ret;

    amxc_var_init(&ret);

    retval = amxb_subscribe_ex(ctx, path, INT32_MAX, AMXB_BE_EVENT_TYPE_DEL, NULL, test_handle_notification, NULL);
    assert_int_equal(retval, 0);

    retval = amxb_del(ctx, path, 0, NULL, &ret, 5);
    assert_int_equal(retval, 0);

    sleep(1);
    expect_check(test_handle_notification, data, amxut_verify_variant_equal_check, expected_notification);
    handle_e2e_events(ctx);

    amxb_unsubscribe(ctx, path, test_handle_notification, NULL);

    amxc_var_clean(&ret);
}

void test_obuspa_subscribe_event(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    amxb_bus_ctx_t* ctx = test_get_bus_ctx();
    amxc_var_t* expected_notification = amxut_util_read_json_from_file("./results/obuspa_event.json");
    amxc_var_t* data = NULL;
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);

    // Note: we need to explicitly add the subscription with a USP Add on
    // Device.LocalAgent.Subscription, because AMX only passes the object path
    // to the backend (without the command) and OBUSPA does not allow
    // recursive subscriptions of type Event
    test_add_event_sub("Device.Greeter.MyEvent!");

    retval = amxb_subscribe_ex(ctx, path, INT32_MAX, AMXB_BE_EVENT_TYPE_EVENT, NULL, test_handle_notification, NULL);
    assert_int_equal(retval, 0);

    data = amxc_var_add_key(amxc_htable_t, &args, "data", NULL);
    amxc_var_add_key(bool, data, "Truth", true);
    amxc_var_add_key(uint32_t, data, "Number", 17);
    amxc_var_add_key(cstring_t, data, "Text", "Original");

    retval = amxb_call(ctx, path, "send_event()", &args, &ret, 5);
    assert_int_equal(retval, 0);

    sleep(1);
    expect_check(test_handle_notification, data, amxut_verify_variant_equal_check, expected_notification);
    handle_e2e_events(ctx);

    amxb_unsubscribe(ctx, path, test_handle_notification, NULL);

    test_del_all_subs();
    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}


