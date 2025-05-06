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
#include "test_amxb_usp_invoke.h"
#include "test_amxb_usp_common.h"
#include "imtp_mock.h"

#define UNUSED __attribute__((unused))

static amxo_parser_t parser;
static const char* odl_defs = "../test_dm.odl";
static amxb_usp_t* usp_ctx = NULL;
static amxp_signal_mngr_t sigmngr;
static amxd_dm_t dm;
static amxb_bus_ctx_t* bus_ctx = NULL;
static amxc_var_t* config = NULL;

static amxd_status_t _run_invoke(UNUSED amxd_object_t* object,
                                 UNUSED amxd_function_t* func,
                                 amxc_var_t* args,
                                 amxc_var_t* ret) {
    bool output_args = GET_BOOL(args, "output_args");
    bool reply_busy = GET_BOOL(args, "reply_busy");
    bool oper_failure = GET_BOOL(args, "oper_failure");

    printf("_run_invoke: dumping input args\n");
    fflush(stdout);
    amxc_var_dump(args, STDOUT_FILENO);

    if(output_args) {
        amxc_var_t* list = NULL;
        amxc_var_t* htable = NULL;
        amxc_var_set_type(ret, AMXC_VAR_ID_HTABLE);
        amxc_var_add_key(cstring_t, ret, "output-text", "foo");
        list = amxc_var_add_key(amxc_llist_t, ret, "output-list", NULL);
        amxc_var_add(uint32_t, list, 999);
        htable = amxc_var_add_key(amxc_htable_t, ret, "output-table", NULL);
        amxc_var_add_key(uint32_t, htable, "number", 999);
    }
    if(reply_busy) {
        amxc_var_set_type(ret, AMXC_VAR_ID_HTABLE);
        amxc_var_add_key(bool, ret, "busy", true);
    }
    if(oper_failure) {
        return amxd_status_unknown_error;
    }

    printf("_run_invoke: dumping output args\n");
    fflush(stdout);
    amxc_var_dump(ret, STDOUT_FILENO);
    return amxd_status_ok;
}

static void test_amxb_usp_async_done_cb(UNUSED const amxb_bus_ctx_t* bus_ctx,
                                        UNUSED amxb_request_t* req,
                                        int status,
                                        UNUSED void* priv) {
    printf("Async function done with status: %d\n", status);
}

int test_dm_setup(UNUSED void** state) {
    amxd_object_t* root_obj = NULL;
    amxc_var_t* usp_section = NULL;

    assert_int_equal(amxd_dm_init(&dm), 0);

    assert_int_equal(amxo_parser_init(&parser), 0);

    root_obj = amxd_dm_get_root(&dm);
    assert_non_null(root_obj);

    amxo_resolver_ftab_add(&parser, "run_invoke", AMXO_FUNC(_run_invoke));

    assert_int_equal(amxo_parser_parse_file(&parser, odl_defs, root_obj), 0);

    assert_int_equal(amxo_parser_invoke_entry_points(&parser, &dm, AMXO_START), 0);

    amxp_sigmngr_init(&sigmngr);

    amxc_var_new(&config);
    amxc_var_set_type(config, AMXC_VAR_ID_HTABLE);
    usp_section = amxc_var_add_key(amxc_htable_t, config, "usp", NULL);
    amxc_var_add_key(bool, usp_section, "requires-device-prefix", false);
    amxb_usp_set_config(usp_section);

    expect_any(__wrap_imtp_connection_connect, icon);
    expect_any(__wrap_imtp_connection_connect, from_uri);
    expect_any(__wrap_imtp_connection_connect, to_uri);
    will_return(__wrap_imtp_connection_connect, 99);
    will_return(__wrap_imtp_connection_connect, 0);
    // write handshake on connect
    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    usp_ctx = amxb_usp_connect(NULL, NULL, "/tmp/usp-ba.sock", &sigmngr);
    assert_ptr_not_equal(usp_ctx, NULL);

    // read handshake before sending register
    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_handshake);
    will_return(__wrap_imtp_connection_read_frame, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);
    assert_int_equal(amxb_usp_register(usp_ctx, &dm), 0);

    return 0;
}

