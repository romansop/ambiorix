# Ambiorix Data Models - RPC Methods

[[_TOC_]]

## Introduction

A data model containing a hierarchical tree of objects (nodes), where each object contains a set of parameters can be used to store configuration options or to represent the current state of a service or application.

Client applications can interact with such a data model to change the configuration or to query the current state.

One of the problems is that a client application often wants to change some configuration and needs to modify multiple objects in the data model tree, this can lead to many "requests" from the client application to the service providing the data model. In such a case the client application is responsible for correctly configuring the service and that configuration logic needs to be re-implemented in each client over and over again. This is not a good design for multiple reasons:

1. The logic how to configure a service should be implemented in the service itself, client applications should only provide the configuration data.
2. What if the service implementation changes, all client must be updated as well.
3. Code duplication should be avoided as much as possible.

Often there is more to a service than only configuration data and status information as stored in the data model. Complex functionality could be implemented in a service as well, and the service wants to expose this functionality in a `public` interface, without the need to create a very complex data model.

Data model RPC methods can help in implementing these use cases.

## Goal

Learn how to implement data model RPC methods.

## Prerequisites

- You finished the [Component Structure](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/build-debug/component-structure/-/blob/main/README.md) tutorial

## RPC Methods

### Define A Method

RPC methods can be defined in an odl file, and must be placed in the object's body definition. A RPC method has a return value an can have zero, one or more arguments.

The syntax to define a method is:

```odl
<RETURN TYPE> <METHOD NAME>([<ARGUMENT> [, <ARGUMENT>, ...]]);
```
When a RPC method is not returning anything the special `void` return type can be used to indicate nothing is returned.

An example of some RPC method definitions from [greeter-plugin](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_plugin):

```odl
%define {
    object Greeter {

        ...

        uint32 setMaxHistory(%in %mandatory uint32 max);

        bool save(%in %strict string file = "${save_file}");
        bool load(%in %strict string file = "${save_file}");

        ...

    }
}
```

The RPC method `setMaxHistory` in this example, changes the read-only parameter `MaxHistory`, but also performs some checks and deletes `History` instances if there are more instances available than the new `MaxHistory`. When done it returns the new set `MaxHistory` which can be different than the `requested` new maximum. `History` instances that have the flag `Retain` set can not be deleted, so it is possible that the new `requested` maximum can not be set. As the real set `MaxHistory` is returned, this functionality can not be implemented using an `event` callback function, because with event callback functions nothing can be returned.

The RPC method `save` saves all persistent data in the data model to a file and the `load` method is able to load the data back in the data model. Adding such functionality to the data model only using `objects` and `parameters` would become very cumbersome.

#### Method Arguments

When a client application wants to invoke your methods, they often want to pass data. The data is passed as method arguments. Each argument has a type and a name, which must match one of the `amxc_var_t` types.

Besides the argument type and the argument name, attributes can be set on arguments. The available attributes for arguments are:

- `%in` - The argument is an input argument (passed from client application to server application), this is the default.
- `%out` - The argument is an output argument (passed from server application to client application)
- `%strict` - The argument type is strict type checked. If the passed type doesn't match the defined type, the RPC method call done by a client application will fail. When an argument is not strict typed (default), the data model server application should be able to handle other value types as well (performs conversions if needed).
- `%mandatory` - By default all arguments are optional, using this attribute the argument becomes mandatory. A RPC method call done by a client application without providing all mandatory input arguments will fail.

For optional arguments a default value can be defined. When a client application is omitting all or some optional arguments, and a default is defined, the data model server application that implements the RPC method will get the default value. If no default value is defined for an optional argument, and a client is omitting that argument, the implementation will not get (receive) the argument.

#### Multi-instance Objects

Methods can be defined on any object, but when defining them on a multi-instance object, it will be available on each instance object by default (instance objects inherit the methods of a multi-instance object). These methods can only be called on the instance objects by default.

This behavior can be changed using method attributes.

- `%instance` - method can only be called on instance objects
- `%template` - method can only be called on multi-instance objects and the method is not inherited by the instances.

These attributes can be used in combination, then the method can be called on all instances and on the multi-instance object as well.

Multi-instance object only methods (with only attribute `%template` set) can be considered as `class methods`.

An example of a multi-instance object method can be found in the [Greeter Plug-in](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_plugin) example:

```odl
%define {
    object Greeter {

        ...

        %persistent %read-only object History[] {
            counted with HistorySize;
            
            %template uint32 clear(%in bool force = false);

            ...

        }

        ...

    }
}
```

In this example when the method is called all `History` instances are deleted. When the `force` argument is set to `true`, all items of which `Retain` is set to `true` are deleted as well.

