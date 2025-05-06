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

#if !defined(__AMXB_USP_H__)
#define __AMXB_USP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <imtp/imtp_connection.h>

#include <amxc/amxc.h>
#include <amxc/amxc_macros.h>
#include <amxp/amxp.h>
#include <amxp/amxp_dir.h>

#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>
#include <amxd/amxd_function.h>
#include <amxd/amxd_object_function.h>
#include <amxd/amxd_object_event.h>
#include <amxd/amxd_path.h>

#include <amxo/amxo.h>

#include <amxb/amxb.h>
#include <amxb/amxb_be_intf.h>

#include <usp/uspl.h>
#include <uspi/uspi_subscription.h>

#define AMXB_USP_TRANSLATE_NONE             0
#define AMXB_USP_TRANSLATE_PATHS            1
#define AMXB_USP_TRANSLATE_MULTIPLE         2
#define AMXB_USP_TRANSLATE_LIST             3
#define AMXB_USP_TRANSLATE_DATA             4

#define IS_SET(b, f) ((b & f) == f)

#define AMXB_USP_COPT_REQUIRES_DEVICE "requires-device-prefix"
#ifdef CONFIG_SAH_MOD_AMXB_USP_REQUIRES_DEVICE_PREFIX
#define AMXB_USP_CVAL_REQUIRES_DEVICE true
#else
#define AMXB_USP_CVAL_REQUIRES_DEVICE false
#endif

typedef struct _amxb_usp_stats {
    uint64_t counter_rx_invoke;
    uint64_t counter_rx_get;
    uint64_t counter_rx_set;
    uint64_t counter_rx_add;
    uint64_t counter_rx_del;
    uint64_t counter_rx_notify;
    uint64_t counter_rx_get_instances;
    uint64_t counter_rx_get_supported;
} amxb_usp_stats_t;

typedef struct _amxb_usp {
    imtp_connection_t* icon;
    amxc_htable_t server_subs;
    amxc_htable_t subscriptions;    // original subscriptions
    amxc_htable_t operate_requests; // hash table to track active operate requests
    amxc_llist_t registrations;
    amxp_signal_mngr_t* sigmngr;
    amxd_dm_t* dm;
    amxd_dm_t* la_dm;
    char* eid;                      // EndpointID of service on the other side
    char* contr_path;               // LocalAgent.Controller.{i}. path
    amxc_htable_it_t hit;           // Iterator to track context in hash table
    amxp_timer_t* register_retry;   // Timer for register retry in case it fails
    amxb_usp_stats_t stats;
} amxb_usp_t;

typedef struct _usp_subscription {
    amxb_usp_t* ctx;
    amxc_htable_it_t hit;
} usp_subscription_t;

typedef struct _usp_request {
    amxb_request_t* request;
    amxc_htable_it_t hit;
} usp_request_t;

typedef struct _amxb_usp_deferred {
    amxb_usp_t* ctx;                // context to send response on
    amxc_var_t data;                // to store from_id, to_id, msg_id, msg_type
    int num_calls;                  // a single request could result in several deferred calls
    amxc_llist_t resp_list;         // to build the response
    amxc_llist_t child_list;        // list of deferred children
} amxb_usp_deferred_t;

typedef struct _amxb_usp_deferred_child {
    amxc_llist_it_t it;             // to track struct in parent
    uint64_t call_id;               // to track current deferred call and cancel it
    uint32_t request_index;         // the request index in the local agent dm
    char* requested_path;           // needed for response
    amxc_var_t* args;               // function arguments, needed for operate resp
    amxb_usp_deferred_t* parent;    // pointer to parent
    char* cmd_key;                  // command_key in case of an operate
} amxb_usp_deferred_child_t;

typedef int (* fn_request_new)(uspl_tx_t* usp_tx,
                               const amxc_var_t* request);

typedef int (* fn_resp_new)(uspl_tx_t* usp_tx,
                            amxc_llist_t* resp_list,
                            const char* msg_id);

