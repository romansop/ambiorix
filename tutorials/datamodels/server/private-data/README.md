# Ambiorix Data Models - Private Data

[[_TOC_]]

## Introduction

A data model containing a hierarchical tree of objects (nodes), where each object contains a set of parameters can be used to store configuration options or to represent the current state of a service or application.

Client applications can interact with such a data model to change the configuration or to query the current state.

Often these services keep internal data structures, that are not exposed in the public data model. These private data structures can be system specific, like network interface data structures, process data structures etc.

The service or application can `attach` these data structures to the data model objects or even to data model parameters, for easy and quick access. 

## Goal

Learn how to store and connect private data structures to data model objects.

## Prerequisites

- You finished the [Component Structure](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/build-debug/component-structure/-/blob/main/README.md) tutorial
- You finished the [RPC methods](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/rpc-methods/-/blob/main/README.md) tutorial

## Private Data

This tutorial will create a data model that can launch child processes, and the C data structure that manages the child process is attached to the object instances that represent the current state of the child process.

The Ambiorix library [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp) provides an API that helps in launching and monitoring child processes. The [process control](http://sah2artifactory01.be.softathome.com:10000/documentation/ambiorix/libraries/libamxp/master/d9/db3/a00066.html) and [Subprocess](http://sah2artifactory01.be.softathome.com:10000/documentation/ambiorix/libraries/libamxp/master/d6/d2c/a00072.html) API. These API's work with the C data structure [amxp_proc_ctrl_t](http://sah2artifactory01.be.softathome.com:10000/documentation/ambiorix/libraries/libamxp/main/d4/d63/a00082.html), which is defined in libamxp.  

### Creating A Child Process

Creating, launcing and stopping a child process using the process control API is straight forward using the following methods:

```C
int amxp_proc_ctrl_new(amxp_proc_ctrl_t** proc, amxp_proc_ctrl_cmd_t cmd_build_fn);
int amxp_proc_ctrl_start(amxp_proc_ctrl_t* proc, uint32_t minutes, amxc_var_t* settings);
int amxp_proc_ctrl_stop(amxp_proc_ctrl_t* proc);
```

The first one will allocate and initializes an `amxp_proc_ctrl_t` structure, the second will launch the child-process and the last one will stop it.

### Tracking Child Process State

The parent process needs to track the state of it's started child-processes, and needs to know when it has stopped for wathever reason. The linux system will send a signal [SIGCHLD](https://man7.org/linux/man-pages/man7/signal.7.html). When using [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt) this signal can be monitored by enabling it in the `%config` section of the main odl file:

```odl
%config {
    ...

    system-signals = [ 17 ]; // enable SIGCHILD

    ...
}
```

The `amxp process control` implementation will translate this system signal to the Ambiorix signal `proc:stopped`, which is passed to the even loop and can be handled by connecting a `slot` callback function to it. More information about Ambiorix signal/slot mechanism can be found in the document [signal/slot](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp/-/blob/main/doc/signal_slot.md).
blob/master/doc/signal_slot.md).

All you need to do is add the Ambiorix signal `proc:stopped` to the global Ambiorix signal manager and connect a `slot` callback function to it. This is done in the function `launcher_init` which can be found in `launcher_main.c`.


```C
static int launcher_init(amxd_dm_t* dm, amxo_parser_t* parser) {
    ...

    amxp_sigmngr_add_signal(NULL, "proc:stopped");
    amxp_slot_connect(NULL, "proc:stopped", NULL, launcher_proc_stopped, NULL);

    ...
}
```

### The Launcher Data Model

Now that we know how to create, start, stop and monitor a child process, it is time to define a simple data model.

```odl
%define {
    %persistent object Launcher {

        %persistent object Application[] {
            on action destroy call cleanup_proc_ctrl;

            %persistent ssv_string Command;
            string Status = "Idle" {
                on action validate call check_enum
                    ["Idle", "Running", "Stopped"];
            }
            %read-only uint32 PID = 0;
            %read-only int32 ExitCode = 0;

            bool start();
            bool stop();
        }
    }
}
```

The above definition can be found in the file `launcher_definition.odl`. The `Launcher` object contains a multi-instance object `Application` where a space separated string is defined that will contain the command that must be executed. This command can be anything and will have the same format as you use in the shell.

Example:

