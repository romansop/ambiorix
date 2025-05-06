/* SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 * SPDX-FileCopyrightText: 2022 the amxb_dbus contributors (see AUTHORS.md)
 *
 * This code is subject to the terms of the BSD+Patent license.
 * See LICENSE file for more details.
 */

#if !defined(__AMXB_DBUS_H__)
#define __AMXB_DBUS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

#include <amxc/amxc_macros.h>
#include <amxc/amxc.h>
#include <amxp/amxp.h>

#include <amxd/amxd_function.h>
#include <amxd/amxd_path.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>

#include <amxb/amxb.h>
#include <amxb/amxb_be_intf.h>

#include <dbus/dbus.h>

#define AMX_DM_INTERFACE "org.prpl.amx"
#define AMX_DEST_PREFIX  "org.prpl.dm."

typedef struct _amxb_dbus_sub {
    amxc_htable_it_t it;
} amxb_dbus_sub_t;

typedef struct _amxb_dbus {
    DBusConnection* dbus_handle;
    DBusWatch* watch;
    amxd_dm_t* dm;
    amxp_signal_mngr_t* sigmngr;
    amxc_htable_t subscribers;
} amxb_dbus_t;

amxb_be_info_t* amxb_be_info(void);

PRIVATE
const amxc_var_t* amxb_dbus_get_config_option(const char* name);

PRIVATE
const amxc_var_t* amxb_dbus_get_config(void);

PRIVATE
int amxb_dbus_register(void* const ctx, amxd_dm_t* const dm);

PRIVATE
int amxb_dbus_to_var(amxc_var_t* value, DBusMessageIter* arg);

PRIVATE
int amxb_var_to_dbus(const amxc_var_t* value, DBusMessageIter* arg);

PRIVATE
int amxb_dbus_async_invoke(void* const ctx,
                           amxb_invoke_t* invoke_ctx,
                           amxc_var_t* args,
                           amxb_request_t* request);

PRIVATE
int amxb_dbus_invoke(void* const ctx,
                     amxb_invoke_t* invoke_ctx,
                     amxc_var_t* args,
                     amxb_request_t* request,
                     int timeout);
PRIVATE
int amxb_dbus_get(void* const ctx,
                  const char* object,
                  const char* search_path,
                  int32_t depth,
                  uint32_t access,
                  amxc_var_t* ret,
                  int timeout);

PRIVATE
int amxb_dbus_set(void* const ctx,
                  const char* object,
                  const char* search_path,
                  uint32_t flags,
                  amxc_var_t* values,
                  amxc_var_t* ovalues,
                  uint32_t access,
                  amxc_var_t* ret,
                  int timeout);

PRIVATE
int amxb_dbus_add(void* const ctx,
                  const char* object,
                  const char* search_path,
                  uint32_t index,
                  const char* name,
                  amxc_var_t* values,
                  uint32_t access,
                  amxc_var_t* ret,
                  int timeout);

PRIVATE
int amxb_dbus_del(void* const ctx,
                  const char* object,
                  const char* search_path,
                  uint32_t index,
                  const char* name,
                  uint32_t access,
                  amxc_var_t* ret,
                  int timeout);

PRIVATE
int amxb_dbus_describe(void* const ctx,
                       const char* object,
                       const char* search_path,
                       uint32_t flags,
                       uint32_t access,
                       amxc_var_t* retval,
                       int timeout);

PRIVATE
int amxb_dbus_gsdm(void* const ctx,
                   const char* object,
                   const char* search_path,
                   uint32_t flags,
                   amxc_var_t* retval,
                   int timeout);

PRIVATE
int amxb_dbus_subscribe(void* const ctx, const char* object);

PRIVATE
int amxb_dbus_close_request(void* const ctx, amxb_request_t* request);

PRIVATE
int amxb_dbus_list(void* const ctx, const char* object, uint32_t flags, uint32_t access, amxb_request_t* request);

#ifdef __cplusplus
}
#endif

#endif // __AMXB_DBUS_H__
