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
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>

#include <amxc/amxc_set.h>

#include "test_amxc_set.h"

#include <amxc/amxc_macros.h>

void test_amxc_set_new_delete_null(UNUSED void** state) {
    // passing NULL pointers should not lead to segfault
    assert_int_not_equal(amxc_set_new(NULL, 0), 0);
    amxc_set_delete(NULL);
}

void test_amxc_set_init_reset_clean_null(UNUSED void** state) {
    // passing NULL pointers should not lead to segfault
    assert_int_equal(amxc_set_init(NULL, 0), -1);
    amxc_set_clean(NULL);
    amxc_set_reset(NULL);
}

void test_amxc_set_new_delete(UNUSED void** state) {
    amxc_set_t* set = NULL;

    assert_int_equal(amxc_set_new(&set, false), 0);
    assert_non_null(set);
    assert_true(amxc_llist_is_empty(&set->list));
    assert_false(set->counted);
    assert_int_equal(set->count, 0);
    assert_null(set->alert_handler);
    assert_null(set->priv);
    amxc_set_delete(&set);
    assert_null(set);

    assert_int_equal(amxc_set_new(&set, true), 0);
    assert_non_null(set);
    assert_true(amxc_llist_is_empty(&set->list));
    assert_true(set->counted);
    assert_int_equal(set->count, 0);
    assert_null(set->alert_handler);
    assert_null(set->priv);
    amxc_set_delete(&set);
    assert_null(set);
}

void test_amxc_set_init_reset_clean(UNUSED void** state) {
    amxc_set_t set;

    assert_int_equal(amxc_set_init(&set, false), 0);
    assert_true(amxc_llist_is_empty(&set.list));
    assert_false(set.counted);
    assert_int_equal(set.count, 0);
    assert_null(set.alert_handler);
    assert_null(set.priv);
    amxc_set_clean(&set);

    assert_int_equal(amxc_set_init(&set, true), 0);
    assert_true(amxc_llist_is_empty(&set.list));
    assert_true(set.counted);
    assert_int_equal(set.count, 0);
    assert_null(set.alert_handler);
    assert_null(set.priv);
    amxc_set_clean(&set);
}

