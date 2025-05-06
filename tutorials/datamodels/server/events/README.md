# Ambiorix Data Models - Events

[[_TOC_]]

## Introduction

The Ambiorix framework is designed to work in single-threaded event driven applications. Events and event handling play a central and important part in these applications. Events (or notifications) provide updates and state changes, where your application or service can act on.

This tutorial will focus on the data model server side, an other tutorial is handling how clients can subscribe for events on data models from other application.

## Goal

The goal of this tutorial is to explain:

- How to act on changes in your data model.
- How to create and send custom events.

## Prerequisites

- You finished the [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) tutorial
- You finished the [Define And Publish](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish/-/blob/main/README.md) tutorial
- You have a basic understanding of the data model concepts as defined by `Broadband Forum` standards
- You have a basic understanding of the Ambiorix variants
- You have basic C knowledge
- You have basic git knowledge
- You can work with docker containers

## Events

The Ambiorix data model library already provides a set of events that can be used and are related to data models. A sub-set of these "default" data model events are also define in the `Broadband Forum`'s [TR-369 USP](https://www.broadband-forum.org/download/TR-369.pdf) specifications chapter 13.6.3

- `ValueChange` 
- `ObjectCreation`
- `ObjectDeletion`

When using the data model transactions API, events will be automatically created and send. When manipulating your data model with other data model API's you have to create and send the events manually using the data model event API.

