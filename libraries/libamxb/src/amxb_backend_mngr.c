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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>

#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>
#include <amxd/amxd_path.h>

#include <amxb/amxb_be_intf.h>
#include <amxb/amxb.h>

#include "amxb_priv.h"
#include "amxb_version.h"

typedef struct _amxb_who_has_cache {
    amxc_htable_it_t hit;
    amxc_llist_it_t lit;
    amxb_bus_ctx_t* ctx;
} amxb_who_has_cache_t;

static amxc_htable_t amxb_backends;
static amxc_var_t load_order;
static amxb_be_funcs_t* current_be = NULL;
static const amxc_var_t* amxb_conf = NULL;

static uint32_t amxb_cache_size = 10;
static amxc_llist_t amxb_cache_list;
static amxc_htable_t amxb_cache_table;

static amxb_version_t lib_version = {
    .major = AMXB_VERSION_MAJOR,
    .minor = AMXB_VERSION_MINOR,
    .build = AMXB_VERSION_BUILD
};

static int amxb_close_backend(UNUSED amxb_be_funcs_t* fns) {
    void* handle = fns->handle;
    char* no_dlclose = getenv("AMXB_NO_DLCLOSE");
    int retval = 0;

    fns->handle = NULL;
    if(no_dlclose == NULL) {
        retval = dlclose(handle);
        if(retval != 0) {
            const char* errstr = dlerror();
            printf("dlclose failed - %s", errstr != NULL? errstr:"no error");
        }
    }

    return retval;
}

static void amxb_be_remove_connections(amxc_llist_it_t* lit) {
    amxb_bus_ctx_t* bus_ctx = amxc_llist_it_get_data(lit, amxb_bus_ctx_t, it);
    amxb_free(&bus_ctx);
}

static void amxb_be_remove_backend(UNUSED const char* key,
                                   amxc_htable_it_t* it) {
    amxb_be_funcs_t* fns = NULL;

    fns = amxc_htable_it_get_data(it, amxb_be_funcs_t, it);
    amxc_llist_clean(&fns->connections, amxb_be_remove_connections);
    if(fns->handle != NULL) {
        amxb_close_backend(fns);
    }
}

static int amxb_be_invoke_on_all_connections(amxc_llist_t* connections,
                                             amxb_be_task_fn_t fn,
                                             const amxc_var_t* args,
                                             uint32_t type,
                                             void* priv) {
    int retval = 0;

    amxc_llist_for_each(it, connections) {
        amxb_bus_ctx_t* ctx = amxc_llist_it_get_data(it, amxb_bus_ctx_t, it);
        if(ctx->socket_type != type) {
            continue;
        }
        retval = fn(ctx, args, priv);
        when_failed(retval, exit);
    }

exit:
    return retval;
}

static bool amxb_has_object(amxb_bus_ctx_t* bus_ctx, const char* object, bool full_match) {
    bool retval = false;
    const amxb_be_funcs_t* fns = NULL;

    fns = bus_ctx->bus_fn;
    if(amxb_is_valid_be_func(fns, has, fns->has)) {
        if(full_match) {
            bus_ctx->stats->counter_tx_has++;
            retval = fns->has(bus_ctx->bus_ctx, object);
        } else {
            amxd_path_t path;
            char* part = NULL;

            amxd_path_init(&path, NULL);
            amxd_path_setf(&path, true, "%s", object);

            do {
                free(part);
                object = amxd_path_get(&path, 0);
                bus_ctx->stats->counter_tx_has++;
                retval = fns->has(bus_ctx->bus_ctx, object);
                part = amxd_path_get_last(&path, true);
            } while(part != NULL && !retval);
            free(part);
            amxd_path_clean(&path);
        }
    }

    return retval;
}

static void amxb_be_object_exists(UNUSED const amxb_bus_ctx_t* bus_ctx,
                                  const amxc_var_t* const data,
                                  void* priv) {
    bool* found = (bool*) priv;

    if(data != NULL) {
        *found = true;
    }
}