It is now time to switch to [lab 1 - Define Phonebook Methods](#lab-1-define-phonebook-methods).

### Implement A Method

Defining a method in the data model definition file is not sufficient to make it work, implementations must be provided as well. 

To implement a data model RPC method, you need to declare a function in your source code that matches with this prototype:

```C
amxd_status_t _<FUNCTION NAME>(amxd_object_t* object,
                               amxd_function_t* func,
                               amxc_var_t* args,
                               amxc_var_t* ret)
```

The `<FUNCTION NAME>` must match the function name as specified in the odl file and must start with a single '_'. Optionally the object name can be used as a prefix for the function name.

So for the `phonebook` data model `add_contact` RPC method, these function names are valid:

```C
amxd_status_t _add_contact(amxd_object_t* object,
                           amxd_function_t* func,
                           amxc_var_t* args,
                           amxc_var_t* ret);

amxd_status_t _Phonebook_add_contact(amxd_object_t* object,
                                     amxd_function_t* func,
                                     amxc_var_t* args,
                                     amxc_var_t* ret);
```

Method implementations always have these arguments:

- `object` - the object pointer on which the method was called
- `func` - pointer to the method definitions
- `args` - the method arguments, these are provided by the caller (the client) or can be the default values as provided with the function definition. The `args` is a hash table variant and all provided method arguments are provided in that hash table with the name of the argument. It is possible that optional arguments that don't have a default value are not provided in this table.
- `ret` - the method return value. This is a variant and can be filled with any value. It is recommended that the variant will be of the type as defined in the function definition.  

#### Function Resolving

The odl parser (libamxo) resolves the functions and has 3 default function resolvers available:

- `Function Table Resolver` - (aka ftab resolver) uses a function table that contains the function names as used in the odl (is the key) and function pointers which are the implementations.
- `Import Resolver` - uses [dlopen](https://linux.die.net/man/3/dlopen) to load a shared object (using keyword `import` in the odl files) and  [dlsym](https://linux.die.net/man/3/dlsym) to resolve the function pointer when a method name is encountered in the odl file.
- `Auto Resolver` - this resolver doesn't resolve the functions by itself, but uses other resolvers and tries them in a specified order. The order in which the resolvers are used is defined by the configuration option `auto-resolver-order`.

##### Function Table Resolver

The function table resolver keeps a hash table where the keys are the function names and the values are the function implementations (function pointers).

The keys in this hash table can be used as method names in the odl. You can add and remove functions/methods to and from the function table using the function table API provided by libamxo:

```C
int amxo_resolver_ftab_add(amxo_parser_t* parser,
                           const char* fn_name,
                           amxo_fn_ptr_t fn);
                           
int amxo_resolver_ftab_remove(amxo_parser_t* parser,
                              const char* fn_name);
```

In most cases you will not need to add functions/methods to the function table when using the `Ambiorix Run Time` (amxrt). When creating a stand alone application or modifying an existing one and you can not make use of [add-on](https://en.wikipedia.org/wiki/Plug-in_(computing)) like system as is done with `Ambiorix Run Time` you will need to provide the function pointers and names using the `Function Table`.

An example of such usage can be found in the [Greeter Application](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_app/-/blob/main/src/greeter_main.c#L113) example:

```C

    ...

    amxo_resolver_ftab_add(parser, "Greeter.echo", AMXO_FUNC(_function_dump));
    amxo_resolver_ftab_add(parser, "Greeter.say", AMXO_FUNC(_Greeter_say));
    amxo_resolver_ftab_add(parser, "Greeter.History.clear", AMXO_FUNC(_History_clear));
    amxo_resolver_ftab_add(parser, "Greeter.setMaxHistory", AMXO_FUNC(_Greeter_setMaxHistory));

    ....

```

##### Import Resolver

The most used resolver will probably be the `Import Resolver`. This resolver makes the Ambiorix [plug-in (add-on)](https://en.wikipedia.org/wiki/Plug-in_(computing)) data model system work.

By using the Linux system dynamic library loader (dl) which exposes functions as [dlopen](https://linux.die.net/man/3/dlopen) and [dlsym](https://linux.die.net/man/3/dlsym) it is easy to dynamically load a shared object and resolve functions from it.

To make this work from an odl file, the odl parser uses this resolver. Whenever the keyword `import` is encountered it loads the mentioned shared object using [dlopen](https://linux.die.net/man/3/dlopen). Whenever function pointers need to be resolved, the import resolver will search all loaded shared objects (with import) for the a symbol with the same name as specified in the odl file, using [dlsym](https://linux.die.net/man/3/dlsym).

##### Auto Resolver

The order in which the `auto` resolver tries the function resolvers can be configured in the configuration section of your odl (preferably the main odl file):

```odl
%config {
    auto-resolver-order = [
                "ftab",
                "import",
                "*"
    ]
}
```

The above configuration is the default configuration for the auto resolver. An `*` can be put at the end to indicate to also use any other registered function resolver in any order.

The auto resolver stops as soon as a function implementation is found.

##### Providing Resolve Instructions

Although you probably never need this functionality, it is mentioned in this tutorial for completeness.

You can provide `resolver instructions` in the odl file, after the definition of the method.
Resolver instructions start with `<!` and end with `!>`, in between resolver instructions can be added, the first part is always the resolver name, all the rest of the resolver instructions depend on the resolver used. When adding resolver instructions read the documentation of the resolver you are using.

Example:

```odl
bool save(%mandatory %in %strict string filename)<!import:phonebook:dummy!>
```

It is now time to switch to [Lab 2 - Set Dummy Implementation](#lab-2-set-dummy-implementation).

#### Method arguments

The arguments provided to the implementation of the data model method are provided as a hash table variant. 

In the `phonebook` data model we can define a method to add a contact:

```odl
%define {
    object Phonebook {

        bool add_contact(%mandatory %in %strict string firstname,
                         %mandatory %in %strict string lastname,
                         %in %strict list phones, 
                         %in %strict list emails);
    ...

```

This method has 4 arguments:

- `firstname`
- `lastname`
- `phones`
- `emails`

The prototype of this method can be:

```C
amxd_status_t _add_contact(amxd_object_t* object,
                           amxd_function_t* func,
                           amxc_var_t* args,
                           amxc_var_t* ret);
```

The defined arguments of the data model method will be passed in the variant `args`. 

```text
{
    firstname = "John",
    lastname = "Doe",
    phones = [ "(+032) 555 552 872", "(+032) 555 552 471" ],
    emails = [ "johnd@ambiorix.com" ]
}
```

The values of these arguments can be fetched using the `amxc_variant` API. In this case all arguments are strict typed, so you can be sure that the strings are string variants and the lists are list variants, so you can use the variant macro's or `amxc_var_constcast` to take the value out of the `args` variant.  

#### Data Model Transactions

As mentioned before in this tutorial, most of the time it is easier to provide an RPC method than letting data model clients implement a sequence of operations on your data model.

One of the advantages is that you can make it look like one atomic operation on your data model to other applications and it is easier for clients to just invoke one single `operation` than implementing a sequence.

The Ambiorix data model library provides data model transactions which can be used to apply data model changes in `one` go.

As set of data model transaction API's are available:

Constructor and destructor functions:

```C
amxd_status_t amxd_trans_init(amxd_trans_t* const trans);
amxd_status_t amxd_trans_new(amxd_trans_t** trans);
void amxd_trans_clean(amxd_trans_t* const trans);
void amxd_trans_delete(amxd_trans_t** trans);
```

Select the object you are working on:

```C
amxd_status_t amxd_trans_select_pathf(amxd_trans_t* const trans,
                                      const char* path,
                                      ...);
amxd_status_t amxd_trans_select_object(amxd_trans_t* trans,
                                       const amxd_object_t* const object);
```

Apply changes:

```C
amxd_status_t amxd_trans_add_inst(amxd_trans_t* const trans,
                                  const uint32_t index,
                                  const char* name);
amxd_status_t amxd_trans_del_inst(amxd_trans_t* const trans,
                                  const uint32_t index,
                                  const char* name);
#define amxd_trans_set_value(type, trans, name, value);
```

Using a transaction you can create instances, delete instances, change parameter values, even change values of read-only parameters for multiple objects.

To create a transaction that can change read-only parameters you need to set a transaction attribute `amxd_tattr_change_ro` using API function:

```C
amxd_status_t amxd_trans_set_attr(amxd_trans_t* trans,
                                  amxd_tattr_id_t attr,
                                  bool enable);
```

When creating a transaction you basically create a sequence of `actions`. When applying this sequence to the data model the Ambiorix data model engine will track the changes and creates a reverse transaction. If one of the actions fails, the reverse transaction is applied and everything is back to its original state.

It is now time to switch to [Lab 3 - Implement add_contact](#lab-3-implement-add_contact).

#### Saving and Loading persistent data model parameters

Making a data model `boot` persistent can be achieved in different ways, in this tutorial RPC methods are used to save and load the data model on request. A data model client can invoke the RPC methods `save`, to save the persistent data and `load` to reload the saved data, to or from a file.

In [lab 1 - Define Phonebook Methods](#lab-1-define-phonebook-methods) the `save` and `load` method were defined in the odl file, the only thing missing is the method implementations and to define which objects and parameters are considered persistent data.

To make objects and parameters persistent, the attrribute `%persistent` must be added to all objects and parameters that must be stored persistent.

To save the persistent data, the function `amxo_parser_save_object` can be used:

```C
int amxo_parser_save_object(amxo_parser_t* pctx,
                            const char* filename,
                            amxd_object_t* object,
                            bool append);
```

The object pointer given to this function is the starting point, this doesn't need to be a root object, it can be any object within your data model, as long as it has the `persistent` attribute set.

When `append` is set to `true` the saved data is appended to the file, if the file already exists, otherwise the file will be created. When `append` is set to `false` and the file exists, it will be overwritten. 

To load a saved file, the function `amxo_parser_parse_file` can be used:

```C
int amxo_parser_parse_file(amxo_parser_t* parser,
                           const char* file_path,
                           amxd_object_t* object);
```

This method can also be used to load data model or mib definitions. The loaded data will be added under the object pointer provided.

---
>**NOTE**<br>
> Most of the time you will want to save the full data model or load saved data under the `root` objects. To get the `common` root object of your data model you can use the function `amxd_dm_get_root`.<br>
>
>
> ```
> amxd_object_t* amxd_dm_get_root(amxd_dm_t* const dm);
> ```
>
---

You should have enough information now to implement the save and load RPC methods, so let's switch to [lab 4 - Implement save and load](#lab-4-implement-save-and-load)


#### Methods with output arguments

Ambiorix methods have 2 ways of returning an outputs to the caller. The first way is to write the return type in front of the function. For example the `save` method from lab 4 will return a boolean:

```
bool save(%mandatory %in %strict string filename);
```

This boolean is returned by updating the return variant of the `save` method implementation. The caller will always receive a list of output arguments when calling an ambiorix method and the first item in the list will always be the return variant.

For the `save` method this can be seen by invoking the command using ubus

```bash
$ sudo ubus call Phonebook save '{"filename":"/tmp/contacts.odl"}'
{
    "retval": true
}
```

However as mentioned before, it is also possible to return outputs to the caller by setting the `%out` attribute in front of the argument. All arguments that are marked as output arguments will be returned in the second element of the list of output arguments and they will be stored here in a hash table. This can be used to easily provide the caller with a number of named output arguments.

For example assume we want to add an output argument to the `save` function to indicate whether a previous file is overwritten. This could be a boolean parameter named `newfile`, which is set to true in case the file is new or false if an old file is overwritten.

After implementing this, the save method would return a second value as can be seen with ubus

```bash
$ sudo ubus call Phonebook save '{"filename":"/tmp/contacts.odl"}'
{
        "retval": true
}
{
        "newfile": false
}
```

The first returned value is still the original return value, but there is now a second element returned which is a hash table with the output arguments.

When implementing TR-181 data models, RPC methods always have a clear indication of input and output arguments. All input arguments should be marked with the `%in` attribute and output arguments with the `%out` attribute in the odl file. This is especially important if the output arguments need to be returned to a remote caller by the USP agent. If every data model implementation would return its output arguments in a custom way, it would be impossible for the USP agent to retrieve them in a generic way and provide them back to the caller. Therefore the USP agent will only look at the second element that is returned after invoking a method. When ambiorix is used, this element is always a hash table of key-value pairs which can easily be parsed and converted to a USP message.

When an error occurs, the RPC method will return with a status different from `amxd_status_ok`. If there is detailed error information available at the server side, it can also be provided by extending the `args` table with a key `err_code` of type `uint32_t` and an `err_msg` of type `cstring_t`. The available error codes for failed RPC methods are listed below

```C
#define USP_ERR_GENERAL_FAILURE           7000       // Message failed. This error indicates a general failure that is described in an err_msg element.
#define USP_ERR_MESSAGE_NOT_UNDERSTOOD    7001       // Attempted message was not understood
#define USP_ERR_REQUEST_DENIED            7002       // Cannot or will not process message
#define USP_ERR_INTERNAL_ERROR            7003       // Message failed due to an internal error
#define USP_ERR_INVALID_ARGUMENTS         7004       // Message failed due to invalid values in the request elements and/or failure to update one or more parameters during Add or Set requests
#define USP_ERR_RESOURCES_EXCEEDED        7005       // Message failed due to memory or processing limitations
#define USP_ERR_PERMISSION_DENIED         7006       // Source endpoint does not have authorisation to use this message
#define USP_ERR_INVALID_CONFIGURATION     7007       // Message failed because the result of processing the message would result in an invalid or unstable state
#define USP_ERR_INVALID_PATH_SYNTAX       7008       // Requested path was invalid or a reference was invalid
#define USP_ERR_OBJECT_DOES_NOT_EXIST     7016       // Requested object instance does not exist
#define USP_ERR_COMMAND_FAILURE           7022       // Command failed to operate
#define USP_ERR_INVALID_PATH              7026       // Path is not present in the data model schema
#define USP_ERR_INVALID_COMMAND_ARGS      7027       // Command failed due to invalid arguments
```

When no `err_code` or `err_msg` is provided, the USP agent will provide a generic error code and message to the caller.

More information regarding USP Operate messages can be found in the [USP specification](https://usp.technology/specification/07-index-messages.html#sec:defined-operations-mechanism).

You can now start working on [lab 5 - methods with output arguments](lab-5-methods-with-output-arguments).

#### Synchronous and asynchronous methods

Methods can be called in a synchronous or an asynchronous way. They can be called in a synchronous way by using `amxb_call`, which will make the caller block until the called function returns or a timeout occurs. They can also be called in an asynchronous way by using `amxb_async_call` and providing a callback function that will be called when the function completes. `amxb_async_call` will always return immediately after the function is launched, so the caller will not be blocked.

On the callee side (server side) methods can also be implemented synchronously or asynchronously. When a synchronous method is entered, the plugin will handle the business logic related to the method and then return with an `amxd_status_t` code at the end of the method. However in some situations, the business logic can take quite a while to complete and you don't want your plugin to block until everything has been handled. In this case the method can be implemented as a deferred method. When an ambiorix method returns with `amxd_status_deferred`, it means that it will return at a later point of time with the real return value and output arguments. This allows you to make a context switch in the plugin before handling the rest of the method.

An example of a deferred method can be found in the [greeter plugin](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_plugin/-/blob/main_v0.4.3/src/dm_greeter_methods.c#L191). Make sure you also take a look at the doxygen documentation regarding deferred methods.


When TR-181 data model methods are implemented, these methods always contain a clear `[ASYNC]` flag in the function description. This flag is typically added to methods that are expected to take a considerable amount of time to complete, so it is recommended that they are called asynchronously and depending on the situation, it might be interesting to implement them in a deferred way. Moreover if USP is used, these methods MUST be called asynchronously to be in line with the specification. Because the USP agent has no prior knowledge of the data model methods it needs to invoke, it doesn't know in advance if a method needs to be invoked synchronously or asynchronously. Therefore it is important that functions are marked with an `%async` userflag which can be retrieved by the USP agent before calling the method. If this flag is set, the USP agent will call the function asynchronously, if it is missing, it will call the function synchronously.

Below is an example of a data model method with the `%async` userflag set:

```
void DummyMethod() {
    userflags %async;
}
```

> Note: if pcb data models are used, the function can be marked as asynchronous by providing the async function attribute to the function i.e.
> async void DummyMethod();

## Practical Labs

By now you should known the drill. All practical labs are done in the `Ambiorix Debug & Development` container and in terminals opened in that container. If you didn't already clone the repository of this tutorial, it is now time to do so.

This tutorial is structured according the `SoftAtHome` component structure policy as explained in the [Component Structure](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/build-debug/component-structure/-/blob/main/README.md) tutorial.

To build and install:

```bash
cd ~/workspace/ambiorix/tutorials/datamodels/server/rpc-methods/
make
sudo -E make install
```

### Lab 1 - Define Phonebook Methods

In our phonebook data model `Contacts` can be added and for each contact multiple phone numbers and e-mail addresses. A client application can use the default data model operators available as explained in [Simple Client](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/client/simple-client/-/blob/main/README.md) tutorial.

If a client application wants to add a contact with multiple phone numbers and multiple e-mail addresses it needs to do multiple calls:

- `add` - Add the new contact
- `add` - Add a phone number (must be repeated for each phone number)
- `add` - Add an e-mail address (must be repeated for each e-mail address)

A method can be provided that can create a new contact and add a list of phone numbers or e-mail addresses.

Currently the data in the phonebook data model is not `boot` persistent, it would be nice that the contact data can be saved and loaded.

Define the following methods in the data model definition (file odl/phonebook_definition.odl) under object `Phonebook`:

```odl
bool add_contact(%mandatory %in %strict string firstname,
                 %mandatory %in %strict string lastname,
                 %in %strict list phones, 
                 %in %strict list emails);

bool save(%mandatory %in %strict string filename);
bool load(%mandatory %in %strict string filename);
```

After adding the method definitions, launch the phonebook data model:

```bash
$ phonebook
WARNING No function implemention found for "add_contact" using "auto"@/etc/amx/phonebook/phonebook_definition.odl:line 7
WARNING No function implemention found for "save" using "auto"@/etc/amx/phonebook/phonebook_definition.odl:line 9
WARNING No function implemention found for "load" using "auto"@/etc/amx/phonebook/phonebook_definition.odl:line 10
```

You will get 3 warnings from the odl parser, as no function implementations can be found.
The odl parser mentions the function resolver that was used to try to find an implementation for these functions. In this case the `auto` function resolver was used, which is the default function resolver.

When trying to call these methods using pcb_cli or ubus you will get an error.

With pcb_cli:

```bash
$ pcb_cli
Copyright (c) 2010 SoftAtHome

Connected to pcb://ipc:{/var/run/pcb_sys}
> Phonebook.save(filename:"/tmp/contacts.odl")
0x0003001C Function is not implemented: Phonebook
Phonebook.save() failed
> Phonebook.load(filename:"/tmp/contacts.odl")
0x0003001C Function is not implemented: Phonebook
Phonebook.load() failed
> Phonebook.add_contact(firstname:"John", lastname:"Doe", phones: [], emails:[] )
0x0003001C Function is not implemented: Phonebook
Phonebook.add_contact() failed
>

```

```bash
$ ubus call Phonebook save '{"filename":"/tmp/contacts.odl"}'
Command failed: Not found
$ ubus call Phonebook load '{"filename":"/tmp/contacts.odl"}'
Command failed: Not found
$ ubus call Phonebook add_contact '{"firstname":"John", "lastname":"Doe"}'
Command failed: Not found
```

### Lab 2 - Set Dummy Implementation

In lab 1 you added the definitions of some functions to the data model definition. When launching the `phonebook` data model you got some warning, when trying to call these methods you got back errors. Both, the warnings and errors, are normal as no implementation was provided.

In the file `src/dm_phonebook_methods.c` (any clue what that source files contains?) a `dummy` data model RPC method is implemented.

```C
amxd_status_t dummy(amxd_object_t* object,
                    amxd_function_t* func,
                    amxc_var_t* args,
                    amxc_var_t* ret) {
    const char* fn_name = amxd_function_get_name(func);
    char* obj_path = amxd_object_get_path(object, AMXD_OBJECT_TERMINATE);

    printf("Function %s called on %s\n", fn_name, obj_path);
    printf("Provided arguments = \n");
    fflush(stdout);
    amxc_var_dump(args, STDOUT_FILENO);

    amxc_var_set(bool, ret, true);
    return amxd_status_ok;
}
```

In the main odl file `odl/phonebook.odl` the shared object that contains the phonebook implementations is imported:

```odl
import "${name}.so" as "${name}";
```

---
> **NOTE**<br>
`${name}` is replaced with the value of the configuration option `name`. You will not find this configuration option any where in the `%config` section of the odl. As we are using the symbolic link feature to amxrt, the name of the symbolic link will be used. The `name` configuration option is added by `amxrt`.
---

An alias for the library is used which makes it easier to reference the shared object.

Using the import resolver and resolver instructions we can now use the `dummy` implementation. This is of course temporary until the real RPC methods are implemented.

The import resolver takes as argument the `<shared object>:<function name>`

Now update the `odl/phonebook_definition.odl` and add resolver instructions to the defined methods.

You can use this instruction:

```text
<!import:phonebook:dummy!>
```

Example:

```odl
bool load(%mandatory %in %strict string filename)<!import:phonebook:dummy!>;
```

When you modified the `odl/phonebook_definition.odl` re-launch the `phonebook` data model, make sure that you run the phonebook data model in foreground, as the `dummy` RPC method is printing to `stdout`.

```bash
$ phonebook
```

If all goes well you should not see any warnings or errors.

In another terminal try to call the methods using pcb_cli or ubus tool

```bash
$ pcb_cli
Copyright (c) 2010 SoftAtHome

Connected to pcb://ipc:{/var/run/pcb_sys}
> Phonebook.add_contact(firstname:"John", lastname:"Doe", phones: [], emails:["john.d@ambiorix.com"] )
Phonebook.add_contact() returns 1
```

In the terminal where the `phonebook` data model was launched you should see now:

```bash
Function add_contact called on Phonebook.
Provided arguments =
{
    emails = [
        "john.d@ambiorix.com"
    ],
    firstname = "John"
    lastname = "Doe",
    phones = [
    ],
}
```

---
> **TIP**<br>
> Using a dummy implementation is an easy way to do prototyping. Often you can re-use the same dummy implementation for all your data model RPC methods.
---

### Lab 3 - Implement add_contact

Now it is time to provide the real implementation of the `add_contact` RPC method. In the file `dm_phonebook_methods.c` you need to add the RPC method implementation, the major part is provided here. 

```C
amxd_status_t _add_contact(amxd_object_t* object,
                           UNUSED amxd_function_t* func,
                           amxc_var_t* args,
                           amxc_var_t* ret) {
    amxd_dm_t* dm = phonebook_get_dm();
    amxd_object_t* contact = amxd_object_get(object, "Contact"); 
    const char* first_name = GET_CHAR(args, "firstname");
    const char* last_name = GET_CHAR(args, "lastname");
    // TODO - fetch the phonenumber list - use GET_ARG macro
    // TODO - fetch the email list - use GET_ARG macro

    amxd_trans_t transaction;
    amxd_trans_init(&transaction);

    // select Phonebook.Contact.
    amxd_trans_select_object(&transaction, contact); 
    // add an instance - new instance is selected Phonebook.Contact.{i}.
    amxd_trans_add_inst(&transaction, 0, NULL); 
    // add parameter values to the instance creation
    amxd_trans_set_value(cstring_t, &transaction, "FirstName", first_name);
    amxd_trans_set_value(cstring_t, &transaction, "LastName", last_name);

    // TODO - create sub-objects PhoneNumber & E-Mail 

    // TODO - apply transaction and set ret variant

    amxd_trans_clean(&transaction);
    return amxd_status_ok;
}
```

All you need to do is add the instances of the `PhoneNumber` and `E-Mail` objects. 

---
> **TIP**<br>
> To add the instances of the `PhoneNumber` and `E-Mail` objects you can implement a static function that takes the transaction, object name, parameter name and values as input.
> ```
> static void add_instances(amxd_trans_t* transaction,
>                           const char* sub_object,
>                           const char* param_name,
>                           amxc_var_t* values) {
>    // select sub object
>    amxd_trans_select_pathf(transaction, ".%s", sub_object);
>
>    amxc_var_for_each(value, values) {
>        const char* v = NULL;
>
>        if(amxc_var_type_of(value) != AMXC_VAR_ID_CSTRING) {
>            continue;
>        }
>
>        v = amxc_var_constcast(cstring_t, value);
>        // add an instance - new instance is selected
>        amxd_trans_add_inst(transaction, 0, NULL);
>        // add parameter value to the instance creation
>        amxd_trans_set_value(cstring_t, transaction, param_name, v);
>        // go one up in hierarchy
>        amxd_trans_select_pathf(transaction, ".^");
>    }
>
>    // go one up in hierarchy (Contact instance)
>    amxd_trans_select_pathf(transaction, ".^");
>}
> ```

---

And also make sure that the transaction is applied. Do not forget to set the return value of the RPC method. The return type of the RPC is defined as a `boolean`, so make sure when the transaction is applied without any errors, the RPC returns `true`.

Now that the real implementation is available, the resolver instructions in the odl file can be removed.

Re-compile and re-launch the phonebook app. When done it must be possible to call the `add_contact` RPC method.

```bash
pcb_cli 'Phonebook.add_contact("John", "Doe", ["(+032) 555 28772","(+032) 555 24712"], ["johnd@ambiorix.com"])'
```

### Lab 4 - Implement save and load

Let's first implement the `save` RPC method.

The function signature for RPC methods is always the same, the `save` RPC method looks like this:

```C
amxd_status_t _save(amxd_object_t* object,
                    amxd_function_t* func,
                    amxc_var_t* args,
                    amxc_var_t* ret);
```

Add this method to the file `src/dm_phonebook_methods.c`. The only thing remaining is implement it.

When the `save` RPC method is called, a file name must be passed as an argument. As this argument is defined as `mandatory` and `strict` we are sure that it is a string and it is passed. The `filename` can be taken from the `args` by using the `GET_CHAR` macro:

```C
const char* filename = GET_CHAR(args, "filename");
```

Use function `amxo_parser_save_object` to save the data model from the `root` object. This function needs as arguments a parser context, and a pointer to the object you want to save.

The parser context can be retrieved using:

```C
amxo_parser_t* parser = phonebook_get_parser();
```

The pointer to the root object of the data model can be retrieved using:

```C
amxd_dm_t* dm = phonebook_get_dm();
amxd_object_t* root = amxd_dm_get_root(dm);
```

When the save function is implemented you should be able to call it using a client application (do not forget to remove the resolver instructions from the odl file):

```bash
pcb_cli 'Phonebook.add_contact("John", "Doe", ["(+032) 555 28772","(+032) 555 24712"], ["johnd@ambiorix.com"])'
ubus call Phonebook save '{"filename":"/tmp/saved_phonebook.odl"}'
```

If all went well, a file is created that should look like:

```
%populate {
    object 'Phonebook' {
        object 'Contact' {
            instance add(1) {
                parameter 'LastName' = "Doe";
                parameter 'Type' = "Unknown";
                parameter 'FirstName' = "John";
                object 'PhoneNumber' {
                    instance add(1) {
                        parameter 'Phone' = "(+032) 555 28772";
                    }
                    instance add(2) {
                        parameter 'Phone' = "(+032) 5552 4712";
                    }
                }
                object 'E-Mail' {
                    instance add(1) {
                        parameter 'E-Mail' = "johnd@ambiorix.com";
                    }
                }
            }
        }
    }
}
```

The only thing that remains is implement the `load` RPC method.

The function signature for RPC methods is always the same, the `load` RPC method looks like this:

```C
amxd_status_t _load(amxd_object_t* object,
                    amxd_function_t* func,
                    amxc_var_t* args,
                    amxc_var_t* ret);
```

To make sure that the data model looks exactly the same as when it was saved, first all contacts in the data model must be removed. This can be achieved in different ways. One option is to build a transaction that deletes all existing instances of the contacts. Another option is to call the protected data model `_del` RPC method, this method is available on any data model object. The `_del` method is defined as:

```
%template %instance void _del(%in uint32 index, %in string name, %in uint32 access = 1, %strict %in string rel_path);
```

This method can be called on any object and can handle search paths or wildcards in the `rel_path` argument. In our case we want to delete all contacts in the data model. If the object on which the method is called is `Phonebook` the `rel_path` argument should be set to `Contact.*`.

Invoking a data model method can be achieved by using:

```C
amxd_status_t amxd_object_invoke_function(amxd_object_t* const object,
                                          const char* func_name,
                                          amxc_var_t* const args,
                                          amxc_var_t* const ret);
```

Putting this all together you should have something like this:

```C
amxc_var_t del_args;
amxc_var_t del_ret;

amxc_var_init(&del_args);
amxc_var_init(&del_ret);
amxc_var_set_type(&del_args, AMXC_VAR_ID_HTABLE);
amxc_var_add_key(cstring_t, &del_args, "rel_path", ".Contact.*.");

rv = amxd_object_invoke_function(object, "_del", &del_args, &del_ret);
```

When no `Contact` instances are available in the data model the `_del` method will return `amxd_status_object_not_found`, which is in our case not an error.

When all contacts are deleted (or no contacts were available), the saved file can be loaded.

```C
rv = amxo_parser_parse_file(parser,
                            filename,
                            root);
```

When the `load` RPC method is implemented, restart the `phonebook` app and call the method:

```
root@3263f0f79744:/# ubus call Phonebook load '{"filename":"/tmp/saved_phonebook.odl"}'
{
        "retval": ""
}
root@3263f0f79744:/# pcb_cli Phonebook?
Phonebook
Phonebook.Contact
Phonebook.Contact.1
Phonebook.Contact.1.LastName=Doe
Phonebook.Contact.1.Type=Unknown
Phonebook.Contact.1.FirstName=John
Phonebook.Contact.1.PhoneNumber
Phonebook.Contact.1.PhoneNumber.1
Phonebook.Contact.1.PhoneNumber.1.Phone=(+032) 555 28772
Phonebook.Contact.1.PhoneNumber.2
Phonebook.Contact.1.PhoneNumber.2.Phone=(+032) 555 24712
Phonebook.Contact.1.E-Mail
Phonebook.Contact.1.E-Mail.1
Phonebook.Contact.1.E-Mail.1.E-Mail=johnd@ambiorix.com
```

### Lab 5 - Methods with output arguments

Let's add a new method to the data model to check the credentials of a user that wants to access the Phonebook data model.

```
void CheckCredentials(%strict %mandatory %in string Username,
                      %strict %mandatory %in string Password,
                      %out string Status);
```

The method takes 2 input arguments `Username` and `Password`, and 1 output argument `Status`. To keep things simple, the `Status` can have 2 values:

- `"Credentials_Good"`: in case a valid username and password are provided.
- `"Credentials_Bad"`: in case invalid credentials are provided.

The method should always return with `amxd_status_ok` unless one of the input arguments is an empty string. When either the `Username` or `Password` is empty, the method should return with `amxd_status_invalid_function_argument` and an `err_code` and `err_msg` should be provided in the output arguments.

You can start your function from this code snippet:

```C
amxd_status_t _CheckCredentials(UNUSED amxd_object_t* object,
                                UNUSED amxd_function_t* func,
                                amxc_var_t* args,
                                UNUSED amxc_var_t* ret) {
    amxd_status_t rv = amxd_status_ok;
    const char* user_txt = GETP_CHAR(args, "Username");
    const char* pass_txt = GETP_CHAR(args, "Password");
    const char* status = NULL;

    // Check the input arguments for empty strings
    // Set the err_code to USP_ERR_INVALID_COMMAND_ARGS in case one of the inputs is empty
    // and add an appropriate err_msg

    // Check if Username is "admin" and Password is "secret" using the strcmp function
    // Set the Status argument based on the provided credentials

exit:
    return rv;
}
```

Below you will find the expected output for good, bad and missing credentials:

```bash
$ sudo ubus call Phonebook CheckCredentials '{"Username": "admin", "Password": "secret"}'
{
        "retval": ""
}
{
        "Status": "Credentials_Good"
}

$ pcb_cli 'Phonebook.CheckCredentials(Username: "admin", Password: "secret")'
Phonebook.CheckCredentials(Status=Credentials_Good) done
```

```bash
$ sudo ubus call Phonebook CheckCredentials '{"Username": "admin", "Password": "unknown"}'
{
        "retval": ""
}
{
        "Status": "Credentials_Bad"
}

$ pcb_cli 'Phonebook.CheckCredentials(Username: "admin", Password: "unknown")'
Phonebook.CheckCredentials(Status=Credentials_Bad) done
```

```bash
$ sudo ubus call Phonebook CheckCredentials '{"Username": "admin", "Password": ""}'
{
        "retval": ""
}
{
        "err_code": 7027,
        "err_msg": "Failed to validate credentials"
}
Command failed: Invalid argument

$ pcb_cli 'Phonebook.CheckCredentials(Username: "admin", Password: "")'
Phonebook.CheckCredentials(err_code=7027, err_msg=Failed to validate credentials) failed
```

## References

- Tutorial: Component Structure<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/build-debug/component-structure/-/blob/main/README.md
- Example: Greeter Plug-in<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_plugin
- Online man page - dlopen<br>
https://linux.die.net/man/3/dlopen
- Online man page - dlsym<br>
https://linux.die.net/man/3/dlsym
- Wikipedia - Plug-in<br>
https://en.wikipedia.org/wiki/Plug-in_(computing)
- USP specification on Operations
https://usp.technology/specification/07-index-messages.html#sec:defined-operations-mechanism
- Predefined USP error codes in libusp
https://gitlab.com/soft.at.home/usp/libraries/libusp/-/blob/main/include/usp/usp_err_codes.h