To be able to handle events in your application, your application must have an [event loop](https://en.wikipedia.org/wiki/Event_loop). When adapting an existing application you will need to integrate the Ambiorix event dispatching functionality in the application. For more information about this you can check the [Greeter Application](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_app) example.

### Default Data Model Events

The Ambiorix data model library already implements a set of "default" data model events. When using data model transactions, the events will be emitted when the transaction is applied with success. Read the [Actions, Transactions and Events](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd/-/blob/main/doc/actions_transactions_events.md) document to get more information about the transaction APIs.

The default data model events are:

- `dm:root-added` - send when a new `root` object has been added
- `dm:root-removed` - send when a `root` object has been deleted
- `dm:object-added` - send when an object has been added in the hierarchical tree
- `dm:object-removed` - send when an object has been removed from the hierarchical tree
- `dm:instance-added` - send when an instance has been created
- `dm:instance-removed` - send when an instance has been deleted
- `dm:object-changed` - send when an object has been changed
- `dm:periodic-inform` - send at a regular interval, only when periodic-inform has been enabled

All events always have an event name and event data. The event data is a `amxc variant` pointer or NULL.

The event data variant is always a variant containing a hash table and contains at least:

- The `object` the notification belongs to, for example `Phonebook.Contact.contact-1.PhoneNumber.`
- The `path` to the object, for example `Phonebook.Contact.1.PhoneNumber.`

Depending on the event other data can be available as well.

Note that client applications that subscribe for events on your data model will also see a `notification` field in the event data. 

### Add Event Callbacks

There are different methods available to install an event callback function. That is a function that will be called whenever a certain event `occurs` that matches the specified criteria or `filter`.

These methods are:

- Using `Object Definition Language` `%populate` section.
- Using the [signal/slot](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp/-/blob/main/doc/signal_slot.md) API directly from within your implementation.

The first method is a good choice when a specific event (or set of events) must always be handled during the lifetime of the application/service. The main disadvantage when using this method is that you are not able to remove the event handler at any point during the lifetime of your application. Often this is not a real problem as you want to keep certain events during the full lifetime.

Using the [signal/slot](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp/-/blob/main/doc/signal_slot.md) API is a good choice when some events must be handled during a limited time.
#### Using Object Definition Language

In the [define and publish](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish) tutorial was mentioned that odl files can contain multiple sections:

- configuration section - this section contains configuration options, which can be application or service specific.
- define section - this section contains the data model definition.
- populate section - this section can be used to set entry point functions, event callback functions or set parameter values or add instances to multi-instance objects.

Until now, most of the time the `%define` section was used to define a data model. Event callback functions can be set in the odl file as well, but must be done in the `%populate` section.

The general odl syntax for setting an event callback function are:

```odl
on event "<EVENT NAME>" call <FUNCTION NAME> [filter "<EXPRESSION>"];
on event "<EVENT NAME>" of "<OBJECT PATH>" call <FUNCTION_NAME>;
```

With the first format it is possible to provide an expression that will be evaluated using the event data, when the event data matches the filter, the callback function is called.

With the second format the events will be filtered on the given object path, only events from the specified object (sub-)tree will trigger the callback function.

The fields, `EVENT NAME`, `OBJECT PATH` can be replaced with a regular expression as well in both formats. To indicate it is a regular expression those fields must be put in the `regexp` function.

Using regular expressions:

```odl
on event regexp("<EVENT REGEXP>") call <FUNCTION NAME> [filter "<EXPRESSION>"];
on event regexp("<EVENT REGEXP>") of regexp("<OBJECT PATH REGEXP>") call <FUNCTION_NAME>;
```

Using regular expressions as event name or object path enables you to handle multiple events with one callback function.

---

***Recommendation***

Although it is possible to handle many types of events from multiple data model object with one single event callback function, it is recommended to create multiple functions each specialized in handling one single type of event or object. This will keep the code simpler and more understandable.

---

Examples of event callbacks in odl files:

From [greeter_plugin example](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_plugin/-/blob/main/odl/greeter_definition.odl#L48)

```odl
%populate {
    on event "dm:object-changed" call enable_greeter
        filter 'object == "Greeter." && 
                parameters.State.from == "Idle" && 
                parameters.State.to == "Start"';

    on event "dm:object-changed" call disable_greeter
        filter 'object == "Greeter." && 
                parameters.State.from == "Running" && 
                parameters.State.to == "Stop"';
}
```

---

**TIP**

During development and debugging it can become very handy to see the events. Using the odl file to add a callback function on `all` events and a simple function that prints the event and event data this can be achieved:

Print event callback function
```C
void _print_event(const char* const sig_name,
                  const amxc_var_t* const data,
                  UNUSED void* const priv) {
    printf("Signal received - %s\n", sig_name);
    printf("Signal data = \n");
    fflush(stdout);
    if(!amxc_var_is_null(data)) {
        amxc_var_dump(data, STDOUT_FILENO);
    }
}
```

Set `print_event` callback function on `all` events:

```odl
%populate {
    on event "*" call print_event;
}
```

---

When using '\*' as the signal name (event name) the slot (callback function) is connected to all known signals of the signal manager. Events (signals) that are added after connecting a callback function using '\*' as a signal name will not be connected to the callback function.

#### Using Code

Event callback functions can be set in code as well. Each data model structure (amxd_dm_t) contains a signal manager, using this signal manager event callback functions (called slots) can be registered on one or more events (signals). The data model implementation uses the generic implemented [observer pattern](https://en.wikipedia.org/wiki/Observer_pattern) [signal/slots](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp/-/blob/main/doc/signal_slot.md). This implementation is based on [QT's singal/slot](https://en.wikipedia.org/wiki/Signals_and_slots) implementation, but written in C without the need for a preprocessor.

Connecting event callback functions (slots) to events (signals) can be done using one of these API functions:

```C
int amxp_slot_connect(amxp_signal_mngr_t* const sig_mngr, const char* const sig_name, const char* const expression, amxp_slot_fn_t fn, void* const priv);
int amxp_slot_connect_filtered(amxp_signal_mngr_t* const sig_mngr, const char* const sig_reg_exp, const char* const expression, amxp_slot_fn_t fn, void* const priv);
int amxp_slot_connect_all(const char* const sig_reg_exp, const char* const expression, amxp_slot_fn_t fn, void* const priv);
```

Removing the event callback function (slot) from a signal can be done using any of these API functions:

```C
int amxp_slot_disconnect(amxp_signal_mngr_t* const sig_mngr, const char* const sig_name, amxp_slot_fn_t fn);
int amxp_slot_disconnect_with_priv(amxp_signal_mngr_t* sig_mngr, amxp_slot_fn_t fn, void* priv);
void amxp_slot_disconnect_all(amxp_slot_fn_t fn);
```

Documentation about these API functions can be found in [libamxp](https://soft.at.home.gitlab.io/ambiorix/libraries/libamxp/doxygen) API documentation.

##### Getting The Data Model Pointer

To be able to connect the callback functions (slots) to any of the data model events (signals), you need to get the data model's signal manager.
When implementing a stand-alone application you have to declare or allocate a data model structure by yourself and initialize it correctly. An example can be found in the [greeter application](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_app/-/blob/main/src/greeter_main.c#L152).

When using the [Ambiorix Runtime](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt), the data model structure is already created and initialized for you. To get the pointer to this `amxd_dm_t` structure the `Ambiorix Runtime` will call the entry-points defined in the odl file, after loading and parsing any odl file provided at command line.

An entry-point is a function that can be called by any application that loads an odl with entry-points defined in it, through the odl parser. 

An entry-point can be defined in any odl file in any `%define` section after `importing` the correct shared object:

Example from [greeter plugin](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_plugin/):

```odl
import "greeter.so" as "greeter";

%define {
    entry-point greeter.greeter_main;
}
```

General syntax:

```odl
entry-point <SHARED OBJECT|ALIAS>.<FUNCTION NAME>;
```

An entry-point function must match this prototype:

```C
int _<FUNCTION NAME>(int reason, amxd_dm_t* dm, amxo_parser_t* parser);
```

When called by an application (for example `amxrt`), three arguments are given:

- `reason` - a reason code, this can be any number and depends on the application implementation. When using `amxrt` only two reason codes exists (0 - Application start and 1 - Application stop).
- `dm` - a pointer to the `amxd_dm_t` structure, this is the pointer we need.
- `parser` - a pointer to the odl parser, also contains all configuration options as defined in the `%config` sections in the odl files

It is safe to store the pointer to the data model structure and the parser. Any application that calls your entry-point must guarantee that both pointers stay valid until the entry-point is called again with an other reason code.

When using `amxrt` (Ambiorix RunTime) it is safe to store the pointers for later use when the `reason` is 0 (Application start). When the entry-point is called with `reason` 1 (Application stop), the stored pointers are not valid any more and should be reset to `NULL`

Example of an entry-point implementation:

Taken from [greeter plugin](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_plugin/-/blob/main/src/greeter_main.c#L88)

```C
int _greeter_main(int reason,
                  amxd_dm_t* dm,
                  amxo_parser_t* parser) {

    switch(reason) {
    case 0:     // START
        app.dm = dm;
        app.parser = parser;
        break;
    case 1:     // STOP
        app.dm = NULL;
        app.parser = NULL;
        break;
    }

    return 0;
}
```

##### Connecting Events and Callback Functions

When you have the pointer to the data model structure `amxd_dm_t` it becomes easy to set an event callback function (slot) to any of the available events (signals).

Example:

```C
amxp_slot_connect(&dm->sigmngr, "dm:object-changed", NULL, _print_event, NULL);
```

When using the above line the event callback function `_print_event` is called each time a `dm:object-changed` event is triggered or emitted.

### Event Filters

Often you are not interested in all possible events, only in some specific events. When adding an event callback function on an event, it is possible to add a filter. The filter can be set using the `Object Definition Language` or using the API functions.

A filter is a logical expression, which will be evaluated on the event data and always evaluates to `true` or `false`.

An expression can consist out of multiple expression parts, concatenated with a logical `OR` or logical `AND`.

Syntax of an expression part

```text
<VALUE> <COMPARE OPERATOR> <VALUE>
```

Each expression part has a compare operator, a left value and a right value.

A VALUE can be:

- a field - name of a field, field names may be put between "{" and "}".
- a constant value - string, number, array, boolean, ...
- a function - must be a function that returns a value

The supported COMPARE OPERATORS are:

- "==" - is equal to
- "!=" - is not equal to
- "<" - is lesser then
- ">" - is bigger then
- "<=" - is lesser then or equal to
- ">=" - is bigger then or equal to
- "matches" - matches a regular expression, right value must be a regular expression
- "starts with" - string starts with a string, right value must be a string (text)
- "in" - value is in an array of values, right value must be an array

Supported logical operators:

- "not", "!" - not
- "and", "&&" - and
- "or", "||" - or

Expression example:

```text
first_name in ['Tess', 'Olivia', 'Abraham'] && age < 25
```

The Ambiorix data model implementation adds some functions to the expression language that can be used in the expressions as well.

- contains(\<VARIANT PATH STRING\>) - returns true or false, depending if the \<VARIANT PATH STRING\> is in the data or not.
- search_path(\<SEARCH PATH\>) - returns a list of existing object paths matching the search path. This function can be used in combination with "in" compare operator.

Example:

```odl
path in search_path("Device.Ethernet.Link.[Enabled == true].")
contains("parameters.Type")
```

### Custom Events

Besides the "default" data model events, it is possible to define your own events. Before a custom event can be triggered or emitted, it must be defined. 

Once the event is defined it can be used anywhere in the code.

#### Define An Event

To define an event there are again two possibilities:

1. Define an event in the odl file
2. Add the event using code

##### Define Event in ODL file

Under any object you can define an event using the keyword `event`.

General syntax:

```odl
event <EVENT NAME>;
```

An example can be found in [Local Agent Threshold](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/localagent_threshold/-/blob/main/odl/la_threshold_definition.odl#L23).

```odl
%define {
    object LocalAgent {
        %read-only csv_string SupportedThresholdOperator = "Rise,Fall,Eq,NotEq";

        object Threshold[] {
            on action validate call threshold_instance_is_valid;
            on action destroy call threshold_instance_cleanup;

            %unique %key string Alias;
            bool Enable = false;
            string OperatingMode {
                default "Normal";
                on action validate call check_enum ["Normal", "Single"];
            }
            string ReferencePath;
            string ThresholdParam;
            string ThresholdOperator {
                default "Rise";
                on action validate call check_is_in "LocalAgent.SupportedThresholdOperator";
            }
            string ThresholdValue;

            event Triggered;
        }
    }
}
```

##### Define Event In Code

Using the data model's signal manager, any event name can be added using the API function:

```C
int amxp_sigmngr_add_signal(amxp_signal_mngr_t* const sig_mngr, const char* name);
```

Example:

```C
amxp_sigmngr_add_signal(&dm->sigmngr, "MyEvent);
```

#### Send Custom Event

Data model event should always be send from a data model object. The data model library provides functions to make it easy to send an event for an object. The function will add the correct elements to the event data, so you don't have to do that by yourself.

```C
void amxd_object_emit_signal(amxd_object_t* const object, const char* name, amxc_var_t* const data);
void amxd_object_trigger_signal(amxd_object_t* const object, const char* name, amxc_var_t* const data);
```

The `data` argument can be any type of variant. When the provided `data` is a variant containing a hash table, the fields `object` and `path` will be added to the hash table. Never use these keys by yourself in your event data as they will be overwritten.

If the `data` variant is not of the hash table type, a hash table variant is created with three items in it:

- `object` - the object path, named notation
- `path` - the object path, index notation
- `data` - your data variant.

## Practical Labs

By now you should known the drill. All practical labs are done in the `Ambiorix Debug & Development` container and in terminals opened in that container, the repository containing this tutorial must be cloned in your workspace directory.

### Lab 1 - Monitor Contacts in Phonebook Data Model

Now lets extend the `phonebook` data model that we started in the [define and publish](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish/-/blob/main/README.md) tutorial. 

Add a parameter to the contacts object

- string Type;

The possible values for this parameter are: `["Unknown", "Family", "Coworker", "Business", "Friend"]`. The default value should be set to `Unknown`. Use the parameter validation action `check_enum` to apply the constraint on this parameter.

An empty `%populate` section is added to the odl file as well.

Add to this section event callback functions:

- `print_event` - on all known events
- `contact_added` - should be called whenever a new `Contact.{i}.` instance object is added to the data model.
- `contact_type_changed` - should be called whenever the `Type` parameter of an contact is changed.

The function implementations are provide in `phonebook.c`. All you need to do is update the odl file. The functions will just print some part of the event data.

The provided code can be build with:

```bash
cd ~/workspace/ambiorix/tutorials/datamodels/server/events/labs/lab1
make
make install
```

Make sure that the `contact_added` is only called when a `Contact` instance is added and not when a `PhoneNumber` or `E-Mail` instance is added.

To help you build the correct filters, here are examples of the event data as printed with the `print_event` function.

Contact instance added event

```text
{
    index = 1,
    keys = {
    },
    name = "1",
    object = "Phonebook.Contact.",
    parameters = {
        FirstName = "John"
        LastName = "Doe",
        Type = "Coworker",
    },
    path = "Phonebook.Contact."
}
```

Type parameter value changed:

```text
{
    object = "Phonebook.Contact.1.",
    parameters = {
        Type = {
            from = "Coworker",
            to = "Friend"
        }
    },
    path = "Phonebook.Contact.1."
}
```

To help you even more, filtering events of `Contact` instances you can use one of the following `object path` filters:

For instance added events:

```text
path matches "^Phonebook\.Contact\.$"
path starts with "Phonebook.Contact."
path in search_path("Phonebook.Contact.")
```

For object changed events:

```text
path matches "^Phonebook\.Contact\.[0-9].*\.$"
path in search_path("Phonebook.Contact.*.")
```

To check if certain parameter has been changed in a object changed event, without caring about the old and new value, you can use the `contains` function in the expression:

```
contains("parameters.Type")
```

When the odl file is update and the code is compiled and installed, launch the phonebook data model:

```bash
amxrt phonebook.odl
```

Using `pcb_cli` you can now add a contact and change the contact type:

```bash
$ pcb_cli
Copyright (c) 2010 SoftAtHome

Connected to pcb://ipc:{/var/run/pcb_sys}

> Phonebook.Contact+{LastName:"Doe", FirstName:"John", Type:"Coworker"}
Phonebook.Contact.1.LastName=Doe  
Phonebook.Contact.1.Type=Coworker
Phonebook.Contact.1.FirstName=John

> Phonebook.Contact.1.Type="Friend"
Phonebook.Contact.1.Type=Friend
```

The output of the phonebook data model application should then look like this (print_event callback handler disabled in the odl file):

```text
New contact instance object added
Last Name  : Doe
First Name : John
Type       : Coworker
Contact instance parameter Type changed
From Coworker to Friend
```

## References

- Tutorial: Getting Started<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md
- Tutorial: Define And Publish<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish/-/blob/main/README.md
- TR-369 USP<br>
https://www.broadband-forum.org/download/TR-369.pdf
- Wikipedia - Event loop<br>
https://en.wikipedia.org/wiki/Event_loop
- Example: Greeter Application<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_app
- Example: Greeter Plugin<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_plugin/
- Example: Local Agent Threshold<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/localagent_threshold/
- Wikipedia - Observer pattern<br>
https://en.wikipedia.org/wiki/Observer_pattern
- Wikipedia - Qt framework - signal/slot<br>
https://en.wikipedia.org/wiki/Signals_and_slots
- Observer pattern - signal/slot (libamxp)<br>
https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp/-/blob/main/doc/signal_slot.md
- Ambiorix Runtime<br>
https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt
- libamxp API documentation<br>
https://soft.at.home.gitlab.io/ambiorix/libraries/libamxp/doxygen
