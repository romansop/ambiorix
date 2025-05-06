# Ambiorix Data Models - Define And Publish

Estimated Time needed: 50 - 70 minutes.

[[_TOC_]]

## Introduction

The Ambiorix framework can help in defining a data model that is fully compliant with a standard data model or a full custom data model.

What is defined in a data model and which functionality it exposes depends on your specific use case. The framework does not provide an implemented data model but provides you tools and API's to make it possible to define, publish and maintain a data model.

## Goal

The goal of this tutorial is to explain:

- How to define a data model using the Ambiorix Object Definition Language
- How to publish a data model using the Ambiorix tools
- How to use the platform specific tools to interact with your published data model.

## Prerequisites

- You finished the [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) tutorial
- You have a good understanding of the data model concepts as defined by `Broadband Forum`

## Data Model

Many data models are already defined by `Broadband Forum`. Most of the time you will use these data model specifications to implement a certain part of the data model. Sometimes these pre-defined data models are not sufficient and don't cover your specific use case or functional area, in that case you can define your own data model.

Before we continue it is very important that you now what the major parts of a data model are:

- `data model` - is a hierarchical set of `objects`, `parameters`, `commands` and/or `events` that define the managed objects.
- `object` - an internal node in the name hierarchy, a node that can have `objects`, `parameters`, `commands` and/or `event` children. An `object` can be addressed by its `object path`. An `object` can have as parent another `object` or can have no parent at all, then it is a `root object`. 
- `parameter` - a name-value pair that represents part of a CPE’s configuration or status. A `parameter` can be addressed by its `parameter path`. A `parameter` is always a child of an `object`.
- `commands` - (aka `functions`) define `object` specific methods within the `data model`. `Commands` can be invoked from within the `data model` owners process or can be invoke by an external source (mostly another application or service).
- `events` - define `object` specific notifications within the `data model`. Subscriptions on events can be taken from within the `data model` owners process or taken by an external source (mostly another application or service).

## Object Definition Language

The Ambiorix framework provides a domain specific language that enables you to describe your data model in an easy way. This language is fully documented.

The documentation can be consulted:

- [Object Definition Language](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md)

The `Object Definition Language` parser is implemented in [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/tree/main).

This library also exposes an API that can be used to `load` and `save` ODL files and even extend the ODL parser. The API of this library will not be handled in this tutorial, in this tutorial we focus on using the `Object Definition Language`.

Typically the data model definitions are written in separate files, named odl files, and have as an extension `.odl`.

### Tools

Two tools are provided by the Ambiorix framework:

- `amxo-cg` - can be used to verify the syntax correctness of an odl file.
- `amxrt` - `ambiorix run time` - can be used to `launch and publish` a data model.

## Define A Data Model

To explain the basics of defining/writing a data model we will not use a real `Broadband Forum` defined data model. We will define our own custom data model, which we will call `Phonebook`.

A phone book is a `list` of contact information, where each `contact` has a name (first and last), one or more phone numbers and one or more e-mail addresses.

### General Syntax

To be able to read or write  `Object Definition Language` files, it is mandatory that you understand some of the `basic` and `general` notations.

There are two major syntaxes:

1. **Single lines**: a single line always ends with a semicolon ';'. The separation of words in a single line is always done with a `space character`, a new line is also interpreted as a space character, so it is possible to spread a single line over multiple lines.

Example:

```
on event "dm:object-changed" call enable_greeter
    filter 'parameters.State.from == "Idle" && 
            parameters.State.to == "Start"';
```

2. **With a body**: Almost all parts described in an odl file can have a body. A body always starts with an open brace `{` and ends with a closing `}`. In all cases a body may be empty.

Example :

```
%persistent %read-only string Message {
    on action validate call check_maximum_length 256;
}
```

### Sections

An ODL file can contain multiple sections, each section has a specific role. The possible sections are:

- configuration section - this section contains configuration options, which can be application or service specific.
- define section - this section contains the data model definition.
- populate section - this section can be used to set entry point functions, event callback functions or set parameter values or add instances to multi-instance objects.

All of these sections can be used zero, 1 or more times in one single ODL file.

Each section starts with a specific keyword and must have a body.

- configuration sections: `%config`
- define sections: `%define`
- populate sections `%populate`

For now we will only cover the `%define` section.

### Defining A Data Model Object

Defining an object is very easy and done using the keyword `object` followed by the object's name. Defining an object must be done in the `define` section.

```odl
%define {
    object Phonebook;
}
```

Using this notation a `singleton` object is defined.