void test_amxc_can_add_remove_flags(UNUSED void** state) {
    amxc_set_t set;

    assert_int_equal(amxc_set_init(&set, false), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    assert_int_equal(amxc_set_get_count(NULL, NULL), 0);
    assert_int_equal(amxc_set_get_count(NULL, "flag1"), 0);

    amxc_set_add_flag(&set, "flag1");
    assert_int_equal(amxc_llist_size(&set.list), 1);
    amxc_set_add_flag(&set, "flag1");
    assert_int_equal(amxc_llist_size(&set.list), 1);
    assert_int_equal(amxc_set_get_count(&set, "flag1"), 1);
    assert_int_equal(amxc_set_get_count(&set, NULL), 1);

    amxc_set_add_flag(&set, "flag2");
    assert_int_equal(amxc_llist_size(&set.list), 2);

    amxc_set_add_flag(&set, "flag3");
    assert_int_equal(amxc_llist_size(&set.list), 3);
    assert_int_equal(amxc_set_get_count(&set, NULL), 3);

    amxc_set_remove_flag(&set, "flag2");
    assert_int_equal(amxc_llist_size(&set.list), 2);
    assert_int_equal(amxc_set_get_count(&set, NULL), 2);

    assert_false(amxc_set_has_flag(&set, NULL));
    assert_false(amxc_set_has_flag(&set, ""));
    assert_false(amxc_set_has_flag(NULL, "flag1"));
    assert_true(amxc_set_has_flag(&set, "flag1"));
    assert_true(amxc_set_has_flag(&set, "flag3"));

    amxc_set_clean(&set);
}

void test_amxc_can_add_remove_counted_flags(UNUSED void** state) {
    amxc_set_t set;

    assert_int_equal(amxc_set_init(&set, true), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    amxc_set_add_flag(&set, "flag1");
    assert_int_equal(amxc_llist_size(&set.list), 1);
    amxc_set_add_flag(&set, "flag1");
    assert_int_equal(amxc_llist_size(&set.list), 1);
    assert_int_equal(amxc_set_get_count(&set, "flag1"), 2);
    assert_int_equal(amxc_set_get_count(&set, NULL), 2);

    amxc_set_add_flag(&set, "flag2");
    assert_int_equal(amxc_llist_size(&set.list), 2);
    assert_int_equal(amxc_set_get_count(&set, NULL), 3);

    amxc_set_add_flag(&set, "flag3");
    assert_int_equal(amxc_llist_size(&set.list), 3);
    assert_int_equal(amxc_set_get_count(&set, NULL), 4);
    amxc_set_add_flag(&set, "flag3");
    amxc_set_add_flag(&set, "flag3");
    assert_int_equal(amxc_set_get_count(&set, "flag3"), 3);
    assert_int_equal(amxc_set_get_count(&set, NULL), 6);

    amxc_set_add_flag(&set, "");
    assert_int_equal(amxc_set_get_count(&set, NULL), 6);

    amxc_set_add_flag(&set, NULL);
    assert_int_equal(amxc_set_get_count(&set, NULL), 6);

    amxc_set_remove_flag(&set, "flag2");
    assert_int_equal(amxc_llist_size(&set.list), 2);
    assert_int_equal(amxc_set_get_count(&set, NULL), 5);
    assert_true(amxc_set_has_flag(&set, "flag1"));
    assert_true(amxc_set_has_flag(&set, "flag3"));

    amxc_set_remove_flag(&set, "flag1");
    assert_true(amxc_set_has_flag(&set, "flag1"));
    assert_int_equal(amxc_set_get_count(&set, "flag1"), 1);
    assert_int_equal(amxc_set_get_count(&set, NULL), 4);
    amxc_set_remove_flag(&set, "flag1");
    assert_false(amxc_set_has_flag(&set, "flag1"));
    assert_int_equal(amxc_set_get_count(&set, "flag1"), 0);
    assert_int_equal(amxc_set_get_count(&set, NULL), 3);
    amxc_set_remove_flag(&set, "flag1");
    assert_false(amxc_set_has_flag(&set, "flag1"));
    assert_int_equal(amxc_set_get_count(&set, "flag1"), 0);
    assert_int_equal(amxc_set_get_count(&set, NULL), 3);

    amxc_set_remove_flag(&set, "");
    assert_int_equal(amxc_set_get_count(&set, NULL), 3);

    amxc_set_remove_flag(&set, NULL);
    assert_int_equal(amxc_set_get_count(&set, NULL), 3);

    amxc_set_clean(&set);
}

void test_amxc_can_subtract_set(UNUSED void** state) {
    amxc_set_t set1;
    amxc_set_t set2;

    assert_int_equal(amxc_set_init(&set1, false), 0);
    assert_int_equal(amxc_set_init(&set2, false), 0);

    amxc_set_add_flag(&set1, "flag1");
    amxc_set_add_flag(&set1, "flag2");
    amxc_set_add_flag(&set1, "flag3");
    amxc_set_add_flag(&set1, "flag4");
    amxc_set_add_flag(&set1, "flag5");

    amxc_set_add_flag(&set2, "flag2");
    amxc_set_add_flag(&set2, "flag4");

    amxc_set_subtract(&set1, &set2);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 3);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 2);

    assert_false(amxc_set_has_flag(&set1, "flag2"));
    assert_false(amxc_set_has_flag(&set1, "flag4"));
    assert_true(amxc_set_has_flag(&set1, "flag1"));
    assert_true(amxc_set_has_flag(&set1, "flag3"));
    assert_true(amxc_set_has_flag(&set1, "flag5"));

    amxc_set_clean(&set1);
    amxc_set_clean(&set2);
}

void test_amxc_can_check_sets_is_subset(UNUSED void** state) {
    amxc_set_t set1;
    amxc_set_t set2;

    assert_int_equal(amxc_set_init(&set1, false), 0);
    assert_int_equal(amxc_set_init(&set2, false), 0);

    assert_true(amxc_set_is_subset(&set1, &set2));
    assert_true(amxc_set_is_subset(&set1, NULL));
    assert_true(amxc_set_is_subset(NULL, &set2));
    assert_true(amxc_set_is_subset(NULL, NULL));

    amxc_set_add_flag(&set1, "flag1");
    amxc_set_add_flag(&set1, "flag2");
    amxc_set_add_flag(&set1, "flag3");
    amxc_set_add_flag(&set1, "flag4");

    assert_false(amxc_set_is_subset(&set1, &set2));

    amxc_set_add_flag(&set2, "flag2");
    amxc_set_add_flag(&set2, "flag4");
    amxc_set_add_flag(&set2, "flag3");
    amxc_set_add_flag(&set2, "flag1");

    assert_true(amxc_set_is_subset(&set1, &set2));
    assert_true(amxc_set_is_subset(&set2, &set1));
    amxc_set_remove_flag(&set2, "flag4");
    assert_false(amxc_set_is_subset(&set1, &set2));
    assert_true(amxc_set_is_subset(&set2, &set1));
    amxc_set_remove_flag(&set1, "flag3");
    assert_false(amxc_set_is_subset(&set1, &set2));
    assert_false(amxc_set_is_subset(&set2, &set1));

    amxc_set_clean(&set1);
    amxc_set_clean(&set2);
}

