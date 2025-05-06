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
#include <string.h>
#include <time.h>
#include <poll.h>

#include <usp/uspl.h>

#include "amxb_usp.h"

static void amxb_usp_populate_ret_var(amxc_var_t* ret, amxc_llist_t* resp_list) {
    if(amxc_llist_size(resp_list) == 1) {
        amxc_var_move(ret, amxc_var_from_llist_it(amxc_llist_get_first(resp_list)));
    } else {
        amxc_var_set_type(ret, AMXC_VAR_ID_LIST);
        amxc_llist_for_each(it, resp_list) {
            amxc_var_t* var = amxc_var_from_llist_it(it);
            amxc_var_set_index(ret, -1, var, AMXC_VAR_FLAG_DEFAULT);
        }
    }
}

static void amxb_usp_add_to_list(amxc_var_t* dst, amxc_var_t* src, bool return_as_list) {
    // For add response to search path, the src will already be of type list
    if(return_as_list && (amxc_var_type_of(src) != AMXC_VAR_ID_LIST)) {
        amxc_var_set_type(dst, AMXC_VAR_ID_LIST);
        amxc_var_t* var = amxc_var_add(amxc_htable_t, dst, NULL);
        amxc_var_move(var, src);
    } else {
        amxc_var_move(dst, src);
    }
}

static char* amxb_usp_topic_from_config(void) {
    char* topic = NULL;
    const amxc_var_t* topic_var = NULL;

    topic_var = amxb_usp_get_config_option("topic");
    topic = amxc_var_dyncast(cstring_t, topic_var);

    return topic;
}

static int amxb_usp_recv_resp(amxb_usp_t* amxb_usp,
                              char* expected_msg_id,
                              fn_resp_extract fn,
                              amxc_var_t* ret,
                              bool return_as_list) {
    int retval = -1;
    int msg_type = -1;
    imtp_frame_t* frame = NULL;
    const imtp_tlv_t* tlv_pbuf = NULL;
    uspl_rx_t* usp_rx = NULL;
    amxc_llist_t resp_list;
    amxc_var_t extract_var;
    const char* msg_id_rx = NULL;

    amxc_llist_init(&resp_list);
    amxc_var_init(&extract_var);

    retval = imtp_connection_read_frame(amxb_usp->icon, &frame);
    when_failed(retval, exit);
    retval = -1;

    tlv_pbuf = imtp_frame_get_first_tlv(frame, imtp_tlv_type_protobuf_bytes);
    when_null(tlv_pbuf, exit);

    usp_rx = uspl_msghandler_unpack_protobuf((unsigned char*) tlv_pbuf->value + tlv_pbuf->offset,
                                             tlv_pbuf->length);
    when_null(usp_rx, exit);

    msg_type = uspl_msghandler_msg_type(usp_rx);

    msg_id_rx = uspl_msghandler_msg_id(usp_rx);
    when_str_empty(msg_id_rx, exit);

    if(strcmp(expected_msg_id, msg_id_rx) == 0) {
        if(msg_type == USP__HEADER__MSG_TYPE__ERROR) {
            retval = uspl_error_resp_extract(usp_rx, ret);
            when_failed_status(retval, exit, retval = -1);
        } else {
            retval = fn(usp_rx, &resp_list);
            when_failed_status(retval, exit, retval = -1);
            amxb_usp_populate_ret_var(&extract_var, &resp_list);
            amxb_usp_add_to_list(ret, &extract_var, return_as_list);
            retval = 0;
            goto exit;
        }
    } else {
        amxb_usp_handle_protobuf(amxb_usp, usp_rx);
    }

exit:
    amxc_var_clean(&extract_var);
    uspl_rx_delete(&usp_rx);
    imtp_frame_delete(&frame);
    amxc_llist_clean(&resp_list, variant_list_it_free);
    return retval;
}

static int amxb_usp_wait_for_handshake(amxb_usp_t* amxb_usp_ctx) {
    struct pollfd fds[1];
    int retval = -1;

    fds[0].fd = amxb_usp_get_fd(amxb_usp_ctx);
    fds[0].events = POLLIN;

    retval = poll(fds, 1, 2 * 1000);
    if(retval <= 0) {
        retval = -1;
        goto exit;
    }

    retval = amxb_usp_read(amxb_usp_ctx);

exit:
    return retval;
}

