/* SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 * SPDX-FileCopyrightText: 2022 the amxb_dbus contributors (see AUTHORS.md)
 *
 * This code is subject to the terms of the BSD+Patent license.
 * See LICENSE file for more details.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

#include <amxc/amxc_variant.h>
#include <amxc/amxc_lqueue.h>

#include <amxp/amxp_signal.h>
#include <amxp/amxp_slot.h>

#include <amxd/amxd_dm.h>

#include "amxb_dbus.h"
#include "test_amxb_dbus_connect.h"

#include <amxc/amxc_macros.h>

static amxp_signal_mngr_t* sigmngr = NULL;


void test_amxb_dbus_connect_disconnect(UNUSED void** state) {
    void* bus_ctx1 = NULL;
    void* bus_ctx = NULL;

    amxp_sigmngr_new(&sigmngr);

    bus_ctx1 = amxb_dbus_connect(NULL, NULL, NULL, sigmngr);
    assert_ptr_not_equal(bus_ctx1, NULL);
    assert_int_equal(amxb_dbus_disconnect(bus_ctx1), 0);
    amxb_dbus_free(bus_ctx1);

    bus_ctx = amxb_dbus_connect("", "", NULL, NULL);
    assert_ptr_equal(bus_ctx, NULL);
    assert_int_not_equal(amxb_dbus_disconnect(bus_ctx), -1);
    amxb_dbus_free(bus_ctx);

    amxp_sigmngr_delete(&sigmngr);
}

void test_amxb_dbus_get_fd(UNUSED void** state) {
    void* bus_ctx = NULL;
    void* bus_ctx1 = NULL;

    amxp_sigmngr_new(&sigmngr);

    bus_ctx = amxb_dbus_connect(NULL, NULL, NULL, sigmngr);
    assert_int_not_equal(amxb_dbus_get_fd(bus_ctx), -1);

    assert_int_not_equal(amxb_dbus_disconnect(bus_ctx), -1);
    amxb_dbus_free(bus_ctx);

    assert_int_equal(amxb_dbus_disconnect(bus_ctx1), 0);
    amxb_dbus_free(bus_ctx1);

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
