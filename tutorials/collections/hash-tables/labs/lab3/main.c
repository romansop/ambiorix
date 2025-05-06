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
#include <string.h>

#include <amxc/amxc.h>

#define SIZE_OF_ARRAY(x) (sizeof(x) / sizeof((x)[0]))

#define FIELD_FIRST_NAME 0
#define FIELD_LAST_NAME  1
#define FIELD_GENDER     2
#define FIELD_AGE        3
#define FIELD_EMAIL      4
#define FIELD_PHONE      5

static char* records[] = {
    "Violet,Elliott,Female,25,v.elliott@randatmail.com,896-3242-73",
    "Dainton,Harris,Male,21,d.harris@randatmail.com,116-4182-47",
    "Max,Elliott,Male,25,m.elliott@randatmail.com,406-1711-61",
    "Deanna,Cole,Female,30,d.cole@randatmail.com,929-0326-86",
    "James,Elliott,Male,23,j.elliott@randatmail.com,412-8887-99",
};

typedef enum _gender {
    male,
    female,
    unknown
} gender_t;

typedef struct person {
    char* first_name;
    char* last_name;
    gender_t gender;
    uint32_t age;
    char* email;
    char* phone;
    amxc_htable_it_t it;
} person_t;

static void lab3_delete_data(const char* key, amxc_htable_it_t* hit) {
    person_t* data = amxc_container_of(hit, person_t, it);
    amxc_htable_it_take(hit);
    free(data->first_name);
    free(data->last_name);
    free(data->email);
    free(data->phone);
    free(data);
}

static void lab3_add_person(amxc_htable_t* contacts, amxc_llist_t* fields) {
    person_t* data = NULL;
    uint32_t index = 0;
    data = calloc(1, sizeof(person_t));

    amxc_llist_for_each(it, fields) {
        amxc_string_t* field = amxc_string_from_llist_it(it);
        switch(index) {
        case FIELD_FIRST_NAME:
            data->first_name = amxc_string_take_buffer(field);
            break;
        case FIELD_LAST_NAME:
            data->last_name = amxc_string_take_buffer(field);
            break;
        case FIELD_GENDER:
            if(strcmp(amxc_string_get(field, 0), "Male") == 0) {
                data->gender = male;
            } else if(strcmp(amxc_string_get(field, 0), "Female") == 0) {
                data->gender = female;
            } else {
                data->gender = unknown;
            }
            break;
        case FIELD_AGE:
            data->age = atoi(amxc_string_get(field, 0));
            break;
        case FIELD_EMAIL:
            data->email = amxc_string_take_buffer(field);
            break;
        case FIELD_PHONE:
            data->phone = amxc_string_take_buffer(field);
            break;
        }
        index++;
    }

    if((data->last_name == NULL) ||
       ( *data->last_name == 0)) {
        printf("ERROR: Contact does not have a last name - will be skipped\n");
        lab3_delete_data(NULL, &data->it);
    } else {
        amxc_htable_insert(contacts, data->last_name, &data->it);
    }
}

static void lab3_parse_records(amxc_htable_t* contacts) {
    amxc_llist_t fields;
    amxc_string_t csv_line;

    amxc_llist_init(&fields);
    amxc_string_init(&csv_line, 0);

    for(int i = 0; i < SIZE_OF_ARRAY(records); i++) {
        amxc_string_set(&csv_line, records[i]);
        if(amxc_string_split_to_llist(&csv_line, &fields, ',') != 0) {
            printf("ERROR in record %d - skipping record\n", i);
        } else {
            lab3_add_person(contacts, &fields);
        }
        amxc_llist_clean(&fields, amxc_string_list_it_free);
    }

    amxc_string_clean(&csv_line);
}

static void lab3_print_person(person_t* person) {
    printf("Person : %s %s\n", person->first_name, person->last_name);

    switch(person->gender) {
    case male:
        printf("\tGender = Male\n");
        break;
    case female:
        printf("\tGender = Female\n");
        break;
    case unknown:
        printf("\tGender = Unknown\n");
        break;
    }

    printf("\tAge    = %d\n", person->age);
    printf("\tE-Mail = %s\n", person->email);
    printf("\tPhone  = %s\n", person->phone);
}

static person_t* lab3_search(amxc_htable_t* contacts,
                             person_t* reference,
                             const char* name) {
    // TODO: implement search
}

void main(int argc, char* argv[]) {
    amxc_htable_t contacts;
    person_t* person = NULL;

    amxc_htable_init(&contacts, 17);

    if(argc != 2) {
        fprintf(stderr, "Usage: %s <last name>\n", argv[0]);
        goto leave;
    }

    lab3_parse_records(&contacts);

    person = lab3_search(&contacts, NULL, argv[1]);
    if(person != NULL) {
        while(person != NULL) {
            lab3_print_person(person);
            person = lab3_search(&contacts, person, NULL);
        }
    } else {
        printf("Not found\n");
    }

leave:
    amxc_htable_clean(&contacts, lab3_delete_data);
}