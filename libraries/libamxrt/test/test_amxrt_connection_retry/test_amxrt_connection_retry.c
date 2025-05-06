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
#include <stdio.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <amxc/amxc_macros.h>
#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxp/amxp_dir.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>
#include <amxb/amxb.h>
#include <amxo/amxo.h>

#include <amxut/amxut_timer.h>

#include "amxrt_priv.h"
#include "amxrt/amxrt.h"

#include "test_amxrt_connection_retry.h"

static int test_add_backend(const char* name, void* priv) {
    amxc_var_t* backends = (amxc_var_t*) priv;
    amxc_var_add(cstring_t, backends, name);

    return 0;
}

int test_amxrt_connection_retry_init(UNUSED void** state) {
    amxrt_new();
    return 0;
}

int test_amxrt_connection_retry_teardown(UNUSED void** state) {
    assert_int_equal(system("killall remote-test-plugin"), 0);
    usleep(100);
    amxrt_delete();
    return 0;
}

void test_can_retry_connection(UNUSED void** state) {
    int index = 0;
    int argc = 2;
    char* argv[] = {"/usr/bin/amxrt", "./test.odl"};
    amxo_parser_t* parser = amxrt_get_parser();
    amxc_var_t* config = amxrt_get_config();
    amxc_var_t* pcb_be = NULL;
    amxc_llist_t* connections = NULL;
    amxp_connection_t* con = NULL;
    amxp_timer_t* retry_timer = amxrt_get_connect_retry_timer();

    assert_int_equal(amxrt_config_init(argc, argv, &index, NULL), 0);
    assert_int_equal(amxrt_load_odl_files(argc, argv, index), 0);

    // Only load pcb backend for test
    amxp_dir_scan(GET_CHAR(config, "backend-dir"), "d_name matches '.*mod-amxb-pcb\\.so'", false,
                  test_add_backend, GET_ARG(config, "backends"));
    pcb_be = GETP_ARG(config, "backends.0");
    assert_non_null(pcb_be);
    amxb_be_load(GET_CHAR(pcb_be, NULL));

    // Connection will fail the first time, because there is nothing to connect to yet
    assert_int_equal(amxrt_connection_connect_all(parser), 0);

    retry_timer = amxrt_get_connect_retry_timer();
    assert_int_equal(amxp_timer_get_state(retry_timer), amxp_timer_running);
    assert_int_equal(amxrt_get_connect_retry_count(), 1);

    // Run app that will create socket
    assert_int_equal(system("../test_remote_plugin/remote-test-plugin ./remote.odl -D"), 0);
    sleep(1);

    // Skip time and retry connection
    connections = amxp_connection_get_connections();
    assert_int_equal(amxc_llist_size(connections), 0);
    amxut_timer_go_to_future_ms(amxp_timer_remaining_time(retry_timer));
    assert_int_equal(amxc_llist_size(connections), 2);
    assert_int_equal(amxp_timer_get_state(retry_timer), amxp_timer_off);

    // Retry count is reset to 0 when everything is connected successfully
    assert_int_equal(amxrt_get_connect_retry_count(), 0);

    // Kill remote service to restart retry mechanism
    assert_int_equal(system("killall remote-test-plugin"), 0);
    usleep(100);

    con = amxc_container_of(amxc_llist_get_first(connections), amxp_connection_t, it);
    assert_non_null(con);

    // Call read function which will fail due to a broken connection
    // Check that connection is removed and retry is started
    con->reader(con->fd, con->priv);
    assert_int_equal(amxc_llist_size(connections), 1);
    assert_int_equal(amxp_timer_get_state(retry_timer), amxp_timer_running);

    // Do the same for 2nd socket
    con = amxc_container_of(amxc_llist_get_first(connections), amxp_connection_t, it);
    assert_non_null(con);
    con->reader(con->fd, con->priv);
    assert_int_equal(amxc_llist_size(connections), 0);
    assert_int_equal(amxp_timer_get_state(retry_timer), amxp_timer_running);

    assert_int_equal(amxrt_get_connect_retry_count(), 1);

    // Go to future
    amxut_timer_go_to_future_ms(amxp_timer_remaining_time(retry_timer));
    assert_int_equal(amxrt_get_connect_retry_count(), 2);

    // Restart process to let reconnect finish
    assert_int_equal(system("../test_remote_plugin/remote-test-plugin ./remote.odl -D"), 0);
    sleep(1);
    amxut_timer_go_to_future_ms(amxp_timer_remaining_time(retry_timer));
    assert_int_equal(amxc_llist_size(connections), 2);
    assert_int_equal(amxp_timer_get_state(retry_timer), amxp_timer_off);
    assert_int_equal(amxrt_get_connect_retry_count(), 0);
}
