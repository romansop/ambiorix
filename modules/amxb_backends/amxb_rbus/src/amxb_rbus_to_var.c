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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>

#include "amxb_rbus.h"

// Convert rbusValue to amxc_var
void amxb_rbus_value_to_var(amxc_var_t* var, rbusValue_t value) {
    switch(rbusValue_GetType(value)) {
    default:
    case RBUS_NONE:
        amxc_var_clean(var);
        break;

    case RBUS_STRING:
        amxc_var_set(cstring_t, var, rbusValue_GetString(value, NULL));
        break;

    case RBUS_BOOLEAN:
        amxc_var_set(bool, var, rbusValue_GetBoolean(value));
        break;

    case RBUS_CHAR:
        amxc_var_set(int8_t, var, rbusValue_GetChar(value));
        break;

    case RBUS_INT8:
        amxc_var_set(int8_t, var, rbusValue_GetInt8(value));
        break;

    case RBUS_BYTE:
        amxc_var_set(uint8_t, var, rbusValue_GetByte(value));
        break;

    case RBUS_UINT8:
        amxc_var_set(uint8_t, var, rbusValue_GetUInt8(value));
        break;

    case RBUS_INT16:
        amxc_var_set(int16_t, var, rbusValue_GetInt16(value));
        break;

    case RBUS_UINT16:
        amxc_var_set(uint16_t, var, rbusValue_GetUInt16(value));
        break;

    case RBUS_INT32:
        amxc_var_set(int32_t, var, rbusValue_GetInt32(value));
        break;

    case RBUS_UINT32:
        amxc_var_set(uint32_t, var, rbusValue_GetUInt32(value));
        break;

    case RBUS_INT64:
        amxc_var_set(int64_t, var, rbusValue_GetInt64(value));
        break;

    case RBUS_UINT64:
        amxc_var_set(uint64_t, var, rbusValue_GetUInt64(value));
        break;

    case RBUS_SINGLE:
        amxc_var_set(double, var, (double) rbusValue_GetSingle(value));
        break;

    case RBUS_DOUBLE:
        amxc_var_set(double, var, rbusValue_GetDouble(value));
        break;

    case RBUS_DATETIME: {
        amxc_ts_t ts;
        struct tm time;
        rbusDateTime_t const* rbus_dt = rbusValue_GetTime(value);
        memcpy(&time, &rbus_dt->m_time, sizeof(struct tm));
        ts.sec = mktime(&time);
        ts.nsec = 0;
        ts.offset = rbus_dt->m_tz.m_tzhour * 60 + rbus_dt->m_tz.m_tzmin;
        if(rbus_dt->m_tz.m_isWest) {
            ts.offset *= -1;
        }
        amxc_var_set(amxc_ts_t, var, &ts);
    }
    break;

    case RBUS_OBJECT: {
        rbusObject_t object = rbusValue_GetObject(value);
        rbusObject_t children = rbusObject_GetChildren(object);
        rbusProperty_t property = rbusObject_GetProperties(object);

        if(property != NULL) {
            amxb_rbus_object_to_var(var, object);
        } else if(children != NULL) {
            amxb_rbus_object_to_lvar(var, object);
        } else {
            amxc_var_set_type(var, AMXC_VAR_ID_HTABLE);
        }
    }
    break;
    }
}

// rbus object can be converted to a hash table variant
void amxb_rbus_object_to_var(amxc_var_t* var, rbusObject_t object) {
    amxc_var_set_type(var, AMXC_VAR_ID_HTABLE);
    rbusProperty_t property = rbusObject_GetProperties(object);

    while(property != NULL) {
        const char* name = rbusProperty_GetName(property);
        rbusValue_t v = rbusProperty_GetValue(property);
        amxc_var_t* tv = amxc_var_add_new_key(var, name);
        amxb_rbus_value_to_var(tv, v);
        property = rbusProperty_GetNext(property);
    }
}

// rbus object with children can be converted to a list variant
void amxb_rbus_object_to_lvar(amxc_var_t* var, rbusObject_t object) {
    amxc_var_set_type(var, AMXC_VAR_ID_LIST);
    rbusObject_t next = rbusObject_GetChildren(object);

    while(next != NULL) {
        rbusProperty_t property = rbusObject_GetProperties(next);
        if(property != NULL) {
            rbusValue_t v = rbusProperty_GetValue(property);
            amxc_var_t* lv = amxc_var_add_new(var);
            amxb_rbus_value_to_var(lv, v);
        }
        next = rbusObject_GetNext(next);
    }
}