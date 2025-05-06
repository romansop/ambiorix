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
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <cmocka.h>
#include <unistd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <string.h>

#include <amxc/amxc.h>
#include <amxp/amxp_signal.h>
#include <amxp/amxp_slot.h>
#include <amxp/amxp_timer.h>

#include <amxd/amxd_common.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>
#include <amxd/amxd_object_event.h>
#include <amxd/amxd_transaction.h>

#include "test_amxd_object_event.h"

#include <amxc/amxc_macros.h>
static amxd_dm_t dm;
static int sfd;
static sigset_t mask;

static void test_event_handler(UNUSED const char* const sig_name,
                               UNUSED const amxc_var_t* const data,
                               void* const priv) {
    int* counter = (int*) priv;
    printf("event recieved %s\n", sig_name);
    (*counter)++;
}

static void test_build_dm(void) {
    amxd_object_t* object = NULL;
    amxd_object_t* child_object = NULL;
    amxd_param_t* param = NULL;
    amxc_var_t* event_data = NULL;

    assert_int_equal(amxd_dm_init(&dm), 0);
    amxc_var_new(&event_data);
    amxc_var_set_type(event_data, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, event_data, "param1", "Text");
    amxc_var_add_key(uint32_t, event_data, "param2", 100);

    assert_int_equal(amxd_object_new(&object, amxd_object_singleton, "MyRootObject"), 0);
    assert_int_equal(amxd_dm_add_root_object(&dm, object), 0);

    assert_int_equal(amxd_object_new(&child_object, amxd_object_singleton, "MyChildObject1"), 0);
    assert_int_equal(amxd_object_add_object(object, child_object), 0);

    assert_int_equal(amxd_param_new(&param, "MyParam1", AMXC_VAR_ID_CSTRING), 0);
    assert_int_equal(amxd_object_add_param(child_object, param), 0);
    assert_int_equal(amxd_param_new(&param, "MyParam2", AMXC_VAR_ID_BOOL), 0);
    assert_int_equal(amxd_object_add_param(child_object, param), 0);
    assert_int_equal(amxd_param_new(&param, "MyParam3", AMXC_VAR_ID_UINT32), 0);
    assert_int_equal(amxd_object_add_param(child_object, param), 0);

    assert_int_equal(amxd_object_new(&child_object, amxd_object_singleton, "MyChildObject2"), 0);
    assert_int_equal(amxd_object_add_object(object, child_object), 0);

    assert_int_equal(amxd_param_new(&param, "MyParam1", AMXC_VAR_ID_CSTRING), 0);
    assert_int_equal(amxd_object_add_param(child_object, param), 0);
    assert_int_equal(amxd_param_new(&param, "MyParam2", AMXC_VAR_ID_BOOL), 0);
    assert_int_equal(amxd_object_add_param(child_object, param), 0);
    assert_int_equal(amxd_param_new(&param, "MyParam3", AMXC_VAR_ID_UINT32), 0);
    assert_int_equal(amxd_object_add_param(child_object, param), 0);
    assert_int_equal(amxd_object_add_event(child_object, "MyEvent1!"), 0);
    assert_int_equal(amxd_object_add_event_ext(child_object, "MyEvent2!", event_data), 0);
}

static int read_sig_alarm(void) {
    while(1) {
        struct signalfd_siginfo si;
        ssize_t res;
        res = read(sfd, &si, sizeof(si));
        assert_false(res < 0);
        assert_false(res != sizeof(si));
        if(si.ssi_signo == SIGALRM) {
            amxp_timers_calculate();
            amxp_timers_check();
            break;
        }
    }

    while(amxp_signal_read() == 0) {
    }

    return 0;
}

int test_object_event_setup(UNUSED void** state) {
    test_build_dm();

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);

    int rv = sigprocmask(SIG_BLOCK, &mask, NULL);
    assert_false(rv < 0);

    sfd = signalfd(-1, &mask, 0);
    assert_false(sfd < 0);

    return 0;
}

int test_object_event_teardown(UNUSED void** state) {
    close(sfd);
    amxd_dm_clean(&dm);

    return 0;
}

