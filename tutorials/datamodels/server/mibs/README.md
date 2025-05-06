# Ambiorix Data Models - Modular Information Block

Estimated Time needed: 40 - 60 minutes.

[[_TOC_]]

## Introduction

Using a data model is an easy place to store configuration options, statuses and information. Using multi-instance objects many instances of the same object can be created each with its own values for the parameters defined in the multi-instance object.

However sometimes there is a need to add more information to such an instance or instances. It is of course possible to define a multi-instance object that can hold all possible parameters, but often you end up with an object that contains loads of parameters, and only a few are used by each instance, all other parameters are just initialized to their default value and never touched.

An example of such a data model is a data model that contains all possible network interfaces of gateway. For wireless network interfaces extra configuration or settings could be needed, like channel, BSSID, and so on. Wired network interfaces do not need that information.

Another example is information collection of all connected devices, which can range from USB, Ethernet, WiFi, IoT (zwave, zigbee, ...) and so on. Each of these devices can expose other information. Defining a fixed multi-instance object with all possible parameters could lead to a complex data model (or object) definition.

## Goal

The goal of this tutorial is to explain:

- How to define modular information blocks.
- How to add a modular information block to an (instance-)object.

## Prerequisites

- You finished the [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) tutorial
- You finished the [Define And Publish](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish/-/blob/main/README.md) tutorial
- You have a basic understanding of the Ambiorix variants
- You have basic C knowledge
- You have basic git knowledge
- You can work with docker containers

## Modular Information Blocks

The `Broadband Forum` data models are not supporting `Modular Information Blocks`, this feature is provided by the Ambiorix data model library (libamxd) to make data models a bit more flexible and expandable.

