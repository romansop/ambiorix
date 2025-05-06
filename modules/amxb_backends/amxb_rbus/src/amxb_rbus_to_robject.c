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

void amxb_rbus_var_to_rvalue(rbusValue_t value, const amxc_var_t* var) {
    switch(amxc_var_type_of(var)) {
    default:
    case AMXC_VAR_ID_NULL:
        break;

    case AMXC_VAR_ID_CSTRING:
    case AMXC_VAR_ID_CSV_STRING:
    case AMXC_VAR_ID_SSV_STRING:
        rbusValue_SetString(value, amxc_var_constcast(cstring_t, var));
        break;

    case AMXC_VAR_ID_BOOL:
        rbusValue_SetBoolean(value, amxc_var_constcast(bool, var));
        break;

    case AMXC_VAR_ID_INT8:
        rbusValue_SetInt8(value, amxc_var_constcast(int8_t, var));
        break;

    case AMXC_VAR_ID_UINT8:
        rbusValue_SetUInt8(value, amxc_var_constcast(uint8_t, var));
        break;

    case AMXC_VAR_ID_INT16:
        rbusValue_SetInt16(value, amxc_var_constcast(int16_t, var));
        break;

    case AMXC_VAR_ID_UINT16:
        rbusValue_SetUInt16(value, amxc_var_constcast(uint16_t, var));
        break;

    case AMXC_VAR_ID_INT32:
        rbusValue_SetInt32(value, amxc_var_constcast(int32_t, var));
        break;

    case AMXC_VAR_ID_UINT32:
        rbusValue_SetUInt32(value, amxc_var_constcast(uint32_t, var));
        break;

    case AMXC_VAR_ID_INT64:
        rbusValue_SetInt64(value, amxc_var_constcast(int64_t, var));
        break;

    case AMXC_VAR_ID_UINT64:
        rbusValue_SetUInt64(value, amxc_var_constcast(uint64_t, var));
        break;

    case AMXC_VAR_ID_DOUBLE:
        rbusValue_SetDouble(value, amxc_var_constcast(double, var));
        break;

    case AMXC_VAR_ID_TIMESTAMP: {
        rbusDateTime_t rbus_dt;
        struct tm time;
        const amxc_ts_t* ts = amxc_var_constcast(amxc_ts_t, var);

        memset(&time, 0, sizeof(struct tm));
        memset(&rbus_dt, 0, sizeof(rbusDateTime_t));
        amxc_ts_to_tm_utc(ts, &time);

        rbus_dt.m_time.tm_sec = time.tm_sec;
        rbus_dt.m_time.tm_min = time.tm_min;
        rbus_dt.m_time.tm_hour = time.tm_hour;
        rbus_dt.m_time.tm_mday = time.tm_mday;
        rbus_dt.m_time.tm_mon = time.tm_mon;
        rbus_dt.m_time.tm_year = time.tm_year;
        rbus_dt.m_time.tm_wday = time.tm_wday;
        rbus_dt.m_time.tm_yday = time.tm_yday;
        rbus_dt.m_time.tm_isdst = time.tm_isdst;

        rbusValue_SetTime(value, &rbus_dt);
    }
    break;

    case AMXC_VAR_ID_HTABLE: {
        rbusObject_t object = NULL;
        rbusObject_Init(&object, NULL);

        amxb_rbus_htvar_to_robject(var, object);

        rbusValue_SetObject(value, object);
        rbusObject_Release(object);
    }
    break;

    case AMXC_VAR_ID_LIST: {
        rbusObject_t object = NULL;
        rbusObject_t children = NULL;

        rbusObject_Init(&object, NULL);
        rbusObject_Init(&children, NULL);

        amxb_rbus_lvar_to_robject(var, children);

        rbusObject_SetChildren(object, children);
        rbusObject_Release(children);
        rbusValue_SetObject(value, object);
        rbusObject_Release(object);
    }
    break;

    }
}

void amxb_rbus_htvar_to_robject(const amxc_var_t* var, rbusObject_t object) {
    amxc_var_for_each(property, var) {
        rbusValue_t v;
        const char* name = amxc_var_key(property);

        rbusValue_Init(&v);
        amxb_rbus_var_to_rvalue(v, property);
        rbusObject_SetValue(object, name, v);
        rbusValue_Release(v);
    }
}

void amxb_rbus_lvar_to_robject(const amxc_var_t* var, rbusObject_t object) {
    rbusObject_t next = NULL;

    amxc_var_for_each(property, var) {
        rbusValue_t v;
        rbusValue_Init(&v);

        if(next != NULL) {
            rbusObject_SetNext(object, next);
            object = next;
            rbusObject_Release(next);
        }

        amxb_rbus_var_to_rvalue(v, property);

        rbusObject_SetValue(object, "array_value", v);
        rbusValue_Release(v);

        rbusObject_Init(&next, "array_value");
    }

    rbusObject_Release(next);
}