### Adding Children

An object with only a name is not very useful. 

As a phone book contains zero, one or more contacts, we need to add another object called `Contact` which is a child object of `Phonebook`.

As we may have multiple contacts in our phone book defining a singleton object is not sufficient, we need a `multi-instance` object.

```odl
%define {
    object Phonebook {
        object Contact[];
    }
}
```

Adding a `child` object is easy, just define the object in the body of its parent. 

Using brackets `[` and `]` after the object name indicates that it is a multi-instance object. Optionally you can specify the maximum number of instances that is allowed to be created within the brackets.

Example:

```odl
%define {
    object Phonebook {
        object Contact[10];
    }
}
```

### Adding Parameters

The initial object hierarchy is defined, but still we can not add any data to the objects like a name. What use does a contact has without a name?

Parameters can be added to any object, and is done in the object body. Parameters are key-value pairs and have a specific type. The parameter types possible are fixed and must be one of:

- string, csv_string, ssv_string
- uint8, uint16, uint32, uint64
- int8, int16, int32, int64
- bool
- datetime

So let us add the first and last name to the `Contact` multi-instance object.

```odl
%define {
    object Phonebook {
        object Contact[] {
            string FirstName;
            string LastName;
        }
    }
}
```

### Naming Conventions

Object and parameter names must be compliant with this naming convention (from [TR-106 Data Model Template for CWMP Endpoints and USP
Agents - Chapter 3](https://www.broadband-forum.org/technical/download/TR-106_Amendment-8.pdf))

>The name of each node in the hierarchy MUST start with a letter or underscore, and subsequent characters MUST be letters, digits, underscores or hyphens. The terms “letter”
and “digit” are as defined in [Appendix B](https://www.w3.org/TR/REC-xml/#CharClasses) of the XML specification.

### ODL Keywords and Names

An object or parameter name can be anything, as long as it matches the naming convention. It is possible that a name of an object or parameter conflicts with one of the ODL keywords.

To solve this problem names of objects, parameters, functions, events may be enclosed between single or double quotes.

We could write our phone book example as follows:

```odl
%define {
    object "Phonebook" {
        object "Contact"[] {
            string "FirstName";
            string "LastName";
        }
    }
}
```

## Publish A Data Model

We defined a very simple data model, but till now it is not published.

To be able to publish a data model, we need an application that:

1. Opens and parses our odl file and builds the data model from it.
1. Connects to available software bus systems.
1. Implements an [event loop](https://en.wikipedia.org/wiki/Event_loop) and dispatches incoming requests and sends back replies.

It is possible to write such an application and do that again for another service over and over again. Most of the time this is `boiler plate code` and the same for most services and applications.

Luckily the Ambiorix framework provides a tool that is doing exactly what we need, this tool is [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt), the Ambiorix Run Time.

### Ambiorix Run Time

In the [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) tutorial is explained how to set-up a `debug and development environment` using a docker container. If you followed that tutorial you should have a running container with two software bus systems running `ubus` and `PCB`.

Let's use this container and publish the data model we have defined till now.

Before continuing make sure that this tutorial is `cloned` in your workspace. The workspace is a directory that is shared between your local host system and the container.

```bash
mkdir -p ~/workspace/ambiorix/tutorials/datamodels/server
cd ~/workspace/ambiorix/tutorials/datamodels/server/
git clone git@gitlab.com:prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish.git
```
And now open a terminal in your container if you don't have a terminal open yet.

```bash
docker exec -ti -u $USER amxdev /bin/bash
```

Make sure that `ubusd` and `pcb_sysbus` are running in your docker container.

Check if `ubusd` is running:

```bash
$ ps aux | grep ubusd
root      7680  0.0  0.0   2436   744 pts/3    S    09:01   0:00 ubusd
sah4009   7834  0.0  0.0   7796   880 pts/1    S+   09:01   0:00 grep ubusd
```

Check if `pcb_sysbus` is running:

```bash
$ ps aux | grep pcb_sysbus
root      7762  0.0  0.0   3676  2164 ?        Ss   09:01   0:00 pcb_sysbus
sah4009   7934  0.0  0.0   7928   824 pts/1    S+   09:01   0:00 grep pcb_sysbus
```

If they are not running, launch them as `root` user. You can switch to the container `root` by using:

```bash
sudo -E su -
export LD_LIBRARY_PATH=/usr/local/lib
```

The environment variable `LD_LIBRARY_PATH` is needed to make `ubus` work. When opening a terminal to the container as the root user this environment variable is made available as well.

```
docker exec -ti amxdev /bin/bash
```

How to start `ubusd` and `pcb_sysbus` can be found in the [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) tutorial.

---

>**NOTE**<br>
`ubusd` must be run as `root` user, otherwise it fails to start. Any application that wants to connect to the `ubusd` must also start as `root` and can drop privileges after connecting to `ubusd`. In this tutorial the privileges of the started applications are not dropped. They keep running under `root` privileges. 

---

If everything is up and running we can finally launch and publish our `phonebook` data model.

Switch to the root user, select the correct directory, and launch the data model as a daemon process:

---

>**Note**<br>
>In following commands replace \<USER\> with your user name. 

---

```bash
sudo -E su -
export LD_LIBRARY_PATH=/usr/local/lib
cd  /home/<USER>/workspace/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1
amxrt -D ./phonebook.odl
```

Using the option `-D` amxrt will daemonize itself and continue to run in background. If you want that the `phonebook` application is running in foreground, just remove the `-D` option.
When running the appllication in foreground you will need to open another terminal to your docker container before continuing.

When the application is running in background (daemonized) it can be stopped by using:

```bash
$ killall amxrt
```

For now just keep it running.

You can check if it is running:

```bash
$ ps aux | grep amxrt
root     14176  0.0  0.0   6744  2920 ?        Ss   09:13   0:00 amxrt -D phonebook.odl
root     15619  0.0  0.0   6072   876 pts/1    S+   09:15   0:00 grep amxrt
```

### Interact Using ubus

Using the tool `ubus` it is possible to send messages and queries to our `phonebook` applications. The tool `ubus` is part of the `ubus` eco-system and not a part of Ambiorix framework.

---

>**NOTE**
>
>The `ubus` command must always be run as root user.
>
>Before running the example commands switch to the container `root` user:
>
>```bash
>sudo -E su -
>export LD_LIBRARY_PATH=/usr/local/lib
>```

---

#### List The Objects

All `objects` registered to `ubusd` can be listed:

```bash
$ ubus list
Phonebook
Phonebook.Contact
```

#### Data Model Interaction

Although we did not define any functionality in our data model, the Ambiorix framework provides functions that can be used to interact with the data model.

The basic functions are:

- _get
- _set
- _add
- _del

> Note: These functions are prefixed with an underscore to prevent conflicts with other data model implementations that already provide these functions.

#### Add A Contact

Using the `ubus` tool you can invoke the `default` data model methods:

```bash
$ ubus call Phonebook.Contact _add
{
        "object": "Phonebook.Contact.1.",
        "index": 1,
        "name": "1",
        "parameters": {

        },
        "path": "Phonebook.Contact.1."
}
```

After adding a contact you can `list` the available objects again:

```bash
$ ubus list
Phonebook
Phonebook.Contact
Phonebook.Contact.1
```

As you can see a new object has appeared `Phonebook.Contact.1` 

#### Getting And Setting Parameters

Use the data model function `_get` to fetch all parameters of an object.

```bash
$ ubus call Phonebook.Contact.1 _get
{
        "Phonebook.Contact.1.": {
                "LastName": "",
                "FirstName": ""
        }
}
```

As you can see the `LastName` and `FirstName` parameters are available in this instance object, but the values are empty. Using the `_set` method the values of these parameters in the instance object can be changed.

This time we need to pass a function argument, which can be done using the `ubus` tool. This tool expects that arguments are written in JSON format.

```bash
ubus call Phonebook.Contact.1 _set '{ "parameters":{ "FirstName":"John", "LastName":"Doe" } }'
{
        "Phonebook.Contact.1.": {
                "LastName": "Doe",
                "FirstName": "John"
        }
}
```

And verify again with the `_get` method:

```bash
ubus call Phonebook.Contact.1 _get
{
        "Phonebook.Contact.1.": {
                "LastName": "Doe",
                "FirstName": "John"
        }
}
```

#### Deleting An Instance Object

Our `John Doe` contact can be deleted as well. Using the method `_del`.

```bash
$ ubus call Phonebook.Contact.1 _del
{
        "retval": [
                "Phonebook.Contact.1."
        ]
}
```

Check with `ubus list` if the instance object is still available or not:

```bash
$ ubus list
Phonebook
Phonebook.Contact
```

#### More advanced functions

Besides a `_get`, `_set`, `_add` and `_delete` Ambiorix also provides some more advanced functions. One of these functions is the `_list` function. Just like the other functions, you can call it using ubus.

```bash
$ ubus call Phonebook _list
{
	"objects": [
		"Contact"
	],
	"functions": [
		"_list",
		"_describe",
		"_get",
		"_get_supported",
		"_set",
		"_add",
		"_del",
		"_exec"
	],
	"parameters": [
		
	]
}
```

The most important thing to note is that this is not the same as a `ubus list`. Ambiorix data models contain more information than can be shown with a `ubus list` command. The Ambiorix `_list` command can be used to show all sub-objects, functions and parameters an object has. We won't go into detail about the other functions, but you can use the `_list`, `_describe` and `_get_supported` functions for introspection purposes.

### Interact Using PCB

Using the tool `pcb_cli` it is possible to send messages and queries to our `phonebook` applications. The tool `pcb_cli` is part of the `PCB` eco-system and not a part of Ambiorix framework.

`pcb_cli` can be started in an interactive mode. 

```bash
$ pcb_cli
Copyright (c) 2010 SoftAtHome

Connected to pcb://ipc:{/var/run/pcb_sys}
>
```

Unlike the `ubus` tool it can be run using your normal user id. 

The example commands must all be entered at the `pcb_cli` interactive prompt. 

#### List The Objects

All `root objects` registered to `pcb_sysbus` can be listed:

```bash
> ls

Process
Debug
Bus
Phonebook
```

You can query the full `phonebook` data model using the `?` query:

```bash
> Phonebook?
Phonebook
Phonebook.Contact
```

#### Data Model Interaction

As `PCB` has some understanding of data models it already provides implementations of some of the available operations. These operations can be used directly using `pcb_cli` and are represented with a single symbol.

The basic functions are:

- _get - '?'
- _set - '='
- _add - '+'
- _del - '-'

#### Add A Contact

Adding a contact using `pcb_cli` can be done as follows:

```bash
> Phonebook.Contact.+
Phonebook.Contact.2
Phonebook.Contact.2.LastName=
Phonebook.Contact.2.FirstName=
```

After adding a contact you can `_get` the available objects again:

```bash
> Phonebook.?
Phonebook
Phonebook.Contact
Phonebook.Contact.2
Phonebook.Contact.2.LastName=
Phonebook.Contact.2.FirstName=
```

As you can see a new object has appeared `Phonebook.Contact.2` 

It is also possible to create a new instance and provide all or some of the values of the parameters:

```bash
> Phonebook.Contact.+{LastName:"Smiths", FirstName:"Michael"}
```

#### Setting Parameters

The values of the individual parameters can be set using:

```bash
> Phonebook.Contact.2.LastName="Doe"
Phonebook.Contact.2.LastName=Doe
```

```bash
> Phonebook.Contact.2.FirstName="Jane"
Phonebook.Contact.2.FirstName=Jane
```

And verify again with the `_get` method using `?` in `pcb_cli`:

```bash
> Phonebook.?
Phonebook
Phonebook.Contact
Phonebook.Contact.2
Phonebook.Contact.2.LastName=Doe
Phonebook.Contact.2.FirstName=Jane
```

--- 

>**NOTE**
>
>When using pcb_cli to set values, make sure you put quotes arround your value if it is a string. The tool `pcb_cli` tries to interprete the values as well.
>
>Example:
>
>```bash
> > Phonebook.Contact.2.LastName=0xFF
>```
>
>Will result in the name "255" as pcb_cli thinks you are providing a hexadecimal number.

---
#### Deleting An Instance Object

Our `Jane Doe` contact can be deleted as well. Using the method `_del`. Deleting an instance object can be done in `pcb_cli` by using the `-` symbol behind the instance path. 

```bash
> Phonebook.Contact.2.-
```

Check with `?` if the instance object is still available or not:

```bash
> Phonebook.?
Phonebook
Phonebook.Contact
```

> Note: It is also possible to call the `_get` function on an object with `Phonebook._get()`. This gives slightly different behavior to calling the `_get` with `?`. First of all, using `?` will invoke the get recursively with a maximum depth parameter. If you use the `_get()` function, you have to provide this information yourself e.g. `Phonebook._get(depth: 3)` for a depth of 3 levels. Secondly the output format from the `_get()` function will be different than when you use `?`, but the information in both outputs will be the same. It is recommended to use `?` for a get, but now you know how any object function can be called using `pcb_cli`.

## Practical Labs

Before you start these labs, make sure you have the `Ambiorix Debug & Development` container installed and are able to open terminal in this container. If you haven't the container running with all Ambiorix libraries installed, please read [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) before continuing. 

All commands provided in these labs must be executed in a container terminal.

To open a terminal in the container use following command

```bash
docker exec -ti -u $USER amxdev /bin/bash
```

---

>Note: <br>
You can open multiple terminals to the same container. 

---

Before you continue, you need to clone this git repository.<br>
You can do this using a terminal on your local machine or if you configured the container correctly you can do the clone from a terminal in your container.

Execute these shell commands in a container terminal (or a terminal on your host machine)

```bash
mkdir -p ~/workspace/ambiorix/tutorials/datamodels/server
cd ~/workspace/ambiorix/tutorials/datamodels/server/
git clone git@gitlab.com:prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish.git
```

### Lab1 - Complete `Phonebook` Data Model

The current `phonebook` data model definition makes it possible to add multiple-contacts and set the parameters `FirstName` and `LastName` for each of those contacts.

Still missing are the phone numbers and e-mail addresses.

It is up to you to add this. Remember it must be possible to add zero, one or more phone number and e-mail addresses.

To help you get started, a more detailed description of the requirements:

- add a definition of a multi-instance object "PhoneNumber" with at least one parameter called "Phone". This new multi-instance object must be a child object of the "Contact" multi-instance object.
- add a definition of a multi-instance object "E-Mail" with at least one parameter called "E-Mail". This new multi-instance object must be a child object of the "Contact" multi-instance object.

Open the already provided odl file with your preferred editor and add the needed object definitions and parameters.

You can find the odl file at:
```bash
cd ~/workspace/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1
```

When the file is edited and you added everything you can use `amxo-cg` to verify that the syntax is correct:

```bash
$ amxo-cg -v labs/lab1/phonebook.odl 
Current parser configuration:
{
    auto-resolver-order = [
        "ftab",
        "import",
        "*"
    ],
    import-dbg = false,
    import-dirs = [
        "."
    ],
    import-pcb-compat = false
    include-dirs = [
        "."
    ],
    verbose = true,
}
Start - <unknown>
|   Open section define - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@1
|   |   Create object singleton Phonebook - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@2
|   |   |   Create object template Contact - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@3
|   |   |   |   Add parameter cstring_t FirstName - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@4
|   |   |   |   Add parameter cstring_t LastName - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@5
|   |   |   |   Create object template PhoneNumber - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@7
|   |   |   |   |   Add parameter cstring_t Phone - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@8
|   |   |   |   Done object PhoneNumber - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@9
|   |   |   |   Create object template E-Mail - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@11
|   |   |   |   |   Add parameter cstring_t E-Mail - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@12
|   |   |   |   Done object E-Mail - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@13
|   |   |   Done object Contact - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@14
|   |   Done object Phonebook - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@15
|   Close section define - /home/sah4009/amx/ambiorix/tutorials/datamodels/server/define-publish/labs/lab1/phonebook.odl@16
End - (null)
```

---

>**NOTE**
>The tool `amxo-cg` prints to the stdout the current used `parser configuration`. The ODL parser (implemented in libamxo) uses run-time configuration options to known where to search files, what must be printed and so on.
>
> In another tutorial [Tools - amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/tools/amxrt) more details about the `parser configuration` is handled. It is recommended to keep following the turorials in the order specified in the [getting started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started) tutorial.
>
> If you want to learn more about the available `parser configuration` options you can also consult the [odl syntax](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md#default-configuration-settings) documentation.
---

Launch the updated odl file and add one or more contact each with one ore more phone number and/or e-mail address.

Use one of the tools `ubus` or `pcb_cli` to interact with the data model.

When added instances and set values for the parameters the data model could look like (using `pcb_cli`)

```bash
> Phonebook.?
Phonebook
Phonebook.Contact
Phonebook.Contact.1
Phonebook.Contact.1.LastName=Doe
Phonebook.Contact.1.FirstName=John
Phonebook.Contact.1.PhoneNumber
Phonebook.Contact.1.PhoneNumber.1
Phonebook.Contact.1.PhoneNumber.1.Phone=(+032)555.89.12
Phonebook.Contact.1.PhoneNumber.2
Phonebook.Contact.1.PhoneNumber.2.Phone=(+32)555.89.14
Phonebook.Contact.1.E-Mail
Phonebook.Contact.1.E-Mail.1
Phonebook.Contact.1.E-Mail.1.E-Mail=john.d@ambiorix.com
Phonebook.Contact.2
Phonebook.Contact.2.LastName=Doe
Phonebook.Contact.2.FirstName=Jane
Phonebook.Contact.2.PhoneNumber
Phonebook.Contact.2.PhoneNumber.1
Phonebook.Contact.2.PhoneNumber.1.Phone=(+032)555.77.54
Phonebook.Contact.2.E-Mail
Phonebook.Contact.2.E-Mail.1
Phonebook.Contact.2.E-Mail.1.E-Mail=jane.d@ambiorix.com
```

### Lab2 - TR-181 EthernetLink:1 Profile

Using the `Object Definition Language` it is possible to describe `standard` data models, as described in one of the `Broadband Forum` data model documents.

The `standard` data model definitions are extensive, but it is not always necessary or mandatory to describe the full data models. Most of the data models define `profiles` which are sub-sets of the full data model. One of these profiles is `EthernetLink:1` profile which can be found in [TR-181 2.12.0 USP](https://usp-data-models.broadband-forum.org/tr-181-2-12-0-usp.html). 

It is up to you to write an odl file that describes this profile. For now it is only needed to describe all `objects` and `parameters` that are mentioned in this profile. So no need to take into account any of the descriptions or constrains as described in that data model.

How to `implement` or describe these constraints will be handled in another tutorial.

To get you started an odl file is available at:

```bash
cd ~/workspace/ambiorix/tutorials/datamodels/server/define-publish/labs/lab2
```

Note that the object `Device` is not added to the data model definition.

When you finished writing the odl file, launch it using `amxrt` and add a `Ethernet.Link.` instance. You can set some values of the parameters as well. Use one of the tools `ubus` or `pcb_cli` to modify the content of the data model.

After adding one link and not setting any values the data model should look like:

(dump taken with `pcb_cli`)

```bash
> Ethernet.?
Ethernet
Ethernet.LinkNumberOfEntries=0
Ethernet.Link
Ethernet.Link.1
Ethernet.Link.1.LastChange=0
Ethernet.Link.1.Enable=0
Ethernet.Link.1.Status=
Ethernet.Link.1.Alias=
Ethernet.Link.1.LowerLayers=
Ethernet.Link.1.MACAddress=
Ethernet.Link.1.Stats
Ethernet.Link.1.Stats.MulticastPacketsSent=0
Ethernet.Link.1.Stats.ErrorsSent=0
Ethernet.Link.1.Stats.BroadcastPacketsSent=0
Ethernet.Link.1.Stats.BytesSent=0
Ethernet.Link.1.Stats.PacketsSent=0
Ethernet.Link.1.Stats.BytesReceived=0
Ethernet.Link.1.Stats.DiscardPacketsReceived=0
Ethernet.Link.1.Stats.ErrorsReceived=0
Ethernet.Link.1.Stats.MulticastPacketsReceived=0
Ethernet.Link.1.Stats.UnknownProtoPacketsReceived=0
Ethernet.Link.1.Stats.UnicastPacketsSent=0
Ethernet.Link.1.Stats.UnicastPacketsReceived=0
Ethernet.Link.1.Stats.PacketsReceived=0
Ethernet.Link.1.Stats.DiscardPacketsSent=0
Ethernet.Link.1.Stats.BroadcastPacketsReceived=0
```

## Conclusion

After finishing this tutorial you have learned:

- How to describe a data model using the `Object Definition Language`
- How to launch a data model using `amxrt`
- How to interact with the data model using `native` tools like `ubus` and `pcb_cli`

## References

- Getting Started<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started
- Set-up debug & development environment<br>
https://gitlab.com/soft.at.home/docker/oss-dbg/-/blob/main/README.md
- Object Definition Language<br>
https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md
- libamxo - the odl parser git repository<br>
https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo
- libamxo - API documentation<br>
https://soft.at.home.gitlab.io/ambiorix/libraries/libamxo/doxygen
- Ambiorix Run Time (amxrt) git repository<br>
https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt
- Broadband Forum<br>
https://www.broadband-forum.org/
- Broadband Forum TR-181 USP Data Model<br>
https://usp-data-models.broadband-forum.org/tr-181-2-12-0-usp.html
- Broadband Forum TR-369 USP<br>
https://www.broadband-forum.org/download/TR-369.pdf
- Broadband Forum TR-106<br>
https://www.broadband-forum.org/technical/download/TR-106_Amendment-8.pdf
- Extensible Markup Language (XML) 1.0<br>
https://www.w3.org/TR/REC-xml/ 
- Wikipedia Event Loop<br>
https://en.wikipedia.org/wiki/Event_loop
