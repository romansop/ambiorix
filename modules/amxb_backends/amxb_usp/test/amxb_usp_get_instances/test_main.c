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

#include "test_amxb_usp_get_instances.h"
#include "test_amxb_usp_common.h"

int main(void) {
    int rv = 0;
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_amxb_usp_read_get_instances),
        cmocka_unit_test(test_amxb_usp_handle_get_instances_error),
        cmocka_unit_test(test_amxb_usp_handle_get_instances_invalid),
        cmocka_unit_test(test_amxb_usp_get_instances_request),
    };
    const struct CMUnitTest tests_e2e_transl[] = {
        cmocka_unit_test(test_e2e_transl_gi),
    };
    const struct CMUnitTest tests_obuspa[] = {
        cmocka_unit_test(test_obuspa_get_instances),
        cmocka_unit_test(test_obuspa_get_instances_depth_0),
        cmocka_unit_test(test_obuspa_get_instances_search_path),
        cmocka_unit_test(test_obuspa_get_instances_singleton),
        cmocka_unit_test(test_obuspa_get_instances_invalid),
        cmocka_unit_test(test_obuspa_get_instances_invalid_sub_object),
        cmocka_unit_test(test_obuspa_get_instances_invalid_parameter_path),
    };

    rv = cmocka_run_group_tests(tests, test_dm_setup, test_dm_teardown);
    if(rv == 0) {
        rv = cmocka_run_group_tests(tests_e2e_transl, test_e2e_transl_setup, test_e2e_transl_teardown);
    }

    // Run similar tests but now with obuspa
    if(rv == 0) {
        rv = cmocka_run_group_tests(tests_obuspa, test_common_obuspa_setup, test_common_obuspa_teardown);
    }

    return rv;
}
