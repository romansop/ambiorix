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

#include "scheduler.h"

static void task_triggered(UNUSED const char* const sig_name,
                           const amxc_var_t* const data,
                           UNUSED void* const priv) {
    const char* reason = GET_CHAR(data, "reason");
    const char* id = GET_CHAR(data, "id");
    amxc_ts_t now;
    char time[40];

    amxc_ts_now(&now);
    amxc_ts_format(&now, time, 40);

    printf("%s called with reason %s at %s\n", id, reason, time);

}

static void task_set_status(const amxd_object_t* task, const char* status) {
    amxd_trans_t trans;

    amxd_trans_init(&trans);
    amxd_trans_set_attr(&trans, amxd_tattr_change_ro, true);
    amxd_trans_select_object(&trans, task);
    amxd_trans_set_value(cstring_t, &trans, "Status", status);
    amxd_trans_apply(&trans, scheduler_get_dm());

    amxd_trans_clean(&trans);
}

static void tasks_update_scheduler(amxd_object_t* task) {
    amxp_scheduler_t* scheduler = scheduler_get_scheduler();
    amxc_var_t params;
    amxc_string_t cron_expr;

    amxc_string_init(&cron_expr, 0);
    amxc_var_init(&params);

    amxd_object_get_params(task, &params, amxd_dm_access_public);

    amxc_string_setf(&cron_expr,
                     "%s %s %s %s %s %s",
                     GET_CHAR(&params, "Second"),
                     GET_CHAR(&params, "Minute"),
                     GET_CHAR(&params, "Hour"),
                     GET_CHAR(&params, "DayOfMonth"),
                     GET_CHAR(&params, "Month"),
                     GET_CHAR(&params, "DayOfWeek"));

    amxp_scheduler_disconnect(scheduler, GET_CHAR(&params, "Alias"), task_triggered);
    if(amxp_scheduler_set_cron_item(scheduler,
                                    GET_CHAR(&params, "Alias"),
                                    amxc_string_get(&cron_expr, 0),
                                    GET_UINT32(&params, "Duration"))) {
        printf("Invalid cron expression: %s\n", amxc_string_get(&cron_expr, 0));
        task_set_status(task, "Error");
        goto exit;
    }

    printf("Scheduler created or updated\n");
    amxp_scheduler_enable_item(scheduler,
                               GET_CHAR(&params, "Alias"),
                               GET_BOOL(&params, "Enable"));
    if(GET_BOOL(&params, "Enable")) {
        task_set_status(task, "Running");
    } else {
        task_set_status(task, "Disabled");
    }

    amxp_scheduler_connect(scheduler,
                           GET_CHAR(&params, "Alias"),
                           task_triggered,
                           NULL);

exit:
    amxc_var_clean(&params);
    amxc_string_clean(&cron_expr);
}

void _task_added(UNUSED const char* const event_name,
                 const amxc_var_t* const event_data,
                 UNUSED void* const priv) {
    amxd_object_t* tasks = amxd_dm_signal_get_object(scheduler_get_dm(), event_data);
    uint32_t index = GET_UINT32(event_data, "index");
    amxd_object_t* task = amxd_object_get_instance(tasks, NULL, index);

    tasks_update_scheduler(task);
}

void _task_changed(UNUSED const char* const event_name,
                   const amxc_var_t* const event_data,
                   UNUSED void* const priv) {
    amxd_object_t* task = amxd_dm_signal_get_object(scheduler_get_dm(), event_data);
    tasks_update_scheduler(task);
}

void _task_deleted(UNUSED const char* const event_name,
                   const amxc_var_t* const event_data,
                   UNUSED void* const priv) {
    amxc_var_t* params = GET_ARG(event_data, "parameters");
    amxp_scheduler_t* scheduler = scheduler_get_scheduler();

    amxp_scheduler_remove_item(scheduler, GET_CHAR(params, "Alias"));
}