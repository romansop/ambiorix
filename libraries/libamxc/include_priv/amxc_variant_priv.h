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
#if !defined(__AMXC_VARIANT_PRIV_H__)
#define __AMXC_VARIANT_PRIV_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <amxc/amxc_htable.h>
#include <amxc/amxc_llist.h>
#include <amxc/amxc_variant.h>
#include <amxc/amxc_variant_type.h>
#include <amxc/amxc_macros.h>

#define OVERFLOW_SIGNED(a, b, min, max) ((a) >= 0 ? (b) > (max) - (a) : (b) < (min) - (a))
#define OVERFLOW_UNSIGNED(a, b, max) ((b) > (max) - (a))

PRIVATE
uint32_t amxc_var_add_type(amxc_var_type_t* const type, const uint32_t index);

PRIVATE
int amxc_var_remove_type(amxc_var_type_t* const type);

PRIVATE
amxc_array_t* amxc_variant_get_types_array(void);

PRIVATE
int amxc_var_default_copy(amxc_var_t* const dest, const amxc_var_t* const src);

PRIVATE
int amxc_var_default_move(amxc_var_t* const dest, amxc_var_t* const src);

PRIVATE
int amxc_var_default_convert_to_null(amxc_var_t* const dest, const amxc_var_t* const src);

PRIVATE
int amxc_var_default_convert_to_list(amxc_var_t* const dest, const amxc_var_t* const src);

PRIVATE
int amxc_var_default_convert_to_htable(amxc_var_t* const dest, const amxc_var_t* const src);

#ifdef __cplusplus
}
#endif

#endif // __AMXC_VARIANT_PRIV_H__
