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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "amxo_parser_priv.h"

static amxc_htable_t resolvers;

int amxo_parser_resolve(amxo_parser_t* parser,
                        const char* resolver_name,
                        const char* func_name,
                        amxo_fn_type_t type,
                        const char* data) {
    int retval = -1;
    amxo_resolver_t* resolver = NULL;
    amxc_htable_it_t* hit = NULL;
    amxc_string_t res_name;
    amxc_string_init(&res_name, 0);

    if(amxc_string_set_resolved(&res_name, resolver_name, &parser->config) > 0) {
        resolver_name = amxc_string_get(&res_name, 0);
    }

    hit = amxc_htable_get(&resolvers, resolver_name);
    when_null(hit, exit);

    parser->status = amxd_status_ok;
    resolver = amxc_htable_it_get_data(hit, amxo_resolver_t, hit);
    parser->resolved_fn = resolver->resolve(parser,
                                            func_name,
                                            type,
                                            data,
                                            resolver->priv);
    if(parser->status == amxd_status_ok) {
        retval = parser->resolved_fn == NULL ? 1 : 0;
    } else {
        retval = -2;
    }

exit:
    amxc_string_clean(&res_name);
    return retval;
}

amxc_htable_t* amxo_parser_get_resolvers(void) {
    return &resolvers;
}

void amxo_parser_clean_resolvers(amxo_parser_t* parser) {
    amxc_htable_it_t* hit_data = NULL;
    amxc_htable_it_t* hit_resolver = NULL;

    when_null(parser->resolvers, exit);

    hit_data = amxc_htable_get_first(parser->resolvers);
    while(hit_data) {
        const char* key = amxc_htable_it_get_key(hit_data);
        amxo_resolver_t* resolver = NULL;
        hit_resolver = amxc_htable_get(&resolvers, key);
        if(hit_resolver != NULL) {
            resolver = amxc_htable_it_get_data(hit_resolver, amxo_resolver_t, hit);
            if(resolver->clean != NULL) {
                resolver->clean(parser, resolver->priv);
            } else {
                amxo_parser_remove_resolver_data(parser, key);
            }
        } else {
            amxo_parser_remove_resolver_data(parser, key);
        }
        hit_data = amxc_htable_get_first(parser->resolvers);
    }

exit:
    return;
}

void amxo_parser_init_resolvers(amxo_parser_t* parser) {
    amxc_htable_for_each(hit, (&resolvers)) {
        amxo_resolver_t* resolver =
            amxc_htable_it_get_data(hit, amxo_resolver_t, hit);
        if(resolver->get != NULL) {
            resolver->get(parser, resolver->priv);
        }
    }

    return;
}

int amxo_register_resolver(const char* name,
                           amxo_resolver_t* resolver) {
    int retval = -1;
    when_null(resolver, exit);
    when_true(!amxd_name_is_valid(name), exit);
    when_not_null(amxc_htable_get(&resolvers, name), exit);
    when_null(resolver->resolve, exit);

    amxc_htable_insert(&resolvers, name, &resolver->hit);

    retval = 0;

exit:
    return retval;
}

int amxo_unregister_resolver(const char* name) {
    int retval = -1;
    amxc_htable_it_t* hit = NULL;
    when_str_empty(name, exit);

    hit = amxc_htable_get(&resolvers, name);
    when_null(hit, exit);
    amxc_htable_it_clean(hit, NULL);

    retval = 0;
exit:
    return retval;
}

amxc_htable_t* amxo_parser_claim_resolver_data(amxo_parser_t* parser,
                                               const char* resolver_name) {
    amxc_htable_t* data_table = NULL;
    amxo_res_data_t* resolver_data = NULL;
    amxc_htable_it_t* it = NULL;
    when_null(parser, exit);
    when_str_empty(resolver_name, exit);
    when_null(amxc_htable_get(&resolvers, resolver_name), exit);

    if(parser->resolvers == NULL) {
        amxc_htable_new(&parser->resolvers, 10);
        when_null(parser->resolvers, exit);
    }

    it = amxc_htable_get(parser->resolvers, resolver_name);
    if(it == NULL) {
        resolver_data = (amxo_res_data_t*) calloc(1, sizeof(amxo_res_data_t));
        when_null(resolver_data, exit);
        amxc_htable_init(&resolver_data->data, 10);
        amxc_htable_insert(parser->resolvers, resolver_name, &resolver_data->hit);
    } else {
        resolver_data = amxc_htable_it_get_data(it, amxo_res_data_t, hit);
    }

    data_table = &resolver_data->data;

exit:
    return data_table;
}

amxc_htable_t* amxo_parser_get_resolver_data(amxo_parser_t* parser,
                                             const char* resolver_name) {
    amxc_htable_t* data_table = NULL;
    amxo_res_data_t* resolver_data = NULL;
    amxc_htable_it_t* it = NULL;
    when_null(parser, exit);
    when_str_empty(resolver_name, exit);
    when_null(parser->resolvers, exit);
    when_null(amxc_htable_get(&resolvers, resolver_name), exit);

    it = amxc_htable_get(parser->resolvers, resolver_name);
    when_null(it, exit);
    resolver_data = amxc_htable_it_get_data(it, amxo_res_data_t, hit);
    data_table = &resolver_data->data;

exit:
    return data_table;
}

void amxo_parser_remove_resolver_data(amxo_parser_t* parser,
                                      const char* resolver_name) {
    amxo_res_data_t* resolver_data = NULL;
    amxc_htable_it_t* it = NULL;
    when_null(parser, exit);
    when_str_empty(resolver_name, exit);
    when_null(parser->resolvers, exit);
    when_null(amxc_htable_get(&resolvers, resolver_name), exit);

    it = amxc_htable_get(parser->resolvers, resolver_name);
    when_null(it, exit);
    resolver_data = amxc_htable_it_get_data(it, amxo_res_data_t, hit);
    amxc_htable_it_take(&resolver_data->hit);
    amxc_htable_it_clean(&resolver_data->hit, NULL);
    free(resolver_data);

exit:
    return;
}

CONSTRUCTOR_LVL(101) static void amxo_resolvers_init(void) {
    amxc_htable_init(&resolvers, 10);
}

DESTRUCTOR_LVL(101) static void amxo_resolvers_cleanup(void) {
    amxc_htable_clean(&resolvers, NULL);
}
