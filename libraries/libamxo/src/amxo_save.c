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

#define PARAM_ATTR_KEY(param) amxc_var_dyncast(bool, GET_ARG(GET_ARG(param, "attributes"), "key"))

#define PARAM_NAME(param) amxc_var_constcast(cstring_t, GET_ARG(param, "name"))

#define PARAM_FLAGS(param) amxc_var_constcast(amxc_llist_t, GET_ARG(param, "flags"))

#define PARAM_VALUE(param) GET_ARG(param, "value")

/** Writes a string literal to the buffer or file.
 *
 * Only safe for writing in-line string literals to the buffer.
 * Do not use with (const) char* or the result of array decay!
 * example correct usage:
 *   amxo_parser_write_raw_literal(fd, buffer, "{\n");
 */
#define amxo_parser_write_raw_literal(fd, buffer, data) amxo_parser_write_raw(fd, buffer, data, sizeof(data) - 1)


static size_t buffer_size = 16348;
static int indentation = 0;

static int amxo_parser_save_object_tree(int fd,
                                        amxd_object_t* object,
                                        uint32_t depth,
                                        amxc_string_t* buffer);

static int amxo_parser_save_value(int fd,
                                  amxc_var_t* value,
                                  amxc_string_t* buffer,
                                  const char* termination);

static int amxo_parser_write(int fd, const char* buf, size_t bytes) {
    int retval = 0;
    size_t length = 0;
    ssize_t written;

    // Ensure bytes is within a reasonable limit to prevent overflow risks
    if(bytes > SSIZE_MAX) {
        retval = -EINVAL;  // Invalid argument
        goto exit;
    }

    while(length < bytes) {
        size_t chunk_size = bytes - length;

        written = write(fd, buf + length, chunk_size);

        if(written < 0) {
            if(errno == EINTR) {
                continue;    // Retry if interrupted
            }
            retval = -errno; // Return negative error code
            break;
        }
        if(written == 0) {
            retval = -EIO; // Unexpected end of write
            break;
        }
        if((size_t) written > chunk_size) { // Safety check
            retval = -EINVAL;
            break;
        }

        length += written;
    }

exit:
    return retval;
}

static void amxo_parser_indent(amxc_string_t* buffer) {
    static const char* spaces = "\t\t\t\t\t\t\t\t\t\t";

    if(indentation > 0) {
        amxc_string_append(buffer, spaces, indentation > 10 ? 10 : indentation);
    }
}

static int amxo_parser_flush_buffer(int fd, amxc_string_t* buffer) {
    int retval = 0;
    const char* buf = amxc_string_get(buffer, 0);
    size_t length = amxc_string_text_length(buffer);

    retval = amxo_parser_write(fd, buf, length);

    amxc_string_reset(buffer);
    return retval;
}

static void amxo_parser_write_raw(int fd,
                                  amxc_string_t* buffer,
                                  const char* data,
                                  size_t data_len) {
    amxc_string_append(buffer, data, data_len);
    if(amxc_string_text_length(buffer) > buffer_size) {
        amxo_parser_flush_buffer(fd, buffer);
    }
}

static inline void amxo_parser_write_raw_prefix_suffix(int fd,
                                                       amxc_string_t* buffer,
                                                       const char* prefix,
                                                       const char* dataptr,
                                                       const char* suffix) {
    amxo_parser_write_raw(fd, buffer, prefix, strlen(prefix));
    amxo_parser_write_raw(fd, buffer, dataptr, strlen(dataptr));
    amxo_parser_write_raw(fd, buffer, suffix, strlen(suffix));
}

static void amxo_parser_writef(int fd,
                               amxc_string_t* buffer,
                               const char* fmt,
                               ...) {
    va_list args;

    va_start(args, fmt);
    amxc_string_vappendf(buffer, fmt, args);
    va_end(args);

    if(amxc_string_text_length(buffer) > buffer_size) {
        amxo_parser_flush_buffer(fd, buffer);
    }
}

static char* amxo_parser_build_filename(amxo_parser_t* pctx,
                                        const char* filename,
                                        bool temp) {
    char* full_name = NULL;
    amxc_string_t full_path;

    amxc_string_init(&full_path, 128);
    amxc_string_appendf(&full_path, "%s%s", filename, temp ? ".tmp" : "");
    amxc_string_resolve(&full_path, &pctx->config);
    full_name = amxc_string_take_buffer(&full_path);
    amxc_string_clean(&full_path);

    return full_name;
}

