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
#ifndef __TEST_AMXB_RBUS_COMMON_H__
#define __TEST_AMXB_RBUS_COMMON_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>

#include <dlfcn.h>
#include <setjmp.h>
#include <cmocka.h>

#include <rbus.h>
#include <rbuscore.h>

#include <amxc/amxc_macros.h>
#include <amxc/amxc.h>
#include <yajl/yajl_gen.h>
#include <amxj/amxj_variant.h>
#include <amxp/amxp.h>
#include <amxd/amxd_types.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_transaction.h>
#include <amxo/amxo.h>
#include <amxb/amxb.h>
#include <amxb/amxb_register.h>

#include <amxut/amxut_util.h>

#include "../include_priv/amxb_rbus.h"

typedef struct _expected {
    const char* name;
    int32_t type;
    uint32_t access;
} expected_type_t;

void cmocka_enable_check(bool enable);
void cmocka_enable_mock(bool enable);
void cmocka_rbus_backend_enable_check(const char* so, bool enable);
void cmocka_rbus_backend_enable_mock(const char* so, bool enable);

int handle_events(void);

int check_rbus_value_str(rbusHandle_t handle, const char* param, const char* expected);
int check_rbus_value_uint32(rbusHandle_t handle, const char* param, uint32_t expected);
int check_rbus_value_int32(rbusHandle_t handle, const char* param, int32_t expected);
int check_rbus_value_bool(rbusHandle_t handle, const char* param, bool expected);
int check_rbus_value_uint8(rbusHandle_t handle, const char* param, uint8_t expected);
int check_rbus_value_int8(rbusHandle_t handle, const char* param, int8_t expected);

void check_supported_datamodel_is_available(rbusHandle_t handle, const char* component, const char* expected[], int line);
void check_supported_datamodel_is_unavailable(rbusHandle_t handle, const char* component, int line);
int check_rows_are_available(rbusHandle_t handle, const char* table, const char* expected[], int line);
void check_element_info(rbusHandle_t handle, const char* element_name, int depth, expected_type_t* expected, int line);

int check_amx_response(amxc_var_t* result, const char* data_file);

#endif // __TEST_AMXB_RBUS_COMMON_H__