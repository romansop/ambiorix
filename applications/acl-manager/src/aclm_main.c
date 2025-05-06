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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <debug/sahtrace.h>

#include "aclm.h"
#include "aclm_utils.h"
#include "aclm_merge.h"
#include "aclm_inotify.h"

static aclm_app_t app;

static void aclm_init(amxd_dm_t* dm, amxo_parser_t* parser) {
    SAH_TRACEZ_INFO(ME, "**************************************");
    SAH_TRACEZ_INFO(ME, "*         ACL Manager started        *");
    SAH_TRACEZ_INFO(ME, "**************************************");
    int retval = -1;
    const char* acl_install_dir = GET_CHAR(&parser->config, "acl_install_dir");
    const char* acl_dir = GET_CHAR(&parser->config, "acl_dir");
    bool inotify_enabled = GET_BOOL(&parser->config, "inotify-enabled");
    app.parser = parser;
    app.dm = dm;

    when_str_empty(acl_install_dir, exit);
    when_str_empty(acl_dir, exit);

    retval = aclm_copy_recursively(acl_install_dir, acl_dir);
    when_failed(retval, exit);

    aclm_create_role_dir(ACLM_MERGE_DIR);

    if(inotify_enabled) {
        retval = aclm_inotify_init(acl_dir);
        if(retval != 0) {
            SAH_TRACEZ_WARNING(ME, "Inotify init failed");
        }
    }

exit:
    return;
}

static void aclm_exit(amxo_parser_t* parser) {
    SAH_TRACEZ_INFO(ME, "**************************************");
    SAH_TRACEZ_INFO(ME, "*         ACL Manager stopped        *");
    SAH_TRACEZ_INFO(ME, "**************************************");
    bool inotify_enabled = GET_BOOL(&parser->config, "inotify-enabled");
    app.parser = NULL;
    app.dm = NULL;
    if(inotify_enabled) {
        aclm_inotify_clean();
    }
}

void _print_event(const char* const sig_name,
                  const amxc_var_t* const data,
                  UNUSED void* const priv) {
    printf("event received - %s\n", sig_name);
    if(data != NULL) {
        printf("Event data = \n");
        fflush(stdout);
        amxc_var_dump(data, STDOUT_FILENO);
    }
}

amxo_parser_t* PRIVATE aclm_get_parser(void) {
    return app.parser;
}

amxd_dm_t* PRIVATE aclm_get_dm(void) {
    return app.dm;
}

int _aclm_main(int reason,
               amxd_dm_t* dm,
               amxo_parser_t* parser) {
    SAH_TRACEZ_INFO(ME, "aclm_main, reason: %d", reason);
    switch(reason) {
    case 0:     // START
        aclm_init(dm, parser);
        break;
    case 1:     // STOP
        aclm_exit(parser);
        break;
    default:
        break;
    }

    return 0;
}
