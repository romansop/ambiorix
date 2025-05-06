# Data Model Example - Scheduler

[[_TOC_]]

## Introduction

The purpose of this example is to demonstrate the use of the libamxp cron and scheduling API.

## Building `amx-scheduler` plugin

### Build Prerequisites

To be able to build this example you will need to install:

- `libamxc`  (Data collections and data containers)
- `libamxd`  (Data Model Management API)
- `libamxo`  (ODL Parsing and saving)
- `libamxp`  (Common Functional Patterns)

### Building

This example can be build in the `Ambiorix Debug And Development` container.

Change your working directory to the root of this example (that is the directory where you cloned the git repository in) and use `make`.

```Bash
cd <amx-scheduler>/
make
```

After the `make` has finished, you can install it by calling `sudo -E make install`.

Examples:

Install in your system root
```bash
$ sudo -E make install
```

## Running `scheduler` plugin

### Run Prerequisites

To be able to run this example you will need to:

- Build and install the example
- Install `amxrt` (`Ambiorix Runtime`)
- Install an `Ambiorix` bus back-end

A software bus must be running and the correct back-end must be installed.

In this explanation it is assumed that you have `ubusd` running and the `ubus` tool is installed (so the `amxb-ubus.so` should be available as well).

#### Configure The Container

1. Launch ubusd (in background) if it was not yet running - this should be done as `root` user
   ```bash
   $ ubusd &
   ```

### Running

Once it is built and everything is installed you can launch the example.

It is recommended to launch it as `root` user. When using the `Ambiorix` container open a termnal as root user:

```bash
docker exec -ti amxdev /bin/bash
```

Launch the scheduler exmaple controller called `amx-scheduler`. The example doesn't execute any tasks, it just prints to stdout when a task should be triggered, started or stopped. Therefor it is recommended to run it in foreground so the prints are visisble.

```bash
# amx-scheduler
```

## `Task Scheduler` - amx-scheduler

### Interact through data model using ubus

Before going into a detailed explanation of the example, try it out first.

1. Add a `Task` instance

   Using `ubus` command line tool
   ```bash
   $ ubus call SSH.Server _add
   {
        "object": "Scheduler.Task.cpe-task1.",
        "index": 1,
        "name": "cpe-task1",
        "parameters": {
                "Alias": "cpe-task1"
        },
        "path": "Scheduler.Task.1."
   }
   ```

   Alternatively use `ubus-cli`
   ```
   ubus-user - ubus: - [ubus-cli] (0)
   > Scheduler.Task.+{Alias="cpe-task1",Enable=true,Second="0/15"}
   Scheduler.Task.1.
   Scheduler.Task.1.Alias="cpe-task1"
   ```

   When the task is added you should see a message printed every 15 seconds of every minute starting at second 0 of each minute.
   ```
   cpe-task1 called with reason trigger at 2023-05-26T11:08:15.920475547Z
   cpe-task1 called with reason trigger at 2023-05-26T11:08:30.920397347Z
   cpe-task1 called with reason trigger at 2023-05-26T11:08:45.924757830Z
   cpe-task1 called with reason trigger at 2023-05-26T11:09:00.925767805Z
   cpe-task1 called with reason trigger at 2023-05-26T11:09:15.926143941Z
   cpe-task1 called with reason trigger at 2023-05-26T11:09:30.927257088Z
   ```

### The data model

#### Scheduler.Task object

The `Scheduler.Task` object is a multi-instance object and contains following parameters:
- Second
- Minute
- Hour
- DayOfMonth
- Month
- DayOfWeek

All of the above parameters are used in a cron expression and are strings. The cron expressions used by the `libamxp` cron parser consists of six fields:

```
<second> <minute> <hour> <day-of-month> <month> <day-of-week>
```

Each field in a cron expression can be one of:
- `*` (all) specifies that event should happen for every time unit.
- `<value>,<value>,<value>` specifies multiple values.
- `<value>-<value>` (range) determines the value range.
- `<value>/<increment>` (increments) specifies the inncremental values.

Examples of a cron expressions:

- `0 0 12 * * *` => at 12:00:00 pm (noon) every day
- `0 0/5 13,18 * * *` => Every five minutes starting at 1 pm and ending at 1:55 pm and the starting at 6pm and ending at 6:55pm every day
- `15,45 13 * 6 Tue` => At 1:15 pm and 1:45 pm every Tuesday in the month of June
- `0 0 12 1-7 * Mon` => Every first monday of the mont at 12:00:00.

Each parameter in the `Task` object represents a field in the cron expression.

Besides these parameters the task object also contains:
- Enable (boolean) Indicates if this task is enabled or disabled, if disabled the example will not print any message for this task.
- Duration (integer) Duration in seconds. The example will print a message when the task starts (indicated by the cron expression) and will print a message when the task stops (indicated by the duration).
- Alias (string) this is the unique key for the instance, this identifier is also used to add schedules to the scheduler.


#### Event subscriptions