```bash
root@3263f0f79744:/home/sah4009/development/ambiorix# ls -la
total 52
drwxr-xr-x 13 sah4009 normal_users 4096 Feb  8 08:47 .
drwxr-xr-x 11 sah4009 normal_users 4096 Feb 18 04:51 ..
drwxr-xr-x  7 sah4009 normal_users 4096 Feb  8 08:42 .repo
drwxr-xr-x  2 root    root         4096 Oct  3  2021 .vscode
drwxr-xr-x  5 sah4009 normal_users 4096 Feb  4 07:04 ambiorix
drwxr-xr-x  9 sah4009 normal_users 4096 Sep 30  2021 applications
drwxr-xr-x  4 sah4009 normal_users 4096 Feb  8 08:47 bindings
drwxr-xr-x  5 sah4009 normal_users 4096 Sep 30  2021 bus_adaptors
drwxr-xr-x  4 sah4009 normal_users 4096 Sep 30  2021 cli_extensions
drwxr-xr-x  5 sah4009 normal_users 4096 Sep 30  2021 dockers
drwxr-xr-x  7 sah4009 normal_users 4096 Feb  8 08:47 examples
drwxr-xr-x 12 sah4009 normal_users 4096 Feb  8 08:48 libraries
drwxr-xr-x  3 sah4009 normal_users 4096 Oct  3 19:13 tools
```

Using the data model definition it is possible to create an instance of the `Application` object and set the command to `ls -la`.

Adding an instance can be done with the ubus tool:

```bash
ubus call Launcher.Application _add '{"parameters":{"Command":"ls -la"}}'
{
        "object": "Launcher.Application.1.",
        "index": 1,
        "name": "1",
        "parameters": {

        },
        "path": "Launcher.Application.1."
}

```

Using the `_get` RPC method you can verify that the instance is created: 


```
root@3263f0f79744:/# ubus call Launcher.Application.1 _get
{
        "Launcher.Application.1.": {
                "PID": 0,
                "Command": "ls -la",
                "Status": "Idle",
                "ExitCode": 0
        }
}
```

