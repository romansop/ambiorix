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

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>

#include <amxd/amxd_dm.h>
#include <amxd/amxd_transaction.h>

#include <amxb/amxb.h>

#include <amxo/amxo.h>

#include "amxb_usp.h"
#include "test_amxb_usp_read.h"
#include "imtp_mock.h"
#include "io_mock.h"

#define UNUSED __attribute__((unused))

static amxo_parser_t parser;
static const char* odl_defs = "../test_dm.odl";
static amxc_var_t* config = NULL;

static amxd_dm_t dm;

int test_dm_setup(UNUSED void** state) {
    amxd_object_t* root_obj = NULL;
    amxc_var_t* usp_section = NULL;

    assert_int_equal(amxd_dm_init(&dm), 0);

    assert_int_equal(amxo_parser_init(&parser), 0);

    amxc_var_new(&config);
    amxc_var_set_type(config, AMXC_VAR_ID_HTABLE);
    usp_section = amxc_var_add_key(amxc_htable_t, config, "usp", NULL);
    amxb_usp_set_config(usp_section);

    root_obj = amxd_dm_get_root(&dm);
    assert_non_null(root_obj);

    assert_int_equal(amxo_parser_parse_file(&parser, odl_defs, root_obj), 0);

    assert_int_equal(amxo_parser_invoke_entry_points(&parser, &dm, AMXO_START), 0);

    return 0;
}

int test_dm_teardown(UNUSED void** state) {
    assert_int_equal(amxo_parser_invoke_entry_points(&parser, &dm, AMXO_STOP), 0);

    amxo_parser_clean(&parser);
    amxc_var_delete(&config);

    amxo_resolver_import_close_all();

    amxd_dm_clean(&dm);

    return 0;
}

void test_amxb_usp_read_raw(UNUSED void** state) {
    void* usp_ctx = NULL;
    amxp_signal_mngr_t sigmngr;
    int count = 256;
    char buf[256] = {};

    amxp_sigmngr_init(&sigmngr);

    expect_any(__wrap_imtp_connection_connect, icon);
    expect_any(__wrap_imtp_connection_connect, from_uri);
    expect_any(__wrap_imtp_connection_connect, to_uri);
    will_return(__wrap_imtp_connection_connect, 99);
    will_return(__wrap_imtp_connection_connect, 0);
    // write handshake on connect
    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    usp_ctx = amxb_usp_connect(NULL, NULL, "/tmp/usp-ba.sock", &sigmngr);
    assert_non_null(usp_ctx);

    // read handshake before sending register
    expect_any(__wrap_imtp_connection_read_frame, icon);
    expect_any(__wrap_imtp_connection_read_frame, frame);
    will_return(__wrap_imtp_connection_read_frame, usp_type_handshake);
    will_return(__wrap_imtp_connection_read_frame, 0);

    // write register
    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);
    assert_int_equal(amxb_usp_register(usp_ctx, &dm), 0);

    expect_any(__wrap_read, fd);
    expect_any(__wrap_read, buf);
    expect_value(__wrap_read, count, count);
    will_return(__wrap_read, count);

    assert_int_equal(amxb_usp_read_raw(usp_ctx, buf, count), count);

    expect_any(__wrap_imtp_connection_delete, icon);
    assert_int_equal(amxb_usp_disconnect(usp_ctx), 0);
    imtp_mock_free_msg_id();
    amxb_usp_free(usp_ctx);
    amxp_sigmngr_clean(&sigmngr);
}

void test_amxb_usp_read_raw_invalid(UNUSED void** state) {
    amxb_usp_t* usp_ctx = NULL;
    int count = 256;
    char buf[256] = {};

    assert_int_equal(amxb_usp_read_raw(usp_ctx, NULL, count), -1);
    usp_ctx = calloc(1, sizeof(amxb_usp_t));
    assert_int_equal(amxb_usp_read_raw(usp_ctx, NULL, count), -1);
    usp_ctx->icon = calloc(1, sizeof(amxb_usp_t));
    assert_int_equal(amxb_usp_read_raw(usp_ctx, NULL, count), -1);

    expect_any(__wrap_read, fd);
    expect_any(__wrap_read, buf);
    expect_value(__wrap_read, count, count);
    will_return(__wrap_read, -1);

    assert_int_equal(amxb_usp_read_raw(usp_ctx, buf, count), -1);

    free(usp_ctx->icon);
    free(usp_ctx);
}