typedef int (* fn_resp_extract)(uspl_rx_t* usp_rx,
                                amxc_llist_t* resp_list);

amxb_be_info_t* amxb_be_info(void);

PRIVATE
int amxb_usp_set_config(amxc_var_t* const configuration);

PRIVATE
amxc_var_t* amxb_usp_get_config_option(const char* name);

PRIVATE
amxc_var_t* amxb_usp_get_config(void);

PRIVATE
void* amxb_usp_connect(const char* host,
                       const char* port,
                       const char* path,
                       amxp_signal_mngr_t* sigmngr);

PRIVATE
void* amxb_usp_listen(const char* host,
                      const char* port,
                      const char* path,
                      amxp_signal_mngr_t* sigmngr);

PRIVATE
void* amxb_usp_accept(void* const ctx,
                      amxp_signal_mngr_t* sigmngr);

PRIVATE
int amxb_usp_disconnect(void* ctx);

PRIVATE
void amxb_usp_free(void* ctx);

PRIVATE
int amxb_usp_get_fd(void* ctx);

PRIVATE
int amxb_usp_read(void* ctx);

PRIVATE
int amxb_usp_handle_read(amxb_usp_t* ctx);

PRIVATE
int amxb_usp_register(void* const ctx,
                      amxd_dm_t* const dm);

PRIVATE
int amxb_usp_handle_register(amxb_usp_t* ctx, uspl_rx_t* usp_data);

PRIVATE
void amxb_usp_send_notification(const char* const sig_name,
                                const amxc_var_t* const data,
                                void* const priv);

PRIVATE
int amxb_usp_read_raw(void* ctx, void* buf, size_t count);

PRIVATE
void amxb_usp_ctx_insert(amxb_usp_t* ctx, const char* contr_path);

PRIVATE
void amxb_usp_ctx_remove(amxb_usp_t* ctx);

PRIVATE
amxb_usp_t* amxb_usp_ctx_get(const char* contr_path);

PRIVATE
bool amxb_usp_has(void* const ctx,
                  const char* object);

// amxb_usp_common.c

PRIVATE
int amxb_usp_handle_protobuf(amxb_usp_t* ctx, uspl_rx_t* usp_data);

PRIVATE
amxc_string_t* amxb_usp_add_dot(const char* str_in);

/* Extract msg_id, from_id and to_id from received USP data */
PRIVATE
int amxb_usp_metadata_extract(uspl_rx_t* usp_data,
                              char** msg_id,
                              char** from_id,
                              char** to_id);

PRIVATE
int amxb_usp_build_and_send_tlv(amxb_usp_t* ctx, uspl_tx_t* usp);

PRIVATE
int amxb_usp_reply(amxb_usp_t* ctx,
                   uspl_rx_t* usp_data,
                   amxc_llist_t* resp_list,
                   fn_resp_new fn);

PRIVATE
int amxb_usp_reply_deferred(amxb_usp_deferred_t* fcall,
                            fn_resp_new fn);

PRIVATE
int amxb_usp_reply_error(amxb_usp_t* ctx,
                         uspl_rx_t* usp_data,
                         int err_code);

PRIVATE
int amxb_usp_tx_new(amxb_usp_t* amxb_usp, uspl_tx_t** usp_tx);

PRIVATE
int amxb_usp_send_req(amxb_usp_t* amxb_usp,
                      amxc_var_t* request,
                      fn_request_new fn,
                      char** msg_id_tx);

PRIVATE
int amxb_usp_poll_response(amxb_usp_t* amxb_usp,
                           char* expected_msg_id,
                           fn_resp_extract fn,
                           amxc_var_t* ret,
                           bool return_as_list,
                           int timeout);

PRIVATE
char* amxb_usp_get_from_id(void);

PRIVATE
char* amxb_usp_get_to_id(amxb_usp_t* ctx);

PRIVATE
int amxb_usp_is_dot(int c);

