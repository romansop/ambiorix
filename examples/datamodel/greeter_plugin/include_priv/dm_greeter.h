/****************************************************************************
**
** Copyright (c) 2020 SoftAtHome
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
#if !defined(__DM_GREETER_H__)
#define __DM_GREETER_H__

#ifdef __cplusplus
extern "C"
{
#endif

amxd_status_t _function_dump(amxd_object_t* object,
                             amxd_function_t* func,
                             amxc_var_t* args,
                             amxc_var_t* ret);

amxd_status_t _deferred_echo(amxd_object_t* object,
                             amxd_function_t* func,
                             amxc_var_t* args,
                             amxc_var_t* ret);

amxd_status_t _Greeter_say(amxd_object_t* object,
                           amxd_function_t* func,
                           amxc_var_t* args,
                           amxc_var_t* ret);

amxd_status_t _History_clear(amxd_object_t* object,
                             amxd_function_t* func,
                             amxc_var_t* args,
                             amxc_var_t* ret);

amxd_status_t _Greeter_setMaxHistory(amxd_object_t* object,
                                     amxd_function_t* func,
                                     amxc_var_t* args,
                                     amxc_var_t* ret);

amxd_status_t _Greeter_save(amxd_object_t* greeter,
                            amxd_function_t* func,
                            amxc_var_t* args,
                            amxc_var_t* ret);

amxd_status_t _Greeter_load(amxd_object_t* greeter,
                            amxd_function_t* func,
                            amxc_var_t* args,
                            amxc_var_t* ret);

amxd_status_t _State_check_change(amxd_object_t* object,
                                  amxd_param_t* param,
                                  amxd_action_t reason,
                                  const amxc_var_t* const args,
                                  amxc_var_t* const retval,
                                  void* priv);

amxd_status_t _stats_read(amxd_object_t* object,
                          amxd_param_t* param,
                          amxd_action_t reason,
                          const amxc_var_t* const args,
                          amxc_var_t* const retval,
                          void* priv);

amxd_status_t _stats_list(amxd_object_t* object,
                          amxd_param_t* param,
                          amxd_action_t reason,
                          const amxc_var_t* const args,
                          amxc_var_t* const retval,
                          void* priv);

amxd_status_t _stats_describe(amxd_object_t* object,
                              amxd_param_t* param,
                              amxd_action_t reason,
                              const amxc_var_t* const args,
                              amxc_var_t* const retval,
                              void* priv);

amxd_status_t _read_utc_time(amxd_object_t* object,
                             amxd_param_t* param,
                             amxd_action_t reason,
                             const amxc_var_t* const args,
                             amxc_var_t* const retval,
                             void* priv);

amxd_status_t _read_local_time(amxd_object_t* object,
                               amxd_param_t* param,
                               amxd_action_t reason,
                               const amxc_var_t* const args,
                               amxc_var_t* const retval,
                               void* priv);

amxd_status_t _periodic_inform(amxd_object_t* object,
                               amxd_function_t* func,
                               amxc_var_t* args,
                               amxc_var_t* ret);

amxd_status_t _send_event(amxd_object_t* object,
                          amxd_function_t* func,
                          amxc_var_t* args,
                          amxc_var_t* ret);

amxd_status_t _Statistics_reset(amxd_object_t* object,
                                amxd_function_t* func,
                                amxc_var_t* args,
                                amxc_var_t* ret);

void _print_event(const char* const sig_name,
                  const amxc_var_t* const data,
                  void* const priv);

void _enable_greeter(const char* const sig_name,
                     const amxc_var_t* const data,
                     void* const priv);

void _disable_greeter(const char* const sig_name,
                      const amxc_var_t* const data,
                      void* const priv);


#ifdef __cplusplus
}
#endif

#endif // __DM_GREETER_H__