void test_amxc_can_check_sets_are_equal(UNUSED void** state) {
    amxc_set_t set1;
    amxc_set_t set2;

    assert_int_equal(amxc_set_init(&set1, false), 0);
    assert_int_equal(amxc_set_init(&set2, false), 0);

    assert_true(amxc_set_is_equal(&set1, &set2));
    assert_true(amxc_set_is_equal(&set1, NULL));
    assert_true(amxc_set_is_equal(NULL, &set2));
    assert_true(amxc_set_is_equal(NULL, NULL));

    amxc_set_add_flag(&set1, "flag1");
    amxc_set_add_flag(&set1, "flag2");
    amxc_set_add_flag(&set1, "flag3");
    amxc_set_add_flag(&set1, "flag4");

    assert_false(amxc_set_is_equal(&set1, &set2));

    amxc_set_add_flag(&set2, "flag2");
    amxc_set_add_flag(&set2, "flag4");
    amxc_set_add_flag(&set2, "flag3");
    amxc_set_add_flag(&set2, "flag1");

    assert_true(amxc_set_is_equal(&set1, &set2));
    assert_true(amxc_set_is_equal(&set2, &set1));
    amxc_set_remove_flag(&set2, "flag4");
    assert_false(amxc_set_is_equal(&set1, &set2));
    assert_false(amxc_set_is_equal(&set2, &set1));

    amxc_set_clean(&set1);
    amxc_set_clean(&set2);
}