static void deferred_child_list_it_free(amxc_llist_it_t* it) {
    amxb_usp_deferred_child_t* child = amxc_container_of(it, amxb_usp_deferred_child_t, it);
    amxb_usp_deferred_child_delete(&child);
}

int amxb_usp_handle_protobuf(amxb_usp_t* ctx, uspl_rx_t* usp_data) {
    int retval = -1;
    int msg_type = 0;

    when_null(ctx, exit);
    when_null(usp_data, exit);

    msg_type = uspl_msghandler_msg_type(usp_data);
    switch(msg_type) {
    case USP__HEADER__MSG_TYPE__GET:
        retval = amxb_usp_handle_get(ctx, usp_data);
        break;
    case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM:
        retval = amxb_usp_handle_get_supported_dm(ctx, usp_data);
        break;
    case USP__HEADER__MSG_TYPE__SET:
        retval = amxb_usp_handle_set(ctx, usp_data);
        break;
    case USP__HEADER__MSG_TYPE__ADD:
        retval = amxb_usp_handle_add(ctx, usp_data);
        break;
    case USP__HEADER__MSG_TYPE__DELETE:
        retval = amxb_usp_handle_delete(ctx, usp_data);
        break;
    case USP__HEADER__MSG_TYPE__NOTIFY:
        retval = amxb_usp_handle_notify(ctx, usp_data);
        break;
    case USP__HEADER__MSG_TYPE__OPERATE:
        retval = amxb_usp_handle_operate(ctx, usp_data);
        break;
    case USP__HEADER__MSG_TYPE__OPERATE_RESP:
        retval = amxb_usp_handle_operate_resp(ctx, usp_data);
        break;
    case USP__HEADER__MSG_TYPE__GET_INSTANCES:
        retval = amxb_usp_handle_get_instances(ctx, usp_data);
        break;
    case USP__HEADER__MSG_TYPE__REGISTER:
        retval = amxb_usp_handle_register(ctx, usp_data);
        break;
    case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO:
    default:
        break;
    }

exit:
    return retval;
}

int amxb_usp_metadata_extract(uspl_rx_t* usp_data, char** msg_id, char** from_id, char** to_id) {
    int retval = -1;

    *msg_id = uspl_msghandler_msg_id(usp_data);
    when_null(*msg_id, exit);
    *from_id = uspl_msghandler_from_id(usp_data);
    when_null(*from_id, exit);
    *to_id = uspl_msghandler_to_id(usp_data);
    when_null(*to_id, exit);

    retval = 0;
exit:
    return retval;
}

int amxb_usp_build_and_send_tlv(amxb_usp_t* ctx, uspl_tx_t* usp) {
    int retval = -1;
    imtp_frame_t* frame = NULL;
    imtp_tlv_t* tlv_pbuf = NULL;
    char* topic = NULL;

    when_null(usp, exit);

    retval = imtp_frame_new(&frame);
    when_failed(retval, exit);

    retval = imtp_tlv_new(&tlv_pbuf, imtp_tlv_type_protobuf_bytes, usp->pbuf_len,
                          usp->pbuf, 0, IMTP_TLV_COPY);
    when_failed(retval, exit);

    retval = imtp_frame_tlv_add(frame, tlv_pbuf);
    when_failed(retval, exit);

    // Add a topic TLV if one can be found in the ODL config section
    topic = amxb_usp_topic_from_config();
    if(topic != NULL) {
        imtp_tlv_t* tlv_topic = NULL;
        retval = imtp_tlv_new(&tlv_topic, imtp_tlv_type_topic, strlen(topic),
                              topic, 0, IMTP_TLV_COPY);
        when_failed(retval, exit);

        retval = imtp_frame_tlv_add(frame, tlv_topic);
        when_failed(retval, exit);
    }

    retval = imtp_connection_write_frame(ctx->icon, frame);
    when_failed(retval, exit);

exit:
    free(topic);
    imtp_frame_delete(&frame);
    return retval;
}