static int amxo_parser_copy(int dest_fd, int source_fd) {
    int retval = 0;

    ssize_t length = 0;
    char buffer[1024];

    length = read(source_fd, buffer, 1024);
    while(length > 0) {
        retval = amxo_parser_write(dest_fd, buffer, (size_t) length);
        when_failed(retval, exit);
        length = read(source_fd, buffer, 1024);
    }

exit:
    return retval;
}

static int amxo_parser_open_file(amxo_parser_t* pctx,
                                 const char* filename,
                                 bool append) {
    int fd = -1;
    char* full_path_tmp = amxo_parser_build_filename(pctx, filename, true);
    char* full_path = amxo_parser_build_filename(pctx, filename, false);
    char* real_path = NULL;
    int open_flags = O_CREAT | O_WRONLY | O_TRUNC;
    int mode_flags = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

    if(append) {
        real_path = realpath(full_path, NULL);
        when_null(real_path, exit);
    }
    fd = open(full_path_tmp, open_flags, mode_flags);
    when_true(fd == -1, exit);

    if(append) {
        int orig_fd = open(real_path, O_RDONLY);
        if(orig_fd == -1) {
            close(fd);
            fd = -1;
            goto exit;
        }
        if(amxo_parser_copy(fd, orig_fd) != 0) {
            close(orig_fd);
            close(fd);
            fd = -1;
            goto exit;
        }
        close(orig_fd);
    }

exit:
    free(full_path_tmp);
    free(full_path);
    free(real_path);
    return fd;
}

static int amxo_parser_save_table_config(int fd,
                                         amxc_var_t* value,
                                         amxc_string_t* buffer) {
    int retval = -1;
    const char* sep = ",";

    amxo_parser_indent(buffer);
    amxc_var_for_each(val, value) {
        const char* key = amxc_var_key(val);
        if(amxc_var_get_next(val) == NULL) {
            sep = "";
        }

        if(strchr(key, '.') != NULL) {
            amxo_parser_write_raw_prefix_suffix(fd, buffer, "\"'", key, "'\" = ");
        } else {
            amxo_parser_write_raw_prefix_suffix(fd, buffer, "\"", key, "\" = ");
        }

        retval = amxo_parser_save_value(fd, val, buffer, sep);
        when_true(retval < 0, exit);
    }
    retval = 0;

exit:
    return retval;
}

static int amxo_parser_save_list_config(int fd,
                                        amxc_var_t* value,
                                        amxc_string_t* buffer) {
    int retval = -1;
    const char* sep = ",";

    amxc_var_for_each(val, value) {
        if(amxc_var_get_next(val) == NULL) {
            sep = "";
        }
        amxo_parser_indent(buffer);
        retval = amxo_parser_save_value(fd, val, buffer, sep);
        amxo_parser_write_raw_literal(fd, buffer, "\n");
        when_true(retval < 0, exit);
    }
    retval = 0;

exit:
    return retval;
}

static void amxo_parser_escape_value(amxc_string_t* value) {
    amxc_string_esc(value);
}

static int amxo_parser_save_value(int fd,
                                  amxc_var_t* value,
                                  amxc_string_t* buffer,
                                  const char* termination) {
    int type = amxc_var_type_of(value);
    int retval = 0;
    char* txt = NULL;
    switch(type) {
    case AMXC_VAR_ID_INT8:
    case AMXC_VAR_ID_INT16:
    case AMXC_VAR_ID_INT32:
    case AMXC_VAR_ID_INT64:
    case AMXC_VAR_ID_UINT8:
    case AMXC_VAR_ID_UINT16:
    case AMXC_VAR_ID_UINT32:
    case AMXC_VAR_ID_UINT64:
    case AMXC_VAR_ID_BOOL:
        txt = amxc_var_dyncast(cstring_t, value);
        amxo_parser_write_raw(fd, buffer, txt, strlen(txt));
        amxo_parser_write_raw(fd, buffer, termination, strlen(termination));
        free(txt);
        break;
    case AMXC_VAR_ID_TIMESTAMP:
        txt = amxc_var_dyncast(cstring_t, value);
        amxo_parser_write_raw_prefix_suffix(fd, buffer, "\"", txt, "\"");
        amxo_parser_write_raw(fd, buffer, termination, strlen(termination));
        free(txt);
        break;
    case AMXC_VAR_ID_CSTRING:
    case AMXC_VAR_ID_SSV_STRING:
    case AMXC_VAR_ID_CSV_STRING: {
        amxc_string_t str_value;
        amxc_string_init(&str_value, 0);
        amxc_string_set(&str_value, amxc_var_constcast(cstring_t, value));
        amxo_parser_escape_value(&str_value);
        amxo_parser_write_raw_prefix_suffix(fd, buffer, "\"", amxc_string_get(&str_value, 0), "\"");
        amxo_parser_write_raw(fd, buffer, termination, strlen(termination));
        amxc_string_clean(&str_value);
    }
    break;
    case AMXC_VAR_ID_HTABLE:
        amxo_parser_write_raw_literal(fd, buffer, "{\n");
        indentation++;
        amxo_parser_save_table_config(fd, value, buffer);
        indentation--;
        amxo_parser_write_raw_prefix_suffix(fd, buffer, "}", termination, "\n");
        break;
    case AMXC_VAR_ID_LIST:
        amxo_parser_write_raw_literal(fd, buffer, "[\n");
        indentation++;
        amxo_parser_save_list_config(fd, value, buffer);
        indentation--;
        amxo_parser_indent(buffer);
        amxo_parser_write_raw_prefix_suffix(fd, buffer, "]", termination, "\n");
        break;
    default:
        retval = -1;
        break;
    }

    return retval;
}