void test_amxc_set_parse(UNUSED void** state) {
    amxc_set_t set;

    assert_int_equal(amxc_set_init(&set, false), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    assert_int_equal(amxc_set_parse(&set, "flag1 flag2 flag3"), 0);
    assert_false(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_llist_size(&set.list), 3);
    assert_true(amxc_set_has_flag(&set, "flag1"));
    assert_true(amxc_set_has_flag(&set, "flag3"));
    assert_false(amxc_set_has_flag(&set, "flag5"));

    amxc_set_reset(&set);
    assert_true(amxc_llist_is_empty(&set.list));

    assert_int_equal(amxc_set_parse(&set, "flag1 flag2 flag3"), 0);
    assert_false(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_llist_size(&set.list), 3);
    assert_int_equal(amxc_set_parse(&set, "flag1 flag4 flag5 flag6 flag1"), 0);
    assert_false(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_llist_size(&set.list), 4);
    assert_true(amxc_set_has_flag(&set, "flag1"));
    assert_false(amxc_set_has_flag(&set, "flag3"));
    assert_true(amxc_set_has_flag(&set, "flag5"));

    amxc_set_reset(&set);
    assert_true(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_set_parse(&set, "abcd abc ab a"), 0);
    assert_false(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_llist_size(&set.list), 4);
    assert_true(amxc_set_has_flag(&set, "abcd"));
    assert_true(amxc_set_has_flag(&set, "abc"));
    assert_true(amxc_set_has_flag(&set, "ab"));
    assert_true(amxc_set_has_flag(&set, "a"));

    amxc_set_clean(&set);
    assert_true(amxc_llist_is_empty(&set.list));
}

void test_amxc_set_parse_with_count(UNUSED void** state) {
    amxc_set_t set;

    assert_int_equal(amxc_set_init(&set, true), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    assert_int_equal(amxc_set_parse(&set, "flag1 flag2:10 flag3:5"), 0);
    assert_false(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_llist_size(&set.list), 3);
    assert_true(amxc_set_has_flag(&set, "flag1"));
    assert_true(amxc_set_has_flag(&set, "flag3"));
    assert_false(amxc_set_has_flag(&set, "flag5"));
    assert_int_equal(amxc_set_get_count(&set, "flag1"), 1);
    assert_int_equal(amxc_set_get_count(&set, "flag3"), 5);
    assert_int_equal(amxc_set_get_count(&set, "flag5"), 0);
    assert_int_equal(amxc_set_get_count(&set, NULL), 16);

    amxc_set_reset(&set);
    assert_true(amxc_llist_is_empty(&set.list));

    assert_int_equal(amxc_set_parse(&set, "flag1:0 flag2:3 flag3:10"), 0);
    assert_false(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_llist_size(&set.list), 2);
    assert_false(amxc_set_has_flag(&set, "flag1"));
    assert_true(amxc_set_has_flag(&set, "flag3"));
    assert_false(amxc_set_has_flag(&set, "flag5"));
    assert_int_equal(amxc_set_get_count(&set, "flag1"), 0);
    assert_int_equal(amxc_set_get_count(&set, "flag3"), 10);
    assert_int_equal(amxc_set_get_count(&set, "flag5"), 0);
    assert_int_equal(amxc_set_get_count(&set, NULL), 13);

    assert_int_equal(amxc_set_parse(&set, "flag1 flag4:2 flag5 flag6:8 flag1:4"), 0);
    assert_false(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_llist_size(&set.list), 4);
    assert_int_equal(amxc_set_get_count(&set, NULL), 16);
    assert_int_equal(amxc_set_get_count(&set, "flag1"), 5);

    assert_int_equal(amxc_set_parse(&set, ""), 0);
    assert_true(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_set_parse(&set, NULL), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    amxc_set_clean(&set);
    assert_true(amxc_llist_is_empty(&set.list));

    assert_int_equal(amxc_set_parse(&set, "flag1:8"), 0);
    assert_false(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_llist_size(&set.list), 1);
    assert_int_equal(amxc_set_get_count(&set, "flag1"), 8);
    assert_int_equal(amxc_set_get_count(&set, NULL), 8);

    assert_int_equal(amxc_set_parse(&set, "flag1:3 flag1:4"), 0);
    assert_false(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_llist_size(&set.list), 1);
    assert_int_equal(amxc_set_get_count(&set, "flag1"), 7);
    assert_int_equal(amxc_set_get_count(&set, NULL), 7);

    amxc_set_reset(&set);
    assert_true(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_set_parse(&set, "abcd:2 abc:3 ab:4 a:2"), 0);
    assert_false(amxc_llist_is_empty(&set.list));
    assert_int_equal(amxc_llist_size(&set.list), 4);
    assert_int_equal(amxc_set_get_count(&set, "abcd"), 2);
    assert_int_equal(amxc_set_get_count(&set, "abc"), 3);
    assert_int_equal(amxc_set_get_count(&set, "ab"), 4);
    assert_int_equal(amxc_set_get_count(&set, "a"), 2);
    assert_int_equal(amxc_set_get_count(&set, NULL), 11);

    amxc_set_clean(&set);
    assert_true(amxc_llist_is_empty(&set.list));
}

void test_amxc_set_parse_invalid_input(UNUSED void** state) {
    amxc_set_t set;

    assert_int_equal(amxc_set_init(&set, true), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    assert_int_not_equal(amxc_set_parse(NULL, "flag1 flag2:10 flag3:5"), 0);
    assert_int_not_equal(amxc_set_parse(&set, "flag1:flag2:5 flag3"), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    amxc_set_clean(&set);
}

void test_amxc_set_to_string_counted(UNUSED void** state) {
    amxc_set_t set;
    char* set_str = NULL;

    assert_int_equal(amxc_set_init(&set, true), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    assert_int_equal(amxc_set_parse(&set, "flag1 flag2:10 flag3:5"), 0);
    set_str = amxc_set_to_string(&set);

    assert_string_equal(set_str, "flag1 flag2:10 flag3:5");

    free(set_str);
    amxc_set_clean(&set);
}

void test_amxc_set_to_string(UNUSED void** state) {
    amxc_set_t set;
    char* set_str = NULL;

    assert_int_equal(amxc_set_init(&set, false), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    assert_int_equal(amxc_set_parse(&set, "flag1 flag2:10 flag3:5"), 0);
    set_str = amxc_set_to_string(&set);

    assert_string_equal(set_str, "flag1 flag2 flag3");

    free(set_str);
    amxc_set_clean(&set);
}

void test_amxc_set_to_string_sep(UNUSED void** state) {
    amxc_set_t set;
    char* set_str = NULL;

    assert_int_equal(amxc_set_init(&set, false), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    assert_int_equal(amxc_set_parse(&set, "flag1 flag2:10 flag3:5"), 0);
    set_str = amxc_set_to_string_sep(&set, ",");

    assert_string_equal(set_str, "flag1,flag2,flag3");

    free(set_str);
    amxc_set_clean(&set);
}

void test_amxc_set_copy(UNUSED void** state) {
    amxc_set_t* set = NULL;
    amxc_set_t* copy = NULL;

    assert_null(amxc_set_copy(NULL));

    assert_int_equal(amxc_set_new(&set, false), 0);
    assert_non_null(set);

    copy = amxc_set_copy(set);
    assert_non_null(copy);
    assert_true(amxc_set_is_equal(set, copy));
    assert_null(copy->alert_handler);
    assert_null(copy->priv);
    amxc_set_delete(&copy);
    assert_null(copy);

    assert_int_equal(amxc_set_parse(set, "flag1 flag2 flag3"), 0);

    copy = amxc_set_copy(set);
    assert_non_null(copy);
    assert_true(amxc_set_is_equal(set, copy));
    assert_null(copy->alert_handler);
    assert_null(copy->priv);
    amxc_set_delete(&copy);
    assert_null(copy);

    amxc_set_delete(&set);
    assert_null(set);

    assert_int_equal(amxc_set_new(&set, true), 0);
    assert_non_null(set);

    copy = amxc_set_copy(set);
    assert_non_null(copy);
    assert_true(amxc_set_is_equal(set, copy));
    assert_null(copy->alert_handler);
    assert_null(copy->priv);
    amxc_set_delete(&copy);
    assert_null(copy);

    assert_int_equal(amxc_set_parse(set, "flag1:3 flag2:5 flag3:23"), 0);

    copy = amxc_set_copy(set);
    assert_non_null(copy);
    assert_true(amxc_set_is_equal(set, copy));
    assert_null(copy->alert_handler);
    assert_null(copy->priv);
    amxc_set_delete(&copy);
    assert_null(copy);

    amxc_set_delete(&set);
    assert_null(set);
}

void test_amxc_set_symmetric_difference(UNUSED void** state) {
    amxc_set_t set1;
    amxc_set_t set2;
    amxc_set_t cmp;

    assert_int_equal(amxc_set_init(&set1, false), 0);
    assert_int_equal(amxc_set_init(&set2, false), 0);
    assert_int_equal(amxc_set_init(&cmp, false), 0);

    amxc_set_symmetric_difference(NULL, NULL);

    assert_int_equal(amxc_set_parse(&set1, "flag1 flag2 flag3"), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag1 flag4 flag5"), 0);
    assert_int_equal(amxc_set_parse(&cmp, "flag1 flag2 flag3"), 0);

    amxc_set_symmetric_difference(&set1, NULL);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_reset(&cmp);
    assert_int_equal(amxc_set_parse(&cmp, "flag2 flag3 flag4 flag5"), 0);
    amxc_set_symmetric_difference(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_clean(&set1);
    amxc_set_clean(&set2);
    amxc_set_clean(&cmp);
    assert_int_equal(amxc_set_init(&set1, true), 0);
    assert_int_equal(amxc_set_init(&set2, true), 0);
    assert_int_equal(amxc_set_init(&cmp, true), 0);

    assert_int_equal(amxc_set_parse(&set1, "flag1:3 flag2:2 flag3:6"), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag1:2 flag4:8 flag5:9"), 0);
    assert_int_equal(amxc_set_parse(&cmp, "flag1:3 flag2:2 flag3:6"), 0);

    amxc_set_symmetric_difference(&set1, NULL);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_reset(&cmp);
    assert_int_equal(amxc_set_parse(&cmp, "flag1:1 flag2:2 flag3:6 flag4:8 flag5:9"), 0);
    amxc_set_symmetric_difference(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_clean(&set1);
    amxc_set_clean(&set2);
    amxc_set_clean(&cmp);
}

void test_amxc_set_union(UNUSED void** state) {
    amxc_set_t set1;
    amxc_set_t set2;
    amxc_set_t cmp;

    assert_int_equal(amxc_set_init(&set1, false), 0);
    assert_int_equal(amxc_set_init(&set2, false), 0);
    assert_int_equal(amxc_set_init(&cmp, false), 0);

    amxc_set_union(NULL, NULL);
    amxc_set_union(NULL, &set1);

    assert_int_equal(amxc_set_parse(&set1, "flag flag1 flag2"), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag       flag2 flag3"), 0);
    assert_int_equal(amxc_set_parse(&cmp, " flag flag1 flag2 flag3"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 3);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 3);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 4);

    amxc_set_union(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_union(&set1, NULL);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_union(&set1, &set1);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, ""), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag       flag2 flag3"), 0);
    assert_int_equal(amxc_set_parse(&cmp, " flag       flag2 flag3"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 0);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 3);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 3);
    assert_true(amxc_set_is_equal(&set2, &cmp));

    amxc_set_union(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, "flag flag1 flag2"), 0);
    assert_int_equal(amxc_set_parse(&set2, ""), 0);
    assert_int_equal(amxc_set_parse(&cmp, " flag flag1 flag2"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 3);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 0);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 3);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_union(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_clean(&set1);
    amxc_set_clean(&set2);
    amxc_set_clean(&cmp);
}

void test_amxc_set_union_with_count(UNUSED void** state) {
    amxc_set_t set1;
    amxc_set_t set2;
    amxc_set_t cmp;

    assert_int_equal(amxc_set_init(&set1, true), 0);
    assert_int_equal(amxc_set_init(&set2, true), 0);
    assert_int_equal(amxc_set_init(&cmp, true), 0);

    amxc_set_union(NULL, NULL);
    amxc_set_union(NULL, &set1);

    assert_int_equal(amxc_set_parse(&set1, "flag   flag1:4 flag2:3"), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag:2         flag2:2 flag3:6"), 0);
    assert_int_equal(amxc_set_parse(&cmp, " flag:3 flag1:4 flag2:5 flag3:6"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 8);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 10);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 18);

    amxc_set_union(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_union(&set1, NULL);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, "flag   flag1:4 flag2:3"), 0);
    assert_int_equal(amxc_set_parse(&cmp, " flag:2 flag1:8 flag2:6"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 8);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 16);
    amxc_set_union(&set1, &set1);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, ""), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag:2         flag2:2 flag3:6"), 0);
    assert_int_equal(amxc_set_parse(&cmp, " flag:2         flag2:2 flag3:6"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 0);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 10);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 10);
    assert_true(amxc_set_is_equal(&set2, &cmp));

    amxc_set_union(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, "flag flag1:4 flag2:3"), 0);
    assert_int_equal(amxc_set_parse(&set2, ""), 0);
    assert_int_equal(amxc_set_parse(&cmp, " flag flag1:4 flag2:3"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 8);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 0);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 8);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_union(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_clean(&set1);
    amxc_set_clean(&set2);
    amxc_set_clean(&cmp);
}

void test_amxc_set_intersect(UNUSED void** state) {
    amxc_set_t set1;
    amxc_set_t set2;
    amxc_set_t cmp;

    assert_int_equal(amxc_set_init(&set1, false), 0);
    assert_int_equal(amxc_set_init(&set2, false), 0);
    assert_int_equal(amxc_set_init(&cmp, false), 0);

    amxc_set_intersect(NULL, NULL);
    amxc_set_intersect(NULL, &set1);

    assert_int_equal(amxc_set_parse(&set1, "flag flag1 flag2"), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag       flag2 flag3"), 0);
    assert_int_equal(amxc_set_parse(&cmp, " flag       flag2"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 3);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 3);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 2);

    amxc_set_intersect(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_reset(&cmp);
    amxc_set_intersect(&set1, NULL);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&cmp, "flag       flag2 flag3"), 0);
    amxc_set_intersect(&set2, &set2);
    assert_true(amxc_set_is_equal(&set2, &cmp));

    assert_int_equal(amxc_set_parse(&set1, ""), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag       flag2 flag3"), 0);
    assert_int_equal(amxc_set_parse(&cmp, ""), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 0);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 3);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 0);

    amxc_set_intersect(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, "flag flag1 flag2"), 0);
    assert_int_equal(amxc_set_parse(&set2, ""), 0);
    assert_int_equal(amxc_set_parse(&cmp, ""), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 3);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 0);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 0);

    amxc_set_intersect(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_clean(&set1);
    amxc_set_clean(&set2);
    amxc_set_clean(&cmp);
}

void test_amxc_set_intersect_with_count(UNUSED void** state) {
    amxc_set_t set1;
    amxc_set_t set2;
    amxc_set_t cmp;

    assert_int_equal(amxc_set_init(&set1, true), 0);
    assert_int_equal(amxc_set_init(&set2, true), 0);
    assert_int_equal(amxc_set_init(&cmp, true), 0);

    amxc_set_intersect(NULL, NULL);
    amxc_set_intersect(NULL, &set1);

    assert_int_equal(amxc_set_parse(&set1, "flag   flag1:4 flag2:3         flag4:5"), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag:2         flag2:2 flag3:6 flag4:5"), 0);
    assert_int_equal(amxc_set_parse(&cmp, " flag:1         flag2:2         flag4:5"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 13);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 15);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 8);

    amxc_set_intersect(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_reset(&cmp);
    amxc_set_intersect(&set1, NULL);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, "flag   flag1:4 flag2:3"), 0);
    assert_int_equal(amxc_set_parse(&cmp, " flag:1 flag1:4 flag2:3"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 8);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 8);
    amxc_set_intersect(&set1, &set1);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, ""), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag:2         flag2:2 flag3:6"), 0);
    assert_int_equal(amxc_set_parse(&cmp, ""), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 0);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 10);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 0);

    amxc_set_intersect(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, "flag flag1:4 flag2:3"), 0);
    assert_int_equal(amxc_set_parse(&set2, ""), 0);
    assert_int_equal(amxc_set_parse(&cmp, ""), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 8);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 0);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 0);

    amxc_set_intersect(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_clean(&set1);
    amxc_set_clean(&set2);
    amxc_set_clean(&cmp);
}

