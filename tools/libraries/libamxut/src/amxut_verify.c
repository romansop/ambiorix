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

#include <stdarg.h> // needed for cmocka
#include <setjmp.h> // needed for cmocka
#include <unistd.h> // needed for cmocka
#include <cmocka.h>

#include <amxc/amxc.h>
#include <yajl/yajl_gen.h>
#include <amxj/amxj_variant.h>

#include "amxut/amxut_verify.h"
#include "amxut/amxut_util.h"

static int amxut_verify_variant_common(amxc_var_t* expected_variant, amxc_var_t* actual_variant) {
    int retval = 0;

    amxc_var_compare(expected_variant, actual_variant, &retval);
    if(retval != 0) {
        printf("***** Unexpected data recieved *****\n");
        printf("EXPECTED:\n");
        fflush(stdout);
        amxc_var_dump(expected_variant, STDOUT_FILENO);
        printf("GOT:\n");
        fflush(stdout);
        amxc_var_dump(actual_variant, STDOUT_FILENO);
        amxc_var_cast(actual_variant, AMXC_VAR_ID_JSON);
        printf("JSON:\n");
        fflush(stdout);
        amxc_var_dump(actual_variant, STDOUT_FILENO);
        printf("************************************\n");
    }

    return retval;
}

int amxut_verify_variant_from_json_file(amxc_var_t* to_check, const char* data_file) {
    int retval = 0;
    amxc_var_t* verify = NULL;

    verify = amxut_util_read_json_from_file(data_file);
    retval = amxut_verify_variant_common(verify, to_check);

    amxc_var_delete(&verify);
    return retval;
}

int amxut_verify_variant_equal_check(const LargestIntegralType value, const LargestIntegralType check_value_data) {
    int retval = -1;
    amxc_var_t* actual_variant = (amxc_var_t*) value;
    amxc_var_t* expected_variant = (amxc_var_t*) check_value_data;

    retval = amxut_verify_variant_common(expected_variant, actual_variant);

    amxc_var_delete(&expected_variant);

    // Return 1 in case of success
    return (retval == 0 ? 1 : 0);
}