static int amxo_parser_save_config_options(int fd,
                                           amxc_var_t* config,
                                           amxc_string_t* buffer) {
    int retval = 0;
    amxo_parser_write_raw_literal(fd, buffer, "%config {\n");
    indentation++;
    amxc_var_for_each(val, config) {
        const char* key = amxc_var_key(val);
        amxo_parser_indent(buffer);
        if(strchr(key, '.') != NULL) {
            amxo_parser_write_raw_prefix_suffix(fd, buffer, "\"'", key, "'\" = ");
        } else {
            amxo_parser_write_raw_prefix_suffix(fd, buffer, "\"", key, "\" = ");
        }

        retval = amxo_parser_save_value(fd, val, buffer, ";");
        when_true(retval < 0, exit);
    }
    indentation--;
    amxo_parser_write_raw_literal(fd, buffer, "}\n");

exit:
    return retval;
}

static int amxo_parser_instance_header(int fd,
                                       amxd_object_t* object,
                                       amxc_string_t* buffer) {
    int retval = 0;
    amxc_var_t params;
    const char* inst_name = amxd_object_get_name(object, AMXD_OBJECT_NAMED);

    amxo_parser_indent(buffer);
    amxo_parser_writef(fd, buffer, "instance add(%d", amxd_object_get_index(object));
    if(amxd_name_is_valid(inst_name)) {
        amxo_parser_write_raw_prefix_suffix(fd, buffer, ", '", inst_name, "'");
    }

    amxc_var_init(&params);
    amxd_object_get_params_with_attr(object, &params, SET_BIT(amxd_pattr_key), amxd_dm_access_private);
    amxc_var_for_each(param, &params) {
        const char* name = amxc_var_key(param);
        amxo_parser_write_raw_prefix_suffix(fd, buffer, ", '", name, "' = ");
        retval = amxo_parser_save_value(fd, param, buffer, "");
        when_true(retval < 0, exit);
    }
    amxo_parser_write_raw_literal(fd, buffer, ") {\n");

exit:
    amxc_var_clean(&params);
    return retval;
}

static int amxo_parser_open_parent_tree(int fd,
                                        amxd_object_t* object,
                                        amxc_string_t* buffer) {
    int retval = 0;
    if(amxd_object_get_type(object) != amxd_object_root) {
        retval = amxo_parser_open_parent_tree(fd,
                                              amxd_object_get_parent(object),
                                              buffer);
        when_true(retval < 0, exit);
    }

    if(amxd_object_get_type(object) == amxd_object_root) {
        goto exit;
    }

    if(amxd_object_get_type(object) == amxd_object_instance) {
        amxo_parser_instance_header(fd, object, buffer);
    } else {
        const char* obj_name = amxd_object_get_name(object, AMXD_OBJECT_NAMED);
        amxo_parser_indent(buffer);
        amxo_parser_write_raw_prefix_suffix(fd, buffer,
                                            "object '", obj_name, "' {\n");
    }
    indentation++;

exit:
    return retval;
}

static int amxo_parser_save_mibs(int fd,
                                 amxd_object_t* object,
                                 amxc_string_t* buffer) {
    int retval = 0;
    amxc_array_it_t* it = amxc_array_get_first(&object->mib_names);
    while(it) {
        const char* name = (const char*) amxc_array_it_get_data(it);
        amxo_parser_indent(buffer);
        amxo_parser_write_raw_prefix_suffix(fd, buffer, "extend using mib '", name, "';\n");
        it = amxc_array_it_get_next(it);
    }

    return retval;
}

