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

#include <amxc/amxc_variant_type.h>
#include <amxc/amxc_variant.h>

#include <amxp/amxp_syssig.h>
#include <amxp/variant_siginfo.h>
#include <amxp_signal_priv.h>

#include "test_syssig.h"

#include <amxc/amxc_macros.h>
int sig_pipe[20] = { -1, -1 };
int count = 0;

int __wrap_signalfd(int fd, const sigset_t* mask, int flags);

static void test_slot_sigchld(const char* const sig_name,
                              const amxc_var_t* const data,
                              UNUSED void* const priv) {
    const amxp_siginfo_t* siginfo = amxc_var_constcast(amxp_siginfo_t, data);

    assert_ptr_not_equal(siginfo, NULL);
    assert_string_equal(sig_name, strsignal(SIGCHLD));
    assert_int_equal(siginfo->ssi_signo, SIGCHLD);
    assert_int_equal(siginfo->ssi_pid, 5012);

    count++;
}

int __wrap_signalfd(int fd, UNUSED const sigset_t* mask, UNUSED int flags) {
    if(fd == -1) {
        assert_int_equal(pipe(sig_pipe), 0);
    } else {
        assert_int_equal(fd, sig_pipe[0]);
    }

    return sig_pipe[0];
}

void test_syssig_get_fd(UNUSED void** state) {
    int fd = amxp_syssig_get_fd();

    assert_int_equal(fd, sig_pipe[0]);
}

void test_syssig_enable(UNUSED void** state) {
    sigset_t mask;
    assert_int_not_equal(amxp_syssig_enable(AMXP_SYSSIG_MAX + 1, true), 0);

    assert_int_equal(amxp_syssig_enable(SIGINT, true), 0);
    assert_int_equal(amxp_syssig_enable(SIGTERM, true), 0);
    assert_int_equal(amxp_syssig_enable(SIGTERM, true), 0);
    assert_int_not_equal(amxp_syssig_enable(SIGKILL, true), 0);
    assert_int_not_equal(amxp_syssig_enable(SIGSTOP, true), 0);

    assert_true(amxp_syssig_is_enabled(SIGINT));
    assert_true(amxp_syssig_is_enabled(SIGTERM));
    assert_false(amxp_syssig_is_enabled(SIGKILL));
    assert_false(amxp_syssig_is_enabled(SIGSTOP));

    sigprocmask(SIG_BLOCK, NULL, &mask);
    assert_int_equal(sigismember(&mask, SIGINT), 1);
    assert_int_equal(sigismember(&mask, SIGTERM), 1);

    assert_false(amxp_syssig_is_enabled(AMXP_SYSSIG_MAX + 1));

    assert_int_equal(amxp_syssig_enable(SIGINT, false), 0);
    assert_int_equal(amxp_syssig_enable(SIGTERM, false), 0);
    assert_int_equal(amxp_syssig_enable(SIGTERM, false), 0);

    assert_false(amxp_syssig_is_enabled(SIGINT));
    assert_false(amxp_syssig_is_enabled(SIGTERM));

    sigprocmask(SIG_BLOCK, NULL, &mask);
    assert_int_equal(sigismember(&mask, SIGINT), 0);
    assert_int_equal(sigismember(&mask, SIGTERM), 0);
}

void test_syssig_read(UNUSED void** state) {
    amxp_siginfo_t siginfo = {
        .ssi_signo = SIGCHLD,
        .ssi_errno = 0,
        .ssi_code = 123,
        .ssi_pid = 5012,
        .ssi_uid = 100
    };

    count = 0;
    assert_int_equal(amxp_syssig_enable(SIGCHLD, true), 0);
    assert_int_equal(amxp_slot_connect(NULL, strsignal(SIGCHLD), NULL, test_slot_sigchld, NULL), 0);
    assert_int_equal(write(sig_pipe[1], &siginfo, sizeof(amxp_siginfo_t)), sizeof(amxp_siginfo_t));
    assert_int_equal(amxp_syssig_read(), 0);
    assert_int_equal(count, 1);
}
