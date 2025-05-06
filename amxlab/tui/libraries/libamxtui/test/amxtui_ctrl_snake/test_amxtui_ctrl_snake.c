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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <inttypes.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxt/amxt_tty.h>

#include <amxtui/amxtui_screen.h>
#include <amxtui/amxtui_widget.h>
#include <amxtui/amxtui_controller.h>
#include <amxtui/ctrl/amxtui_ctrl_snake.h>

#include "../../include_priv/amxtui_priv.h"
#include "../mocks/mocks.h"

#include "test_amxtui_ctrl_snake.h"

amxtui_ctrl_t ctrl_screen;

void test_amxtui_ctrl_snake_create(UNUSED void** state) {
    amxtui_widget_t* widget1 = NULL;
    amxtui_widget_t* widget2 = NULL;
    amxtui_ctrl_t* ctrl = NULL;

    set_terminal_size(80, 50);
    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget1,
                      "snake",
                      100, amxtui_percentage,
                      100, amxtui_percentage);
    amxtui_widget_new(&widget2,
                      NULL,
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget1), 0);
    assert_int_equal(amxtui_screen_add_widget(widget2), 0);
    amxtui_screen_redraw();

    ctrl = amxtui_widget_get_ctrl(widget1);
    assert_non_null(ctrl);
    amxtui_widget_set_ctrl(widget2, ctrl);
    assert_true(amxtui_ctrl_is_type(ctrl, "snake"));

    assert_int_equal(amxtui_widget_delete(&widget1), 0);
    amxtui_screen_redraw();

    ctrl = amxtui_widget_get_ctrl(widget2);
    assert_non_null(ctrl);
    assert_true(amxtui_ctrl_is_type(ctrl, "snake"));

    assert_int_equal(amxtui_widget_delete(&widget2), 0);

    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_ctrl_snake_control(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;
    amxtui_ctrl_t* ctrl = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    amxtui_widget_new(&widget,
                      "snake",
                      100, amxtui_percentage,
                      100, amxtui_percentage);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);
    ctrl = amxtui_widget_get_ctrl(widget);
    assert_non_null(ctrl);
    amxtui_snake_widget_start(widget);
    amxtui_screen_redraw();

    read_sigalrm();
    amxp_timers_calculate();
    amxp_timers_check();

    read_sigalrm();
    amxp_timers_calculate();
    amxp_timers_check();

    read_sigalrm();
    amxp_timers_calculate();
    amxp_timers_check();

    read_sigalrm();
    amxp_timers_calculate();
    amxp_timers_check();

    test_key_up();
    read_sigalrm();
    amxp_timers_calculate();
    amxp_timers_check();

    test_key_left();
    read_sigalrm();
    amxp_timers_calculate();
    amxp_timers_check();

    test_key_up();
    read_sigalrm();
    amxp_timers_calculate();
    amxp_timers_check();

    test_key_right();
    read_sigalrm();
    amxp_timers_calculate();
    amxp_timers_check();

    test_key_down();
    read_sigalrm();
    amxp_timers_calculate();
    amxp_timers_check();

    read_sigalrm();
    amxp_timers_calculate();
    amxp_timers_check();

    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