int test_dm_teardown(UNUSED void** state) {
    expect_any(__wrap_imtp_connection_delete, icon);
    assert_int_equal(amxb_usp_disconnect(usp_ctx), 0);
    amxb_usp_free(usp_ctx);
    amxp_sigmngr_clean(&sigmngr);

    assert_int_equal(amxo_parser_invoke_entry_points(&parser, &dm, AMXO_STOP), 0);

    amxo_parser_clean(&parser);
    amxc_var_delete(&config);

    amxo_resolver_import_close_all();

    amxd_dm_clean(&dm);

    return 0;
}

int test_e2e_setup(UNUSED void** state) {
    assert_int_equal(system("amxrt ../amxb_usp_common/odl/test_config.odl ../amxb_usp_common/odl/tr181-mqtt_definition.odl ../amxb_usp_common/odl/tr181-mqtt_defaults.odl -D"), 0);
    sleep(1);

    assert_int_equal(amxb_be_load("../mod-amxb-test-usp.so"), 0);
    assert_int_equal(amxb_connect(&bus_ctx, "usp:/tmp/test.sock"), 0);

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

void test_amxb_usp_read_operate_sync(UNUSED void** state) {
    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_operate);
    will_return(__wrap_imtp_connection_read_frame, USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OUTPUT_ARGS);
    will_return(__wrap_imtp_connection_read_frame, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    assert_int_equal(amxb_usp_read(usp_ctx), 0);
}

void test_amxb_usp_read_operate_sync_failure(UNUSED void** state) {
    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_operate);
    will_return(__wrap_imtp_connection_read_frame, USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_CMD_FAILURE);
    will_return(__wrap_imtp_connection_read_frame, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    assert_int_equal(amxb_usp_read(usp_ctx), 0);
}

void test_amxb_usp_handle_operate_error(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t oper_var;

    amxc_var_init(&oper_var);
    // Create operate request var
    amxc_var_set_type(&oper_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &oper_var, "command", "dummy()");
    amxc_var_add_key(cstring_t, &oper_var, "command_key", "dummy_key");
    amxc_var_add_key(bool, &oper_var, "send_resp", true);

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_operate_new(usp_tx, &oper_var), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    usp_rx->msg->body->msg_body_case = USP__BODY__MSG_BODY_ERROR;
    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    // Test sending error message in case of wrong USP message
    assert_int_equal(amxb_usp_handle_operate(usp_ctx, usp_rx), 0);

    amxc_var_clean(&oper_var);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_amxb_usp_handle_operate_invalid(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t oper_var;

    amxc_var_init(&oper_var);

    // Create operate request var
    amxc_var_set_type(&oper_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &oper_var, "command", ".?.dummy.?.");
    amxc_var_add_key(cstring_t, &oper_var, "command_key", "dummy_key");
    amxc_var_add_key(bool, &oper_var, "send_resp", true);

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_operate_new(usp_tx, &oper_var), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    // Test handle operate with invalid input args
    assert_int_equal(amxb_usp_handle_operate(NULL, usp_rx), -1);
    assert_int_equal(amxb_usp_handle_operate(usp_ctx, NULL), -1);

    // Test handle operate with invalid command path
    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);
    assert_int_equal(amxb_usp_handle_operate(usp_ctx, usp_rx), 0);

    amxc_var_clean(&oper_var);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_amxb_usp_add_command_key_to_input_args(UNUSED void** state) {
    uspl_tx_t* usp_tx = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_var_t oper_var;

    amxc_var_init(&oper_var);

    // Create operate request var
    amxc_var_set_type(&oper_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &oper_var, "command", "test_root.child.1.run_invoke()");
    amxc_var_add_key(cstring_t, &oper_var, "command_key", "test_root.child.1.run_invoke-0");
    amxc_var_add_key(bool, &oper_var, "send_resp", true);

    assert_int_equal(uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver"), 0);
    assert_int_equal(uspl_operate_new(usp_tx, &oper_var), 0);
    usp_rx = uspl_msghandler_unpack_protobuf(usp_tx->pbuf, usp_tx->pbuf_len);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);
    assert_int_equal(amxb_usp_handle_operate(usp_ctx, usp_rx), 0);

    amxc_var_clean(&oper_var);
    uspl_tx_delete(&usp_tx);
    uspl_rx_delete(&usp_rx);
}

void test_amxb_usp_invoke_request_sync(UNUSED void** state) {
    amxb_invoke_t* invoke_ctx = calloc(1, sizeof(amxb_invoke_t));
    amxb_request_t* request = calloc(1, sizeof(amxb_request_t));
    amxc_var_t args;
    amxc_var_t* result = NULL;
    amxc_var_t* return_var = NULL;
    amxc_var_t* out_args_var = NULL;

    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "text", "hello");
    amxc_var_add_key(uint32_t, &args, "count", 12);

    amxc_var_new(&result);
    request->result = result;

    invoke_ctx->object = strdup("test_root.child.1.");
    invoke_ctx->method = strdup("run_invoke()");
    invoke_ctx->interface = NULL;

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_operate_resp);
    will_return(__wrap_imtp_connection_read_frame, USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OUTPUT_ARGS);
    will_return(__wrap_imtp_connection_read_frame, 0);

    assert_int_equal(amxb_usp_invoke(usp_ctx, invoke_ctx, &args, request, 1), 0);
    printf("Dumping result variant of amxb_usp_invoke\n");
    fflush(stdout);
    amxc_var_dump(request->result, STDOUT_FILENO);
    return_var = GETI_ARG(request->result, 0);
    assert_true(amxc_var_constcast(bool, return_var));

    out_args_var = GETI_ARG(request->result, 1);
    assert_null(GET_ARG(out_args_var, "_retval"));
    test_verify_data(out_args_var, "output-text", "foo");
    test_verify_data(out_args_var, "output-table.number", "999");
    test_verify_data(out_args_var, "output-list.0", "999");

    amxc_var_delete(&result);
    free(request);
    free(invoke_ctx->method);
    free(invoke_ctx->object);
    free(invoke_ctx);
    amxc_var_clean(&args);
}

