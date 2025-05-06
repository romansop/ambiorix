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
#include <poll.h>

#include <yajl/yajl_gen.h>
#include <amxc/amxc_variant_type.h>
#include <amxj/amxj_variant.h>

#include "amxb_usp.h"
#include "amxb_usp_la.h"

static int cmd_key_counter = 0;

// Reuse command_path to generate a command_key if none is provided
static const char* generate_cmd_key(amxc_string_t* command_path) {
    amxc_string_appendf(command_path, "-%d", cmd_key_counter);
    cmd_key_counter++;

    return amxc_string_get(command_path, 0);
}

static void amxb_usp_convert_out_args(amxc_var_t* dst, amxc_var_t* output_args) {
    amxc_var_t* return_variant = GET_ARG(output_args, "_retval");
    amxc_var_t* dst_element = NULL;

    dst_element = amxc_var_add(amxc_htable_t, dst, NULL);
    amxc_var_move(dst_element, return_variant);
    amxc_var_delete(&return_variant);

    // Add output args if there are any
    if(GETI_ARG(output_args, 0) != NULL) {
        dst_element = amxc_var_add(amxc_htable_t, dst, NULL);
        amxc_var_move(dst_element, output_args);
    }
}

static amxd_status_t amxb_usp_convert_cmd_failure(amxc_var_t* dst, amxc_var_t* cmd_failure) {
    amxc_var_t* dst_element = NULL;
    amxd_status_t err_code = amxd_status_unknown_error;

    amxc_var_set_type(dst, AMXC_VAR_ID_LIST);
    dst_element = amxc_var_add(cstring_t, dst, "");
    amxc_var_set_type(dst_element, AMXC_VAR_ID_NULL);
    dst_element = amxc_var_add(amxc_htable_t, dst, NULL);
    amxc_var_move(dst_element, cmd_failure);

    err_code = GET_UINT32(dst_element, "err_code");
    return err_code == 0 ? amxd_status_unknown_error : err_code;
}

// Input arguments are all received as string values. They don't get through ambiorix type checking.
static void amxb_usp_input_cast(amxc_var_t* src) {
    amxc_var_for_each(var, src) {
        amxc_var_cast(var, AMXC_VAR_ID_ANY);
    }
}

static void amxb_usp_operate_add_braces(amxc_string_t* command_path) {
    const char* last = amxc_string_get(command_path, amxc_string_text_length(command_path) - 1);
    when_null(last, exit);

    if(*last != ')') {
        amxc_string_append(command_path, "()", 2);
    }
exit:
    return;
}

static void amxb_usp_operate_populate_request(amxc_var_t* request,
                                              const char* object,
                                              const char* method,
                                              amxc_var_t* args) {
    amxc_string_t command_path;
    const char* command_key = NULL;
    const amxc_htable_t* input_args = NULL;

    amxc_string_init(&command_path, 0);
    amxc_string_setf(&command_path, "%s.%s", object, method);
    amxb_usp_operate_add_braces(&command_path);

    amxc_var_set_type(request, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, request, "command", amxc_string_get(&command_path, 0));

    command_key = GET_CHAR(args, "command_key");
    if(command_key == NULL) {
        command_key = generate_cmd_key(&command_path);
        amxc_var_add_key(cstring_t, request, "command_key", command_key);
    } else {
        amxc_var_add_key(cstring_t, request, "command_key", command_key);
        amxc_var_t* cmd_key = amxc_var_take_key(args, "command_key");
        amxc_var_delete(&cmd_key);
    }

    // Always send response in backend
    amxc_var_add_key(bool, request, "send_resp", true);
    input_args = amxc_var_constcast(amxc_htable_t, args);
    amxc_var_add_key(amxc_htable_t, request, "input_args", input_args);

    amxc_string_clean(&command_path);
}

