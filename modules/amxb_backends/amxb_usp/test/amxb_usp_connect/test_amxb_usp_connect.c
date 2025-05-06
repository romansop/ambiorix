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

#include <amxc/amxc_variant.h>
#include <amxc/amxc_htable.h>
#include <amxc/amxc_lqueue.h>

#include <amxp/amxp_signal.h>
#include <amxp/amxp_slot.h>

#include <amxd/amxd_dm.h>

#include <amxb/amxb_error.h>
#include <amxb/amxb.h>

#include "amxb_usp.h"
#include "test_amxb_usp_connect.h"
#include "test_amxb_usp_common.h"
#include "imtp_mock.h"

#define UNUSED __attribute__((unused))

static amxp_signal_mngr_t* sigmngr = NULL;
static amxc_var_t* config = NULL;

int test_setup(UNUSED void** state) {
    amxc_var_t* usp_section = NULL;

    amxc_var_new(&config);
    amxc_var_set_type(config, AMXC_VAR_ID_HTABLE);
    usp_section = amxc_var_add_key(amxc_htable_t, config, "usp", NULL);

    amxb_usp_set_config(usp_section);

    return 0;
}

int test_teardown(UNUSED void** state) {
    amxc_var_delete(&config);
    return 0;
}

void test_amxb_usp_connect_disconnect(UNUSED void** state) {
    amxb_usp_t* usp_ctx1 = NULL;
    amxb_usp_t* usp_ctx2 = NULL;
    amxp_sigmngr_new(&sigmngr);

    usp_ctx1 = amxb_usp_connect(NULL, NULL, NULL, sigmngr);
    assert_ptr_equal(usp_ctx1, NULL);

    usp_ctx1 = amxb_usp_connect("", NULL, NULL, sigmngr);
    assert_ptr_equal(usp_ctx1, NULL);

    usp_ctx1 = amxb_usp_connect("", "", NULL, sigmngr);
    assert_ptr_equal(usp_ctx1, NULL);

    usp_ctx1 = amxb_usp_connect("localhost", NULL, NULL, sigmngr);
    assert_ptr_equal(usp_ctx1, NULL);

    usp_ctx1 = amxb_usp_connect("localhost", "", NULL, sigmngr);
    assert_ptr_equal(usp_ctx1, NULL);

    usp_ctx1 = amxb_usp_connect("localhost", "2001", NULL, sigmngr);
    assert_ptr_equal(usp_ctx1, NULL);

    usp_ctx2 = amxb_usp_connect("localhost", NULL, NULL, sigmngr);
    assert_ptr_equal(usp_ctx2, NULL);

    expect_any_count(__wrap_imtp_connection_connect, icon, 2);
    expect_any_count(__wrap_imtp_connection_connect, from_uri, 2);
    expect_any_count(__wrap_imtp_connection_connect, to_uri, 2);
    will_return(__wrap_imtp_connection_connect, 99);
    will_return(__wrap_imtp_connection_connect, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    usp_ctx1 = amxb_usp_connect(NULL, NULL, "/tmp/test-1.sock", sigmngr);
    assert_ptr_not_equal(usp_ctx1, NULL);
    assert_string_equal(usp_ctx1->icon->addr_other->sun_path, "/tmp/test-1.sock");
    assert_int_equal(usp_ctx1->icon->flags, SOCK_STREAM);

    will_return(__wrap_imtp_connection_connect, 100);
    will_return(__wrap_imtp_connection_connect, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    usp_ctx2 = amxb_usp_connect(NULL, NULL, "/tmp/test-2.sock", sigmngr);
    assert_ptr_not_equal(usp_ctx2, NULL);
    assert_string_equal(usp_ctx2->icon->addr_other->sun_path, "/tmp/test-2.sock");
    assert_int_equal(usp_ctx2->icon->flags, SOCK_STREAM);

    assert_ptr_not_equal(usp_ctx1, usp_ctx2);

    expect_any_count(__wrap_imtp_connection_delete, icon, 2);

    assert_int_equal(amxb_usp_disconnect(usp_ctx1), 0);
    assert_int_equal(amxb_usp_disconnect(usp_ctx2), 0);

    assert_int_equal(amxb_usp_disconnect(NULL), -1);
    amxb_usp_free(usp_ctx1);
    amxb_usp_free(usp_ctx2);
    amxp_sigmngr_delete(&sigmngr);
}

void test_amxb_usp_get_fd(UNUSED void** state) {
    void* usp_ctx = NULL;
    amxp_sigmngr_new(&sigmngr);

    expect_any(__wrap_imtp_connection_connect, icon);
    expect_any(__wrap_imtp_connection_connect, from_uri);
    expect_any(__wrap_imtp_connection_connect, to_uri);
    will_return(__wrap_imtp_connection_connect, 99);
    will_return(__wrap_imtp_connection_connect, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    usp_ctx = amxb_usp_connect(NULL, NULL, "/tmp/test-fd.sock", sigmngr);
    assert_ptr_not_equal(usp_ctx, NULL);

    assert_true(amxb_usp_get_fd(usp_ctx) > -1);

    expect_any(__wrap_imtp_connection_delete, icon);
    assert_int_equal(amxb_usp_disconnect(usp_ctx), 0);
    amxb_usp_free(usp_ctx);

    amxp_sigmngr_delete(&sigmngr);
}

void test_amxb_info(UNUSED void** state) {
    amxb_be_info_t* info = NULL;

    info = amxb_be_info();

    assert_ptr_not_equal(info, NULL);
    assert_ptr_not_equal(info->min_supported, NULL);
    assert_ptr_not_equal(info->max_supported, NULL);
    assert_ptr_not_equal(info->be_version, NULL);
    assert_ptr_not_equal(info->name, NULL);
}

void test_amxb_usp_send_and_recv_eid(UNUSED void** state) {
    amxb_usp_t* usp_ctx = NULL;
    amxp_sigmngr_new(&sigmngr);
    amxc_var_t data;

    amxc_var_init(&data);

    expect_any(__wrap_imtp_connection_connect, icon);
    expect_any(__wrap_imtp_connection_connect, from_uri);
    expect_any(__wrap_imtp_connection_connect, to_uri);
    will_return(__wrap_imtp_connection_connect, 99);
    will_return(__wrap_imtp_connection_connect, 0);

    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    // Send eid on connect
    usp_ctx = amxb_usp_connect(NULL, NULL, "/tmp/test-fd.sock", sigmngr);
    assert_ptr_not_equal(usp_ctx, NULL);

    // Trigger slot to receive eid
    amxc_var_set_type(&data, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &data, "EndpointID", "dummy");
    amxc_var_add_key(uint32_t, &data, "fd", usp_ctx->icon->fd);
    amxp_sigmngr_emit_signal(NULL, "usp:eid-added", &data);

    handle_events();

    expect_any(__wrap_imtp_connection_delete, icon);
    assert_int_equal(amxb_usp_disconnect(usp_ctx), 0);
    amxb_usp_free(usp_ctx);

    amxc_var_clean(&data);
    amxp_sigmngr_delete(&sigmngr);
}

void test_amxb_usp_listen_and_accept(UNUSED void** state) {
    void* usp_listen_ctx = NULL;
    void* usp_accepted_ctx = NULL;
    amxp_sigmngr_new(&sigmngr);

    will_return(__wrap_imtp_connection_listen, 3);
    will_return(__wrap_imtp_connection_listen, 0);

    usp_listen_ctx = amxb_usp_listen(NULL, NULL, "/tmp/test-fd.sock", sigmngr);
    assert_ptr_not_equal(usp_listen_ctx, NULL);

    will_return(__wrap_imtp_connection_accept, 4);
    will_return(__wrap_imtp_connection_accept, 0);
    expect_any(__wrap_imtp_connection_write_frame, icon);
    expect_any(__wrap_imtp_connection_write_frame, frame);

    usp_accepted_ctx = amxb_usp_accept(usp_listen_ctx, sigmngr);
    assert_ptr_not_equal(usp_accepted_ctx, NULL);

    expect_any(__wrap_imtp_connection_delete, icon);
    assert_int_equal(amxb_usp_disconnect(usp_accepted_ctx), 0);
    amxb_usp_free(usp_accepted_ctx);

    expect_any(__wrap_imtp_connection_delete, icon);
    assert_int_equal(amxb_usp_disconnect(usp_listen_ctx), 0);
    amxb_usp_free(usp_listen_ctx);

    amxp_sigmngr_delete(&sigmngr);
}
