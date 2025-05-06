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
#include <setjmp.h>
#include <cmocka.h>

#include <yajl/yajl_gen.h>

#include <amxc/amxc.h>
#include <amxj/amxj_variant.h>
#include <amxut/amxut_util.h>

#include "amxb_usp.h"
#include "test_amxb_usp_common.h"
#include "io_mock.h"
#include "imtp_mock.h"

static amxd_dm_t dm;
static amxo_parser_t parser;
static const char* odl_defs = "../test_dm.odl";
static amxp_signal_mngr_t sigmngr;
static amxc_var_t* config = NULL;
static int sfd = 0;
amxb_bus_ctx_t* obuspa_ctx = NULL;

bool test_verify_data(const amxc_var_t* data, const char* field, const char* value) {
    bool rv = false;
    char* field_value = NULL;
    amxc_var_t* field_data = GETP_ARG(data, field);

    printf("Verify event data: check field [%s] contains [%s]\n", field, value);
    fflush(stdout);
    assert_non_null(field_data);

    field_value = amxc_var_dyncast(cstring_t, field_data);
    assert_non_null(field_data);

    rv = (strcmp(field_value, value) == 0);

    free(field_value);
    return rv;
}

void handle_events(void) {
    while(amxp_signal_read() == 0) {
        printf(".");
    }
    printf("\n");
}

void handle_e2e_events(amxb_bus_ctx_t* bus_ctx) {
    usleep(100);
    printf("Handling events \n");
    while(amxb_read(bus_ctx) >= 0) {
        printf("R");
    }
    while(amxp_signal_read() == 0) {
        printf(".");
    }
    printf("\n");
    fflush(stdout);
}

void capture_sigalrm(void) {
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);

    sigprocmask(SIG_BLOCK, &mask, NULL);

    sfd = signalfd(-1, &mask, 0);
}

void read_sigalrm(uint32_t timeout) {
    struct signalfd_siginfo fdsi;
    ssize_t s;
    fd_set set;
    struct timeval ts;

    ts.tv_sec = timeout;
    ts.tv_usec = 0;

    FD_ZERO(&set);
    FD_SET(sfd, &set);
    int rv = select(sfd + 1, &set, NULL, NULL, &ts);
    if(rv > 0) {
        s = __real_read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
    } else {
        printf("Timeout\n");
        return;
    }

    assert_int_equal(s, sizeof(struct signalfd_siginfo));
    if(fdsi.ssi_signo == SIGALRM) {
        printf("Got SIGALRM\n");
        fflush(stdout);
    } else {
        printf("Read unexpected signal\n");
        fflush(stdout);
    }
}

amxd_dm_t* test_get_dm(void) {
    return &dm;
}

int test_variant_equal_check(const LargestIntegralType value, const LargestIntegralType check_value_data) {
    amxc_var_t* actual_variant = (amxc_var_t*) value;
    amxc_var_t* expected_variant = (amxc_var_t*) check_value_data;
    int retval = 0;

    amxc_var_compare(expected_variant, actual_variant, &retval);
    if(retval != 0) {
        printf("***** Unexpected data recieved *****\n");
        printf("EXPECTED:\n");
        fflush(stdout);
        amxc_var_dump(expected_variant, STDOUT_FILENO);
        printf("GOT:\n");
        fflush(stdout);
        amxc_var_dump(actual_variant, STDOUT_FILENO);
        amxc_var_cast(actual_variant, AMXC_VAR_ID_JSON);
        printf("JSON:\n");
        fflush(stdout);
        amxc_var_dump(actual_variant, STDOUT_FILENO);
        printf("************************************\n");
    }

    amxc_var_delete(&expected_variant);
    return 1;
}

void test_write_json_to_file(const amxc_var_t* const json_var, const char* dest_file) {
    variant_json_t* writer = NULL;
    int fd = -1;

    amxj_writer_new(&writer, json_var);

    fd = open(dest_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    when_true(fd < 0, exit);

    amxj_write(writer, fd);

exit:
    amxj_writer_delete(&writer);
    if(fd != -1) {
        close(fd);
    }
}

int test_common_setup(amxb_usp_t** usp_ctx) {
    amxd_object_t* root_obj = NULL;
    amxc_var_t* usp_section = NULL;

    assert_int_equal(amxd_dm_init(&dm), 0);

    assert_int_equal(amxo_parser_init(&parser), 0);

    root_obj = amxd_dm_get_root(&dm);
    assert_non_null(root_obj);

    assert_int_equal(amxo_parser_parse_file(&parser, odl_defs, root_obj), 0);

    assert_int_equal(amxo_parser_invoke_entry_points(&parser, &dm, AMXO_START), 0);

    amxp_sigmngr_init(&sigmngr);

    amxc_var_new(&config);
    amxc_var_set_type(config, AMXC_VAR_ID_HTABLE);
    usp_section = amxc_var_add_key(amxc_htable_t, config, "usp", NULL);
    amxc_var_add_key(bool, usp_section, AMXB_USP_COPT_REQUIRES_DEVICE, false);
    amxb_usp_set_config(usp_section);

    expect_any(__wrap_imtp_connection_connect, icon);
    expect_any(__wrap_imtp_connection_connect, from_uri);
    expect_any(__wrap_imtp_connection_connect, to_uri);
    will_return(__wrap_imtp_connection_connect, 99);
    will_return(__wrap_imtp_connection_connect, 0);
    // write handshake on connect
    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    *usp_ctx = amxb_usp_connect(NULL, NULL, "/tmp/usp-ba.sock", &sigmngr);
    assert_ptr_not_equal(*usp_ctx, NULL);

    // read handshake before sending register
    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_handshake);
    will_return(__wrap_imtp_connection_read_frame, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);
    assert_int_equal(amxb_usp_register(*usp_ctx, &dm), 0);

    return 0;
}

