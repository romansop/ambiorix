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

#include <stdlib.h> // Needed for cmocka
#include <setjmp.h> // Needed for cmocka
#include <stdarg.h> // Needed for cmocka
#include <cmocka.h>

#include <amxut/amxut_verify.h>
#include <debug/sahtrace.h>
#include <amxc/amxc_macros.h>

#include "test_amxut_verify.h"

static amxc_var_t* test_build_expected_variant(void) {
    amxc_var_t* result = NULL;
    amxc_var_t* table = NULL;
    amxc_var_t* list = NULL;

    amxc_var_new(&result);
    amxc_var_set_type(result, AMXC_VAR_ID_HTABLE);

    table = amxc_var_add_key(amxc_htable_t, result, "table", NULL);
    amxc_var_add_key(cstring_t, table, "text", "pizza");
    amxc_var_add_key(uint32_t, table, "number", 99);
    list = amxc_var_add_key(amxc_llist_t, result, "list", NULL);
    amxc_var_add(bool, list, true);
    amxc_var_add(bool, list, false);

    return result;
}

static void test_some_event_callback_function(amxc_var_t* data) {
    // This represents an event callback function that takes a data variant as input argument.
    // We want to check that the data variant is as expected
    check_expected(data);
}

void test_amxut_verify_variant_from_json_file(UNUSED void** state) {
    const char* filename = "data.json";
    amxc_var_t* expected_variant = NULL;

    expected_variant = test_build_expected_variant();
    assert_int_equal(amxut_verify_variant_from_json_file(expected_variant, filename), 0);

    amxc_var_delete(&expected_variant);
}

void test_amxut_verify_variant_equal_check(UNUSED void** state) {
    amxc_var_t* expected_variant = NULL;
    amxc_var_t* actual_variant = NULL;

    expected_variant = test_build_expected_variant();
    expect_check(test_some_event_callback_function, data, amxut_verify_variant_equal_check, expected_variant);

    // Normally this would be provided by some actual means, but for testing the test function
    // we're just going to take the exact same variant.
    actual_variant = test_build_expected_variant();

    // Assume the callback function is called from somewhere
    test_some_event_callback_function(actual_variant);

    // We only need to clean up the actual variant. The expected variant will be cleaned up
    // by amxut_verify_variant_equal_check
    amxc_var_delete(&actual_variant);
}