void test_amxc_set_subtract(UNUSED void** state) {
    amxc_set_t set1;
    amxc_set_t set2;
    amxc_set_t cmp;

    assert_int_equal(amxc_set_init(&set1, false), 0);
    assert_int_equal(amxc_set_init(&set2, false), 0);
    assert_int_equal(amxc_set_init(&cmp, false), 0);

    amxc_set_subtract(NULL, NULL);
    amxc_set_subtract(NULL, &set1);

    assert_int_equal(amxc_set_parse(&set1, "flag flag1 flag2"), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag       flag2 flag3"), 0);
    assert_int_equal(amxc_set_parse(&cmp, "      flag1"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 3);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 3);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 1);

    amxc_set_subtract(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_subtract(&set1, NULL);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, "flag flag1 flag2"), 0);
    assert_int_equal(amxc_set_parse(&cmp, ""), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 3);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 0);
    amxc_set_subtract(&set1, &set1);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, ""), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag       flag2 flag3"), 0);
    assert_int_equal(amxc_set_parse(&cmp, ""), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 0);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 3);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 0);

    amxc_set_subtract(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, "flag flag1 flag2"), 0);
    assert_int_equal(amxc_set_parse(&set2, ""), 0);
    assert_int_equal(amxc_set_parse(&cmp, "flag flag1 flag2"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 3);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 0);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 3);

    amxc_set_subtract(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_clean(&set1);
    amxc_set_clean(&set2);
    amxc_set_clean(&cmp);
}

