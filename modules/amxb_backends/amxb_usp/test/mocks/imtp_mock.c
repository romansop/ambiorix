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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <amxc/amxc_macros.h>
#include <usp/uspl.h>

#include "imtp_mock.h"

static imtp_connection_t* con_accepted = NULL;
static char* last_msg_id = NULL;

static void mock_get(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t request;
    amxc_var_t* paths = NULL;
    uint32_t max_depth = 20;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&request);
    amxc_var_set_type(&request, AMXC_VAR_ID_HTABLE);
    paths = amxc_var_add_key(amxc_llist_t, &request, "paths", NULL);
    amxc_var_add(cstring_t, paths, "test_root.child.1.");
    amxc_var_add(cstring_t, paths, "test_root.child.2.");
    amxc_var_add_key(uint32_t, &request, "max_depth", max_depth);

    // Create protobuf
    uspl_get_new(usp_tx, &request);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&request);
    uspl_tx_delete(&usp_tx);
}

static void mock_get_resp(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t ret;
    amxc_var_t* result;
    amxc_var_t* resolved_path;
    amxc_llist_t resp_list;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_HTABLE);
    amxc_llist_init(&resp_list);

    // Add result variant
    result = amxc_var_add_key(amxc_htable_t, &ret, "result", NULL);
    amxc_var_add_key(cstring_t, result, "requested_path", "test_root.child.1.");
    amxc_var_add_key(int64_t, result, "err_code", 0);
    resolved_path = amxc_var_add_key(amxc_htable_t, &ret, "test_root.child.1.", NULL);
    amxc_var_add_key(cstring_t, resolved_path, "Alias", "One");
    amxc_var_add_key(cstring_t, resolved_path, "Name", "Child One");

    amxc_llist_append(&resp_list, &ret.lit);

    // Create protobuf
    uspl_get_resp_new(usp_tx, &resp_list, last_msg_id);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&ret);
    uspl_tx_delete(&usp_tx);
}

static void mock_set(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* record = NULL;
    amxc_var_t parent_table;
    amxc_var_t* requests = NULL;
    amxc_var_t* request = NULL;
    amxc_var_t* params_list = NULL;
    amxc_var_t* params_entry = NULL;

    uspl_tx_new(&record, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&parent_table);
    amxc_var_set_type(&parent_table, AMXC_VAR_ID_HTABLE);
    requests = amxc_var_add_key(amxc_llist_t, &parent_table, "requests", NULL);
    request = amxc_var_add(amxc_htable_t, requests, NULL);
    amxc_var_add_key(cstring_t, request, "object_path", "test_root.child.2.box.1.");
    params_list = amxc_var_add_key(amxc_llist_t, request, "parameters", NULL);
    params_entry = amxc_var_add(amxc_htable_t, params_list, NULL);
    amxc_var_add_key(cstring_t, params_entry, "param", "length");
    amxc_var_add_key(uint32_t, params_entry, "value", 75);
    amxc_var_add_key(bool, params_entry, "required", true);
    params_entry = amxc_var_add(amxc_htable_t, params_list, NULL);
    amxc_var_add_key(cstring_t, params_entry, "param", "liar");
    amxc_var_add_key(bool, params_entry, "value", true);
    amxc_var_add_key(bool, params_entry, "required", true);

    amxc_var_dump(&parent_table, STDOUT_FILENO);

    // Create protobuf
    uspl_set_new(record, &parent_table);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, record->pbuf_len, record->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    uspl_tx_delete(&record);
    amxc_var_clean(&parent_table);
}

static void mock_set_resp(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t ret;
    amxc_var_t* success = NULL;
    amxc_var_t* updated_inst_results = NULL;
    amxc_var_t* result = NULL;
    amxc_var_t* updated_params = NULL;
    amxc_llist_t resp_list;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_HTABLE);
    amxc_llist_init(&resp_list);

    amxc_var_add_key(cstring_t, &ret, "requested_path", "test_root.child.1.");
    amxc_var_add_key(uint32_t, &ret, "oper_status", USP__SET_RESP__UPDATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS);
    success = amxc_var_add_key(amxc_htable_t, &ret, "success", NULL);
    updated_inst_results = amxc_var_add_key(amxc_llist_t, success, "updated_inst_results", NULL);
    result = amxc_var_add(amxc_htable_t, updated_inst_results, NULL);
    amxc_var_add_key(cstring_t, result, "affected_path", "test_root.child.1.");
    updated_params = amxc_var_add_key(amxc_htable_t, result, "updated_params", NULL);
    amxc_var_add_key(cstring_t, updated_params, "Name", "Super dummy");

    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_llist_append(&resp_list, &ret.lit);

    // Create protobuf
    uspl_set_resp_new(usp_tx, &resp_list, last_msg_id);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&ret);
    uspl_tx_delete(&usp_tx);
}

