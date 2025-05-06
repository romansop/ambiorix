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
#include <stdlib.h>
#include <string.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>
#include <amxd/amxd_object_function.h>
#include <amxb/amxb_be.h>
#include <amxo/amxo.h>

#include "dummy_error_be.h"

#include <amxc/amxc_macros.h>

/**
 * @file A dummy bus backend that returns error on almost any operation.
 *
 * Most operations are implemented (the trivial way, i.e. return error).
 * "has" (for amxb_who_has), "connect", "disconnect" are implemented normally.
 */

typedef struct {
    amxd_dm_t remote_dm;
    amxo_parser_t parser;
    uint32_t caps;
} amxb_dummy_error_t;

static void* amxb_dummy_error_connect(UNUSED const char* host,
                                      UNUSED const char* port,
                                      UNUSED const char* path,
                                      UNUSED amxp_signal_mngr_t* sigmngr) {
    amxb_dummy_error_t* dummy_error = calloc(1, sizeof(amxb_dummy_error_t));
    when_null(dummy_error, exit);

    dummy_error->caps = AMXB_BE_DISCOVER_DESCRIBE | AMXB_BE_DISCOVER_LIST | AMXB_BE_DISCOVER | AMXB_BE_DISCOVER_RESOLVE;
    amxd_dm_init(&dummy_error->remote_dm);
    amxo_parser_init(&dummy_error->parser);

exit:
    return dummy_error;
}

static int amxb_dummy_error_disconnect(UNUSED void* ctx) {
    return 0;
}

static int amxb_dummy_error_invoke(UNUSED void* const ctx,
                                   UNUSED amxb_invoke_t* invoke_ctx,
                                   UNUSED amxc_var_t* args,
                                   UNUSED amxb_request_t* request,
                                   UNUSED int timeout) {

    return -1;
}

static void amxb_dummy_error_free(void* ctx) {
    amxb_dummy_error_t* dummy_error = (amxb_dummy_error_t*) ctx;
    when_null(dummy_error, exit);
    amxo_parser_clean(&dummy_error->parser);
    amxd_dm_clean(&dummy_error->remote_dm);
    free(dummy_error);
exit:
    return;
}

static int amxb_dummy_error_register(UNUSED void* const ctx,
                                     UNUSED amxd_dm_t* const dm) {
    return 0;
}

static bool amxb_dummy_error_has(void* const ctx,
                                 const char* object) {
    amxb_dummy_error_t* dummy_error = (amxb_dummy_error_t*) ctx;
    when_null(dummy_error, error);
    amxd_object_t* obj = amxd_dm_findf(&dummy_error->remote_dm, "%s", object);

    return obj != NULL;

error:
    return false;
}

static uint32_t amxb_dummy_error_capabilities(void* const ctx) {
    amxb_dummy_error_t* dummy_error = (amxb_dummy_error_t*) ctx;
    when_null(dummy_error, error);

    return dummy_error->caps;
error:
    return 0;
}

/** @implements @ref amxb_be_get_t */
static int amxb_dummy_error_get(UNUSED void* const ctx,
                                UNUSED const char* object,
                                UNUSED const char* search_path,
                                UNUSED int32_t depth,
                                UNUSED uint32_t access,
                                UNUSED amxc_var_t* ret,
                                UNUSED int timeout) {
    return -1;
}

/** @implements @ref amxb_be_set_t */
static int amxb_dummy_error_set(UNUSED void* const ctx,
                                UNUSED const char* object,
                                UNUSED const char* search_path,
                                UNUSED uint32_t flags,
                                UNUSED amxc_var_t* values,
                                UNUSED amxc_var_t* ovalues,
                                UNUSED uint32_t access,
                                UNUSED amxc_var_t* ret,
                                UNUSED int timeout) {
    return -1;
}

/** @implements amxb_be_add_t */
static int amxb_dummy_error_add (UNUSED void* const ctx,
                                 UNUSED const char* object,
                                 UNUSED const char* search_path,
                                 UNUSED uint32_t index,
                                 UNUSED const char* name,
                                 UNUSED amxc_var_t* values,
                                 UNUSED uint32_t access,
                                 UNUSED amxc_var_t* ret,
                                 UNUSED int timeout) {
    return -1;
}

/** @implements @ref amxb_be_async_invoke_fn_t */
static int amxb_dummy_error_async_invoke(UNUSED void* const ctx,
                                         UNUSED amxb_invoke_t* invoke_ctx,
                                         UNUSED amxc_var_t* args,
                                         UNUSED amxb_request_t* request) {
    return -1;
}

