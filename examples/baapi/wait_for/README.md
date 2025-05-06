# Bus Agnostic Example - Subscribe

1. [Introduction](#introduction)  
2. [Building `amx-wait-for`](#building-amx-wait-for)  
2.1. [Build Prerequisites](#build-prerequisites)  
2.2. [Building](#building)  
3. [Running `amx-wait-for`](#running-amx-wait-for)  
3.1. [Run Prerequisites](#run-prerequisites)  
3.2. [Running](#running)

## Introduction

The `amx-wait-for` bus agnostic example can be used to wait for an object until it becoms available on the used bus system. An object will become availabe as soon as an `data model` provider registered the object on the bus system. 

Using the `Ambiorix` framework it is possible to connect to different software bus systems (ubus, pcb, ...) and create applications that can interact with all these buses without the need of changing the code depending on the bus implementation that is used.

The `amx-wait-for` application is an example. Most bus systems have support for notifications or events. This example demonstrates that it is possible with the `Ambiorix` framework to connect to a bus and wait until an object vecomes available on the used bus system, basically it waits for a specific event.

This examples demonstrates the following features:
- wait until an object is registered/available in the bus system (if the bus system supports this functionality)

## Building `amx-wait-for`

### Build Prerequisites

To be able to build this example you will need to install:

- `libamxc`  (Data collections and data containers)
- `libamxp`  (Common Functional Patterns)
- `libamxb`  (Bus agnostic API)
- `libamxrt`

### Building

Change your working directory to the root of this example and use `make`.

```Bash
cd <SubscribeRoot>/
make
```

After the `make` has finished, you can install it by calling `sudo -E make install`. This will
install the application in `/usr/bin/` by default. You can specify a different install location
for example the output directory by using `make install DEST=./output BINDIR=""`.

Examples:

Install to output directory
```bash
$ make install DEST=./output BINDIR=""
make -C src all
/usr/bin/install -D -p -m 0755 src/amx-wait-for ./output/amx-wait-for
```

## Running `amx-wait-for`

### Run Prerequisites

To be able to run this example you will need to:

- Build and install the example
- Install an `Ambiorix` bus back-end
- Make a `data model` available on the bus

In this explanation it is assumed that you have `ubus` running and the `ubus` tool is installed (so the `amxb-ubus.so` should be available as well). For the remote end the `greeter_plugin` example is used, this will register the object `Greeter.` to the bus system. Using ubus requires root permissions, so everything that is mentioned from now on must be run as a root user. You can switch to the root user with the following command:

```bash
sudo -E su -
```

Before continuing, make sure that you have the `greeter_plugin` example running (as root) and it is connected to ubus.

You can verify this by using the `ubus` tool.

```bash
$ ubus list
Greeter
Greeter.History
Greeter.History.1
Greeter.Statistics
```

### Running

Once it is built and everything is installed you can launch the example.

This application can take some options, followed by two command line arguments.

```bash
amx-wait-for [OPTIONS] <OBJECT>
```

- OBJECT - the object path of the `data model` object that you want to check if it is available or wait until it becomes available.

```bash
$ amx-wait-for Greeter.History. 
```

If the `greeter_plugin` is running, the application will immediatly exits and tells you that the object `Greeter.History.` is available. If you stop the `greeter_plugin` and then relaunch the `amx-wait-for` it will wait until the object `Greeter.History.` is available. So it exits as soon as you launch the `greeter_plugin` again.

### Lua Alternative

The same can be achieved with a simple lua script:

```lua
local lamx = require 'lamx'

lamx.auto_connect()

local el = lamx.eventloop.new()


local objects_available = function()
    print("Wait done - object " .. arg[1] .. " is available");
    el:stop()
end

print("Wait until object " .. arg[1] .. " is available");
lamx.bus.wait_for(arg[1], objects_available)

el:start()

lamx.disconnect_all()
```