int amxb_usp_reply(amxb_usp_t* ctx,
                   uspl_rx_t* usp_data,
                   amxc_llist_t* resp_list,
                   fn_resp_new fn) {
    int retval = -1;
    char* msg_id = NULL;
    char* from_id = NULL;
    char* to_id = NULL;
    uspl_tx_t* usp_tx = NULL;

    when_null(ctx, exit);
    when_null(usp_data, exit);
    when_null(resp_list, exit);

    retval = amxb_usp_metadata_extract(usp_data, &msg_id, &from_id, &to_id);
    when_failed(retval, exit);

    retval = uspl_tx_new(&usp_tx, to_id, from_id);
    when_failed(retval, exit);

    retval = fn(usp_tx, resp_list, msg_id);
    when_failed(retval, exit);

    retval = amxb_usp_build_and_send_tlv(ctx, usp_tx);
    when_failed(retval, exit);

    retval = 0;
exit:
    uspl_tx_delete(&usp_tx);
    return retval;
}

int amxb_usp_reply_deferred(amxb_usp_deferred_t* fcall,
                            fn_resp_new fn) {
    int retval = -1;
    const char* msg_id = GET_CHAR(&fcall->data, "msg_id");
    const char* from_id = GET_CHAR(&fcall->data, "from_id");
    const char* to_id = GET_CHAR(&fcall->data, "to_id");
    uspl_tx_t* usp_tx = NULL;

    when_null(fcall, exit);

    retval = uspl_tx_new(&usp_tx, to_id, from_id);
    when_failed(retval, exit);

    retval = fn(usp_tx, &fcall->resp_list, msg_id);
    when_failed(retval, exit);

    retval = amxb_usp_build_and_send_tlv(fcall->ctx, usp_tx);
    when_failed(retval, exit);

    retval = 0;
exit:
    uspl_tx_delete(&usp_tx);
    return retval;
}

int amxb_usp_reply_error(amxb_usp_t* ctx,
                         uspl_rx_t* usp_data,
                         int err_code) {
    int retval = -1;
    char* msg_id = NULL;
    char* from_id = NULL;
    char* to_id = NULL;
    uspl_tx_t* record = NULL;
    amxc_var_t error;

    amxc_var_init(&error);

    when_null(ctx, exit);
    when_null(usp_data, exit);

    amxc_var_set_type(&error, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &error, "err_code", err_code);
    amxc_var_add_key(cstring_t, &error, "err_msg", uspl_error_code_to_str(err_code));

    retval = amxb_usp_metadata_extract(usp_data, &msg_id, &from_id, &to_id);
    when_failed(retval, exit);

    retval = uspl_tx_new(&record, to_id, from_id);
    when_failed(retval, exit);

    retval = uspl_error_resp_new(record, &error, msg_id);
    when_failed(retval, exit);

    retval = amxb_usp_build_and_send_tlv(ctx, record);
    when_failed(retval, exit);

exit:
    amxc_var_clean(&error);
    uspl_tx_delete(&record);
    return retval;
}

int amxb_usp_tx_new(amxb_usp_t* amxb_usp, uspl_tx_t** usp_tx) {
    int retval = -1;
    char* from_id = amxb_usp_get_from_id();
    char* to_id = amxb_usp_get_to_id(amxb_usp);

    when_str_empty(to_id, exit);
    retval = uspl_tx_new(usp_tx, from_id, to_id);

exit:
    free(to_id);
    free(from_id);
    return retval;
}

int amxb_usp_send_req(amxb_usp_t* amxb_usp,
                      amxc_var_t* request,
                      fn_request_new fn,
                      char** msg_id_tx) {
    int retval = -1;
    uspl_tx_t* usp_tx = NULL;

    retval = amxb_usp_tx_new(amxb_usp, &usp_tx);
    when_failed(retval, exit);

    retval = fn(usp_tx, request);
    when_failed(retval, exit);

    retval = amxb_usp_build_and_send_tlv(amxb_usp, usp_tx);
    when_failed(retval, exit);

    when_str_empty_status(usp_tx->msg_id, exit, retval = -1);

    when_null(msg_id_tx, exit);
    *msg_id_tx = strdup(usp_tx->msg_id);

exit:
    uspl_tx_delete(&usp_tx);
    return retval;
}

