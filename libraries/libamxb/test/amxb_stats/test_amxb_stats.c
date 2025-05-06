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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <signal.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>

#include <amxd/amxd_dm.h>
#include <amxd/amxd_path.h>

#include <amxb/amxb.h>
#include <amxb/amxb_register.h>
#include <amxb/amxb_be_intf.h>
#include <amxb/amxb_stats.h>

#include <amxo/amxo.h>

#include "dummy_be.h"
#include "dummy_error_be.h"
#include "test_amxb_stats.h"

#include <amxc/amxc_macros.h>
static amxb_bus_ctx_t* bus_ctx_normal = NULL;
static amxb_bus_ctx_t* bus_ctx_error = NULL;
static amxb_bus_ctx_t* bus_ctx_error_without_get_filtered = NULL;
static amxd_dm_t local_dm;
static amxo_parser_t parser;


static void s_assert_field(amxc_var_t* var, const char* field, uint64_t expected_value) {
    assert_non_null(GETP_ARG(var, field));
    assert_int_equal(GETP_UINT32(var, field), expected_value);
}

static void s_assert_stats(amxb_bus_ctx_t* bus_ctx, const char* field, uint64_t expected_value) {
    amxc_var_t stats;
    amxc_var_init(&stats);
    amxb_stats_get(bus_ctx, &stats);
    s_assert_field(&stats, field, expected_value);
    amxc_var_clean(&stats);
}

static void s_assert_stats_all_zero(amxb_bus_ctx_t* bus_ctx) {
    amxc_var_t stats;
    amxc_var_init(&stats);
    amxb_stats_get(bus_ctx, &stats);
    s_assert_field(&stats, "tx.operation.add", 0);
    s_assert_field(&stats, "tx.operation.async_invoke", 0);
    s_assert_field(&stats, "tx.operation.del", 0);
    s_assert_field(&stats, "tx.operation.describe", 0);
    s_assert_field(&stats, "tx.operation.get_filtered", 0);
    s_assert_field(&stats, "tx.operation.get_instances", 0);
    s_assert_field(&stats, "tx.operation.get_supported", 0);
    s_assert_field(&stats, "tx.operation.has", 0);
    s_assert_field(&stats, "tx.operation.invoke", 0);
    s_assert_field(&stats, "tx.operation.list", 0);
    s_assert_field(&stats, "tx.operation.subscribe", 0);
    s_assert_field(&stats, "tx.operation.unsubscribe", 0);
    s_assert_field(&stats, "tx.operation.wait_for", 0);
    amxc_var_clean(&stats);
}

int test_amxb_stats_setup(UNUSED void** state) {
    amxd_dm_init(&local_dm);
    amxo_parser_init(&parser);

    test_register_dummy_be();
    test_register_dummy_error_be();
    test_register_dummy_error_be_without_get_filtered();

    assert_int_equal(amxb_connect(&bus_ctx_normal, "dummy:/tmp/dummy.sock"), 0);
    test_load_dummy_remote("./remote.odl");

    assert_int_equal(amxb_connect(&bus_ctx_error, "dummy-error:/tmp/dummy_error1.sock"), 0);
    test_load_dummy_error_remote(bus_ctx_error, "./remote_error1.odl");
    assert_int_equal(amxb_connect(&bus_ctx_error_without_get_filtered, "dummy-error-without-getfiltered:/tmp/dummy_error2.sock"), 0);
    test_load_dummy_error_remote(bus_ctx_error_without_get_filtered, "./remote_error2.odl");

    amxo_parser_parse_file(&parser, "./local.odl", amxd_dm_get_root(&local_dm));
    assert_int_equal(amxb_register(bus_ctx_normal, &local_dm), 0);
    assert_int_equal(amxb_register(bus_ctx_error, &local_dm), 0);
    assert_int_equal(amxb_register(bus_ctx_error_without_get_filtered, &local_dm), 0);

    s_assert_stats_all_zero(bus_ctx_normal);
    s_assert_stats_all_zero(bus_ctx_error);
    s_assert_stats_all_zero(bus_ctx_error_without_get_filtered);

    return 0;
}