static void amxb_usp_operate_req_output_args(amxc_var_t* response, amxc_var_t* ret, amxc_var_t* args) {
    amxc_var_t* output_args = NULL;
    amxc_var_t* cmd_key = GET_ARG(args, "command_key");
    amxc_var_t* args_table = GET_ARG(args, "args");

    amxc_var_delete(&cmd_key);

    amxc_var_add_key(uint32_t, response, "operation_resp_case", USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OUTPUT_ARGS);
    output_args = amxc_var_add_key(amxc_htable_t, response, "output_args", NULL);

    if(args_table != NULL) {
        amxc_var_move(output_args, args_table);
    }

    if(ret->type_id != AMXC_VAR_ID_NULL) {
        amxc_var_t* return_variant = amxc_var_add_key(amxc_htable_t, output_args, "_retval", NULL);
        amxc_var_move(return_variant, ret);
    }
}

static void amxb_usp_operate_req_obj_path(amxc_var_t* response, uint32_t request_index) {
    amxc_var_t* req_obj_path = NULL;
    amxc_string_t request_instance;

    amxc_string_init(&request_instance, 0);
    amxc_string_setf(&request_instance, "Device.LocalAgent.Request.%d.", request_index);

    amxc_var_add_key(uint32_t, response, "operation_resp_case", USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OBJ_PATH);
    req_obj_path = amxc_var_add_new_key(response, "req_obj_path");
    amxc_var_push(cstring_t, req_obj_path, amxc_string_take_buffer(&request_instance));

    amxc_string_clean(&request_instance);
}

static void amxb_usp_operate_failure(amxc_var_t* response, amxc_var_t* args, amxd_status_t status) {
    amxc_var_t* cmd_failure = NULL;
    uint32_t err_code = GET_UINT32(args, "err_code");
    const char* err_msg = GET_CHAR(args, "err_msg");

    amxc_var_add_key(uint32_t, response, "operation_resp_case", USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_CMD_FAILURE);
    cmd_failure = amxc_var_add_key(amxc_htable_t, response, "cmd_failure", NULL);

    if(err_code == 0) {
        err_code = uspl_amxd_status_to_usp_error(status);
    }
    amxc_var_add_key(uint32_t, cmd_failure, "err_code", err_code);

    if((err_msg != NULL) && (*err_msg != 0)) {
        amxc_var_add_key(cstring_t, cmd_failure, "err_msg", err_msg);
    } else {
        amxc_var_add_key(cstring_t, cmd_failure, "err_msg", uspl_error_code_to_str(err_code));
    }
}

/**
   Convert the return variant from an invoke to a variant that can be passed as
   one of the inputs to uspl_operate_resp_new.

   If retval != 0, check for err_code and err_msg in output args. If they are
   missing, add generic error code instead
 */
static void amxb_usp_invoke_result_to_operate_resp(amxc_llist_t* resp_list,
                                                   amxc_var_t* ret,
                                                   amxc_var_t* args,
                                                   amxd_status_t status,
                                                   const char* path,
                                                   const char* func,
                                                   bool translate,
                                                   uint32_t request_index) {
    amxc_string_t executed_cmd;
    amxc_var_t* response = NULL;
    amxd_path_t transl_path;

    amxd_path_init(&transl_path, path);
    amxc_var_new(&response);
    amxc_var_set_type(response, AMXC_VAR_ID_HTABLE);

    if(translate) {
        const char* requested = NULL;
        const char* translated = NULL;
        amxb_usp_translate_path(&transl_path, &requested, &translated);
    }

    amxc_string_init(&executed_cmd, 0);
    amxc_string_setf(&executed_cmd, "%s%s", amxd_path_get(&transl_path, AMXD_OBJECT_TERMINATE), func);
    amxc_var_add_key(cstring_t, response, "executed_command", amxc_string_get(&executed_cmd, 0));

    if(status == 0) {
        amxb_usp_operate_req_output_args(response, ret, args);
    } else if(request_index > 0) {
        amxb_usp_operate_req_obj_path(response, request_index);
    } else {
        amxb_usp_operate_failure(response, args, status);
    }

    amxd_path_clean(&transl_path);
    amxc_string_clean(&executed_cmd);
    amxc_llist_append(resp_list, &response->lit);
}

