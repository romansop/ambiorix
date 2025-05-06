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

typedef struct _data {
    uint32_t number;
    amxc_llist_it_t ll_it;
} data_t;

static void lab2_delete_data(amxc_llist_it_t* lit) {
    data_t* d = amxc_container_of(lit, data_t, ll_it);
    free(d);
}

static void lab2_add_number(amxc_llist_t* list, uint32_t number) {
    data_t* d = calloc(1, sizeof(data_t));
    d->number = number;

    amxc_llist_append(list, &d->ll_it);
}

static void lab2_fill_llist(amxc_llist_t* list,
                            uint32_t seed,
                            uint32_t nloops) {
    srand(seed);
    for(int i = 0; i < nloops; i++) {
        lab2_add_number(list, rand() % 100);
    }
}

static void lab2_print_llist(amxc_llist_t* list) {
    amxc_llist_for_each(lit, list) {
        data_t* d = amxc_container_of(lit, data_t, ll_it);
        printf("Number = %d\n", d->number);
    }
}

static void lab2_split_llist(amxc_llist_t* list,
                             amxc_llist_t* odd_numbers,
                             amxc_llist_t* even_numbers) {
    // split the linked list in two separate linked lists
    // one linked list should only contain odd numbers
    // the other linked list should only contain even numbers
}

void main(int argc, char* argv[]) {
    amxc_llist_t llist_numbers;
    amxc_llist_t odd_numbers;
    amxc_llist_t even_numbers;

    amxc_llist_init(&llist_numbers);
    amxc_llist_init(&odd_numbers);
    amxc_llist_init(&even_numbers);

    if(argc != 3) {
        fprintf(stderr, "Usage: %s <seed> <nloops>\n", argv[0]);
        goto leave;
    }

    lab2_fill_llist(&llist_numbers, atoi(argv[1]), atoi(argv[2]));
    printf("All numbers:\n");
    lab2_print_llist(&llist_numbers);

    lab2_split_llist(&llist_numbers, &odd_numbers, &even_numbers);

    printf("Odd numbers are:\n");
    lab2_print_llist(&odd_numbers);

    printf("Even numbers are:\n");
    lab2_print_llist(&even_numbers);


leave:
    amxc_llist_clean(&llist_numbers, lab2_delete_data);
    amxc_llist_clean(&odd_numbers, lab2_delete_data);
    amxc_llist_clean(&even_numbers, lab2_delete_data);
}