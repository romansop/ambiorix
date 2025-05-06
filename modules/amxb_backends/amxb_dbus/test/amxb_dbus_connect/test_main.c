/* SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 * SPDX-FileCopyrightText: 2022 the amxb_dbus contributors (see AUTHORS.md)
 *
 * This code is subject to the terms of the BSD+Patent license.
 * See LICENSE file for more details.
 */

#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>
#include <stdio.h>

#include "test_amxb_dbus_connect.h"

int main(void) {

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_amxb_dbus_connect_disconnect),
        cmocka_unit_test(test_amxb_dbus_get_fd),
        cmocka_unit_test(test_amxb_info),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