static amxd_object_t* amxb_usp_find_object(amxb_usp_deferred_t* fcall,
                                           uint32_t* request_index,
                                           amxd_path_t* path,
                                           const char* func,
                                           amxc_var_t* args,
                                           const char* cmd_key) {
    amxd_object_t* object = NULL;
    char* first = NULL;

    // two options available
    object = amxd_dm_findf(fcall->ctx->dm, "%s", amxd_path_get(path, AMXD_OBJECT_TERMINATE));
    if(object != NULL) {
        // 1. the object can be found, call the method directly
        amxd_function_t* func_def = NULL;
        const char* requested = NULL;
        const char* translated = NULL;

        if(strcmp(func, "_exec") == 0) {
            func = GET_CHAR(args, "method");
        }

        amxb_usp_translate_path(path, &requested, &translated);
        func_def = amxd_object_get_function(object, func);
        if(amxd_function_is_attr_set(func_def, amxd_fattr_async)) {
            const char* originator = GET_CHAR(&fcall->data, "from_id");
            *request_index = amxb_usp_la_add_request(originator, amxd_path_get(path, AMXD_OBJECT_TERMINATE), func, cmd_key);
        }
    } else {
        // 2. the object can not be found, take the first part, maybe it can be forwarded
        // Take first object from requested path and put everything else in rel_path
        // Don't invoke function on data model root, because then the forwarding from
        // dm_proxy will not be handled
        first = amxd_path_get_first(path, true);
        amxb_usp_rel_path_set(args, path, false);
        object = amxd_dm_findf(fcall->ctx->dm, "%s", first);
    }

    free(first);

    return object;
}

static amxd_status_t amxb_usp_amx_call_resolved(amxb_usp_deferred_t* fcall,
                                                const char* requested_path,
                                                const char* resolved_path,
                                                const char* func,
                                                amxc_var_t* args,
                                                amxc_var_t* ret,
                                                amxp_deferred_fn_t cb,
                                                const char* cmd_key,
                                                uint32_t* request_index) {
    amxd_status_t retval = amxd_status_function_not_found;
    amxd_object_t* object = NULL;
    amxd_path_t path;
    amxc_var_t event;

    amxc_var_init(&event);
    amxd_path_init(&path, resolved_path);
    object = amxb_usp_find_object(fcall, request_index, &path, func, args, cmd_key);

    amxb_usp_la_set_request_status(*request_index, "Active");
    retval = amxd_object_invoke_function(object, func, args, ret);
    if(retval == amxd_status_deferred) {
        amxb_usp_deferred_child_t* child = NULL;
        uint64_t call_id = amxc_var_constcast(uint64_t, ret);

        amxb_usp_deferred_child_new(&child, fcall, call_id, *request_index, requested_path, args);
        amxd_function_set_deferred_cb(call_id, cb, child);
    } else {
        // In case of async method, update Request Status and emit event
        if(*request_index > 0) {
            amxb_usp_la_set_request_status(*request_index, retval == amxd_status_ok ? "Success" : "Error");
            amxb_usp_la_emit_complete(object, *request_index, ret, args, retval);
        }
        fcall->num_calls--;
    }

    amxc_var_clean(&event);
    amxd_path_clean(&path);
    return retval;
}