int test_amxb_stats_teardown(UNUSED void** state) {
    amxb_disconnect(bus_ctx_normal);
    amxb_disconnect(bus_ctx_error);
    amxb_disconnect(bus_ctx_error_without_get_filtered);
    amxb_free(&bus_ctx_normal);
    amxb_free(&bus_ctx_error);
    amxb_free(&bus_ctx_error_without_get_filtered);

    amxo_parser_clean(&parser);
    amxd_dm_clean(&local_dm);

    test_unregister_dummy_be();
    test_unregister_dummy_error_be();

    return 0;
}


void test_amxb_stats_get__call(UNUSED void** state) {
    // WHEN performing an invoke
    amxb_call(bus_ctx_normal, "SomeRemoteObject", "SomeFunction", NULL, NULL, 5);

    // THEN the number of invocations in the statistics increases by 1.
    s_assert_stats(bus_ctx_normal, "tx.operation.invoke", 1);
}

void test_amxb_stats_get__get_unsupported(UNUSED void** state) {
    amxc_var_t result;
    amxc_var_init(&result);

    // WHEN performing a "get" while the bus does not support "get"
    assert_int_equal(0, amxb_get(bus_ctx_normal, "SomeRemoteObject.", 0, &result, 5));

    // THEN the number of "get"s in the statistics stays 0
    s_assert_stats(bus_ctx_normal, "tx.operation.get", 0);
    // THEN the number of "invoke"s in the statistics increased by 1.
    s_assert_stats(bus_ctx_normal, "tx.operation.invoke", 1);

    // cleanup:
    amxc_var_clean(&result);
}

void test_amxb_stats_get__get(UNUSED void** state) {
    amxc_var_t result;
    amxc_var_init(&result);

    // GIVEN a bus without "get_filtered" but with "get"
    // (because if there's "get_filtered") then it's used instead of "get".

    // WHEN performing 3 "get"s on that bus
    amxb_get(bus_ctx_error_without_get_filtered, "SomeRemoteObjectForErrorBus2.", 0, &result, 5);
    amxb_get(bus_ctx_error_without_get_filtered, "SomeRemoteObjectForErrorBus2.Something", 0, &result, 5);
    amxb_get(bus_ctx_error_without_get_filtered, "SomeRemoteObjectForErrorBus2.Something.SomethingElse.", 0, &result, 5);

    // THEN the number of "get"s in the statistics became 3
    s_assert_stats(bus_ctx_error_without_get_filtered, "tx.operation.get", 3);

    // cleanup:
    amxc_var_clean(&result);
}

void test_amxb_stats_get__set(UNUSED void** state) {
    amxc_var_t result;
    amxc_var_init(&result);
    amxc_var_t values;
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "SomeParameter", "some value");

    // WHEN performing 2 "set"s
    amxb_set(bus_ctx_error, "SomeRemoteObjectForErrorBus1.", &values, &result, 5);
    amxb_set(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject", &values, &result, 5);

    // THEN the number of "set"s in the statistics became 2
    s_assert_stats(bus_ctx_error, "tx.operation.set", 2);

    // cleanup:
    amxc_var_clean(&result);
    amxc_var_clean(&values);
}