static int amxo_parser_save_param_flags(int fd,
                                        amxc_var_t* flags,
                                        amxc_string_t* buffer) {
    int retval = 0;
    const char* sep = "";
    size_t sep_length = 0;
    amxo_parser_write_raw_literal(fd, buffer, " {\n");
    indentation++;
    amxo_parser_indent(buffer);
    amxo_parser_write_raw_literal(fd, buffer, "userflags ");
    amxc_var_for_each(var_flag, flags) {
        const char* flag = amxc_var_key(var_flag);
        bool is_set = GET_BOOL(var_flag, NULL);
        if((flag != NULL) && is_set) {
            amxo_parser_write_raw(fd, buffer, sep, sep_length);
            amxo_parser_write_raw_literal(fd, buffer, " %");
            amxo_parser_write_raw(fd, buffer, flag, strlen(flag));
        }
        sep = ",";
        sep_length = 1;
    }
    amxo_parser_write_raw_literal(fd, buffer, ";\n");
    indentation--;
    amxo_parser_indent(buffer);
    amxo_parser_write_raw_literal(fd, buffer, "}\n");

    return retval;
}

static int amxo_parser_save_params(int fd,
                                   amxd_object_t* object,
                                   amxc_string_t* buffer) {
    int retval = 0;
    amxc_var_t params;
    uint32_t attributes = SET_BIT(amxd_pattr_persistent);
    amxc_var_init(&params);

    amxd_object_get_params_with_attr(object, &params, attributes, amxd_dm_access_private);

    amxc_var_for_each(param, &params) {
        const char* name = amxc_var_key(param);
        amxd_param_t* param_def = amxd_object_get_param_def(object, name);
        if(param_def != NULL) {
            if((amxd_object_get_type(object) == amxd_object_template) && (param_def->attr.templ == 0)) {
                continue;
            }
            if((amxd_object_get_type(object) == amxd_object_instance) && (param_def->attr.instance == 0)) {
                continue;
            }
            amxo_parser_indent(buffer);
            amxo_parser_write_raw_prefix_suffix(fd, buffer, "parameter '", name, "' = ");
            retval = amxo_parser_save_value(fd, param, buffer, "");
            if(param_def->flags != NULL) {
                amxo_parser_save_param_flags(fd, param_def->flags, buffer);
            } else {
                amxo_parser_write_raw_literal(fd, buffer, ";\n");
            }
        } else {
            amxo_parser_indent(buffer);
            amxo_parser_write_raw_prefix_suffix(fd, buffer, "parameter '", name, "' = ");
            retval = amxo_parser_save_value(fd, param, buffer, "");
            amxo_parser_write_raw_literal(fd, buffer, ";\n");
        }
        when_true(retval < 0, exit);
    }

exit:
    amxc_var_clean(&params);
    return retval;
}

static int amxo_parser_save_leave(int fd,
                                  amxc_llist_it_t* it,
                                  uint32_t depth,
                                  amxc_string_t* buffer) {
    int retval = 0;
    amxd_object_t* obj = amxc_container_of(it, amxd_object_t, it);
    if(!amxd_object_is_attr_set(obj, amxd_oattr_persistent) &&
       ( amxd_object_get_type(obj) != amxd_object_template)) {
        goto exit;
    }
    if(amxd_object_get_type(obj) == amxd_object_instance) {
        amxo_parser_instance_header(fd, obj, buffer);
    } else {
        const char* obj_name = amxd_object_get_name(obj, AMXD_OBJECT_NAMED);
        amxo_parser_indent(buffer);
        amxo_parser_write_raw_prefix_suffix(fd, buffer,
                                            "object '", obj_name, "' {\n");

    }
    indentation++;
    retval = amxo_parser_save_object_tree(fd, obj, depth, buffer);
    when_true(retval < 0, exit);
    indentation--;
    amxo_parser_indent(buffer);
    amxo_parser_write_raw_literal(fd, buffer, "}\n");

exit:
    return retval;
}

static int amxo_parser_save_object_tree(int fd,
                                        amxd_object_t* object,
                                        uint32_t depth,
                                        amxc_string_t* buffer) {
    int retval = 0;

    if(amxd_object_get_type(object) != amxd_object_root) {
        retval = amxo_parser_save_mibs(fd, object, buffer);
        when_failed(retval, exit);
        retval = amxo_parser_save_params(fd, object, buffer);
        when_failed(retval, exit);
        if(depth != UINT32_MAX) {
            depth--;

        }
        when_true(depth == 0, exit);
    }

    if(amxd_object_get_type(object) == amxd_object_template) {
        amxd_object_for_each(instance, it, object) {
            retval = amxo_parser_save_leave(fd, it, depth, buffer);
            when_failed(retval, exit);
        }
    } else {
        amxd_object_for_each(child, it, object) {
            retval = amxo_parser_save_leave(fd, it, depth, buffer);
            when_failed(retval, exit);
        }
    }

exit:
    return retval;
}