In this tutorial this feature is explained using the `phonebook` data model. You can either follow step by step and apply the explanations on [Practical lab 1](#part-1-add-contact-type-mibs) or read it through and then dive in the practical lab.

You can already clone this repository here if you want to:

```bash
mkdir -p ~/workspace/ambiorix/tutorials/datamodels/server
cd ~/workspace/ambiorix/tutorials/datamodels/server
git clone git@gitlab.com:prpl-foundation/components/ambiorix/tutorials/datamodels/server/mibs.git
```

### Define a MIB

`Modular Information Blocks` are defined in a similar way as data model objects. In your odl file a mib is defined in a `%define` section using the keyword `mib` and has a name. The content of a mib is defined in the mib body and can contain the same members as an object definition.

You can add parameter definitions and object definitions in a mib body.

Example:

```odl
%define {
    mib Coworker {
        string Department = "" {
            on action validate call check_maximum_length 64;
        }
        string Team = "" {
            on action validate call check_maximum_length 64;
        }
    }
}
```

This mib example is already added to the `phobebook.odl` in lab1.

Other mibs can be defined as well for the other types of contacts. You can now switch to [Practical lab 1 part 1](#part-1-add-contact-type-mibs) or continue reading.

### Add a MIB to an object

Defining mib's is not sufficient, you must add them to the correct data model objects to make them `visible`.

Most of the time you want to apply them dynamically, depending on a condition or when certain event has happened. To have this dynamic behavior you must add the mib in code.

Adding a mib to an object can be done with:

```C
amxd_status_t amxd_object_add_mib(amxd_object_t* const object, const char* mib_name);
```

This function returns `amxd_status_ok` when the mib is added to the object. When the mib is added all parameters or child objects defined in the mib are added to the object.

Using function:

```C
bool amxd_object_has_mib(amxd_object_t* object, const char* mib_name);
```

you can check if a MIB was added to the object or not.

Now you can switch to [Practical lab 1 part 2](#part-2-add-mib-to-contact) and update the code so the correct mib is added the newly created contacts or continue reading. 
### Remove a MIB from an object

A MIB can be removed from an object as well using the function:

```C
amxd_status_t amxd_object_remove_mib(amxd_object_t* const object, const char* mib_name);
```

This function will return `amxd_status_ok` when the MIB is removed or the MIB was not added to the object.

### MIB restrictions and limitations

There are some restrictions and limitations when using MIBs.

A MIB can not be added to an object when this would lead to duplicate parameter names or object names, this is a naming conflict.

Example:

```odl
%define {
    mib MyMib {
        string MyParam;
    }
}

%define {
    object MyObject {
        string MyParam;
    }

    object MyOtherObject {
        string MyText;
    }
}
```

In above example, adding MIB `MyMib` to object `MyObject` will not be possible as that leads to a parameter name conflict, while adding the same MIB to object `MyOtherObject` is not a problem at all.

You can now continue with the practical lab [Practical lab 1 part 3](#part-3-replace-mib-when-contact-type-changes)
## Practical Labs

By now you should known the drill. All practical labs are done in the `Ambiorix Debug & Development` container and in terminals opened in that container. If you didn't already clone the repository of this tutorial, it is now time to do so.

### Lab 1 - Using mibs

For this lab you need to compile some code. 

```bash
cd ~/workspace/ambiorix/tutorials/datamodels/server/mibs/labs/lab1
make
make install
```

#### Part 1 - Add contact type mibs

Add mib definitions for all contact types, a definition for "Coworker" is already added.

Other mibs:

- `Family` - add parameter `Relation` which is a string and can be `Sibling`, `Parent`, `Grandparent`, `Uncle`, `Aunt` or an empty string and parameter `Birthday` which must be a date time parameter.
- `Business` - add string parameter `Company` and string parameter `Website`.
- `Friend` - add a comma separated value string parameter `Hobbies`

---
>**Note**<br>
>When only changing the odl files there is no need to compile the source code. 
---

#### Part 2 - Add mib to contact

When a contact is added an event callback function is triggered. This event callback function is added to the data model in the odl file:

```odl
on event "dm:instance-added" call contact_added
    filter 'path matches "^Phonebook\.Contact\.$"';
```

In this event callback function the correct mib can be added to the new contact using:

```C
amxd_status_t amxd_object_add_mib(amxd_object_t* const object, const char* mib_name);
```

To be able to add the mib, an object pointer is needed (the new contact instance) and the mib name as defined in the odl file. Both can be extracted from the event data.

Get the pointer to the new contact instance:

```C
amxd_object_t* contacts = amxd_dm_signal_get_object(the_data_model, data);
amxd_object_t* contact = amxd_object_get_instance(contacts, NULL, GET_UINT32(data, "index"));
```

---
>**Note**
> When working with hash table variants there are some convience macro's available. These macro's make it easier to fetch parts from the composite variant:
>
> Example:<br>
> Instead of writing
> ```C
> uint32_t number = amxc_var_constcast(uint32_t, amxc_var_get_key(myvar, "Number", AMXC_VAR_FLAG_DEFAULT));
> ```
> You can use the macro:<br>
> ```C
> uint32_t number = GET_UINT32(myvar, "Number");
> ```
>
> See the [amxc_variant API](https://soft.at.home.gitlab.io/ambiorix/libraries/libamxc/master/d5/de9/a00176.html) documentation to see the full list of available macro's 
---

The event (or signal) `dm:instance-added` is send using the multi-instance object `Phonebook.Contact`, in the event data the index of the new created instance is available.

The defined mib names are matching the possible values of the contact `Type` parameter, so using that value as the mib name should be sufficient.

To get the value of the `Type` parameter there are multiple options:

- get it from the event data
  ```C
  const char* type = GETP_CHAR(data, "parameters.Type");
  ```
- get it from the contact object
  ```C
  char* type = amxd_object_get_value(cstring_t, contact, "Type", NULL);
  ```

---

>**NOTE**
When fetching the value from the the `contact` instance object, you are getting a copy. The returned value must be freed.<br>
When using the value from the event data, you can access the value directly, no copy taken, so no free needed.

---

Now you have all arguments, to add the mib to the newly created instance.

---

>**NOTE**
For the `Unknown` contact type no mib was defined. When creating a new contact instance and setting the type to `Unknown` will not add any mib to the newly created contact. The function call `amxd_object_add_mib` will not return `amxd_status_ok`. In this case we just ignore this error as it is the intention not to add any mib.
>
>You can add error handling and check if the return values is `amxd_status_ok` or not. That is up to you.

---

You can now test and check if the mibs are added when creating a new contact.

```bash
$ pcb_cli
Copyright (c) 2010 SoftAtHome

Connected to pcb://ipc:{/var/run/pcb_sys}
> Phonebook.Contact.+{LastName:"Doe",FirstName:"John",Type:"Coworker"}
Phonebook.Contact.1.LastName=Doe
Phonebook.Contact.1.Type=Coworker
Phonebook.Contact.1.FirstName=John
> Phonebook.Contact.1?
Phonebook.Contact.1
Phonebook.Contact.1.Team=
Phonebook.Contact.1.LastName=Doe
Phonebook.Contact.1.Department=
Phonebook.Contact.1.Type=Coworker
Phonebook.Contact.1.FirstName=John
Phonebook.Contact.1.PhoneNumber
Phonebook.Contact.1.E-Mail
> Phonebook.Contact.+{LastName:"Doe",FirstName:"Jane",Type:"Business"}
Phonebook.Contact.2.LastName=Doe
Phonebook.Contact.2.Type=Business
Phonebook.Contact.2.FirstName=Jane
> Phonebook.Contact.2?
Phonebook.Contact.2
Phonebook.Contact.2.Website=
Phonebook.Contact.2.LastName=Doe
Phonebook.Contact.2.Type=Business
Phonebook.Contact.2.FirstName=Jane
Phonebook.Contact.2.Company=
Phonebook.Contact.2.PhoneNumber
Phonebook.Contact.2.E-Mail
>
```

As the mib is added after creating the contact, the mib parameters must be set after creating the contact.

```bash
$ pcb_cli
Copyright (c) 2010 SoftAtHome

Connected to pcb://ipc:{/var/run/pcb_sys}
> Phonebook.Contact.1.Department="Sales"
Phonebook.Contact.1.Department=Sales
> Phonebook.Contact.2.Company="qacafe"
Phonebook.Contact.2.Company=qacafe
>
```

#### Part 3 - Replace mib when contact type changes

When the parameter `Type` value changes an event callback function is triggered. This event callback function is added to the event data model in the odl file: 

```odl
on event "dm:object-changed" call contact_type_changed
    filter 'path in search_path("Phonebook.Contact.*") &&
            contains("parameters.Type")';
```

The event data will contain the old value and the new value of the `Type` parameter. The old value can be taken from the event data using:

```C
const char* old_type = GETP_CHAR(data, "parameters.Type.from");
```

The new value can be taken from the data model object or from the event data, here we will use the event data:

```C
const char* new_type = GETP_CHAR(data, "parameters.Type.to");
```

When the event callback function is called, the type has been changed, this is ensured by the event filter. No need to check that the old and new value are different.

The first thing that needs to be done is remove the previous added MIB, use:

```C
amxd_status_t amxd_object_remove_mib(amxd_object_t* const object, const char* mib_name);
```

And then install the new MIB using:

```C
amxd_status_t amxd_object_add_mib(amxd_object_t* const object, const char* mib_name);
```

Both functions need the mib name, in our case the mib name is the same as the type value, we already have these.

We also need an object pointer, as the event `dm:object-changed` is send from the object that has been changed, fetching the object pointer is easy:

```C
amxd_object_t* contact = amxd_dm_signal_get_object(the_data_model, data);
```

## References

- Tutorial: Getting Started<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md
- Tutorial: Define And Publish<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish/-/blob/main/README.md
