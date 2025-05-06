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

#include <stdlib.h>
#include <string.h>

#include "amxb_dbus_version.h"
#include "amxb_dbus.h"
#include "amxb_dbus_methods.h"

static const char* amx_introspection_xml =
    DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE
    "<node>\n"

    "  <interface name='org.freedesktop.DBus.Introspectable'>\n"
    "    <method name='Introspect'>\n"
    "      <arg name='data' type='s' direction='out' />\n"
    "    </method>\n"
    "  </interface>\n"

    "  <interface name='${dm-interface}'>\n"
    "    <method name='get'>\n"
    "      <arg name='path'          type='s'      direction='in'  />\n"
    "      <arg name='parameters'    type='as'     direction='in'  />\n"
    "      <arg name='depth'         type='i'      direction='in'  />\n"
    "      <arg name='access'        type='u'      direction='in'  />\n"
    "      <arg name='filter'        type='s'      direction='in'  />\n"
    "      <arg name='result'        type='av'     direction='out' />\n"
    "      <arg name='status'        type='i'      direction='out' />\n"
    "    </method>\n"

    "    <method name='set'>\n"
    "      <arg name='path'          type='s'      direction='in'  />\n"
    "      <arg name='parameters'    type='a{sv}'  direction='in'  />\n"
    "      <arg name='oparameters'   type='a{sv}'  direction='in'  />\n"
    "      <arg name='access'        type='u'      direction='in'  />\n"
    "      <arg name='allow_partial' type='b'      direction='in'  />\n"
    "      <arg name='result'        type='av'     direction='out' />\n"
    "      <arg name='status'        type='i'      direction='out' />\n"
    "    </method>\n"

    "    <method name='add'>\n"
    "      <arg name='path'          type='s'      direction='in'  />\n"
    "      <arg name='parameters'    type='a{sv}'  direction='in'  />\n"
    "      <arg name='index'         type='u'      direction='in'  />\n"
    "      <arg name='name'          type='s'      direction='in'  />\n"
    "      <arg name='access'        type='u'      direction='in'  />\n"
    "      <arg name='result'        type='av'     direction='out' />\n"
    "      <arg name='status'        type='i'      direction='out' />\n"
    "    </method>\n"

    "    <method name='del'>\n"
    "      <arg name='path'          type='s'      direction='in'  />\n"
    "      <arg name='index'         type='u'      direction='in'  />\n"
    "      <arg name='name'          type='s'      direction='in'  />\n"
    "      <arg name='access'        type='u'      direction='in'  />\n"
    "      <arg name='result'        type='av'     direction='out' />\n"
    "      <arg name='status'        type='i'      direction='out' />\n"
    "    </method>\n"

    "    <method name='gsdm'>\n"
    "      <arg name='path'             type='s'      direction='in'  />\n"
    "      <arg name='parameters'       type='b'      direction='in'  />\n"
    "      <arg name='functions'        type='b'      direction='in'  />\n"
    "      <arg name='events'           type='b'      direction='in'  />\n"
    "      <arg name='first_level_only' type='b'      direction='in'  />\n"
    "      <arg name='result'           type='av'     direction='out' />\n"
    "      <arg name='status'           type='i'      direction='out' />\n"
    "    </method>\n"

    "    <method name='execute'>\n"
    "      <arg name='path'          type='s'      direction='in'  />\n"
    "      <arg name='method'        type='s'      direction='in'  />\n"
    "      <arg name='args'          type='a{sv}'  direction='in'  />\n"
    "      <arg name='result'        type='av'     direction='out' />\n"
    "      <arg name='status'        type='i'      direction='out' />\n"
    "    </method>\n"

    "    <method name='describe'>\n"
    "      <arg name='path'          type='s'      direction='in'  />\n"
    "      <arg name='parameters'    type='b'      direction='in'  />\n"
    "      <arg name='functions'     type='b'      direction='in'  />\n"
    "      <arg name='objects'       type='b'      direction='in'  />\n"
    "      <arg name='instances'     type='b'      direction='in'  />\n"
    "      <arg name='exists'        type='b'      direction='in'  />\n"
    "      <arg name='events'        type='b'      direction='in'  />\n"
    "      <arg name='access'        type='u'      direction='in'  />\n"
    "      <arg name='result'        type='av'     direction='out' />\n"
    "      <arg name='status'        type='i'      direction='out' />\n"
    "    </method>\n"

    "    <signal name='dmevent'>\n"
    "      <arg type='a{sv}'/>\n"
    "    </signal>\n"

    "  </interface>\n"

    "</node>\n";

DBusHandlerResult amxb_dbus_introspect(amxb_dbus_t* amxb_dbus_ctx,
                                       DBusMessage* message,
                                       UNUSED const char* amx_method,
                                       UNUSED const char* amx_args[],
                                       UNUSED amxc_var_t* args) {
    DBusHandlerResult result = DBUS_HANDLER_RESULT_HANDLED;
    DBusMessage* reply = NULL;
    amxc_string_t xml;
    amxc_string_init(&xml, 0);
    const char* resolved = NULL;

    if(!(reply = dbus_message_new_method_return(message))) {
        goto exit;
    }

    amxc_string_set_resolved(&xml, amx_introspection_xml, amxb_dbus_get_config());
    resolved = amxc_string_get(&xml, 0);
    dbus_message_append_args(reply,
                             DBUS_TYPE_STRING, &resolved,
                             DBUS_TYPE_INVALID);

exit:
    if(reply == NULL) {
        result = DBUS_HANDLER_RESULT_NEED_MEMORY;
    } else {
        if(!dbus_connection_send(amxb_dbus_ctx->dbus_handle, reply, NULL)) {
            result = DBUS_HANDLER_RESULT_NEED_MEMORY;
        }
        dbus_message_unref(reply);
    }
    amxc_string_clean(&xml);
    return result;
}
