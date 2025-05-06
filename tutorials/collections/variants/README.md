# Variants

Estimated Time needed: 60 - 90 minutes.

[[_TOC_]]

## Introduction

In `C` programming language all data types are always `fixed` types which can not change at run-time. This makes it hard to define functions or callbacks of which some of the arguments could be a different type depending on the situation. Typically in `C` programming this is solved by using `void *` and `casting`. The disadvantage of this technique is that you lose type information, which could lead to unwanted side effects.

Many other programming language have a `variant` type, this is a structure that can hold any type of data, without losing the data type information.

From [wikipedia - Variant Type](https://en.wikipedia.org/wiki/Variant_type)

> Variant is a data type in certain programming languages, particularly Visual Basic, OCaml, Delphi and C++ when using the Component Object Model.

In `C` programming language the same functionality can be achieved by using a `tagged union`.

From [wikipedia - Tagged Union](https://en.wikipedia.org/wiki/Tagged_union)

> In computer science, a tagged union, also called a variant, variant record, choice type, discriminated union, disjoint union, sum type or coproduct, is a data structure used to hold a value that could take on several different, but fixed, types. Only one of the types can be in use at any one time, and a tag field explicitly indicates which one is in use. It can be thought of as a type that has several "cases", each of which should be handled correctly when that type is manipulated. This is critical in defining recursive datatypes, in which some component of a value may have the same type as the value itself, for example in defining a type for representing trees, where it is necessary to distinguish multi-node subtrees and leafs. Like ordinary unions, tagged unions can save storage by overlapping storage areas for each type, since only one is in use at a time.

The `Ambiorix variant data container` is a tagged union and can contain many different types, each variant can hold only one data type at a given point in time.

## Goal

The goal of this tutorial is:
- to learn how to use the variant API
- where to find the API documentation

It is not the intention of this tutorial to explain the Ambiorix API's in depth, it will provide you links and pointers that will help you to get more information, and of course all sources are available in gitlab.

## Prerequisite

- You finished the [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) tutorial
- Basic knowledge of C
- Basic knowledge of git

## Variants

### API Documentation

The full variant API - structures, macros and functions - is documented. <br>

The documentation can be consulted:

- In the header file [amxc_variant.h](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc/-/blob/main/include/amxc/amxc_variant.h)
- As a [web page](https://soft.at.home.gitlab.io/ambiorix/libraries/libamxc/doxygen) 

### Supported Variant Types

The `Ambiorix variant` by default supports a set of data types, which can be separated into `primitive` and `composite` data types.

The default supported `primitive` data types are:

- null - `void` or `NULL`
- string - `char *`
- signed 8 bit integer - `int8_t`
- signed 16 bit integer - `int16_t`
- signed 32 bit integer - `int32_t`
- signed 64 bit integer - `int64_t`
- unsigned 8 bit integer - `uint8_t`
- unsigned 16 bit integer - `uint16_t`
- unsigned 32 bit integer - `uint32_t`
- unsigned 64 bit integer - `uint64_t`
- floating point - `float`
- double - `double`
- file descriptor `int` 
- timestamp - `amxc_ts_t`
- boolean - `bool`

A `composite` data type is one that contains many variants, like a linked list of variants or a table containing key-value pairs where each value is a variant.

The default supported `composite` data types are:

- list - `amxc_llist_t` - contains a linked list of variants
- htable - `amxc_htable_t` - contains a hash table of variants, each item in the hash table has a key and a value. The value is a variant.

---

> **NOTE ABOUT STRING TYPES**
> 
> Some special string types are supported as well: these are just strings, but as the variant type is different, it contains extra information. These types are:<br>
> - csv_string - contains a string of `Comma Separated Values`
> - ssv_string - contains a string of `Space Separated Values`

---

---

> **NOTE ABOUT FILE DESCRIPTOR TYPES**
>
> A file descriptor in the Linux system is represented as an integer. When setting a file descriptor as a variant type, the variant implementation is verifying that the given integer is a valid file descriptor.

---

---

> **NOTE ABOUT TIMESTAMP TYPES**
> 
> Timestamps are not a standard `C` type and are implemented in this library as a structure. The timestamp structure is considered a `primitive` as it does not contain multiple variants.

---

This set of `variant types` is the default set, it is possible to add your own `custom` types. How a custom type can be added is not handled in this document.

###  Using Variants

#### Construct and Destruct a Variant

As with all `C` variables, you can define variants on the stack or allocate memory from the heap.

To define a local variant on the stack and initialize it:

```C
amxc_var_t MyVariant;
amxc_var_init(&MyVariant);
```

To allocate memory from the heap:

```C
amxc_var_t *pMyVariant = NULL;
amxc_var_new(&pMyVariant);
```

Both methods will create a variant containing a `null` type. It is also important to clean-up the variant when not needed anymore.

```C
amxc_var_clean(&MyVariant);
amxc_var_delete(&pMyVariant);
```

The function `amxc_var_clean` will free all internally allocated memory (if any) to store the data, and will `reset` the variant back to the `null` type. This function can be used as well on variants that are allocated on the heap. After calling `amxc_var_clean` the variant variable is still usable.

The function `amxc_var_delete` will also free the memory that was allocated with `amxc_var_new` and resets the pointer to NULL.

#### Type Names and Type Identifiers

Each type has an identifier (integer) and a name (string). The default supported types have the following identifers and names:

- ID = `AMXC_VAR_ID_NULL`, name = `null`
- ID = `AMXC_VAR_ID_CSTRING`, name = `cstring_t`
- ID = `AMXC_VAR_ID_INT8`, name = `int8_t`
- ID = `AMXC_VAR_ID_INT16`, name = `int16_t`
- ID = `AMXC_VAR_ID_INT32`, name = `int32_t`
- ID = `AMXC_VAR_ID_INT64`, name = `int64_t`
- ID = `AMXC_VAR_ID_UINT8`, name = `uint8_t`
- ID = `AMXC_VAR_ID_UINT16`, name = `uint16_t`
- ID = `AMXC_VAR_ID_UINT32`, name = `uint32_t`
- ID = `AMXC_VAR_ID_UINT64`, name = `uint64_t`
- ID = `AMXC_VAR_ID_FLOAT`, name = `float`
- ID = `AMXC_VAR_ID_DOUBLE`, name = `double`
- ID = `AMXC_VAR_ID_BOOL`, name = `bool`
- ID = `AMXC_VAR_ID_LIST`, name = `amxc_llist_t`
- ID = `AMXC_VAR_ID_HTABLE`, name = `amxc_htable_t`
- ID = `AMXC_VAR_ID_FD`, name = `fd_t`
- ID = `AMXC_VAR_ID_TIMESTAMP`, name = `amxc_ts_t`
- ID = `AMXC_VAR_ID_CSV_STRING`, name = `csv_string_t`
- ID = `AMXC_VAR_ID_SSV_STRING`, name = `ssv_string_t`

Depending on the function or macro used, the identifier must be specified, or the name.

---

> **NOTE ABOUT CUSTOM TYPES**
>
> Custom variant types can be defined. When such a custom type is `registered`, an identifier is assigned to it, the name is provided when registering the custom type. The chosen identifier can be different each time, therefore you need to fetch the identifier using the custom type's name. A function is available for this `amxc_var_get_type_id_from_name`. How to define and register your own custom type is out of the scope of this document and will be explained in a separate document.

---

#### Changing the Type

When a variant is instantiated, it is always a `null` variant. This type does not contain any data. Changing the type of a variant can be done using the function

```C
int amxc_var_set_type(amxc_var_t * const var, const uint32_t type);
```

The type identifier (argument `type`) can be any of the type identifiers as mentioned in [Type Names and Type Identifiers](#type-names-and-type-identifiers)

When changing or setting the type of the variant, the current data stored in the variant is removed and the variant will contain the default value of the `type` set. The default value for integer, float, double is `0`, for booleans `false`, for timestamps `unknown time`, for strings `null string`, for list an empty linked list, for htable an empty hash table.

For primitive types it is not needed to first set the type and then set the value. The value can be set immediately after instantiating the variant.

#### Checking the Type

How to check that the type contained in a variant is the type that is expected?

To get the type identifier of a variant, use the function:

```C
uint32_t amxc_var_type_of(const amxc_var_t * const var);
```

This function will return the type identifier of the contained data.

To fetch the name of the type contained in the variant, use:

```C
const char *amxc_var_type_name_of(const amxc_var_t * const var);
```

Note that this function is returning a `const char *`, there is no need to free the returned pointer (it is not possible as it is `const`).

#### Converting And Casting Variants

Often you need the get the value of the variant in another type then how it is stored within the variant. Or you want to cast the variant to another type.

Requesting the value to be converted can be done by using `amxc_var_dyncast`. The macro `amxc_var_dyncast` is handled in more detail in section [Getting Primitive Values](#getting-primitive-values). To change the type and convert the value in the container to that type you can use `amxc_var_cast`.

Some types can be `automatically` casted to the best fitted type. For detailed information about this, consult the documentation of that variant type.

Example:

Assume you have a variant containing a string "1024", this can be casted to a `uint32_t` type. 

```C
amxc_var_t var;

amxc_var_init(&var);
amxc_var_set(cstring_t, &var, "1024");
amxc_var_cast(&var, AMXC_VAR_ID_UINT32);
```

Or use `autocast` 

```C
amxc_var_t var;

amxc_var_init(&var);
amxc_var_set(cstring_t, &var, "1024");
amxc_var_cast(&var, AMXC_VAR_ID_ANY);
```

#### Setting Values

##### Setting Primitive Values

Setting `primitive` values is easy and can be done by using the `amxc_var_set` macro. This macro can be used directly after instantiating the variant.

```C
amxc_var_t my_var;

amxc_var_init(&my_var);
amxc_var_set(cstring_t, &my_var, "Hello World");
```

The macro takes as first argument the type name, as second the pointer to the variant, and as last argument the value. The value `type` must match the type mentioned as first argument.

The macro uses the type name and not the type identifier. The preprocessor will translate this macro into a function call.

```C
int amxc_var_set_cstring_t(amxc_var_t * const var, const char * const val);
```

##### Setting Composite Values

Building a composite variant - this is a variant containing variants - requires a little bit more effort, by executing following steps:

- Instantiate the variant: `amxc_var_init`
- Set the correct `composite` type: `amxc_var_set_type`
- Fill the variant: use the macros `amxc_var_add` or `amxc_var_add_key`, depending on the composite type used.

To build a variant containing a linked list of variants

```C
amxc_var_t my_var;

amxc_var_init(&my_var);
amxc_var_set_type(&my_var, AMXC_VAR_ID_LIST);
amxc_var_add(cstring_t, &my_var, "Hello");
amxc_var_add(cstring_t, &my_var, "World");
```

To build a variant containing a key - value pairs where the values are variants.
The keys are always strings.

```C
amxc_var_t my_var;

amxc_var_init(&my_var);
amxc_var_set_type(&my_var, AMXC_VAR_ID_HTABLE);
amxc_var_add_key(cstring_t, &my_var, "key1", "Hello");
amxc_var_add_key(cstring_t, &my_var, "key2", "World");
```

It is possible to create a variant containing a linked list of variants, where each variant in the linked list is a variant containing key-value pairs.

Example:

Let's assume you have a list of users and for each user a user-id, name, and access rights. In `C` a structure can be defined containing this data and a linked list iterator, to be able to add this data in a linked list.

```C
typedef struct _system_user {
    uint32_t user_id;
    char *name;
    uint32_t access_rights;
    amxc_llist_t lit;
} system_user_t;
```

A function to add an instance of this `user` information could look like this (without error checking):

```C
void add_user(amxc_llist_t *users, uint32_t user_id, const char *name, uint32_t access_rights) {
    int name_length = strlen(name);
    system_user_t *user = calloc(1, sizeof(system_user_t));
    user->user_id = user_id;
    user->name = calloc(1, name_length + 1);
    strcpy(user->name, name);
    user->access_rights = access_rights;

    amxc_llist_append(users, &user->lit);
}
```

Other functions can then call this one to add an instance, example:

```C
amxc_llist_t *my_worker_function(void) {
    amxc_llist_t *my_users = NULL;
    amxc_llist_new(&my_users);

    add_user(my_users, 1, "John Doe", 0x755);
    add_user(my_users, 2, "Jane Doe", 0x644);
    add_user(my_users, 3, "Super", 0x777);

    return my_users;
}
```

The same can be achieved with variants, but now you do not need to define a `fixed` structure, all you need is a variant that is initialized as a `list` type.

The function could look like (without error checking):

```C
void add_user(amxc_var_t *users, uint32_t user_id, const char *name, uint32_t access_rights) {
    amxc_var_t *user = amxc_var_add(amxc_htable_t, users, NULL);
    amxc_var_add_key(uint32_t, user, "user_id", user_id);
    amxc_var_add_key(cstring_t, user, "name", name);
    amxc_var_add_key(uint32_t, user, "access_rights", access_rights);
}
```

Others can then use this function to fill the variant list

```C
amxc_var_t *my_worker_function(void) {
    amxc_var_t *my_users = NULL;
    amxc_var_new(&my_users);
    amxc_var_set_type(my_users, AMXC_VAR_ID_LIST);

    add_user(my_users, 1, "John Doe", 0x755);
    add_user(my_users, 2, "Jane Doe", 0x644);
    add_user(my_users, 3, "Super", 0x777);

    return my_users;
}
```


The first line in this function adds an empty key-value pair variant to the `users` variant which should be initialized to a `list` type.

You can now try [Practical Lab1](#lab1-build-a-composite-variant), or just continue reading.
Before switching to the `Practical lab` first read [Practical Labs](#practical-labs).

#### Getting Values

##### Getting Primitive Values

Getting the values out of the variant can be done in two different ways:

1. Using macro `amxc_var_constcast`
2. Using macro `amxc_var_dyncast`

Both will retrieve the value of the variant, the difference is that `amxc_var_dyncast` will do conversion if needed and allocate memory if needed, while `amxc_var_constcast` will return the stored value that is in the variant as `const` if it is a pointer.

As `amxc_var_constcast` is never doing any conversions, it will return the default value of the requested type if the type contained in the variant is different.

Some examples:

- The variant contains a string and an integer is requested.

    ```C
    amxc_var_t my_var;
    amxc_var_init(&my_var);
    amxc_var_set(cstring_t, &my_var, "1234");

    int32_t number = amxc_var_constcast(uint32_t, &my_var); // number will be 0
    int32_t number = amxc_var_dyncast(uint32_t, &my_var); // number will be 1234
    ```

- The variant contains a boolean and a string is requested

    ```C
    amxc_var_t my_var;
    amxc_var_init(&my_var);
    amxc_var_set(bool, &my_var, true);

    const char *txt = amxc_var_constcast(cstring_t, &my_var); // txt will be NULL
    char *txt = amxc_var_dyncast(cstring_t, &my_var); // txt will be "true", txt must be freed when not needed anymore
    ```

##### Getting Composite Values

###### Accessing Individual Primitives

Getting the individual values out of a composite variant can be done by accessing the individual variant by its `path`.

The function to retrieve a primitive variant out of the composite variant is:

```C
amxc_var_t *amxc_var_get_path(const amxc_var_t * const var,
                              const char * const path,
                              const int flags);
```

This function will give the pointer to the variant as in the composite variant or a newly allocated variant, which you need to free with `amxc_var_delete`. The `flags` argument is used to indicate what you want, the variant as is or a copy. The flags can be:

- AMXC_VAR_FLAG_DEFAULT (no copy, use the variant as is)
- AMXC_VAR_FLAG_COPY (allocate a new variant, and copy the data)

---

> NOTE
>
> When using the default behavior `AMXC_VAR_FLAG_DEFAULT` the pointer to the variant in the composite variant is returned. When changing the value of that returned value, the value in the composite variant is changed.

---

Example:

Let's take the `users` data set (see [Setting Composite Values](#setting-composite-values)) and assume a complex variant is created with this data (JSON notation)

```json
[
    {
        "user_id": 1,
        "name": "John Doe",
        "access_rights": 0x775
    },
    {
        "user_id": 2,
        "name": "Jane Doe",
        "access_rights": 0x664
    },
    {
        "user_id": 3,
        "name": "Super",
        "access_rights": 0x777
    }
]
```

Getting the name of the second user in the composite variant can be achieved by:

```C
amxc_var_t *var_name = amxc_var_get_path(&users, "1.name", AMXC_VAR_FLAG_DEFAULT);
const char *str_name = amxc_var_constcast(cstring_t, var_name);
```

A list can be accessed using an index, the first element in a list will have index 0. Key-value pairs can be accessed by key.

###### Accessing The List 

When a variant contains a `list` type, the pointer to the linked list in the variant can be fetched using the macro `amxc_var_constcast`.

```C
const amxc_llist_t *var_list = amxc_var_constcast(amxc_llist_t, &my_var);
```

Use the `amxc_llist_t` API to access or iterate over the list. Each item in the list will be a variant. Use function `amxc_var_from_llist_it` to get the variant pointer.

###### Accessing The Hash Table

When a variant contains a `htable` type, the pointer to the hash table in the variant can be fetched using the macro `amxc_var_constcast`. 

```C
const amxc_htable_t *var_table = amxc_var_constcast(amxc_htable_t, &my_var);
```

Use the `amxc_htable_t` API to access or iterate over the hash table. Each item in the table will be a variant. Use function `amxc_var_from_htable_it` to get the variant pointer.

###### Iterating A Composite List

A composite variant containing a list of variants can be iterated using macro `amxc_var_for_each`

Example:

```C
amxc_var_for_each(user, (&users)) {
    amxc_var_t *var_name = amxc_var_get_path(user, "name", AMXC_VAR_FLAG_DEFAULT);
    const char *str_name = amxc_var_constcast(cstring_t, var_name);
}
```

---

> NOTE
>
> Iterating with the macro `amxc_var_for_each` works on both composite list variants and composite key-value pair variants.
>
> As an alternative you can get the pointer to the linked list embedded in the variant using `amxc_var_constcast(amxc_llist_t, &users)` and then use the linked list macro `amxc_llist_iterate` or `amxc_llist_for_each` to iterate over the linked list of variants.
>
> Or incase the variant contains a hash table use `amxc_var_constcast(amxc_htable, ...)` to get the embedded hash table and use the macro `amxc_htable_iterate` or `amxc_htable_for_each` to iterate over all items in the hash table.
>
> To get the variant pointer from a linked list iterator use macro `amxc_var_from_llist_it`. To get the variant pointer from a hash table use  macro `amxc_var_from_htable_it`


---
#### Inspecting The Content

With very complex composite variants it can be difficult to understand what is stored in the variant. A debug function is available, that will print the content of the variant in a JSON like structure. 

```C
int amxc_var_dump(const amxc_var_t * const var, int fd);
```

The file descriptor can be STDOUT_FILENO, STDERR_FILENO or any other file descriptor with write permissions.

Example output (using the `users` example)

```txt
[
    {
        user_id: 1,
        name: "John Doe",
        access_rights: 0x775
    },
    {
        user_id: 2,
        name: "Jane Doe",
        access_rights: 0x664
    },
    {
        user_id: 3,
        name: "Super",
        access_rights: 0x777
    }
]
```

## Practical Labs

Before you start these labs, make sure you have the `Ambiorix Debug & Development` container installed and are able to open terminal in this container. If you haven't the container running with all Ambiorix libraries installed, please read [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) before continuing. 

All commands provided in these labs must be executed in a container terminal.

To open a terminal in the container use following command

```bash
docker exec -ti -u $USER amxdev /bin/bash
```

---

> Note: <br>
You can open multiple terminals to the same container. 

---

Before you continue, you need to clone this git repository.<br>
You can do this using a terminal on your local machine or if you configured the container correctly you can do the clone from a terminal in your container.

Execute these shell commands in a container terminal (or a terminal on your host machine)

```bash
mkdir -p ~/workspace/ambiorix/tutorials/collections/
cd ~/workspace/ambiorix/tutorials/collections/
git clone git@gitlab.com:prpl-foundation/components/ambiorix/tutorials/collections/variants.git
```

### Lab1 - Build a composite variant

Using variant you can create complex data structures using composite variants.

In this lab you need to build a composite variant containing this data:

```text
{
    LocalAgent = {
        parameters = {
            Enable = "True",
            Status = "Up"
        }
    }
    multi_instance = false;
    err_code = 123;
}
```

The application can be compiled with this command:

```bash
cd ~/workspace/ambiorix/tutorials/collections/variants/labs/lab1
gcc main.c -lamxc -o lab1
```

When the function is implemented the output should look like this:

```bash
Variant contains:
{
    LocalAgent = {
        parameters = {
            Enable = "True",
            Status = "Up"
        }
    }
    err_code = 123,
    multi-instance = false,
}
```

To implement the functions about 9 lines of code must be added.

### Lab2 - Modify the content of a variant

Using a variant you can create complex data structures using composite variants.

In this lab you need to use the variant build in Lab1 and convert it so the `parameters` are changed from a table into an array (linked list) containing the names of the parameters.

Starting from a variant containing this data:

```text
{
    LocalAgent = {
        parameters = {
            Enable = "True",
            Status = "Up"
        }
    }
    multi_instance = false;
    err_code = 123;
}
```

Change it into a variant containing the following data:

```text
{
    LocalAgent = {
        parameters = [
            "Enable",
            "Status"
        ]
    }
    multi_instance = false;
    err_code = 123;
}
```

The application can be compiled with this command:

```bash
cd ~/workspace/ambiorix/tutorials/collections/variants/labs/lab2
gcc main.c -lamxc -o lab2
```

To implement the function about 10 lines of code must be added.

---

>**TIP**
> To retrieve a sub-variant from a composite variant you can use one of the following functions:
> - amxc_var_get_key - when the composite variant is a table
> - amxc_var_get_index - when the composite variant is an array.
>
> Example:
> ```C
> amxc_var_t* object_table = amxc_var_get_key(myvar, "LocalAgent", AMXC_VAR_FLAG_DEFAULT);
> ```
> Will get `amxc_var_t*` of the composite variant with key `LocalAgent` out of the hash table variant (composite) `myvar`.
>
> A composite htable variant is a variant containing a hash table. The pointer to the hash table can be fetched using:
> ```C
> const amxc_htable_t* ht_params = amxc_var_constcast(amxc_htable_t, params_table);
> ```
> After that you can use the hash table API to manipulate the hash table itself.
---

When the function is implemented the output should look like this:

```bash
Variant contains:
{
    LocalAgent = {
        parameters = {
            Enable = "True",
            Status = "Up"
        }
    },
    err_code = 123,
    multi-instance = false
}
Modified variant contains:
{
    LocalAgent = {
        parameters = [
            "Enable",
            "Status"
        ]
    },
    err_code = 123,
    multi-instance = false
}
```

Also use `valgrind` to verify if you don't have any memory leak(s).

```bash
$ valgrind ./lab2
==22811== Memcheck, a memory error detector
==22811== Copyright (C) 2002-2017, and GNU GPL d, by Julian Seward et al.
==22811== Using Valgrind-3.14.0 and LibVEX; rerun with -h for copyright info
==22811== Command: ./lab2
==22811==
Variant contains:
{
    LocalAgent = {
        parameters = {
            Enable = "True",
            Status = "Up"
        }
    },
    err_code = 123,
    multi-instance = false
}
Modified variant contains:
{
    LocalAgent = {
        parameters = [
            "Enable",
            "Status"
        ]
    },
    err_code = 123,
    multi-instance = false
}
==22811==
==22811== HEAP SUMMARY:
==22811==     in use at exit: 0 bytes in 0 blocks
==22811==   total heap usage: 59 allocs, 59 frees, 4,386 bytes allocated
==22811==
==22811== All heap blocks were freed -- no leaks are possible
==22811==
==22811== For counts of detected and suppressed errors, rerun with: -v
==22811== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

### Lab3 - Conversions And Casts

In this lab the focus is on the conversion and cast functionality of the Ambiorix variants.

When you have as string containing comma separated values and want to access each value in the string individually, you normally start parsing the string.
Writing such a string parser can get complicated and often you need to write a lot of code for this.

Lets assume we have a string containing a persons first and last name, a list of his favorite movies or television series and his age. The list of movies or series is yet another comma separated values string and should not be splitted.

```C
const char *input = "John,Doe,\"The Return Of The Jedi,Married With Children,Friends\",45";
```

After splitting the above string the individual parts should be:

```text
John
Doe
The Return Of The Jedi,Married With Children,Friends
45
```

And we want to access the age as an integer and not a string.

It is up to you to implement, using the variant API, the string split and cast the age to an integer, also print the value and the type of each part.

Example:
```C
    printf("%s ", amxc_var_constcast(cstring_t, var));
    printf("(type = %s)\n", amxc_var_type_name_of(var));
```

The application can be compiled with this command:

```bash
cd ~/workspace/ambiorix/tutorials/collections/variants/labs/lab3
gcc main.c -lamxc -o lab3
```

To implement the function about 7 - 10 lines of code must be added, depending on the solution you chose.

When the function is implemented the output should look like this:

```C
$ ./lab3 
John (type = cstring_t)
Doe (type = cstring_t)
"The Return Of The Jedi,Married With Children,Friends" (type = cstring_t)
45 (type = uint32_t)
```

Hint: make sure to have a look at the [documentation](https://soft.at.home.gitlab.io/ambiorix/libraries/libamxc/doxygen). There might be some very useful functions to help you with this lab.

## Example - amx-variant-contacts

A more complete example is available at<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/collections/variant_contacts

This example is documented and explained in the [README.md](https://gitlab.com/prpl-foundation/components/ambiorix/examples/collections/variant_contacts/-/blob/main/README.md) file.

### Challenge

In this example the parsing of the data is done using `amxc_string_split_to_llist` which creates a linked list containing the fields. Parsing the string can be done using variants as well.

As a challenge, modify this example to use variant to split the input records into their separate fields.

## Conclusion

After completing this tutorial, you have learned:

- How to use the Ambiorix variant API
- To build and manipulate a variant
- Where to find the API documentation

## References

- Ambiorix Tutorials - Getting Started<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md
- Wikipedia - Variant<br> 
https://en.wikipedia.org/wiki/Variant_type
- Wikipedia - Tagged Union<br> 
https://en.wikipedia.org/wiki/Tagged_union
- libamxc - API Documentation<br>
https://soft.at.home.gitlab.io/ambiorix/libraries/libamxc/doxygen 
- amxc-variant-contacts example<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/collections/variant_contacts
- Valgrind<br>
https://valgrind.org/
