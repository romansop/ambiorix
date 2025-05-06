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

#include "../../include_priv/amxtui_priv.h"
#include "../mocks/mocks.h"

#include "test_amxtui_screen.h"

amxtui_ctrl_t ctrl_screen;

static bool test_screen_handle_ctrl_key(UNUSED amxtui_widget_t* widget,
                                        UNUSED amxtui_ctrl_t* ctrl,
                                        uint32_t ctrl_key) {
    bool handled = true;

    switch(ctrl_key) {
    case amxt_key_shift_up:
    case amxt_key_ctrl_up:
        break;
    default:
        handled = false;
        break;
    }
    return handled;
}

static bool test_screen_handle_key(UNUSED amxtui_widget_t* widget,
                                   UNUSED amxtui_ctrl_t* ctrl,
                                   char key) {
    check_expected(key);
    return (bool) mock();
}

void test_amxtui_screen_init(UNUSED void** state) {
    set_terminal_size(120, 80);

    assert_int_equal(amxtui_screen_init(), 0);

    assert_int_equal(amxtui_screen_width(), 120);
    assert_int_equal(amxtui_screen_height(), 80);

    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_screen_handles_resize(UNUSED void** state) {
    assert_int_equal(amxtui_screen_init(), 0);

    set_terminal_size(80, 60);

    assert_true(amxtui_screen_resized());
    assert_int_equal(amxtui_screen_width(), 80);
    assert_int_equal(amxtui_screen_height(), 60);

    set_terminal_size(120, 80);
    assert_true(amxtui_screen_resized());
    assert_int_equal(amxtui_screen_width(), 120);
    assert_int_equal(amxtui_screen_height(), 80);

    assert_false(amxtui_screen_resized());
    assert_int_equal(amxtui_screen_width(), 120);
    assert_int_equal(amxtui_screen_height(), 80);

    set_terminal_size(80, 60);
    amxp_sigmngr_trigger_signal(NULL, strsignal(SIGWINCH), NULL);
    assert_int_equal(amxtui_screen_width(), 80);
    assert_int_equal(amxtui_screen_height(), 60);

    set_terminal_size(120, 80);

    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_screen_can_redraw(UNUSED void** state) {
    assert_int_equal(amxtui_screen_init(), 0);
    assert_int_equal(amxtui_screen_redraw(), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_screen_can_add_widget(UNUSED void** state) {
    amxtui_widget_t* widget = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    assert_int_equal(amxtui_widget_new(&widget, NULL, 100, amxtui_percentage, 100, amxtui_percentage), 0);
    assert_non_null(widget);

    assert_int_equal(amxtui_screen_add_widget(widget), 0);

    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_widget_width(widget), amxtui_screen_width() - 2); // subsctract border
    assert_int_equal(amxtui_widget_height(widget), amxtui_screen_height() - 2);

    assert_int_equal(amxtui_screen_remove_widget(widget), 0);

    assert_int_equal(amxtui_widget_delete(&widget), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_screen_can_add_child_widgets(UNUSED void** state) {
    amxtui_widget_t* widget1 = NULL;
    amxtui_widget_t* widget2 = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    assert_int_equal(amxtui_widget_new(&widget1, NULL, 100, amxtui_percentage, 100, amxtui_percentage), 0);
    assert_non_null(widget1);
    assert_int_equal(amxtui_widget_new(&widget2, NULL, 100, amxtui_percentage, 100, amxtui_percentage), 0);
    assert_non_null(widget2);

    assert_int_equal(amxtui_screen_add_widget(widget1), 0);
    assert_int_equal(amxtui_widget_add_widget(widget1, widget2), 0);

    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_widget_width(widget1), amxtui_screen_width() - 2);        // subsctract border
    assert_int_equal(amxtui_widget_height(widget1), amxtui_screen_height() - 2);
    assert_int_equal(amxtui_widget_width(widget2), amxtui_widget_width(widget1) - 2); // subsctract border
    assert_int_equal(amxtui_widget_height(widget2), amxtui_widget_height(widget1) - 2);

    assert_int_equal(amxtui_screen_set_focus(widget2), 0);

    assert_int_equal(amxtui_screen_remove_widget(widget1), 0);

    assert_int_equal(amxtui_widget_delete(&widget2), 0);
    assert_int_equal(amxtui_widget_delete(&widget1), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_screen_can_position_widget(UNUSED void** state) {
    amxtui_widget_t* widget1 = NULL;
    amxtui_widget_t* widget2 = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    assert_int_equal(amxtui_widget_new(&widget1, NULL, 50, amxtui_percentage, 100, amxtui_percentage), 0);
    assert_non_null(widget1);
    assert_int_equal(amxtui_widget_new(&widget2, NULL, 50, amxtui_percentage, 100, amxtui_percentage), 0);
    assert_non_null(widget2);

    assert_int_equal(amxtui_screen_add_widget(widget1), 0);
    assert_int_equal(amxtui_screen_add_widget(widget2), 0);

    assert_int_equal(amxtui_widget_set_pos(widget1, amxtui_allign_left, amxtui_alligned, 0, amxtui_absolute), 0);
    assert_int_equal(amxtui_widget_set_pos(widget2, amxtui_allign_right, amxtui_alligned, 0, amxtui_absolute), 0);

    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_widget_width(widget1), (amxtui_screen_width() / 2) - 2); // subsctract border
    assert_int_equal(amxtui_widget_height(widget1), amxtui_screen_height() - 2);
    assert_int_equal(amxtui_widget_width(widget2), (amxtui_screen_width() / 2) - 2); // subsctract border
    assert_int_equal(amxtui_widget_height(widget2), amxtui_screen_height() - 2);

    assert_int_equal(amxtui_widget_set_box(widget2, false), 0);
    assert_int_equal(amxtui_screen_redraw(), 0);
    assert_int_equal(amxtui_widget_width(widget2), (amxtui_screen_width() / 2)); // subsctract border
    assert_int_equal(amxtui_widget_height(widget2), amxtui_screen_height());

    assert_int_equal(amxtui_widget_screen_pos_x(widget1), 0);
    assert_int_equal(amxtui_widget_screen_pos_x(widget2), (amxtui_screen_width() / 2));

    assert_int_equal(amxtui_screen_remove_widget(widget1), 0);
    assert_int_equal(amxtui_screen_remove_widget(widget2), 0);

    assert_int_equal(amxtui_widget_delete(&widget1), 0);
    assert_int_equal(amxtui_widget_delete(&widget2), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_screen_can_set_focus(UNUSED void** state) {
    amxtui_widget_t* widget1 = NULL;
    amxtui_widget_t* widget2 = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    assert_int_equal(amxtui_widget_new(&widget1, NULL, 50, amxtui_percentage, 100, amxtui_percentage), 0);
    assert_non_null(widget1);
    assert_int_equal(amxtui_widget_new(&widget2, NULL, 50, amxtui_percentage, 100, amxtui_percentage), 0);
    assert_non_null(widget2);

    assert_int_equal(amxtui_screen_add_widget(widget1), 0);
    assert_int_equal(amxtui_screen_add_widget(widget2), 0);

    assert_int_equal(amxtui_widget_set_pos(widget1, amxtui_allign_left, amxtui_alligned, 0, amxtui_absolute), 0);
    assert_int_equal(amxtui_widget_set_pos(widget2, amxtui_allign_right, amxtui_alligned, 0, amxtui_absolute), 0);

    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_screen_set_focus(widget1), 0);
    assert_true(amxtui_widget_has_focus(widget1));
    assert_false(amxtui_widget_has_focus(widget2));
    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_screen_set_focus(widget2), 0);
    assert_true(amxtui_widget_has_focus(widget2));
    assert_false(amxtui_widget_has_focus(widget1));
    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_screen_remove_widget(widget1), 0);
    assert_int_equal(amxtui_screen_remove_widget(widget2), 0);

    assert_int_equal(amxtui_widget_delete(&widget1), 0);
    assert_int_equal(amxtui_widget_delete(&widget2), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_screen_can_show_and_hide(UNUSED void** state) {
    amxtui_widget_t* widget1 = NULL;
    amxtui_widget_t* widget2 = NULL;

    assert_int_equal(amxtui_screen_init(), 0);

    assert_int_equal(amxtui_widget_new(&widget1, NULL, 50, amxtui_percentage, 100, amxtui_percentage), 0);
    assert_non_null(widget1);
    assert_int_equal(amxtui_widget_new(&widget2, NULL, 50, amxtui_percentage, 100, amxtui_percentage), 0);
    assert_non_null(widget2);

    assert_int_equal(amxtui_screen_add_widget(widget1), 0);
    assert_int_equal(amxtui_screen_add_widget(widget2), 0);

    assert_int_equal(amxtui_widget_set_pos(widget1, amxtui_allign_left, amxtui_alligned, 0, amxtui_absolute), 0);
    assert_int_equal(amxtui_widget_set_pos(widget2, amxtui_allign_right, amxtui_alligned, 0, amxtui_absolute), 0);

    assert_int_equal(amxtui_screen_redraw(), 0);
    assert_true(widget1->attrs.is_visible);
    assert_true(widget2->attrs.is_visible);

    assert_int_equal(amxtui_screen_hide(widget1), 0);
    assert_false(widget1->attrs.is_visible);
    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_screen_show(widget1), 0);
    assert_true(widget1->attrs.is_visible);
    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_screen_remove_widget(widget1), 0);
    assert_int_equal(amxtui_screen_remove_widget(widget2), 0);

    assert_int_equal(amxtui_widget_delete(&widget1), 0);
    assert_int_equal(amxtui_widget_delete(&widget2), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_screen_can_read_input(UNUSED void** state) {
    amxtui_widget_t* widget1 = NULL;
    const unsigned char bytes[3] = {0x1b, 0x5b, 0x44};

    assert_int_equal(amxtui_screen_init(), 0);
    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_widget_new(&widget1, NULL, 100, amxtui_percentage, 100, amxtui_percentage), 0);

    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, "a");
    assert_int_equal(amxtui_screen_read(), 0);

    assert_int_equal(amxtui_screen_add_widget(widget1), 0);
    assert_int_equal(amxtui_widget_set_pos(widget1, amxtui_allign_left, amxtui_alligned, 0, amxtui_absolute), 0);
    assert_int_equal(amxtui_screen_set_focus(widget1), 0);
    assert_int_equal(amxtui_screen_redraw(), 0);

    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, "a");
    assert_int_equal(amxtui_screen_read(), 0);

    will_return(__wrap_amxt_read, 3);
    will_return(__wrap_amxt_read, bytes);
    assert_int_equal(amxtui_screen_read(), 0);

    assert_int_equal(amxtui_widget_delete(&widget1), 0);
    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_screen_can_show_message_box(UNUSED void** state) {
    amxtui_widget_t* widget1 = NULL;
    const unsigned char bytes[3] = {0x1b, 0x5b, 0x44};

    assert_int_equal(amxtui_screen_init(), 0);
    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_widget_new(&widget1, NULL, 100, amxtui_percentage, 100, amxtui_percentage), 0);
    assert_int_equal(amxtui_screen_add_widget(widget1), 0);
    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_screen_show_message("Test", NULL, amxtui_message_normal, amxtui_allign_center), 0);

    will_return(__wrap_amxt_read, 3);
    will_return(__wrap_amxt_read, bytes);
    assert_int_equal(amxtui_screen_read(), 0);

    while(amxp_signal_read() == 0) {
        printf(".");
    }

    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, "\n");
    assert_int_equal(amxtui_screen_read(), 0);

    while(amxp_signal_read() == 0) {
        printf(".");
    }

    assert_int_equal(amxtui_screen_show_message("Test", widget1, amxtui_message_normal, amxtui_allign_center), 0);

    set_terminal_size(80, 60);
    amxp_sigmngr_trigger_signal(NULL, strsignal(SIGWINCH), NULL);

    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, "\n");
    assert_int_equal(amxtui_screen_read(), 0);

    while(amxp_signal_read() == 0) {
        printf(".");
    }
    printf("\n");

    set_terminal_size(120, 80);
    assert_int_equal(amxtui_screen_show_message("Test", NULL, amxtui_message_warning, amxtui_allign_center), 0);

    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, "\n");
    assert_int_equal(amxtui_screen_read(), 0);

    while(amxp_signal_read() == 0) {
        printf(".");
    }

    assert_int_equal(amxtui_screen_show_message("Test", NULL, amxtui_message_error, amxtui_allign_center), 0);

    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, "\n");
    assert_int_equal(amxtui_screen_read(), 0);

    while(amxp_signal_read() == 0) {
        printf(".");
    }

    assert_int_equal(amxtui_screen_show_message("Test", NULL, 710, amxtui_allign_center), 0);

    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, "\n");
    assert_int_equal(amxtui_screen_read(), 0);

    while(amxp_signal_read() == 0) {
        printf(".");
    }

    assert_int_equal(amxtui_screen_show_message("This is a very long message to check if word wrapping is not causing any problems in the code,\n therefor the text must be very long as it should not fit in the width of the message box.\nThis should be on the next line",
                                                NULL, 710, amxtui_allign_center), 0);

    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, "\n");
    assert_int_equal(amxtui_screen_read(), 0);

    while(amxp_signal_read() == 0) {
        printf(".");
    }

    assert_int_equal(amxtui_widget_delete(&widget1), 0);

    assert_int_equal(amxtui_screen_cleanup(), 0);
}

void test_amxtui_screen_can_set_screen_ctrl(UNUSED void** state) {
    const unsigned char bytes[3] = {0x1b, 0x5b, 0x44};
    assert_int_equal(amxtui_screen_init(), 0);
    assert_int_equal(amxtui_screen_redraw(), 0);

    assert_int_equal(amxtui_ctrl_init(&ctrl_screen,
                                      NULL,
                                      test_screen_handle_ctrl_key,
                                      test_screen_handle_key,
                                      NULL), 0);
    assert_int_equal(amxtui_screen_set_controller(&ctrl_screen), 0);

    will_return(__wrap_amxt_read, 3);
    will_return(__wrap_amxt_read, bytes);
    assert_int_equal(amxtui_screen_read(), 0);

    while(amxp_signal_read() == 0) {
        printf(".");
    }

    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, "a");

    will_return(test_screen_handle_key, true);
    expect_value(test_screen_handle_key, key, 'a');

    assert_int_equal(amxtui_screen_read(), 0);


    while(amxp_signal_read() == 0) {
        printf(".");
    }

    assert_int_equal(amxtui_screen_cleanup(), 0);
    assert_int_equal(amxtui_ctrl_clean(&ctrl_screen), 0);

}
