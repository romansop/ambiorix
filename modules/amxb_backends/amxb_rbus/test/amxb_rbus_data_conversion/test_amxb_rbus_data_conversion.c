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

#define _GNU_SOURCE
#include <string.h>
#include <unistd.h>

#include "../common/include/amxb_rbus_test.h"
#include "test_amxb_rbus_data_conversion.h"

int test_amx_setup(UNUSED void** state) {
    return 0;
}

int test_amx_teardown(UNUSED void** state) {
    return 0;
}

void test_amxb_rbus_rbus2amx_simple_types(UNUSED void** state) {
    rbusValue_t value = NULL;
    amxc_var_t var;

    amxc_var_init(&var);
    rbusValue_Init(&value);

    // string
    rbusValue_SetString(value, "Text");
    amxb_rbus_value_to_var(&var, value);
    assert_int_equal(amxc_var_type_of(&var), AMXC_VAR_ID_CSTRING);
    assert_string_equal(GET_CHAR(&var, NULL), "Text");

    // float
    rbusValue_SetSingle(value, 3.14);
    amxb_rbus_value_to_var(&var, value);
    assert_int_equal(amxc_var_type_of(&var), AMXC_VAR_ID_DOUBLE);
    assert_float_equal(amxc_var_constcast(double, &var), 3.14, 0);

    // char
    rbusValue_SetChar(value, 'a');
    amxb_rbus_value_to_var(&var, value);
    assert_int_equal(amxc_var_type_of(&var), AMXC_VAR_ID_INT8);
    assert_int_equal(amxc_var_constcast(int8_t, &var), (int8_t) 'a');

    // byte
    rbusValue_SetByte(value, 128);
    amxb_rbus_value_to_var(&var, value);
    assert_int_equal(amxc_var_type_of(&var), AMXC_VAR_ID_UINT8);
    assert_int_equal(amxc_var_constcast(uint8_t, &var), 128);

    // date time
    rbusValue_SetByte(value, 128);
    amxb_rbus_value_to_var(&var, value);
    assert_int_equal(amxc_var_type_of(&var), AMXC_VAR_ID_UINT8);
    assert_int_equal(amxc_var_constcast(uint8_t, &var), 128);

    rbusValue_Release(value);
    amxc_var_clean(&var);
}

void test_amxb_rbus_rbus2amx_time_type(UNUSED void** state) {
    amxc_ts_t now;
    amxc_var_t var;
    rbusValue_t value = NULL;
    rbusDateTime_t* rbus_dt = NULL;
    char* time_str = NULL;
    const char* test_time = "2024-06-21T06:37:24Z";

    amxc_var_init(&var);
    assert_int_equal(amxc_ts_parse(&now, test_time, strlen(test_time)), 0);
    amxc_var_set(amxc_ts_t, &var, &now);
    time_str = amxc_var_dyncast(cstring_t, &var);
    assert_string_equal(time_str, "2024-06-21T06:37:24Z");
    free(time_str);

    rbusValue_Init(&value);
    amxb_rbus_var_to_rvalue(value, &var);
    rbus_dt = (rbusDateTime_t*) rbusValue_GetTime(value);
    rbus_dt->m_tz.m_tzhour = 1;
    rbus_dt->m_tz.m_isWest = true;

    amxb_rbus_value_to_var(&var, value);
    time_str = amxc_var_dyncast(cstring_t, &var);
    assert_string_equal(time_str, "2024-06-21T05:37:24-01:00");
    free(time_str);

    rbusValue_Release(value);
    amxc_var_clean(&var);
}

void test_amxb_rbus_amx2rbus_htable_type(UNUSED void** state) {
    amxc_var_t original;
    amxc_var_t result;
    rbusValue_t value = NULL;
    amxc_var_t* field4 = NULL;
    int compare_result = 0;

    amxc_var_init(&original);
    amxc_var_init(&result);
    rbusValue_Init(&value);

    amxc_var_set_type(&original, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &original, "field1", "Text");
    amxc_var_add_key(uint64_t, &original, "field2", 7000000);
    amxc_var_add_key(int64_t, &original, "field3", -7000000);
    amxc_var_add_new_key(&original, "field4");

    // convert htable variant to rbus value and back
    amxc_var_dump(&original, STDOUT_FILENO);
    amxb_rbus_var_to_rvalue(value, &original);
    amxb_rbus_value_to_var(&result, value);
    amxc_var_dump(&result, STDOUT_FILENO);

    // amxc_var_compare can't handle NULL variants
    field4 = GET_ARG(&original, "field4");
    assert_non_null(field4);
    assert_true(amxc_var_is_null(field4));
    amxc_var_delete(&field4);
    field4 = GET_ARG(&result, "field4");
    assert_non_null(field4);
    assert_true(amxc_var_is_null(field4));
    amxc_var_delete(&field4);

    // the end result should be exactly the same
    assert_int_equal(amxc_var_compare(&original, &result, &compare_result), 0);
    assert_int_equal(compare_result, 0);

    rbusValue_Release(value);
    amxc_var_clean(&result);
    amxc_var_clean(&original);
}