int amxb_usp_poll_response(amxb_usp_t* amxb_usp,
                           char* expected_msg_id,
                           fn_resp_extract fn,
                           amxc_var_t* ret,
                           bool return_as_list,
                           int timeout) {
    int retval = -1;
    time_t now;
    time_t break_time;
    double diff = timeout;
    struct pollfd fds;
    sigset_t origmask;
    sigset_t sigmask;

    when_str_empty(expected_msg_id, exit);

    fds.fd = amxb_usp_get_fd(amxb_usp);
    fds.events = POLLIN;

    time(&now);
    break_time = now + timeout;

    /* Block SIGALARM to avoid interrupting poll */
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigmask, &origmask);

    while(1) {
        retval = poll(&fds, 1, (int) (diff * 1000));
        if((retval == -1) || (retval == 0)) {
            retval = -1;
            goto exit;
        } else {
            retval = amxb_usp_recv_resp(amxb_usp, expected_msg_id, fn, ret, return_as_list);
            if(retval == 0) {
                break;
            }
        }
        time(&now);
        diff = difftime(break_time, now);
        if(diff <= 0) {
            break;
        }
    }

exit:
    sigprocmask(SIG_SETMASK, &origmask, NULL);
    return retval;
}

char* amxb_usp_get_from_id(void) {
    const amxc_var_t* eid = amxb_usp_get_config_option("EndpointID");
    char* from_id = NULL;

    if(eid != NULL) {
        from_id = amxc_var_dyncast(cstring_t, eid);
    } else {
        from_id = strdup("amxb-usp-sender");
    }

    return from_id;
}

char* amxb_usp_get_to_id(amxb_usp_t* ctx) {
    char* to_id = NULL;

    when_null(ctx, exit);
    if((ctx->eid != NULL) && (*(ctx->eid) != 0)) {
        to_id = strdup(ctx->eid);
    } else {
        if((amxb_usp_wait_for_handshake(ctx) == 0) && (ctx->eid != NULL)) {
            to_id = strdup(ctx->eid);
        }
    }

exit:
    return to_id;
}

int amxb_usp_is_dot(int c) {
    return (c == '.') ? 1 : 0;
}

amxd_status_t amxb_usp_filter_ret(amxc_var_t* ret) {
    amxd_status_t status = amxd_status_ok;
    amxc_var_for_each(entry, ret) {
        amxc_var_t* result = GET_ARG(entry, "result");
        status = uspl_usp_error_to_amxd_status(GET_UINT32(result, "err_code"));
        amxc_var_delete(&result);
    }
    return status;
}

void amxb_usp_request_free(UNUSED const char* key, amxc_htable_it_t* it) {
    usp_request_t* usp_request = amxc_container_of(it, usp_request_t, hit);
    if(usp_request->request != NULL) {
        free(usp_request->request->bus_data);
        usp_request->request->bus_data = NULL;
        usp_request->request = NULL;
    }
    free(usp_request);
}

amxc_string_t* amxb_usp_add_dot(const char* str_in) {
    size_t len = 0;
    amxc_string_t* dotted_path = NULL;

    when_str_empty(str_in, exit);

    amxc_string_new(&dotted_path, 0);
    amxc_string_setf(dotted_path, "%s", str_in);
    len = amxc_string_text_length(dotted_path);

    if(str_in[len - 1] != '.') {
        amxc_string_append(dotted_path, ".", 1);
    }

exit:
    return dotted_path;
}

int amxb_usp_deferred_child_new(amxb_usp_deferred_child_t** child,
                                amxb_usp_deferred_t* parent,
                                uint64_t call_id,
                                uint32_t request_index,
                                const char* requested_path,
                                amxc_var_t* args) {
    int retval = -1;

    when_null(child, exit);
    when_null(parent, exit);
    when_str_empty(requested_path, exit);

    *child = (amxb_usp_deferred_child_t*) calloc(1, sizeof(amxb_usp_deferred_child_t));
    when_null(*child, exit);

    (*child)->parent = parent;
    (*child)->call_id = call_id;
    (*child)->request_index = request_index;
    (*child)->requested_path = strdup(requested_path);

    amxc_var_new(&(*child)->args);
    amxc_var_copy((*child)->args, args);

    amxc_llist_append(&parent->child_list, &(*child)->it);

    retval = 0;
exit:
    return retval;
}