static void mock_add(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t add_var;
    amxc_var_t* requests = NULL;
    amxc_var_t* request = NULL;
    amxc_var_t* params = NULL;
    amxc_var_t* param = NULL;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&add_var);
    amxc_var_set_type(&add_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(bool, &add_var, "allow_partial", false);
    requests = amxc_var_add_key(amxc_llist_t, &add_var, "requests", NULL);

    request = amxc_var_add(amxc_htable_t, requests, NULL);
    amxc_var_add_key(cstring_t, request, "object_path", "test_root.child.");
    params = amxc_var_add_key(amxc_llist_t, request, "parameters", NULL);

    param = amxc_var_add(amxc_htable_t, params, NULL);
    amxc_var_add_key(cstring_t, param, "param", "Alias");
    amxc_var_add_key(cstring_t, param, "value", "AddedChild");
    amxc_var_add_key(bool, param, "required", true);

    param = amxc_var_add(amxc_htable_t, params, NULL);
    amxc_var_add_key(cstring_t, param, "param", "Name");
    amxc_var_add_key(cstring_t, param, "value", "NewChild");
    amxc_var_add_key(bool, param, "required", true);

    // Create protobuf
    uspl_add_new(usp_tx, &add_var);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&add_var);
    uspl_tx_delete(&usp_tx);
}

static void mock_add_resp(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t ret;
    amxc_var_t* result = NULL;
    amxc_llist_t resp_list;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_HTABLE);
    amxc_llist_init(&resp_list);

    // Add result variant
    amxc_var_add_key(uint32_t, &ret, "index", 2);
    amxc_var_add_key(cstring_t, &ret, "name", "2");
    amxc_var_add_key(cstring_t, &ret, "object", "test_root.child.2.");
    amxc_var_add_key(amxc_htable_t, &ret, "parameters", NULL);
    amxc_var_add_key(cstring_t, &ret, "path", "test_root.child.2.");
    result = amxc_var_add_key(amxc_htable_t, &ret, "result", NULL);
    amxc_var_add_key(cstring_t, result, "requested_path", "test_root.child.");
    amxc_var_add_key(uint32_t, result, "err_code", USP_ERR_OK);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_llist_append(&resp_list, &ret.lit);

    // Create protobuf
    uspl_add_resp_new(usp_tx, &resp_list, last_msg_id);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&ret);
    uspl_tx_delete(&usp_tx);
}

static void mock_delete(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t delete_var;
    amxc_var_t* requests = NULL;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&delete_var);
    amxc_var_set_type(&delete_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(bool, &delete_var, "allow_partial", false);
    requests = amxc_var_add_key(amxc_llist_t, &delete_var, "requests", NULL);
    amxc_var_add(cstring_t, requests, "test_root.child.[Alias == \"Dummy\"].");

    // Create protobuf
    uspl_delete_new(usp_tx, &delete_var);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&delete_var);
    uspl_tx_delete(&usp_tx);
}

static void mock_delete_resp(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t ret;
    amxc_var_t* result = NULL;
    amxc_var_t* affected_paths = NULL;
    amxc_llist_t resp_list;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_HTABLE);
    amxc_llist_init(&resp_list);

    // Add result variant
    result = amxc_var_add_key(amxc_htable_t, &ret, "result", NULL);
    amxc_var_add_key(uint32_t, result, "err_code", 0);
    amxc_var_add_key(cstring_t, result, "requested_path", "test_root.child.1.");

    affected_paths = amxc_var_add_key(amxc_llist_t, &ret, "affected_paths", NULL);
    amxc_var_add_key(amxc_llist_t, &ret, "unaffected_paths", NULL);
    amxc_var_add(cstring_t, affected_paths, "test_root.child.1.");
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_llist_append(&resp_list, &ret.lit);

    // Create protobuf
    uspl_delete_resp_new(usp_tx, &resp_list, last_msg_id);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&ret);
    uspl_tx_delete(&usp_tx);
}

