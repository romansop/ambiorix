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

#include <string.h>
#include <stdio.h>

#include "ba_cli_priv.h"

#include "mod_pcb_cli_cmd.h"
#include "mod_pcb_cli_common.h"

static void mod_pcb_cli_write_resolved_paths(amxc_var_t* paths) {
    amxt_tty_t* tty = ba_cli_get_tty();
    const amxc_llist_t* lpaths = amxc_var_constcast(amxc_llist_t, paths);

    amxc_llist_iterate(it, lpaths) {
        const char* path = amxc_var_constcast(cstring_t, amxc_var_from_llist_it(it));
        amxt_tty_writef(tty, "${color.green}%s${color.reset}\n", path);
    }
}

int mod_pcb_cli_cmd_resolve(UNUSED const char* function_name,
                            amxc_var_t* args,
                            UNUSED amxc_var_t* ret) {
    amxc_var_t options;
    amxc_var_t paths;
    char* input_path = NULL;
    amxb_bus_ctx_t* bus_ctx = NULL;
    amxd_path_t path;
    int rv = 0;
    amxt_tty_t* tty = ba_cli_get_tty();

    amxc_var_init(&options);
    amxc_var_init(&paths);
    amxd_path_init(&path, NULL);

    rv = mod_pcb_cli_cmd_parse(args, PCB_RESOLVE, NULL, &input_path, &options, NULL);
    when_failed(rv, exit);

    bus_ctx = mod_pcb_cli_get_bus_ctx(input_path);
    when_null(bus_ctx, exit);

    amxd_path_setf(&path, false, "%s", input_path);
    rv = amxb_resolve(bus_ctx, &path, &paths);
    when_failed(rv, exit);

    if(!mod_pcb_cli_output_json(tty, &paths)) {
        mod_pcb_cli_write_resolved_paths(&paths);
    }

exit:
    amxc_var_clean(&options);
    amxc_var_clean(&paths);
    amxd_path_clean(&path);
    free(input_path);
    return rv;
}
