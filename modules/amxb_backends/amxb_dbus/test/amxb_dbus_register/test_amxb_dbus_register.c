/* SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 * SPDX-FileCopyrightText: 2022 the amxb_rbus contributors (see AUTHORS.md)
 *
 * This code is subject to the terms of the BSD+Patent license.
 * See LICENSE file for more details.
 */

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
#include <amxd/amxd_object.h>
#include <amxd/amxd_transaction.h>
#include <amxo/amxo.h>

#include <amxb/amxb.h>
#include <amxb/amxb_register.h>

#include "amxb_rbus.h"
#include "test_amxb_rbus_register.h"

static amxb_bus_ctx_t* bus_ctx = NULL;
static amxd_dm_t dm;
static amxo_parser_t parser;
static amxd_object_t* template = NULL;

static void handle_events(void) {
    printf("Handling events ");
    while(amxp_signal_read() == 0) {
        printf(".");
    }
    printf("\n");
}

int test_amxb_rbus_register_setup(UNUSED void** state) {
    assert_int_equal(amxd_dm_init(&dm), 0);
    assert_int_equal(amxo_parser_init(&parser), 0);

    assert_int_equal(amxb_be_load("../../output/x86_64-linux-gnu/mod-amxb-rbus.so"), 0);
    assert_int_equal(amxb_connect(&bus_ctx, "rbus:"), 0);

    amxo_parser_parse_file(&parser, "../test_data/test_nemo.odl", amxd_dm_get_root(&dm));

    return 0;
}

int test_amxb_rbus_register_teardown(UNUSED void** state) {
    amxd_dm_clean(&dm);
    amxo_parser_clean(&parser);

    amxb_disconnect(bus_ctx);
    amxb_free(&bus_ctx);
    amxb_be_remove_all();
    unlink("../test_data/test_nemo.odl");
    return 0;
}

void test_rbus_can_register_dm(UNUSED void** state) {
    assert_int_equal(amxb_register(bus_ctx, &dm), 0);
}

void test_rbus_new_objects_are_registered_template_instance(UNUSED void** state) {
    amxd_object_t* object = NULL;

    assert_int_equal(amxd_object_new(&template, amxd_object_template, "Test_Tempalet"), 0);
    assert_ptr_not_equal(template, NULL);

    assert_int_equal(amxd_object_new_instance(&object, template, "instance1", 1, NULL), 0);
    assert_ptr_not_equal(object, NULL);
    assert_int_equal(amxd_dm_add_root_object(&dm, template), 0);

    handle_events();

    assert_int_equal(system("rbuscli  getname Test_Tempalet."), 0);
    amxb_free(object);
}

void test_rbus_new_root_objects_are_registered(UNUSED void** state) {
    amxd_object_t* object = NULL;
    amxd_param_t* param = NULL;

    assert_int_equal(amxd_object_new(&object, amxd_object_singleton, "test_object"), 0);
    assert_int_equal(amxd_param_new(&param, "Max", AMXC_VAR_ID_UINT32), 0);
    assert_int_equal(amxd_object_add_param(object, param), 0);
    assert_ptr_not_equal(object, NULL);
    amxc_var_set(uint32_t, &param->value, 2);
    assert_int_equal(amxd_dm_add_root_object(&dm, object), 0);

    handle_events();

    assert_int_equal(system("rbuscli getvalues test_object."), 0);
    amxb_free(object);
    amxb_free(param);
}

void test_rbus_removed_objects_are_unregistered(UNUSED void** state) {
    amxd_object_free(&template);
    assert_int_equal(system("rbuscli  getname Test_Tempalet."), 0);
}
