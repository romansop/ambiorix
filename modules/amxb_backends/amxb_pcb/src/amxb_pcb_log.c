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

#include "amxb_pcb.h"
#include <syslog.h>
#include <fcntl.h>

void amxb_pcb_log(const char* fmt, ...) {
    amxc_string_t message;
    const char* dest = amxb_pcb_log_output();
    int fd = -1;
    va_list args;

    when_null(fmt, exit);
    when_str_empty(dest, exit);

    amxc_string_init(&message, 0);
    va_start(args, fmt);
    amxc_string_vappendf(&message, fmt, args);
    va_end(args);

    if(strcmp(dest, "syslog") == 0) {
        syslog(LOG_DAEMON | LOG_DEBUG, "%s", amxc_string_get(&message, 0));
    } else {
        amxc_ts_t now;
        char ts[32];
        int open_flags = O_CREAT | O_WRONLY | O_APPEND;
        int mode_flags = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
        int ret = -1;
        fd = open(dest, open_flags, mode_flags);
        when_true(fd == -1, exit_clean);
        amxc_ts_now(&now);
        amxc_ts_format(&now, ts, 32);
        ret = write(fd, ts, strlen(ts));
        when_true(ret == -1, exit_clean);
        ret = write(fd, " - ", 3);
        when_true(ret == -1, exit_clean);
        ret = write(fd, amxc_string_get(&message, 0), amxc_string_text_length(&message));
        when_true(ret == -1, exit_clean);
        ret = write(fd, "\n", 1);
        when_true(ret == -1, exit_clean);
    }

exit_clean:
    amxc_string_clean(&message);

exit:
    if(fd != -1) {
        close(fd);
    }
}

void amxb_pcb_log_pcb_request(const char* msg, request_t* req) {
    switch(request_type(req)) {
    case request_type_get_object:
        amxb_pcb_log("%s (GET) %p", msg, (void*) req);
        break;
    case request_type_set_object:
        amxb_pcb_log("%s (SET) %p", msg, (void*) req);
        break;
    case request_type_create_instance:
        amxb_pcb_log("%s (ADD) %p", msg, (void*) req);
        break;
    case request_type_delete_instance:
        amxb_pcb_log("%s (DEL) %p", msg, (void*) req);
        break;
    case request_type_exec_function:
        amxb_pcb_log("%s (INVOKE) %p", msg, (void*) req);
        break;
    default:
        break;
    }

    amxb_pcb_log("    Request path   = [%s]", request_path(req));
    amxb_pcb_log("    Request attrib = [0x%X]", request_attributes(req));
    amxb_pcb_log("    User ID        = [%d]", request_userID(req));
    amxb_pcb_log("    Source PID     = [%d]", request_getPid(req));

    switch(request_type(req)) {
    case request_type_get_object:
        amxb_pcb_log("    depth          = [%d]", request_depth(req));
        break;
    case request_type_set_object:
        amxb_pcb_log("    Nr. Of Params  = [%d]", llist_size(request_parameterList(req)));
        break;
    case request_type_create_instance:
        amxb_pcb_log("    Index          = [%d]", request_instanceIndex(req));
        amxb_pcb_log("    Key            = [%s]", request_instanceKey(req));
        break;
    case request_type_exec_function:
        amxb_pcb_log("    Function name  = [%s]", request_functionName(req));
        amxb_pcb_log("    Nr. Of Args    = [%d]", llist_size(request_parameterList(req)));
        break;
    default:
        break;
    }
}

void amxb_pcb_log_variant(const char* msg, const amxc_var_t* var) {
    const amxc_var_t* output = amxb_pcb_get_config_option("log");
    const char* dest = NULL;
    int fd = -1;

    when_null(output, exit);
    dest = GET_CHAR(output, NULL);
    when_str_empty(dest, exit);

    if(strcmp(dest, "syslog") == 0) {
        syslog(LOG_DAEMON | LOG_DEBUG, "%s", msg);
        amxc_var_log(var);
    } else {
        amxc_ts_t now;
        char ts[32];
        int open_flags = O_CREAT | O_WRONLY | O_APPEND;
        int mode_flags = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
        int ret = -1;
        fd = open(dest, open_flags, mode_flags);
        when_true(fd == -1, exit);
        amxc_ts_now(&now);
        amxc_ts_format(&now, ts, 32);
        ret = write(fd, ts, strlen(ts));
        when_true(ret == -1, exit);
        ret = write(fd, " - ", 3);
        when_true(ret == -1, exit);
        ret = write(fd, msg, strlen(msg));
        when_true(ret == -1, exit);
        ret = write(fd, "\n", 1);
        when_true(ret == -1, exit);
        amxc_var_dump(var, fd);
    }

exit:
    if(fd != -1) {
        close(fd);
    }
}

