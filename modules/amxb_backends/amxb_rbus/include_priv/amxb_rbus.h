/****************************************************************************
**
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
** SPDX-FileCopyrightText: Copyright (c) 2024 SoftAtHome
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
#if !defined(__AMXB_RBUS_H__)
#define __AMXB_RBUS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include <rbus.h>

#include <amxc/amxc_macros.h>
#include <amxc/amxc.h>
#include <amxp/amxp.h>

#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>
#include <amxd/amxd_object_event.h>
#include <amxd/amxd_function.h>
#include <amxd/amxd_path.h>
#include <amxd/amxd_transaction.h>

#include <amxb/amxb.h>
#include <amxb/amxb_be_intf.h>

#define AMXB_RBUS_TRANSLATE_NONE     0
#define AMXB_RBUS_TRANSLATE_PATHS    1
#define AMXB_RBUS_TRANSLATE_MULTIPLE 2
#define AMXB_RBUS_TRANSLATE_LIST     3
#define AMXB_RBUS_TRANSLATE_DATA     4

typedef struct _amxb_rbus amxb_rbus_t;
typedef struct _amxb_rbus_item amxb_rbus_item_t;

typedef void (* amxb_rbus_handler_t) (amxb_rbus_t* amxb_rbus_ctx,
                                      amxb_rbus_item_t* item);

struct _amxb_rbus {
    rbusHandle_t handle;
    int socket[2];                 // socket 0 - main loop - socket 1  rbus
    pthread_mutex_t mutex;         // protection for in and out lists
    amxc_llist_t in;               // items received from rbus
    amxc_llist_t out;              // items passed to rbus
    amxp_signal_mngr_t* sigmngr;
    amxd_dm_t* dm;
    amxc_htable_t subscribers;
    amxc_htable_t subscriptions;
    uint32_t subscription_count;
    amxc_llist_it_t it;
    amxc_var_t ignore;             // htable of object paths for which the events are ignored
                                   // IMPORTANT rbus doesn't like that add/del row is done
                                   //           when it was requested by a data model
                                   //           consumer, through a request.
    amxc_htable_t sessions;        // open sessions.
    pthread_t tid;                 // thread-id of main thread
    bool register_done;            // marks when registration is done
};

#define AMXB_RBUS_ITEM_OK     0
#define AMXB_RBUS_ITEM_CLOSED 1

struct _amxb_rbus_item {
    amxc_llist_it_t it;
    amxc_string_t name;
    rbusObject_t in_params;
    rbusObject_t out_params;
    rbusError_t status;
    pthread_t tid;
    amxb_rbus_handler_t handler;
    uint32_t state;
    amxc_string_t transaction_id;
    void* priv;                 // should only be changed in main thread
};

typedef struct _rbus_condition_wait {
    pthread_cond_t condition;
    pthread_mutex_t lock;
    rbusValue_t value;
    rbusProperty_t property;
    rbusError_t status;
    const char* alias;
    uint32_t index;
    pthread_t tid;              // thread-id of orginator thread
} rbus_condition_wait_t;

typedef struct _rbus_session {
    amxd_trans_t transaction;
    amxc_htable_it_t hit;
} rbus_session_t;

amxb_be_info_t* amxb_be_info(void);

PRIVATE
void* amxb_rbus_connect(const char* host,
                        const char* port,
                        const char* path,
                        amxp_signal_mngr_t* sigmngr);

PRIVATE
int amxb_rbus_disconnect(void* ctx);

PRIVATE
int amxb_rbus_get_fd(void* ctx);

PRIVATE
int amxb_rbus_read(void* ctx);

PRIVATE
void amxb_rbus_free(void* ctx);

PRIVATE
int amxb_rbus_async_invoke(void* const ctx,
                           amxb_invoke_t* invoke_ctx,
                           amxc_var_t* args,
                           amxb_request_t* request);

PRIVATE
int amxb_rbus_invoke(void* const ctx,
                     amxb_invoke_t* invoke_ctx,
                     amxc_var_t* args,
                     amxb_request_t* request,
                     int timeout);

PRIVATE
int amxb_rbus_close_request(void* const ctx,
                            amxb_request_t* request);

PRIVATE
int amxb_rbus_wait_request(void* const ctx, amxb_request_t* request, int timeout);

PRIVATE
int amxb_rbus_register(void* const ctx, amxd_dm_t* const dm);

PRIVATE
void amxb_rbus_unregister(amxb_rbus_t* amxb_rbus_ctx);

PRIVATE
amxc_var_t* amxb_rbus_get_config_option(const char* name);

PRIVATE
int amxb_rbus_subscribe(void* const ctx, const char* object);

PRIVATE
int amxb_rbus_unsubscribe(void* const ctx, const char* object);

PRIVATE
int amxb_rbus_get(void* const ctx,
                  const char* object,
                  const char* search_path,
                  int32_t depth,
                  uint32_t access,
                  amxc_var_t* ret,
                  int timeout);

PRIVATE
int amxb_rbus_get_filtered(void* const ctx,
                           const char* object,
                           const char* search_path,
                           const char* filter,
                           int32_t depth,
                           uint32_t access,
                           amxc_var_t* ret,
                           int timeout);

PRIVATE
int amxb_rbus_set(void* const ctx,
                  const char* object,
                  const char* search_path,
                  uint32_t flags,
                  amxc_var_t* values,
                  amxc_var_t* ovalues,
                  uint32_t access,
                  amxc_var_t* ret,
                  int timeout);

PRIVATE
int amxb_rbus_add(void* const ctx,
                  const char* object,
                  const char* search_path,
                  uint32_t index,
                  const char* name,
                  amxc_var_t* values,
                  uint32_t access,
                  amxc_var_t* ret,
                  int timeout);

PRIVATE
int amxb_rbus_del(void* const ctx,
                  const char* object,
                  const char* search_path,
                  uint32_t index,
                  const char* name,
                  uint32_t access,
                  amxc_var_t* ret,
                  UNUSED int timeout);

PRIVATE
int amxb_rbus_describe(void* const ctx,
                       const char* object,
                       const char* search_path,
                       uint32_t flags,
                       uint32_t access,
                       amxc_var_t* ret,
                       UNUSED int timeout);

PRIVATE
int amxb_rbus_gsdm(void* const ctx,
                   const char* object,
                   UNUSED const char* search_path,
                   UNUSED uint32_t flags,
                   UNUSED amxc_var_t* values,
                   int timeout);
PRIVATE
uint32_t amxb_rbus_capabilities(void* const ctx);

PRIVATE
bool amxb_rbus_has(void* const ctx, const char* object);

PRIVATE
int amxb_rbus_wait_for(void* const ctx, const char* object);

PRIVATE
int amxb_rbus_list(void* const ctx,
                   const char* object,
                   uint32_t flags,
                   uint32_t access,
                   amxb_request_t* request);

PRIVATE
int amxb_rbus_get_instances(void* const ctx,
                            const char* object,
                            const char* search_path,
                            int32_t depth,
                            uint32_t access,
                            amxc_var_t* ret,
                            int timeout);
// data conversions
PRIVATE
void amxb_rbus_object_to_var(amxc_var_t* var, rbusObject_t object);

PRIVATE
void amxb_rbus_object_to_lvar(amxc_var_t* var, rbusObject_t object);

PRIVATE
void amxb_rbus_value_to_var(amxc_var_t* var, rbusValue_t value);

PRIVATE
void amxb_rbus_htvar_to_robject(const amxc_var_t* var, rbusObject_t object);

PRIVATE
void amxb_rbus_lvar_to_robject(const amxc_var_t* var, rbusObject_t object);

PRIVATE
void amxb_rbus_var_to_rvalue(rbusValue_t value, const amxc_var_t* var);

// utility functions
PRIVATE
amxb_rbus_t* amxb_rbus_get_ctx(rbusHandle_t handle);

PRIVATE
void amxb_rbus_item_free(amxb_rbus_item_t* rbus_item);

PRIVATE
void amxb_rbus_clean_item(amxc_llist_it_t* it);

PRIVATE
bool amxb_rbus_remote_is_amx(amxb_rbus_t* rbus_ctx, const char* object);

PRIVATE
rbusError_t amxb_rbus_common_handler(rbusHandle_t handle,
                                     const char* name,
                                     const char* requester,
                                     uint32_t session,
                                     rbus_condition_wait_t* cond_wait,
                                     amxb_rbus_handler_t handler);
PRIVATE
amxd_trans_t* amxb_rbus_open_transaction(amxb_rbus_t* amxb_rbus_ctx, amxc_string_t* id);

PRIVATE
amxd_trans_t* amxb_rbus_get_transaction(amxb_rbus_t* amxb_rbus_ctx, amxc_string_t* id);

PRIVATE
void amxb_rbus_close_transaction(amxb_rbus_t* amxb_rbus_ctx, amxc_string_t* id);

PRIVATE
void amxb_rbus_remove_subs(amxb_rbus_t* amxb_rbus_ctx);

PRIVATE
int amxb_rbus_invoke_root(amxb_rbus_t* amxb_rbus_ctx,
                          const char* object,
                          const char* method,
                          amxc_var_t* args,
                          amxb_request_t* request);

PRIVATE
int amxb_rbus_call_get(amxb_rbus_t* amxb_rbus_ctx,
                       const char* function,
                       const char* object,
                       const char* search_path,
                       int32_t depth,
                       uint32_t access,
                       amxc_var_t* ret);

PRIVATE
int amxb_rbus_resolve(amxb_rbus_t* amxb_rbus_ctx,
                      const char* object_path,
                      const char* search_path,
                      int32_t depth,
                      amxc_var_t* resolved_objects);

// translate functions
PRIVATE
rbusError_t amxb_rbus_translate_status2rbus(amxd_status_t status);

PRIVATE
amxd_status_t amxb_rbus_translate_rbus2status(rbusError_t error);

PRIVATE
void amxb_rbus_translate_set_paths(void);

PRIVATE
void amxb_rbus_translate_path(amxd_path_t* path, const char** requested, const char** translated);

PRIVATE
char* amxb_rbus_translate_register_path(char* path);

// Ambiorix _ calls return translation
PRIVATE
int amxb_rbus_call_needs_translation(const char* method);

PRIVATE
void amxb_rbus_translate_data(const amxc_var_t* data);

PRIVATE
void amxb_rbus_call_translate(amxc_var_t* out, int funcid);

// rbus specific functions
void amxb_rbus_set_timeout(int timeout); // timeout must be provided in seconds

// utilities
PRIVATE
int isdot(int c);

PRIVATE
int isbraces(int c);

PRIVATE
int amxb_rbus_set_values(amxb_rbus_t* amxb_rbus_ctx,
                         bool required,
                         const char* path,
                         amxc_var_t* values,
                         amxc_var_t* data);

#ifdef __cplusplus
}
#endif

#endif // __AMXB_RBUS_H__