In the `amx-scheduler_definition.odl` file some event handlers are installed on events of the `Scheduler.Task` object (or it's instances).

When a new instance is added:
```
    on event "dm:instance-added" call task_added
        filter 'path == "Scheduler.Task."';
```

When an instance is deleted:
```
    on event "dm:instance-removed" call task_deleted
        filter 'path == "Scheduler.Task."';
```

When parameters are changed in a Task
```
    on event "dm:object-changed" call task_changed
        filter 'path matches "Scheduler\.Task\.[0-9]+\."';
```

### Source Files

#### `scheduler_main.c`

This file contains:
- A structure definition. Only one instance of this structure will be available and contains the application specific data. The fields of the instance will be filled when the entry-point function is called with reason 0 (START).

   ```C
   typedef struct _scheduler_app {
      amxd_dm_t* dm;
      amxo_parser_t* parser;
      amxp_scheduler_t scheduler;
   } scheduler_app_t;

   static scheduler_app_t app;
   ```

- Some helper functions that can be used to retrieve the application's data model, the parser and the confifuration option and most important the scheduler instance used.

   ```C
   amxd_dm_t* scheduler_get_dm(void) {
      return app.dm;
   }

   amxc_var_t* scheduler_get_config(void) {
      return &app.parser->config;
   }

   amxo_parser_t* scheduler_get_parser(void) {
      return app.parser;
   }

   amxp_scheduler_t* scheduler_get_scheduler(void) {
      return &app.scheduler;
   }
   ```

- The entry-point implementation and some helper functions.

  ```
   int _scheduler_main(int reason,
                     amxd_dm_t* dm,
                     amxo_parser_t* parser) {

      switch(reason) {
      case 0:     // START
         app.dm = dm;
         app.parser = parser;
         amxp_scheduler_init(&app.scheduler);
         break;
      case 1:     // STOP
         amxp_scheduler_clean(&app.scheduler);
         app.dm = NULL;
         app.parser = parser;
         break;
      }

      return 0;
   }
  ```

#### `scheduler_event.c`

This file implements the event handlers that are defined in `amx-scheduler_definition.odl`

The `_task_added` function will create new schedule item and add it to the scheduler.
It fetches the parameters of the newly created instance and then calls the helper function
`tasks_update_scheduler`. That helper function is also used by the event handler function `_task_changed` and will create or update the schedule item. The `Alias` parameter is used as the schedule identifier.

```C
void _task_added(UNUSED const char* const event_name,
                 const amxc_var_t* const event_data,
                 UNUSED void* const priv) {
    amxd_object_t* tasks = amxd_dm_signal_get_object(scheduler_get_dm(), event_data);
    uint32_t index = GET_UINT32(event_data, "index");
    amxd_object_t* task = amxd_object_get_instance(tasks, NULL, index);
    amxc_var_t params;

    amxc_var_init(&params);
    amxd_object_get_params(task, &params, amxd_dm_access_public);
    tasks_update_scheduler(&params);

    amxc_var_clean(&params);
}
```

The `_task_deleted` function will removed the schedule item from the scheduler using the `Alias` parameter as schedule identifier.

```C
void _task_deleted(UNUSED const char* const event_name,
                   const amxc_var_t* const event_data,
                   UNUSED void* const priv) {
    amxc_var_t* params = GET_ARG(event_data, "parameters");
    amxp_scheduler_t* scheduler = scheduler_get_scheduler();

    amxp_scheduler_remove_item(scheduler, GET_CHAR(params, "Alias"));
}
```

The helper function `tasks_update_scheduler` will create a new schedule item or updates an exising one. It uses the parameter values from the instance object to build the cron expression. The `Alias` parameter value is used as the identifier of the schedule item.

The builded cron expression is parser and added to the scheduler using method `amxp_scheduler_set_cron_item`. The `Enable` parameter value is used to enable or disable to schedule item, this is done by calling the method `amxp_scheduler_enable_item`. 

If the provided cron expression is invalid or couldn't be added to the scheduler, the `Status` parameter will be set to "Error". 

The last step is to connect a callback function for the schedule item, that is done using `amxp_scheduler_connect`.

When the schedule item is triggered or started the callback function is called, when the item has a duration specified the callback function will be called as well when the duration has expired (time since start).

```C
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
```

The last function in this file is the schedule item callback function. The scheduler uses signal/slots mechanism of `libamxp`.

The scheduler will emit or trigger one of the following signals:
- `trigger:<ID>`
- `start:<ID>`
- `stop:<ID>`

The callback function just prints some information to stdout:

```C
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
```

Documentation is available for:
- cron API: http://sah2artifactory01.be.softathome.com:10000/documentation/ambiorix/libraries/libamxp/master/db/dab/a00089.html
- scheduler API: http://sah2artifactory01.be.softathome.com:10000/documentation/ambiorix/libraries/libamxp/master/dc/d54/a00094.html

Or you can build the documentation from the directory wher you cloned `libamxp`:

```
make doc
```