Now it is time to put all things together, jump to [lab 1](#lab-1-implement-start-and-stop-data-model-rpc-methods). 

### Freeing the memory

The `amxp_proc_ctrl_t` that is allocated when the child process is started must be freed at some point in time, otherwise you will end up with memory leaks.

It is possible to free the allocated structure when the child process is stopped using the data model RPC method `stop`, but this will not cover all cases. 

A better place to free the allocated structure is when the instance object is removed. In the odl file a callback function can be added to the multi-instance object that will be called when the multi-instance object is deleted. This callback function will be inherited by the instance objects. 

To add the callback function in the odl a `destroy` action must be added:

```odl
    on action destroy call cleanup_proc_ctrl;
```

Switch to [lab 2](#Lab-2-add-destroy-action) to add the destroy action.

You can take a look at the example [ssh-server](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/ssh-server) for a more real life implementation using the child process control API and private object data.


## Practical Labs

By now you should known the drill. All practical labs are done in the `Ambiorix Debug & Development` container and in terminals opened in that container. If you didn't already clone the repository of this tutorial, it is now time to do so.

To build this tutorial, use `make` in the root directory of the git repository, to install it use `sudo -E make install`, use `launcher` to start it.

#### Lab 1 - Implement start and stop data model RPC methods

Calling the RPC method `Launcher.Application.1.start()` should launch the command. To launch the command a `amxp_proc_ctrl_t` structure must be allocated. This allocated structure can the be attached to the instance object. Each object in the data model has a member called `priv`, which is a void pointer and can contain any arbitrary pointer. The Ambiorix libraries will only pass the private data to callback functions, but will not use that pointer for other purposes.

Calling the RPC method `Launcher.Application.1.stop()` should stop the started child process.

In the file `src/dm_launcher_methods.c` you will find a template implementation of both RPC methods.

The start RPC method:

```C
amxd_status_t _start(amxd_object_t* object,
                     UNUSED amxd_function_t* func,
                     UNUSED amxc_var_t* args,
                     UNUSED amxc_var_t* ret) {
    int retval = 0;

    // TODO: fetch the attached amxp_proc_ctrl_t pointer

    char* status = amxd_object_get_value(cstring_t, object, "Status", NULL);
    amxd_param_t* cmd = amxd_object_get_param_def(object, "Command");
    amxc_var_t settings;
    amxc_var_init(&settings);

    // TODO: check if a amxp_proc_ctrl_t was already attached, if not allocate one
    //       and attach it to the object

    if((status == NULL) || (strcmp(status, "Running") != 0)) {
        // TODO: convert the command to a list
        //       launch the command (child process)

        if(retval == 0) {
            retval = launcher_update_status(object, "Running");
        } else {
            launcher_update_status(object, "Error");
        }
    }

    free(status);

    return (retval == 0)?amxd_status_ok:amxd_status_unknown_error;
}
```

To be able to launch the child process using the `amxp_proc_ctrl_t` API you will need a method that builds an array that contains the command. You can use this method:

```C
static int launcher_build_cmd(amxc_array_t* cmd, amxc_var_t* settings) {
    amxc_var_for_each(p, settings) {
        amxc_array_append_data(cmd, strdup(GET_CHAR(p, NULL)));
        amxc_var_delete(&p);
    }

    return 0;
}
```

The stop RPC method:

```C
amxd_status_t _stop(amxd_object_t* object,
                     UNUSED amxd_function_t* func,
                     UNUSED amxc_var_t* args,
                     UNUSED amxc_var_t* ret) {
    int retval = 0;
    // TODO: fetch the attached amxp_proc_ctrl_t pointer
    char* status = amxd_object_get_value(cstring_t, object, "Status", NULL);

    // TODO: check if amxp_proc_Ctrl_t pointer was attached, if not leave

    if((status != NULL) || (strcmp(status, "Running") == 0)) {
        // TODO: stop the child process
    }

    return (retval == 0)?amxd_status_ok:amxd_status_unknown_error;
}
```

When both methods are implemented, you should be able to start and stop the child processes. Note that the function `launcher_update_status` will update the instance object using the allocated `amxp_proc_ctrl_t` which is attached to the object. It will update the parameter `PID` (process ID) and when the process stops it will also set the `ExitCode` parameter.

When the child processes stops by itselfs for any reason, the system signal SIGCHLD will be triggered. The ambiorix library libamxp will catch this signal and adds it to the eventloop, so it can be handled. The handling of the signal is implemented in the file `src/launcher_main.c`

```C
static void launcher_proc_stopped(UNUSED const char* const event_name,
                                  const amxc_var_t* const event_data,
                                  UNUSED void* const priv) {
    uint32_t pid = amxc_var_dyncast(uint32_t, event_data);
    amxd_dm_t* dm = launcher_get_dm();
    amxd_object_t* object = NULL;

    object = amxd_dm_findf(dm, "Launcher.Application.[ PID == %d ].", pid);
    if(object != NULL) {
        launcher_update_status(object, "Stopped");
    }
}

static int launcher_init(amxd_dm_t* dm, amxo_parser_t* parser) {
    launcher.dm = dm;
    launcher.parser = parser;

    amxp_sigmngr_add_signal(NULL, "proc:stopped");
    amxp_slot_connect(NULL, "proc:stopped", NULL, launcher_proc_stopped, NULL);

    return 0;
}
```

#### Lab 2 - Add destroy action

In the odl definition file `odl/launcher_definition.odl` add the destroy action to the multi-instance object `Application`

In the file `src/dm_launcher_action.c` this action callback can be implemented. 

```C
amxd_status_t _cleanup_proc_ctrl(amxd_object_t* object,
                              UNUSED amxd_param_t* param,
                              amxd_action_t reason,
                              UNUSED const amxc_var_t* const args,
                              amxc_var_t* const retval,
                              UNUSED void* priv) {
    amxd_status_t status = amxd_status_ok;
    amxp_proc_ctrl_t* proc_ctrl = NULL;

    if(reason != action_object_destroy) {
        status = amxd_status_function_not_implemented;
        goto leave;
    }

    // fetch the amxp_proc_ctrl_t pointer
    proc_ctrl = (amxp_proc_ctrl_t*) object->priv;

    if (proc_ctrl != NULL) {
        // TODO: free the allocated structure
    }
    
    amxc_var_clean(retval);

leave:
    return status;
}
```


## References

- Tutorial: Component Structure<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/build-debug/component-structure/-/blob/main/README.md
- Tutorial: RPC Methods<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/rpc-methods/-/blob/main/README.md
- Library libamxp<br>
https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp
- libamxp: process control<br>
http://sah2artifactory01.be.softathome.com:10000/documentation/ambiorix/libraries/libamxp/master/d9/db3/a00066.html
- libamxp: subprocess<br>
http://sah2artifactory01.be.softathome.com:10000/documentation/ambiorix/libraries/libamxp/master/d6/d2c/a00072.html
- Linux signals<br>
https://man7.org/linux/man-pages/man7/signal.7.html
- Application: amxrt<br>
https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt
- Ambiorix signal/slot<br>
https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp/-/blob/main/doc/signal_slot.md
- Example: ssh-server<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/ssh-server