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
#include <sys/ioctl.h>

#include "mocks.h"

struct winsize terminal_ws;

void read_sigalrm(void) {
    sigset_t mask;
    int sfd;
    struct signalfd_siginfo fdsi;
    ssize_t s;

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);

    sigprocmask(SIG_BLOCK, &mask, NULL);

    sfd = signalfd(-1, &mask, 0);
    s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
    assert_int_equal(s, sizeof(struct signalfd_siginfo));
    if(fdsi.ssi_signo == SIGALRM) {
        printf("Got SIGALRM\n");
    } else {
        printf("Read unexpected signal\n");
    }
}

void test_handle_signals(void) {
    while(amxp_signal_read() == 0) {
        printf(".");
    }
}

void set_terminal_size(int cols, int rows) {
    terminal_ws.ws_col = cols;
    terminal_ws.ws_row = rows;
}

WINDOW* __wrap_initscr(void) {
    return NULL;
}

int __wrap_start_color(void) {
    return 0;
}

int __wrap_cbreak(void) {
    return 0;
}

int __wrap_keypad(UNUSED WINDOW* w, UNUSED bool b) {
    return 0;
}

int __wrap_noecho(void) {
    return 0;
}

int __wrap_resize_term(UNUSED int r, UNUSED int c) {
    return 0;
}

int __wrap_mvwprintw(UNUSED WINDOW* w, UNUSED int r, UNUSED int c, UNUSED const char* txt, ...) {
    return 0;
}

int __wrap_ioctl(UNUSED int fd, unsigned long request, ...) {
    va_list args;

    va_start(args, request);
    if(request == TIOCGWINSZ) {
        struct winsize* ws = (struct winsize*) va_arg(args, struct winsize*);
        ws->ws_col = terminal_ws.ws_col;
        ws->ws_row = terminal_ws.ws_row;
    }
    va_end(args);

    return 0;
}

ssize_t __wrap_amxt_read(UNUSED int fd, char* buf, UNUSED size_t bufsize) {
    int rv = mock();
    unsigned char* bytes = mock_ptr_type(unsigned char*);

    for(int i = 0; i < rv; i++) {
        buf[i] = bytes[i];
    }
    return rv;
}

void test_key_down(void) {
    const unsigned char left[3] = {0x1b, 0x5b, 0x42};

    will_return(__wrap_amxt_read, 3);
    will_return(__wrap_amxt_read, left);
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key_up(void) {
    const unsigned char up[3] = {0x1b, 0x5b, 0x41};

    will_return(__wrap_amxt_read, 3);
    will_return(__wrap_amxt_read, up);
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key_left(void) {
    const unsigned char left[3] = {0x1b, 0x5b, 0x44};

    will_return(__wrap_amxt_read, 3);
    will_return(__wrap_amxt_read, left);
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key_right(void) {
    const unsigned char right[3] = {0x1b, 0x5b, 0x43};

    will_return(__wrap_amxt_read, 3);
    will_return(__wrap_amxt_read, right);
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key_home(void) {
    const unsigned char home[3] = {0x1b, 0x5b, 0x48};

    will_return(__wrap_amxt_read, 3);
    will_return(__wrap_amxt_read, home);
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key_end(void) {
    const unsigned char end[3] = {0x1b, 0x5b, 0x46};

    will_return(__wrap_amxt_read, 3);
    will_return(__wrap_amxt_read, end);
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key_toggle(void) {
    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, " ");
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key_enter(void) {
    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, "\n");
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key_backspace(void) {
    const unsigned char backspace[1] = {0x08};

    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, backspace);
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key(const char* key) {
    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, key);
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key_page_down(void) {
    const unsigned char page_down[4] = {0x1b, 0x5b, 0x36, 0x7e};

    will_return(__wrap_amxt_read, 4);
    will_return(__wrap_amxt_read, page_down);
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key_page_up(void) {
    const unsigned char page_up[4] = {0x1b, 0x5b, 0x35, 0x7e};

    will_return(__wrap_amxt_read, 4);
    will_return(__wrap_amxt_read, page_up);
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}

void test_key_tab(void) {
    const unsigned char tab[1] = {0x09};

    will_return(__wrap_amxt_read, 1);
    will_return(__wrap_amxt_read, tab);
    assert_int_equal(amxtui_screen_read(), 0);
    test_handle_signals();
}