PRIVATE
amxd_status_t amxb_usp_filter_ret(amxc_var_t* ret);

PRIVATE
amxd_dm_t* amxb_usp_get_la(void);

PRIVATE
void amxb_usp_request_free(const char* key, amxc_htable_it_t* it);

PRIVATE
int amxb_usp_deferred_child_new(amxb_usp_deferred_child_t** child,
                                amxb_usp_deferred_t* parent,
                                uint64_t call_id,
                                uint32_t request_index,
                                const char* requested_path,
                                amxc_var_t* args);

PRIVATE
void amxb_usp_deferred_child_delete(amxb_usp_deferred_child_t** child);

PRIVATE
int amxb_usp_deferred_child_set_cmd_key(amxb_usp_deferred_child_t* child, const char* cmd_key);

PRIVATE
int amxb_usp_deferred_new(amxb_usp_deferred_t** fcall,
                          amxb_usp_t* ctx,
                          uspl_rx_t* usp_rx,
                          int num_calls);

PRIVATE
void amxb_usp_deferred_delete(amxb_usp_deferred_t** fcall);


PRIVATE
void amxb_usp_rel_path_set(amxc_var_t* args, amxd_path_t* path, bool set_param);

PRIVATE
void amxb_usp_amx_call_done(const amxc_var_t* const data,
                            void* const priv);

PRIVATE
amxd_status_t amxb_usp_amx_call(amxb_usp_deferred_t* fcall,
                                const char* target_path,
                                const char* requested_path,
                                bool set_param,
                                const char* func,
                                amxc_var_t* args,
                                amxc_var_t* ret,
                                amxp_deferred_fn_t cb);

PRIVATE
bool amxb_usp_path_starts_with_device(const char* path);

PRIVATE
amxd_status_t amxb_usp_convert_error(amxc_var_t* ret, amxc_var_t* src);

// amxb_usp_get.c

PRIVATE
void amxb_usp_get_deferred_resp(const amxc_var_t* const data,
                                amxb_usp_deferred_t* fcall,
                                amxb_usp_deferred_child_t* child);

PRIVATE
int amxb_usp_handle_get(amxb_usp_t* ctx, uspl_rx_t* usp_data);

PRIVATE
int amxb_usp_get(void* const ctx,
                 const char* object,
                 const char* search_path,
                 int32_t depth,
                 uint32_t access,
                 amxc_var_t* values,
                 int timeout);

// amxb_usp_set.c

PRIVATE
void amxb_usp_set_deferred_resp(const amxc_var_t* const data,
                                amxb_usp_deferred_t* fcall,
                                amxb_usp_deferred_child_t* child);

PRIVATE
int amxb_usp_handle_set(amxb_usp_t* ctx, uspl_rx_t* usp_data);

PRIVATE
int amxb_usp_set(void* const ctx,
                 const char* object,
                 const char* search_path,
                 uint32_t flags,
                 amxc_var_t* values,
                 amxc_var_t* ovalues,
                 UNUSED uint32_t access,
                 UNUSED amxc_var_t* ret,
                 int timeout);

// amxb_usp_get_supported_dm.c

PRIVATE
void amxb_usp_get_supported_dm_deferred_resp(const amxc_var_t* const data,
                                             amxb_usp_deferred_t* fcall,
                                             amxb_usp_deferred_child_t* child);

PRIVATE
int amxb_usp_handle_get_supported_dm(amxb_usp_t* ctx, uspl_rx_t* usp_data);

PRIVATE
int amxb_usp_get_supported(void* const ctx,
                           const char* object,
                           const char* search_path,
                           uint32_t flags,
                           amxc_var_t* values,
                           int timeout);

// amxb_usp_add.c

PRIVATE
void amxb_usp_add_deferred_resp(const amxc_var_t* const data,
                                amxb_usp_deferred_t* fcall,
                                amxb_usp_deferred_child_t* child);

PRIVATE
int amxb_usp_handle_add(amxb_usp_t* ctx, uspl_rx_t* usp_data);

