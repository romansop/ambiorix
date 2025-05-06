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

#if !defined(__AMXUT_VERIFY_H__)
#define __AMXUT_VERIFY_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h> // needed for cmocka
#include <setjmp.h> // needed for cmocka
#include <unistd.h> // needed for cmocka
#include <cmocka.h>
#include <amxc/amxc_variant.h>

/**
 * Load a variant from a JSON file and compare it with the provided variant.
 *
 * @param to_check the variant to check
 * @param data_file the name of the file containing the JSON encoded variant data.
 *
 * Returns 0 in case the variants are equal, any other value indicates an error.
 */
int amxut_verify_variant_from_json_file(amxc_var_t* to_check, const char* data_file);

/**
 * Compares 2 variants to see if they are equal.
 *
 * This function can be used in combination with the check_expected function from cmocka. You would
 * need to call it like:
 *
 * expect_check(function_where_check_happens, actual_variant, amxut_verify_variant_equal_check, expected_variant);
 *
 * where the input arguments are:
 * - The function where the check needs to happen
 * - The name of the real variant that will be provided as an argument to the function
 * - The name of this check function
 * - The expected variant which needs to be provided in advance by the tester
 * All of the argument names depend on your situation except for the amxut_verify_variant_equal_check
 * function.
 *
 * Make sure you also call check_expected(actual_variant) in the function_where_check_happens to
 * trigger the check.
 *
 * Returns 1 in case the variants are equal (needed for check_expected) and will stop the test
 * in case they are not equal.
 */
int amxut_verify_variant_equal_check(const LargestIntegralType value, const LargestIntegralType check_value_data);

#ifdef __cplusplus
}
#endif

#endif
