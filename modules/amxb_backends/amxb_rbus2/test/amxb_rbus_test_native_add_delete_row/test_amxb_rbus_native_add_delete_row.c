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

#include <unistd.h>

#include "../common/include/amxb_rbus_test.h"
#include "test_amxb_rbus_native_add_delete_row.h"

rbusHandle_t handle = NULL;

int test_native_setup(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    cmocka_enable_check(true);

    assert_int_equal(rbus_checkStatus(), RBUS_ENABLED);

    printf("SETUP: Starting amxrt data model providers\n");
    //system("amxrt -D ../common/odl/softwaremodules.odl"); // TODO: Add tests - translation functionality
    system("amxrt -D ../common/odl/registration_test.odl");
    system("amxrt -D ../common/odl/restricted_instances.odl");
    printf("SETUP: Started ...\n");

    sleep(1);

    printf("SETUP: Connect to rbus\n");
    expect_string(__wrap_rbus_open, componentName, "rbus-unit-test");
    rc = rbus_open(&handle, "rbus-unit-test");
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("SETUP: Connected ...\n");

    return 0;
}

int test_native_teardown(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;

    printf("TEARDOWN: Stopping amxrt data model providers\n");
    system("killall amxrt");
    printf("TEARDOWN: Stopped ...\n");

    printf("TEARDOWN: Disconnect from rbus\n");
    rc = rbus_close(handle);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    printf("TEARDOWN: Disconnected ...\n");

    return 0;
}

void test_native_rbus_add_row(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    uint32_t index = 0;
    amxc_string_t expected_alias;
    amxc_string_t row_path;
    const char* expected_rows_1[] = { "TopLevel.Level1.1.Level2B.1.",
        "TopLevel.Level1.1.Level2B.2." };
    const char* expected_rows_2[] = { "TopLevel.Level1.1.Level2B.1.",
        "TopLevel.Level1.1.Level2B.2.",
        "TopLevel.Level1.1.Level2B.3." };

    amxc_string_init(&expected_alias, 0);
    amxc_string_init(&row_path, 0);

    // When adding a row to an existing table without providing the Alias
    rc = rbusTable_addRow(handle, "TopLevel.Level1.1.Level2B.", NULL, &index);

    // the return code must RBUS_ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);

    // the index must be 1
    assert_int_equal(index, 2);

    amxc_string_setf(&expected_alias, "cpe-Level2B-%d", index);
    amxc_string_setf(&row_path, "TopLevel.Level1.1.Level2B.%d.Alias", index);

    // the row must exist and the alias must match the expected generated Alias
    assert_int_equal(check_rbus_value_str(handle, amxc_string_get(&row_path, 0), amxc_string_get(&expected_alias, 0)), 0);
    // Other parameters should have the default values as defined in the template object
    amxc_string_setf(&row_path, "TopLevel.Level1.1.Level2B.%d.L2BText", index);
    assert_int_equal(check_rbus_value_str(handle, amxc_string_get(&row_path, 0), ""), 0);
    amxc_string_setf(&row_path, "TopLevel.Level1.1.Level2B.%d.L2BBool", index);
    assert_int_equal(check_rbus_value_bool(handle, amxc_string_get(&row_path, 0), false), 0);

    // the row must be returned when fetching the row names
    check_rows_are_available(handle, "TopLevel.Level1.1.Level2B.", expected_rows_1, __LINE__);

    // When adding a row to an existing table without providing the Alias
    rc = rbusTable_addRow(handle, "TopLevel.Level1.1.Level2B.", NULL, &index);

    // the return code must RBUS_ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);

    // the index must be 2
    assert_int_equal(index, 3);

    // the row must be returned when fetching the row names
    check_rows_are_available(handle, "TopLevel.Level1.1.Level2B.", expected_rows_2, __LINE__);

    amxc_string_clean(&expected_alias);
    amxc_string_clean(&row_path);
}

void test_native_rbus_add_row_on_non_tables_must_fail(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    uint32_t index = 0;

    // When adding a row to non table (singleton)
    rc = rbusTable_addRow(handle, "TopLevel.Level1.1.Level2A.", NULL, &index);

    // the return code must not be RBUS_ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);

    // When adding a row to non table (instance)
    rc = rbusTable_addRow(handle, "TopLevel.Level1.1.", NULL, &index);

    // the return code must not be RBUS_ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_del_row(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    char* str_val = NULL;
    int num_vals = 0;
    rbusProperty_t output_vals = NULL;
    const char* partial_path[1] = {"TopLevel.Level1.1.Level2B.2."};
    const char* expected_rows_1[] = { "TopLevel.Level1.1.Level2B.1.",
        "TopLevel.Level1.1.Level2B.3." };

    // When deleting an existing row
    rc = rbusTable_removeRow(handle, "TopLevel.Level1.1.Level2B.2.");

    // the return code must RBUS_ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);

    // the row must not be provided anymore in the list
    check_rows_are_available(handle, "TopLevel.Level1.1.Level2B.", expected_rows_1, __LINE__);

    // and the row should not be accessible anymore
    rc = rbus_getExt(handle, 1, partial_path, &num_vals, &output_vals);

    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
    rc = rbus_getStr(handle, "TopLevel.Level1.1.Level2B.2.L2BText", &str_val);
    // The return code must not be ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_del_non_existing_row(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    const char* expected_rows_1[] = { "TopLevel.Level1.1.Level2B.1.",
        "TopLevel.Level1.1.Level2B.3." };

    // When deleting an non-existing row
    rc = rbusTable_removeRow(handle, "TopLevel.Level1.1.Level2B.2.");

    // the return code must RBUS_ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);

    // all existing rows must still be available
    check_rows_are_available(handle, "TopLevel.Level1.1.Level2B.", expected_rows_1, __LINE__);
}

