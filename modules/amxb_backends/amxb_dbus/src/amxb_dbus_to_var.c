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

static int amxb_dbus_map(amxc_var_t* map, DBusMessageIter* elm) {
    int retval = amxd_status_ok;
    DBusMessageIter sub;
    amxc_var_t var_key;
    amxc_var_t* var_data = NULL;

    const char* key = NULL;
    amxc_var_init(&var_key);

    while(dbus_message_iter_get_arg_type(elm) != DBUS_TYPE_INVALID) {
        // get key
        dbus_message_iter_recurse(elm, &sub);
        retval = amxb_dbus_to_var(&var_key, &sub);
        if(retval != amxd_status_ok) {
            goto exit;
        }

        key = amxc_var_constcast(cstring_t, &var_key);
        var_data = amxc_var_add_new_key(map, key);

        // get data
        dbus_message_iter_next(&sub);
        retval = amxb_dbus_to_var(var_data, &sub);
        if(retval != amxd_status_ok) {
            goto exit;
        }

        dbus_message_iter_next(elm);
    }

exit:
    amxc_var_clean(&var_key);

    return retval;
}

static int amxb_dbus_array(amxc_var_t* list, DBusMessageIter* elm) {
    amxc_var_t* var = NULL;
    int retval = amxd_status_ok;

    while(dbus_message_iter_get_arg_type(elm) != DBUS_TYPE_INVALID) {
        var = amxc_var_add_new(list);
        retval = amxb_dbus_to_var(var, elm);
        if(retval != amxd_status_ok) {
            amxc_var_delete(&var);
            goto exit;
        }

        dbus_message_iter_next(elm);
    }

exit:
    return retval;
}

int amxb_dbus_to_var(amxc_var_t* value, DBusMessageIter* arg) {
    int retval = 0;
    switch(dbus_message_iter_get_arg_type(arg)) {
    case DBUS_TYPE_STRING: {
        char* data = NULL;
        dbus_message_iter_get_basic(arg, &data);
        retval = amxc_var_set(cstring_t, value, data);
    }
    break;
    case DBUS_TYPE_INT16: {
        int16_t data = 0;
        dbus_message_iter_get_basic(arg, &data);
        retval = amxc_var_set(int16_t, value, data);
    }
    break;
    case DBUS_TYPE_INT32: {
        int32_t data = 0;
        dbus_message_iter_get_basic(arg, &data);
        retval = amxc_var_set(int32_t, value, data);
    }
    break;
    case DBUS_TYPE_INT64: {
        int64_t data = 0;
        dbus_message_iter_get_basic(arg, &data);
        retval = amxc_var_set(int64_t, value, data);
    }
    break;
    case DBUS_TYPE_BYTE: {
        uint8_t data = 0;
        dbus_message_iter_get_basic(arg, &data);
        retval = amxc_var_set(uint8_t, value, data);
    }
    break;
    case DBUS_TYPE_UINT16: {
        uint16_t data = 0;
        dbus_message_iter_get_basic(arg, &data);
        retval = amxc_var_set(uint16_t, value, data);
    }
    break;
    case DBUS_TYPE_UINT32: {
        uint32_t data = 0;
        dbus_message_iter_get_basic(arg, &data);
        retval = amxc_var_set(uint32_t, value, data);
    }
    break;
    case DBUS_TYPE_UINT64: {
        uint64_t data = 0;
        dbus_message_iter_get_basic(arg, &data);
        retval = amxc_var_set(uint64_t, value, data);
    }
    break;
    case DBUS_TYPE_BOOLEAN: {
        bool data = false;
        dbus_message_iter_get_basic(arg, &data);
        retval = amxc_var_set(bool, value, data);
    }
    break;
    case DBUS_TYPE_VARIANT: {
        DBusMessageIter dbus_variant;
        dbus_message_iter_recurse(arg, &dbus_variant);
        retval = amxb_dbus_to_var(value, &dbus_variant);
    }
    break;
    case DBUS_TYPE_STRUCT:
    case DBUS_TYPE_ARRAY: {
        DBusMessageIter sub;
        dbus_message_iter_recurse(arg, &sub);

        if(dbus_message_iter_get_arg_type(&sub) == DBUS_MESSAGE_TYPE_INVALID) {
            amxc_var_set_type(value, AMXC_VAR_ID_NULL);
            retval = 0;
        } else if(dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_DICT_ENTRY) {
            // a dictionary, convert to a variant map
            amxc_var_set_type(value, AMXC_VAR_ID_HTABLE);
            retval = amxb_dbus_map(value, &sub);
        } else {
            // an array of values, convert to a variant list
            amxc_var_set_type(value, AMXC_VAR_ID_LIST);
            retval = amxb_dbus_array(value, &sub);
        }


    }
    break;
    case DBUS_TYPE_OBJECT_PATH: {
        char* data = NULL;
        dbus_message_iter_get_basic(arg, &data);
        retval = amxc_var_set(cstring_t, value, data);
    }
    break;
    case DBUS_TYPE_DOUBLE:
    case DBUS_TYPE_UNIX_FD:
    case DBUS_STRUCT_BEGIN_CHAR:
    case DBUS_STRUCT_END_CHAR:
    case DBUS_DICT_ENTRY_BEGIN_CHAR:
    case DBUS_DICT_ENTRY_END_CHAR:
    case DBUS_TYPE_SIGNATURE:
    default:
        break;
    }

    return retval;
}
