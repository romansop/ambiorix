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

#include <stdlib.h>
#include <string.h>

#include "amxb_usp.h"
#include "amxb_usp_la.h"
#include "amxb_usp_threshold_expr.h"

amxd_status_t amxb_usp_la_threshold_instance_is_valid(amxd_object_t* object,
                                                      UNUSED amxd_param_t* param,
                                                      UNUSED amxd_action_t reason,
                                                      UNUSED const amxc_var_t* const args,
                                                      UNUSED amxc_var_t* const retval,
                                                      UNUSED void* priv) {
    amxd_status_t status = amxd_status_unknown_error;
    amxc_var_t params;

    bool enabled = false;
    const char* ref_path = NULL;
    const char* ref_param = NULL;

    amxc_var_init(&params);
    amxd_object_get_params(object, &params, amxd_dm_access_protected);

    enabled = GET_BOOL(&params, "Enable");
    ref_path = GET_CHAR(&params, "ReferencePath");
    ref_param = GET_CHAR(&params, "ThresholdParam");

    if(enabled) {
        if((ref_path == NULL) || (*ref_path == 0)) {
            status = amxd_status_invalid_value;
            goto exit;
        }
        if((ref_param == NULL) || (*ref_param == 0)) {
            status = amxd_status_invalid_value;
            goto exit;
        }
    }

    status = amxd_status_ok;

exit:
    amxc_var_clean(&params);
    return status;
}

amxd_status_t amxb_usp_la_threshold_instance_cleanup(amxd_object_t* object,
                                                     UNUSED amxd_param_t* param,
                                                     UNUSED amxd_action_t reason,
                                                     UNUSED const amxc_var_t* const args,
                                                     UNUSED amxc_var_t* const retval,
                                                     UNUSED void* priv) {
    amxd_status_t status = amxd_status_unknown_error;
    amxb_usp_t* ctx = NULL;

    ctx = (amxb_usp_t*) object->priv;
    when_null(ctx, exit);
    when_null(ctx->dm, exit);

    threshold_unsubscribe(object, ctx);

    status = amxd_status_ok;

exit:
    return status;
}