void test_amxc_set_subtract_with_count(UNUSED void** state) {
    amxc_set_t set1;
    amxc_set_t set2;
    amxc_set_t cmp;

    assert_int_equal(amxc_set_init(&set1, true), 0);
    assert_int_equal(amxc_set_init(&set2, true), 0);
    assert_int_equal(amxc_set_init(&cmp, true), 0);

    amxc_set_subtract(NULL, NULL);
    amxc_set_subtract(NULL, &set1);

    assert_int_equal(amxc_set_parse(&set1, "flag   flag1:4 flag2:3         flag4:5"), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag:2         flag2:2 flag3:6 flag4:5"), 0);
    assert_int_equal(amxc_set_parse(&cmp, "        flag1:4 flag2:1                "), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 13);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 15);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 5);

    amxc_set_subtract(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_subtract(&set1, NULL);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, "flag   flag1:4 flag2:3"), 0);
    assert_int_equal(amxc_set_parse(&cmp, ""), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 8);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 0);
    amxc_set_subtract(&set1, &set1);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, ""), 0);
    assert_int_equal(amxc_set_parse(&set2, "flag:2         flag2:2 flag3:6"), 0);
    assert_int_equal(amxc_set_parse(&cmp, ""), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 0);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 10);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 0);

    amxc_set_subtract(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    assert_int_equal(amxc_set_parse(&set1, "flag flag1:4 flag2:3"), 0);
    assert_int_equal(amxc_set_parse(&set2, ""), 0);
    assert_int_equal(amxc_set_parse(&cmp, " flag flag1:4 flag2:3"), 0);
    assert_int_equal(amxc_set_get_count(&set1, NULL), 8);
    assert_int_equal(amxc_set_get_count(&set2, NULL), 0);
    assert_int_equal(amxc_set_get_count(&cmp, NULL), 8);

    amxc_set_subtract(&set1, &set2);
    assert_true(amxc_set_is_equal(&set1, &cmp));

    amxc_set_clean(&set1);
    amxc_set_clean(&set2);
    amxc_set_clean(&cmp);
}


