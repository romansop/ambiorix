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
#if !defined(__AMXB_UBUS_H__)
#define __AMXB_UBUS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <libubus.h>
#include <libubox/blob.h>
#include <libubox/uloop.h>
#include <libubox/usock.h>
#include <libubox/list.h>

#include <amxc/amxc_macros.h>
#include <amxc/amxc.h>
#include <amxp/amxp.h>

#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>
#include <amxd/amxd_function.h>
#include <amxd/amxd_path.h>

#include <amxb/amxb.h>
#include <amxb/amxb_be_intf.h>

typedef struct _amxb_ubus_sub {
    amxc_htable_it_t it;
    struct ubus_subscriber sub;
    amxp_timer_t* reactivate;
    uint32_t id;
} amxb_ubus_sub_t;

typedef struct _amxb_ubus_stats {
    uint64_t counter_rx_invoke;
} amxb_ubus_stats_t;

typedef struct _amxb_ubus {
    struct ubus_context* ubus_ctx;
    amxc_htable_t subscribers;          // one for each object
    struct blob_buf b;
    amxp_signal_mngr_t* sigmngr;
    amxc_llist_t registered_objs;
    struct ubus_event_handler watcher;
    struct ubus_event_handler wait_watcher;
    amxc_llist_t pending_reqs;
    amxd_dm_t* dm;
    amxb_ubus_stats_t stats;
} amxb_ubus_t;

typedef struct _amxb_ubus_object {
    amxc_llist_it_t it;
    char* ubus_path;
    uint32_t ubus_path_len;
    struct ubus_object ubus_obj;
    amxb_ubus_t* amxb_ubus_ctx;
} amxb_ubus_object_t;

typedef struct _amxb_ubus_request {
    struct ubus_request* ubus_req;
    uint32_t id;
    amxc_var_t args;
    amxb_request_t* request;
    bool converted;
    uint32_t items;    // nr of items expected when reassembling fragmented messages
} amxb_ubus_request_t;

amxb_be_info_t* amxb_be_info(void);

PRIVATE
int amxb_ubus_parse_blob(amxc_var_t* var,
                         struct blob_attr* attr,
                         bool table);

PRIVATE
int amxb_ubus_parse_blob_table(amxc_var_t* var,
                               struct blob_attr* attr,
                               int len);

PRIVATE
int amxb_ubus_parse_blob_array(amxc_var_t* var,
                               struct blob_attr* attr,
                               int len);

PRIVATE
int amxb_ubus_format_blob(amxc_var_t* data,
                          const char* key,
                          struct blob_buf* b);

PRIVATE
int amxb_ubus_format_blob_table(const amxc_htable_t* table,
                                struct blob_buf* b);

PRIVATE
int amxb_ubus_format_blob_array(const amxc_llist_t* list,
                                struct blob_buf* b);

PRIVATE
int amxb_ubus_get_longest_path(amxb_ubus_t* amxb_ubus_ctx,
                               amxd_path_t* path,
                               amxc_string_t* rel_path);

PRIVATE
int amxb_ubus_invoke_base(amxb_ubus_t* amxb_ubus_ctx,
                          const char* object,
                          amxc_var_t* args,
                          amxb_request_t* request,
                          uint32_t* id);

PRIVATE
void* amxb_ubus_connect(const char* host,
                        const char* port,
                        const char* path,
                        amxp_signal_mngr_t* sigmngr);

PRIVATE
int amxb_ubus_disconnect(void* ctx);

PRIVATE
int amxb_ubus_get_fd(void* ctx);

PRIVATE
int amxb_ubus_read(void* ctx);

PRIVATE
void amxb_ubus_free(void* ctx);

PRIVATE
int amxb_ubus_invoke(void* const ctx,
                     amxb_invoke_t* invoke_ctx,
                     amxc_var_t* args,
                     amxb_request_t* request,
                     int timeout);

PRIVATE
int amxb_ubus_async_invoke(void* const ctx,
                           amxb_invoke_t* invoke_ctx,
                           amxc_var_t* args,
                           amxb_request_t* request);

PRIVATE
int amxb_ubus_wait_request(void* const ctx,
                           amxb_request_t* request,
                           int timeout);

PRIVATE
int amxb_ubus_close_request(void* const ctx,
                            amxb_request_t* request);

PRIVATE
int amxb_ubus_subscribe(void* const ctx,
                        const char* object);

PRIVATE
int amxb_ubus_unsubscribe(void* const ctx,
                          const char* object);

PRIVATE
int amxb_ubus_register(void* const ctx,
                       amxd_dm_t* const dm);

PRIVATE
void amxb_ubus_cancel_requests(amxb_ubus_t* amxb_ubus_ctx);

PRIVATE
void amxb_ubus_obj_it_free(amxc_llist_it_t* it);

PRIVATE
int amxb_ubus_list(void* const bus_ctx,
                   const char* object,
                   uint32_t flags,
                   uint32_t access,
                   amxb_request_t* request);

PRIVATE
const amxc_var_t* amxb_ubus_get_config_option(const char* name);

PRIVATE
const char* amxb_ubus_log_output(void);

PRIVATE
void amxb_ubus_log(const char* fmt, ...)
__attribute__ ((format(printf, 1, 2)));

PRIVATE
void amxb_ubus_log_variant(const char* msg, const amxc_var_t* var);

#ifdef __cplusplus
}
#endif

#endif // __AMXB_UBUS_H__

