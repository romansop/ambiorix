/****************************************************************************
**
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
** SPDX-FileCopyrightText: Copyright (c) 2024 SoftAtHome
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
#include "../include/wrap.h"
#include "../include/amxb_rbus_test.h"

static bool check_expected = false;
static bool check_mock = false;

typedef void (* enable_check_t) (bool enable);

void cmocka_enable_check(bool enable) {
    check_expected = enable;
}

void cmocka_enable_mock(bool enable) {
    check_mock = enable;
}

void cmocka_rbus_backend_enable_check(const char* so, bool enable) {
    void* handle = dlopen(so, RTLD_LAZY);
    enable_check_t fn = NULL;

    if(handle != NULL) {
        fn = (enable_check_t) dlsym(handle, "cmocka_enable_check");
        if(fn != NULL) {
            fn(enable);
        }
        dlclose(handle);
    }
}

void cmocka_rbus_backend_enable_mock(const char* so, bool enable) {
    void* handle = dlopen(so, RTLD_LAZY);
    enable_check_t fn = NULL;

    if(handle != NULL) {
        fn = (enable_check_t) dlsym(handle, "cmocka_enable_mock");
        if(fn != NULL) {
            fn(enable);
        }
        dlclose(handle);
    }
}

// Wrapping rbus function, this will enable checking if the correct arguments are passed
// or make it possible to let the rbus API fail in a controlled manner
rbusError_t __wrap_rbus_open(rbusHandle_t* handle, char const* componentName) {
    if(check_expected) {
        check_expected(componentName);
    }

    if(check_mock) {
        return mock();
    } else {
        return __real_rbus_open(handle, componentName);
    }
}

rbusError_t __wrap_rbus_close(rbusHandle_t handle) {
    rbusError_t rv = RBUS_ERROR_SUCCESS;
    rv = __real_rbus_close(handle);
    if(check_mock) {
        rv = mock();
    }

    return rv;
}

rbusError_t __wrap_rbus_regDataElements(rbusHandle_t handle, int numDataElements, rbusDataElement_t* elements) {
    printf("registering [%s]\n", elements[0].name);
    if(check_expected) {
        char* name = elements[0].name;
        check_expected(name);
    }
    return __real_rbus_regDataElements(handle, numDataElements, elements);
}

rbusError_t __wrap_rbus_unregDataElements(rbusHandle_t handle, int numDataElements, rbusDataElement_t* elements) {
    printf("unregistering [%s]\n", elements[0].name);
    if(check_expected) {
        char* name = elements[0].name;
        check_expected(name);
    }
    return __real_rbus_unregDataElements(handle, numDataElements, elements);
}

rbusError_t __wrap_rbusTable_registerRow(rbusHandle_t handle, char const* tableName, uint32_t instNum, char const* aliasName) {
    printf("registering [%s%d.]\n", tableName, instNum);
    if(check_expected) {
        check_expected(tableName);
        check_expected(instNum);
        if(aliasName != NULL) {
            check_expected(aliasName);
        }
    }
    return __real_rbusTable_registerRow(handle, tableName, instNum, aliasName);
}

rbusError_t __wrap_rbusTable_unregisterRow(rbusHandle_t handle, char const* rowName) {
    printf("unregistering [%s]\n", rowName);
    if(check_expected) {
        check_expected(rowName);
    }
    return __real_rbusTable_unregisterRow(handle, rowName);
}

// Wrap some system calls
int __wrap_socketpair(int domain, int type, int protocol, int sv[2]) {
    if(check_mock) {
        return mock();
    } else {
        return __real_socketpair(domain, type, protocol, sv);
    }
}

int __wrap_pthread_create(pthread_t* restrict thread,
                          const pthread_attr_t* restrict attr,
                          void* (*start_routine)(void*),
                          void* restrict arg) {
    if(check_mock) {
        return mock();
    } else {
        return __real_pthread_create(thread, attr, start_routine, arg);
    }
}