PRIVATE
int amxb_usp_add(void* const ctx,
                 const char* object,
                 const char* search_path,
                 UNUSED uint32_t index,
                 UNUSED const char* name,
                 amxc_var_t* values,
                 UNUSED uint32_t access,
                 amxc_var_t* ret,
                 int timeout);

// amxb_usp_delete.c

PRIVATE
void amxb_usp_delete_deferred_resp(const amxc_var_t* const data,
                                   amxb_usp_deferred_t* fcall,
                                   amxb_usp_deferred_child_t* child);

PRIVATE
int amxb_usp_handle_delete(amxb_usp_t* ctx, uspl_rx_t* usp_data);

PRIVATE
int amxb_usp_delete(void* const ctx,
                    const char* object,
                    const char* search_path,
                    UNUSED uint32_t index,
                    UNUSED const char* name,
                    UNUSED uint32_t access,
                    amxc_var_t* ret,
                    int timeout);

// amxb_usp_subscribe.c

PRIVATE
int amxb_usp_subscribe(void* const ctx,
                       const char* object);

PRIVATE
int amxb_usp_subscribe_v2(void* const ctx,
                          amxd_path_t* path,
                          int32_t depth,
                          uint32_t event_types);

PRIVATE
int amxb_usp_unsubscribe(void* const ctx,
                         const char* object);

// amxb_usp_notification.c

PRIVATE
int amxb_usp_handle_notify(amxb_usp_t* ctx, uspl_rx_t* usp_data);

// amxb_usp_invoke.c

PRIVATE
void amxb_usp_operate_deferred_resp(const amxc_var_t* const data,
                                    amxb_usp_deferred_t* fcall,
                                    amxb_usp_deferred_child_t* child);

PRIVATE
int amxb_usp_handle_operate(amxb_usp_t* ctx, uspl_rx_t* usp_data);

PRIVATE
int amxb_usp_invoke(void* const ctx,
                    amxb_invoke_t* invoke_ctx,
                    amxc_var_t* args,
                    amxb_request_t* request,
                    int timeout);

PRIVATE
int amxb_usp_handle_operate_resp(amxb_usp_t* ctx, uspl_rx_t* usp_data);

PRIVATE
int amxb_usp_async_invoke(void* const ctx,
                          amxb_invoke_t* invoke_ctx,
                          amxc_var_t* args,
                          amxb_request_t* request);

PRIVATE
int amxb_usp_wait_for_request(void* const ctx,
                              amxb_request_t* request,
                              int timeout);

PRIVATE
int amxb_usp_close_request(void* const ctx, amxb_request_t* request);

// amxb_usp_get_instances.c

PRIVATE
void amxb_usp_get_instances_deferred_resp(const amxc_var_t* const data,
                                          amxb_usp_deferred_t* fcall,
                                          amxb_usp_deferred_child_t* child);

PRIVATE
int amxb_usp_handle_get_instances(amxb_usp_t* ctx, uspl_rx_t* usp_data);

PRIVATE
int amxb_usp_get_instances(void* const ctx,
                           const char* object,
                           const char* search_path,
                           int32_t depth,
                           uint32_t access,
                           amxc_var_t* ret,
                           int timeout);

// amxb_usp_translate.c
PRIVATE
void amxb_usp_translate_set_paths(void);

PRIVATE
void amxb_usp_translate_path(amxd_path_t* path, const char** requested, const char** translated);

PRIVATE
void amxb_usp_translate_register_path(amxd_path_t* path);

PRIVATE
void amxb_usp_call_translate(amxc_var_t* out, int funcid);

PRIVATE
void amxb_usp_translate_data(const amxc_var_t* data);

PRIVATE
int amxb_usp_get_stats(void* const ctx,
                       amxc_var_t* stats);

PRIVATE
int amxb_usp_reset_stats(void* const ctx);

#ifdef __cplusplus
}
#endif

#endif // __AMXB_USP_H__
