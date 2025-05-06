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

#include <string.h>

#include "amxb_rbus.h"

int rbus_discoverWildcardDestinations(const char* expression, int* count, char*** destinations);
int rbus_discoverObjectElements(const char* object, int* count, char*** elements);

typedef enum _element_type {
    element_type_parameter,
    element_type_method,
    element_type_event
} element_type_t;

typedef enum _object_type {
    object_type_table,
    object_type_object
} object_type_t;

static int amxb_rbus_find_providers(const char* object,
                                    amxc_llist_t* providers) {
    int count = 0;
    int retval = 0;
    char** destinations = NULL;
    amxc_string_t* provider = NULL;

    retval = rbus_discoverWildcardDestinations(object, &count, &destinations);
    when_failed(retval, exit);
    when_true(count == 0, exit);

    for(int i = 0; i < count; i++) {
        amxc_string_new(&provider, 0);
        amxc_string_set(provider, destinations[i]);
        amxc_llist_append(providers, &provider->it);
        free(destinations[i]);
    }
    free(destinations);

exit:
    return retval;
}

static element_type_t amxb_rbus_get_element_type(const char* element_name) {
    uint32_t length = strlen(element_name);
    element_type_t type = element_type_parameter;

    if((length >= 2) && (strncmp(element_name + (length - 2), "()", 2) == 0)) {
        type = element_type_method;
    } else if((length >= 1) && (element_name[length - 1] == '!')) {
        type = element_type_event;
    }

    return type;
}

static void amxb_rbus_add_element(const char* element_name,
                                  amxc_var_t* gsdm_object,
                                  uint32_t flags) {
    amxc_var_t* element = NULL;
    element_type_t type = element_type_parameter;
    when_null(element_name, exit);

    type = amxb_rbus_get_element_type(element_name);

    if(((flags & AMXB_FLAG_FUNCTIONS) != 0) && (type == element_type_method)) {
        element = GET_ARG(gsdm_object, "supported_commands");
        if(element == NULL) {
            element = amxc_var_add_key(amxc_llist_t, gsdm_object, "supported_commands", NULL);
        }
        element = amxc_var_add(amxc_htable_t, element, NULL);
        amxc_var_add_key(uint32_t, element, "command_type", 0);
        amxc_var_add_key(amxc_llist_t, element, "output_arg_names", NULL);
        amxc_var_add_key(amxc_llist_t, element, "input_arg_names", NULL);
        amxc_var_add_key(cstring_t, element, "command_name", element_name);
    }

    if(((flags & AMXB_FLAG_PARAMETERS) != 0) && (type == element_type_parameter)) {
        element = GET_ARG(gsdm_object, "supported_params");
        if(element == NULL) {
            element = amxc_var_add_key(amxc_llist_t, gsdm_object, "supported_params", NULL);
        }
        element = amxc_var_add(amxc_htable_t, element, NULL);
        amxc_var_add_key(uint32_t, element, "type", 1);
        amxc_var_add_key(bool, element, "value_change", true);
        amxc_var_add_key(uint32_t, element, "access", 1);
        amxc_var_add_key(cstring_t, element, "param_name", element_name);
    }

    if(((flags & AMXB_FLAG_EVENTS) != 0) && (type == element_type_event)) {
        element = GET_ARG(gsdm_object, "supported_events");
        if(element == NULL) {
            element = amxc_var_add_key(amxc_htable_t, gsdm_object, "supported_events", NULL);
        }
        amxc_var_add_key(amxc_llist_t, element, element_name, NULL);
    }

exit:
    return;
}

static void amxb_rbus_add_object(const char* gsdm_path,
                                 object_type_t type,
                                 const char* element_name,
                                 uint32_t flags,
                                 amxc_var_t* result) {
    amxc_var_t* gsdm_object = NULL;

    gsdm_object = amxc_var_add_key(amxc_htable_t, result, gsdm_path, NULL);
    if(gsdm_object == NULL) {
        gsdm_object = GET_ARG(result, gsdm_path);
    }
    amxb_rbus_add_element(element_name, gsdm_object, flags);

    if(type == object_type_object) {
        amxc_var_add_key(bool, gsdm_object, "is_multi_instance", false);
    } else {
        amxc_var_add_key(bool, gsdm_object, "is_multi_instance", true);
        amxc_var_add_key(uint32_t, gsdm_object, "access", 1);
    }
}

