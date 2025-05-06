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

#ifndef __TEST_EXPRESSION_H__
#define __TEST_EXPRESSION_H__

void test_can_create_expression(void** state);
void test_can_evaluate_expression(void** state);
void test_precedence_is_respected(void** state);
void test_comperators_are_correct(void** state);
void test_invalid_syntax(void** state);
void test_invalid_value_types(void** state);
void test_can_fetch_fields(void** state);
void test_invalid_field_names(void** state);
void test_can_use_custom_field_fetcher(void** state);
void test_api_arguments_validation(void** state);
void test_selects_first_in_first_existing(void** state);
void test_fails_with_invalid_regexp(void** state);
void test_is_empty_function(void** state);
void test_is_empty_function_with_var(void** state);
void test_contains(void** state);
void test_contains_no_get_field(void** state);
void test_contains_no_data(void** state);
void test_contains_invalid_usage(void** state);
void test_flag_expressions(void** state);
void test_flag_expressions_no_operators(void** state);
void test_in_operators(void** state);
void test_equals_ignorecase(void** state);
void test_buildf_expression(void** state);
void test_buildf_expression_allocate(void** state);
void test_buildf_expression_invalid_value(void** state);
void test_buildf_expression_all_whitelisted(void** state);
void test_buildf_expression_invalid_args(void** state);
void test_buildf_are_strings_safe(void** state);
void test_get_string(void** state);
void test_get_string_null(void** state);
void test_expression_with_invalid_list_does_not_memory_leak(void** state);

#endif // __TEST_EXPRESSION_H__