void test_can_start_and_stop_periodic_inform(UNUSED void** state) {
    amxd_object_t* object = NULL;
    amxp_timer_t* expire = NULL;
    int counter = 0;

    amxp_timer_new(&expire, NULL, NULL);
    object = amxd_dm_findf(&dm, "MyRootObject.MyChildObject1");

    amxp_timer_start(expire, 5000);

    counter = 0;
    amxp_slot_connect(&dm.sigmngr, "dm:periodic-inform", NULL, test_event_handler, &counter);
    assert_int_equal(amxd_object_new_pi(object, 1), 0);
    read_sig_alarm();
    read_sig_alarm();
    read_sig_alarm();
    assert_int_equal(amxd_object_delete_pi(object), 0);
    assert_int_equal(counter, 3);

    counter = 0;
    read_sig_alarm();
    assert_int_equal(counter, 0);

    while(amxp_signal_read() == 0) {
    }

    amxp_timer_start(expire, 5000);

    assert_int_equal(amxd_object_new_pi(object, 1), 0);
    read_sig_alarm();
    read_sig_alarm();
    read_sig_alarm();
    assert_int_equal(amxd_object_delete_pi(object), 0);
    assert_true(counter >= 2);

    counter = 0;
    read_sig_alarm();
    assert_int_equal(counter, 0);

    amxp_timer_set_interval(expire, 1000);
    amxp_timer_start(expire, 5000);

    counter = 0;
    read_sig_alarm();
    read_sig_alarm();
    read_sig_alarm();
    assert_int_equal(counter, 0);
    amxp_timer_delete(&expire);
    amxp_slot_disconnect(&dm.sigmngr, "dm:periodic-inform", test_event_handler);
}

void test_only_one_periodic_inform_per_object(UNUSED void** state) {
    amxd_object_t* object = NULL;
    amxp_timer_t* expire = NULL;
    int counter = 0;

    amxp_timer_new(&expire, NULL, NULL);
    object = amxd_dm_findf(&dm, "MyRootObject.MyChildObject1");

    amxp_slot_connect(&dm.sigmngr, "dm:periodic-inform", NULL, test_event_handler, &counter);
    assert_int_equal(amxd_object_new_pi(object, 2), 0);
    assert_int_not_equal(amxd_object_new_pi(object, 1), 0);
    read_sig_alarm();
    read_sig_alarm();
    read_sig_alarm();
    assert_int_equal(amxd_object_delete_pi(object), 0);
    assert_true(counter >= 2);

    counter = 0;
    amxp_timer_set_interval(expire, 10000);
    amxp_timer_start(expire, 1000);
    read_sig_alarm();
    read_sig_alarm();
    read_sig_alarm();
    assert_int_equal(counter, 0);
    amxp_timer_delete(&expire);
    amxp_slot_disconnect(&dm.sigmngr, "dm:periodic-inform", test_event_handler);
}

void test_timer_stops_when_object_is_destroyed(UNUSED void** state) {
    amxd_object_t* object = NULL;
    amxp_timer_t* expire = NULL;
    int counter = 0;

    amxp_timer_new(&expire, NULL, NULL);
    object = amxd_dm_findf(&dm, "MyRootObject.MyChildObject1");

    amxp_timer_start(expire, 5000);
    read_sig_alarm();
    assert_int_equal(counter, 0);

    amxp_timer_start(expire, 5000);
    amxp_slot_connect(&dm.sigmngr, "dm:periodic-inform", NULL, test_event_handler, &counter);
    assert_int_equal(amxd_object_new_pi(object, 1), 0);
    read_sig_alarm();
    amxd_object_delete(&object);
    read_sig_alarm();
    assert_int_equal(counter, 1);
    amxp_timer_delete(&expire);
    amxp_slot_disconnect(&dm.sigmngr, "dm:periodic-inform", test_event_handler);
}

void test_events_are_not_duplicated(UNUSED void** state) {
    amxd_trans_t transaction;
    int counter = 0;
    amxd_object_t* child_object = NULL;
    amxd_object_t* child_object2 = NULL;
    amxd_object_t* root = amxd_dm_findf(&dm, "MyRootObject");

    amxp_slot_connect(&dm.sigmngr, "*", NULL, test_event_handler, &counter);

    assert_int_equal(amxd_object_new(&child_object, amxd_object_template, "MyTemplate"), 0);
    assert_int_equal(amxd_object_add_object(root, child_object), 0);
    assert_int_equal(amxd_object_new(&child_object2, amxd_object_singleton, "MySubObject"), 0);
    assert_int_equal(amxd_object_set_attr(child_object2, amxd_oattr_private, true), 0);
    assert_int_equal(amxd_object_add_object(child_object, child_object2), 0);
    assert_int_equal(amxd_object_new(&child_object2, amxd_object_template, "MySubTemplate"), 0);
    assert_int_equal(amxd_object_add_object(child_object, child_object2), 0);

    amxd_trans_init(&transaction);
    amxd_trans_select_pathf(&transaction, "MyRootObject.MyTemplate.");
    amxd_trans_add_inst(&transaction, 0, NULL);
    amxd_trans_select_pathf(&transaction, ".MySubTemplate.");
    amxd_trans_add_inst(&transaction, 0, NULL);
    assert_int_equal(amxd_trans_apply(&transaction, &dm), 0);

    while(amxp_signal_read() == 0) {
    }

    assert_int_equal(counter, 6); // 2 instance add + 4 object added

    amxp_slot_disconnect_all(test_event_handler);

    amxd_trans_clean(&transaction);
}

