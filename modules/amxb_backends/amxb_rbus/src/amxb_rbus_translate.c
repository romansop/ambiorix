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

static amxc_array_t* paths = NULL;
static amxc_var_t* translate = NULL;

static void amxb_rbus_translate_get(const char* path, const char** requested, const char** translated) {
    amxd_path_t p;
    amxd_path_init(&p, path);
    amxb_rbus_translate_path(&p, requested, translated);
    amxd_path_clean(&p);
}

static void amxb_rbus_translate_paths(amxc_var_t* ret) {
    amxc_var_t* objects = GETI_ARG(ret, 0);
    amxc_var_t* translated_objects;
    amxc_string_t str_path;
    const char* requested = NULL;
    const char* translated = NULL;

    amxc_var_new(&translated_objects);
    amxc_var_set_type(translated_objects, amxc_var_type_of(objects));
    amxc_string_init(&str_path, 0);

    amxc_var_for_each(object, objects) {
        const char* path = amxc_var_key(object);
        if(requested == NULL) {
            amxb_rbus_translate_get(path, &requested, &translated);
            if(requested == NULL) {
                break;
            }
        }
        amxc_string_set(&str_path, path);
        amxc_string_replace(&str_path, requested, translated, 1);
        amxc_var_set_key(translated_objects, amxc_string_get(&str_path, 0), object, AMXC_VAR_FLAG_DEFAULT);
    }

    if(requested != NULL) {
        amxc_var_delete(&objects);
        amxc_var_set_index(ret, 0, translated_objects, AMXC_VAR_FLAG_DEFAULT);
    } else {
        amxc_var_delete(&translated_objects);
    }

    amxc_string_clean(&str_path);
}

static void amxb_rbus_translate_single(const amxc_var_t* data, const char* requested, const char* translated) {
    amxc_var_t* eobject = GET_ARG(data, "eobject");
    amxc_var_t* object = GET_ARG(data, "object");
    amxc_var_t* path = GET_ARG(data, "path");

    amxc_string_t str_path;
    amxc_string_init(&str_path, 0);
    if(requested != NULL) {
        amxc_string_set(&str_path, GET_CHAR(path, NULL));
        amxc_string_replace(&str_path, requested, translated, 1);
        amxc_var_set(cstring_t, path, amxc_string_get(&str_path, 0));

        amxc_string_set(&str_path, GET_CHAR(object, NULL));
        amxc_string_replace(&str_path, requested, translated, 1);
        amxc_var_set(cstring_t, object, amxc_string_get(&str_path, 0));

        amxc_string_set(&str_path, GET_CHAR(eobject, NULL));
        amxc_string_replace(&str_path, requested, translated, 1);
        amxc_var_set(cstring_t, eobject, amxc_string_get(&str_path, 0));
    }
    amxc_string_clean(&str_path);
}

static void amxb_rbus_translate_multiple(amxc_var_t* data) {
    if(amxc_var_type_of(data) == AMXC_VAR_ID_LIST) {
        const char* requested = NULL;
        const char* translated = NULL;
        amxc_var_t* path = GETP_ARG(data, "0.path");
        amxb_rbus_translate_get(GET_CHAR(path, NULL), &requested, &translated);

        if(requested != NULL) {
            amxc_var_for_each(entry, data) {
                amxb_rbus_translate_single(entry, requested, translated);
            }
        }
    } else {
        amxb_rbus_translate_data(data);
    }
}

static void amxb_rbus_translate_list(amxc_var_t* data) {
    const char* requested = NULL;
    const char* translated = NULL;
    amxc_var_t* path = GETI_ARG(data, 0);
    amxc_string_t str_path;

    amxc_string_init(&str_path, 0);
    amxb_rbus_translate_get(GET_CHAR(path, NULL), &requested, &translated);

    if(requested != NULL) {
        amxc_var_for_each(entry, data) {
            const char* object = amxc_var_constcast(cstring_t, entry);
            amxc_string_set(&str_path, object);
            amxc_string_replace(&str_path, requested, translated, 1);
            amxc_var_set(cstring_t, entry, amxc_string_get(&str_path, 0));
        }
    }

    amxc_string_clean(&str_path);
}

void amxb_rbus_translate_set_paths(void) {
    amxc_var_t* cfg_trans = amxb_rbus_get_config_option("translate");
    amxc_var_t reverse_translate;

    amxc_array_delete(&paths, NULL);

    when_null(cfg_trans, exit);
    when_false(amxc_var_type_of(cfg_trans) == AMXC_VAR_ID_HTABLE, exit);

    amxc_var_init(&reverse_translate);
    amxc_var_set_type(&reverse_translate, AMXC_VAR_ID_HTABLE);

    translate = cfg_trans;
    amxc_var_for_each(t, translate) {
        const char* key = amxc_var_key(t);
        const char* value = GET_CHAR(t, NULL);
        if(GET_ARG(translate, value) == NULL) {
            amxc_var_add_key(cstring_t, &reverse_translate, value, key);
        }
    }

    amxc_var_for_each(t, &reverse_translate) {
        amxc_var_set_key(translate, amxc_var_key(t), t, AMXC_VAR_FLAG_DEFAULT);
    }

    paths = amxc_htable_get_sorted_keys(amxc_var_constcast(amxc_htable_t, translate));

    amxc_var_clean(&reverse_translate);

exit:
    return;
}