/** @implements @ref amxb_be_del_t */
static int amxb_dummy_error_del(UNUSED void* const ctx,
                                UNUSED const char* object,
                                UNUSED const char* search_path,
                                UNUSED uint32_t index,
                                UNUSED const char* name,
                                UNUSED uint32_t access,
                                UNUSED amxc_var_t* ret,
                                UNUSED int timeout) {
    return -1;
}

/** @implements @ref amxb_be_get_filtered_t */
static int amxb_dummy_error_get_filtered (UNUSED void* const ctx,
                                          UNUSED const char* object,
                                          UNUSED const char* search_path,
                                          UNUSED const char* filter,
                                          UNUSED int32_t depth,
                                          UNUSED uint32_t access,
                                          UNUSED amxc_var_t* ret,
                                          UNUSED int timeout) {
    return -1;
}

/** @implements @ref amxb_be_get_instances_t */
static int amxb_dummy_error_get_instances (UNUSED void* const ctx,
                                           UNUSED const char* object,
                                           UNUSED const char* search_path,
                                           UNUSED int32_t depth,
                                           UNUSED uint32_t access,
                                           UNUSED amxc_var_t* ret,
                                           UNUSED int timeout) {
    return -1;
}

/** @implements @ref amxb_be_get_supported_t */
static int amxb_dummy_error_get_supported (UNUSED void* const ctx,
                                           UNUSED const char* object,
                                           UNUSED const char* search_path,
                                           UNUSED uint32_t flags,
                                           UNUSED amxc_var_t* retval,
                                           UNUSED int timeout) {
    return -1;
}

/** @implements @ref amxb_be_describe_t */
static int amxb_dummy_error_describe (UNUSED void* const ctx,
                                      UNUSED const char* object,
                                      UNUSED const char* search_path,
                                      UNUSED uint32_t flags,
                                      UNUSED uint32_t access,
                                      UNUSED amxc_var_t* retval,
                                      UNUSED int timeout) {
    return -1;
}

/** @implements @ref amxb_be_list_t */
static int amxb_dummy_error_list (UNUSED void* const ctx,
                                  UNUSED const char* object,
                                  UNUSED uint32_t flags,
                                  UNUSED uint32_t access,
                                  amxb_request_t* request) {
    amxb_close_request(&request); // avoid memory leak.
    return 1;
}

/** @implements @ref amxb_be_subscribe_fn_t */
static int amxb_dummy_error_subscribe (UNUSED void* const ctx,
                                       UNUSED const char* object) {
    // return ok such that later we can unsubscribe.
    return 0;
}

/** @implements @ref amxb_be_unsubscribe_fn_t */
static int amxb_dummy_error_unsubscribe (UNUSED void* const ctx,
                                         UNUSED const char* object) {
    return 0;
}

/** @implements @ref amxb_be_wait_for_fn_t */
static int amxb_dummy_error_wait_for(UNUSED void* const ctx,
                                     UNUSED const char* object) {
    return 1;
}

/** @implements @ref amxb_be_stats_t */
static int amxb_dummy_error_get_stats (UNUSED void* const ctx,
                                       UNUSED amxc_var_t* stats) {
    amxc_var_t* operation_rx = NULL;
    amxc_var_t* rx = amxc_var_add_new_key(stats, "rx");
    amxc_var_set_type(rx, AMXC_VAR_ID_HTABLE);
    operation_rx = amxc_var_add_new_key(rx, "operation");
    amxc_var_set_type(operation_rx, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint64_t, operation_rx, "some_field_coming_from_the_bus_backend", 12345);
    return 0;
}

static amxb_be_funcs_t amxb_dummy_error_impl = {
    .connect = amxb_dummy_error_connect,
    .disconnect = amxb_dummy_error_disconnect,
    .get_fd = NULL,
    .read = NULL,
    .invoke = amxb_dummy_error_invoke,
    .wait_request = NULL,
    .close_request = NULL,
    .free = amxb_dummy_error_free,
    .register_dm = amxb_dummy_error_register,
    .has = amxb_dummy_error_has,
    .capabilities = amxb_dummy_error_capabilities,
    .get = amxb_dummy_error_get,
    .set = amxb_dummy_error_set,
    .add = amxb_dummy_error_add,
    .del = amxb_dummy_error_del,
    .get_filtered = amxb_dummy_error_get_filtered,
    .get_instances = amxb_dummy_error_get_instances,
    .get_supported = amxb_dummy_error_get_supported,
    .describe = amxb_dummy_error_describe,
    .async_invoke = amxb_dummy_error_async_invoke,
    .list = amxb_dummy_error_list,
    .subscribe = amxb_dummy_error_subscribe,
    .unsubscribe = amxb_dummy_error_unsubscribe,
    .wait_for = amxb_dummy_error_wait_for,
    .get_stats = amxb_dummy_error_get_stats,
    // name has "-" instead of "_" because the URI parser chokes on "_" in "dummy_error://"-URIs.
    .name = "dummy-error",
    .size = sizeof(amxb_be_funcs_t),
};

