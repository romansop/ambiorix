/* SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 * SPDX-FileCopyrightText: 2022 the amxb_dbus contributors (see AUTHORS.md)
 *
 * This code is subject to the terms of the BSD+Patent license.
 * See LICENSE file for more details.
 */

#if !defined(__AMXB_DBUS_METHODS_H__)
#define __AMXB_DBUS_METHODS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <amxc/amxc_macros.h>

typedef DBusHandlerResult (* amxb_dbus_func_t) (amxb_dbus_t* amxb_dbus_ctx,
                                                DBusMessage* message,
                                                const char* amx_method,
                                                const char* amx_args[],
                                                amxc_var_t* args);

// Helper metohds
PRIVATE
int isdot(int c);

PRIVATE
void amxd_dbus_dbus_args_to_amx_args(amxc_var_t* dbus_args,
                                     const char* arg_names[],
                                     amxc_var_t* fn_args);

PRIVATE
void amxd_dbus_amx_out_to_dbus_args(DBusMessage* reply,
                                    amxc_var_t* rv,
                                    amxc_var_t* out,
                                    int status);

PRIVATE
DBusMessage* amxb_dbus_call(amxb_dbus_t* amxb_dbus_ctx,
                            const char* object,
                            const char* method,
                            amxc_var_t* args,
                            int timeout);
PRIVATE
void amxb_dbus_fetch_retval(DBusMessage* reply,
                            amxc_var_t* rv,
                            amxc_var_t* out,
                            int* status);

PRIVATE
DBusHandlerResult amxb_dbus_introspect(amxb_dbus_t* amxb_dbus_ctx,
                                       DBusMessage* message,
                                       const char* amx_method,
                                       const char* amx_args[],
                                       amxc_var_t* args);

PRIVATE
DBusHandlerResult amxb_dbus_handle_msg(DBusConnection* connection,
                                       DBusMessage* message,
                                       void* user_data);

#ifdef __cplusplus
}
#endif

#endif // __AMXB_DBUS_METHODS_H__