static amxd_status_t amxb_usp_invoke_on_resolved_paths(amxb_usp_deferred_t* fcall,
                                                       amxc_var_t* get_resp,
                                                       amxd_status_t status,
                                                       const char* requested_path,
                                                       amxc_var_t* args,
                                                       const char* cmd_key) {
    amxd_status_t retval = amxd_status_ok;
    const char* func = NULL;
    amxd_path_t full_path;

    amxd_path_init(&full_path, requested_path);
    func = amxd_path_get_param(&full_path);

    if(status != amxd_status_ok) {
        // Build error response
    }

    fcall->num_calls = amxc_htable_size(amxc_var_constcast(amxc_htable_t, get_resp));
    amxc_var_for_each(entry, get_resp) {
        amxc_var_t ret;
        amxc_var_t args_copy;
        const char* resolved_path = amxc_var_key(entry);
        uint32_t request_index = 0;

        amxc_var_init(&ret);
        amxc_var_init(&args_copy);

        // Function call will update args variant, so we need a copy of the argument to call
        // the function for multiple resolved paths
        amxc_var_copy(&args_copy, args);

        retval = amxb_usp_amx_call_resolved(fcall, requested_path, resolved_path, "_exec", &args_copy, &ret, amxb_usp_amx_call_done, cmd_key, &request_index);

        // Build response now if function is not deferred OR if it is an asynchronous function
        if((status != amxd_status_deferred) || (request_index > 0)) {
            amxb_usp_invoke_result_to_operate_resp(&fcall->resp_list, &ret, &args_copy, status, resolved_path, func, true, request_index);
        }
        amxc_var_clean(&args_copy);
        amxc_var_clean(&ret);
    }

    amxd_path_clean(&full_path);
    return retval;
}

static void amxb_usp_resolve_done(const amxc_var_t* const data,
                                  void* const priv) {
    amxb_usp_deferred_child_t* child = (amxb_usp_deferred_child_t*) priv;
    amxb_usp_deferred_t* fcall = child->parent;
    amxb_usp_invoke_on_resolved_paths(fcall, GET_ARG(data, "retval"), GET_UINT32(data, "status"), child->requested_path, child->args, child->cmd_key);
}

static amxd_status_t amxb_usp_resolve_search_path(amxb_usp_deferred_t* fcall,
                                                  amxd_path_t* cmd_path,
                                                  const char* requested_path,
                                                  amxc_var_t* args,
                                                  const char* cmd_key) {
    amxd_status_t retval = amxd_status_function_not_found;
    amxd_object_t* object = NULL;
    char* first = NULL;
    amxc_var_t get_args;
    amxc_var_t ret;

    amxc_var_init(&get_args);
    amxc_var_init(&ret);

    // Take first object from requested path and put everything else in rel_path
    // Don't invoke function on data model root, because then the forwarding from
    // dm_proxy will not be handled
    first = amxd_path_get_first(cmd_path, true);

    amxc_var_set_type(&get_args, AMXC_VAR_ID_HTABLE);
    amxb_usp_rel_path_set(&get_args, cmd_path, false);
    amxc_var_add_key(uint32_t, &get_args, "depth", 0);

    object = amxd_dm_findf(fcall->ctx->dm, "%s", first);
    when_null(object, exit);

    retval = amxd_object_invoke_function(object, "_get", &get_args, &ret);
    if(retval == amxd_status_deferred) {
        amxb_usp_deferred_child_t* child = NULL;
        uint64_t call_id = amxc_var_constcast(uint64_t, &ret);

        amxb_usp_deferred_child_new(&child, fcall, call_id, 0, requested_path, args);
        amxb_usp_deferred_child_set_cmd_key(child, cmd_key);
        amxd_function_set_deferred_cb(call_id, amxb_usp_resolve_done, child);
    } else {
        retval = amxb_usp_invoke_on_resolved_paths(fcall, &ret, retval, requested_path, args, cmd_key);
    }

exit:
    amxc_var_clean(&ret);
    amxc_var_clean(&get_args);
    free(first);
    return retval;
}

static int is_brace(int c) {
    return ((c == '(') || (c == ')')) ? 1 : 0;
}

static void amxb_usp_invoke_add_stripped_method(amxc_var_t* args, const char* func) {
    amxc_string_t stripped_func;
    amxc_var_t* method = NULL;

    amxc_string_init(&stripped_func, 0);
    amxc_string_setf(&stripped_func, "%s", func);
    amxc_string_trimr(&stripped_func, is_brace);
    method = amxc_var_add_new_key(args, "method");
    amxc_var_push(cstring_t, method, amxc_string_take_buffer(&stripped_func));

    amxc_string_clean(&stripped_func);
}