static void mock_get_supported_dm(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t request;
    amxc_var_t* flags;
    amxc_var_t* paths;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&request);
    amxc_var_set_type(&request, AMXC_VAR_ID_HTABLE);
    flags = amxc_var_add_key(amxc_htable_t, &request, "flags", NULL);
    amxc_var_add_key(bool, flags, "first_level_only", true);
    amxc_var_add_key(bool, flags, "return_commands", true);
    amxc_var_add_key(bool, flags, "return_params", true);
    paths = amxc_var_add_key(amxc_llist_t, &request, "paths", NULL);
    amxc_var_add(cstring_t, paths, "test_root.");

    // Create protobuf
    uspl_get_supported_dm_new(usp_tx, &request);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&request);
    uspl_tx_delete(&usp_tx);
}

static void mock_get_supported_dm_resp(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t ret;
    amxc_var_t* result = NULL;
    amxc_var_t* supported_obj = NULL;
    amxc_llist_t resp_list;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_HTABLE);
    amxc_llist_init(&resp_list);

    // Add result variant
    result = amxc_var_add_key(amxc_htable_t, &ret, "result", NULL);
    amxc_var_add_key(uint32_t, result, "err_code", 0);
    amxc_var_add_key(cstring_t, result, "requested_path", "test_root.child.1.");

    supported_obj = amxc_var_add_key(amxc_htable_t, &ret, "test_root.", NULL);
    amxc_var_add_key(bool, supported_obj, "is_multi_instance", false);
    amxc_var_add_key(amxc_llist_t, supported_obj, "supported_commands", NULL);
    amxc_var_add_key(amxc_llist_t, supported_obj, "supported_params", NULL);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_llist_append(&resp_list, &ret.lit);

    // Create protobuf
    uspl_get_supported_dm_resp_new(usp_tx, &resp_list, last_msg_id);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&ret);
    uspl_tx_delete(&usp_tx);
}

static void mock_operate(imtp_frame_t** frame, int option) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t request;
    amxc_var_t* input_args = NULL;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&request);
    amxc_var_set_type(&request, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &request, "command", "test_root.child.1.run_invoke");
    amxc_var_add_key(cstring_t, &request, "command_key", "test_root.child.1.run_invoke-0");
    amxc_var_add_key(bool, &request, "send_resp", true);

    // Always add mandatory input arguments
    input_args = amxc_var_add_key(amxc_htable_t, &request, "input_args", NULL);
    amxc_var_add_key(cstring_t, input_args, "text", "hello");
    amxc_var_add_key(uint32_t, input_args, "count", 12);

    switch(option) {
    case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OUTPUT_ARGS:
        amxc_var_add_key(bool, input_args, "output_args", true);
        break;
    case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OBJ_PATH:
        amxc_var_add_key(bool, input_args, "reply_busy", true);
        break;
    case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_CMD_FAILURE:
        amxc_var_add_key(bool, input_args, "oper_failure", true);
        break;
    default:
        break;
    }

    // Create protobuf
    uspl_operate_new(usp_tx, &request);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&request);
    uspl_tx_delete(&usp_tx);
}