void test_amxb_usp_invoke_request_sync_failure(UNUSED void** state) {
    amxb_invoke_t* invoke_ctx = calloc(1, sizeof(amxb_invoke_t));
    amxb_request_t* request = calloc(1, sizeof(amxb_request_t));
    amxc_var_t args;
    amxc_var_t* result = NULL;
    amxc_var_t* output_args = NULL;

    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "text", "hello");
    amxc_var_add_key(uint32_t, &args, "count", 12);

    amxc_var_new(&result);
    request->result = result;

    invoke_ctx->object = strdup("test_root.child.1.");
    invoke_ctx->method = strdup("run_invoke()");
    invoke_ctx->interface = NULL;

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_operate_resp);
    will_return(__wrap_imtp_connection_read_frame, USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_CMD_FAILURE);
    will_return(__wrap_imtp_connection_read_frame, 0);

    assert_int_not_equal(amxb_usp_invoke(usp_ctx, invoke_ctx, &args, request, 1), 0);
    printf("Dumping result variant of amxb_usp_invoke\n");
    fflush(stdout);
    amxc_var_dump(request->result, STDOUT_FILENO);
    output_args = GETI_ARG(request->result, 1);
    test_verify_data(output_args, "err_code", "7000");
    test_verify_data(output_args, "err_msg", "Generic error message");
    amxc_var_delete(&result);

    free(request);
    free(invoke_ctx->method);
    free(invoke_ctx->object);
    free(invoke_ctx);
    amxc_var_clean(&args);
}