void test_amxb_stats_get__add(UNUSED void** state) {
    amxc_var_t result;
    amxc_var_init(&result);
    amxc_var_t values;
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "SomeParameter", "some value");

    // WHEN performing 4 "add"s
    amxb_add(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject1", 0, NULL, &values, &result, 5);
    amxb_add(bus_ctx_error, "SomeRemoteObjectForErrorBus1.Sub.Sub.Sub.", 0, NULL, &values, &result, 5);
    amxb_add(bus_ctx_error, "SomeRemoteObjectForErrorBus1.", 0, NULL, &values, &result, 5);
    amxb_add(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject2", 0, NULL, &values, &result, 5);

    // THEN the number of "add"s in the statistics became 4
    s_assert_stats(bus_ctx_error, "tx.operation.add", 4);

    // cleanup:
    amxc_var_clean(&result);
    amxc_var_clean(&values);
}

void test_amxb_stats_get__async_invoke(UNUSED void** state) {
    amxc_var_t values;
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "SomeParameter", "some value");

    // WHEN performing 3 "async_invoke"s
    amxb_async_call(bus_ctx_error, "SomeRemoteObjectForErrorBus1.", "MyMethod", &values, NULL, NULL);
    amxb_async_call(bus_ctx_error, "SomeRemoteObjectForErrorBus1.", "MyMethod2", &values, NULL, NULL);
    amxb_async_call(bus_ctx_error, "SomeRemoteObjectForErrorBus1.", "MyMethod3", &values, NULL, NULL);

    // THEN the number of "async_invoke"s in the statistics became 3
    s_assert_stats(bus_ctx_error, "tx.operation.async_invoke", 3);

    // cleanup:
    amxc_var_clean(&values);
}