static void mock_operate_resp(imtp_frame_t** frame, int option) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t ret;
    amxc_llist_t resp_list;
    char* msg_id = imtp_mock_get_msg_id();

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&ret);
    amxc_llist_init(&resp_list);

    // Add result variant
    amxc_var_set_type(&ret, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &ret, "executed_command", "test_root.child.1.run_invoke");
    amxc_var_add_key(uint32_t, &ret, "operation_resp_case", option);

    if(option == USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OUTPUT_ARGS) {
        amxc_var_t* output_args = NULL;
        amxc_var_t* list_arg = NULL;
        amxc_var_t* htable_arg = NULL;

        output_args = amxc_var_add_key(amxc_htable_t, &ret, "output_args", NULL);
        amxc_var_add_key(bool, output_args, "_retval", true);
        amxc_var_add_key(cstring_t, output_args, "output-text", "foo");
        list_arg = amxc_var_add_key(amxc_llist_t, output_args, "output-list", NULL);
        amxc_var_add(uint32_t, list_arg, 999);
        htable_arg = amxc_var_add_key(amxc_htable_t, output_args, "output-table", NULL);
        amxc_var_add_key(uint32_t, htable_arg, "number", 999);
    } else if(option == USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_CMD_FAILURE) {
        amxc_var_t* cmd_failure = amxc_var_add_key(amxc_htable_t, &ret, "cmd_failure", NULL);
        amxc_var_add_key(uint32_t, cmd_failure, "err_code", 7000);
        amxc_var_add_key(cstring_t, cmd_failure, "err_msg", "Generic error message");
    }

    printf("Dumping created variant in function mock_operate_resp\n");
    fflush(stdout);
    amxc_var_dump(&ret, STDOUT_FILENO);

    amxc_llist_append(&resp_list, &ret.lit);

    // Create protobuf
    if(msg_id != NULL) {
        uspl_operate_resp_new(usp_tx, &resp_list, msg_id);
    } else {
        uspl_operate_resp_new(usp_tx, &resp_list, last_msg_id);
    }

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&ret);
    uspl_tx_delete(&usp_tx);
}

static void mock_get_instances(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t request;
    amxc_var_t* obj_paths = NULL;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_var_init(&request);
    amxc_var_set_type(&request, AMXC_VAR_ID_HTABLE);
    obj_paths = amxc_var_add_key(amxc_llist_t, &request, "obj_paths", NULL);
    amxc_var_add(cstring_t, obj_paths, "test_root.child.");
    amxc_var_add_key(bool, &request, "first_level_only", true);

    // Create protobuf
    uspl_get_instances_new(usp_tx, &request);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&request);
    uspl_tx_delete(&usp_tx);
}

static void mock_get_instances_resp(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_pbuf = NULL;
    uspl_tx_t* usp_tx = NULL;
    amxc_var_t ret;
    amxc_var_t* curr_insts = NULL;
    amxc_var_t* inst = NULL;
    amxc_var_t* unique_keys = NULL;
    amxc_llist_t resp_list;

    uspl_tx_new(&usp_tx, "usp:/tmp/test-sender", "usp:/tmp/test-receiver");

    amxc_llist_init(&resp_list);

    amxc_var_init(&ret);
    amxc_var_set_type(&ret, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &ret, "requested_path", "test_root.child.");
    amxc_var_add_key(uint32_t, &ret, "err_code", 0);
    curr_insts = amxc_var_add_key(amxc_llist_t, &ret, "curr_insts", NULL);
    inst = amxc_var_add(amxc_htable_t, curr_insts, NULL);
    amxc_var_add_key(cstring_t, inst, "inst_path", "test_root.child.1.");
    unique_keys = amxc_var_add_key(amxc_htable_t, inst, "unique_keys", NULL);
    amxc_var_add_key(cstring_t, unique_keys, "Alias", "One");

    amxc_llist_append(&resp_list, &ret.lit);

    // Create protobuf
    uspl_get_instances_resp_new(usp_tx, &resp_list, last_msg_id);

    // Create tlv for protobuf and append to frame
    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp_tx->pbuf_len, usp_tx->pbuf, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_pbuf);

    amxc_var_clean(&ret);
    uspl_tx_delete(&usp_tx);
}

static void mock_handshake(imtp_frame_t** frame) {
    imtp_tlv_t* tlv_handshake = NULL;
    const char* eid = "proto::from_id";

    imtp_frame_new(frame);
    imtp_tlv_new(&tlv_handshake, imtp_tlv_type_handshake, strlen(eid),
                 (char*) eid, 0, IMTP_TLV_COPY);
    imtp_frame_tlv_add(*frame, tlv_handshake);
}

int __wrap_imtp_connection_connect(imtp_connection_t** icon, char* from_uri, char* to_uri) {
    check_expected_ptr(icon);
    check_expected_ptr(from_uri);
    check_expected_ptr(to_uri);

    assert_non_null(icon);
    assert_non_null(to_uri);
    assert_null(from_uri);

    *icon = calloc(1, sizeof(imtp_connection_t));
    (*icon)->addr_other = calloc(1, sizeof(struct sockaddr_un));
    memcpy((*icon)->addr_other->sun_path, to_uri, strlen(to_uri));
    (*icon)->addr_other->sun_family = AF_UNIX;
    (*icon)->fd = mock_type(int);
    (*icon)->flags = SOCK_STREAM;

    return mock_type(int);
}