void amxb_usp_deferred_child_delete(amxb_usp_deferred_child_t** child) {
    when_null(child, exit);
    when_null(*child, exit);

    amxc_var_delete(&(*child)->args);
    free((*child)->requested_path);
    free((*child)->cmd_key);
    (*child)->requested_path = NULL;
    (*child)->parent = NULL;
    (*child)->call_id = 0;
    free(*child);
    *child = NULL;

exit:
    return;
}

int amxb_usp_deferred_child_set_cmd_key(amxb_usp_deferred_child_t* child, const char* cmd_key) {
    int retval = -1;

    when_null(child, exit);
    when_str_empty(cmd_key, exit);

    child->cmd_key = strdup(cmd_key);
    when_null(child->cmd_key, exit);

    retval = 0;
exit:
    return retval;
}

int amxb_usp_deferred_new(amxb_usp_deferred_t** fcall, amxb_usp_t* ctx, uspl_rx_t* usp_rx, int num_calls) {
    int retval = -1;

    when_null(fcall, exit);
    when_null(ctx, exit);
    when_null(usp_rx, exit);

    *fcall = (amxb_usp_deferred_t*) calloc(1, sizeof(amxb_usp_deferred_t));
    when_null(*fcall, exit);

    amxc_var_init(&(*fcall)->data);
    amxc_var_set_type(&(*fcall)->data, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &(*fcall)->data, "from_id", uspl_msghandler_from_id(usp_rx));
    amxc_var_add_key(cstring_t, &(*fcall)->data, "to_id", uspl_msghandler_to_id(usp_rx));
    amxc_var_add_key(cstring_t, &(*fcall)->data, "msg_id", uspl_msghandler_msg_id(usp_rx));
    amxc_var_add_key(uint32_t, &(*fcall)->data, "msg_type", uspl_msghandler_msg_type(usp_rx));
    amxc_llist_init(&(*fcall)->resp_list);
    amxc_llist_init(&(*fcall)->child_list);
    (*fcall)->ctx = ctx;
    (*fcall)->num_calls = num_calls;

    retval = 0;
exit:
    return retval;
}

void amxb_usp_deferred_delete(amxb_usp_deferred_t** fcall) {
    when_null(fcall, exit);
    when_null(*fcall, exit);

    amxc_llist_clean(&(*fcall)->child_list, deferred_child_list_it_free);
    amxc_llist_clean(&(*fcall)->resp_list, variant_list_it_free);
    amxc_var_clean(&(*fcall)->data);
    free(*fcall);
    *fcall = NULL;

exit:
    return;
}

// The rel_path is built based on the requested_path that is provided. When we are handling an
// operate message, the requeted_path will include the function name e.g. "Device.IP._describe()".
// In this case, we don't want to add the last part (the function) to the rel_path. When we are
// handling a get message, we will call _get and the requested_path can be a parameter path. In
// this case we do want to include the parameter in the rel_path, therefore this function takes
// a set_param argument.
void amxb_usp_rel_path_set(amxc_var_t* args, amxd_path_t* path, bool set_param) {
    amxc_string_t full_path;
    amxc_var_t* rel_path = NULL;
    const char* param = NULL;

    amxc_string_init(&full_path, 0);

    rel_path = GET_ARG(args, "rel_path");
    if(rel_path == NULL) {
        rel_path = amxc_var_add_new_key(args, "rel_path");
    }

    param = amxd_path_get_param(path);
    if(set_param) {
        amxc_string_setf(&full_path, "%s%s", amxd_path_get(path, AMXD_OBJECT_TERMINATE),
                         param != NULL ? param : "");
    } else {
        amxc_string_setf(&full_path, "%s", amxd_path_get(path, AMXD_OBJECT_TERMINATE));
    }
    amxc_var_push(cstring_t, rel_path, amxc_string_take_buffer(&full_path));

    amxc_string_clean(&full_path);
}