void test_amxb_usp_invoke_async(UNUSED void** state) {
    amxb_invoke_t* invoke_ctx = calloc(1, sizeof(amxb_invoke_t));
    amxb_request_t* request = NULL;
    amxc_var_t args;
    amxc_var_t* result = NULL;

    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "text", "hello");
    amxc_var_add_key(uint32_t, &args, "count", 12);

    amxc_var_new(&result);
    amxb_request_new(&request);
    request->result = result;
    request->done_fn = test_amxb_usp_async_done_cb;

    invoke_ctx->object = strdup("test_root.child.1.");
    invoke_ctx->method = strdup("run_invoke()");
    invoke_ctx->interface = NULL;

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    assert_int_equal(amxb_usp_async_invoke(usp_ctx, invoke_ctx, &args, request), 0);

    // Read response
    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_operate_resp);
    will_return(__wrap_imtp_connection_read_frame, USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OUTPUT_ARGS);
    will_return(__wrap_imtp_connection_read_frame, 0);
    assert_int_equal(amxb_usp_read(usp_ctx), 0);

    imtp_mock_free_msg_id();
    amxb_close_request(&request);
    free(invoke_ctx->method);
    free(invoke_ctx->object);
    free(invoke_ctx);
    amxc_var_clean(&args);
}

static void test_e2e_sync_call_common(const char* object) {
    const char* method = "TestRpc";
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "text", "hello");
    amxc_var_add_key(uint32_t, &args, "number", 5);
    assert_int_equal(amxb_call(bus_ctx, object, method, &args, &ret, 5), 0);
    printf("Return variant\n");
    fflush(stdout);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_null(GETP_ARG(&ret, "1._retval"));
    assert_true(test_verify_data(&ret, "0", "5"));
    assert_true(test_verify_data(&ret, "1.combined", "hello - 5"));

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_e2e_sync_call(UNUSED void** state) {
    test_e2e_sync_call_common("MQTT.Client.1.");
}

static void test_async_call_done(UNUSED const amxb_bus_ctx_t* bus_ctx,
                                 amxb_request_t* req,
                                 UNUSED int status,
                                 UNUSED void* priv) {
    printf("test_async_call_done\n");
    fflush(stdout);
    amxc_var_dump(req->result, STDOUT_FILENO);
    assert_true(test_verify_data(req->result, "0", "5"));
    assert_true(test_verify_data(req->result, "1.combined", "hello - 5"));

    amxb_close_request(&req);
}

static void test_e2e_async_call_common(const char* object) {
    const char* method = "TestRpc";
    amxc_var_t args;
    amxb_request_t* request = NULL;

    amxc_var_init(&args);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "text", "hello");
    amxc_var_add_key(uint32_t, &args, "number", 5);
    request = amxb_async_call(bus_ctx, object, method, &args, test_async_call_done, NULL);
    assert_non_null(request);
    assert_int_equal(amxb_wait_for_request(request, 5), 0);

    amxc_var_clean(&args);
}

void test_e2e_async_call(UNUSED void** state) {
    test_e2e_async_call_common("MQTT.Client.1.");
}

void test_e2e_transl_sync_call(UNUSED void** state) {
    test_e2e_sync_call_common("Device.MQTT.Client.1.");
}

void test_e2e_transl_async_call(UNUSED void** state) {
    test_e2e_async_call_common("Device.MQTT.Client.1.");
}

