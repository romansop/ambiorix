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

#if !defined(__AMXC_MACROS_H__)
#define __AMXC_MACROS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(USE_DOXYGEN)

#ifndef PRIVATE
#define PRIVATE __attribute__ ((visibility("hidden")))
#endif

#ifndef UNUSED
#define UNUSED __attribute__((unused))
#endif

#ifndef WARN_UNUSED_RETURN
#define WARN_UNUSED_RETURN __attribute__ ((warn_unused_result))
#endif

#ifndef CONSTRUCTOR_LVL
#define CONSTRUCTOR_LVL(x) __attribute__((constructor(x)))
#endif

#ifndef DESTRUCTOR_LVL
#define DESTRUCTOR_LVL(x) __attribute__((destructor(x)))
#endif

#ifndef CONSTRUCTOR
#define CONSTRUCTOR __attribute__((constructor))
#endif

#ifndef DESTRUCTOR
#define DESTRUCTOR __attribute__((destructor))
#endif

#else // !defined(USE_DOXYGEN)

#ifndef PRIVATE
#define PRIVATE
#endif

#ifndef UNUSED
#define UNUSED
#endif

#ifndef WARN_UNUSED_RETURN
#define WARN_UNUSED_RETURN
#endif

#ifndef CONSTRUCTOR_LVL
#define CONSTRUCTOR_LVL(x)
#endif

#ifndef DESTRUCTOR_LVL
#define DESTRUCTOR_LVL(x)
#endif

#ifndef CONSTRUCTOR
#define CONSTRUCTOR
#endif

#ifndef DESTRUCTOR
#define DESTRUCTOR
#endif

#endif // !defined(USE_DOXYGEN)

#ifndef when_null
#define when_null(x, l) if((x) == NULL) {  goto l; }
#endif

#ifndef when_not_null
#define when_not_null(x, l) if((x) != NULL) {  goto l; }
#endif

#ifndef when_true
#define when_true(x, l) if((x)) {  goto l; }
#endif

#ifndef when_false
#define when_false(x, l) if(!(x)) {  goto l; }
#endif

#ifndef when_failed
#define when_failed(x, l) if((x) != 0) {  goto l; }
#endif

#ifndef when_str_empty
#define when_str_empty(x, l) if((x) == NULL || *(x) == 0) {  goto l; }
#endif

#ifndef when_null_status
#define when_null_status(x, l, c) if((x) == NULL) { c; goto l; }
#endif

#ifndef when_not_null_status
#define when_not_null_status(x, l, c) if((x) != NULL) { c; goto l; }
#endif

#ifndef when_true_status
#define when_true_status(x, l, c) if((x)) { c; goto l; }
#endif

#ifndef when_false_status
#define when_false_status(x, l, c) if(!(x)) { c; goto l; }
#endif

#ifndef when_failed_status
#define when_failed_status(x, l, c) if((x) != 0) { c; goto l; }
#endif


#ifndef when_str_empty_status
#define when_str_empty_status(x, l, c) if((x) == NULL || (x)[0] == '\0') { c; goto l; }
#endif

#ifdef __cplusplus
}
#endif

#endif // __AMXC_MACROS_H__