static amxb_bus_ctx_t* amxb_be_find_connection(amxc_llist_t* connections,
                                               const char* object_path,
                                               bool full_match) {
    amxb_bus_ctx_t* ctx = NULL;
    amxc_var_t data;
    bool found = false;
    amxd_path_t path;
    char* fixed_path = NULL;
    uint32_t caps = 0;

    amxd_path_init(&path, NULL);
    amxc_var_init(&data);
    amxd_path_setf(&path, true, "%s", object_path);
    fixed_path = amxd_path_get_fixed_part(&path, false);

    // remove the trailing dot if any
    if((fixed_path != NULL) && (*fixed_path != 0)) {
        int len = strlen(fixed_path);
        if(fixed_path[len - 1] == '.') {
            fixed_path[len - 1] = 0;
        }
    }

    amxc_llist_for_each(it, connections) {
        ctx = amxc_llist_it_get_data(it, amxb_bus_ctx_t, it);
        if(ctx->socket_type == AMXB_LISTEN_SOCK) {
            ctx = NULL;
            continue;
        }
        if(amxb_is_local_object(ctx, fixed_path)) {
            break;
        }
        caps = amxb_be_get_capabilities(ctx);
        if(((caps & AMXB_BE_DISCOVER) == AMXB_BE_DISCOVER) &&
           amxb_has_object(ctx, fixed_path, full_match)) {
            break;
        }
        if(((caps & AMXB_BE_DISCOVER_DESCRIBE) == AMXB_BE_DISCOVER_DESCRIBE) &&
           ( amxb_describe(ctx, fixed_path, AMXB_FLAG_EXISTS, &data, amxb_get_internal_timeout()) == 0)) {
            if(amxc_var_is_null(&data)) {
                ctx = NULL;
                continue;
            }
            break;
        }
        if((caps & AMXB_BE_DISCOVER_LIST) == AMXB_BE_DISCOVER_LIST) {
            amxb_list(ctx, fixed_path, AMXB_FLAG_FIRST_LVL | AMXB_FLAG_EXISTS,
                      amxb_be_object_exists, &found);
            if(found) {
                break;
            }
        }
        ctx = NULL;
    }

    free(fixed_path);
    amxd_path_clean(&path);
    amxc_var_clean(&data);
    return ctx;
}

static bool amxb_be_contains(amxc_var_t* list, const char* be_name) {
    bool found = false;
    when_str_empty(be_name, exit);
    when_null(list, exit);

    amxc_var_for_each(be, list) {
        if(amxc_var_type_of(be) != AMXC_VAR_ID_CSTRING) {
            amxc_var_delete(&be);
            continue;
        }
        if(strcmp(be_name, GET_CHAR(be, NULL)) == 0) {
            found = true;
            break;
        }
    }

exit:
    return found;
}

static void amxb_apply_be_config(amxb_be_funcs_t* funcs,
                                 amxc_var_t* const backend_order,
                                 amxc_var_t* const config) {
    if(amxb_is_valid_be_func(funcs, set_config, funcs->set_config)) {
        funcs->set_config(config);
    }
    if(!amxb_be_contains(backend_order, funcs->name)) {
        amxc_var_add(cstring_t, backend_order, funcs->name);
    }
}

