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
#include <sys/time.h>
#include <sys/resource.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <inttypes.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <cmocka.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxt/amxt_tty.h>

#include "test_config.h"

#include <amxc/amxc_macros.h>
static char input_data[64] = "";
static bool fail = false;
static int istty = 0;

ssize_t __wrap_read(int fd, void* buf, size_t count);
ssize_t __wrap_write(int fd, void* buf, size_t count);
int __wrap_isatty(int fd);
int __wrap_ttyname_r(int fd, char* buf, size_t buflen);
int __wrap_open(const char* path, int oflag, int mode);
int __real_open(const char* path, int oflag, int mode);

ssize_t __wrap_read(UNUSED int fd, void* buf, size_t count) {
    if(fail) {
        return -1;
    } else {
        size_t length = strlen(input_data);
        ssize_t bytes = count < length ? count : length;
        memcpy(buf, input_data, bytes);
        return bytes;
    }
}

ssize_t __wrap_write(UNUSED int fd, UNUSED void* buf, size_t count) {
    ssize_t bytes = count;
    return bytes;
}

int __wrap_isatty(UNUSED int fd) {
    return istty;
}

int __wrap_ttyname_r(UNUSED int fd, char* buf, size_t buflen) {
    strncpy(buf, "/tmp/test_tty", buflen);

    return 0;
}

int __wrap_open(const char* path, int oflag, int mode) {
    if(strcmp(path, "/tmp/test_tty") == 0) {
        return 0;
    }
    return __real_open(path, oflag, mode);
}

void test_can_claim_or_get_config_option(UNUSED void** state) {
    amxt_tty_t* tty = NULL;
    amxc_var_t* option = NULL;

    istty = 1;
    assert_int_equal(amxt_tty_open(&tty, fileno(stdin)), 0);

    option = amxt_tty_get_config(tty, "option");
    assert_null(option);
    option = amxt_tty_claim_config(tty, "option");
    assert_non_null(option);
    option = amxt_tty_get_config(tty, "option");
    assert_non_null(option);
    assert_ptr_equal(amxt_tty_claim_config(tty, "option"), option);

    assert_null(amxt_tty_claim_config(NULL, "option"));
    assert_null(amxt_tty_claim_config(tty, ""));
    assert_null(amxt_tty_claim_config(tty, NULL));
    assert_null(amxt_tty_claim_config(tty, "P&C"));

    assert_null(amxt_tty_get_config(NULL, "option"));
    assert_null(amxt_tty_get_config(tty, ""));
    assert_null(amxt_tty_get_config(tty, NULL));
    assert_null(amxt_tty_get_config(tty, "P&C"));

    amxt_tty_close(&tty);
}

void test_can_set_config_option(UNUSED void** state) {
    amxt_tty_t* tty = NULL;
    amxc_var_t value;

    amxc_var_init(&value);
    istty = 1;
    assert_int_equal(amxt_tty_open(&tty, fileno(stdin)), 0);

    assert_null(amxt_tty_get_config(tty, "option"));
    assert_int_equal(amxt_tty_set_config(tty, "option", &value), 0);
    assert_non_null(amxt_tty_get_config(tty, "option"));

    assert_int_not_equal(amxt_tty_set_config(NULL, "option", &value), 0);
    assert_int_not_equal(amxt_tty_set_config(tty, "", &value), 0);
    assert_int_not_equal(amxt_tty_set_config(tty, "P&C", &value), 0);
    assert_int_not_equal(amxt_tty_set_config(tty, NULL, &value), 0);
    assert_int_not_equal(amxt_tty_set_config(tty, "option", NULL), 0);

    amxt_tty_close(&tty);
}