rbusError_t amxb_rbus_translate_status2rbus(amxd_status_t status) {
    static const uint32_t error_codes[] = {
        RBUS_ERROR_SUCCESS,                         //amxd_status_ok
        RBUS_ERROR_BUS_ERROR,                       //amxd_status_unknown_error
        RBUS_ERROR_DESTINATION_NOT_FOUND,           //amxd_status_object_not_found
        RBUS_ERROR_ELEMENT_DOES_NOT_EXIST,          //amxd_status_function_not_found
        RBUS_ERROR_ELEMENT_DOES_NOT_EXIST,          //amxd_status_parameter_not_found
        RBUS_ERROR_INVALID_METHOD,                  //amxd_status_function_not_implemented
        RBUS_ERROR_INVALID_METHOD,                  //amxd_status_invalid_function
        RBUS_ERROR_INVALID_INPUT,                   //amxd_status_invalid_function_argument
        RBUS_ERROR_ELEMENT_NAME_MISSING,            //amxd_status_invalid_name
        RBUS_ERROR_INVALID_INPUT,                   //amxd_status_invalid_attr
        RBUS_ERROR_INVALID_INPUT,                   //amxd_status_invalid_value
        RBUS_ERROR_INVALID_OPERATION,               //amxd_status_invalid_action,
        RBUS_ERROR_INVALID_INPUT,                   //amxd_status_invalid_type
        RBUS_ERROR_ELEMENT_NAME_DUPLICATE,          //amxd_status_duplicate
        RBUS_ERROR_ASYNC_RESPONSE,                  //amxd_status_deferred
        RBUS_ERROR_ACCESS_NOT_ALLOWED,              //amxd_status_read_only
        RBUS_ERROR_ELEMENT_NAME_MISSING,            //amxd_status_missing_key
        RBUS_ERROR_INVALID_INPUT,                   //amxd_status_file_not_found
        RBUS_ERROR_INVALID_INPUT,                   //amxd_status_invalid_arg
        RBUS_ERROR_OUT_OF_RESOURCES,                //amxd_status_out_of_mem
        RBUS_ERROR_OUT_OF_RESOURCES,                //amxd_status_recursion
        RBUS_ERROR_INVALID_INPUT,                   //amxd_status_invalid_path
        RBUS_ERROR_INVALID_INPUT,                   //amxd_status_invalid_expr
        RBUS_ERROR_ACCESS_NOT_ALLOWED,              //amxd_status_permission_denied
        RBUS_ERROR_BUS_ERROR,                       //amxd_status_not_supported
        RBUS_ERROR_BUS_ERROR,                       //amxd_status_not_instantiated
        RBUS_ERROR_BUS_ERROR,                       //amxd_status_not_a_template
        RBUS_ERROR_TIMEOUT,                         //amxd_status_timeout
    };
    rbusError_t error = RBUS_ERROR_BUS_ERROR;
    if((error >= 0) && (error < sizeof(error_codes) / sizeof(error_codes[0]))) {
        error = error_codes[status];
    }
    return error;
}

amxd_status_t amxb_rbus_translate_rbus2status(rbusError_t error) {
    static const uint32_t status_codes[] = {
        amxd_status_ok,                         //RBUS_ERROR_SUCCESS
        amxd_status_unknown_error,              //RBUS_ERROR_BUS_ERROR
        amxd_status_invalid_value,              //RBUS_ERROR_INVALID_INPUT
        amxd_status_unknown_error,              //RBUS_ERROR_NOT_INITIALIZED
        amxd_status_out_of_mem,                 //RBUS_ERROR_OUT_OF_RESOURCES
        amxd_status_object_not_found,           //RBUS_ERROR_DESTINATION_NOT_FOUND
        amxd_status_object_not_found,           //RBUS_ERROR_DESTINATION_NOT_REACHABLE
        amxd_status_unknown_error,              //RBUS_ERROR_DESTINATION_RESPONSE_FAILURE
        amxd_status_unknown_error,              //RBUS_ERROR_INVALID_RESPONSE_FROM_DESTINATION
        amxd_status_invalid_action,             //RBUS_ERROR_INVALID_OPERATION
        amxd_status_invalid_type,               //RBUS_ERROR_INVALID_EVENT
        amxd_status_unknown_error,              //RBUS_ERROR_INVALID_HANDLE
        amxd_status_unknown_error,              //RBUS_ERROR_SESSION_ALREADY_EXIST
        amxd_status_duplicate,                  //RBUS_ERROR_COMPONENT_NAME_DUPLICATE
        amxd_status_duplicate,                  //RBUS_ERROR_ELEMENT_NAME_DUPLICATE
        amxd_status_invalid_name,               //RBUS_ERROR_ELEMENT_NAME_MISSING
        amxd_status_invalid_path,               //RBUS_ERROR_COMPONENT_DOES_NOT_EXIST
        amxd_status_invalid_path,               //RBUS_ERROR_ELEMENT_DOES_NOT_EXIST
        amxd_status_read_only,                  //RBUS_ERROR_ACCESS_NOT_ALLOWED
        amxd_status_unknown_error,              //RBUS_ERROR_INVALID_CONTEXT
        amxd_status_timeout,                    //RBUS_ERROR_TIMEOUT
        amxd_status_deferred,                   //RBUS_ERROR_ASYNC_RESPONSE
        amxd_status_invalid_function,           //RBUS_ERROR_INVALID_METHOD
        amxd_status_unknown_error,              //RBUS_ERROR_NOSUBSCRIBERS
        amxd_status_unknown_error,              //RBUS_ERROR_SUBSCRIPTION_ALREADY_EXIST
    };
    amxd_status_t status = amxd_status_unknown_error;
    if((error >= 0) && (error < sizeof(status_codes) / sizeof(status_codes[0]))) {
        status = status_codes[error];
    }
    return status;
}