int __wrap_imtp_connection_listen(imtp_connection_t** icon, char* uri, imtp_connection_accept_cb_t fn) {
    assert_non_null(icon);
    assert_non_null(uri);
    assert_null(fn);

    *icon = calloc(1, sizeof(imtp_connection_t));
    (*icon)->addr_self = calloc(1, sizeof(struct sockaddr_un));
    memcpy((*icon)->addr_self->sun_path, uri, strlen(uri));
    (*icon)->addr_self->sun_family = AF_UNIX;
    (*icon)->fd = mock_type(int);
    (*icon)->flags = SOCK_STREAM | IMTP_LISTEN;

    return mock_type(int);
}

int __wrap_imtp_connection_accept(imtp_connection_t* icon) {
    assert_non_null(icon);

    con_accepted = calloc(1, sizeof(imtp_connection_t));
    con_accepted->fd = mock_type(int);
    con_accepted->flags = SOCK_STREAM;

    return mock_type(int);
}

imtp_connection_t* __wrap_imtp_connection_get_con(UNUSED int fd) {
    return con_accepted;
}

void __wrap_imtp_connection_delete(imtp_connection_t** icon) {
    check_expected_ptr(icon);

    assert_non_null(icon);

    if(*icon != NULL) {
        if((*icon)->addr_self != NULL) {
            unlink((*icon)->addr_self->sun_path);
        }
        free((*icon)->addr_self);
        free((*icon)->addr_other);
    }
    free(*icon);
    *icon = NULL;
}

int __wrap_imtp_connection_read_frame(imtp_connection_t* icon, imtp_frame_t** frame) {
    check_expected_ptr(icon);
    check_expected_ptr(frame);

    int rv = mock_type(int);
    switch(rv) {
    case usp_type_get:
        mock_get(frame);
        break;
    case usp_type_get_resp:
        mock_get_resp(frame);
        break;
    case usp_type_set:
        mock_set(frame);
        break;
    case usp_type_set_resp:
        mock_set_resp(frame);
        break;
    case usp_type_add:
        mock_add(frame);
        break;
    case usp_type_add_resp:
        mock_add_resp(frame);
        break;
    case usp_type_delete:
        mock_delete(frame);
        break;
    case usp_type_delete_resp:
        mock_delete_resp(frame);
        break;
    case usp_type_get_supported_dm:
        mock_get_supported_dm(frame);
        break;
    case usp_type_get_supported_dm_resp:
        mock_get_supported_dm_resp(frame);
        break;
    case usp_type_operate:
        mock_operate(frame, mock_type(int));
        break;
    case usp_type_operate_resp:
        mock_operate_resp(frame, mock_type(int));
        break;
    case usp_type_get_instances:
        mock_get_instances(frame);
        break;
    case usp_type_get_instances_resp:
        mock_get_instances_resp(frame);
        break;
    case usp_type_handshake:
        mock_handshake(frame);
        break;
    default:
        break;
    }

    return mock_type(int);
}

int __wrap_imtp_connection_write_frame(imtp_connection_t* icon, imtp_frame_t* frame) {
    check_expected_ptr(icon);
    check_expected_ptr(frame);
    const imtp_tlv_t* tlv_pbuf = NULL;

    tlv_pbuf = imtp_frame_get_first_tlv(frame, imtp_tlv_type_protobuf_bytes);
    if(tlv_pbuf != NULL) {
        uspl_rx_t* usp_rx = \
            uspl_msghandler_unpack_protobuf((unsigned char*) tlv_pbuf->value + tlv_pbuf->offset,
                                            tlv_pbuf->length);
        free(last_msg_id);
        last_msg_id = strdup(uspl_msghandler_msg_id(usp_rx));
        uspl_rx_delete(&usp_rx);
    }

    return 0;
}

void imtp_mock_free_msg_id(void) {
    free(last_msg_id);
    last_msg_id = NULL;
}

char* imtp_mock_get_msg_id(void) {
    return last_msg_id;
}
