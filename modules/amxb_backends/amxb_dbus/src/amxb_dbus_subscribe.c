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

#include "amxb_dbus_version.h"
#include "amxb_dbus.h"
#include "amxb_dbus_methods.h"

int amxb_dbus_subscribe(void* const ctx, const char* object) {
    amxb_dbus_t* amxb_dbus_ctx = (amxb_dbus_t*) ctx;
    amxd_path_t path;
    amxc_string_t dbus_filter;
    const amxc_var_t* interface = NULL;
    char* root_obj = NULL;
    amxc_htable_it_t* it = NULL;
    amxb_dbus_sub_t* amxb_dbus_sub = NULL;

    amxc_string_init(&dbus_filter, 0);
    amxd_path_init(&path, NULL);
    amxd_path_setf(&path, true, "%s", object);

    it = amxc_htable_get(&amxb_dbus_ctx->subscribers, amxd_path_get(&path, 0));
    when_not_null(it, exit);

    amxb_dbus_sub = (amxb_dbus_sub_t*) calloc(1, sizeof(amxb_dbus_sub_t));
    amxc_htable_insert(&amxb_dbus_ctx->subscribers,
                       object,
                       &amxb_dbus_sub->it);

    interface = amxb_dbus_get_config_option("dm-interface");

    root_obj = amxd_path_get_first(&path, false);
    amxc_string_setf(&dbus_filter,
                     "type='signal',interface='%s',member='dmevent'",
                     GET_CHAR(interface, NULL));
    dbus_bus_add_match(amxb_dbus_ctx->dbus_handle, amxc_string_get(&dbus_filter, 0), NULL);

exit:
    free(root_obj);
    amxc_string_clean(&dbus_filter);
    amxd_path_clean(&path);

    return 0;
}
