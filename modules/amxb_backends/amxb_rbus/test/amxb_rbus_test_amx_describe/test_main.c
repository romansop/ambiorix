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

#include "test_amxb_rbus_amxb_describe.h"

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_amxb_rbus_describe_instance_full_using_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_instance_full_not_using_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_instance_full),
        cmocka_unit_test(test_amxb_rbus_describe_instance_only_using_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_instance_only_not_using_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_instance_only),

        cmocka_unit_test(test_amxb_rbus_describe_table_full_using_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_table_full_without_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_table_full),
        cmocka_unit_test(test_amxb_rbus_describe_table_only_using_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_table_only_without_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_table_only),

        cmocka_unit_test(test_amxb_rbus_describe_singelton_full_using_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_singelton_full_without_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_singleton_full),
        cmocka_unit_test(test_amxb_rbus_describe_singelton_object_only_with_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_singelton_object_only_without_amx_calls),
        cmocka_unit_test(test_amxb_rbus_describe_object_only),

        cmocka_unit_test(test_amxb_rbus_describe_on_none_existing_returns_empty_table),
    };
    return cmocka_run_group_tests(tests, test_amx_setup, test_amx_teardown);
}