void test_amxb_stats_get__del(UNUSED void** state) {
    amxc_var_t result;
    amxc_var_init(&result);

    // WHEN performing 2 "del"s
    amxb_del(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject.", 1, NULL, &result, 5);
    amxb_del(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject.", 0, "SomeAlias", &result, 5);

    // THEN the number of "del"s in the statistics became 2
    s_assert_stats(bus_ctx_error, "tx.operation.del", 2);

    // cleanup:
    amxc_var_clean(&result);
}

void test_amxb_stats_get__describe(UNUSED void** state) {
    amxc_var_t result;
    amxc_var_init(&result);

    // WHEN performing 2 "describe"s
    amxb_describe(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject.", AMXB_FLAG_EXISTS, &result, 5);
    amxb_describe(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject2.", 0, &result, 5);

    // THEN the number of "describe"s in the statistics became 2
    s_assert_stats(bus_ctx_error, "tx.operation.describe", 2);

    // cleanup:
    amxc_var_clean(&result);
}

void test_amxb_stats_get__get_filtered(UNUSED void** state) {
    amxc_var_t result;
    amxc_var_init(&result);

    // WHEN performing 2 "get_filtered"s
    amxb_get_filtered(bus_ctx_error, "SomeRemoteObjectForErrorBus1.", "attributes.persistent==true", 5, &result, 5);
    amxb_get_filtered(bus_ctx_error, "SomeRemoteObjectForErrorBus1.", "attributes.persistent==false", 5, &result, 5);

    // THEN the number of "get_filtered"s in the statistics became 2
    s_assert_stats(bus_ctx_error, "tx.operation.get_filtered", 2);

    // cleanup:
    amxc_var_clean(&result);
}

void test_amxb_stats_get__get_instances(UNUSED void** state) {
    amxc_var_t result;
    amxc_var_init(&result);

    // WHEN performing 3 "get_instances"s
    amxb_get_instances(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SomeTemplate.", 5, &result, 5);
    amxb_get_instances(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SomeTemplate.", 4, &result, 5);
    amxb_get_instances(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SomeOtherTemplate.", 4, &result, 5);

    // THEN the number of "get_instances"s in the statistics became 3
    s_assert_stats(bus_ctx_error, "tx.operation.get_instances", 3);

    // cleanup:
    amxc_var_clean(&result);
}

void test_amxb_stats_get__get_supported(UNUSED void** state) {
    amxc_var_t result;
    amxc_var_init(&result);

    // WHEN performing 4 "get_supported"s
    amxb_get_supported(bus_ctx_error, "SomeRemoteObjectForErrorBus1.Subobject1.", 0, &result, 5);
    amxb_get_supported(bus_ctx_error, "SomeRemoteObjectForErrorBus1.Foo.", 0, &result, 5);
    amxb_get_supported(bus_ctx_error, "SomeRemoteObjectForErrorBus1.Bar.", 0, &result, 5);
    amxb_get_supported(bus_ctx_error, "SomeRemoteObjectForErrorBus1.FooBar.", 0, &result, 5);

    // THEN the number of "get_supported"s in the statistics became 4
    s_assert_stats(bus_ctx_error, "tx.operation.get_supported", 4);

    // cleanup:
    amxc_var_clean(&result);
}

void test_amxb_stats_get__has(UNUSED void** state) {
    // WHEN performing 2 "has"s
    amxb_be_who_has_ex("SomeObjectThatNobodyHas.", true);
    amxb_be_who_has_ex("SomeOtherObjectThatNobodyHas.", true);

    // THEN the number of "has"s in the statistics became 2
    s_assert_stats(bus_ctx_error, "tx.operation.has", 2);
}

static void s_list_cb(UNUSED const amxb_bus_ctx_t* bus_ctx,
                      UNUSED const amxc_var_t* const data,
                      UNUSED void* priv) {
}

void test_amxb_stats_get__list(UNUSED void** state) {
    // WHEN performing 2 "list"s
    amxb_list(bus_ctx_error, "SomeRemoteObjectForErrorBus1.", 0, s_list_cb, NULL);
    amxb_list(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject.", 0, s_list_cb, NULL);

    // THEN the number of "list"s in the statistics became 2
    s_assert_stats(bus_ctx_error, "tx.operation.list", 2);
}

/** @implements @ref amxp_slot_fn_t */
static void s_subscribe_cb(UNUSED const char* const sig_name,
                           UNUSED const amxc_var_t* const data,
                           UNUSED void* const priv) {

}

void test_amxb_stats_get__subscribe(UNUSED void** state) {
    // WHEN performing 3 "subscribe"s
    amxb_subscribe(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject", "Something == true", s_subscribe_cb, NULL);
    amxb_subscribe(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject2.", "Something == false", s_subscribe_cb, NULL);
    amxb_subscribe(bus_ctx_error, "SomeRemoteObjectForErrorBus1.OtherSubObject.", NULL, s_subscribe_cb, NULL);

    // THEN the number of "subscribe"s in the statistics became 3
    s_assert_stats(bus_ctx_error, "tx.operation.subscribe", 3);
}

void test_amxb_stats_get__unsubscribe(UNUSED void** state) {
    // GIVEN a bus context with 2 subscriptions
    amxb_subscribe(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject", "Something == true", s_subscribe_cb, NULL);
    amxb_subscribe(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject2.", NULL, s_subscribe_cb, NULL);

    // WHEN unsubscribing
    amxb_unsubscribe(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject", s_subscribe_cb, NULL);
    amxb_unsubscribe(bus_ctx_error, "SomeRemoteObjectForErrorBus1.SubObject2.", s_subscribe_cb, NULL);

    // THEN the number of "unsubscribe"s in the statistics became 2
    s_assert_stats(bus_ctx_error, "tx.operation.unsubscribe", 2);
}

void test_amxb_stats_get__wait_for(UNUSED void** state) {
    // WHEN waiting for three objects
    amxb_wait_for_object("SomeObjectThatIsCurrentlyNotThereYet");
    amxb_wait_for_object("SomeOtherObjectThatIsCurrentlyNotThereYet");
    amxb_wait_for_object("SomethingElse");

    // THEN the number of "wait_for"s in the statistics became 3
    s_assert_stats(bus_ctx_error, "tx.operation.wait_for", 3);
    while(amxp_signal_read() == 0) {
    }
}

void test_amxb_stats_get__stats_generated_by_backend(UNUSED void** state) {
    amxc_var_t stats;

    // GIVEN a bus backend that supports "get_stats"
    amxb_bus_ctx_t* bus = bus_ctx_error;

    // WHEN fetching the bus statistics
    amxc_var_init(&stats);
    amxb_stats_get(bus, &stats);

    // THEN the statistics as reported by the bus backend are included in the bus statistics
    s_assert_field(&stats, "rx.operation.some_field_coming_from_the_bus_backend", 12345);

    // cleanup
    amxc_var_clean(&stats);
}