int test_common_teardown(amxb_usp_t** usp_ctx) {
    expect_any(__wrap_imtp_connection_delete, icon);
    assert_int_equal(amxb_usp_disconnect(*usp_ctx), 0);
    amxb_usp_free(*usp_ctx);
    amxp_sigmngr_clean(&sigmngr);

    assert_int_equal(amxo_parser_invoke_entry_points(&parser, &dm, AMXO_STOP), 0);

    amxo_parser_clean(&parser);
    amxc_var_delete(&config);

    amxo_resolver_import_close_all();

    amxd_dm_clean(&dm);

    return 0;
}

int test_common_e2e_proxy_setup(amxb_bus_ctx_t** bus_ctx) {
    // Launch MQTT data model that will be proxied
    assert_int_equal(system("amxrt ../amxb_usp_common/odl/test_config.odl ../amxb_usp_common/odl/tr181-mqtt_definition.odl ../amxb_usp_common/odl/tr181-mqtt_defaults.odl -D"), 0);
    sleep(1);

    // Launch proxy
    assert_int_equal(system("amxrt ../amxb_usp_common/odl/device-proxy.odl -D"), 0);
    sleep(1);

    // Load backend
    assert_int_equal(amxb_be_load("../mod-amxb-test-usp.so"), 0);

    // Make sure config is set
    amxc_var_new(&config);
    amxc_var_set_type(config, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(amxc_htable_t, config, "usp", NULL);
    amxb_set_config(config);

    // Connect to USP socket
    assert_int_equal(amxb_connect(bus_ctx, "usp:/tmp/test-device.sock"), 0);
    sleep(1);

    return 0;
}

int test_common_e2e_proxy_teardown(amxb_bus_ctx_t** bus_ctx) {
    amxc_var_delete(&config);
    amxb_disconnect(*bus_ctx);
    amxb_free(bus_ctx);

    assert_int_equal(system("killall amxrt"), 0);

    amxb_be_remove_all();
    return 0;
}

int test_common_e2e_transl_setup(amxb_bus_ctx_t** bus_ctx) {
    // Launch MQTT data model that will do its own translations
    assert_int_equal(system("amxrt ../amxb_usp_common/odl/tr181-mqtt_translations.odl ../amxb_usp_common/odl/tr181-mqtt_definition.odl ../amxb_usp_common/odl/tr181-mqtt_defaults.odl -D"), 0);
    sleep(1);

    // Load backend
    assert_int_equal(amxb_be_load("../mod-amxb-test-usp.so"), 0);

    // Make sure config is set
    amxc_var_new(&config);
    amxc_var_set_type(config, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(amxc_htable_t, config, "usp", NULL);
    amxb_set_config(config);

    // Connect to USP socket
    assert_int_equal(amxb_connect(bus_ctx, "usp:/tmp/test.sock"), 0);
    sleep(1);

    return 0;
}

int test_common_e2e_transl_teardown(amxb_bus_ctx_t** bus_ctx) {
    return test_common_e2e_proxy_teardown(bus_ctx);
}

int test_common_obuspa_setup(UNUSED void** state) {
    amxc_var_t* usp_section = NULL;
    amxc_var_t ret;

    amxc_var_init(&ret);

    // Launch obuspa
    assert_int_equal(system("obuspa -f /tmp/usp_broker.db -s /tmp/broker_cli -p -v 4 -r ../obuspa/broker_reset.txt &"), 0);
    sleep(3);

    assert_int_equal(system("echo obuspa pid=$(pidof obuspa)"), 0);

    // Launch Greeter DM that will play the role of USP Service connected to obuspa
    assert_int_equal(system("amxrt ../amxb_usp_common/odl/greeter_obuspa.odl -D"), 0);
    sleep(3);

    // Load backend
    assert_int_equal(amxb_be_load("../mod-amxb-test-usp.so"), 0);

    // Make sure config is set
    amxc_var_new(&config);
    amxc_var_set_type(config, AMXC_VAR_ID_HTABLE);
    usp_section = amxc_var_add_key(amxc_htable_t, config, "usp", NULL);
    amxc_var_add_key(cstring_t, usp_section, "EndpointID", "proto::test-controller");
    amxb_set_config(config);

    // Connect to obuspa agent socket
    assert_int_equal(amxb_connect(&obuspa_ctx, "usp:/tmp/broker_agent_path"), 0);
    while(true) {
        amxb_get(obuspa_ctx, "Device.Greeter.", 0, &ret, 5);
        amxc_var_dump(&ret, 1);
        if(GETP_ARG(&ret, "0.0") == NULL) {
            printf("Failed to get Device.Greeter, sleeping...\n");
            sleep(1);
        } else {
            printf("Found Device.Greeter.\n");
            break;
        }
        fflush(stdout);
    }

    amxc_var_clean(&ret);
    return 0;
}

int test_common_obuspa_teardown(UNUSED void** state) {
    amxc_var_delete(&config);
    amxb_disconnect(obuspa_ctx);
    amxb_free(&obuspa_ctx);

    assert_int_equal(system("killall amxrt"), 0);
    assert_int_equal(system("killall obuspa"), 0);
    assert_int_equal(system("rm -f /tmp/broker_cli"), 0);
    assert_int_equal(system("rm -f /tmp/broker_agent_path"), 0);
    assert_int_equal(system("rm -f /tmp/broker_controller_path"), 0);
    assert_int_equal(system("rm -f /tmp/usp_broker.db"), 0);

    amxb_be_remove_all();
    return 0;
}

amxb_bus_ctx_t* test_get_bus_ctx(void) {
    return obuspa_ctx;
}
