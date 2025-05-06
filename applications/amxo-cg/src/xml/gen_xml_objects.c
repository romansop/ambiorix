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

#include <regex.h>

#include "gen_xml.h"

static void gen_xml_object_set_type(xmlNodePtr node, amxd_object_type_t type) {
    xml_gen_t* xml_ctx = gen_xml_get_ctx();
    if(type == amxd_object_template) {
        xmlSetNsProp(node, xml_ctx->ns, BAD_CAST "template", BAD_CAST "true");
    } else {
        xmlUnsetNsProp(node, xml_ctx->ns, BAD_CAST "template");
    }
    if(type == amxd_object_instance) {
        xmlSetNsProp(node, xml_ctx->ns, BAD_CAST "instance", BAD_CAST "true");
    } else {
        xmlUnsetNsProp(node, xml_ctx->ns, BAD_CAST "instance");
    }
    if(type == amxd_object_mib) {
        xmlSetNsProp(node, xml_ctx->ns, BAD_CAST "mib", BAD_CAST "true");
    } else {
        xmlUnsetNsProp(node, xml_ctx->ns, BAD_CAST "mib");
    }
}

static void gen_xml_object_add(amxo_parser_t* parser,
                               amxd_object_t* parent,
                               const char* name,
                               UNUSED int64_t attr_bitmask,
                               amxd_object_type_t type,
                               uint32_t path_flags) {
    xml_gen_t* xml_ctx = gen_xml_get_ctx();
    char* path = amxd_object_get_path(parent, path_flags | AMXD_OBJECT_TERMINATE);
    amxc_string_t full_path;
    amxc_string_t trans_name;
    xmlNodePtr parent_node = NULL;
    xmlNodePtr child = NULL;

    amxc_string_init(&full_path, 0);
    amxc_string_init(&trans_name, 0);

    amxc_string_setf(&trans_name, "%s", name);
    if(path != NULL) {
        amxc_string_setf(&full_path, "%s%s.", path, name);
    } else {
        amxc_string_setf(&full_path, "%s.", name);
    }

    gen_xml_translate_path(parser, &full_path, &trans_name);

    parent_node = gen_xml_get_parent_node(xml_ctx->doc, &full_path);
    if((type == amxd_object_template) &&
       ((path_flags & AMXD_OBJECT_SUPPORTED) == AMXD_OBJECT_SUPPORTED)) {
        amxc_string_append(&full_path, "{i}.", 4);
    }

    child = gen_xml_find(xml_ctx->doc, amxc_string_get(&full_path, 0), NULL);
    if(child == NULL) {
        child = xmlNewNode(xml_ctx->ns, BAD_CAST "object");
        xmlAddChild(parent_node, child);
        xmlSetNsProp(child, xml_ctx->ns, BAD_CAST "name", BAD_CAST amxc_string_get(&trans_name, 0));
        xmlSetNsProp(child, xml_ctx->ns, BAD_CAST "path", BAD_CAST amxc_string_get(&full_path, 0));
        gen_xml_add_defined(parser, child);
    }

    gen_xml_object_set_type(child, type);

    xml_ctx->xml_object = child;

    free(path);
    amxc_string_clean(&trans_name);
    amxc_string_clean(&full_path);
}

static void gen_xml_add_mibs(xmlNodePtr xml_object, amxd_object_t* object) {
    xml_gen_t* xml_ctx = gen_xml_get_ctx();
    xmlNodePtr xml_mib = gen_xml_find_node(xml_object, "extend-with");
    size_t len = 0;
    while(xml_mib) {
        xmlUnlinkNode(xml_mib);
        xmlFreeNode(xml_mib);
        xml_mib = gen_xml_find_node(xml_object, "extend-with");
    }

    len = amxc_array_size(&object->mib_names);
    for(size_t i = 0; i < len; i++) {
        const char* mib_name = (const char*) amxc_array_get_data_at(&object->mib_names, i);
        if(mib_name == NULL) {
            continue;
        }
        xmlNewChild(xml_object, xml_ctx->ns, BAD_CAST "extend-with", BAD_CAST mib_name);
    }
}

static void gen_xml_add_tree(amxd_object_t* const object,
                             UNUSED int32_t depth,
                             void* priv) {
    xml_gen_t* xml_ctx = gen_xml_get_ctx();
    uint32_t flags =
        (xml_ctx->section == 1) ? AMXD_OBJECT_SUPPORTED : AMXD_OBJECT_INDEXED;
    amxo_parser_t* parser = (amxo_parser_t*) priv;
    char* path = amxd_object_get_path(object, flags | AMXD_OBJECT_TERMINATE);

    xmlNodePtr xml_object = gen_xml_find(xml_ctx->doc, path, NULL);

    if(xml_object == NULL) {
        amxd_object_t* parent = amxd_object_get_parent(object);
        const char* iname = amxd_object_get_name(object, AMXD_OBJECT_INDEXED);
        const char* name = amxd_object_get_name(object, AMXD_OBJECT_NAMED);
        uint64_t attrs = amxd_object_get_attrs(object);
        amxd_object_type_t type = amxd_object_get_type(object);
        if(amxd_object_get_type(object) == amxd_object_instance) {
            gen_xml_object_add(parser, parent, iname, attrs, type, flags);
        } else {
            gen_xml_object_add(parser, parent, name, attrs, type, flags);
        }
    } else {
        xml_ctx->xml_object = xml_object;
    }

    xml_ctx->xml_param = NULL;
    amxd_object_for_each(parameter, it, object) {
        amxd_param_t* param = amxc_container_of(it, amxd_param_t, it);
        gen_xml_parameter_set(parser, object, param, &param->value);
        xml_ctx->xml_param = NULL;
    }

    gen_xml_add_mibs(xml_ctx->xml_object, object);

    free(path);
    return;
}