void test_e2e_transl_sync_call_search_path(UNUSED void** state) {
    const char* object = "Device.MQTT.Client.*.";
    const char* method = "TestRpc";
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "text", "hello");
    amxc_var_add_key(uint32_t, &args, "number", 5);
    assert_int_equal(amxb_call(bus_ctx, object, method, &args, &ret, 5), 0);
    printf("Return variant\n");
    fflush(stdout);
    amxc_var_dump(&ret, STDOUT_FILENO);

    assert_null(GETP_ARG(&ret, "0.1._retval"));
    assert_true(test_verify_data(&ret, "0.0", "5"));
    assert_true(test_verify_data(&ret, "0.1.combined", "hello - 5"));
    assert_null(GETP_ARG(&ret, "1.1._retval"));
    assert_true(test_verify_data(&ret, "1.0", "5"));
    assert_true(test_verify_data(&ret, "1.1.combined", "hello - 5"));

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

static void test_async_call_done_search_path(UNUSED const amxb_bus_ctx_t* bus_ctx,
                                             amxb_request_t* req,
                                             UNUSED int status,
                                             UNUSED void* priv) {
    printf("test_async_call_done_search_path\n");
    fflush(stdout);
    amxc_var_dump(req->result, STDOUT_FILENO);
    assert_true(test_verify_data(req->result, "0.0", "5"));
    assert_true(test_verify_data(req->result, "0.1.combined", "hello - 5"));
    assert_true(test_verify_data(req->result, "1.0", "5"));
    assert_true(test_verify_data(req->result, "1.1.combined", "hello - 5"));

    amxb_close_request(&req);
}

void test_e2e_transl_async_call_search_path(UNUSED void** state) {
    const char* object = "Device.MQTT.Client.*.";
    const char* method = "TestRpc";
    amxc_var_t args;
    amxb_request_t* request = NULL;

    amxc_var_init(&args);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &args, "text", "hello");
    amxc_var_add_key(uint32_t, &args, "number", 5);
    request = amxb_async_call(bus_ctx, object, method, &args, test_async_call_done_search_path, NULL);
    assert_non_null(request);
    assert_int_equal(amxb_wait_for_request(request, 5), 0);

    amxc_var_clean(&args);
}

static void test_handle_notification(UNUSED const char* const sig_name,
                                     const amxc_var_t* const data,
                                     UNUSED void* const priv) {
    amxc_var_t* cmd_key = GET_ARG(data, "cmd_key");

    amxc_var_dump(data, STDOUT_FILENO);

    // Don't check cmd_key, because it will change between tests and we want to resuse the same
    // expected variant (we don't care about the command key anyway)
    amxc_var_delete(&cmd_key);
    check_expected(data);
    //amxut_util_write_to_json_file(data, "./results/e2e_transl_deferred_async_notification.json");
}

void test_e2e_transl_deferred_async_method(UNUSED void** state) {
    amxc_var_t* expected_notification = amxut_util_read_json_from_file("./results/e2e_transl_deferred_async_notification.json");
    amxc_var_t ret;
    amxc_var_t values;

    amxc_var_init(&ret);
    amxc_var_init(&values);

    assert_int_equal(amxb_subscribe(bus_ctx, "Device.MQTT.Client.1.", NULL, test_handle_notification, NULL), 0);

    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &values, "time", 1);
    amxc_var_add_key(cstring_t, &values, "message", "hello");
    assert_int_equal(amxb_call(bus_ctx, "Device.MQTT.Client.1.", "test_out_args_deferred", &values, &ret, 5), 0);

    amxc_var_dump(&ret, STDOUT_FILENO);

    sleep(2);
    expect_check(test_handle_notification, data, amxut_verify_variant_equal_check, expected_notification);
    handle_e2e_events(bus_ctx);
    assert_int_equal(amxb_unsubscribe(bus_ctx, "Device.MQTT.Client.1.", test_handle_notification, NULL), 0);

    amxc_var_clean(&values);
    amxc_var_clean(&ret);
}


/************************* OBUSPA tests *************************/

