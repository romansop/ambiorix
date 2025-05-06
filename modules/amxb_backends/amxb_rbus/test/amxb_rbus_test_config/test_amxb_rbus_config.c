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

#include "../common/include/amxb_rbus_test.h"
#include "test_amxb_rbus_config.h"

static amxc_var_t config;

int test_setup(UNUSED void** state) {
    amxc_var_init(&config);
    amxc_var_set_type(&config, AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxb_be_load("../mod-amxb-test-rbus.so"), 0);

    return 0;
}

int test_setup_no_rbus(UNUSED void** state) {
    amxc_var_init(&config);
    amxc_var_set_type(&config, AMXC_VAR_ID_HTABLE);
    assert_int_equal(amxb_be_load("../mod-amxb-test-rbus.so"), 0);

    system("killall rtrouted");
    system("rm -rf /tmp/rtrouted");

    return 0;
}

int test_teardown(UNUSED void** state) {
    amxb_be_remove_all();
    amxc_var_clean(&config);
    return 0;
}

void test_amxb_rbus_provides_default_uri(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_t* uris = NULL;

    // when the back-end is loaded
    // and an empty config is set
    assert_int_equal(amxb_set_config(&config), 0);

    // the back-end must add a specific config option which must be a table
    rbus_config = amxc_var_get_key(&config, "rbus", AMXC_VAR_FLAG_DEFAULT);
    assert_non_null(rbus_config);
    amxc_var_dump(rbus_config, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(rbus_config), AMXC_VAR_ID_HTABLE);

    // the default uris must be filled and must be a list
    uris = amxc_var_get_key(rbus_config, "uris", AMXC_VAR_FLAG_DEFAULT);
    assert_non_null(uris);
    assert_int_equal(amxc_var_type_of(uris), AMXC_VAR_ID_LIST);
    // only one default uri must be added "rbus:"
    assert_non_null(GETI_CHAR(uris, 0));
    assert_null(GETI_CHAR(uris, 1));
    assert_string_equal(GETI_CHAR(uris, 0), "rbus:");
}

void test_amxb_rbus_does_not_overwrite_provided_uris(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_t* uris = NULL;
    amxc_var_t* uri = NULL;

    // when the back-end is loaded and configuration is provide
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    uris = amxc_var_add_key(amxc_llist_t, rbus_config, "uris", NULL);
    uri = amxc_var_add(cstring_t, uris, "rbus:/tmp/rtrouted-test.socket");
    assert_int_equal(amxb_set_config(&config), 0);
    amxc_var_dump(rbus_config, STDOUT_FILENO);

    // the back-end must take the cofiguration as is and must not update
    // or add the default uri
    assert_ptr_equal(rbus_config, amxc_var_get_key(&config, "rbus", AMXC_VAR_FLAG_DEFAULT));
    assert_ptr_equal(uris, amxc_var_get_key(rbus_config, "uris", AMXC_VAR_FLAG_DEFAULT));
    assert_ptr_equal(uri, GETI_ARG(uris, 0));
    assert_string_equal(GET_CHAR(uri, NULL), "rbus:/tmp/rtrouted-test.socket");
    assert_null(GETI_CHAR(uris, 1));
}

void test_amxb_rbus_does_not_accept_invalid_uris_type(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_t* uris = NULL;

    // when the back-end is loaded and configuration is provide with invalid uris type
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    uris = amxc_var_add_key(cstring_t, rbus_config, "uris", "rbus:/tmp/rtrouted-test.socket");
    assert_int_equal(amxb_set_config(&config), 0);
    amxc_var_dump(rbus_config, STDOUT_FILENO);

    // the set config will make it a list and provide the default uri
    assert_int_equal(amxc_var_type_of(uris), AMXC_VAR_ID_LIST);
    // only one default uri must be added "rbus:"
    assert_non_null(GETI_CHAR(uris, 0));
    assert_null(GETI_CHAR(uris, 1));
    assert_string_equal(GETI_CHAR(uris, 0), "rbus:");
}

void test_amxb_rbus_does_not_provide_default_uri_when_socket_does_not_exist(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_t* uris = NULL;

    // when the back-end is loaded
    // and an empty config is set
    assert_int_equal(amxb_set_config(&config), 0);

    // the back-end must add a specific config option which must be a table
    rbus_config = amxc_var_get_key(&config, "rbus", AMXC_VAR_FLAG_DEFAULT);
    assert_non_null(rbus_config);
    amxc_var_dump(rbus_config, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(rbus_config), AMXC_VAR_ID_HTABLE);

    // the default uris must be filled and must be a list
    uris = amxc_var_get_key(rbus_config, "uris", AMXC_VAR_FLAG_DEFAULT);
    assert_non_null(uris);
    assert_int_equal(amxc_var_type_of(uris), AMXC_VAR_ID_LIST);
    // the list must be empty
    assert_null(GETI_CHAR(uris, 0));
}

void test_amxb_rbus_indicates_if_amx_calls_are_supported(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_t* amx_calls = NULL;

    // when the back-end is loaded
    // and an empty config is set
    assert_int_equal(amxb_set_config(&config), 0);

    // the back-end must add a specific config option which must be a table
    rbus_config = amxc_var_get_key(&config, "rbus", AMXC_VAR_FLAG_DEFAULT);
    assert_non_null(rbus_config);
    amxc_var_dump(rbus_config, STDOUT_FILENO);
    assert_int_equal(amxc_var_type_of(rbus_config), AMXC_VAR_ID_HTABLE);

    // and must indicate that the internal amx data model calls are supported
    amx_calls = amxc_var_get_key(rbus_config, "use-amx-calls", AMXC_VAR_FLAG_DEFAULT);
    assert_non_null(amx_calls);
    assert_int_equal(amxc_var_type_of(amx_calls), AMXC_VAR_ID_BOOL);
    // by default the amx-calls are supported
    assert_true(GET_BOOL(amx_calls, NULL));
}

void test_amxb_rbus_accept_option_to_turn_off_amx_calls(UNUSED void** state) {
    amxc_var_t* rbus_config = NULL;
    amxc_var_t* amx_calls = NULL;

    // when the back-end is loaded and us-amx-calls is set off
    rbus_config = amxc_var_add_key(amxc_htable_t, &config, "rbus", NULL);
    amx_calls = amxc_var_add_key(bool, rbus_config, "use-amx-calls", false);
    assert_int_equal(amxb_set_config(&config), 0);
    amxc_var_dump(rbus_config, STDOUT_FILENO);

    // the back-end must accept the config option and not change the value
    assert_ptr_equal(amx_calls, GET_ARG(rbus_config, "use-amx-calls"));
    assert_false(GET_BOOL(amx_calls, NULL));
}