static bool gen_xml_skip_object(amxo_parser_t* parser,
                                amxd_object_t* object,
                                const char* name,
                                int64_t attr_bitmask) {
    bool skip = false;
    regex_t regexp;
    amxc_string_t full_path;
    const char* white_list = GETP_CHAR(&parser->config, "amxo-cg.xml.white-list");
    bool skip_protected = GETP_BOOL(&parser->config, "amxo-cg.xml.skip-protected");

    memset(&regexp, 0, sizeof(regexp));
    amxc_string_init(&full_path, 0);

    if(IS_BIT_SET(attr_bitmask, amxd_oattr_private)) {
        skip = true;
        goto exit;
    }

    // check skip protected
    if(IS_BIT_SET(attr_bitmask, amxd_oattr_protected) && skip_protected) {
        skip = true;
        goto exit;
    }

    when_str_empty(white_list, exit);

    // check skip not matching paths
    if(amxd_object_get_type(object) == amxd_object_root) {
        if(name != NULL) {
            amxc_string_setf(&full_path, "%s.", name);
        }
    } else {
        if(name != NULL) {
            amxc_string_setf(&full_path, "%s%s.", amxd_object_get_path(object, AMXD_OBJECT_TERMINATE), name);
        } else {
            amxc_string_setf(&full_path, "%s", amxd_object_get_path(object, AMXD_OBJECT_TERMINATE));
        }
    }

    gen_xml_translate_path(parser, &full_path, NULL);

    if(regcomp(&regexp, white_list, REG_EXTENDED) == 0) {
        if(regexec(&regexp, amxc_string_get(&full_path, 0), 0, NULL, 0) != 0) {
            skip = true;
        }
        regfree(&regexp);
    }

exit:
    amxc_string_clean(&full_path);
    return skip;
}

void gen_xml_object_start(amxo_parser_t* parser,
                          amxd_object_t* parent,
                          const char* name,
                          int64_t attr_bitmask,
                          amxd_object_type_t type) {
    xml_gen_t* xml_ctx = gen_xml_get_ctx();

    if(xml_ctx->xml_dm_root == NULL) {
        xml_ctx->section = 1;
        xml_ctx->xml_dm_root = xmlNewNode(xml_ctx->ns, BAD_CAST "datamodel");
        xmlSetNsProp(xml_ctx->xml_dm_root, xml_ctx->ns, BAD_CAST "source", BAD_CAST parser->file);
        xmlAddChild(xml_ctx->xml_root, xml_ctx->xml_dm_root);
    }

    if(gen_xml_skip_object(parser, parent, name, attr_bitmask)) {
        xml_ctx->object_skip++;
    }

    if(xml_ctx->object_skip == 0) {
        gen_xml_object_add(parser, parent, name,
                           attr_bitmask, type, AMXD_OBJECT_SUPPORTED);

        gen_xml_add_description(xml_ctx->xml_object);
        gen_xml_add_version(xml_ctx->xml_object);
    }

}

void gen_xml_object_instance(amxo_parser_t* parser,
                             amxd_object_t* parent,
                             uint32_t index,
                             const char* name) {
    xmlNodePtr child = NULL;
    xml_gen_t* xml_ctx = gen_xml_get_ctx();
    amxd_object_t* instance = amxd_object_get_instance(parent, name, index);
    amxc_var_t* amxo_cg = amxo_parser_get_config(parser, "amxo-cg");
    bool skip_instances = GETP_BOOL(amxo_cg, "xml.skip-instances");
    char* path = amxd_object_get_path(instance, AMXD_OBJECT_INDEXED | AMXD_OBJECT_TERMINATE);
    amxc_string_t full_path;

    amxc_string_init(&full_path, 0);
    amxc_string_setf(&full_path, "%s.", path);

    if(skip_instances || gen_xml_skip_object(parser, parent, name, 0)) {
        xml_ctx->object_skip++;
    }

    if(xml_ctx->object_skip == 0) {
        gen_xml_translate_path(parser, &full_path, NULL);

        child = gen_xml_find(xml_ctx->doc, path, NULL);
        if(child == NULL) {
            child = xmlNewNode(xml_ctx->ns, BAD_CAST "object");
            xmlSetNsProp(child, xml_ctx->ns, BAD_CAST "name",
                         BAD_CAST amxd_object_get_name(instance, AMXD_OBJECT_INDEXED));
            xmlSetNsProp(child, xml_ctx->ns, BAD_CAST "path", BAD_CAST amxc_string_get(&full_path, 0));

            xmlSetNsProp(child, xml_ctx->ns, BAD_CAST "instance", BAD_CAST "true");
            xmlAddChild(xml_ctx->xml_object, child);

            gen_xml_add_description(child);
            gen_xml_add_version(xml_ctx->xml_object);
        }

        xml_ctx->xml_object = child;

        amxd_object_hierarchy_walk(instance, amxd_direction_down,
                                   NULL, gen_xml_add_tree,
                                   INT32_MAX, parser);

        xml_ctx->xml_object = child;
    }

    amxc_string_clean(&full_path);
    free(path);
}

