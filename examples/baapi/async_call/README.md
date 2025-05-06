# Bus Agnostic Example - Asynchronous call

1. [Introduction](#introduction)  
2. [Building `amx-async-call`](#building-amx-async-call)  
2.1. [Build Prerequisites](#build-prerequisites)  
2.2. [Building](#building)  
3. [Running `amx-async-call`](#running-amx-async-call)  
3.1. [Run Prerequisites](#run-prerequisites)  
3.2. [Running](#running)

## Introduction

The `amx-async-call` bus agnostic example can be used to do an asynchronous RPC call to a `data model` provider. 

Using the `Ambiorix` framework it is possible to connect to different software bus systems (ubus, pcb, ...) and create applications that can interact with all these buses without the need of changing the code depending on the bus implementation that is used.

The `amx-async-call` application is an example. Most bus systems have support for method calls (RPC). This example demonstrates that it is possible with the `Ambiorix` framework to connect to a bus and invoke a method in an asynchronous way.

This examples demonstrates the following features:
- wait until an object is registered/available in the bus system (if the bus system supports this functionality)
- call a method of an data model provider in an asynchrous way.

## Building `amx-async-call`

### Build Prerequisites

To be able to build this example you will need to install:

- `libamxc`  (Data collections and data containers)
- `libamxj`
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
/usr/bin/install -D -p -m 0755 src/amx-async-call ./output/amx-async-call
```

## Running `amx-async-call`

### Run Prerequisites

To be able to run this example you will need to:

- Build and install the example
- Install an `Ambiorix` bus back-end
- Make a `data model` available on the bus with at least one RPC method
- A software bus must be running and the correct back-end must be installed.

In this explanation it is assumed that you have `ubus` running and the `ubus` tool is installed (so the `amxb-ubus.so` should be available as well). For the remote end the `greeter_plugin` example is used, this will expose some data model methods. Using ubus requires root permissions, so everything that is mentioned from now on must be run as a root user. You can switch to the root user with the following command:

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

This application can take some options, followed by three command line arguments.

```bash
amx-async-call [OPTIONS] <OBJECT PATH> <METHOD> [<ARGS>]
```

- OBJECT PATH - the object path of the `data model` object of which you want to call a method.
- METHOD - the name of the method you want to call.
- ARGS - (optional) the method in arguments provided in JSON format.

```bash
$ amx-async-call Greeter. say '{"from":"me", "message":"Hello world"}' 
```

When launched the method `say` of object `Greeter.` is called. The return value and out argumetns (if any) are printed to stdout.

---
> **NOTE**
> To make the `Greeter.say` work you need to set the `State` of the `Greeter` application.
> ```bash
> ubus call Greeter _set '{"parameters":{"State":"Start"}}'
> ```

```text
$ amx-async-call Greeter. say '{"from":"me", "message":"Hello World"}'
Function call done status = 0
Function return value = 
"Hello World"
Function out args = 
NULL
```
