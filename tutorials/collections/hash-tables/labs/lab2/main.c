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

#define FIELD_WORD        0
#define FIELD_EXPLANATION 1

static char* words[] = {
    "absent,\"not present at a usual or expected place\"",
    "dangerous,\"involving possible injury, harm, or death : characterized by danger\"",
    "paraphrase,\"to say (something that someone else has said or written) using different words\"",
    "unfortunate,\"not fortunate: such as having bad luck\"",
};

typedef struct _word {
    char* word;
    char* explanation;
    amxc_htable_it_t it;
} word_t;

static void lab2_delete_data(const char* key, amxc_htable_it_t* hit) {
    word_t* data = amxc_container_of(hit, word_t, it);
    amxc_htable_it_take(hit);
    free(data->word);
    free(data->explanation);
    free(data);
}

static void lab2_add_word(amxc_htable_t* dictionary, amxc_llist_t* fields) {
    word_t* data = NULL;
    uint32_t index = 0;
    data = calloc(1, sizeof(word_t));

    amxc_llist_for_each(it, fields) {
        amxc_string_t* field = amxc_string_from_llist_it(it);
        switch(index) {
        case FIELD_WORD:
            data->word = amxc_string_take_buffer(field);
            break;
        case FIELD_EXPLANATION:
            data->explanation = amxc_string_take_buffer(field);
            break;
        }
        index++;
    }

    amxc_htable_insert(dictionary, data->word, &data->it);
}

static void lab2_parse_records(amxc_htable_t* dictionary) {
    amxc_llist_t fields;
    amxc_string_t csv_line;

    amxc_llist_init(&fields);
    amxc_string_init(&csv_line, 0);

    for(int i = 0; i < SIZE_OF_ARRAY(words); i++) {
        amxc_string_set(&csv_line, words[i]);
        if(amxc_string_split_to_llist(&csv_line, &fields, ',') != 0) {
            printf("ERROR in record %d - skipping record\n", i);
        } else {
            lab2_add_word(dictionary, &fields);
        }
        amxc_llist_clean(&fields, amxc_string_list_it_free);
    }

    amxc_string_clean(&csv_line);
}

static void lab2_print_all_keys(amxc_htable_t* dictionary) {
    // TODO: iterate over all items in the hash table and print the key
    // (about 4 lines of code)
}

void main(int argc, char* argv[]) {
    amxc_htable_t dictionary;
    word_t* word = NULL;

    amxc_htable_init(&dictionary, 20);

    lab2_parse_records(&dictionary);

    lab2_print_all_keys(&dictionary);

leave:
    amxc_htable_clean(&dictionary, lab2_delete_data);
}