static amxb_bus_ctx_t* amxb_be_try_find(const char* object_path, bool full_match) {
    amxb_bus_ctx_t* ctx = NULL;
    amxc_var_t* backend_order = amxc_var_get_key(amxb_conf, "backend-order", AMXC_VAR_FLAG_DEFAULT);

    // go over the backends in the defined order
    if(backend_order != NULL) {
        amxc_var_for_each(be_name, backend_order) {
            amxb_be_funcs_t* funcs = NULL;
            amxc_htable_it_t* hit = amxc_htable_get(&amxb_backends, GET_CHAR(be_name, NULL));
            if(hit == NULL) {
                if(amxc_var_type_of(be_name) != AMXC_VAR_ID_CSTRING) {
                    // remove invalid entries
                    amxc_var_delete(&be_name);
                }
                continue;
            }

            funcs = amxc_container_of(hit, amxb_be_funcs_t, it);
            // try to find a bus context (connection) that can provide the path
            ctx = amxb_be_find_connection(&funcs->connections, object_path, full_match);
            if(ctx != NULL) {
                break;
            }
        }
    }

    if(ctx == NULL) {
        // fallback for backwards compatibility (when no backend_order is available)
        amxc_htable_for_each(hit, &amxb_backends) {
            amxb_be_funcs_t* funcs = NULL;
            funcs = amxc_container_of(hit, amxb_be_funcs_t, it);
            // try to find a bus context (connection) that can provide the path
            ctx = amxb_be_find_connection(&funcs->connections, object_path, full_match);
            if(ctx != NULL) {
                break;
            }
        }
    }

    return ctx;
}

// returns 0 when lib version and be version are equal
//         1 when lib version is > then be version
//        -1 when lib version is < then be version
//
// -1 can be used as a wildcard in the be version
// if major be version is -1 all lib version match
// if minor be version is -1 only checks major versions
// if build be version is -1 only major and minor versions are checked
int amxb_check_version(const amxb_version_t* be_version) {
    if(lib_version.major > be_version->major) {
        return 1;
    }
    if(lib_version.major < be_version->major) {
        return -1;
    }
    if(be_version->minor == -1) {
        return 0;
    }
    if(lib_version.minor > be_version->minor) {
        return 1;
    }
    if(lib_version.minor < be_version->minor) {
        return -1;
    }
    if(be_version->build == -1) {
        return 0;
    }
    if(lib_version.build > be_version->build) {
        return 1;
    }
    if(lib_version.build < be_version->build) {
        return -1;
    }
    return 0;
}

int amxb_check_be_versions(const amxb_version_t* min,
                           const amxb_version_t* max) {
    if((min->major == -1) || (max->major == -1)) {
        return -1;
    }

    if(min->major > max->major) {
        return -1;
    }
    if(min->major == max->major) {
        if((min->minor > max->minor) && (max->minor != -1)) {
            return -1;
        }
        if(min->minor == max->minor) {
            if((min->build > max->build) && (max->build != -1)) {
                return -1;
            }
        }
    }

    return 0;
}

const amxb_version_t* amxb_get_version(void) {
    static amxb_version_t libv;

    libv.major = lib_version.major;
    libv.minor = lib_version.minor;
    libv.build = lib_version.build;

    return &libv;
}

int amxb_be_register(amxb_be_funcs_t* const funcs) {
    int retval = -1;
    amxc_htable_it_t* hit = NULL;

    when_null(funcs, exit);
    if((funcs->name == NULL) || (*(funcs->name) == 0)) {
        printf("amxb_be_register() ERROR: amxb_be_funcs_t->name=NULL\n");
        goto exit;
    }
    if(funcs->size > sizeof(amxb_be_funcs_t)) {
        printf("amxb_be_register() ERROR: amxb_be_funcs->size > sizeof(amxb_be_funcs_t)\n");
        goto exit;
    }

    if(amxc_htable_is_empty(&amxb_backends)) {
        when_failed(amxc_htable_init(&amxb_backends, 10), exit);
    } else {
        hit = amxc_htable_get(&amxb_backends, funcs->name);
        if(hit != NULL) {
            printf("amxb_be_register() ERROR: %s backend is already registered: Is it declared twice in /usr/bin/mods/amxb/ ?\n", funcs->name);
            goto exit;
        }
    }

    current_be = funcs;
    funcs->handle = NULL;

    amxc_htable_it_init(&funcs->it);
    amxc_llist_init(&funcs->connections);
    when_failed(amxc_htable_insert(&amxb_backends, funcs->name, &funcs->it), exit);

    if(amxb_conf != NULL) {
        amxc_var_t* cfg = amxc_var_get_key(amxb_conf, funcs->name, AMXC_VAR_FLAG_DEFAULT);
        amxc_var_t* backend_order = amxc_var_get_key(amxb_conf, "backend-order", AMXC_VAR_FLAG_DEFAULT);
        amxb_apply_be_config(funcs, backend_order, cfg);
    } else {
        amxc_var_add(cstring_t, &load_order, funcs->name);
    }

    retval = 0;

exit:
    return retval;
}

