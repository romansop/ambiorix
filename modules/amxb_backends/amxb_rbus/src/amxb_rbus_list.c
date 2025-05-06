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

#include <string.h>

#include "amxb_rbus.h"

// Following is giving problems
// #include <rbuscore.h>
//
// For now just declare the needed function here (returning int instead of rbusCoreError_t)
int rbus_discoverRegisteredComponents(int* count, char*** components);

static int amxb_rbus_get_component_elements(amxb_rbus_t* amxb_rbus_ctx, amxb_request_t* request, const char* component) {
    int retval = 0;
    int num_elements = 0;
    char** element_names = NULL;
    amxc_var_t names;

    amxc_var_init(&names);
    amxc_var_set_type(&names, AMXC_VAR_ID_HTABLE);

    retval = rbus_discoverComponentDataElements(amxb_rbus_ctx->handle, component, false, &num_elements, &element_names);

    for(int i = 0; i < num_elements; i++) {
        if(strcmp(element_names[i], component) != 0) {
            char* dot_pos = strchr(element_names[i], '.');
            if(dot_pos != NULL) {
                *(dot_pos + 1) = 0;
            }
            amxc_var_add_key(bool, &names, element_names[i], true);
        }
        free(element_names[i]);
    }
    free(element_names);

    amxc_var_for_each(root, &names) {
        amxc_var_add(cstring_t, request->result, amxc_var_key(root));
    }

    if(request->cb_fn != NULL) {
        const amxb_bus_ctx_t* ctx = amxb_request_get_ctx(request);
        request->cb_fn(ctx, request->result, request->priv);
        amxc_var_set_type(request->result, AMXC_VAR_ID_LIST);
    }

    amxc_var_clean(&names);
    return retval;
}

static int amxb_rbus_list_collect_root(amxb_rbus_t* amxb_rbus_ctx, amxb_request_t* request) {
    int retval = 0;
    int componentCnt = 0;
    char** pComponentNames = NULL;
    amxc_llist_t components;

    amxc_llist_init(&components);

    retval = rbus_discoverRegisteredComponents(&componentCnt, &pComponentNames);
    for(int i = 0; i < componentCnt; i++) {
        if(strstr(pComponentNames[i], ".INBOX.") == 0) {
            amxc_string_t* comp = NULL;
            amxc_string_new(&comp, 0);
            amxc_string_set(comp, pComponentNames[i]);
            amxc_llist_append(&components, &comp->it);
        }
        free(pComponentNames[i]);
    }
    free(pComponentNames);

    // for each of the found component names fetch the available elements.
    amxc_llist_for_each(it, &components) {
        amxc_string_t* comp = amxc_string_from_llist_it(it);
        amxb_rbus_get_component_elements(amxb_rbus_ctx, request, amxc_string_get(comp, 0));

    }

    amxc_llist_clean(&components, amxc_string_list_it_free);
    return retval;
}

static int amxb_rbus_list_collect(amxb_rbus_t* amxb_rbus_ctx,
                                  const char* object,
                                  uint32_t flags,
                                  uint32_t access,
                                  amxb_request_t* request) {
    int retval = 0;
    rbusElementInfo_t* element = NULL;
    rbusElementInfo_t* elems = NULL;
    amxd_object_type_t type = amxd_object_singleton;
    amxd_object_type_t parent_type = amxd_object_singleton;

    // retrieve the element info for the requested path.
    retval = rbusElementInfo_get(amxb_rbus_ctx->handle, object, 0, &element);
    if(element != NULL) {
        if(element->type == RBUS_ELEMENT_TYPE_TABLE) {
            parent_type = amxd_object_template;
        }
        rbusElementInfo_free(amxb_rbus_ctx->handle, element);

        retval = rbusElementInfo_get(amxb_rbus_ctx->handle, object, -1, &elems);

        for(element = elems; element != NULL; element = element->next) {
            type = (parent_type == amxd_object_template)? amxd_object_instance:amxd_object_singleton;
            if(element->type == RBUS_ELEMENT_TYPE_TABLE) {
                type = amxd_object_template;
            }
            if(((flags & AMXB_FLAG_OBJECTS) != 0) && (type != amxd_object_instance)) {
                if((element->type == 0) || (element->type == RBUS_ELEMENT_TYPE_TABLE)) {
                    amxc_var_add(cstring_t, request->result, element->name);
                }
            }
            if(((flags & AMXB_FLAG_INSTANCES) != 0) && (type == amxd_object_instance)) {
                if(element->type == 0) {
                    amxc_var_add(cstring_t, request->result, element->name);
                }
            }
            if(((flags & AMXB_FLAG_PARAMETERS) != 0) && (element->type == RBUS_ELEMENT_TYPE_PROPERTY)) {
                amxc_var_add(cstring_t, request->result, element->name);
            }
            if(((flags & AMXB_FLAG_FUNCTIONS) != 0) && (element->type == RBUS_ELEMENT_TYPE_METHOD)) {
                if(access != amxd_dm_access_protected) {
                    if(strstr(element->name, "._") == 0) {
                        amxc_var_add(cstring_t, request->result, element->name);
                    }
                } else {
                    amxc_var_add(cstring_t, request->result, element->name);
                }
            }
            if(((flags & AMXB_FLAG_EVENTS) != 0) && (element->type == RBUS_ELEMENT_TYPE_EVENT)) {
                amxc_var_add(cstring_t, request->result, element->name);
            }
        }

        if(request->cb_fn != NULL) {
            const amxb_bus_ctx_t* ctx = amxb_request_get_ctx(request);
            request->cb_fn(ctx, request->result, request->priv);
            amxc_var_set_type(request->result, AMXC_VAR_ID_LIST);
        }
    }

    if(elems != NULL) {
        rbusElementInfo_free(amxb_rbus_ctx->handle, elems);
    }

    return retval;
}

int amxb_rbus_list(void* const ctx,
                   const char* object,
                   uint32_t flags,
                   uint32_t access,
                   amxb_request_t* request) {
    int retval = 0;
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;

    amxc_var_new(&request->result);
    amxc_var_set_type(request->result, AMXC_VAR_ID_LIST);

    if((object == NULL) || (*object == 0)) {
        // LEVEL 0 of the data model is a special case
        // fetch list of root objects and reply all the names
        // This requires more work, all component names must be fetched using rbus_discoverRegisteredComponents
        // and for each of the components all elements must be fetched using rbus_discoverComponentDataElements
        // It is not possible to specify the depth for this, although the function has an
        // argument `nextlevel` it is ignored in the implementation
        // Ignore flags for this one.
        retval = amxb_rbus_list_collect_root(amxb_rbus_ctx, request);
    } else {
        // All other levels
        retval = amxb_rbus_list_collect(amxb_rbus_ctx, object, flags, access, request);
    }

    if(request->cb_fn != NULL) {
        const amxb_bus_ctx_t* c = amxb_request_get_ctx(request);
        request->cb_fn(c, NULL, request->priv);
    }

    amxb_close_request(&request);

    return retval;
}