void amxb_rbus_translate_path(amxd_path_t* path, const char** requested, const char** translated) {
    if(translate != NULL) {
        size_t size = amxc_array_size(paths);
        const char* param = amxd_path_get_param(path);
        amxc_string_t new_path;

        amxc_string_init(&new_path, 0);
        for(size_t i = size; i > 0; i--) {
            const char* p = (const char*) amxc_array_get_data_at(paths, i - 1);
            size_t len = strlen(p);
            if(strncmp(p, amxd_path_get(path, AMXD_OBJECT_TERMINATE), len) == 0) {
                *translated = GET_CHAR(translate, p);
                *requested = p;

                if(param != NULL) {
                    amxc_string_setf(&new_path, "%s%s", amxd_path_get(path, AMXD_OBJECT_TERMINATE), amxd_path_get_param(path));
                } else {
                    amxc_string_setf(&new_path, "%s", amxd_path_get(path, AMXD_OBJECT_TERMINATE));
                }
                amxc_string_replace(&new_path, p, *translated, 1);
                amxd_path_setf(path, false, "%s", amxc_string_get(&new_path, 0));
                break;
            }
        }
        amxc_string_clean(&new_path);
    }
}

char* amxb_rbus_translate_register_path(char* path) {
    amxd_path_t p;
    const char* requested = NULL;
    const char* translated = NULL;

    when_null(translate, exit);
    amxd_path_init(&p, path);
    amxb_rbus_translate_path(&p, &requested, &translated);
    free(path);
    path = strdup(amxd_path_get(&p, AMXD_OBJECT_TERMINATE));
    amxd_path_clean(&p);

exit:
    return path;
}

int amxb_rbus_call_needs_translation(const char* method) {
    int retval = AMXB_RBUS_TRANSLATE_NONE;

    if(method[0] == '_') {
        if((strcmp(method, "_get") == 0) ||
           (strcmp(method, "_set") == 0) ||
           (strcmp(method, "_get_supported") == 0) ||
           (strcmp(method, "_get_instances") == 0)) {
            retval = AMXB_RBUS_TRANSLATE_PATHS;
        } else if(strcmp(method, "_add") == 0) {
            retval = AMXB_RBUS_TRANSLATE_MULTIPLE;
        } else if(strcmp(method, "_del") == 0) {
            retval = AMXB_RBUS_TRANSLATE_LIST;
        } else if(strcmp(method, "_describe") == 0) {
            retval = AMXB_RBUS_TRANSLATE_DATA;
        }
    }

    return retval;
}

void amxb_rbus_translate_data(const amxc_var_t* data) {
    amxc_var_t* path = GET_ARG(data, "path");
    const char* requested = NULL;
    const char* translated = NULL;

    amxb_rbus_translate_get(GET_CHAR(path, NULL), &requested, &translated);
    amxb_rbus_translate_single(data, requested, translated);
}

void amxb_rbus_call_translate(amxc_var_t* out, int funcid) {
    switch(funcid) {
    default:
    case AMXB_RBUS_TRANSLATE_NONE:
        break;
    case AMXB_RBUS_TRANSLATE_PATHS:
        amxb_rbus_translate_paths(out);
        break;
    case AMXB_RBUS_TRANSLATE_MULTIPLE:
        amxb_rbus_translate_multiple(GETI_ARG(out, 0));
        break;
    case AMXB_RBUS_TRANSLATE_LIST:
        amxb_rbus_translate_list(GETI_ARG(out, 0));
        break;
    case AMXB_RBUS_TRANSLATE_DATA:
        amxb_rbus_translate_data(GETI_ARG(out, 0));
        break;
    }
}

DESTRUCTOR static void amxb_rbus_cleanup_translation_keys(void) {
    amxc_array_delete(&paths, NULL);
}
