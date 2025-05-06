/****************************************************************************
**
** Copyright (c) 2020 SoftAtHome
**
** Redistribution and use in source and binary forms, with or
** without modification, are permitted provided that the following
** conditions are met:
**
** 1. Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** 2. Redistributions in binary form must reproduce the above
** copyright notice, this list of conditions and the following
** disclaimer in the documentation and/or other materials provided
** with the distribution.
**
** Subject to the terms and conditions of this license, each
** copyright holder and contributor hereby grants to those receiving
** rights under this license a perpetual, worldwide, non-exclusive,
** no-charge, royalty-free, irrevocable (except for failure to
** satisfy the conditions of this license) patent license to make,
** have made, use, offer to sell, sell, import, and otherwise
** transfer this software, where such license applies only to those
** patent claims, already acquired or hereafter acquired, licensable
** by such copyright holder or contributor that are necessarily
** infringed by:
**
** (a) their Contribution(s) (the licensed copyrights of copyright
** holders and non-copyrightable additions of contributors, in
** source or binary form) alone; or
**
** (b) combination of their Contribution(s) with the work of
** authorship to which such Contribution(s) was added by such
** copyright holder or contributor, if, at the time the Contribution
** is added, such addition causes such combination to be necessarily
** infringed. The patent license shall not apply to any other
** combinations which include the Contribution.
**
** Except as expressly stated above, no rights or licenses from any
** copyright holder or contributor is granted under this license,
** whether expressly, by implication, estoppel or otherwise.
**
** DISCLAIMER
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
** CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
** USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
** AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
** ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>
#include <unistd.h>

#include "test_async_call.h"
#include "main.h"

int __wrap_amxrt_connect(void);
int __wrap_amxrt_el_create(void);
int __real_amxrt_el_create(void);
amxb_request_t* __wrap_amxb_async_call(amxb_bus_ctx_t* const ctx,
                                       const char* object,
                                       const char* method,
                                       amxc_var_t* args,
                                       amxb_be_done_cb_fn_t done_fn,
                                       void* priv);
int __wrap_amxb_close_request(amxb_request_t** request);
amxb_bus_ctx_t* __wrap_amxb_be_who_has(const char* object);

static void complete_async_call(UNUSED const amxc_var_t* const data, void* const priv) {
    amxb_request_t* req = (amxb_request_t*) priv;
    req->done_fn((amxb_bus_ctx_t*) req->bus_data, req, 0, NULL);
    amxc_var_delete(&req->result);
    free(req);
}

int __wrap_amxrt_connect(void) {
    return mock();
}

int __wrap_amxrt_el_create(void) {
    int retval = mock();

    if(retval == 0) {
        return __real_amxrt_el_create();
    } else {
        return retval;
    }
}

amxb_request_t* __wrap_amxb_async_call(amxb_bus_ctx_t* const bus_ctx,
                                       UNUSED const char* object,
                                       UNUSED const char* method,
                                       UNUSED amxc_var_t* args,
                                       amxb_be_done_cb_fn_t done_fn,
                                       UNUSED void* priv) {
    int retval = (int) mock();
    amxb_request_t* req = NULL;


    if(retval == 0) {
        req = calloc(1, sizeof(amxb_request_t));
        req->done_fn = done_fn;
        req->bus_data = bus_ctx;
        amxc_var_new(&req->result);
        amxc_var_init(req->result);
        amxc_var_set_type(req->result, AMXC_VAR_ID_LIST);
        amxp_sigmngr_deferred_call(NULL, complete_async_call, NULL, req);
    }

    return req;
}

int __wrap_amxb_close_request(UNUSED amxb_request_t** request) {
    return mock();
}

amxb_bus_ctx_t* __wrap_amxb_be_who_has(UNUSED const char* object) {
    static amxb_bus_ctx_t fake;
    int retval = mock();

    if(retval == 0) {
        return &fake; // fake address
    } else {
        return NULL;
    }
}

int test_async_call_setup(UNUSED void** state) {
    return 0;
}

int test_async_call_teardown(UNUSED void** state) {
    return 0;
}

void test_fails_when_no_path_given(UNUSED void** state) {
    char* argv[] = {"amx-async-call"};
    assert_int_not_equal(ASYNC_CALL_MAIN(sizeof(argv) / sizeof(argv[0]), argv), 0);
}

void test_can_print_extended_help(UNUSED void** state) {
    char* argv[] = {"amx-async-call", "-H"};
    assert_int_not_equal(ASYNC_CALL_MAIN(sizeof(argv) / sizeof(argv[0]), argv), 0);
}

void test_can_print_help(UNUSED void** state) {
    char* argv[] = {"amx-async-call", "-h"};
    assert_int_not_equal(ASYNC_CALL_MAIN(sizeof(argv) / sizeof(argv[0]), argv), 0);
}

void test_can_do_async_call(UNUSED void** state) {
    char* argv[] = {"amx-async-call", "MyObject.", "myfunction"};
    will_return(__wrap_amxrt_connect, 0);
    will_return(__wrap_amxrt_el_create, 0);
    will_return(__wrap_amxb_be_who_has, 0);
    will_return(__wrap_amxb_async_call, 0);
    will_return(__wrap_amxb_close_request, 0);

    assert_int_equal(ASYNC_CALL_MAIN(sizeof(argv) / sizeof(argv[0]), argv), 0);
}

void test_can_do_async_call_with_args(UNUSED void** state) {
    char* argv[] = {"amx-async-call", "MyObject.", "myfunction", "{\"arg1\":\"Value1\",\"arg2\":100}"};
    will_return(__wrap_amxrt_connect, 0);
    will_return(__wrap_amxrt_el_create, 0);
    will_return(__wrap_amxb_be_who_has, 0);
    will_return(__wrap_amxb_async_call, 0);
    will_return(__wrap_amxb_close_request, 0);

    assert_int_equal(ASYNC_CALL_MAIN(sizeof(argv) / sizeof(argv[0]), argv), 0);
}

void test_async_call_fails(UNUSED void** state) {
    char* argv[] = {"amx-async-call", "MyObject.", "myfunction"};
    will_return(__wrap_amxrt_connect, 0);
    will_return(__wrap_amxrt_el_create, 0);
    will_return(__wrap_amxb_be_who_has, 0);
    will_return(__wrap_amxb_async_call, -1);

    assert_int_not_equal(ASYNC_CALL_MAIN(sizeof(argv) / sizeof(argv[0]), argv), 0);
}

void test_async_call_fails_when_object_not_found(UNUSED void** state) {
    char* argv[] = {"amx-async-call", "MyObject.", "myfunction"};
    will_return(__wrap_amxrt_connect, 0);
    will_return(__wrap_amxrt_el_create, 0);
    will_return(__wrap_amxb_be_who_has, -1);

    assert_int_not_equal(ASYNC_CALL_MAIN(sizeof(argv) / sizeof(argv[0]), argv), 0);
}

void test_can_do_async_call_fails_when_args_are_invalid_json(UNUSED void** state) {
    char* argv[] = {"amx-async-call", "MyObject.", "myfunction", "{arg1=\"Value1\",arg2=100}"};
    assert_int_not_equal(ASYNC_CALL_MAIN(sizeof(argv) / sizeof(argv[0]), argv), 0);
}

void test_async_call_fails_when_failed_to_connect(UNUSED void** state) {
    char* argv[] = {"amx-async-call", "MyObject.", "myfunction"};
    will_return(__wrap_amxrt_connect, -1);

    assert_int_not_equal(ASYNC_CALL_MAIN(sizeof(argv) / sizeof(argv[0]), argv), 0);
}

void test_async_call_fails_can_not_create_el(UNUSED void** state) {
    char* argv[] = {"amx-async-call", "MyObject.", "myfunction"};
    will_return(__wrap_amxrt_connect, 0);
    will_return(__wrap_amxrt_el_create, -1);

    assert_int_not_equal(ASYNC_CALL_MAIN(sizeof(argv) / sizeof(argv[0]), argv), 0);
}
