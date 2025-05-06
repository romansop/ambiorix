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

#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxd/amxd_transaction.h>

#include "amxb_usp.h"
#include "amxb_usp_la.h"

uint32_t amxb_usp_la_add_request(const char* originator,
                                 const char* path,
                                 const char* func,
                                 const char* key) {
    amxd_trans_t trans;
    uint32_t index = 0;
    amxd_dm_t* dm = amxb_usp_get_la();
    amxc_string_t command;

    amxc_string_init(&command, 0);

    when_null(dm, exit);
    when_str_empty(originator, exit);
    when_str_empty(path, exit);
    when_str_empty(func, exit);
    when_str_empty(key, exit);

    amxd_trans_init(&trans);
    amxd_trans_set_attr(&trans, amxd_tattr_change_ro, true);
    amxc_string_setf(&command, "%s%s()", path, func);
    amxd_trans_select_pathf(&trans, "Device.LocalAgent.Request.");
    amxd_trans_add_inst(&trans, 0, NULL);
    amxd_trans_set_value(cstring_t, &trans, "Originator", originator);
    amxd_trans_set_value(cstring_t, &trans, "Command", amxc_string_get(&command, 0));
    amxd_trans_set_value(cstring_t, &trans, "CommandKey", key);

    if(amxd_trans_apply(&trans, dm) == amxd_status_ok) {
        index = GETP_UINT32(&trans.retvals, "1.index");
    }
    amxd_trans_clean(&trans);

exit:
    amxc_string_clean(&command);
    return index;
}

amxd_status_t amxb_usp_la_set_request_status(uint32_t index,
                                             const char* status) {
    amxd_trans_t trans;
    amxd_status_t retval = amxd_status_invalid_value;
    amxd_dm_t* dm = amxb_usp_get_la();
    amxc_string_t command;

    amxc_string_init(&command, 0);

    when_null(dm, exit);
    when_true(index == 0, exit);

    amxd_trans_init(&trans);
    amxd_trans_set_attr(&trans, amxd_tattr_change_ro, true);
    amxd_trans_select_pathf(&trans, "Device.LocalAgent.Request.%d.", index);
    amxd_trans_set_value(cstring_t, &trans, "Status", status);

    retval = amxd_trans_apply(&trans, dm);
    amxd_trans_clean(&trans);

exit:
    amxc_string_clean(&command);
    return retval;
}

void amxb_usp_la_remove_request(UNUSED const char* const sig_name,
                                const amxc_var_t* const data,
                                UNUSED void* const priv) {
    amxd_dm_t* dm = amxb_usp_get_la();
    amxd_object_t* request = amxd_dm_signal_get_object(dm, data);
    amxd_trans_t trans;

    amxd_trans_init(&trans);
    amxd_trans_set_attr(&trans, amxd_tattr_change_ro, true);
    amxd_trans_select_pathf(&trans, "Device.LocalAgent.Request.");
    amxd_trans_del_inst(&trans, amxd_object_get_index(request), NULL);

    amxd_trans_apply(&trans, dm);
    amxd_trans_clean(&trans);
}

void amxb_usp_la_emit_complete(amxd_object_t* object,
                               uint32_t request_index,
                               amxc_var_t* ret,
                               amxc_var_t* out,
                               amxd_status_t status) {
    amxc_var_t event_data;
    amxc_var_t* data = NULL;
    amxc_var_t* output_args = NULL;
    amxd_dm_t* dm = amxb_usp_get_la();
    amxd_object_t* request_obj = amxd_dm_findf(dm, "Device.LocalAgent.Request.%d.", request_index);
    const char* key = GET_CHAR(amxd_object_get_param_value(request_obj, "CommandKey"), NULL);
    const char* cmd = GET_CHAR(amxd_object_get_param_value(request_obj, "Command"), NULL);
    amxd_path_t full_cmd;

    amxd_path_init(&full_cmd, NULL);
    amxc_var_init(&event_data);
    when_null(request_obj, exit);

    amxd_path_setf(&full_cmd, false, "%s", cmd);

    amxc_var_set_type(&event_data, AMXC_VAR_ID_HTABLE);
    data = amxc_var_add_key(amxc_htable_t, &event_data, "data", NULL);
    amxc_var_set_key(data, "output_args", out, AMXC_VAR_FLAG_COPY);
    output_args = GET_ARG(data, "output_args");
    amxc_var_set_key(output_args, "_retval", ret, AMXC_VAR_FLAG_COPY);
    amxc_var_add_key(uint32_t, data, "status", status);
    amxc_var_add_key(cstring_t, data, "cmd", amxd_path_get_param(&full_cmd));
    amxc_var_add_key(cstring_t, data, "cmd_key", key);

    amxd_object_emit_signal(object, "dm:operation-complete", &event_data);

exit:
    amxd_path_clean(&full_cmd);
    amxc_var_clean(&event_data);
}