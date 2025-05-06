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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "wifi_scheduler.h"

static void schedule_triggered(UNUSED const char* const sig_name,
                               const amxc_var_t* const data,
                               void* const priv) {
    amxd_object_t* schedule = (amxd_object_t*) priv;
    char* path = amxd_object_get_path(schedule, 0);
    const char* reason = GET_CHAR(data, "reason");
    const char* id = GET_CHAR(data, "id");
    amxc_ts_t now;
    char time[40];

    amxc_ts_now(&now);
    amxc_ts_to_local(&now);
    amxc_ts_format_precision(&now, time, 40, 0);

    printf("'%s' of schedule '%s' called with reason %s at %s\n", id, path, reason, time);

    free(path);
}

static void update_scheduler(amxd_object_t* schedule) {
    amxd_object_t* parent = amxd_object_get_parent(schedule);
    amxp_scheduler_t* scheduler = (amxp_scheduler_t*) parent->priv;
    char* path = amxd_object_get_path(parent, 0);
    amxc_var_t params;

    amxc_var_init(&params);

    if(scheduler == NULL) {
        amxp_scheduler_new(&scheduler);
        amxp_scheduler_use_local_time(scheduler, true);
        parent->priv = scheduler;
    }

    amxd_object_get_params(schedule, &params, amxd_dm_access_public);

    amxp_scheduler_disconnect(scheduler, GET_CHAR(&params, "Alias"), schedule_triggered);
    if(amxp_scheduler_set_weekly_item(scheduler,
                                      GET_CHAR(&params, "Alias"),
                                      GET_CHAR(&params, "StartTime"),
                                      GET_CHAR(&params, "Day"),
                                      GET_UINT32(&params, "Duration"))) {
        printf("Failed to set scheduler item\n");
        goto exit;
    }

    printf("Schedule '%s' for '%s' created or updated\n", GET_CHAR(&params, "Alias"), path);

    amxp_scheduler_connect(scheduler,
                           GET_CHAR(&params, "Alias"),
                           schedule_triggered,
                           parent);

exit:
    free(path);
    amxc_var_clean(&params);
}

void _schedule_added(UNUSED const char* const event_name,
                     const amxc_var_t* const event_data,
                     UNUSED void* const priv) {
    amxd_object_t* schedules = amxd_dm_signal_get_object(wifi_scheduler_get_dm(), event_data);
    uint32_t index = GET_UINT32(event_data, "index");
    amxd_object_t* schedule = amxd_object_get_instance(schedules, NULL, index);

    update_scheduler(schedule);
}

void _schedule_changed(UNUSED const char* const event_name,
                       const amxc_var_t* const event_data,
                       UNUSED void* const priv) {
    amxd_object_t* schedule = amxd_dm_signal_get_object(wifi_scheduler_get_dm(), event_data);

    update_scheduler(schedule);
}

void _schedule_deleted(UNUSED const char* const event_name,
                       const amxc_var_t* const event_data,
                       UNUSED void* const priv) {
    amxd_object_t* schedules = amxd_dm_signal_get_object(wifi_scheduler_get_dm(), event_data);
    amxc_var_t* params = GET_ARG(event_data, "parameters");
    amxp_scheduler_t* scheduler = NULL;

    when_null(schedules, exit);
    scheduler = (amxp_scheduler_t*) schedules->priv;

    amxp_scheduler_remove_item(scheduler, GET_CHAR(params, "Alias"));

exit:
    return;
}