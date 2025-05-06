# Ambiorix Data Models - Object And Parameter Validation

[[_TOC_]]

## Introduction

In tutorial [Define And Publish](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish/-/blob/master/README.md) is explained how a simple data model can be defined using the [Object Definition Language](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md). To make such data model more useful we need to make sure that all objects and parameters are valid. 

Often not all possible values are valid, often constraints need to be applied. When we take a look at the [TR-181 Data Model](https://usp-data-models.broadband-forum.org/tr-181-2-12-0-usp.html) definition and specification, many constraints are mentioned in the descriptive text or together with the parameter types. 

Some examples from [TR-181 Data Model](https://usp-data-models.broadband-forum.org/tr-181-2-12-0-usp.html)

In the [DeviceInfo](https://usp-data-models.broadband-forum.org/tr-181-2-12-0-usp.html#D.Device:2.Device.DeviceInfo.) object most of the string parameters also contain a minimum or maximum length:

- `Manufacturer` string(64) 
- `ManufacturerOUI` string(6:6)

Often in the descriptive text next to the parameter more constrains are mentioned, as with the `ManufacturerOUI:
>Organizationally unique identifier of the device manufacturer. Represented as a six hexadecimal-digit value using all upper-case letters and including any leading zeros. Possible patterns:
>[0-9A-F]{6}
The value MUST be a valid OUI as defined in [OUI].

Constraints are often also put on objects, for an example see [DeviceInfo,VendorConfigFile.{i}.](https://usp-data-models.broadband-forum.org/tr-181-2-12-0-usp.html#D.Device:2.Device.DeviceInfo.VendorConfigFile.{i}.) object description:
> At most one entry in this table can exist with a given value for Alias, or with a given value for Name. The non-functional key parameter Alias is immutable and therefore MUST NOT change once it's been assigned.

The constraint here is that each instance must have a unique value for the parameter `Alias` and a unique value for the parameter `Name`, adding multiple instances that have the same `Alias` or the same `Name` is not allowed.

The Ambiorix framework data model library `libamxd` can help in implementing these constraints, some of these constraints and validations are already provided by the framework, others (like domain specific constraints) can be added.

## Goal

The goal of this tutorial is to explain:

- How to make sure parameter values in instance are `unique`
- How to add parameter value validation to your data model
- How to add object validation to your data model
- How to add `custom` validation functions and add them to your data model
- How to make `common custom` validation functions available to other data models.

## Prerequisites

- You finished the [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) tutorial
- You finished the [Define And Publish](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish/-/blob/main/README.md) tutoriaL
- You have a basic understanding of the data model concepts as defined by `Broadband Forum` standards
- You have a basic understanding of the Ambiorix variants
- You have basic C knowledge
- You have basic git knowledge
- You can work with docker containers

## Background Information - Data Model Actions

Before diving in object and parameter validation, first look at the Ambiorix Data Model Engine in more detail.

In this tutorial only the `validate` action on objects and parameters will be explained in detail.

It is recommended to read this part, as it provides some extra information.

### Data Model Actions 

As you already know by now a 'data model' is a hierarchical tree of objects (nodes) that can contain parameters and methods. Where the values of the parameters or objects is stored or how that data is retrieved can vary. It is possible that data is stored in a file and it can be extracted from that file. 

By default the data model engine stores all values in memory, but if you want to read or write data from another source it is possible.

The data model engine works with `action` functions, each action function performs a specific task. The available set of actions is fixed.

The available actions for objects are:

- `read`: reads some or all parameter values of an object
- `write`: changes some or all parameters values of an object
- `validate`: verifies if an object is valid
- `describe`: describes some or all parts of an object (used for introspection)
- `list`: lists parameters, functions, child objects, instance objects of an object (names only, used for introspection)
- `add-inst`: adds an instance (if possible)
- `del-inst`: checks if an instance can be deleted.
- `destroy`: clean-up action

The available actions for parameters are:

- `read`: read the value of the parameter
- `write`: change the value of the parameter
- `validate`: verifies if a new value is valid for the parameter
- `describe`: describes the parameter, its attributes, type (used for introspection)
- `destroy`: clean-up action

All the existing actions have a default implementation, which can be overridden, and in most cases you don't need to implement action functions.

Using the `Object Definition Language` it is possible to set a specific action implementation for an object action or a parameter action. Depending on the place in the data model definition (parameter body or object body) it becomes a parameter or object action. The generic odl syntax is:

```odl
on <ACTION NAME> call <FUNCTION> [<DATA>];
```

Note that optionally some extra data can be added as well, when data is added, the odl parser will create a variant and stores the data in that variant. When the action is invoked by the data model engine, this variant containing the data will be passed to the action implementation as private data.

### Example

The set of action names is fixed and defined by the data model engine. To change the action implementation function attached to an object or parameter, you need add an `action` line in the body of the object or parameter.

Odl example:

```odl
%define {
    object MyObject {
        string MySecret = "This is secret" {
            on action read call hide_value;
        }
    }
}
```

In this example the default `read` implementation of parameter `MyObject.MySecret` is overridden with the function `hide_value`. This special read action implementation is provided by the Ambiorix data model library (libamxd) and will always return an empty string when someone tries to get the value.

You can quickly verify this, by creating a file `test.odl` somewhere in your container, copy&paste the example object definition in that file and launch this simple data model:

```bash
amxrt ./test.odl
```

When using the `pcb_cli` or the `ubus` tool to read the parameter value, you will always see an empty string.

```bash
$ pcb_cli
Copyright (c) 2010 SoftAtHome

Connected to pcb://ipc:{/var/run/pcb_sys}
> MyObject?
MyObject
MyObject.MySecret=
```

### Action Implementations

All action implementation must match this prototype:

```C
amxd_status_t _<FUNCTION NAME>(amxd_object_t* object, amxd_param_t* param, amxd_action_t reason, const amxc_var_t* const args, amxc_var_t* const retval, void* priv);
```

- `object` - the pointer to the object on which the action is invoked, for parameter actions this is the object containing the parameter
- `param` - the pointer to the parameter on which the action is invoked, for object actions this will be `NULL`
- `reason` - this is the action identifier, also known as the `reason` the function is called. It is possible (but not recommended) to implement an action implementation that can handle multiple actions.
- `args` - a variant containing extra data (arguments) specific for the actions, can be NULL
- `retval` - a variant where the result of the action can be stored, this can be NULL. What must be stored in the variant depends on the `action`
- `priv` - pointer to some private action data. When an action was added to an object or parameter using the `Object Definition Language` and data is defined behind this will be a pointer to a variant containing the data. When adding an action from code, you can use any pointer as private data.

### More Examples

#### cpu-info

If you are interested in a more extended example that overrides default actions see example [cpu-info](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/cpu-info).

That example uses the files `/proc/cpuinfo` and `/proc/stat` to fetch information about the cpu's of the system, also take a look at the data model definition [object CPU](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/cpu-info/-/blob/main/odl/cpu_info_definition.odl#L14) to see all the action overrides done for that object.

#### Snippet - custom parameter validation - only accept odd numers

This snippet shows a custom parameter validation action that verifies if an odd number is given.

```
amxd_status_t _is_odd(amxd_object_t* object,
                      amxd_param_t* param,
                      amxd_action_t reason,
                      const amxc_var_t* const args,
                      amxc_var_t* const retval,
                      void* priv) {
    amxd_status_t status = amxd_status_unknown_error;
    int64_t number = 0;

    if(reason != action_param_validate) {
        status = amxd_status_function_not_implemented;
        goto exit;
    }
    // checks if value can be converted to parameter type.
    status = amxd_action_param_validate(object, param, reason, args, retval, priv);
    if (status != amxd_status_ok) {
        goto exit;
    }
    
    // cast to an integer and check if it is odd
    number = amxc_var_dyncast(int64_t, args);
    if ((number % 2) != 0) {
        status = amxd_status_ok;
    } else {
        status = amxd_status_invalid_value;
    }


exit:
    return status;
}
```

## Object Validation And Constraints

### Creating Instances With Unique Parameter Values

The more complex constraints are the constraints that span multiple objects in the data model, such as
> At most one instance in this multi-instance object can exist with a given value for a certain parameter

Using the Ambiorix data model implementation such constraints can easily be defined.

#### Unique Key Parameters

The `Object Definition Language` provides keywords that makes it possible to set `special` attributes to objects, parameters, functions. 

There are two attributes for parameters that can be used to indicate that the value of the  parameter must be treated as an unique key, these attributes are:

- `%unique`
- `%key`

These attributes only have effect when used in multi-instance objects, when adding them to parameters of a singelton (aka static) object, they will be ignored.

Using the attribute `%unique` without specifying `%key` also has no effect, on the other hand the attribute `%key` can be used without attribute `%unique`.

Example:
```odl
%define {
    object DeviceInfo {
        object VendorConfigFile[] {
            %unique %key string Alias;
            %unique %key string Name;
            string Version;

            ...

        }
    }
}
```

By adding the parameter attributes `%unique` and `%key` to a parameter, it will not be possible to create two instances that have the same value for these parameters.

By default parameters defined with the `%key` attribute will be immutable, they can only be set once and can not be changed anymore by external sources (like other applications). The values of these key parameters must be provided at creation time, with one exception the `Alias` parameter.

#### The Alias Parameter

In [TR-181 Data Model](https://usp-data-models.broadband-forum.org/tr-181-2-12-0-usp.html) all multi-instance objects have a parameter named `Alias`. This parameter is a non functional parameter and is only used to identify the instances by name.

The Ambiorix data model implementation automatically can detect and handle `Alias` parameters if the following conditions are met:

1. It must be named `Alias` (case sensitive) and preferable of the `string` type.
1. It must have the `%unique` and `%key` attributes.

When creating an instance for a multi-instance object that contains an `Alias` parameter and no value is provided for the  `Alias` parameter, a value will be chosen. The value chosen will always start with `cpe-` and will end with `-\<index\>`, index is the instance identifier of the instance. Between this pre- and suffix the name of the multi-instance object is set.

The `Alias` parameter is immutable and can not be changed during the lifetime of the object that contains this parameter.

#### Compound Key Parameters

In some cases it is possible that the same combination of values must be unique. In that case it is possible that multiple-instances of the same multi-instance object have the same values, but the combination of values is unique.

The multi-instance object [Nat.PortMapping.{i}.](https://usp-data-models.broadband-forum.org/tr-181-2-12-0-usp.html#D.Device:2.Device.NAT.PortMapping.{i}.) is such an example:
> ...or with all the same values for RemoteHost, ExternalPort and Protocol.

Such a constrain can also be defined in the odl file.

Example:

```odl
%define {
    object Nat {
        object PortMapping[] {
            %unique %key string Alias;
            %key string RemoteHost;
            %key uint32 ExternalPort;
            %key string Protocol;
            bool Enable;

            ...

        }
    }
}
```

In this case the `Alias` parameter defines the instance uinquelly and the combination of `RemoteHost`, `ExternalPort` and `Protocol` is also unique.

### Validating An Object

Sometimes you need to write some custom validation logic, for objects this logic is often `domain` specific and can not be provided by the Ambiorix framework.

#### Data Model Actions

To be able to add `domain` specific logic the data model implementation provides hooks. One set of hooks that are provided are the data model `action` hooks. Actions are a central part of the data model, see [Data Model Actions](#data-model-actions) in this tutorial for more information. 

It is possible to add multiple implementations of each action to an object, in some cases like `validate` it can be useful to have multiple validate actions, in other cases, like `read` and `write` actions it is less useful to have multiple implementations on the same object.

Example of an object validation action:

From example [tr181-localagent-threshold](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/localagent_threshold/-/blob/main/odl/la_threshold_definition.odl#L6)

```odl
%define {
    object LocalAgent {
        %read-only csv_string SupportedThresholdOperator = "Rise,Fall,Eq,NotEq";

        object Threshold[] {
            on action validate call threshold_instance_is_valid;

            %unique %key string Alias;
            bool Enable = false;
            string OperatingMode {
                default "Normal";
                on action validate call check_enum ["Normal", "Single"];
            }

            ....
        }
    }
}
```

#### Writing An Object Validation Action Function

First of all some good practices when writing an `Action` implementation in general

1. Always check the `reason` code, to be sure it matches the intended implementation. Any action implementation can be used as an implementation for any action. When the reason provided is not matching the intended implementation, return as error code `amxd_status_function_not_implemented`, this will make sure the default implementation is executed.
1. Although it is possible to implement an `action` that can be used for multiple actions, it is recommended to implement a `specialized` function that only handle a specific action.
1. Make sure that you do not invoke other `actions` from within your action implementation, invoking`read` actions is an exception on this rule.
1. Only do what the `action` is intended for. Example: changing parameter values in a validation action should not be done. (Separation of responsibilities)

An example of an object validation action can be found in the example [localagent_threshold](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/localagent_threshold/-/blob/main/src/dm_threshold_actions.c#L68).

```C
amxd_status_t _threshold_instance_is_valid(amxd_object_t* object,
                                           UNUSED amxd_param_t* param,
                                           amxd_action_t reason,
                                           UNUSED const amxc_var_t* const args,
                                           UNUSED amxc_var_t* const retval,
                                           UNUSED void* priv) {
    amxd_status_t status = amxd_status_unknown_error;
    amxc_var_t params;

    bool enabled = false;
    const char* ref_path = NULL;
    const char* ref_param = NULL;

    amxc_var_init(&params);

    if(reason != action_object_validate) {
        status = amxd_status_function_not_implemented;
        goto exit;
    }

    amxd_object_get_params(object, &params, amxd_dm_access_protected);

    enabled = GET_BOOL(&params, "Enable");
    ref_path = GET_CHAR(&params, "ReferencePath");
    ref_param = GET_CHAR(&params, "ThresholdParam");

    if(enabled) {
        if((ref_path == NULL) || (*ref_path == 0)) {
            status = amxd_status_invalid_value;
            goto exit;
        }
        if((ref_param == NULL) || (*ref_param == 0)) {
            status = amxd_status_invalid_value;
            goto exit;
        }
    }

    status = amxd_status_ok;

exit:
    amxc_var_clean(&params);
    return status;
}
```

When implementing an object validation action function not all provided arguments need to be used, it is even possible that some of the arguments are `NULL` pointers.

Typically for an object validation function these arguments are valid and provided:

- object - pointer to the data model object that needs to be validated
- reason - the reason code, this is normally `action_object_validate`, but could be any other reason code if your validation action implementation is used in the wrong context.
- priv - optionally this is a pointer to a variant that contains extra data that can be used in the validation process. Typically this data is defined in the data model definition file (odl).

An object validation function must return:

- `amxd_status_ok` when the object is valid
- `amxd_status_function_not_implemented` when the reason code is not matching the intended implementation
- any other error code when the object is invalid.

## Parameter Validation And Constraints

Parameter validation is most of the time a lot easier, and many of the common constraints are already implemented in the Ambiorix data model library (libamxd) and can be used as is.

Custom or domain specific logic can be applied on parameters as well, using the same mechanism as with objects: `action`. A set of actions for parameters is available, all parameter actions have a default implementation and in most cases these implementations are sufficient, see [Data Model Actions](#data-model-actions) in this tutorial for more information. 

It is possible to add multiple implementations of each action to a parameter, in some cases like `validate` it can be useful to have multiple validate actions, in other cases, like `read` and `write` actions it is less useful to have multiple implementations on the same parameter.

### Available Validation Action Functions

Besides the default action implementation of the parameter validate `action`, other parameter validate actions are available and provided by the data model library (libamxd).

The set of parameter validate actions that is available are:

- `check_minimum` - checks if an integer is a at least a given value
- `check_minimum_length` - checks if a string is a at least of a minimum length
- `check_maximum` - checks if an integer is not higher then a given value
- `check_maximum_length` - checks if a string is not longer then a given length
- `check_range` - checks if an integer is within a specific range 
- `check_enum` - checks if the value is one of the given values
- `check_is_in` - checks if the value is one of the values specified in a csv (comma separated values) parameter in the data model (must be in the same process)

You can find the implementation of these parameter validation actions in the file [amxd_action_param_validate.c](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd/-/blob/master/src/amxd_action_param_validate.c) and the declaration in [amxd_action.h](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd/-/blob/main/include/amxd/amxd_action.h)

All these validation functions need an argument, the argument value can be added to the data model definition (odl file) as well and must be put behind the function name.

It is possible to apply zero, one or more validation actions on one single parameter. When multiple validation functions are applied, a new value is only accepted when all validation functions succeed. 

It is very important that when combining multiple constrains that there are still valid values possible. 

Make sure that in the data model definition a valid value is set, otherwise the data model definition can not be loaded.

A validation action can only be added in a parameter definition body.

Example [DeviceInfo](https://usp-data-models.broadband-forum.org/tr-181-2-12-0-usp.html#D.Device:2.Device.DeviceInfo.):

```odl
%define {
    object DeviceInfo {
        string Manufacturer {
            on action validate call check_maximum_length 64;
        }
        string ManufacturerOUI = "000000" {
            on action validate call check_maximum_length 6;
            on action validate call check_minimum_length 6;
        }
    }
}
```

Example [LocalAgent.Threshold.{i}.](https://usp-data-models.broadband-forum.org/tr-181-2-13-0-usp-diffs.html#D.Device:2.Device.LocalAgent.Threshold.{i}.) (from TR-181 2.13.0):

```odl
%define {
    object LocalAgent {
        %read-only csv_string SupportedThresholdOperator = "Rise,Fall,Eq,NotEq";
        object Threshold[] {
            %unique %key string Alias;
            bool Enable = false;
            string OperatingMode = "Default" {
                on action validate call check_enum ["Normal", "Single"];
            }
            string ThresholdOperator = "Rise" {
               on action validate call check_is_in "LocalAgent.SupportedThresholdOperator";
           }
        }
    }
}
```

### Read Only Parameters

Making a parameter read-only can be considered as a constraint, it will make it impossible for external sources (other applications, data model clients), to change the value of the parameter. 

In the `Object Definition Language` an attribute is available to indicate that a parameter can not be changed by external sources, this attribute is `%read-only`.

Example:
```odl
%define {
    object DeviceInfo {
        object VendorConfigFile[] {
            %unique %key string Alias;
            %unique %key string Name;
            %read-only string Version;

            ...

        }
    }
}
```

--- 
>**NOTE**
>
>Never add the `%read-only` attribute on key parameters. This will make it impossible for external sources to create a new instance, as they can not set the initial values for these key parameters.
>
>The Ambiorix data model implementation will make the key parameters read-only after the instance has been created and the initial values are set. This is the default behavior and most cases the correct behavior.

---

When a parameter is read-only it is still possible for the data model owner to change the value using the data model management (libamxd) API. How to use the API is handled in another tutorial.

### Custom Parameter Validation Functions

To implement a parameter validate action function, you need to implement a function that is matching this signature:

```C
typedef amxd_status_t (* amxd_action_fn_t) (amxd_object_t* const object,
                                            amxd_param_t* const param,
                                            amxd_action_t reason,
                                            const amxc_var_t* const args,
                                            amxc_var_t* const retval,
                                            void* priv);  
```

Note that this is exactly the same function signature as an object validation function. All object or parameter actions must be implemented using this function signature.

Example parameter validation actions:

Check if the new value is an `even number`

```C
amxd_status_t _is_even(amxd_object_t* object,
                       amxd_param_t* param,
                       amxd_action_t reason,
                       const amxc_var_t* const args,
                       amxc_var_t* const retval,
                       void* priv) {
    amxd_status_t status = amxd_status_unknown_error;
    int64_t number = 0;

    if(reason != action_param_validate) {
        status = amxd_status_function_not_implemented;
        goto exit;
    }

    // checks if value can be converted to parameter type.
    status = amxd_action_param_validate(object, param, reason, args, retval, priv);
    if (status != amxd_status_ok) {
        goto exit;
    }
    
    // 
    number = amxc_var_dyncast(int64_t, args);
    if ((number % 2) == 0) {
        status = amxd_status_ok;
    } else {
        status = amxd_status_invalid_value;
    }


exit:
    return status;
}
```

Check if a string is matching a `regular expression`:
From data model [extension module](https://gitlab.com/prpl-foundation/components/core/modules/mod_dmext/-/blob/main/src/dmext_param_regexp_validate.c#L123).

```C
amxd_status_t _matches_regexp(amxd_object_t* object,
                              amxd_param_t* param,
                              amxd_action_t reason,
                              const amxc_var_t* const args,
                              amxc_var_t* const retval,
                              void* priv) {
    amxd_status_t status = amxd_status_unknown_error;
    amxc_var_t* data = (amxc_var_t*) priv;
    const char* regexp_str = NULL;
    char* value_str = NULL;

    when_null(param, exit);
    when_true_status(reason != action_param_validate, exit,
                     status = amxd_status_function_not_implemented);
    when_null_status(data, exit, status = amxd_status_invalid_arg);
    when_true_status(!dmext_var_is_string(data), exit,
                     status = amxd_status_invalid_arg);
    status =
        amxd_action_param_validate(object, param, reason, args, retval, priv);
    when_failed(status, exit);

    regexp_str = amxc_var_constcast(cstring_t, data);
    when_str_empty_status(regexp_str, exit, status = amxd_status_invalid_arg);

    value_str = amxc_var_dyncast(cstring_t, args);
    when_str_empty_status(value_str, exit, status = amxd_status_invalid_value);

    status = dmext_validate_regexp(regexp_str, value_str);

exit:
    free(value_str);
    return status;
}
```

Both validation functions can be used in an odl file:

```odl
%define {
    object MyObject {
        string ManufacturerOUI = "000000" {
            on action validate call matches_regexp "[0-9A-F]{6}";
        }

        uint32 Number = 20 {
            on action validate call is_even;
        }
    }
}
```

---

**NOTE**

If you are not familiar with regular expressions you can read the [Regular expression](https://en.wikipedia.org/wiki/Regular_expression) article on wikipedia. 

In the above given example from [extension module](https://gitlab.com/prpl-foundation/components/core/modules/mod_dmext/-/blob/main/src/dmext_param_regexp_validate.c#L123), [POSIX extended](https://en.wikipedia.org/wiki/Regular_expression#POSIX_basic_and_extended) regular expressions are used. 

---

When implementing a parameter validation action function not all provided arguments needs to be used, it is even possible that some of the arguments are `NULL` pointers.

Typically for an object validation function these arguments are valid and provided:

- object - pointer to the data model object that contains the parameter for which a new value needs to be validated.
- param - pointer to the data model parameter for which a new value needs to be validated.
- reason - the reason code, this is normally `action_parameter_validate`, but could be any other reason code if your validation action implementation is used in the wrong context.
- args - is a variant containing the new value that needs to be validated.
- priv - optionally this contains a pointer to a variant that contains extra data that can be used in the validation process. Typically this data is defined in the data model definition file (odl).

A parameter validation function must return

- `amxd_status_ok` when the new value is valid
- `amxd_status_function_not_implemented` when the reason code is not matching the intended implementation
- any other error code when the new value is invalid.

Note that the parameter still contains the `old` value. The `new` value is provided in the `args` argument.

## Data model actions versus data model events

A common mistake people make when they want to use data model actions is that they try to do too much. A read action can for example be used to read something from a non-standard location or to hide a hidden parameter value. A write action can for example be used to write something to a non-standard location or to update the private data of an object/parameter. If something must happen as a result of a write action, events should be used instead. In many situations, multiple objects or parameters are related to each other. If one of them changes, the other one should be updated as well. This can be done with data model transactions, but you are not allowed to use them in the middle of an action callback. Events must be used instead.

## Data Model Modules

Often `domain` specific logic, like parameter validate actions, could be re-used in different applications. Copying and pasting the code in your application could be a solution, but not a good one. A better solution would to implement it once, and use the same code or binary wherever or whenever you need it.

With the `Object Definition Language` it is possible to import a `module` that contains functions that you can re-use in your data model definition.

An example of such module is [mod_dmext](https://gitlab.com/prpl-foundation/components/core/modules/mod_dmext). This module provides common TR-181 parameter validation functions which you can use in your data model. How to build and install this module is explained in the [README.md](https://gitlab.com/prpl-foundation/components/core/modules/mod_dmext/-/blob/main/README.md) of that module.

Using the module in your data model definition is easy, use keyword `import` to import the shared object. After that you can use any of the functions provided in that module. All available functions you can use are documented in the [README.md](https://gitlab.com/prpl-foundation/components/core/modules/mod_dmext/-/blob/main/README.md) of that module.


```odl
import "mod-dmext.so";

%define {
    object DeviceInfo {
        string ManufacturerOUI = "000000" {
            on action validate call matches_regexp "[0-9A-F]{6}";
        }
    }
}
```

When the odl parser encounters the `import` keyword, it will try to load the mentioned file using [dlopen](https://linux.die.net/man/3/dlopen). When the `shared object` is loaded the symbols in that shared object can be used.

### Function Resolving

In the `Object Definition Language` action implementations can be added to objects in the object body or to parameters in the parameter body. The function name is always mentioned behind the keyword `call` (See [Object Definition Language](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md) for more information). 

The `import resolver` will use that function name prefixed with a "_" and uses [dlsym](https://linux.die.net/man/3/dlsym) to fetch the function pointer from any loaded `module` shared object. When the symbol is found it will be used as an `action` implementation.

The odl parser can use other function resolvers as well, consult the odl parser's documentation to check which function resolvers are available.

## Practical Labs

By now you should known the drill. All practical labs are done in the `Ambiorix Debug & Development` container and in terminals opened in that container, the repository containing this tutorial must be cloned in your workspace directory.

### Lab 1 - Add Parameter Validation To The `Phonebook` Data Model

In [Define And Publish - Lab 1](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish#lab1-complete-phonebook-data-model) you have completed the `phonebook` data model. 

The string parameter `E-Mail` can currently contain any string, but it should be a valid e-mail address. Using the regular expression "^[[:graph:]]+@[[:graph:]]+\.[[:graph:]]+$" some basic validation can be done to check that the string is an e-mail address. More complete regular expressions exist to do full e-mail address validation.

Also the string parameter containing a `PhoneNumber` should be validated, alphabetic characters are not valid in a phone number, "^\(\\+[0-9]{2,3}\)?([ 0-9]){8,16}$" could be used as regular expression to validate the phone number.

It is up to you to add two parameter validation actions to the correct parameters. There is no need to implement any validation function, you can use the `mod_dmext` module.

Clone `mod_dmext`:

---

>**NOTE**
>
>The following set of commands can be executes on your local machine or from a container terminal, if you configured the container correctly.

---

```bash
cd ~/workspace
mkdir -p amx/modules
cd amx/modules
git clone git@gitlab.com:prpl-foundation/components/core/modules/mod_dmext.git
```

Build `mod_dmext`:

--- 

>**NOTE**
>
>Building must be done from within the container

---

```bash
cd ~/workspace/amx/modules/mod_dmext
make
sudo -E make install
```

After adding the validation actions to the odl file (phonebook.odl in labs/lab1), launch the phonebook using `amxrt`

```bash
cd ~/workspace/ambiorix/tutorials/datamodels/server/validation/labs/lab1
amxrt -D phonebook.odl
```

Now add a contact with an e-mail and a phonebumber, you can use `pcb_cli` or `ubus` tool to add contacts, phonenumbers and e-mail addresses.

Example using pcb_cli:

```bash
$ pcb_cli
Copyright (c) 2010 SoftAtHome

Connected to pcb://ipc:{/var/run/pcb_sys}
> Phonebook.Contact.+
Phonebook.Contact.1
Phonebook.Contact.1.LastName=
Phonebook.Contact.1.FirstName=
> Phonebook.Contact.1.E-Mail.+
Phonebook.Contact.1.E-Mail.1
Phonebook.Contact.1.E-Mail.1.E-Mail=
> Phonebook.Contact.1.PhoneNumber.+
Phonebook.Contact.1.PhoneNumber.1
Phonebook.Contact.1.PhoneNumber.1.Phone=
> Phonebook.Contact.1?
Phonebook.Contact.1
Phonebook.Contact.1.LastName=
Phonebook.Contact.1.FirstName=
Phonebook.Contact.1.PhoneNumber
Phonebook.Contact.1.PhoneNumber.1
Phonebook.Contact.1.PhoneNumber.1.Phone=
Phonebook.Contact.1.E-Mail
Phonebook.Contact.1.E-Mail.1
Phonebook.Contact.1.E-Mail.1.E-Mail=
> Phonebook.Contact.1.PhoneNumber.1.Phone="(+032) 555 552 471"
Phonebook.Contact.1.PhoneNumber.1.Phone=(+032) 555 552 471
> Phonebook.Contact.1.PhoneNumber.1.Phone="This is not a valid phone number"
0x0003001E Invalid parameter value: Phonebook.Contact.1.PhoneNumber.1
> Phonebook.Contact.1.E-Mail.1.E-Mail="This is not a valid e-mail"
0x0003001E Invalid parameter value: Phonebook.Contact.1.E-Mail.1
> Phonebook.Contact.1.E-Mail.1.E-Mail="john.d@ambiorix.com"
Phonebook.Contact.1.E-Mail.1.E-Mail=john.d@ambiorix.com
>
```

### Lab 2 - TR-181 EthernetLink:1 Profile

In [Define And Publish - Lab 2](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish#lab2-tr-181-ethernetlink1-profile) you created a data model based on the `TR-181 EthernetLink:1 Profile`.

For this lab you also need the data model extension module.

In this lab there is some code to implement the constraint:
>  At most one enabled entry in this table can exist with a given value for MACAddress.

This constraint is defined in the TR-181 description of object `Device.Ethernet.Link.{i}.`. This can only be verified using an object validation action, as it spans over multiple parameters of the instance object (`Enabled` and `MACAddress`) and spans over all instances of `Ethernet.Link.{i}.` multi-instance object.

The code available implements this constrains.

The code can be compiled with:

```bash
cd ~/workspace/ambiorix/tutorials/datamodels/server/validation/labs/lab2
make
make install
```

In the provided odl file the function is already added to the object as a validation aciton:

```odl
import "mod-dmext.so";
import "ethernetlink.so";
 
%define {
    object Ethernet {
        uint32 LinkNumberOfEntries;
        object Link[] {
            on action validate call link_instance_is_valid;

            bool Enable;
            string Status = "Down" {
                // TODO: add validation action
            }
            %unique %key string Alias { // TODO: Make it a real "Alias"
                // TODO: add validation action
            }
            uint32 LastChange;
            csv_string LowerLayers = "" {
                on action validate call check_maximum_length 1024;
            }
            string MACAddress {
                // TODO: Add validation action, make sure it is a valid MACAddress, see mod_dmext module
            }
            object Stats {
                uint64 BytesSent;
                uint64 BytesReceived;
                uint64 PacketsSent;
                uint64 PacketsReceived;
                uint32 ErrorsSent;
                uint32 ErrorsReceived;
                uint64 UnicastPacketsSent;
                uint64 UnicastPacketsReceived;
                uint32 DiscardPacketsSent;
                uint32 DiscardPacketsReceived;
                uint64 MulticastPacketsSent;
                uint64 MulticastPacketsReceived;
                uint64 BroadcastPacketsSent;
                uint64 BroadcastPacketsReceived;
                uint32 UnknownProtoPacketsReceived;
            }
        }
    }
}
```

Now complete the data model, add validation actions where needed. Make sure that all parameters are correctly defined, also take care of the `key` parameter `Alias`, for now don't make any of the parameters `read-only`.

After completing the odl file, launch the data model using `amxrt`

```bash
cd ~/workspace/ambiorix/tutorials/datamodels/server/validation/labs/lab2
amxrt ethernetlink.odl
```

Now verify with `pcb_cli` if you can add a `Ethernet.Link` instance. 

```bash
$ pcb_cli
Copyright (c) 2010 SoftAtHome

Connected to pcb://ipc:{/var/run/pcb_sys}
> Ethernet.Link+{MACAddress:"AA:BB:CC:01:02:03"}     
Ethernet.Link.cpe-Link-1.MACAddress=AA:BB:CC:01:02:03                                      
> Ethernet.Link+{MACAddress:"AA:BB:CC:01:02:03"}     
Ethernet.Link.cpe-Link-2.MACAddress=AA:BB:CC:01:02:03       
```

Enable the two instances, the second you enable should fail, as both `Link` instances have the same `MACAddress`.

```bash
> Ethernet.Link.cpe-Link-1.Enable=true
Ethernet.Link.cpe-Link-1.Enable=1
> Ethernet.Link.cpe-Link-2.Enable=true
0x0003001E Invalid parameter value: Ethernet.Link.cpe-Link-2
```

But when you set ` Ethernet.Link.cpe-Link-1.Enable` to false, you should be able to set `Ethernet.Link.cpe-Link-2.Enable` to true.

The `Alias` parameter should be automatically filled, as no value has been provided. It will not be possible to create another instance with the same `Alias` value (if you defined the Alias correct in the odl).

```bash
$ pcb_cli
Copyright (c) 2010 SoftAtHome

Connected to pcb://ipc:{/var/run/pcb_sys}
> Ethernet.Link+{Alias:"cpe-Link-1"}
0x0003001E Invalid parameter value: Ethernet.Link
```

Using any other value for the `Alias`, that is not yet in use, will be valid.

```bash
> Ethernet.Link+{Alias:"my-link-object"}
Ethernet.Link.my-link-object.Alias=my-link-object
> Ethernet.Link?
Ethernet.Link
Ethernet.Link.cpe-Link-1
Ethernet.Link.cpe-Link-1.LastChange=0
Ethernet.Link.cpe-Link-1.Enable=0
Ethernet.Link.cpe-Link-1.Status=Down
Ethernet.Link.cpe-Link-1.Alias=cpe-Link-1
Ethernet.Link.cpe-Link-1.LowerLayers=
Ethernet.Link.cpe-Link-1.MACAddress=
Ethernet.Link.cpe-Link-1.Stats
Ethernet.Link.cpe-Link-1.Stats.MulticastPacketsSent=0
Ethernet.Link.cpe-Link-1.Stats.ErrorsSent=0
Ethernet.Link.cpe-Link-1.Stats.BroadcastPacketsSent=0
Ethernet.Link.cpe-Link-1.Stats.BytesSent=0
Ethernet.Link.cpe-Link-1.Stats.PacketsSent=0
Ethernet.Link.cpe-Link-1.Stats.BytesReceived=0
Ethernet.Link.cpe-Link-1.Stats.DiscardPacketsReceived=0
Ethernet.Link.cpe-Link-1.Stats.ErrorsReceived=0
Ethernet.Link.cpe-Link-1.Stats.MulticastPacketsReceived=0
Ethernet.Link.cpe-Link-1.Stats.UnknownProtoPacketsReceived=0
Ethernet.Link.cpe-Link-1.Stats.UnicastPacketsSent=0
Ethernet.Link.cpe-Link-1.Stats.UnicastPacketsReceived=0
Ethernet.Link.cpe-Link-1.Stats.PacketsReceived=0
Ethernet.Link.cpe-Link-1.Stats.DiscardPacketsSent=0
Ethernet.Link.cpe-Link-1.Stats.BroadcastPacketsReceived=0
Ethernet.Link.my-link-object
Ethernet.Link.my-link-object.LastChange=0
Ethernet.Link.my-link-object.Enable=0
Ethernet.Link.my-link-object.Status=Down
Ethernet.Link.my-link-object.Alias=my-link-object
Ethernet.Link.my-link-object.LowerLayers=
Ethernet.Link.my-link-object.MACAddress=
Ethernet.Link.my-link-object.Stats
Ethernet.Link.my-link-object.Stats.MulticastPacketsSent=0
Ethernet.Link.my-link-object.Stats.ErrorsSent=0
Ethernet.Link.my-link-object.Stats.BroadcastPacketsSent=0
Ethernet.Link.my-link-object.Stats.BytesSent=0
Ethernet.Link.my-link-object.Stats.PacketsSent=0
Ethernet.Link.my-link-object.Stats.BytesReceived=0
Ethernet.Link.my-link-object.Stats.DiscardPacketsReceived=0
Ethernet.Link.my-link-object.Stats.ErrorsReceived=0
Ethernet.Link.my-link-object.Stats.MulticastPacketsReceived=0
Ethernet.Link.my-link-object.Stats.UnknownProtoPacketsReceived=0
Ethernet.Link.my-link-object.Stats.UnicastPacketsSent=0
Ethernet.Link.my-link-object.Stats.UnicastPacketsReceived=0
Ethernet.Link.my-link-object.Stats.PacketsReceived=0
Ethernet.Link.my-link-object.Stats.DiscardPacketsSent=0
Ethernet.Link.my-link-object.Stats.BroadcastPacketsReceived=0
```

Note that the Alias value is used in the object path.

## References

- Ambiorix Data Model Server - Define And Publish Tutorial<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish/-/blob/main/README.md
- Object Definition Language<br>
https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md
- Broadband Forum TR-181 2.12.0 (usp) Data Model<br>
https://usp-data-models.broadband-forum.org/tr-181-2-12-0-usp.html
- Broadband Forum TR-181 2.13.0 (usp) Data Model (changes)<br>
https://usp-data-models.broadband-forum.org/tr-181-2-13-0-usp-diffs.html
- (TR-181) Data Model Extension Module<br>
https://gitlab.com/prpl-foundation/components/core/modules/mod_dmext/
- Linux On-line man page - dlsym<br>
https://linux.die.net/man/3/dlsym
- Linux On-line man page - dlopen<br>
https://linux.die.net/man/3/dlopen
- Define And Publish - Lab 1<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish#lab1-complete-phonebook-data-model
- Define And Publish - Lab 2<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish#lab2-tr-181-ethernetlink1-profile
- Example cpu-info<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/cpu-info)
- Wikipedia - Regular expression<br>
https://en.wikipedia.org/wiki/Regular_expression
