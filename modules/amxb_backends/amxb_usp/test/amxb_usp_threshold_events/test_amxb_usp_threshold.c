/****************************************************************************
**
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
** SPDX-FileCopyrightText: Copyright (c) 2024 SoftAtHome
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
#include "amxb_usp_la.h"
#include "test_amxb_usp_threshold.h"
#include "test_amxb_usp_common.h"
#include "imtp_mock.h"

#define UNUSED __attribute__((unused))

static amxb_bus_ctx_t* bus_ctx = NULL;
static amxc_var_t events;
static amxc_var_t* config = NULL;
static int event_count = 0;
static const char* threshold_instance = "Device.LocalAgent.Threshold.1.";

static void event_handler(const char* const sig_name,
                          const amxc_var_t* const data,
                          void* const priv) {
    int* event_count = (int*) priv;
    amxc_var_t* event = NULL;
    printf("threshold Unit test: Event received = %s\n", sig_name);
    fflush(stdout);
    amxc_var_dump(data, STDOUT_FILENO);
    assert_true(test_verify_data(data, "ParamPath", "Ethernet.Interface.1.Stats.BytesReceived"));
    if(0 == *event_count) {
        assert_true(test_verify_data(data, "ParamValue", "210"));
    } else if(1 == *event_count) {
        assert_true(test_verify_data(data, "ParamValue", "250"));
    } else if(2 == *event_count) {
        assert_true(test_verify_data(data, "ParamValue", "200"));
    } else if(3 == *event_count) {
        assert_true(test_verify_data(data, "ParamValue", "150"));
    } else if(4 == *event_count) {
        assert_true(test_verify_data(data, "ParamValue", "200"));
    }
    assert_true(test_verify_data(data, "notification", "Triggered!"));
    assert_true(test_verify_data(data, "path", "Device.LocalAgent.Threshold.1."));

    event = amxc_var_add_new(&events);
    amxc_var_copy(event, data);

    (*event_count)++;
    printf("event_count %d \n", *event_count);
    fflush(stdout);
}

int test_e2e_setup_threshold(UNUSED void** state) {

    amxc_var_t* usp_config = NULL;

    amxc_var_init(&events);
    amxc_var_new(&config);
    amxc_var_set_type(&events, AMXC_VAR_ID_LIST);
    amxc_var_set_type(config, AMXC_VAR_ID_HTABLE);
    usp_config = amxc_var_add_key(amxc_htable_t, config, "usp", NULL);
    amxc_var_add_key(bool, usp_config, "requires-device-prefix", false);

    // Launch Ethernet data model with LocalAgent data model
    assert_int_equal(system("amxrt ../amxb_usp_common/odl/tr181-mqtt_translations.odl " \
                            "../amxb_usp_common/odl/ethernet-definition.odl " \
                            "../amxb_usp_common/odl/ethernet-defaults.odl -D"), 0);
    sleep(1);

    assert_int_equal(amxb_be_load("../mod-amxb-test-usp.so"), 0);
    amxb_set_config(config);
    assert_int_equal(amxb_connect(&bus_ctx, "usp:/tmp/test.sock"), 0);
    usleep(100);
    assert_int_equal(amxb_read(bus_ctx), 0); // Read handshake

    return 0;
}

int test_e2e_teardown_threshold(UNUSED void** state) {
    amxb_disconnect(bus_ctx);
    amxb_free(&bus_ctx);

    assert_int_equal(system("killall amxrt"), 0);

    amxb_be_remove_all();
    amxc_var_delete(&config);
    amxc_var_clean(&events);
    return 0;
}

void test_e2e_threshold_instance_added(UNUSED void** state) {
    const char* object = "Device.LocalAgent.Threshold.";
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);

    assert_int_equal(amxb_get(bus_ctx, "Ethernet.", -1, &ret, 30), 0);

    printf("=================== subscribe to 'Device.LocalAgent.Threshold.' =======================\n");
    fflush(stdout);
    assert_int_equal(amxb_subscribe(bus_ctx, object, "notification == 'Triggered!'", event_handler, &event_count), 0);
    usleep(500);
    handle_events();

    printf("=================== add new threshold to 'Device.LocalAgent.Threshold.' =======================\n");
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "Alias", "ethernet-BytesReceived");
    amxc_var_add_key(bool, &args, "Enable", true);
    amxc_var_add_key(cstring_t, &args, "OperatingMode", "Normal");
    amxc_var_add_key(cstring_t, &args, "ReferencePath", "Ethernet.Interface.[Enable == true].Stats.");
    amxc_var_add_key(cstring_t, &args, "ThresholdParam", "BytesReceived");
    amxc_var_add_key(cstring_t, &args, "ThresholdValue", "200");
    amxc_var_add_key(cstring_t, &args, "ThresholdOperator", "Rise");

    assert_int_equal(amxb_add(bus_ctx, object, 0, NULL, &args, &ret, 5), amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);
    assert_true(test_verify_data(&ret, "0.index", "1"));
    assert_true(test_verify_data(&ret, "0.parameters.Alias", "ethernet-BytesReceived"));

    amxb_read(bus_ctx);
    handle_events();

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_threshold_instance_rised(UNUSED void** state) {
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);

    printf("=================== Change the value of 'Ethernet.Interface.1.Stats' to 210 =======================\n");
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "BytesReceived", 210);
    assert_int_equal(amxb_set(bus_ctx, "Ethernet.Interface.1.Stats", &args, &ret, 5), 0);
    usleep(500);
    amxb_read(bus_ctx);
    handle_events();
    assert_int_equal(event_count, 1);

    printf("=================== Change the value of 'Ethernet.Interface.1.Stats' to 190 then to 250 =======================\n");
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "BytesReceived", 190);
    assert_int_equal(amxb_set(bus_ctx, "Ethernet.Interface.1.Stats", &args, &ret, 5), 0);
    usleep(500);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "BytesReceived", 250);
    assert_int_equal(amxb_set(bus_ctx, "Ethernet.Interface.1.Stats", &args, &ret, 5), 0);
    usleep(500);
    amxb_read(bus_ctx);
    handle_events();
    assert_int_equal(event_count, 2);

    printf("=================== Dump the path 'Device.LocalAgent.' =======================\n");
    fflush(stdout);
    assert_int_equal(amxb_get(bus_ctx, "Device.LocalAgent.", -1, &ret, 5), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    usleep(10);
    printf("=================== Dump the path 'Ethernet.Interface.' =======================\n");
    fflush(stdout);
    assert_int_equal(amxb_get(bus_ctx, "Ethernet.Interface.", -1, &ret, 5), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    usleep(10);

    amxb_read(bus_ctx);
    handle_events();

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
    handle_events();
    sleep(1);
}


void test_e2e_threshold_change_mode_to_single(UNUSED void** state) {
    amxc_var_t args;
    amxc_var_t ret;

    amxc_var_init(&args);
    amxc_var_init(&ret);

    printf("=================== set parameter '%s' to '%s' in '%s' =======================\n", "ThresholdOperator", "Eq", threshold_instance);
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "ThresholdOperator", "Eq");
    assert_int_equal(amxb_set(bus_ctx, threshold_instance, &args, &ret, 5), 0);
    handle_events();

    printf("=================== Dump the path '%s' =======================\n", threshold_instance);
    fflush(stdout);
    assert_int_equal(amxb_get(bus_ctx, threshold_instance, -1, &ret, 10), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    printf("=================== Change the value of 'Ethernet.Interface.1.Stats' to 200 =======================\n");
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "BytesReceived", 200);
    assert_int_equal(amxb_set(bus_ctx, "Ethernet.Interface.1.Stats", &args, &ret, 5), 0);
    usleep(1000);
    amxb_read(bus_ctx);
    handle_events();
    assert_int_equal(event_count, 3);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_threshold_change_mode_to_fall(UNUSED void** state) {
    amxc_var_t args;
    amxc_var_t ret;

    amxc_var_init(&args);
    amxc_var_init(&ret);

    printf("=================== set parameter '%s' to '%s' in '%s' =======================\n", "ThresholdOperator", "Fall", threshold_instance);
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "ThresholdOperator", "Fall");
    assert_int_equal(amxb_set(bus_ctx, threshold_instance, &args, &ret, 5), 0);
    handle_events();

    printf("=================== Dump the path '%s' =======================\n", threshold_instance);
    fflush(stdout);
    assert_int_equal(amxb_get(bus_ctx, threshold_instance, -1, &ret, 10), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    printf("=================== Change the value of 'Ethernet.Interface.1.Stats' to 150 =======================\n");
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "BytesReceived", 150);
    assert_int_equal(amxb_set(bus_ctx, "Ethernet.Interface.1.Stats", &args, &ret, 5), 0);
    usleep(1000);
    amxb_read(bus_ctx);
    handle_events();
    assert_int_equal(event_count, 4);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_threshold_change_mode_to_NotEq(UNUSED void** state) {
    amxc_var_t args;
    amxc_var_t ret;

    amxc_var_init(&args);
    amxc_var_init(&ret);

    printf("=================== set parameter '%s' to '%s' in '%s' =======================\n", "ThresholdOperator", "NotEq", threshold_instance);
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "ThresholdOperator", "NotEq");
    amxc_var_add_key(cstring_t, &args, "ThresholdValue", "150");
    assert_int_equal(amxb_set(bus_ctx, threshold_instance, &args, &ret, 5), 0);
    handle_events();

    printf("=================== Dump the path '%s' =======================\n", threshold_instance);
    fflush(stdout);
    assert_int_equal(amxb_get(bus_ctx, threshold_instance, -1, &ret, 10), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    printf("=================== Change the value of 'Ethernet.Interface.1.Stats' to 200 =======================\n");
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "BytesReceived", 200);
    assert_int_equal(amxb_set(bus_ctx, "Ethernet.Interface.1.Stats", &args, &ret, 5), 0);
    usleep(1000);
    amxb_read(bus_ctx);
    handle_events();
    assert_int_equal(event_count, 5);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_threshold_change_mode_to_cross(UNUSED void** state) {
    amxc_var_t args;
    amxc_var_t ret;

    amxc_var_init(&args);
    amxc_var_init(&ret);

    printf("=================== set parameter '%s' to '%s' in '%s' =======================\n", "ThresholdOperator", "Cross", threshold_instance);
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "ThresholdOperator", "Cross");
    amxc_var_add_key(cstring_t, &args, "ThresholdValue", "300");
    assert_int_equal(amxb_set(bus_ctx, threshold_instance, &args, &ret, 5), 0);
    handle_events();

    printf("=================== Dump the path '%s' =======================\n", threshold_instance);
    fflush(stdout);
    assert_int_equal(amxb_get(bus_ctx, threshold_instance, -1, &ret, 10), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    printf("=================== Change the value of 'Ethernet.Interface.1.Stats' to 350 =======================\n");
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "BytesReceived", 350);
    assert_int_equal(amxb_set(bus_ctx, "Ethernet.Interface.1.Stats", &args, &ret, 5), 0);
    usleep(1000);
    amxb_read(bus_ctx);
    handle_events();
    assert_int_equal(event_count, 6);

    printf("=================== Change the value of 'Ethernet.Interface.1.Stats' to 290 =======================\n");
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "BytesReceived", 290);
    assert_int_equal(amxb_set(bus_ctx, "Ethernet.Interface.1.Stats", &args, &ret, 5), 0);
    usleep(1000);
    amxb_read(bus_ctx);
    handle_events();
    assert_int_equal(event_count, 7);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}


void test_e2e_threshold_instance_deleted(UNUSED void** state) {
    const char* object = "Device.LocalAgent.Threshold.";
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&args);
    amxc_var_init(&ret);

    printf("=================== Delete threshold to 'Device.LocalAgent.Threshold.' =======================\n");
    fflush(stdout);
    assert_int_equal(amxb_del(bus_ctx, threshold_instance, 0, NULL, &ret, 5), amxd_status_ok);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_true(test_verify_data(&ret, "0", "Device.LocalAgent.Threshold.1."));

    printf("=================== Dump the path 'Device.LocalAgent.' after threshold instance deletion ==================\n");
    fflush(stdout);
    assert_int_equal(amxb_get(bus_ctx, "Device.LocalAgent.", -1, &ret, 5), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    usleep(10);

    printf("=================== Change the value of 'Ethernet.Interface.1.Stats' to 350 =======================\n");
    fflush(stdout);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "BytesReceived", 350);
    assert_int_equal(amxb_set(bus_ctx, "Ethernet.Interface.1.Stats", &args, &ret, 5), 0);
    usleep(1000);
    amxb_read(bus_ctx);
    handle_events();

    printf("=================== No event should be triggered since the threshold instace is deleted =======================\n");
    fflush(stdout);
    assert_int_equal(event_count, 7);

    printf("=================== Dump the path '%s' =======================\n", "Ethernet.Interface.1.");
    fflush(stdout);
    assert_int_equal(amxb_get(bus_ctx, "Ethernet.Interface.1.", -1, &ret, 10), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    printf("=================== unsubscribe to 'Device.LocalAgent.Threshold.' =======================\n");
    fflush(stdout);
    assert_int_equal(amxb_unsubscribe(bus_ctx, object, event_handler, &event_count), 0);
    usleep(500);
    handle_events();

    printf("=================== End: Dump the path 'Device.LocalAgent.' =======================\n");
    fflush(stdout);
    assert_int_equal(amxb_get(bus_ctx, "Device.LocalAgent.", -1, &ret, 5), 0);
    amxc_var_dump(&ret, STDOUT_FILENO);
    usleep(10);

    amxb_read(bus_ctx);
    handle_events();
    usleep(100);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}
