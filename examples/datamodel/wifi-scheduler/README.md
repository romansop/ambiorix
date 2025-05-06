# Data Model Example - WiFi Scheduler

[[_TOC_]]

## Introduction

The purpose of this example is to demonstrate the use of the libamxp scheduling API. It uses the WiFi schedule data model definition to show how the API could be used.

## Building `wifi-scheduler` plugin

### Build Prerequisites

To be able to build this example you will need to install:

- `libamxc`  (Data collections and data containers)
- `libamxd`  (Data Model Management API)
- `libamxo`  (ODL Parsing and saving)
- `libamxp`  (Common Functional Patterns)
- `mod-dmext` Data model extention - used for parameter validation

### Building

This example can be build in the `Ambiorix Debug And Development` container.

Change your working directory to the root of this example (that is the directory where you cloned the git repository in) and use `make`.

```Bash
cd <wifi-scheduler>/
make
```

After the `make` has finished, you can install it by calling `sudo -E make install`.

Examples:

Install in your system root
```bash
$ sudo -E make install
```

## Running `wifi-scheduler` plugin

### Run Prerequisites

To be able to run this example you will need to:

- Build and install the example
- Install `amxrt` (`Ambiorix Runtime`)
- Install an `Ambiorix` bus back-end
- Install the data model extention module `mod-dmext`

A software bus must be running and the correct back-end must be installed.

In this explanation it is assumed that you have `ubusd` running and the `ubus` tool is installed (so the `amxb-ubus.so` should be available as well). Optionally the ambiorix cli tool can be installed and used.

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

Launch the scheduler exmaple controller called `wifi-scheduler`. The example doesn't execute any tasks, it just prints to stdout when a task should be triggered, started or stopped. Therefor it is recommended to run it in foreground so the prints are visible.

```bash
# wifi-scheduler
```

## `WiFi Scheduler` - wifi-scheduler

### Interact through data model using ubus

Before going into a detailed explanation of the example, try it out first.

#### Add a `Schedule` instance

Using `ubus` command line tool
```bash
$ ubus call WiFi.DataElements.Network.SSID.1.Schedule _add '{"parameters":{"Day":"tuesday,wednesday", "StartTime":"09:00", "Duration":3600 }}'
{
   "object": "WiFi.DataElements.Network.SSID.wl0.Schedule.cpe-Schedule-1.",
   "index": 1,
   "name": "cpe-Schedule-1",
   "parameters": {
      "Alias": "cpe-Schedule-1"
   },
   "path": "WiFi.DataElements.Network.SSID.1.Schedule.1."
}
{

}
{
   "amxd-error-code": 0
}
```

The above command will create a new instance for `WiFi.DataElements.Network.SSID.1.Schedule`. This new schedule instance will cause that a message is printed every `Tuesday` and `Wednesday` at 09:00 and 1 hour later (3600 seconds) another message will be printed.

Alternatively use `ubus-cli`
```
$ ubus-cli 
Copyright (c) 2020 - 2023 SoftAtHome
amxcli version : 0.2.28

!amx silent true

                  _  ___  ____
_ __  _ __ _ __ | |/ _ \/ ___|
| '_ \| '__| '_ \| | | | \___ \
| |_) | |  | |_) | | |_| |___) |
| .__/|_|  | .__/|_|\___/|____/
|_|        |_| based on OpenWrt
-----------------------------------------------------
ubus - cli
-----------------------------------------------------

sah4009 - ubus: - [ubus-cli] (0)
>  WiFi.DataElements.Network.SSID.1.Schedule+{Day="tuesday,wednesday",StartTime="08:00",Duration=10}
WiFi.DataElements.Network.SSID.1.Schedule.2.
WiFi.DataElements.Network.SSID.1.Schedule.2.Alias="cpe-Schedule-2"
```

When schedule instance is created you should see a message printed in the terminal where the `wifi-scheduler` is running
```
Schedule 'cpe-Schedule-1' for 'WiFi.DataElements.Network.SSID.wl0.Schedule' created
Schedule 'cpe-Schedule-2' for 'WiFi.DataElements.Network.SSID.wl0.Schedule' created
```

