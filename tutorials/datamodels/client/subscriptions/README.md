# Ambiorix Data Models - Subscriptions

Estimated Time Needed: 45 - 60 minutes.

[[_TOC_]]

## Introduction

When using a modular system, different processes can provide their own data model. However one process might be interested in data model changes of another process. This information can be obtained by sending out events and subscribing to these events. 

## Goal

The goal of this tutorial is to explain:

- Which default events exist in the Ambiorix framework.
- How to subscribe to events from other data models.

## Prerequisites

- You finished the [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) tutorial
- You finished the [Data Model Client  - Simple Client](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/client/simple-client/-/blob/main/README.md) tutorial
- You have a basic understanding of the Ambiorix variants

## Different types of events

The Ambiorix data model already provides a set of default events. These default events are automatically sent when the data model is updated using a `transaction`. Transactions will be discussed in a different tutorial, but you can see a transaction as a group of multiple actions that are executed at once. If one of these actions fails, nothing should be changed. The full documenation on transactions can be found in [Actions, Transactions & Events](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd/-/blob/main/doc/actions_transactions_events.md).

The default events are explained in the [ODL documentation](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md#define-events), but are repeated here for clarity:

- `dm:root-added` - send when a new `root` object has been added
- `dm:root-removed` - send when a `root` object has been deleted
- `dm:object-added` - send when an object has been added in the hierarchical tree
- `dm:object-removed` - send when an object has been removed from the hierarchical tree
- `dm:instance-added` - send when an instance has been created
- `dm:instance-removed` - send when an instance has been deleted
- `dm:object-changed` - send when an object has been changed
- `dm:periodic-inform` - send at a regular interval, only when periodic-inform has been enabled
- `app:start` - The application should send this event when the data model is loaded
- `app:stop` - The application should send this event when the data model is going to be removed.

Besides these events, it is also possible to create object-defined events. These events must be defined in the `object definition body`.

### ODL event syntax

>Syntax:
>
>```odl
>event "<name>";
>```
A concrete example of such an event would be a [Device.LocalAgent.Threshold.{i}.Triggered!](https://usp-data-models.broadband-forum.org/tr-181-2-13-0-usp.html#D.Device:2.Device.LocalAgent.Threshold.{i}.Triggered!) event. This event must be send out when a focused parameter reached a given value.

Sending out an object-defined event can be done by using
```
void amxd_object_emit_signal(amxd_object_t* const object, const char* name, amxc_var_t* const data);
```

This is considered as an advanced feature and will not be discussed in this tutorial. If you are interested in this functionality, you can have a look at the [cpu-info example](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/cpu-info). Here we will focus on the default events instead.

## What does an ambiorix event looks like

When an ambiorix event is triggered, a variant is sent out containing the event data. Depending on the back-end you are using, this variant can be converted to a back-end specific message before it is send out over the bus. At the receiving side, this message is converted back to a variant.
The nice thing about this system is that you don't have to care about how the data is transported. You just want to be able to receive events.

An event variant is a hash table of data. Depending on which event was triggered, different information can be available in the table, but it will always contain at least 3 entries:

- The `notification` type, for example `dm:object-added`
- The `object` the notification belongs to, for example `Phonebook.Contact.contact-1.PhoneNumber.`
- The `path` to the object, for example `Phonebook.Contact.1.PhoneNumber.`

The difference between the `object` and `path` is that an `object` will contain the instance name (Alias) if it has one, while this Alias is replaced by an index for the `path`.
In case of the Phonebook example, there is no parameter Alias, so the `object` name will be the same as the `path`: `Phonebook.Contact.1.Phonenumber.`.

Here you can see the corresponding variant:

```
{
    notification = "dm:object-added",
    object = "Phonebook.Contact.1.PhoneNumber.",
    path = "Phonebook.Contact.1.PhoneNumber."
}
```

Depending on which event was sent, other information can also be present in the event variant. For example when adding an instance of `Contact` to the `Phonebook` object, you will get the following event:

```
{
    index = 1,
    keys = {
    },
    name = "1",
    notification = "dm:instance-added",
    object = "Phonebook.Contact.",
    parameters = {
        FirstName = ""
        LastName = "",
    },
    path = "Phonebook.Contact."
}
```

Besides the `notification`, `object` and `path`, the event also has entries for the instance `index`, its `parameters` at creation time and its `keys`. The `key parameters` of an instance uniquely identify that instance. In other words, you can never have 2 instances with the same key parameters. More on this is explained in the odl [documentation](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md) and in the tutorial [Object And Parameter Validation](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/validation).

## Ambiorix subscription functions

Subscribing to events and unsubscribing from events from an object can be done with

```
int amxb_subscribe(amxb_bus_ctx_t* const ctx,
                   const char* object,
                   const char* expression,
                   amxp_slot_fn_t slot_cb,
                   void* priv);

int amxb_unsubscribe(amxb_bus_ctx_t* const ctx,
                     const char* object,
                     amxp_slot_fn_t slot_cb,
                     void* priv);
```

How it works is explained in the [documentation](https://soft.at.home.gitlab.io/ambiorix/libraries/libamxb/master/dc/d54/a00094.html#ga9ad6c052f5f2494b73c5d8f296729ef4).
The callback function that is called when an event is received has the following prototype:

```
typedef void(* amxp_slot_fn_t) (const char *const sig_name, const amxc_var_t *const data, void *const priv)
```

Its arguments are also explained in the [documentation](https://soft.at.home.gitlab.io/ambiorix/libraries/libamxp/master/d2/d79/a00051.html#ga26f72e3fe02d7ba95264f44c9b7d3c6f). The passed private data is the same data that was provided when subscribing.

The passed `expression` when subscribing can be used to filter out certain events. A few examples are given here to show which kinds of expressions are valid.
Suppose you are subscribing to events from the `Phonebook.Contact.` object. By default you will receive all events that are sent out from this object. If you are only interested in events when `Contact` objects are added or removed, you could apply a filter such as:

```
const char* expression = "notification in ['dm:instance-added','dm:instance-removed']"
```

This will filter out all events with a different notification. If you are only interested in notifications when a certain parameter is set, you could add this to the expression as well:

```
const char* expression = "notification in ['dm:instance-added','dm:instance-removed'] && parameters.FirstName == 'Ella'"
```

Now you will only receive events if an instance is added or removed and the parameter `FirstName` is equal to `Ella`. It is also possible to expand this search path with wildcards. Suppose you want to be notified of all additions or removals where the `FirstName` starts with `Ell`, then you can use the `matches` keyword and as a value pass a [posix extended regular expression](https://www.gnu.org/software/findutils/manual/html_node/find_html/posix_002dextended-regular-expression-syntax.html):

```
const char* expression = "notification in ['dm:instance-added','dm:instance-removed'] && parameters.FirstName matches 'Ell\.*'"
```

Now you will also get events when a `Contact` named `Elliot` is added.

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
mkdir -p ~/workspace/ambiorix/tutorials/datamodels/client
cd ~/workspace/ambiorix/tutorials/datamodels/client/
git clone git@gitlab.com:prpl-foundation/components/ambiorix/tutorials/datamodels/client/subscriptions.git
```

### Lab1 - Interactive subscribe example

There already exists an example subscription client for Ambiorix and we will use it in this lab. This example can be found [here](https://gitlab.com/prpl-foundation/components/ambiorix/examples/baapi/subscribe).

It explains how to subscribe to events using a command line application. The README file explains how to subscribe to events from the greeter plugin using ubus. In this lab we will subscribe to events from our Phonebook applications using PCB.

---

> Note: you can also make this lab using ubus. Which ubus commands to execute are not listed here, but the output you receive from amx-subscribe should be the same.

---

Let's create a directory for the example and clone it there.

---
>**NOTE**
>
>If you followed the [Getting Started]() tutorial, all examples should already be available in your workspace directory, if it is already available, you can skip cloning the repository.

---

```bash
mkdir -p ~/workspace/ambiorix/examples/baapi
cd ~/workspace/ambiorix/examples/baapi
git clone git@gitlab.com:prpl-foundation/components/ambiorix/examples/baapi/subscribe.git
```

You can build the applications using

```bash
cd ~/workspace/ambiorix/examples/baapi/subscribe
make
sudo -E make install
```

The application uses `AMXB_BACKEND` as an environment variable to determine the back-end that will be used. Since we are using PCB in this lab, we will set it to

```bash
export AMXB_BACKEND=/usr/bin/mods/amxb/mod-amxb-pcb.so
```

Before continuing, switch to the `lab1` folder and launch the `phonebook.odl` using amxrt

```bash
cd ~/workspace/ambiorix/tutorials/datamodels/client/subscriptions/labs/lab1
amxrt -D phonebook.odl
```

Now you can subscribe to events using the subscribe application:

```bash
amx-subscribe <URI> <OBJECT> [<EXPRESSION>]
```

- URI - the uri to connect to, for pcb this is normally pcb:/var/run/pcb_sys
- OBJECT - the object path of the `data model` object of which you want to receive events.
- EXPRESSION - (optional) an event filter expression.

We will listen to events from the Phonebook object

```bash
$ amx-subscribe pcb:/var/run/pcb_sys Phonebook. 
```

When launched nothing will happen, the `amx-subscribe` application is now waiting for events. All you need to do is make sure the `phonebook` is generating events on the object `Phonebook.`

So open a new console and run `pcb_cli`. First check that the `Phonebook.` object can be found

```bash
> Phonebook.?
Phonebook
Phonebook.Contact
```

Then add a new Contact

```bash
> Phonebook.Contact.+
Phonebook.Contact.1
Phonebook.Contact.1.LastName=
Phonebook.Contact.1.FirstName=
```

Now you should see incoming events in the terminal where you subscribed to events.

```
Notification received [Phonebook]:
{
    index = 1,
    keys = {
    },
    name = "1",
    notification = "dm:instance-added",
    object = "Phonebook.Contact.",
    parameters = {
        FirstName = ""
        LastName = "",
    },
    path = "Phonebook.Contact."
}
Notification received [Phonebook]:
{
    notification = "dm:object-added",
    object = "Phonebook.Contact.1.PhoneNumber.",
    path = "Phonebook.Contact.1.PhoneNumber."
}
Notification received [Phonebook]:
{
    notification = "dm:object-added",
    object = "Phonebook.Contact.1.E-Mail.",
    path = "Phonebook.Contact.1.E-Mail."
}
```

The `amx-subscribe` that is running, will print all events from all objects under the `Phonebook` object. Using the `expression` feature of the `Ambiorix` framework , it is possible to easily filter out the events you want to see.

Now stop the `amx-subscribe` (press CTRL+C in the console where it is running) application and relaunch it:

```bash
amx-subscribe pcb:/var/run/pcb_sys Phonebook.Contact. "notification in ['dm:instance-added','dm:instance-removed'] && parameters.FirstName == 'Ella'"
```

Add a new contact named `John`
```bash
> Phonebook.Contact.+{FirstName="John"}
Phonebook.Contact.2.FirstName=John
```

This should not generate any event. Add a new contact named `Ella`
```bash
> Phonebook.Contact.+{FirstName="Ella"}
Phonebook.Contact.3.FirstName=Ella
```

Now you will get an event because the subscribe expression matches the event

```
Notification received [Phonebook.Contact]:
{
    index = 3,
    keys = {
    },
    name = "3",
    notification = "dm:instance-added",
    object = "Phonebook.Contact.",
    parameters = {
        FirstName = "Ella"
        LastName = "",
    },
    path = "Phonebook.Contact."
}
```

Similarly if you remove both contacts, you will only receive an event for deleting the contact named `Ella`.

```
> Phonebook.Contact.2.-
> Phonebook.Contact.3.-
```
```
Notification received [Phonebook.Contact]:
{
    index = 3,
    keys = {
    },
    name = "3",
    notification = "dm:instance-removed",
    object = "Phonebook.Contact.",
    parameters = {
        FirstName = "Ella"
        LastName = "",
    },
    path = "Phonebook.Contact."
}
```

### Lab2 - Subscribing to events in C code

In this lab we will again launch the phonebook ODL from lab1 using amxrt and subscribe to its events from a
simple plugin. Some minimal C code and a small ODL file have been prepared for you in the lab2
folder. The ODL file only contains the name of your plugin and an entry point to start running the
code. All other config options are set to their default values using `amxrt`. These will not be
discussed in detail here, but they are printed when running the application.

Before starting the application one environment variable must be defined.

If you want to use `PCB bus system`

```bash
export AMXB_URI=pcb:/var/run/pcb_sys
```

If you want to use `ubus bus system`:

```bash
export AMXB_URI=ubus:/var/run/ubus/ubus.sock
```

Your goal is to subscribe to events from the `Phonebook` datamodel using `amxb_subscribe`. Use an
expression filter to only subscribe to events where a `Contact` is added with the parameter
`FirstName` set to `Elliott`. Add a callback function that is called when an event is received. It
should dump the event variant contents to stdout.

You can build and run your code from the lab2 folder using:

```bash
cd ~/workspace/ambiorix/tutorials/datamodels/client/subscriptions/labs/lab2
make
make install

amxrt subscriptions.odl
```

Make sure you are running the phonebook.odl file from lab1 in a different terminal or in the background

```bash
cd ~/workspace/ambiorix/tutorials/datamodels/client/subscriptions/labs/lab1
amxrt -D phonebook.odl
```

Depending on your choice of `AMXB_URI` previously, you can use either pcb or ubus to add and
delete object instances.

## Conclusion

After finishing this tutorial you have learned:

- Which default events exist in the Ambiorix framework.
- How to subscribe to events from other data models.

## References

- Getting Started<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started
- Data Model Client  - Simple Client<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/client/simple-client/-/blob/main/README.md
- Set-up debug & development environment<br>
https://gitlab.com/prpl-foundation/components/ambiorix/dockers/amx-sah-dbg
- Actions, Transactions & Events<br>
https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd/-/blob/main/doc/actions_transactions_events.md
- Object Definition Language<br>
https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md
- libamxo - the odl parser git repository<br>
https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo
- libamxo - API documentation<br>
https://soft.at.home.gitlab.io/ambiorix/libraries/libamxo/doxygen
- CPU info example<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/cpu-info
- Object And Parameter Validation<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/validation
- Ambiorix Run Time (amxrt) git repository<br>
https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt
- BAAPI subscribe example application<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/baapi/subscribe
- Broadband Forum<br>
https://www.broadband-forum.org/
- Broadband Forum TR-181 USP Data Model<br>
https://usp-data-models.broadband-forum.org/tr-181-2-12-0-usp.html
- Broadband Forum TR-369 USP<br>
https://www.broadband-forum.org/download/TR-369.pdf
- Broadband Forum TR-106<br>
https://www.broadband-forum.org/technical/download/TR-106_Amendment-8.pdf
- POSIX extended regular expression syntax<br>
https://www.gnu.org/software/findutils/manual/html_node/find_html/posix_002dextended-regular-expression-syntax.html
