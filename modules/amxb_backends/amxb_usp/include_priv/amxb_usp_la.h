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

#if !defined(__AMXB_USP_LA_H__)
#define __AMXB_USP_LA_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "amxb_usp.h"

// amxb_usp_la_subs.c
PRIVATE
void amxb_usp_la_subs_notification(const char* const sig_name,
                                   const amxc_var_t* const data,
                                   void* const priv);

PRIVATE
amxd_status_t amxb_usp_la_subs_add(amxb_usp_deferred_t* fcall,
                                   amxc_var_t* parameters);

PRIVATE
amxd_status_t amxb_usp_la_subs_del(amxb_usp_deferred_t* fcall,
                                   const char* requested_path);

PRIVATE
void amxb_usp_la_subs_remove(amxb_usp_t* ctx);

PRIVATE
void amxb_usp_la_subs_it_free(amxc_llist_it_t* lit);

// LOCAL AGENT DATAMODEL

// amxb_usp_la_contr.c
// Add or Delete controller instance
// - done when connection accepted or disconnected
// - when disconnected all subscriptions for the controller are removed
PRIVATE
amxd_status_t amxb_usp_la_contr_add(amxb_usp_t* ctx, amxc_string_t* ctrl_path);

PRIVATE
amxd_status_t amxb_usp_la_contr_del(amxb_usp_t* ctx);


// amxb_usp_la_subs.c

// Check that the subscription instance can be created and create it
// Sets the "Recipient" parameter to the path of the requesting controller
PRIVATE
amxd_status_t amxb_usp_la_subs_add_inst(amxd_object_t* const object,
                                        amxd_param_t* const p,
                                        amxd_action_t reason,
                                        const amxc_var_t* const args,
                                        amxc_var_t* const retval,
                                        void* priv);

PRIVATE
amxd_status_t amxb_usp_la_subs_remove_matching(amxd_object_t* const object,
                                               amxd_param_t* const p,
                                               amxd_action_t reason,
                                               const amxc_var_t* const args,
                                               amxc_var_t* const retval,
                                               void* priv);
PRIVATE
amxd_status_t amxb_usp_la_subs_delete_inst(amxd_object_t* const object,
                                           amxd_param_t* const p,
                                           amxd_action_t reason,
                                           const amxc_var_t* const args,
                                           amxc_var_t* const retval,
                                           void* priv);

PRIVATE
void amxb_usp_la_subscription_added(const char* const sig_name,
                                    const amxc_var_t* const data,
                                    void* const priv);

// amxb_usp_la_request.c

PRIVATE
uint32_t amxb_usp_la_add_request(const char* originator,
                                 const char* path,
                                 const char* func,
                                 const char* key);

PRIVATE
amxd_status_t amxb_usp_la_set_request_status(uint32_t index,
                                             const char* status);

PRIVATE
void amxb_usp_la_remove_request(const char* const sig_name,
                                const amxc_var_t* const data,
                                void* const priv);

PRIVATE
void amxb_usp_la_emit_complete(amxd_object_t* object,
                               uint32_t request_index,
                               amxc_var_t* ret,
                               amxc_var_t* out,
                               amxd_status_t status);

// amxb_usp_threshold_actions.c

// Check that the threshold instance can be created
PRIVATE
amxd_status_t amxb_usp_la_threshold_instance_is_valid(amxd_object_t* object,
                                                      amxd_param_t* param,
                                                      amxd_action_t reason,
                                                      const amxc_var_t* const args,
                                                      amxc_var_t* const retval,
                                                      void* priv);

// Triggered whenever the threshold instance is being destroyed
PRIVATE
amxd_status_t amxb_usp_la_threshold_instance_cleanup(amxd_object_t* object,
                                                     amxd_param_t* param,
                                                     amxd_action_t reason,
                                                     const amxc_var_t* const args,
                                                     amxc_var_t* const retval,
                                                     void* priv);

// amxb_usp_threshold_events.c

// Add the threshold instance and set the object priv data
PRIVATE
amxd_status_t amxb_usp_la_threshold_create(amxd_object_t* const object,
                                           amxd_param_t* const p,
                                           amxd_action_t reason,
                                           const amxc_var_t* const args,
                                           amxc_var_t* const retval,
                                           void* priv);

// Complete the creation of the threshold instance and add the needed subscriptions
PRIVATE
void amxb_usp_la_threshold_added(const char* const sig_name,
                                 const amxc_var_t* const data,
                                 void* const priv);

// Triggered whenever the threshold instance is being changed
PRIVATE
void amxb_usp_la_threshold_changed(const char* const sig_name,
                                   const amxc_var_t* const data,
                                   void* const priv);

PRIVATE
void threshold_unsubscribe(amxd_object_t* object,
                           amxb_usp_t* ctx);

#ifdef __cplusplus
}
#endif

#endif // __AMXB_USP_LA_H__