static void test_add_oper_complete_sub(const char* ref_list) {
    int retval = -1;
    amxc_var_t values;
    amxc_var_t ret;

    amxc_var_init(&values);
    amxc_var_init(&ret);

    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "ReferenceList", ref_list);
    amxc_var_add_key(cstring_t, &values, "NotifType", "OperationComplete");
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

static void test_variant_check(amxc_var_t* variant_to_check) {
    check_expected(variant_to_check);
}

static void test_out_args_done(UNUSED const amxb_bus_ctx_t* bus_ctx,
                               amxb_request_t* req,
                               UNUSED int status,
                               UNUSED void* priv) {
    amxc_var_dump(req->result, STDOUT_FILENO);
    test_variant_check(req->result);
    amxb_close_request(&req);
}

void test_obuspa_invoke_sync_method(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    const char* method = "test_out_args()";
    const char* results_file = "./results/obuspa_invoke_sync.json";
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);

    retval = amxb_call(test_get_bus_ctx(), path, method, &args, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

static void test_obuspa_invoke_async_method_common(bool subscribe_ex) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    const char* method = "test_out_args_deferred()";
    const char* results_file = "./results/obuspa_invoke_async.json";
    amxc_var_t* expected_notification = amxut_util_read_json_from_file("./results/obuspa_invoke_async_notification.json");
    amxb_bus_ctx_t* ctx = test_get_bus_ctx();
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "time", 1);

    // Note: we need to explicitly add the subscription with a USP Add on
    // Device.LocalAgent.Subscription, because AMX only passes the object path
    // to the backend (without the command) and OBUSPA does not allow
    // recursive subscriptions of type OperationComplete
    test_add_oper_complete_sub("Device.Greeter.test_out_args_deferred()");

    if(subscribe_ex) {
        retval = amxb_subscribe_ex(ctx, path, INT32_MAX, AMXB_BE_EVENT_TYPE_COMPL, NULL, test_handle_notification, NULL);
    } else {
        retval = amxb_subscribe(ctx, path, NULL, test_handle_notification, NULL);
    }
    assert_int_equal(retval, 0);

    retval = amxb_call(ctx, path, method, &args, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    // Give other process time to send notification
    sleep(2);
    expect_check(test_handle_notification, data, amxut_verify_variant_equal_check, expected_notification);
    handle_e2e_events(ctx);

    amxb_unsubscribe(ctx, path, test_handle_notification, NULL);

    test_del_all_subs();
    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_obuspa_invoke_async_method_subscribe(UNUSED void** state) {
    test_obuspa_invoke_async_method_common(false);
}

void test_obuspa_invoke_async_method_subscribe_ex(UNUSED void** state) {
    test_obuspa_invoke_async_method_common(true);
}

void test_obuspa_invoke_invalid_path(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeters.";
    const char* method = "test_out_args()";
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);

    retval = amxb_call(test_get_bus_ctx(), path, method, &args, &ret, 5);
    assert_int_equal(retval, amxd_status_invalid_path);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}

void test_obuspa_async_invoke_sync_method(UNUSED void** state) {
    amxb_request_t* req = NULL;
    const char* path = "Device.Greeter.";
    const char* method = "test_out_args()";
    amxc_var_t* expected_result = amxut_util_read_json_from_file("./results/obuspa_invoke_sync.json");
    amxb_bus_ctx_t* ctx = test_get_bus_ctx();
    amxc_var_t args;

    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);

    req = amxb_async_call(ctx, path, method, &args, test_out_args_done, NULL);
    assert_non_null(req);

    sleep(2);
    expect_check(test_variant_check, variant_to_check, amxut_verify_variant_equal_check, expected_result);
    handle_e2e_events(ctx);

    amxc_var_clean(&args);
}