static int amxo_parser_close_parent_tree(int fd,
                                         amxd_object_t* object,
                                         amxc_string_t* buffer) {
    int retval = 0;
    if(amxd_object_get_type(object) == amxd_object_root) {
        goto exit;
    }

    indentation--;
    amxo_parser_indent(buffer);
    amxo_parser_write_raw_literal(fd, buffer, "}\n");

    retval = amxo_parser_close_parent_tree(fd,
                                           amxd_object_get_parent(object),
                                           buffer);

exit:
    return retval;
}

static int amxo_parser_save_tree(int fd,
                                 amxd_object_t* object,
                                 uint32_t depth,
                                 amxc_string_t* buffer) {
    int retval = 0;
    amxo_parser_write_raw_literal(fd, buffer, "%populate {\n");
    indentation++;

    retval = amxo_parser_open_parent_tree(fd, object, buffer);
    when_failed(retval, exit);
    retval = amxo_parser_save_object_tree(fd, object, depth, buffer);
    when_failed(retval, exit);
    retval = amxo_parser_close_parent_tree(fd, object, buffer);
    when_failed(retval, exit);

    indentation--;
    amxo_parser_write_raw_literal(fd, buffer, "}\n");

exit:
    return retval;
}

static void amxo_parser_remove_file(amxo_parser_t* pctx,
                                    const char* filename,
                                    int fd) {
    char* full_path_tmp = amxo_parser_build_filename(pctx, filename, true);

    close(fd);
    unlink(full_path_tmp);
    free(full_path_tmp);
}

static int amxo_parser_close_file(amxo_parser_t* pctx,
                                  const char* filename,
                                  int fd) {
    int retval = 0;
    char* full_path_tmp = amxo_parser_build_filename(pctx, filename, true);
    char* full_path = amxo_parser_build_filename(pctx, filename, false);

    retval = fsync(fd);
    when_failed(retval, exit);
    retval = close(fd);
    when_failed(retval, exit);
    retval = rename(full_path_tmp, full_path);
    when_failed(retval, exit);

exit:
    free(full_path_tmp);
    free(full_path);
    return retval;
}

int amxo_parser_save(amxo_parser_t* pctx,
                     const char* filename,
                     amxd_object_t* object,
                     uint32_t depth,
                     amxc_var_t* config,
                     bool append) {
    int retval = -1;
    int fd = -1;
    amxc_string_t buffer;

    if(GETP_ARG(&pctx->config, "odl.buffer-size") != NULL) {
        buffer_size = GETP_INT32(&pctx->config, "odl.buffer-size");
    }
    amxc_string_init(&buffer, buffer_size + 1024);

    when_null(pctx, exit);
    when_str_empty(filename, exit);

    indentation = 0;
    pctx->status = amxd_status_ok;
    fd = amxo_parser_open_file(pctx, filename, append);
    when_true(fd < 0, exit);
    if((config != NULL) && (amxc_var_type_of(config) == AMXC_VAR_ID_HTABLE)) {
        retval = amxo_parser_save_config_options(fd, config, &buffer);
        when_true(retval < 0, exit);
    }

    if(object != NULL) {
        retval = amxo_parser_save_tree(fd, object, depth, &buffer);
        when_true(retval < 0, exit);
    }

    amxo_parser_flush_buffer(fd, &buffer);

exit:
    if(fd >= 0) {
        if(retval < 0) {
            amxo_parser_remove_file(pctx, filename, fd);
        } else {
            amxo_parser_close_file(pctx, filename, fd);
        }
    } else {
        retval = -1;
    }
    amxc_string_clean(&buffer);
    return retval;
}

int amxo_parser_save_config(amxo_parser_t* pctx,
                            const char* filename,
                            amxc_var_t* config,
                            bool append) {
    return amxo_parser_save(pctx, filename, NULL, 0, config, append);
}

int amxo_parser_save_object(amxo_parser_t* pctx,
                            const char* filename,
                            amxd_object_t* object,
                            bool append) {
    return amxo_parser_save(pctx, filename, object, UINT32_MAX, NULL, append);
}