int amxb_be_unregister(amxb_be_funcs_t* const funcs) {
    int retval = -1;
    amxc_htable_it_t* hit = NULL;

    when_null(funcs, exit);
    when_null(funcs->name, exit);
    when_true(*(funcs->name) == 0, exit);

    hit = amxc_htable_get(&amxb_backends, funcs->name);

    when_null(hit, exit);
    when_true(hit != &funcs->it, exit);

    amxc_htable_it_clean(hit, amxb_be_remove_backend);

    funcs->handle = NULL;

    retval = 0;

exit:
    if(amxc_htable_is_empty(&amxb_backends)) {
        amxc_htable_clean(&amxb_backends, NULL);
        amxc_var_clean(&load_order);
        amxc_var_set_type(&load_order, AMXC_VAR_ID_LIST);
    }
    return retval;
}

amxb_be_funcs_t* amxb_be_find(const char* name) {
    amxb_be_funcs_t* fns = NULL;
    amxc_htable_it_t* hit = NULL;

    when_null(name, exit);
    when_true(*(name) == 0, exit);

    hit = amxc_htable_get(&amxb_backends, name);
    when_null(hit, exit);

    fns = amxc_htable_it_get_data(hit, amxb_be_funcs_t, it);

exit:
    return fns;
}

const amxb_be_info_t* amxb_be_get_info(const char* name) {
    amxb_be_funcs_t* fns = NULL;
    amxb_be_info_fn_t be_get_info = NULL;
    amxb_be_info_t* info = NULL;
    amxc_htable_it_t* hit = NULL;

    when_null(name, exit);
    when_true(*(name) == 0, exit);

    hit = amxc_htable_get(&amxb_backends, name);
    when_null(hit, exit);

    fns = amxc_htable_it_get_data(hit, amxb_be_funcs_t, it);

    be_get_info = (amxb_be_info_fn_t) dlsym(fns->handle, "amxb_be_info");
    when_null(be_get_info, exit);
    info = be_get_info();

exit:
    return info;
}