static amxd_status_t amxb_usp_invoke_functions(amxb_usp_deferred_t* fcall,
                                               amxc_var_t* operate) {
    amxd_status_t status = amxd_status_unknown_error;
    amxd_path_t cmd_path;
    amxc_var_t ret;
    const char* requested = NULL;
    const char* translated = NULL;
    const char* func = NULL;
    const char* command = GET_CHAR(operate, "command");
    const char* cmd_key = GET_CHAR(operate, "command_key");
    amxc_var_t* in_args = NULL;
    amxc_var_t args;

    amxc_var_init(&ret);
    amxc_var_init(&args);
    amxd_path_init(&cmd_path, command);
    when_false(amxd_path_is_valid(&cmd_path), exit);

    // check if translation is need
    // if translation configuration is available, this will translate
    // the requested path into the internal path
    amxb_usp_translate_path(&cmd_path, &requested, &translated);

    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    in_args = amxc_var_add_key(amxc_htable_t, &args, "args", NULL);
    if(GET_ARG(operate, "input_args") != NULL) {
        amxc_var_move(in_args, GET_ARG(operate, "input_args"));
    }

    amxb_usp_input_cast(in_args);

    func = amxd_path_get_param(&cmd_path);
    amxb_usp_invoke_add_stripped_method(&args, func);

    if(!amxd_path_is_search_path(&cmd_path)) {
        const char* path = amxd_path_get(&cmd_path, AMXD_OBJECT_TERMINATE);
        uint32_t request_index = 0;
        fcall->num_calls++;
        status = amxb_usp_amx_call_resolved(fcall, command, path, "_exec", &args, &ret, amxb_usp_amx_call_done, cmd_key, &request_index);

        // Build response now if function is not deferred OR if it is an asynchronous function
        if((status != amxd_status_deferred) || (request_index > 0)) {
            amxb_usp_invoke_result_to_operate_resp(&fcall->resp_list, &ret, &args, status, path, func, true, request_index);
        }
    } else {
        status = amxb_usp_resolve_search_path(fcall, &cmd_path, command, &args, cmd_key);
    }

exit:
    amxc_var_clean(&args);
    amxc_var_clean(&ret);
    amxd_path_clean(&cmd_path);
    return status;
}

static void amxb_usp_convert_req_obj_path(amxc_var_t* dst, amxc_var_t* src) {
    amxc_var_t* oper_resp_case = GET_ARG(src, "operation_resp_case");
    amxc_var_delete(&oper_resp_case);
    amxc_var_move(dst, src);
}

static int amxb_usp_operate_resp_to_ambiorix_variant_single(amxc_var_t* dst, amxc_var_t* src) {
    int retval = -1;
    uint32_t resp_case = GET_UINT32(src, "operation_resp_case");
    amxc_var_t* output_args = NULL;
    amxc_var_t* cmd_failure = NULL;

    switch(resp_case) {
    case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OBJ_PATH:
        amxb_usp_convert_req_obj_path(dst, src);
        retval = 0;
        break;
    case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OUTPUT_ARGS:
        output_args = GETP_ARG(src, "output_args");
        amxb_usp_convert_out_args(dst, output_args);
        retval = 0;
        break;
    case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_CMD_FAILURE:
        cmd_failure = GET_ARG(src, "cmd_failure");
        retval = amxb_usp_convert_cmd_failure(dst, cmd_failure);
        break;
    default:
        // If none of the above, it could be an error message, so we can try to forward some information
        // from there to the caller
        retval = amxb_usp_convert_error(dst, src);
        break;
    }

    return retval;
}