The above command will create a new instance for `WiFi.DataElements.Network.SSID.1.Schedule`. This new schedule instance will cause that a message is printed every `Tuesday` and `Wednesday` at 08:00 and 10 seconds later another message will be printed.

### The data model

#### Schedule objects

In this example data model three schedule objects are defined:

- WiFi.DataElements.Network.Schedule.{i}.
- WiFi.DataElements.Network.SSID.{i}.Schedule.{i}.
- WiFi.DataElements.Network.Group.{i}.Schedule.{i}.

All three are multi-instance objects and provide the same set of parameters:
- Alias - (string) the unique key for the instances.
- Day - (csv_string) comma separated list of values, each value must be a name of a weekday.
- StartTime - (string) must be the time in "HH:MM" format. [HH] refers to a zero-padded hour between 00 and 23. [MM] refers to a zero-padded minute between 00 and 59.
- Duration - (uint32) duration in seconds, maximum duration is 24 * 60 * 60 * 7 (is the maximum number of seconds in a week)

Only the parameters related to the schedule are mentioned here, other parameters may be available and defined.

```
   %unique %key string Alias;
   %persistent csv_string Day {
      on action validate call check_enum ["monday","tuesday","wednesday","thursday","friday","saterday","sunday"];
   }
   %persistent string StartTime {
      on action validate call matches_regexp  "^([0-1]?[0-9]|2[0-3]):[0-5][0-9]$";
   }
   %persistent uint32 Duration {
      on action validate call check_maximum 604800;
   }
```
#### Event subscriptions

For each `Schedule` multi-instance object, some event handlers are added. 

```
   on event "dm:instance-added" call schedule_added;
   on event "dm:instance-removed" call schedule_deleted;
   on event "dm:object-changed" call schedule_changed;
```

When a schedule instance is added the function `schedule_added` will be called. This function will create a new
schedule item and activates it.

When a schedule instance is deleted the function `schedule_delete` will be called. This function will remove a schedule item. After deletion no callback for this schedule will be called again.

When one of the parameter values of an instance are changed the function `schedule_changed` will be called. When needed the existing schedule item will be updated.

#### Action handlers

As each `Schedule` multi-instance object will manage its own instance of a `amxp_scheduler_t` structure, which will be stored as private data in the multi-instance object, a destroy handler is added to each of them. The destroy handler will do the clean-up of the scheduler.

```
   on action destroy call schedule_cleanup;
```

### Implementation

The implementation uses the following libamxp schedule API's:

- `amxp_scheduler_new`
- `amxp_scheduler_use_local_time`
- `amxp_scheduler_set_weekly_item`
- `amxp_scheduler_remove_item`
- `amxp_scheduler_delete`

Three source files provide the implementation of this example:

- wifi_scheduler_main.c
- wifi_scheduler_event.c
- wifi_scheduler_action.c

#### `wifi_scheduler_main.c`

This file contains:
- A structure definition. Only one instance of this structure will be available and contains the application specific data. The fields of the instance will be filled when the entry-point function is called with reason 0 (START).

   ```C
   typedef struct _wifi_scheduler_app {
      amxd_dm_t* dm;
      amxo_parser_t* parser;
   } wifi_scheduler_app_t;

   static wifi_scheduler_app_t app;
   ```

- Some helper functions that can be used to retrieve the application's data model, the parser and the confifuration option.

   ```C
   amxd_dm_t* wifi_scheduler_get_dm(void) {
      return app.dm;
   }

   amxc_var_t* wifi_scheduler_get_config(void) {
      return &app.parser->config;
   }

   amxo_parser_t* wifi_scheduler_get_parser(void) {
      return app.parser;
   }
   ```

- The entry-point implementation. This method will be called by the odl parser after all odl files are read and the data model is created.

  ```
   int _wifi_scheduler_main(int reason,
                           amxd_dm_t* dm,
                           amxo_parser_t* parser) {

      switch(reason) {
      case 0:     // START
         app.dm = dm;
         app.parser = parser;
         break;
      case 1:     // STOP
         app.dm = NULL;
         app.parser = parser;
         break;
      }

      return 0;
   }
  ```

