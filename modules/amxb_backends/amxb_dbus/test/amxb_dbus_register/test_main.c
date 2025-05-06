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

#include "test_amxb_dbus_register.h"

int main(void) {
    int rv = 0;
    const struct CMUnitTest amx_tests[] = {
        cmocka_unit_test(test_dbus_can_register_dm),
        cmocka_unit_test(test_dbus_new_objects_are_registered_template_instance),
        cmocka_unit_test(test_dbus_new_root_objects_are_registered),
        cmocka_unit_test(test_dbus_removed_objects_are_unregistered),
    };

    rv = cmocka_run_group_tests(amx_tests, test_amxb_dbus_register_setup, test_amxb_dbus_register_teardown);
    return rv;
}
