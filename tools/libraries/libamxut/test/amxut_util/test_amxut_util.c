/****************************************************************************
**
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
** SPDX-FileCopyrightText: Copyright (c) 2024 SoftAtHome
**
** Redistribution and use in source and binary forms, with or
** without modification, are permitted provided that the following
** conditions are met:
**
** 1. Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** 2. Redistributions in binary form must reproduce the above
** copyright notice, this list of conditions and the following
** disclaimer in the documentation and/or other materials provided
** with the distribution.
**
** Subject to the terms and conditions of this license, each
** copyright holder and contributor hereby grants to those receiving
** rights under this license a perpetual, worldwide, non-exclusive,
** no-charge, royalty-free, irrevocable (except for failure to
** satisfy the conditions of this license) patent license to make,
** have made, use, offer to sell, sell, import, and otherwise
** transfer this software, where such license applies only to those
** patent claims, already acquired or hereafter acquired, licensable
** by such copyright holder or contributor that are necessarily
** infringed by:
**
** (a) their Contribution(s) (the licensed copyrights of copyright
** holders and non-copyrightable additions of contributors, in
** source or binary form) alone; or
**
** (b) combination of their Contribution(s) with the work of
** authorship to which such Contribution(s) was added by such
** copyright holder or contributor, if, at the time the Contribution
** is added, such addition causes such combination to be necessarily
** infringed. The patent license shall not apply to any other
** combinations which include the Contribution.
**
** Except as expressly stated above, no rights or licenses from any
** copyright holder or contributor is granted under this license,
** whether expressly, by implication, estoppel or otherwise.
**
** DISCLAIMER
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
** CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
** USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
** AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
** ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "test_amxut_util.h"
#include <stdlib.h> // Needed for cmocka
#include <setjmp.h> // Needed for cmocka
#include <stdarg.h> // Needed for cmocka
#include <cmocka.h>
#include <amxut/amxut_util.h>
#include <debug/sahtrace.h>
#include <amxc/amxc_macros.h>

void amxut_util_read_json_from_file__normal_case(UNUSED void** state) {
    // GIVEN a file with json data
    const char* file = "jsonfile.json";

    // WHEN reading the file
    amxc_var_t* contents = amxut_util_read_json_from_file(file);

    // THEN its contents is the variant representation of the file
    assert_non_null(contents);
    assert_string_equal(GETP_CHAR(contents, "a string"), "mystring");
    assert_true(GETP_BOOL(contents, "a boolean") == true);
    assert_true(GETP_BOOL(contents, "another boolean") == false);
    assert_int_equal(GETP_INT32(contents, "a map.a submap.an integer"), 1234);
    assert_int_equal(GETP_INT32(contents, "a map.a submap.a list of integers.0"), 1);
    assert_int_equal(GETP_INT32(contents, "a map.a submap.a list of integers.1"), 2);
    assert_int_equal(GETP_INT32(contents, "a map.a submap.a list of integers.2"), 3);
    assert_int_equal(GETP_INT32(contents, "a map.a submap.a list of integers.3"), 4);
    assert_string_equal(GETP_CHAR(contents, "a map.a submap.a list of maps.0.day"), "Monday");
    assert_string_equal(GETP_CHAR(contents, "a map.a submap.a list of maps.0.food"), "pancakes");
    assert_string_equal(GETP_CHAR(contents, "a map.a submap.a list of maps.1.day"), "Friday");
    assert_string_equal(GETP_CHAR(contents, "a map.a submap.a list of maps.1.food"), "pasta");

    // THEN the types are as expected
    assert_int_equal(amxc_var_type_of(contents), AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxc_var_type_of(GETP_ARG(contents, "a string")), AMXC_VAR_ID_CSTRING);
    assert_int_equal(amxc_var_type_of(GETP_ARG(contents, "a boolean")), AMXC_VAR_ID_BOOL);
    assert_int_equal(amxc_var_type_of(GETP_ARG(contents, "a map")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxc_var_type_of(GETP_ARG(contents, "a map.a submap")), AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxc_var_type_of(GETP_ARG(contents, "a map.a submap.an integer")), AMXC_VAR_ID_INT64);
    assert_int_equal(amxc_var_type_of(GETP_ARG(contents, "a map.a submap.a list of integers")), AMXC_VAR_ID_LIST);

    // cleanup
    amxc_var_delete(&contents);
}

void amxut_util_read_write_json(UNUSED void** state) {
    // GIVEN a variant and a filename
    const char* filename = "read_write.json";
    amxc_var_t input_var;
    amxc_var_t* output_var = NULL;
    amxc_var_t* table = NULL;
    amxc_var_t* list = NULL;
    int result = 0;

    amxc_var_init(&input_var);
    amxc_var_set_type(&input_var, AMXC_VAR_ID_HTABLE);
    table = amxc_var_add_key(amxc_htable_t, &input_var, "table", NULL);
    amxc_var_add_key(cstring_t, table, "text", "pizza");
    amxc_var_add_key(uint32_t, table, "number", 99);
    list = amxc_var_add_key(amxc_llist_t, &input_var, "list", NULL);
    amxc_var_add(bool, list, true);
    amxc_var_add(bool, list, false);

    // WHEN writing it to a file
    amxut_util_write_to_json_file(&input_var, filename);

    // AND reading it back from the file
    output_var = amxut_util_read_json_from_file(filename);
    assert_non_null(output_var);

    // THEN the variant that is read is the same as the original variant
    assert_int_equal(amxc_var_compare(&input_var, output_var, &result), 0);
    assert_int_equal(result, 0);

    // cleanup
    amxc_var_clean(&input_var);
    amxc_var_delete(&output_var);
}
