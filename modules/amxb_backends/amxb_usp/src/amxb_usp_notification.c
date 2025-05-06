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

#include <yajl/yajl_gen.h>
#include <amxc/amxc_variant_type.h>
#include <amxj/amxj_variant.h>

#include "amxb_usp.h"

static int amxb_usp_notify_convert_event(const amxc_var_t* input_var, amxc_var_t* output_var) {
    int retval = -1;
    amxc_var_t* event = NULL;
    amxc_var_t* event_name = NULL;

    event = GET_ARG(input_var, "event");
    when_null(event, exit);

    event_name = amxc_var_take_key(event, "event_name");
    when_null(event_name, exit);

    // Need to cast everything back to the right type
    amxc_var_for_each(element, event) {
        char* value = amxc_var_dyncast(cstring_t, element);

        if(amxc_var_set(jstring_t, element, value) != 0) {
            amxc_var_set(cstring_t, element, value);
        }
        amxc_var_cast(element, AMXC_VAR_ID_ANY);
        free(value);
    }

    amxc_var_add_key(cstring_t, event, "notification", amxc_var_constcast(cstring_t, event_name));

    amxc_var_move(output_var, event);

    retval = 0;
exit:
    amxc_var_delete(&event_name);
    return retval;
}

static int amxb_usp_notify_convert_amx(const amxc_var_t* input_var, amxc_var_t* output_var) {
    int retval = -1;
    amxc_var_t* amx_notification = GET_ARG(input_var, "amx_notification");

    when_null(amx_notification, exit);

    amxc_var_move(output_var, amx_notification);

    retval = 0;
exit:
    return retval;
}