/**
 * When ".get_instances" is supported by the bus backend, amxb uses this instead of ".get".
 * This makes it hard to test ".get".
 * So this is a variant of the dummy bus backend without ".get_instances".
 */
static amxb_be_funcs_t amxb_dummy_error_impl_without_get_filtered = {
    .connect = amxb_dummy_error_connect,
    .disconnect = amxb_dummy_error_disconnect,
    .get_fd = NULL,
    .read = NULL,
    .invoke = amxb_dummy_error_invoke,
    .wait_request = NULL,
    .close_request = NULL,
    .free = amxb_dummy_error_free,
    .register_dm = amxb_dummy_error_register,
    .has = amxb_dummy_error_has,
    .capabilities = amxb_dummy_error_capabilities,
    .get = amxb_dummy_error_get,
    .set = amxb_dummy_error_set,
    .add = amxb_dummy_error_add,
    .del = amxb_dummy_error_del,
    .get_filtered = NULL,  // <-- no ".get_instances" because then ".get" is not used anymore.
    .get_instances = amxb_dummy_error_get_instances,
    .get_supported = amxb_dummy_error_get_supported,
    .describe = amxb_dummy_error_describe,
    .async_invoke = amxb_dummy_error_async_invoke,
    .list = amxb_dummy_error_list,
    .subscribe = amxb_dummy_error_subscribe,
    .unsubscribe = amxb_dummy_error_unsubscribe,
    .wait_for = amxb_dummy_error_wait_for,
    // name has "-" instead of "_" because the URI parser chokes on "_" in "dummy_error_without_getfiltered://"-URIs.
    .name = "dummy-error-without-getfiltered",
    .size = sizeof(amxb_be_funcs_t),
};

static amxb_version_t sup_min_lib_version = {
    .major = 2,
    .minor = 0,
    .build = -1
};

static amxb_version_t sup_max_lib_version = {
    .major = 2,
    .minor = -1,
    .build = -1
};

static amxb_version_t dummy_error_be_version = {
    .major = 0,
    .minor = 0,
    .build = 0,
};

amxb_be_info_t amxb_dummy_error_be_info = {
    .min_supported = &sup_min_lib_version,
    .max_supported = &sup_max_lib_version,
    .be_version = &dummy_error_be_version,
    .name = "dummy_error",
    .description = "AMXB Dummy Backend that returns an error for almost everything, for testing",
    .funcs = &amxb_dummy_error_impl,
};

int test_register_dummy_error_be(void) {
    return amxb_be_register(&amxb_dummy_error_impl);
}

int test_unregister_dummy_error_be(void) {
    return amxb_be_unregister(&amxb_dummy_error_impl);
}

int test_register_dummy_error_be_without_get_filtered(void) {
    return amxb_be_register(&amxb_dummy_error_impl_without_get_filtered);
}

int test_unregister_dummy_error_be_without_get_filtered(void) {
    return amxb_be_unregister(&amxb_dummy_error_impl_without_get_filtered);
}

int test_load_dummy_error_remote(amxb_bus_ctx_t* bus_ctx, const char* odl) {
    amxb_dummy_error_t* dummy_error = NULL;
    when_null(bus_ctx, error);
    // Check if called for another bus backend:
    when_false(bus_ctx->bus_fn == &amxb_dummy_error_impl || bus_ctx->bus_fn == &amxb_dummy_error_impl_without_get_filtered, error);
    dummy_error = (amxb_dummy_error_t*) bus_ctx->bus_ctx;
    when_null(dummy_error, error);

    amxd_object_t* root_obj = amxd_dm_get_root(&dummy_error->remote_dm);

    return amxo_parser_parse_file(&dummy_error->parser, odl, root_obj);
error:
    return -1;
}