#### `wifi_scheduler_event.c`

This file implements the event handlers that are defined in `wifi-scheduler_definition.odl`

The `_schedule_added` function will create new schedule item and add it to the scheduler.
It fetches the parameters of the newly created instance and then calls the helper function
`update_scheduler`. That helper function is also used by the event handler function `_schedule_changed` and will create or update the schedule item. The `Alias` parameter is used as the schedule identifier.

```C
void _schedule_added(UNUSED const char* const event_name,
                     const amxc_var_t* const event_data,
                     UNUSED void* const priv) {
    amxd_object_t* schedules = amxd_dm_signal_get_object(wifi_scheduler_get_dm(), event_data);
    uint32_t index = GET_UINT32(event_data, "index");
    amxd_object_t* schedule = amxd_object_get_instance(schedules, NULL, index);

    update_scheduler(schedule, true);
}
```

The `_schedule_deleted` function will removed the schedule item from the scheduler using the `Alias` parameter as schedule identifier.

```C
void _schedule_deleted(UNUSED const char* const event_name,
                       const amxc_var_t* const event_data,
                       UNUSED void* const priv) {
    amxd_object_t* schedules = amxd_dm_signal_get_object(wifi_scheduler_get_dm(), event_data);
    amxc_var_t* params = GET_ARG(event_data, "parameters");
    amxp_scheduler_t* scheduler = (amxp_scheduler_t*) schedules->priv;

    amxp_scheduler_remove_item(scheduler, GET_CHAR(params, "Alias"));
}
```

The helper function `update_scheduler` will create a new schedule item or updates an exising one. It uses the parameter values from the instance object to build the schedule item. The `Alias` parameter value is used as the identifier of the schedule item.

If the multi-instance object (parent of the new instance) doesn't contain a `amxp_scheduler_t` instance a new one is allocated using `amxp_scheduler_new` and set as private data on the multi-instance object. The scheduler is set to work on local time, so it is easier for an end user to input data. This is done using method `amxp_scheduler_use_local_time`. (By default a scheduler will work on UTC time).

```C
    if(scheduler == NULL) {
        amxp_scheduler_new(&scheduler);
        amxp_scheduler_use_local_time(scheduler, true);
        parent->priv = scheduler;
    }
```

A new schedule item is added using `amxp_scheduler_set_weekly_item` and the instance object parameters values are used and passed to this function:
- `Alias` - used as the identifer for the schedule item.
- `StartTime` - time in "HH:MM" format
- `Day` - list of week days
- `Duration` - number of seconds

If a schedule item already exists with the given id (Alias value), the schedule item will be updated.

If creation or updating the item fails, a message will be printed to stdout.

```C
    amxd_object_get_params(schedule, &params, amxd_dm_access_public);
    if(amxp_scheduler_set_weekly_item(scheduler,
                                      GET_CHAR(&params, "Alias"),
                                      GET_CHAR(&params, "StartTime"),
                                      GET_CHAR(&params, "Day"),
                                      GET_UINT32(&params, "Duration"))) {
        printf("Failed to set scheduler item\n");
        goto exit;
    }
```

If it is a new instance (argument `is_new` will be set to true), a callback function is connected to the schedule item. This callback function will be called at the correct time (depending on Day and StartTime values) with reason "start" and will be called (depending on Duration value) with reason "stop".

```C
   amxp_scheduler_connect(scheduler,
                           GET_CHAR(&params, "Alias"),
                           schedule_triggered,
                           parent);
```

The last function in this file is the schedule item callback function. The scheduler uses signal/slots mechanism of `libamxp`.

The scheduler will emit or trigger one of the following signals:
- `trigger:<ID>` - if duration is 0
- `start:<ID>` 
- `stop:<ID>`

The callback function just prints some information to stdout:

```C
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
```

Documentation is available for:
- scheduler API: http://sah2artifactory01.be.softathome.com:10000/documentation/ambiorix/libraries/libamxp/master/dc/d54/a00094.html

Or you can build the documentation from the directory where you cloned `libamxp`:

```
make doc
```
