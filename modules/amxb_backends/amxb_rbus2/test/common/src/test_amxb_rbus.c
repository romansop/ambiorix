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
#include <string.h>

#include "../../common/include/amxb_rbus_test.h"

int handle_events(void) {
    int retval = 0;

    amxp_timers_calculate();
    amxp_timers_check();

    printf("Handling events \n");
    while(amxp_signal_read() == 0) {
        retval++;
    }
    printf("\n");
    fflush(stdout);
    return retval;
}

int check_rbus_value_str(rbusHandle_t handle, const char* param, const char* expected) {
    int rc = RBUS_ERROR_BUS_ERROR;
    char* str_val = NULL;

    rc = rbus_getStr(handle, param, &str_val);
    when_failed(rc, exit);
    printf("Got %s - expected %s\n", str_val, expected);
    rc = strcmp(str_val, expected);
    free(str_val);
    str_val = NULL;

exit:
    return rc;
}

int check_rbus_value_uint32(rbusHandle_t handle, const char* param, uint32_t expected) {
    int rc = RBUS_ERROR_BUS_ERROR;
    uint32_t uval = 0;

    rc = rbus_getUint(handle, param, &uval);
    when_failed(rc, exit);
    if(uval != expected) {
        rc = RBUS_ERROR_BUS_ERROR;
    }

exit:
    return rc;
}

int check_rbus_value_int32(rbusHandle_t handle, const char* param, int32_t expected) {
    int rc = RBUS_ERROR_BUS_ERROR;
    int32_t val = 0;

    rc = rbus_getInt(handle, param, &val);
    when_failed(rc, exit);
    if(val != expected) {
        rc = RBUS_ERROR_BUS_ERROR;
    }

exit:
    return rc;
}

int check_rbus_value_bool(rbusHandle_t handle, const char* param, bool expected) {
    int rc = RBUS_ERROR_BUS_ERROR;
    bool val = 0;

    rc = rbus_getBoolean(handle, param, &val);
    when_failed(rc, exit);
    if(val != expected) {
        rc = RBUS_ERROR_BUS_ERROR;
    }

exit:
    return rc;
}

int check_rbus_value_uint8(rbusHandle_t handle, const char* param, uint8_t expected) {
    int rc = RBUS_ERROR_BUS_ERROR;
    rbusValue_t value = NULL;

    rc = rbus_get(handle, param, &value);
    when_failed(rc, exit);

    if(rbusValue_GetUInt8(value) != expected) {
        rc = RBUS_ERROR_BUS_ERROR;
    }

    rbusValue_Release(value);

exit:
    return rc;
}

int check_rbus_value_int8(rbusHandle_t handle, const char* param, int8_t expected) {
    int rc = RBUS_ERROR_BUS_ERROR;
    rbusValue_t value = NULL;

    rc = rbus_get(handle, param, &value);
    when_failed(rc, exit);

    if(rbusValue_GetInt8(value) != expected) {
        rc = RBUS_ERROR_BUS_ERROR;
    }

    rbusValue_Release(value);

exit:
    return rc;
}

void check_supported_datamodel_is_available(rbusHandle_t handle, const char* component, const char* expected[], int line) {
    int rc = RBUS_ERROR_SUCCESS;
    int num_elements = 0;
    char** elements = NULL;

    printf("Check data model is available called from line %d\n", line);
    fflush(stdout);

    // When calling rbus_discoverComponentDataElements on the component name
    rc = rbus_discoverComponentDataElements(handle, component, false, &num_elements, &elements);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // All available parameters, events and methods must be returned
    assert_int_not_equal(num_elements, 0);
    // item 0 is the component name and must match with the name provided
    assert_string_equal(component, elements[0]);
    free(elements[0]);
    for(int i = 1; i < num_elements; i++) {
        printf("Element [%s]\n", elements[i]);
        assert_string_equal(expected[i - 1], elements[i]);
        free(elements[i]);
    }
    free(elements);
}

void check_supported_datamodel_is_unavailable(rbusHandle_t handle, const char* component, int line) {
    int rc = RBUS_ERROR_SUCCESS;
    int num_elements = 0;
    char** elements = NULL;

    printf("Check data model is unavailable called from line %d\n", line);
    fflush(stdout);

    // When calling rbus_discoverComponentDataElements on the component name
    rc = rbus_discoverComponentDataElements(handle, component, false, &num_elements, &elements);
    // The return code must be ERROR_SUCCESS
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    // Only the component name should be available.
    assert_int_equal(num_elements, 1);
    // item 0 is the component name and must match with the name provided
    assert_string_equal(component, elements[0]);
    free(elements[0]);
    free(elements);
}

int check_rows_are_available(rbusHandle_t handle, const char* table, const char* expected[], int line) {
    int rc = RBUS_ERROR_SUCCESS;
    rbusRowName_t* rows = NULL;
    rbusRowName_t* row = NULL;
    int i = 0;

    printf("Check rows are available is called from line %d\n", line);
    printf("Checking rows of table [%s]\n", table);
    fflush(stdout);

    rc = rbusTable_getRowNames(handle, table, &rows);
    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    if(expected == NULL) {
        assert_null(rows);
    } else {
        assert_non_null(rows);
    }
    for(row = rows; row != NULL; row = row->next) {
        printf("Found row: [%s] (Alias = [%s])\n", row->name, row->alias ? row->alias : "");
        assert_string_equal(row->name, expected[i]);
        i++;
    }
    rbusTable_freeRowNames(handle, rows);

    return i;
}

void check_element_info(rbusHandle_t handle, const char* element_name, int depth, expected_type_t* expected, int line) {
    int rc = RBUS_ERROR_SUCCESS;
    int index = 0;

    rbusElementInfo_t* element = NULL;
    rbusElementInfo_t* elems = NULL;

    rc = rbusElementInfo_get(handle, element_name, depth, &elems);
    printf("Check element info is called from line %d\n", line);
    printf("Checking element [%s]\n", element_name);
    fflush(stdout);

    assert_int_equal(rc, RBUS_ERROR_SUCCESS);
    assert_non_null(elems);

    for(element = elems; element != NULL; element = element->next) {
        printf("Got element %s (type = %d, access = %d)\n", element->name, element->type, element->access);
        fflush(stdout);
        assert_string_equal(expected[index].name, element->name);
        assert_int_equal(expected[index].type, element->type);
        assert_int_equal(expected[index].access, element->access);
        index++;
    }

    rbusElementInfo_free(handle, elems);
}

int check_amx_response(amxc_var_t* result, const char* data_file) {
    int retval = 0;
    amxc_var_t* verify = NULL;

    verify = amxut_util_read_json_from_file(data_file);
    amxc_var_compare(result, verify, &retval);
    if(retval != 0) {
        printf("***** Unexpected data recieved *****\n");
        printf("EXPECTED:\n");
        fflush(stdout);
        amxc_var_dump(verify, STDOUT_FILENO);
        printf("GOT:\n");
        fflush(stdout);
        amxc_var_dump(result, STDOUT_FILENO);
        amxc_var_cast(result, AMXC_VAR_ID_JSON);
        printf("JSON:\n");
        fflush(stdout);
        amxc_var_dump(result, STDOUT_FILENO);
        printf("************************************\n");
    }
    amxc_var_delete(&verify);

    return retval;
}