void test_amxc_set_get_first_flag(UNUSED void** state) {
    amxc_set_t set;
    const amxc_flag_t* flag;

    assert_int_equal(amxc_set_init(&set, false), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    amxc_set_add_flag(&set, "flag1");
    amxc_set_add_flag(&set, "flag2");

    flag = amxc_set_get_first_flag(&set);

    assert_string_equal(flag->flag, "flag1");

    amxc_set_clean(&set);
}

void test_amxc_set_get_first_flag_when_null(UNUSED void** state) {
    amxc_set_t set;
    const amxc_flag_t* flag;

    assert_int_equal(amxc_set_init(&set, false), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    flag = amxc_set_get_first_flag(&set);

    assert_null(flag);

    amxc_set_clean(&set);
}

void test_amxc_flag_get_next(UNUSED void** state) {
    amxc_set_t set;
    const amxc_flag_t* flag1;
    const amxc_flag_t* flag2;

    assert_int_equal(amxc_set_init(&set, false), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    amxc_set_add_flag(&set, "flag1");
    amxc_set_add_flag(&set, "flag2");

    flag1 = amxc_set_get_first_flag(&set);
    flag2 = amxc_flag_get_next((amxc_flag_t*) flag1);

    assert_string_equal(flag2->flag, "flag2");

    amxc_set_clean(&set);
}

void test_amxc_flag_get_next_when_null(UNUSED void** state) {
    amxc_set_t set;
    const amxc_flag_t* flag1;
    const amxc_flag_t* flag2;

    assert_int_equal(amxc_set_init(&set, false), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    amxc_set_add_flag(&set, "flag1");

    flag1 = amxc_set_get_first_flag(&set);
    flag2 = amxc_flag_get_next((amxc_flag_t*) flag1);

    assert_null(flag2);

    amxc_set_clean(&set);
}

void test_amxc_flag_get_value(UNUSED void** state) {
    amxc_set_t set;
    const amxc_flag_t* flag;

    assert_int_equal(amxc_set_init(&set, false), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    amxc_set_add_flag(&set, "flag1");

    flag = amxc_set_get_first_flag(&set);

    assert_string_equal(amxc_flag_get_value((amxc_flag_t*) flag), "flag1");

    amxc_set_clean(&set);
}

void test_amxc_set_iterate(UNUSED void** state) {
    amxc_set_t set;
    int i = 0;

    const char* flags[] = {
        "flag1",
        "flag2",
        "flag3",
    };

    assert_int_equal(amxc_set_init(&set, false), 0);
    assert_true(amxc_llist_is_empty(&set.list));

    amxc_set_add_flag(&set, flags[0]);
    amxc_set_add_flag(&set, flags[1]);
    amxc_set_add_flag(&set, flags[2]);

    amxc_set_iterate(flag, &set) {
        const char* flag_name = amxc_flag_get_value((amxc_flag_t*) flag);
        assert_string_equal(flag_name, flags[i]);
        i++;
    }

    assert_int_equal(i, 3);

    amxc_set_clean(&set);
}

static void test_amxc_set_alert(amxc_set_t* set,
                                const char* flag,
                                bool value,
                                void* priv) {
    function_called();

    assert_non_null(set);
    assert_non_null(flag);

    check_expected(flag);
    check_expected(value);
    check_expected(priv);
}

void test_amxc_set_callback(UNUSED void** state) {
    amxc_set_t set;
    int ref = 4;
    assert_int_equal(amxc_set_init(&set, false), 0);
    amxc_set_alert_cb(&set, test_amxc_set_alert, &ref);

    expect_function_call(test_amxc_set_alert);
    expect_string(test_amxc_set_alert, flag, "flag1");
    expect_value(test_amxc_set_alert, value, true);
    expect_value(test_amxc_set_alert, priv, &ref);
    amxc_set_add_flag(&set, "flag1");

    expect_function_call(test_amxc_set_alert);
    expect_string(test_amxc_set_alert, flag, "flag2");
    expect_value(test_amxc_set_alert, value, true);
    expect_value(test_amxc_set_alert, priv, &ref);
    amxc_set_add_flag(&set, "flag2");

    expect_function_call(test_amxc_set_alert);
    expect_string(test_amxc_set_alert, flag, "flag1");
    expect_value(test_amxc_set_alert, value, false);
    expect_value(test_amxc_set_alert, priv, &ref);
    amxc_set_remove_flag(&set, "flag1");

    amxc_set_clean(&set);
}
