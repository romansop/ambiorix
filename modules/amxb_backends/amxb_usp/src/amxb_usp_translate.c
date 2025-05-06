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

#include "amxb_usp.h"

static amxc_array_t* paths = NULL;
static amxc_var_t* translate = NULL;

static void amxb_usp_translate_get(const char* path, const char** requested, const char** translated) {
    amxd_path_t p;
    amxd_path_init(&p, path);
    amxb_usp_translate_path(&p, requested, translated);
    amxd_path_clean(&p);
}

static void amxb_usp_translate_paths(amxc_var_t* ret) {
    amxc_var_t* translated_objects = NULL;
    amxc_string_t str_path;
    const char* requested = NULL;
    const char* translated = NULL;

    amxc_var_new(&translated_objects);
    amxc_var_set_type(translated_objects, amxc_var_type_of(ret));
    amxc_string_init(&str_path, 0);

    amxc_var_for_each(object, ret) {
        const char* path = amxc_var_key(object);
        if(requested == NULL) {
            amxb_usp_translate_get(path, &requested, &translated);
            if(requested == NULL) {
                break;
            }
        }
        amxc_string_set(&str_path, path);
        amxc_string_replace(&str_path, requested, translated, 1);
        amxc_var_set_key(translated_objects, amxc_string_get(&str_path, 0), object, AMXC_VAR_FLAG_DEFAULT);
    }

    if(requested != NULL) {
        amxc_var_move(ret, translated_objects);
    }

    amxc_var_delete(&translated_objects);
    amxc_string_clean(&str_path);
}

static void amxb_usp_translate_single(const amxc_var_t* data, const char* requested, const char* translated) {
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

static void amxb_usp_translate_multiple(amxc_var_t* data) {
    if(amxc_var_type_of(data) == AMXC_VAR_ID_LIST) {
        const char* requested = NULL;
        const char* translated = NULL;
        amxc_var_t* path = GETP_ARG(data, "0.path");
        amxb_usp_translate_get(GET_CHAR(path, NULL), &requested, &translated);

        if(requested != NULL) {
            amxc_var_for_each(entry, data) {
                amxb_usp_translate_single(entry, requested, translated);
            }
        }
    } else {
        amxb_usp_translate_data(data);
    }
}

static void amxb_usp_translate_list(amxc_var_t* data) {
    const char* requested = NULL;
    const char* translated = NULL;
    amxc_var_t* path = GETI_ARG(data, 0);
    amxc_string_t str_path;

    amxc_string_init(&str_path, 0);
    amxb_usp_translate_get(GET_CHAR(path, NULL), &requested, &translated);

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

void amxb_usp_translate_set_paths(void) {
    amxc_var_t* cfg_trans = amxb_usp_get_config_option("translate");
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

void amxb_usp_translate_path(amxd_path_t* path, const char** requested, const char** translated) {
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

void amxb_usp_translate_register_path(amxd_path_t* path) {
    const char* requested = NULL;
    const char* translated = NULL;

    when_null(translate, exit);
    when_null(path, exit);

    amxb_usp_translate_path(path, &requested, &translated);

exit:
    return;
}

void amxb_usp_translate_data(const amxc_var_t* data) {
    amxc_var_t* path = GET_ARG(data, "path");
    const char* requested = NULL;
    const char* translated = NULL;

    amxb_usp_translate_get(GET_CHAR(path, NULL), &requested, &translated);
    amxb_usp_translate_single(data, requested, translated);
}

void amxb_usp_call_translate(amxc_var_t* out, int funcid) {
    switch(funcid) {
    case AMXB_USP_TRANSLATE_NONE:
        break;
    case AMXB_USP_TRANSLATE_PATHS:
        amxb_usp_translate_paths(out);
        break;
    case AMXB_USP_TRANSLATE_MULTIPLE:
        amxb_usp_translate_multiple(out);
        break;
    case AMXB_USP_TRANSLATE_LIST:
        amxb_usp_translate_list(out);
        break;
    case AMXB_USP_TRANSLATE_DATA:
        amxb_usp_translate_data(out);
        break;
    }
}

DESTRUCTOR static void amxb_usp_cleanup_translation_keys(void) {
    amxc_array_delete(&paths, NULL);
}