static int amxb_usp_operate_resp_to_ambiorix_variant(amxc_var_t* dst, amxc_var_t* src) {
    int retval = 0;

    // If src contains more than 1 element, return list of lists where the inner list
    // contains the return variant at index 0 and the output args at index 1
    // This will only be the case for invokes on search paths that target multiple instances
    amxc_var_set_type(dst, AMXC_VAR_ID_LIST);
    if(amxc_var_type_of(src) == AMXC_VAR_ID_LIST) {
        amxc_var_for_each(result, src) {
            amxc_var_t* dst_element = amxc_var_add(amxc_llist_t, dst, NULL);
            retval |= amxb_usp_operate_resp_to_ambiorix_variant_single(dst_element, result);
        }
    } else {
        retval = amxb_usp_operate_resp_to_ambiorix_variant_single(dst, src);
    }

    return retval;
}

static void amxb_usp_request_done(UNUSED amxb_usp_t* usp_ctx,
                                  usp_request_t* usp_request,
                                  amxc_llist_t* resp_list) {
    amxb_request_t* request = usp_request->request;
    const amxb_bus_ctx_t* ctx = amxb_request_get_ctx(request);
    amxd_status_t status = amxd_status_ok;

    // If resp_list contains more than 1 element, return list of lists where the inner list
    // contains the return variant at index 0 and the output args at index 1
    // This will only be the case for invokes on search paths that target multiple instances
    amxc_var_set_type(request->result, AMXC_VAR_ID_LIST);
    if(amxc_llist_size(resp_list) > 1) {
        amxc_llist_for_each(it, resp_list) {
            amxc_var_t* result = amxc_var_from_llist_it(it);
            amxc_var_t* dst_element = amxc_var_add(amxc_llist_t, request->result, NULL);
            status |= amxb_usp_operate_resp_to_ambiorix_variant_single(dst_element, result);
        }
    } else {
        amxc_var_t* result = amxc_var_from_llist_it(amxc_llist_get_first(resp_list));
        status = amxb_usp_operate_resp_to_ambiorix_variant_single(request->result, result);
    }

    if(request->done_fn != NULL) {
        // In the done_fn, the caller should call amxb_close_request to free up memory for the
        // amxb_request_t. When amxb_usp_request_done returns, we must make sure that the request
        // struct is no longer accessed, because it will lead to invalid reads
        free(usp_request->request->bus_data);
        usp_request->request->bus_data = NULL;
        usp_request->request = NULL;
        request->done_fn(ctx,
                         request,
                         status,
                         request->priv);
    }
}