static int amxb_rbus_add_data_elements(amxd_path_t* requested,
                                       int count,
                                       int pos,
                                       char** elements,
                                       uint32_t flags,
                                       amxc_var_t* result) {
    amxd_path_t current;
    amxd_path_t temp;
    char* requested_path = amxd_path_get_supported_path(requested);
    uint32_t requested_depth = 0;
    int i = 0;

    amxd_path_init(&current, NULL);
    amxd_path_init(&temp, requested_path);
    requested_depth = amxd_path_get_depth(&temp);

    for(i = pos; i < count; i++) {
        const char* gsdm_path = NULL;
        const char* param = NULL;
        char* current_path = NULL;
        uint32_t gsdm_len = 0;
        uint32_t current_depth = 0;

        amxd_path_setf(&current, false, "%s", elements[i]);
        param = amxd_path_get_param(&current);
        current_path = amxd_path_get_supported_path(&current);
        amxd_path_setf(&temp, false, "%s", current_path);
        current_depth = amxd_path_get_depth(&temp);
        gsdm_path = amxd_path_get(&current, AMXD_OBJECT_TERMINATE);

        free(elements[i]);
        elements[i] = NULL;

        if(current_depth < requested_depth) {
            free(current_path);
            continue;
        }

        if((flags & AMXB_FLAG_FIRST_LVL) == AMXB_FLAG_FIRST_LVL) {
            if(current_depth > requested_depth + 1) {
                free(current_path);
                continue;
            }
        }

        if(strncmp(requested_path, current_path, strlen(requested_path)) != 0) {
            free(current_path);
            continue;
        }

        if((param != NULL) && (param[0] == '_')) {
            free(current_path);
            continue;
        }

        gsdm_len = (gsdm_path == NULL)? 0:strlen(gsdm_path);
        if((param != NULL) && (strcmp(param, "{i}") == 0)) {
            amxd_path_append(&current, "{i}", true);
            gsdm_path = amxd_path_get(&current, AMXD_OBJECT_TERMINATE);
            amxb_rbus_add_object(gsdm_path, object_type_table, NULL, flags, result);
        } else if((gsdm_len > 4) && (strcmp(gsdm_path + (gsdm_len - 4), "{i}.") == 0)) {
            if(((flags & AMXB_FLAG_FIRST_LVL) == AMXB_FLAG_FIRST_LVL) && (current_depth != requested_depth)) {
                param = NULL;
            }
            amxb_rbus_add_object(gsdm_path, object_type_table, param, flags, result);
        } else {
            if(((flags & AMXB_FLAG_FIRST_LVL) == AMXB_FLAG_FIRST_LVL) && (current_depth != requested_depth)) {
                param = NULL;
            }
            amxb_rbus_add_object(gsdm_path, object_type_object, param, flags, result);
        }

        free(current_path);
    }

    free(requested_path);
    amxd_path_clean(&current);
    amxd_path_clean(&temp);
    return 0;
}

static int amxb_rbus_get_data_elements(amxb_rbus_t* amxb_rbus_ctx,
                                       const char* object,
                                       amxc_llist_t* providers,
                                       uint32_t flags,
                                       amxc_var_t* result) {
    int retval = 0;
    amxd_path_t request_path;
    int count = 0;
    char** elements = NULL;

    amxd_path_init(&request_path, object);

    amxc_llist_for_each(it, providers) {
        amxc_string_t* provider = amxc_container_of(it, amxc_string_t, it);
        const char* p = amxc_string_get(provider, 0);
        retval = rbus_discoverComponentDataElements(amxb_rbus_ctx->handle, p, true, &count, &elements);

        if((retval == 0) && (count != 0)) {
            free(elements[0]);
            amxb_rbus_add_data_elements(&request_path, count, 1, elements, flags, result);
            free(elements);
        }
    }

    amxd_path_clean(&request_path);
    return retval;
}

// The object path must end with '.'
int amxb_rbus_gsdm(void* const ctx,
                   const char* object,
                   const char* search_path,
                   uint32_t flags,
                   amxc_var_t* values,
                   int timeout) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    int retval = 0;
    amxc_string_t full_path;
    amxc_llist_t providers;
    amxc_var_t* result = NULL;

    amxc_string_init(&full_path, 0);
    amxc_llist_init(&providers);

    amxc_string_setf(&full_path, "%s%s", object, search_path);
    amxc_var_set_type(values, AMXC_VAR_ID_LIST);
    result = amxc_var_add(amxc_htable_t, values, NULL);

    // apply the timeout
    amxb_rbus_set_timeout(timeout);

    // find providers
    retval = amxb_rbus_find_providers(amxc_string_get(&full_path, 0), &providers);
    when_failed(retval, exit);

    // find registered data elements
    retval = amxb_rbus_get_data_elements(amxb_rbus_ctx, amxc_string_get(&full_path, 0), &providers, flags, result);
exit:
    amxc_llist_clean(&providers, amxc_string_list_it_free);
    amxc_string_clean(&full_path);

    return retval;
}
