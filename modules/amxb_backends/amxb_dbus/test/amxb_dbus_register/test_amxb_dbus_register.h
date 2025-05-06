/* SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 * SPDX-FileCopyrightText: 2022 the amxb_rbus contributors (see AUTHORS.md)
 *
 * This code is subject to the terms of the BSD+Patent license.
 * See LICENSE file for more details.
 */

#ifndef __TEST_AMXB_RBUS_REGISTER_H__
#define __TEST_AMXB_RBUS_REGISTER_H__

#include <amxc/amxc_macros.h>
int test_amxb_rbus_register_setup(UNUSED void** state);
int test_amxb_rbus_register_teardown(UNUSED void** state);

void test_rbus_can_register_dm(UNUSED void** state);
void test_rbus_new_objects_are_registered_template_instance(UNUSED void** state);
void test_rbus_new_root_objects_are_registered(UNUSED void** state);
void test_rbus_removed_objects_are_unregistered(UNUSED void** state);
#endif // __TEST_AMXB_RBUS_REGISTER_H__