void test_can_allocate_event_data(UNUSED void** state) {
    amxc_var_t* event_data = NULL;
    amxd_object_t* object = amxd_dm_findf(&dm, "MyRootObject.MyChildObject2.");
    assert_non_null(object);

    event_data = amxd_object_new_event_data(object, "MyEvent1!");
    assert_non_null(event_data);

    assert_int_equal(amxc_var_type_of(event_data), AMXC_VAR_ID_NULL);
    amxc_var_delete(&event_data);

    event_data = amxd_object_new_event_data(object, "MyEvent2!");
    assert_non_null(event_data);
    assert_int_equal(amxc_var_type_of(event_data), AMXC_VAR_ID_HTABLE);

    amxc_var_delete(&event_data);
}

void test_allocate_event_data_fails_when_event_not_found(UNUSED void** state) {
    amxc_var_t* event_data = NULL;
    amxd_object_t* object = amxd_dm_findf(&dm, "MyRootObject.MyChildObject2.");
    assert_non_null(object);

    event_data = amxd_object_new_event_data(object, "MyEvent3!");
    assert_null(event_data);

    event_data = amxd_object_new_event_data(object, NULL);
    assert_null(event_data);

    event_data = amxd_object_new_event_data(object, "");
    assert_null(event_data);

    event_data = amxd_object_new_event_data(NULL, "MyEvent2!");
    assert_null(event_data);
}

void test_can_send_event(UNUSED void** state) {
    amxc_var_t* event_data = NULL;
    uint32_t count = 0;
    amxd_object_t* object = amxd_dm_findf(&dm, "MyRootObject.MyChildObject2.");
    assert_non_null(object);

    while(amxp_signal_read() == 0) {
    }

    event_data = amxd_object_new_event_data(object, "MyEvent2!");
    amxd_object_emit_signal(object, "MyEvent2!", event_data);
    amxc_var_delete(&event_data);

    while(amxp_signal_read() == 0) {
        count++;
    }

    assert_int_equal(count, 1);
}

void test_can_change_event_definition(UNUSED void** state) {
    amxc_var_t* event_data = NULL;
    amxd_object_t* object = amxd_dm_findf(&dm, "MyRootObject.MyChildObject2.");
    assert_non_null(object);

    assert_int_equal(amxd_object_add_event(object, "MyEvent2!"), 0);
    event_data = amxd_object_new_event_data(object, "MyEvent2!");
    assert_non_null(event_data);

    assert_int_equal(amxc_var_type_of(event_data), AMXC_VAR_ID_NULL);
    amxc_var_delete(&event_data);
}

void test_can_remove_event_definition(UNUSED void** state) {
    amxc_var_t* event_data = NULL;
    amxd_object_t* object = amxd_dm_findf(&dm, "MyRootObject.MyChildObject2.");
    assert_non_null(object);

    amxd_object_remove_event(object, "MyEvent2!");
    event_data = amxd_object_new_event_data(object, "MyEvent2!");
    assert_null(event_data);
}

void test_event_changed_is_send(UNUSED void** state) {
    amxd_trans_t transaction;
    int counter = 0;
    amxc_var_t data;
    amxc_var_t* params = NULL;

    amxp_slot_connect(&dm.sigmngr, "dm:object-changed", NULL, test_event_handler, &counter);

    assert_int_equal(amxd_trans_init(&transaction), 0);
    assert_int_equal(amxd_trans_select_pathf(&transaction, "MyRootObject.MyChildObject2."), 0);
    assert_int_equal(amxd_trans_set_value(cstring_t, &transaction, "MyParam1", "Hello world"), 0);
    assert_int_equal(amxd_trans_set_value(bool, &transaction, "MyParam2", true), 0);
    amxd_trans_dump(&transaction, STDOUT_FILENO, false);
    assert_int_equal(amxd_trans_apply(&transaction, &dm), 0);
    amxd_trans_clean(&transaction);

    while(amxp_signal_read() == 0) {
    }
    assert_int_equal(counter, 1);
    counter = 0;

    amxc_var_init(&data);
    amxc_var_set_type(&data, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(uint32_t, &data, "access", amxd_dm_access_public);
    amxc_var_add_key(bool, &data, "set_read_only", false);
    params = amxc_var_add_key(amxc_htable_t, &data, "oparameters", NULL);
    amxc_var_add_key(cstring_t, params, "MyParam1", "Hello Universe");
    amxc_var_add_key(bool, params, "MyParam2", false);

    assert_int_equal(amxd_trans_init(&transaction), 0);
    assert_int_equal(amxd_trans_select_pathf(&transaction, "MyRootObject.MyChildObject2."), 0);
    amxd_trans_add_action(&transaction, action_object_write, &data);
    amxd_trans_dump(&transaction, STDOUT_FILENO, false);
    assert_int_equal(amxd_trans_apply(&transaction, &dm), 0);
    amxd_trans_clean(&transaction);

    while(amxp_signal_read() == 0) {
    }
    assert_int_equal(counter, 1);

    amxc_var_clean(&data);
    amxp_slot_disconnect(&dm.sigmngr, "dm:object-changed", test_event_handler);
}