void amxb_usp_operate_deferred_resp(const amxc_var_t* const data,
                                    amxb_usp_deferred_t* fcall,
                                    amxb_usp_deferred_child_t* child) {
    amxc_var_t* retval = GET_ARG(data, "retval");
    amxc_var_t* out_args = GET_ARG(data, "args");
    uint32_t status = GET_UINT32(data, "status");
    amxc_var_t copy_data;
    amxd_path_t path;
    amxd_object_t* object = NULL;
    char* requested_path = NULL;
    const char* method = NULL;
    const char* requested = NULL;
    const char* translated = NULL;

    amxc_var_init(&copy_data);
    amxc_var_copy(&copy_data, data);
    amxd_path_init(&path, child->requested_path);

    requested_path = strdup(amxd_path_get(&path, AMXD_OBJECT_TERMINATE));

    // Need to translate path to find object in local DM
    amxb_usp_translate_path(&path, &requested, &translated);
    object = amxd_dm_findf(fcall->ctx->dm, "%s", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
    method = amxd_path_get_param(&path);

    amxb_usp_la_set_request_status(child->request_index, status == amxd_status_ok ? "Success" : "Error");

    // Emit event in case of async method and send response in case of sync method
    // retval is re-populated in amxb_usp_la_emit_complete which is not correct. It is clearer to
    // do this in a separate function (and operate_resp of type req_obj_path is not needed in deferred resp)
    if(child->request_index > 0) {
        amxb_usp_la_emit_complete(object, child->request_index, retval, out_args, status);
    } else {
        amxb_usp_invoke_result_to_operate_resp(&fcall->resp_list, retval, child->args, status,
                                               requested_path, method, false, child->request_index);
        // If all calls are done, send response
        if(fcall->num_calls == 0) {
            amxb_usp_reply_deferred(fcall, uspl_operate_resp_new);
        }
    }

    // If all calls are done, delete fcall
    if(fcall->num_calls == 0) {
        amxb_usp_deferred_delete(&fcall);
    }

    free(requested_path);
    amxc_var_clean(&copy_data);
    amxd_path_clean(&path);
}

int amxb_usp_handle_operate(amxb_usp_t* ctx, uspl_rx_t* usp_rx) {
    int retval = -1;
    amxd_status_t status = amxd_status_unknown_error;
    amxc_var_t operate;
    amxc_llist_t resp_list;
    amxb_usp_deferred_t* fcall = NULL;

    amxc_var_init(&operate);
    amxc_llist_init(&resp_list);

    when_null(ctx, exit);
    when_null(usp_rx, exit);

    ctx->stats.counter_rx_invoke++;

    retval = uspl_operate_extract(usp_rx, &operate);
    if(retval != 0) {
        retval = amxb_usp_reply_error(ctx, usp_rx, retval);
        goto exit;
    }

    retval = amxb_usp_deferred_new(&fcall, ctx, usp_rx, 0);
    when_failed(retval, exit);

    status = amxb_usp_invoke_functions(fcall, &operate);

    // If status is deferred and function is synchronous, wait for deferred function to complete
    // before sending a response. The resp_list will be empty. If status is deferred and function is
    // synchronous, we need to send the OPERATE_RESP now with the (list of) req_obj_path(s).
    // If status is not deferred, just send the response and clean up the fcall.
    if((status == amxd_status_deferred) && (amxc_llist_size(&fcall->resp_list) == 0)) {
        retval = 0;
        goto exit;
    } else if((status == amxd_status_deferred) && (amxc_llist_size(&fcall->resp_list) > 0)) {
        retval = amxb_usp_reply(ctx, usp_rx, &fcall->resp_list, uspl_operate_resp_new);
    } else {
        retval = amxb_usp_reply(ctx, usp_rx, &fcall->resp_list, uspl_operate_resp_new);
        amxb_usp_deferred_delete(&fcall);
    }

exit:
    amxc_var_clean(&operate);
    amxc_llist_clean(&resp_list, variant_list_it_free);
    return retval;
}

int amxb_usp_invoke(void* const ctx,
                    amxb_invoke_t* invoke_ctx,
                    amxc_var_t* args,
                    amxb_request_t* request,
                    int timeout) {
    int retval = amxd_status_object_not_found;
    amxb_usp_t* amxb_usp = (amxb_usp_t*) ctx;
    amxc_var_t operate;
    amxc_var_t result;
    char* msg_id = NULL;
    bool requires_device = GET_BOOL(amxb_usp_get_config(), AMXB_USP_COPT_REQUIRES_DEVICE);

    amxc_var_init(&operate);
    amxc_var_init(&result);

    when_null(invoke_ctx->object, exit);
    if(requires_device) {
        when_false(amxb_usp_path_starts_with_device(invoke_ctx->object), exit);
    }

    amxb_usp_operate_populate_request(&operate, invoke_ctx->object, invoke_ctx->method, args);

    retval = amxb_usp_send_req(amxb_usp, &operate, uspl_operate_new, &msg_id);
    when_failed(retval, exit);

    retval = amxb_usp_poll_response(amxb_usp, msg_id, uspl_operate_resp_extract, &result, false, timeout);
    when_failed(retval, exit);

    retval = amxb_usp_operate_resp_to_ambiorix_variant(request->result, &result);

exit:
    free(msg_id);
    amxc_var_clean(&result);
    amxc_var_clean(&operate);
    return retval;
}

int amxb_usp_handle_operate_resp(amxb_usp_t* ctx, uspl_rx_t* usp_rx) {
    int retval = -1;
    amxc_llist_t resp_list;
    char* msg_id = NULL;
    usp_request_t* usp_request = NULL;
    amxc_htable_it_t* hit = NULL;

    amxc_llist_init(&resp_list);

    when_null(ctx, exit);
    when_null(usp_rx, exit);

    // Reply/response is not considered an rx action that must increase a statistics counter,
    // so no `ctx->stats.counter_rx_something` here.

    retval = uspl_operate_resp_extract(usp_rx, &resp_list);
    when_failed(retval, exit);

    msg_id = uspl_msghandler_msg_id(usp_rx);
    when_str_empty(msg_id, exit);

    hit = amxc_htable_take(&ctx->operate_requests, msg_id);
    when_null(hit, exit);

    usp_request = amxc_container_of(hit, usp_request_t, hit);
    amxb_usp_request_done(ctx, usp_request, &resp_list);

exit:
    amxc_htable_it_clean(hit, amxb_usp_request_free);
    amxc_llist_clean(&resp_list, variant_list_it_free);
    return retval;
}

int amxb_usp_async_invoke(void* const ctx,
                          amxb_invoke_t* invoke_ctx,
                          amxc_var_t* args,
                          amxb_request_t* request) {
    int retval = amxd_status_object_not_found;
    amxb_usp_t* amxb_usp = (amxb_usp_t*) ctx;
    char* from_id = amxb_usp_get_from_id();
    char* to_id = amxb_usp_get_to_id(amxb_usp);
    uspl_tx_t* usp_tx = NULL;
    usp_request_t* usp_request = NULL;
    amxc_var_t operate;
    bool requires_device = GET_BOOL(amxb_usp_get_config(), AMXB_USP_COPT_REQUIRES_DEVICE);

    amxc_var_init(&operate);

    when_null(invoke_ctx->object, exit);
    if(requires_device) {
        when_false(amxb_usp_path_starts_with_device(invoke_ctx->object), exit);
    }

    amxb_usp_operate_populate_request(&operate, invoke_ctx->object, invoke_ctx->method, args);

    retval = uspl_tx_new(&usp_tx, from_id, to_id);
    when_failed(retval, exit);

    retval = uspl_operate_new(usp_tx, &operate);
    when_failed(retval, exit);

    retval = amxb_usp_build_and_send_tlv(amxb_usp, usp_tx);
    when_failed(retval, exit);

    usp_request = (usp_request_t*) calloc(1, sizeof(usp_request_t));
    when_null(usp_request, exit);

    usp_request->request = request;
    request->bus_data = strdup(usp_tx->msg_id);
    amxc_htable_insert(&amxb_usp->operate_requests, usp_tx->msg_id, &usp_request->hit);

exit:
    free(to_id);
    free(from_id);
    uspl_tx_delete(&usp_tx);
    amxc_var_clean(&operate);
    return retval;
}

int amxb_usp_wait_for_request(void* const ctx,
                              UNUSED amxb_request_t* request,
                              int timeout) {
    struct pollfd fds[1];
    int fd = amxb_usp_get_fd(ctx);
    int retval = -1;
    sigset_t origmask;
    sigset_t sigmask;

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    /* Block SIGALARM to avoid interrupting poll */
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigmask, &origmask);

    retval = poll(fds, 1, timeout * 1000);

    sigprocmask(SIG_SETMASK, &origmask, NULL);

    if(retval <= 0) {
        retval = -1;
        goto exit;
    }

    if(fds[0].revents & POLLIN) {
        retval = amxb_usp_read(ctx);
    } else {
        retval = -1;
    }

exit:
    return retval;
}

int amxb_usp_close_request(void* const ctx, amxb_request_t* request) {
    char* msg_id = (char*) request->bus_data;
    amxb_usp_t* usp_ctx = (amxb_usp_t*) ctx;
    amxc_htable_it_t* hit = NULL;

    hit = amxc_htable_take(&usp_ctx->operate_requests, msg_id);
    amxc_htable_it_clean(hit, amxb_usp_request_free);

    return 0;
}