void test_native_rbus_del_non_rows_must_fail(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;

    // When deleting a table
    rc = rbusTable_removeRow(handle, "TopLevel.Level1.1.Level2B.");

    // the return code must RBUS_ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);

    // When deleting a singleton
    rc = rbusTable_removeRow(handle, "TopLevel.Level1.1.Level2A.");

    // the return code must RBUS_ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
}

void test_native_rbus_gets_error_when_adding_row_fails(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    uint32_t index = 0;
    const char* expected_rows_1[] = { "Restricted.ROObject.1.", "Restricted.ROObject.2." };
    const char* expected_rows_2[] = { "Restricted.LimitedInstances.1.", "Restricted.LimitedInstances.2."};
    char* str_val = NULL;

    // Check if the tables exists and contains rows:
    rc = rbus_getStr(handle, "Restricted.ROObject.1.Alias", &str_val);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    assert_string_equal(str_val, "cpe-ROObject-1");
    free(str_val);
    // the rows must be returned when fetching the row names
    assert_int_equal(check_rows_are_available(handle, "Restricted.ROObject.", expected_rows_1, __LINE__), 2);

    rc = rbus_getStr(handle, "Restricted.LimitedInstances.1.Alias", &str_val);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    assert_string_equal(str_val, "cpe-LimitedInstances-1");
    free(str_val);
    // the rows must be returned when fetching the row names
    assert_int_equal(check_rows_are_available(handle, "Restricted.LimitedInstances.", expected_rows_2, __LINE__), 2);

    // When adding a row to a read-only table
    rc = rbusTable_addRow(handle, "Restricted.ROObject.", NULL, &index);

    // the return code must not be RBUS_ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
    // and the index must still be 0
    assert_int_equal(index, 0);
    // and the same rows are still available
    assert_int_equal(check_rows_are_available(handle, "Restricted.ROObject.", expected_rows_1, __LINE__), 2);

    // When adding a row to an existing table but it fails (limited number of instances)
    rc = rbusTable_addRow(handle, "Restricted.LimitedInstances.", NULL, &index);

    // the return code must not be RBUS_ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
    // and the index must still be 0
    assert_int_equal(index, 0);
    // and the same rows are still available
    assert_int_equal(check_rows_are_available(handle, "Restricted.LimitedInstances.", expected_rows_2, __LINE__), 2);
}

void test_native_rbus_gets_error_when_deleting_row_fails(UNUSED void** state) {
    int rc = RBUS_ERROR_SUCCESS;
    char* str_val = NULL;
    const char* expected_rows_1[] = { "Restricted.ROObject.1.", "Restricted.ROObject.2." };
    const char* expected_rows_2[] = { "Restricted.LimitedInstances.1.", "Restricted.LimitedInstances.2."};

    // Check if the tables exists and contains rows:
    rc = rbus_getStr(handle, "Restricted.ROObject.1.Alias", &str_val);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    assert_string_equal(str_val, "cpe-ROObject-1");
    free(str_val);
    // the rows must be returned when fetching the row names
    assert_int_equal(check_rows_are_available(handle, "Restricted.ROObject.", expected_rows_1, __LINE__), 2);

    rc = rbus_getStr(handle, "Restricted.LimitedInstances.1.Alias", &str_val);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    assert_string_equal(str_val, "cpe-LimitedInstances-1");
    free(str_val);
    // the rows must be returned when fetching the row names
    assert_int_equal(check_rows_are_available(handle, "Restricted.LimitedInstances.", expected_rows_2, __LINE__), 2);

    // When deleting a row from a read-only table
    rc = rbusTable_removeRow(handle, "Restricted.ROObject.1.");

    // the return code must not be RBUS_ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
    // and the same rows are still available
    assert_int_equal(check_rows_are_available(handle, "Restricted.ROObject.", expected_rows_1, __LINE__), 2);
    // And the rows still exist:
    rc = rbus_getStr(handle, "Restricted.ROObject.1.Alias", &str_val);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    assert_string_equal(str_val, "cpe-ROObject-1");
    free(str_val);

    // When deleting a row from an existing table but it fails
    rc = rbusTable_removeRow(handle, "Restricted.LimitedInstances.1.");

    // the return code must not be RBUS_ERROR_SUCCESS
    assert_int_not_equal(rc, RBUS_ERROR_SUCCESS);
    // and the same rows are still available
    assert_int_equal(check_rows_are_available(handle, "Restricted.LimitedInstances.", expected_rows_2, __LINE__), 2);
    // And the rows still exist:
    rc = rbus_getStr(handle, "Restricted.LimitedInstances.1.Alias", &str_val);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    assert_string_equal(str_val, "cpe-LimitedInstances-1");
    free(str_val);
}