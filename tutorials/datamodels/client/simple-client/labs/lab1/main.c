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

#include <stdlib.h>
#include <stdio.h>

#include <amxc/amxc.h>
#include <amxp/amxp.h>
#include <amxd/amxd_dm.h>
#include <amxb/amxb.h>

static int app_check_args(int argc, char* argv[]) {
    int retval = 0;

    if(argc < 4) {
        printf("\n\nInvalid number of arguments\n");
        printf("Usage: %s <FIRST NAME> <LAST NAME> <E-MAIL> <PHONE>\n", argv[0]);
        retval = 1;
    }

    return retval;
}

static int app_initialize(amxb_bus_ctx_t** bus_ctx) {
    int retval = 0;
    const char* ba_backend = getenv("AMXB_BACKEND");
    const char* uri = getenv("AMXB_URI");

    if(ba_backend == NULL) {
        printf("No backend defined - set environment variable 'AMXB_BACKEND'\n");
        retval = 2;
        goto leave;
    }

    if(uri== NULL) {
        printf("No URI defined - set environment variable 'AMXB_URI'\n");
        retval = 2;
        goto leave;
    }

    // TODO: Load back-end
    // TODO: Connect to bus system

leave:
    return retval;
}

static uint32_t app_add_contact(amxb_bus_ctx_t* bus_ctx,
                                const char* first_name,
                                const char* last_name) {
    uint32_t index = 0;
    int rv = 0;
    const char* object = "Phonebook.Contact.";
    amxc_var_t ret;
    amxc_var_t values;
    amxc_var_t* var_index = NULL;

    amxc_var_init(&ret);
    amxc_var_init(&values);
    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "FirstName", first_name);
    amxc_var_add_key(cstring_t, &values, "LastName", last_name);

    // TODO: Add contact
    amxc_var_dump(&ret, STDOUT_FILENO);
    // TODO: Get instance number (instance index)

leave:
    amxc_var_clean(&ret);
    amxc_var_clean(&values);
    return index;
}

static int app_add_email(amxb_bus_ctx_t* bus_ctx,
                         uint32_t index,
                         const char* email) {
    int rv = 0;
    amxc_string_t object;
    amxc_var_t ret;
    amxc_var_t values;

    amxc_string_init(&object, 0);
    amxc_var_init(&ret);
    amxc_var_init(&values);

    amxc_string_setf(&object, "Phonebook.Contact.%d.E-Mail.", index);

    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "E-Mail", email);

    rv = amxb_add(bus_ctx, amxc_string_get(&object, 0), 0, NULL, &values, &ret, 5);
    if(rv != 0) {
        printf("Add failed - retval = %d\n", rv);
        goto leave;
    }

leave:
    amxc_string_clean(&object);
    amxc_var_clean(&ret);
    amxc_var_clean(&values);
    return rv;
}

static int app_add_phone(amxb_bus_ctx_t* bus_ctx,
                         uint32_t index,
                         const char* phone) {
    int rv = 0;
    amxc_string_t object;
    amxc_var_t ret;
    amxc_var_t values;

    amxc_string_init(&object, 0);
    amxc_var_init(&ret);
    amxc_var_init(&values);

    amxc_string_setf(&object, "Phonebook.Contact.%d.PhoneNumber.", index);

    amxc_var_set_type(&values, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(cstring_t, &values, "Phone", phone);

    rv = amxb_add(bus_ctx, amxc_string_get(&object, 0), 0, NULL, &values, &ret, 5);
    if(rv != 0) {
        printf("Add failed - retval = %d\n", rv);
        goto leave;
    }

leave:
    amxc_string_clean(&object);
    amxc_var_clean(&ret);
    amxc_var_clean(&values);
    return rv;
}

int main(int argc, char* argv[]) {
    int retval = 0;
    uint32_t index = 0;
    amxb_bus_ctx_t* bus_ctx = NULL;

    retval = app_check_args(argc, argv);
    if (retval != 0) {
        goto leave;
    }

    retval = app_initialize(&bus_ctx);
    if (retval != 0) {
        goto leave;
    }

    index = app_add_contact(bus_ctx, argv[1], argv[2]);
    if (index == 0) {
        goto leave;
    }

    retval = app_add_email(bus_ctx, index, argv[3]);
    if (retval != 0) {
        goto leave;
    }

    retval = app_add_phone(bus_ctx, index, argv[4]);
    if (retval != 0) {
        goto leave;
    }

leave:
    amxb_free(&bus_ctx);
    amxb_be_remove_all();
    return retval;
}
