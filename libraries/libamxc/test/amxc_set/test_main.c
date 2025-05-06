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
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "test_amxc_set.h"

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_amxc_set_new_delete_null),
        cmocka_unit_test(test_amxc_set_init_reset_clean_null),
        cmocka_unit_test(test_amxc_set_new_delete),
        cmocka_unit_test(test_amxc_set_init_reset_clean),
        cmocka_unit_test(test_amxc_can_add_remove_flags),
        cmocka_unit_test(test_amxc_can_add_remove_counted_flags),
        cmocka_unit_test(test_amxc_can_subtract_set),
        cmocka_unit_test(test_amxc_can_check_sets_is_subset),
        cmocka_unit_test(test_amxc_can_check_sets_are_equal),
        cmocka_unit_test(test_amxc_set_parse),
        cmocka_unit_test(test_amxc_set_parse_with_count),
        cmocka_unit_test(test_amxc_set_parse_invalid_input),
        cmocka_unit_test(test_amxc_set_to_string_counted),
        cmocka_unit_test(test_amxc_set_to_string),
        cmocka_unit_test(test_amxc_set_to_string_sep),
        cmocka_unit_test(test_amxc_set_copy),
        cmocka_unit_test(test_amxc_set_symmetric_difference),
        cmocka_unit_test(test_amxc_set_union),
        cmocka_unit_test(test_amxc_set_union_with_count),
        cmocka_unit_test(test_amxc_set_intersect),
        cmocka_unit_test(test_amxc_set_intersect_with_count),
        cmocka_unit_test(test_amxc_set_subtract),
        cmocka_unit_test(test_amxc_set_subtract_with_count),
        cmocka_unit_test(test_amxc_set_get_first_flag),
        cmocka_unit_test(test_amxc_set_get_first_flag_when_null),
        cmocka_unit_test(test_amxc_flag_get_next),
        cmocka_unit_test(test_amxc_flag_get_next_when_null),
        cmocka_unit_test(test_amxc_flag_get_value),
        cmocka_unit_test(test_amxc_set_iterate),
        cmocka_unit_test(test_amxc_set_callback),
    };
    return cmocka_run_group_tests_name("amxc-set", tests, NULL, NULL);
}