int amxb_be_load(const char* path_name) {
    int retval = -1;
    void* handle = NULL;
    amxb_be_info_fn_t be_get_info = NULL;
    amxb_be_info_t* be_info = NULL;
    when_str_empty(path_name, exit);

    // Set current backend functions to NULL.
    // If the loaded shared object has a construct that calls amxb_be_register
    // the current_be will be set correctly.
    // This happends during the dlopen
    current_be = NULL;
    handle = dlopen(path_name, RTLD_LAZY);
    if(handle == NULL) {
        const char* errstr = dlerror();
        printf("amxb_be_load(): DLOPEN(%s) - %s\n", path_name, errstr != NULL? errstr:"no error");
        goto exit;
    }

    be_get_info = (amxb_be_info_fn_t) dlsym(handle, "amxb_be_info");
    when_null(be_get_info, exit);

    be_info = be_get_info();
    when_null(be_info, exit);
    when_null(be_info->min_supported, exit);
    when_null(be_info->max_supported, exit);

    when_failed(amxb_check_be_versions(be_info->min_supported,
                                       be_info->max_supported), exit);

    // Development versions of the library can container version 0.0.0
    // Accept the backend if the version is 0.0.0, otherwise
    // theck that the back-end version is valid
    if((lib_version.major != 0) ||
       (lib_version.minor != 0) ||
       (lib_version.build != 0)) {
        if(amxb_check_version(be_info->min_supported) < 0) {
            printf("amxb_be_load() ERROR: libamxb version is too low\n");
            goto exit;
        }
        if(amxb_check_version(be_info->max_supported) > 0) {
            printf("amxb_be_load() ERROR: libamxb version is too high\n");
            goto exit;
        }
    }

    // The loaded back-end could call amxb_be_register using a constructor
    // function. This method is not always reliable, depending on the platform
    // the constructor is not invoked.
    // Or the back-end did not call amxb_be_register function

    // Prefered solution is that the backend gives the function table using
    // the back-end information structure. The following lines are here for
    // backwards compatibility.
    if(current_be == NULL) {
        when_null(be_info->funcs, exit);
        if(be_info->funcs->handle == handle) {
            dlclose(handle);
        } else if(be_info->funcs->handle != NULL) {
            goto exit;
        } else {
            if(amxb_be_register(be_info->funcs) != 0) {
                printf("amxb_be_load() ERROR: Failed to register %s backend\n", be_info->name);
                goto exit;
            }
        }
    } else {
        be_info->funcs = current_be;
    }

    be_info->funcs->handle = handle;

    retval = 0;

exit:
    if((retval != 0) && (handle != NULL)) {
        printf("amxb_be_load(): amxb version is %d.%d.%d\n",
               lib_version.major, lib_version.minor, lib_version.build);
        if((be_info != NULL) && (be_info->min_supported != NULL) && (be_info->max_supported != NULL)) {
            const amxb_version_t* min = be_info->min_supported;
            const amxb_version_t* max = be_info->max_supported;
            printf("amxb_be_load(): %s min amxb version: %d.%d.%d\n",
                   be_info->name, min->major, min->minor, min->build);
            printf("amxb_be_load(): %s max amxb version: %d.%d.%d\n",
                   be_info->name, max->major, max->minor, max->build);
        } else {
            printf("amxb_be_load() ERROR: Failed to get backend info from %s\n", path_name);
        }
        dlclose(handle);
    }
    current_be = NULL;
    return retval;
}

int amxb_be_load_multiple(amxc_var_t* const bes) {
    int retval = -1;
    const amxc_llist_t* list = NULL;
    amxc_llist_t list_bes;

    amxc_llist_init(&list_bes);

    when_null(bes, exit);
    when_true(amxc_var_type_of(bes) != AMXC_VAR_ID_LIST &&
              amxc_var_type_of(bes) != AMXC_VAR_ID_CSTRING, exit);


    if(amxc_var_type_of(bes) == AMXC_VAR_ID_CSTRING) {
        amxc_string_t* str_bes = amxc_var_take(amxc_string_t, bes);
        when_null(str_bes, exit);
        amxc_string_split_to_llist(str_bes, &list_bes, ':');
        amxc_var_clean(bes);
        amxc_var_set_type(bes, AMXC_VAR_ID_LIST);
        amxc_llist_for_each(it, (&list_bes)) {
            amxc_string_t* str = amxc_string_from_llist_it(it);
            amxc_var_add(cstring_t, bes, amxc_string_get(str, 0));
        }
        amxc_string_delete(&str_bes);
    }
    list = amxc_var_constcast(amxc_llist_t, bes);

    when_true(amxc_llist_is_empty(list), exit);
    retval = 0;
    amxc_llist_for_each(it, list) {
        amxc_var_t* be = amxc_var_from_llist_it(it);
        if(amxb_be_load(amxc_var_constcast(cstring_t, be)) == 0) {
            amxc_var_delete(&be);
        } else {
            retval++;
        }
    }

exit:
    amxc_llist_clean(&list_bes, amxc_string_list_it_free);
    return retval;
}

int amxb_be_remove(const char* backend_name) {
    amxb_be_funcs_t* fns = NULL;
    amxc_htable_it_t* hit = NULL;
    int retval = -1;

    when_str_empty(backend_name, exit);

    hit = amxc_htable_get(&amxb_backends, backend_name);
    when_null(hit, exit);

    fns = amxc_htable_it_get_data(hit, amxb_be_funcs_t, it);
    when_null(fns->handle, exit);
    amxc_llist_clean(&fns->connections, amxb_be_remove_connections);
    amxc_htable_it_clean(hit, NULL);

    retval = amxb_close_backend(fns);

exit:
    if(amxc_htable_is_empty(&amxb_backends)) {
        amxc_htable_clean(&amxb_backends, NULL);
        amxc_var_clean(&load_order);
        amxc_var_set_type(&load_order, AMXC_VAR_ID_LIST);
    }
    return retval;
}