static int amxb_usp_notify_convert_value_change(const amxc_var_t* const input_var, amxc_var_t* output_var) {
    int retval = -1;
    amxc_var_t* value_change = GET_ARG(input_var, "value_change");
    amxc_var_t* parameters = NULL;
    amxc_var_t* param = NULL;
    amxd_path_t path;

    amxd_path_init(&path, NULL);

    when_null(value_change, exit);

    amxd_path_setf(&path, false, "%s", GET_CHAR(value_change, "param_path"));

    amxc_var_set_type(output_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, output_var, "path", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
    amxc_var_add_key(cstring_t, output_var, "object", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
    amxc_var_add_key(cstring_t, output_var, "notification", "dm:object-changed");
    parameters = amxc_var_add_key(amxc_htable_t, output_var, "parameters", NULL);
    param = amxc_var_add_key(amxc_htable_t, parameters, amxd_path_get_param(&path), NULL);

    // From is not provided by USP notification, so use empty string
    amxc_var_add_key(cstring_t, param, "from", "");
    amxc_var_add_key(cstring_t, param, "to", GET_CHAR(value_change, "param_value"));

    retval = 0;
exit:
    amxd_path_clean(&path);
    return retval;
}

static int amxb_usp_notify_convert_object_creation_deletion(const amxc_var_t* const input_var, amxc_var_t* output_var) {
    amxc_var_t* keys = NULL;
    amxc_var_t* parameters = NULL;
    amxd_path_t path;
    char* last = NULL;

    amxd_path_init(&path, NULL);

    amxd_path_setf(&path, true, "%s", GET_CHAR(input_var, "path"));
    last = amxd_path_get_last(&path, true);
    if((last != NULL) && (last[strlen(last) - 1] == '.')) {
        last[strlen(last) - 1] = 0;
    }

    amxc_var_set_type(output_var, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, output_var, "path", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
    amxc_var_add_key(cstring_t, output_var, "object", amxd_path_get(&path, AMXD_OBJECT_TERMINATE));
    amxc_var_add_key(cstring_t, output_var, "index", last);
    amxc_var_cast(GET_ARG(output_var, "index"), AMXC_VAR_ID_UINT32);
    keys = amxc_var_add_key(amxc_htable_t, output_var, "keys", NULL);
    parameters = amxc_var_add_key(amxc_htable_t, output_var, "parameters", NULL);

    amxc_var_for_each(key, GET_ARG(input_var, "keys")) {
        amxc_var_add_key(cstring_t, keys, amxc_var_key(key), amxc_var_constcast(cstring_t, key));
        amxc_var_add_key(cstring_t, parameters, amxc_var_key(key), amxc_var_constcast(cstring_t, key));
    }

    amxd_path_clean(&path);
    free(last);
    return 0;
}
static int amxb_usp_notify_convert_object_creation(const amxc_var_t* const input_var, amxc_var_t* output_var) {
    int retval = -1;
    amxc_var_t* obj_creation = GET_ARG(input_var, "obj_creation");

    when_null(obj_creation, exit);

    retval = amxb_usp_notify_convert_object_creation_deletion(obj_creation, output_var);
    amxc_var_add_key(cstring_t, output_var, "notification", "dm:instance-added");
    when_failed(retval, exit);

exit:
    return retval;
}

static int amxb_usp_notify_convert_object_deletion(const amxc_var_t* const input_var, amxc_var_t* output_var) {
    int retval = -1;
    amxc_var_t* obj_deletion = GET_ARG(input_var, "obj_deletion");

    when_null(obj_deletion, exit);

    retval = amxb_usp_notify_convert_object_creation_deletion(obj_deletion, output_var);
    amxc_var_add_key(cstring_t, output_var, "notification", "dm:instance-removed");

exit:
    return retval;
}

static int amxb_usp_notify_convert_operation_complete(amxc_var_t* input_var, amxc_var_t* output_var) {
    int retval = -1;
    amxc_var_t* oper_complete = GET_ARG(input_var, "oper_complete");
    amxc_var_t* operation_case = NULL;

    when_null(oper_complete, exit);

    amxc_var_move(output_var, oper_complete);
    operation_case = GET_ARG(output_var, "operation_case");
    amxc_var_delete(&operation_case);
    amxc_var_add_key(cstring_t, output_var, "notification", "OperationComplete");

    retval = 0;
exit:
    return retval;
}

/* Move the relevant information from the notify_var to a new variant */
static int amxb_usp_notify_convert(amxc_var_t* input_var, amxc_var_t* output_var) {
    int retval = -1;
    uint32_t notification_case = GET_UINT32(input_var, "notification_case");

    switch(notification_case) {
    case USP__NOTIFY__NOTIFICATION_EVENT:
        retval = amxb_usp_notify_convert_event(input_var, output_var);
        break;
    case USP__NOTIFY__NOTIFICATION_VALUE_CHANGE:
        retval = amxb_usp_notify_convert_value_change(input_var, output_var);
        break;
    case USP__NOTIFY__NOTIFICATION_OBJ_CREATION:
        retval = amxb_usp_notify_convert_object_creation(input_var, output_var);
        break;
    case USP__NOTIFY__NOTIFICATION_OBJ_DELETION:
        retval = amxb_usp_notify_convert_object_deletion(input_var, output_var);
        break;
    case USP__NOTIFY__NOTIFICATION_OPER_COMPLETE:
        retval = amxb_usp_notify_convert_operation_complete(input_var, output_var);
        break;
    case USP__NOTIFY__NOTIFICATION_AMX_NOTIFICATION:
        retval = amxb_usp_notify_convert_amx(input_var, output_var);
        break;
    default:
        break;
    }

    return retval;
}

static void amxb_usp_notify_emit_signals(amxb_usp_t* ctx, amxc_var_t* data) {
    const char* obj_path = NULL;
    uint32_t index = GET_UINT32(data, "index");
    amxd_path_t path;

    amxd_path_init(&path, NULL);

    obj_path = GET_CHAR(data, "path");
    when_str_empty(obj_path, exit);

    if(index > 0) {
        amxd_path_setf(&path, true, "%s%d", obj_path, index);
    } else {
        amxd_path_setf(&path, true, "%s", obj_path);
    }
    obj_path = amxd_path_get(&path, 0);

    amxc_htable_for_each(hit, &ctx->subscriptions) {
        const char* key = amxc_htable_it_get_key(hit);
        amxp_expr_t expr;
        amxp_expr_status_t status;
        amxc_string_t expr_builder;

        amxc_string_init(&expr_builder, 0);
        amxc_string_setf(&expr_builder, "'%s' starts with '%s'", obj_path, key);

        amxp_expr_init(&expr, amxc_string_get(&expr_builder, 0));
        if(amxp_expr_eval(&expr, &status)) {
            amxp_sigmngr_emit_signal(ctx->sigmngr, key, data);
        }

        amxp_expr_clean(&expr);
        amxc_string_clean(&expr_builder);
    }

exit:
    amxd_path_clean(&path);
}

int amxb_usp_handle_notify(amxb_usp_t* ctx, uspl_rx_t* usp_data) {
    int retval = -1;
    amxc_var_t notify_var;
    amxc_var_t converted_var;

    amxc_var_init(&notify_var);
    amxc_var_init(&converted_var);

    when_null(ctx, exit);
    when_null(usp_data, exit);

    ctx->stats.counter_rx_notify++;

    retval = uspl_notify_extract(usp_data, &notify_var);
    when_failed(retval, exit);

    retval = amxb_usp_notify_convert(&notify_var, &converted_var);
    when_failed(retval, exit);

    amxb_usp_notify_emit_signals(ctx, &converted_var);

    retval = 0;
exit:
    amxc_var_clean(&converted_var);
    amxc_var_clean(&notify_var);
    return retval;
}
