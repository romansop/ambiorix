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

#include "amxb_pcb.h"

void amxb_pcb_delete_pending_fcall(amxc_llist_it_t* it) {
    amxb_pcb_deferred_t* fcall = amxc_container_of(it, amxb_pcb_deferred_t, it);
    amxb_pcb_log("Cancel fcall (%p ID = %" PRIu64 ") - Request = %p (%s:%d) ", (void*) fcall, fcall->call_id, (void*) fcall->req, __FILE__, __LINE__);
    amxd_function_set_deferred_cb(fcall->call_id, NULL, NULL);
    amxd_function_deferred_remove(fcall->call_id);
    amxc_llist_clean(&fcall->calls, amxb_pcb_delete_pending_fcall);
    amxb_pcb_log("Free fcall %p, request = %p (%s:%d)", (void*) fcall, (void*) fcall->req, __FILE__, __LINE__);
    free(fcall);
}


void amxb_pcb_fcall_new(amxb_pcb_deferred_t** fcall, amxb_pcb_t* amxb_pcb, request_t* req, amxc_llist_t* parent) {
    *fcall = (amxb_pcb_deferred_t*) calloc(1, sizeof(amxb_pcb_deferred_t));
    when_null(*fcall, exit);

    (*fcall)->req = req;
    (*fcall)->amxb_pcb = amxb_pcb;
    amxc_llist_append(parent, &(*fcall)->it);

exit:
    return;
}

void amxb_pcb_fcall_delete(amxb_pcb_deferred_t** fcall) {
    amxc_llist_it_take(&(*fcall)->it);
    amxc_llist_clean(&(*fcall)->calls, amxb_pcb_delete_pending_fcall);
    free(*fcall);
    *fcall = NULL;
}