void amxb_be_remove_all(void) {
    amxc_htable_clean(&amxb_backends, amxb_be_remove_backend);
    amxc_var_clean(&load_order);
    amxc_var_set_type(&load_order, AMXC_VAR_ID_LIST);
    amxb_conf = NULL;
}

amxc_array_t* amxb_be_list(void) {
    return amxc_htable_get_sorted_keys(&amxb_backends);
}

int amxb_be_for_all_connections(amxb_be_task_fn_t fn,
                                const amxc_var_t* args,
                                void* priv) {
    int retval = AMXB_ERROR_UNKNOWN;
    when_null(fn, exit);

    amxc_htable_for_each(hit, &amxb_backends) {
        amxb_be_funcs_t* funcs = amxc_htable_it_get_data(hit, amxb_be_funcs_t, it);
        if(amxb_be_invoke_on_all_connections(&funcs->connections,
                                             fn,
                                             args,
                                             AMXB_DATA_SOCK,
                                             priv) == AMXB_STATUS_OK) {
            retval = AMXB_STATUS_OK;
        }
    }

exit:
    return retval;
}

int amxb_be_for_all_listeners(amxb_be_task_fn_t fn,
                              const amxc_var_t* args,
                              void* priv) {
    int retval = 0;
    when_null(fn, exit);

    amxc_htable_for_each(hit, &amxb_backends) {
        amxb_be_funcs_t* funcs = amxc_htable_it_get_data(hit, amxb_be_funcs_t, it);
        retval = amxb_be_invoke_on_all_connections(&funcs->connections,
                                                   fn,
                                                   args,
                                                   AMXB_LISTEN_SOCK,
                                                   priv);
    }

exit:
    return retval;
}

amxb_bus_ctx_t* amxb_be_who_has(const char* object_path) {
    return amxb_be_who_has_ex(object_path, false);
}

amxb_bus_ctx_t* amxb_be_who_has_ex(const char* object_path, bool full_match) {
    amxb_bus_ctx_t* ctx = NULL;
    amxc_htable_it_t* cache_hit = NULL;
    amxb_who_has_cache_t* cache = NULL;

    when_str_empty(object_path, exit);
    when_true(strcmp(object_path, ".") == 0, exit);

    // first chech if an entry if found in the cache
    cache_hit = amxc_htable_get(&amxb_cache_table, object_path);
    if(cache_hit != NULL) {
        // entry found
        cache = amxc_container_of(cache_hit, amxb_who_has_cache_t, hit);
        ctx = cache->ctx;
        // move it to the front
        amxc_llist_prepend(&amxb_cache_list, &cache->lit);
        // and leave
        goto exit;
    }

    ctx = amxb_be_try_find(object_path, full_match);
    when_null(ctx, exit);
    when_true(amxb_cache_size == 0, exit);

    // other wise add the entry to the cache
    if(amxc_htable_size(&amxb_cache_table) >= amxb_cache_size) {
        // max cache size has been reached, remove the oldest entry
        // this will be the last one
        amxc_llist_it_t* it = amxc_llist_take_last(&amxb_cache_list);
        if(it != NULL) {
            cache = amxc_container_of(it, amxb_who_has_cache_t, lit);
            amxc_htable_it_clean(&cache->hit, NULL);
            free(cache);
        }
    }

    // add a new cache entry to the front.
    cache = (amxb_who_has_cache_t*) calloc(1, sizeof(amxb_who_has_cache_t));
    when_null(cache, exit);
    cache->ctx = ctx;
    amxc_htable_insert(&amxb_cache_table, object_path, &cache->hit);
    amxc_llist_prepend(&amxb_cache_list, &cache->lit);

exit:
    return ctx;
}