void amxb_usp_amx_call_done(const amxc_var_t* const data,
                            void* const priv) {
    amxb_usp_deferred_child_t* child = (amxb_usp_deferred_child_t*) priv;
    amxb_usp_deferred_t* fcall = child->parent;
    uint32_t msg_type = GET_UINT32(&fcall->data, "msg_type");

    fcall->num_calls--;
    switch(msg_type) {
    case USP__HEADER__MSG_TYPE__GET:
        amxb_usp_get_deferred_resp(data, fcall, child);
        break;
    case USP__HEADER__MSG_TYPE__SET:
        amxb_usp_set_deferred_resp(data, fcall, child);
        break;
    case USP__HEADER__MSG_TYPE__ADD:
        amxb_usp_add_deferred_resp(data, fcall, child);
        break;
    case USP__HEADER__MSG_TYPE__DELETE:
        amxb_usp_delete_deferred_resp(data, fcall, child);
        break;
    case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM:
        amxb_usp_get_supported_dm_deferred_resp(data, fcall, child);
        break;
    case USP__HEADER__MSG_TYPE__GET_INSTANCES:
        amxb_usp_get_instances_deferred_resp(data, fcall, child);
        break;
    case USP__HEADER__MSG_TYPE__OPERATE:
        amxb_usp_operate_deferred_resp(data, fcall, child);
        break;
    default:
        break;
    }
}

static amxd_object_t* amxb_usp_fetch_object(amxb_usp_deferred_t* fcall,
                                            const char* first_part,
                                            const char* requested_path) {
    amxd_object_t* object = NULL;

    // The Device.LocalAgent. data model has its own amxd_dm_t struct to keep a
    // clear separation between the real DM and the USP specific DM
    // Requests for Device.LocalAgent. need to be called on a different object
    if(strncmp(requested_path, "Device.LocalAgent.", 18) == 0) {
        object = amxd_dm_findf(fcall->ctx->la_dm, "%s", first_part);
    } else {
        object = amxd_dm_findf(fcall->ctx->dm, "%s", first_part);
    }

    return object;
}

amxd_status_t amxb_usp_amx_call(amxb_usp_deferred_t* fcall,
                                const char* translated_path,
                                const char* requested_path,
                                bool set_param,
                                const char* func,
                                amxc_var_t* args,
                                amxc_var_t* ret,
                                amxp_deferred_fn_t cb) {
    amxd_status_t retval = amxd_status_object_not_found;
    amxd_object_t* object = NULL;
    amxd_object_t* la_root = amxd_dm_get_root(fcall->ctx->la_dm);
    char* first_part = NULL;
    amxd_path_t path;

    amxd_path_init(&path, translated_path);
    la_root->priv = fcall->ctx;

    // Take first object from requested path and put everything else in rel_path
    // Don't invoke function on data model root, because then the forwarding from
    // dm_proxy will not be handled
    first_part = amxd_path_get_first(&path, true);
    amxb_usp_rel_path_set(args, &path, set_param);

    object = amxb_usp_fetch_object(fcall, first_part, requested_path);
    if(object == NULL) {
        retval = amxd_status_object_not_found;
        fcall->num_calls--;
        goto exit;
    }

    retval = amxd_object_invoke_function(object, func, args, ret);
    if(retval == amxd_status_deferred) {
        amxb_usp_deferred_child_t* child = NULL;
        uint64_t call_id = amxc_var_constcast(uint64_t, ret);

        amxb_usp_deferred_child_new(&child, fcall, call_id, 0, requested_path, args);
        amxd_function_set_deferred_cb(call_id, cb, child);
    } else {
        fcall->num_calls--;
    }

exit:
    free(first_part);
    amxd_path_clean(&path);
    la_root->priv = NULL;

    return retval;
}

bool amxb_usp_path_starts_with_device(const char* path) {
    bool has_prefix = false;

    when_str_empty(path, exit);

    when_false(strncmp(path, "Device.", 7) == 0, exit);
    has_prefix = true;

exit:
    return has_prefix;
}

amxd_status_t amxb_usp_convert_error(amxc_var_t* ret, amxc_var_t* src) {
    amxd_status_t status = amxd_status_ok;
    amxc_var_t* err_code = GET_ARG(src, "err_code");

    if(err_code != NULL) {
        status = uspl_usp_error_to_amxd_status(amxc_var_dyncast(uint32_t, err_code));
        amxc_var_set(uint32_t, err_code, status);
        amxc_var_move(ret, src);
    }

    return status;
}
