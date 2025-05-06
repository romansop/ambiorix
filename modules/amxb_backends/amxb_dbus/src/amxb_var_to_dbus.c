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

static const char* amxb_dbus_variant_to_dbus_signature(const amxc_var_t* data) {
    switch(amxc_var_type_of(data)) {
    case AMXC_VAR_ID_CSTRING:
        return DBUS_TYPE_STRING_AS_STRING;
        break;
    case AMXC_VAR_ID_INT8:
    case AMXC_VAR_ID_INT16:
        return DBUS_TYPE_INT16_AS_STRING;
        break;
    case AMXC_VAR_ID_INT32:
        return DBUS_TYPE_INT32_AS_STRING;
        break;
    case AMXC_VAR_ID_INT64:
        return DBUS_TYPE_INT64_AS_STRING;
        break;
    case AMXC_VAR_ID_UINT8:
        return DBUS_TYPE_BYTE_AS_STRING;
        break;
    case AMXC_VAR_ID_UINT16:
        return DBUS_TYPE_UINT16_AS_STRING;
        break;
    case AMXC_VAR_ID_UINT32:
        return DBUS_TYPE_UINT32_AS_STRING;
        break;
    case AMXC_VAR_ID_UINT64:
        return DBUS_TYPE_UINT64_AS_STRING;
        break;
    case AMXC_VAR_ID_BOOL:
        return DBUS_TYPE_BOOLEAN_AS_STRING;
        break;
    case AMXC_VAR_ID_DOUBLE:
        return DBUS_TYPE_DOUBLE_AS_STRING;
        break;
    case AMXC_VAR_ID_LIST:
        return DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_VARIANT_AS_STRING;
        break;
    case AMXC_VAR_ID_HTABLE:
        return DBUS_TYPE_ARRAY_AS_STRING DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING DBUS_DICT_ENTRY_END_CHAR_AS_STRING;
    default:
        return DBUS_TYPE_STRING_AS_STRING;
        break;
    }

    return "";
}

static int amxb_dbus_add_dict(const amxc_var_t* table, DBusMessageIter* arg) {
    DBusMessageIter dict;
    DBusMessageIter entry;
    DBusMessageIter variant;
    int retval = 0;

    const char* signature = DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING DBUS_DICT_ENTRY_END_CHAR_AS_STRING;

    // create dbus container using dbus_message_iter_open_container
    dbus_message_iter_open_container(arg, DBUS_TYPE_ARRAY, signature, &dict);

    // loop over all the elements and add them to the dbus array
    amxc_var_for_each(value, table) {
        const char* key = amxc_var_key(value);

        // add dict entry
        dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
        // set the key
        dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
        // open the variant
        dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, amxb_dbus_variant_to_dbus_signature(value), &variant);
        // set the value
        amxb_var_to_dbus(value, &variant);
        // close variant
        dbus_message_iter_close_container(&entry, &variant);
        // close dict entry
        dbus_message_iter_close_container(&dict, &entry);
    }

    // close the dict
    dbus_message_iter_close_container(arg, &dict);

    return retval;
}

static int amxb_dbus_add_array(const amxc_var_t* list, DBusMessageIter* arg) {
    DBusMessageIter array;
    DBusMessageIter variant;

    dbus_message_iter_open_container(arg, DBUS_TYPE_ARRAY, DBUS_TYPE_VARIANT_AS_STRING, &array);
    amxc_var_for_each(value, list) {
        dbus_message_iter_open_container(&array, DBUS_TYPE_VARIANT, amxb_dbus_variant_to_dbus_signature(value), &variant);
        amxb_var_to_dbus(value, &variant);
        dbus_message_iter_close_container(&array, &variant);
    }
    dbus_message_iter_close_container(arg, &array);

    return 0;

}

int amxb_var_to_dbus(const amxc_var_t* value, DBusMessageIter* arg) {
    int retval = 0;
    switch(amxc_var_type_of(value)) {
    case AMXC_VAR_ID_NULL: {
        amxc_var_t empty_table;
        amxc_var_init(&empty_table);
        amxc_var_set_type(&empty_table, AMXC_VAR_ID_HTABLE);
        retval = amxb_dbus_add_dict(&empty_table, arg);
        amxc_var_clean(&empty_table);
    }
    break;
    case AMXC_VAR_ID_CSTRING: {
        const char* str = amxc_var_constcast(cstring_t, value);
        retval = dbus_message_iter_append_basic(arg, DBUS_TYPE_STRING, &str);
    }
    break;
    case AMXC_VAR_ID_INT8:
    case AMXC_VAR_ID_INT16: {
        int16_t i = amxc_var_constcast(int16_t, value);
        retval = dbus_message_iter_append_basic(arg, DBUS_TYPE_INT16, &i);
    }
    break;
    case AMXC_VAR_ID_INT32: {
        int32_t i = amxc_var_constcast(int32_t, value);
        retval = dbus_message_iter_append_basic(arg, DBUS_TYPE_INT32, &i);
    }
    break;
    case AMXC_VAR_ID_INT64: {
        int64_t i = amxc_var_constcast(int64_t, value);
        retval = dbus_message_iter_append_basic(arg, DBUS_TYPE_INT64, &i);
    }
    break;
    case AMXC_VAR_ID_UINT8: {
        uint8_t i = amxc_var_constcast(uint8_t, value);
        retval = dbus_message_iter_append_basic(arg, DBUS_TYPE_BYTE, &i);
    }
    break;
    case AMXC_VAR_ID_UINT16: {
        uint16_t i = amxc_var_constcast(uint16_t, value);
        retval = dbus_message_iter_append_basic(arg, DBUS_TYPE_UINT16, &i);
    }
    break;
    case AMXC_VAR_ID_UINT32: {
        uint32_t i = amxc_var_constcast(uint32_t, value);
        retval = dbus_message_iter_append_basic(arg, DBUS_TYPE_UINT32, &i);
    }
    break;
    case AMXC_VAR_ID_UINT64: {
        uint64_t i = amxc_var_constcast(uint64_t, value);
        retval = dbus_message_iter_append_basic(arg, DBUS_TYPE_UINT64, &i);
    }
    break;
    case AMXC_VAR_ID_BOOL: {
        int i = amxc_var_dyncast(int32_t, value);
        retval = dbus_message_iter_append_basic(arg, DBUS_TYPE_BOOLEAN, &i);
    }
    break;
    case AMXC_VAR_ID_DOUBLE: {
        double i = amxc_var_constcast(double, value);
        retval = dbus_message_iter_append_basic(arg, DBUS_TYPE_DOUBLE, &i);
    }
    break;
    case AMXC_VAR_ID_LIST:
        retval = amxb_dbus_add_array(value, arg);
        break;
    case AMXC_VAR_ID_HTABLE:
        retval = amxb_dbus_add_dict(value, arg);
        break;
    default: {
        char* str = amxc_var_dyncast(cstring_t, value);
        retval = dbus_message_iter_append_basic(arg, DBUS_TYPE_STRING, &str);
        free(str);
    }
    break;
    }

    return retval;
}