void amxb_be_cache_remove_ctx(amxb_bus_ctx_t* ctx) {
    when_null(ctx, exit);

    amxc_llist_for_each(lit, &amxb_cache_list) {
        amxb_who_has_cache_t* cache = amxc_container_of(lit, amxb_who_has_cache_t, lit);
        if(cache->ctx == ctx) {
            amxc_llist_it_take(&cache->lit);
            amxc_htable_it_clean(&cache->hit, NULL);
            free(cache);
        }
    }

exit:
    return;
}

void amxb_be_cache_remove_path(const char* object_path) {
    amxc_htable_it_t* cache_hit = NULL;
    amxb_who_has_cache_t* cache = NULL;

    when_str_empty(object_path, exit);
    when_true(strcmp(object_path, ".") == 0, exit);

    cache_hit = amxc_htable_get(&amxb_cache_table, object_path);
    when_null(cache_hit, exit);

    cache = amxc_container_of(cache_hit, amxb_who_has_cache_t, hit);
    amxc_llist_it_take(&cache->lit);
    amxc_htable_it_clean(&cache->hit, NULL);
    free(cache);

exit:
    return;
}

void amxb_be_cache_set_size(uint32_t size) {
    uint32_t items = amxc_htable_size(&amxb_cache_table);
    if(size < items) {
        amxc_llist_for_each_reverse(it, &amxb_cache_list) {
            amxb_who_has_cache_t* cache = amxc_container_of(it, amxb_who_has_cache_t, lit);
            amxc_llist_it_take(&cache->lit);
            amxc_htable_it_clean(&cache->hit, NULL);
            free(cache);
            items--;
            if(size >= items) {
                break;
            }
        }
    }

    amxb_cache_size = size;
}

int amxb_set_config(amxc_var_t* const config) {
    amxb_conf = config;
    if(config == NULL) {
        amxc_htable_for_each(hit, &amxb_backends) {
            amxb_be_funcs_t* funcs = amxc_htable_it_get_data(hit, amxb_be_funcs_t, it);
            amxb_apply_be_config(funcs, NULL, NULL);
        }
    } else {
        amxc_var_t* backend_order = GET_ARG(amxb_conf, "backend-order");
        if(backend_order == NULL) {
            backend_order = amxc_var_add_key(amxc_llist_t, config, "backend-order", NULL);
        }
        amxc_var_for_each(name, &load_order) {
            amxc_htable_it_t* hit = amxc_htable_get(&amxb_backends, GET_CHAR(name, NULL));
            amxb_be_funcs_t* funcs = amxc_htable_it_get_data(hit, amxb_be_funcs_t, it);
            amxc_var_t* be_config = GET_ARG(config, GET_CHAR(name, NULL));
            if(be_config == NULL) {
                be_config = amxc_var_add_key(amxc_htable_t, config, GET_CHAR(name, NULL), NULL);
            }
            amxb_apply_be_config(funcs, backend_order, be_config);
        }
    }

    return 0;
}

char* amxb_be_name_from_uri(const char* uri) {
    int retval = -1;
    char* scheme = NULL;
    char* host = NULL;
    char* port = NULL;
    char* path = NULL;

    when_str_empty(uri, exit);

    retval = amxb_uri_parse(uri, &scheme, &host, &port, &path);
    when_failed(retval, exit);
    if(amxc_htable_get(&amxb_backends, scheme) == NULL) {
        free(scheme);
        scheme = NULL;
    }

exit:
    free(host);
    free(port);
    free(path);
    return scheme;
}

CONSTRUCTOR static void amxb_cache_init(void) {
    amxc_htable_init(&amxb_cache_table, amxb_cache_size * 2);
    amxc_var_init(&load_order);
    amxc_var_set_type(&load_order, AMXC_VAR_ID_LIST);
}

DESTRUCTOR static void amxb_cache_clean(void) {
    amxc_htable_clean(&amxb_cache_table, NULL);
    amxc_var_clean(&load_order);
}
