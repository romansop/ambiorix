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

#include <unistd.h>
#include <fcntl.h>

#include "../common/include/amxb_rbus_test.h"
#include "test_amxb_rbus_connect.h"

static amxc_var_t config;

int test_setup(UNUSED void** state) {
    amxc_var_init(&config);
    amxc_var_set_type(&config, AMXC_VAR_ID_HTABLE);

    assert_int_equal(amxb_be_load("../mod-amxb-test-rbus.so"), 0);
    // enable cmocka checking in the test build of the backend
    cmocka_rbus_backend_enable_check("../mod-amxb-test-rbus.so", true);

    return 0;
}

int test_teardown(UNUSED void** state) {
    amxb_be_remove_all();
    amxc_var_clean(&config);
    return 0;
}

void test_amxb_rbus_connects_using_rbus_open(UNUSED void** state) {
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxc_string_t id;
    char* name = NULL;
    amxc_string_init(&id, 0);
    // The default one will be "amx-<PID>"
    amxc_string_setf(&id, "amx-%d", getpid());
    name = amxc_string_take_buffer(&id);

    // when connecting to rbus, the amxb-rbus backend must call rbus_open
    // and provide the component name, if no register-name option is set
    // the backend creates a default name "amx-<PID>"
    expect_string(__wrap_rbus_open, componentName, name);
    assert_int_equal(amxb_connect(&bus_ctx, "rbus:"), 0);
    // after creating the connection, the bus ctx must be set
    assert_non_null(bus_ctx);

    free(name);
    amxc_string_clean(&id);
    amxb_free(&bus_ctx);
}

void test_amxb_rbus_connects_using_rbus_open_with_configured_name(UNUSED void** state) {
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxc_var_t* rbus_config = NULL;
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    amxc_var_add_key(cstring_t, rbus_config, "register-name", "test-amxb-rbus");
    assert_int_equal(amxb_set_config(&config), 0);

    // when connecting to rbus, the amxb-rbus backend must call rbus_open
    // and provide the component name, if the register name is provided in the
    // backend configuration it must be used
    expect_string(__wrap_rbus_open, componentName, "test-amxb-rbus");
    assert_int_equal(amxb_connect(&bus_ctx, "rbus:"), 0);
    // after creating the connection, the bus ctx must be set
    assert_non_null(bus_ctx);

    amxb_free(&bus_ctx);
}

void test_amxb_rbus_can_handle_rbus_open_failure(UNUSED void** state) {
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxc_var_t* rbus_config = NULL;
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    amxc_var_add_key(cstring_t, rbus_config, "register-name", "test-amxb-rbus");
    assert_int_equal(amxb_set_config(&config), 0);

    // when connecting to rbus, and crbus open fails
    cmocka_rbus_backend_enable_mock("../mod-amxb-test-rbus.so", true);
    expect_string(__wrap_rbus_open, componentName, "test-amxb-rbus");
    will_return(__wrap_rbus_open, RBUS_ERROR_BUS_ERROR);
    // then amxb_connect must fail as well
    assert_int_not_equal(amxb_connect(&bus_ctx, "rbus:"), 0);
    cmocka_rbus_backend_enable_mock("../mod-amxb-test-rbus.so", false);
    // and the bus_ctx must remain NULL
    assert_null(bus_ctx);

    amxb_free(&bus_ctx);
}

void test_amxb_rbus_connect_fails_when_no_socket_pair_can_be_created(UNUSED void** state) {
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxc_var_t* rbus_config = NULL;
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    amxc_var_add_key(cstring_t, rbus_config, "register-name", "test-amxb-rbus");
    assert_int_equal(amxb_set_config(&config), 0);

    // when connecting to rbus, and creation of the socket pair fails
    cmocka_rbus_backend_enable_mock("../mod-amxb-test-rbus.so", true);
    expect_string(__wrap_rbus_open, componentName, "test-amxb-rbus");
    will_return(__wrap_rbus_open, RBUS_ERROR_SUCCESS);
    will_return(__wrap_rbus_close, RBUS_ERROR_SUCCESS);
    will_return(__wrap_socketpair, -1);
    // then amxb_connect must fail as well
    assert_int_not_equal(amxb_connect(&bus_ctx, "rbus:"), 0);
    cmocka_rbus_backend_enable_mock("../mod-amxb-test-rbus.so", false);
    // and the bus_ctx must remain NULL
    assert_null(bus_ctx);

    amxb_free(&bus_ctx);
}

void test_amxb_rbus_can_disconnect_from_rbus_daemon(UNUSED void** state) {
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxc_var_t* rbus_config = NULL;
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    amxc_var_add_key(cstring_t, rbus_config, "register-name", "test-amxb-rbus");
    assert_int_equal(amxb_set_config(&config), 0);

    // when connecting is open to rbus daemon
    expect_string(__wrap_rbus_open, componentName, "test-amxb-rbus");
    assert_int_equal(amxb_connect(&bus_ctx, "rbus:"), 0);
    assert_non_null(bus_ctx);

    // it must be possible to discconect
    assert_int_equal(amxb_disconnect(bus_ctx), 0);
    assert_non_null(bus_ctx);

    amxb_free(&bus_ctx);
}

void test_amxb_rbus_propagates_failure_to_disconnect(UNUSED void** state) {
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxc_var_t* rbus_config = NULL;
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    amxc_var_add_key(cstring_t, rbus_config, "register-name", "test-amxb-rbus");
    assert_int_equal(amxb_set_config(&config), 0);

    // when connecting is open to rbus daemon
    expect_string(__wrap_rbus_open, componentName, "test-amxb-rbus");
    assert_int_equal(amxb_connect(&bus_ctx, "rbus:"), 0);
    assert_non_null(bus_ctx);

    // it must be possible to discconect
    cmocka_rbus_backend_enable_mock("../mod-amxb-test-rbus.so", true);
    will_return(__wrap_rbus_close, RBUS_ERROR_BUS_ERROR);
    assert_int_not_equal(amxb_disconnect(bus_ctx), 0);
    assert_non_null(bus_ctx);
    cmocka_rbus_backend_enable_mock("../mod-amxb-test-rbus.so", false);

    amxb_free(&bus_ctx);
}

void test_amxb_rbus_provides_valid_fd(UNUSED void** state) {
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    assert_int_equal(amxb_set_config(&config), 0);

    // when connecting is open to rbus daemon
    expect_string(__wrap_rbus_open, componentName, "test-amxb-rbus");
    assert_int_equal(amxb_connect(&bus_ctx, "rbus:"), 0);
    assert_non_null(bus_ctx);

    // The back-end must provide a valid file descriptor
    assert_int_not_equal(amxb_get_fd(bus_ctx), -1);
    assert_int_equal(fcntl(amxb_get_fd(bus_ctx), F_GETFD), 0);

    // When the connection is closed
    cmocka_rbus_backend_enable_mock("../mod-amxb-test-rbus.so", true);
    will_return(__wrap_rbus_close, RBUS_ERROR_BUS_ERROR);
    assert_int_not_equal(amxb_disconnect(bus_ctx), 0);
    assert_non_null(bus_ctx);
    cmocka_rbus_backend_enable_mock("../mod-amxb-test-rbus.so", false);

    // the backend should not provide a valid file descriptor
    assert_int_equal(amxb_get_fd(bus_ctx), -1);

    amxb_free(&bus_ctx);
}