void gen_xml_object_select(UNUSED amxo_parser_t* parser,
                           amxd_object_t* parent,
                           const char* path) {
    xml_gen_t* xml_ctx = gen_xml_get_ctx();
    char* ppath = NULL;
    amxd_object_t* object = NULL;
    amxc_string_t full_path;
    amxc_string_init(&full_path, 0);

    ppath = amxd_object_get_path(parent, AMXD_OBJECT_INDEXED | AMXD_OBJECT_TERMINATE);
    amxc_string_setf(&full_path, "%s%s.", ppath == NULL ? "" : ppath, path);
    gen_xml_translate_path(parser, &full_path, NULL);
    xml_ctx->xml_object = gen_xml_find(xml_ctx->doc, amxc_string_get(&full_path, 0), NULL);
    free(ppath);

    if(xml_ctx->xml_object != NULL) {
        goto exit;
    }

    object = amxd_object_findf(parent, "%s.", path);
    if(object != NULL) {
        ppath = amxd_object_get_path(object, AMXD_OBJECT_INDEXED | AMXD_OBJECT_TERMINATE);
        amxc_string_setf(&full_path, "%s", ppath);
        gen_xml_translate_path(parser, &full_path, NULL);
        xml_ctx->xml_object = gen_xml_find(xml_ctx->doc, amxc_string_get(&full_path, 0), NULL);
        free(ppath);

        if(xml_ctx->xml_object != NULL) {
            goto exit;
        }
    }

    ppath = amxd_object_get_path(parent, AMXD_OBJECT_SUPPORTED | AMXD_OBJECT_TERMINATE);
    amxc_string_setf(&full_path, "%s%s.", ppath == NULL ? "" : ppath, path);
    gen_xml_translate_path(parser, &full_path, NULL);
    xml_ctx->xml_object = gen_xml_find(xml_ctx->doc, amxc_string_get(&full_path, 0), NULL);
    free(ppath);

    if(xml_ctx->xml_object != NULL) {
        goto exit;
    }

    if(object != NULL) {
        ppath = amxd_object_get_path(object, AMXD_OBJECT_SUPPORTED | AMXD_OBJECT_TERMINATE);
        amxc_string_setf(&full_path, "%s", ppath);
        gen_xml_translate_path(parser, &full_path, NULL);
        xml_ctx->xml_object = gen_xml_find(xml_ctx->doc, amxc_string_get(&full_path, 0), NULL);
        free(ppath);

        if(xml_ctx->xml_object != NULL) {
            goto exit;
        }
    }

    // No xml object found - skip this object
    xml_ctx->object_skip++;

exit:
    amxc_string_clean(&full_path);
}

void gen_xml_object_end(UNUSED amxo_parser_t* parser,
                        amxd_object_t* object) {
    uint32_t attrs = amxd_object_get_attrs(object);
    const char* attr_names[] = { "read-only", "persistent", "private", "locked", "protected" };
    xml_gen_t* xml_ctx = gen_xml_get_ctx();
    amxc_var_t* amxo_cg = amxo_parser_get_config(parser, "amxo-cg");
    bool skip_instances = GETP_BOOL(amxo_cg, "xml.skip-instances");

    if(xml_ctx->object_skip == 0) {
        if(amxd_object_get_type(object) == amxd_object_singleton) {
            attrs |= (1 << amxd_oattr_read_only);
        }
        gen_xml_attributes(xml_ctx->xml_object, attrs, amxd_oattr_max, attr_names);

        xml_ctx->xml_object = xml_ctx->xml_object == NULL? NULL:xml_ctx->xml_object->parent;
        if((xml_ctx->xml_object != NULL) && (xml_ctx->xml_object == xml_ctx->xml_dm_root)) {
            xml_ctx->xml_object = NULL;
        }
    }

    if(((amxd_object_get_type(object) == amxd_object_instance) && skip_instances) ||
       gen_xml_skip_object(parser, object, NULL, attrs)) {
        xml_ctx->object_skip--;
    }
}

void gen_xml_object_add_mib(amxo_parser_t* parser,
                            amxd_object_t* object,
                            UNUSED const char* mib_name) {
    xml_gen_t* xml_ctx = gen_xml_get_ctx();
    xmlNodePtr current = xml_ctx->xml_object;

    amxd_object_hierarchy_walk(object, amxd_direction_down,
                               NULL, gen_xml_add_tree,
                               INT32_MAX, parser);

    xml_ctx->xml_object = current;
}