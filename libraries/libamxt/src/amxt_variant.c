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

#include <amxt/amxt_variant.h>

#include "amxt_priv.h"

static int variant_tty_init(amxc_var_t* const var) {
    var->data.data = NULL;

    return 0;
}

static void variant_tty_delete(amxc_var_t* var) {
    var->data.data = NULL;
}

static int variant_tty_copy(amxc_var_t* const dest,
                            const amxc_var_t* const src) {
    amxt_tty_t* src_tty = (amxt_tty_t*) src->data.data;

    dest->data.data = src_tty;
    return 0;
}

static int variant_tty_move(amxc_var_t* const dest,
                            amxc_var_t* const src) {
    amxt_tty_t* src_tty = (amxt_tty_t*) src->data.data;

    dest->data.data = src_tty;
    src->data.data = NULL;
    return 0;
}

static int variant_tty_convert_to_null(amxc_var_t* const dest,
                                       UNUSED const amxc_var_t* const src) {
    dest->data.data = NULL;
    return 0;
}

static int variant_tty_convert_to_fd(amxc_var_t* const dest,
                                     const amxc_var_t* const src) {
    amxt_tty_t* tty = (amxt_tty_t*) src->data.data;
    dest->data.fd = amxt_tty_fd(tty);
    return 0;
}

static int variant_tty_convert_to(amxc_var_t* const dest,
                                  const amxc_var_t* const src) {
    int retval = -1;

    amxc_var_convert_fn_t convfn[AMXC_VAR_ID_CUSTOM_BASE] = {
        variant_tty_convert_to_null,    // NULL
        NULL,                           // CSTRING
        NULL,                           // INT8
        NULL,                           // INT16
        NULL,                           // INT32
        NULL,                           // INT64
        NULL,                           // UINT8
        NULL,                           // UIN16
        NULL,                           // UINT32
        NULL,                           // UINT64
        NULL,                           // FLOAT
        NULL,                           // DOUBLE
        NULL,                           // BOOL
        NULL,                           // LIST
        NULL,                           // HTABLE
        variant_tty_convert_to_fd,      // FD
        NULL,                           // TIMESTAMP
        NULL,                           // CSV_STRING
        NULL,                           // SSV_STRING
        variant_tty_convert_to_fd       // ANY
    };

    if(dest->type_id >= AMXC_VAR_ID_CUSTOM_BASE) {
        goto exit;
    }

    if(convfn[dest->type_id] != NULL) {
        if(dest->type_id == AMXC_VAR_ID_ANY) {
            amxc_var_set_type(dest, AMXC_VAR_ID_FD);
        }
        retval = convfn[dest->type_id](dest, src);
    }

exit:
    return retval;
}

static amxc_var_type_t variant_tty = {
    .init = variant_tty_init,
    .del = variant_tty_delete,
    .copy = variant_tty_copy,
    .move = variant_tty_move,
    .convert_from = NULL,
    .convert_to = variant_tty_convert_to,
    .compare = NULL,
    .get_key = NULL,
    .set_key = NULL,
    .get_index = NULL,
    .set_index = NULL,
    .add_value = NULL,
    .type_id = 0,
    .hit = { .ait = NULL, .key = NULL, .next = NULL },
    .name = AMXC_VAR_NAME_TTY
};

CONSTRUCTOR_LVL(200) static void variant_tty_register(void) {
    amxc_var_register_type(&variant_tty);
}

DESTRUCTOR_LVL(200) static void variant_tty_unregister(void) {
    amxc_var_unregister_type(&variant_tty);
}

amxt_tty_t* amxc_var_get_const_amxt_tty_t(const amxc_var_t* const var) {
    amxt_tty_t* retval = NULL;

    when_null(var, exit);
    when_true(var->type_id != AMXC_VAR_ID_TTY, exit);

    retval = (amxt_tty_t*) var->data.data;
exit:
    return retval;
}

amxt_tty_t* amxc_var_take_amxt_tty_t(amxc_var_t* const var) {
    amxt_tty_t* retval = NULL;

    when_null(var, exit);
    when_true(var->type_id != AMXC_VAR_ID_TTY, exit);

    retval = (amxt_tty_t*) var->data.data;
    var->data.data = NULL;
    var->type_id = AMXC_VAR_ID_NULL;

exit:
    return retval;
}

int amxc_var_set_amxt_tty_t(amxc_var_t* const var, amxt_tty_t* const val) {
    int retval = -1;

    when_null(var, exit);
    when_null(val, exit);
    when_failed(amxc_var_set_type(var, AMXC_VAR_ID_TTY), exit);

    var->data.data = val;

    retval = 0;

exit:
    return retval;
}

amxc_var_t* amxc_var_add_new_amxt_tty_t(amxc_var_t* const var,
                                        amxt_tty_t* val) {
    amxc_var_t* subvar = NULL;

    when_null(var, exit);
    subvar = amxc_var_add_new(var);
    when_null(subvar, exit);

    if(amxc_var_set_amxt_tty_t(subvar, val) != 0) {
        amxc_var_delete(&subvar);
    }

exit:
    return subvar;
}

amxc_var_t* amxc_var_add_new_key_amxt_tty_t(amxc_var_t* const var,
                                            const char* key,
                                            amxt_tty_t* val) {
    amxc_var_t* subvar = NULL;

    when_null(var, exit);
    subvar = amxc_var_add_new_key(var, key);
    when_null(subvar, exit);

    if(amxc_var_set_amxt_tty_t(subvar, val) != 0) {
        amxc_var_delete(&subvar);
    }

exit:
    return subvar;
}

