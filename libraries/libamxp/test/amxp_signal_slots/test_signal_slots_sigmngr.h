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

#ifndef __TEST_SIGNAL_SLOTS_SIGMNGR_H__
#define __TEST_SIGNAL_SLOTS_SIGMNGR_H__

int test_delete_sigmngr_teardown(void** state);

void test_sigmngr_new_delete(void** state);
void test_sigmngr_delete_pending_sigs(void** state);
void test_sigmngr_trigger(void** state);
void test_sigmngr_emit(void** state);
void test_sigmngr_suspend_resume(void** state);
void test_sigmngr_suspend_resume_without_signals_queued(void** state);
void test_sigmngr_connect_all(void** state);
void test_sigmngr_connect_all2(void** state);
void test_sigmngr_disconnect_all_1(void** state);
void test_sigmngr_disconnect_all_2(void** state);
void test_sigmngr_add_remove_signal(void** state);
void test_sigmngr_crash_connect_not_existing_signal(void** state);
void test_sigmngr_find_signal(void** state);
void test_signal_has_slots(void** state);
void test_slot_disconnect_priv(void** state);
void test_does_not_crash_with_rogue_signal(void** state);
void test_sigmngr_disable(void** state);
void test_sigmngr_trigger_unknown_signal_should_not_segfault(void** state);
void test_removing_signal_in_slot(void** state);
void test_delete_sigmngr_in_slot(void** state);
void test_deferred_call(void** state);
void test_remove_deferred_call(void** state);
void test_deferred_call_delete_sigmngr(void** state);
void test_can_disconnect_in_slot(void** state);
void test_can_delete_signal_in_slot(void** state);
void test_can_delete_signal_in_multiple_slots(void** state);
void test_can_trigger_non_registered_signal(void** state);
void test_delete_sigmngr_in_regexp_slot(void** state);
void test_delete_sigmngr_in_slot_and_trigger_slot(void** state);

#endif // __TEST_SIGNAL_SLOTS_SIGMNGR_H__