static void test_obuspa_async_invoke_async_method_common(bool subscribe_ex) {
    amxb_request_t* req = NULL;
    int retval = -1;
    const char* path = "Device.Greeter.";
    const char* method = "test_out_args_deferred()";
    amxc_var_t* expected_result = amxut_util_read_json_from_file("./results/obuspa_invoke_async.json");
    amxc_var_t* expected_notification = amxut_util_read_json_from_file("./results/obuspa_invoke_async_notification.json");
    amxb_bus_ctx_t* ctx = test_get_bus_ctx();
    amxc_var_t args;

    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "time", 1);

    // Note: we need to explicitly add the subscription with a USP Add on
    // Device.LocalAgent.Subscription, because AMX only passes the object path
    // to the backend (without the command) and OBUSPA does not allow
    // recursive subscriptions of type OperationComplete
    test_add_oper_complete_sub("Device.Greeter.test_out_args_deferred()");

    if(subscribe_ex) {
        retval = amxb_subscribe_ex(ctx, path, INT32_MAX, AMXB_BE_EVENT_TYPE_COMPL, NULL, test_handle_notification, NULL);
    } else {
        retval = amxb_subscribe(ctx, path, NULL, test_handle_notification, NULL);
    }
    assert_int_equal(retval, 0);

    req = amxb_async_call(ctx, path, method, &args, test_out_args_done, NULL);
    assert_non_null(req);

    sleep(2);
    expect_check(test_variant_check, variant_to_check, amxut_verify_variant_equal_check, expected_result);
    expect_check(test_handle_notification, data, amxut_verify_variant_equal_check, expected_notification);
    handle_e2e_events(ctx);

    amxb_unsubscribe(test_get_bus_ctx(), path, test_handle_notification, NULL);
    test_del_all_subs();
    amxc_var_clean(&args);
}

void test_obuspa_async_invoke_async_method_subscribe(UNUSED void** state) {
    test_obuspa_async_invoke_async_method_common(false);
}

void test_obuspa_async_invoke_async_method_subscribe_ex(UNUSED void** state) {
    test_obuspa_async_invoke_async_method_common(true);
}

// This test ensures there is only a single notification if there are 2 subscriptions
// for different command paths
void test_obuspa_invoke_async_method_double_subscription(UNUSED void** state) {
    int retval = -1;
    const char* path = "Device.Greeter.";
    const char* method = "test_out_args_deferred()";
    const char* results_file = "./results/obuspa_invoke_async_method_double_subscription.json";
    amxc_var_t* expected_notification = amxut_util_read_json_from_file("./results/obuspa_invoke_async_notification.json");
    amxb_bus_ctx_t* ctx = test_get_bus_ctx();
    amxc_var_t ret;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &args, "time", 1);

    // Note: we need to explicitly add the subscription with a USP Add on
    // Device.LocalAgent.Subscription, because AMX only passes the object path
    // to the backend (without the command) and OBUSPA does not allow
    // recursive subscriptions of type OperationComplete
    test_add_oper_complete_sub("Device.Greeter.test_out_args_deferred()");
    test_add_oper_complete_sub("Device.Greeter.print_message()");

    retval = amxb_subscribe_ex(ctx, path, INT32_MAX, AMXB_BE_EVENT_TYPE_COMPL, NULL, test_handle_notification, NULL);
    assert_int_equal(retval, 0);

    retval = amxb_call(ctx, path, method, &args, &ret, 5);
    assert_int_equal(retval, 0);
    amxc_var_dump(&ret, STDOUT_FILENO);

    //amxut_util_write_to_json_file(&ret, results_file);
    assert_int_equal(amxut_verify_variant_from_json_file(&ret, results_file), 0);

    // Give other process time to send notification
    sleep(2);

    // Only provide 1 expect_check. In case the bug is triggered, there would be 2 matching
    // notifications causing the test to fail.
    expect_check(test_handle_notification, data, amxut_verify_variant_equal_check, expected_notification);
    handle_e2e_events(ctx);

    amxb_unsubscribe(ctx, path, test_handle_notification, NULL);

    test_del_all_subs();
    amxc_var_clean(&args);
    amxc_var_clean(&ret);
}