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

#if !defined(__AMXT_HISTORY_H__)
#define __AMXT_HISTORY_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#ifndef __AMXC_VARIANT_H__
#error "include <amxc/amxc_variant.h> before <amxt/amxt_history.h>"
#endif

typedef struct _amxt_hist {
    amxc_var_t history;
    uint32_t current_pos;
    uint32_t max;
} amxt_hist_t;

int amxt_hist_init(amxt_hist_t* const hist);
int amxt_hist_clean(amxt_hist_t* const hist);

int amxt_hist_set_pos(amxt_hist_t* const hist, const uint32_t index);
int amxt_hist_get_pos(amxt_hist_t* const hist);

int amxt_hist_add(amxt_hist_t* const hist, const char* text);
int amxt_hist_update(amxt_hist_t* const hist, const char* text);

const char* amxt_hist_get_current(const amxt_hist_t* const hist);
const char* amxt_hist_get_next(amxt_hist_t* const hist);
const char* amxt_hist_get_prev(amxt_hist_t* const hist);

int amxt_hist_save(const amxt_hist_t* const hist, const char* path_name);
int amxt_hist_load(amxt_hist_t* const hist, const char* path_name);

#ifdef __cplusplus
}
#endif

#endif // __AMXT_HISTORY_H__
