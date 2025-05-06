# Ambiorix Linked List

Estimated Time Needed: 60 - 75 minutes.

[[_TOC_]]

## Introduction

Wikipedia [Doubly linked list](https://en.wikipedia.org/wiki/Doubly_linked_list)

> In computer science, a doubly linked list is a linked data structure that consists of a set of sequentially linked records called nodes. Each node contains three major parts: two link fields (references to the previous and to the next node in the sequence of nodes) and one or more data fields. The beginning and ending nodes' previous and next links, respectively, point to some kind of terminator, typically a sentinel node or null, to facilitate traversal of the list. 

---

![](doc/images/Doubly-linked-list.png) 

[Figure 1]:<br>
Figure 1<br>
A doubly linked list whose nodes contain three fields: the link to the previous node, an integer value, and the link to the next node.

---

## Goal

The goal of this tutorial is to explain:

- why a generic linked list implementation is needed.
- how  the Ambiorix linked list is working
- how to use the Ambiorix linked list API.

## Prerequisites

- You finished the [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) tutorial
- Basic knowledge of C
- Basic knowledge of git

## Linked Lists in C

### Data Structures

In C a node is typically defined as a structure, taking the above example this can be translated in C using this structure:

```C
struct node {
    uint32_t number;
    struct node* next;
    struct node* prev;
};
```

Creating the linked list as shown in [Figure 1] could be done using this code:

```C
struct node* build_llist(void) {
    struct node* node1 = calloc(1, sizeof(struct node));
    struct node* node2 = calloc(1, sizeof(struct node));
    struct node* node3 = calloc(1, sizeof(struct node));

    node1->number = 12;
    node1->next = node2;

    node2->prev = node1;
    node2->number = 99;
    node2->next = node3;

    node3->prev = node2;
    node3->number = 37;

    return node1;
}
```

### The Need For An API

Manipulating the linked list can become cumbersome without an API that provides the basic operations like add, remove, iterate. Using the example structure the following API could help in adding a node or deleting a node.

```C
void llist_node_add(struct node* list, struct* item);
void llist_node_remove(struct node* item);
```

Implementing these function would make it possible to easier add nodes to the linked list and remove nodes from the linked list, but still there is a problem with this API. These functions will only work for the `struct node` type. When another structure is defined, and a linked list needs to be created, you have to write other functions to manipulate that linked list.

This way of working is not very re-usable.

### Using `offsetof`

From the [man](https://www.man7.org/linux/man-pages/man3/offsetof.3) pages

> The macro offsetof() returns the offset of the field member from the start of the structure type.
> This macro is useful because the sizes of the fields that compose a structure can vary across implementations, and compilers may insert different numbers of padding bytes between fields. Consequently, an element's offset is not necessarily given by the sum of the sizes of the previous elements.

To make a linked list API reusable it must be independent of the data type or structure used. The only thing that is common for all linked lists are the two pointers `next` and `prev` which point to the next node and previous node.

If we define a structure like this, and call this structure the linked list iterator.

```C
typedef struct _llist_it {
    struct _llist_it* next;
    struct _llist_it* prev;
} llist_it_t;
```

It is possible to create a linked list of nodes and it does not contain any references to specific data.

Going back to the example linked list, the data structure can be redefined:

```C
struct node {
    uint32_t number;
    llist_it_t it;
};
```

The API functions could then be redefined using the linked list iterator structure:

```C
void llist_node_add(llist_it_t* list, llist_it_t* item);
void llist_node_remove(llist_it_t* item);
```

As the iterator is part of the structure containing the data, we can use the `offsetof` macro to calculate the address of the structure containing the iterator, using the formule

```
<address of iterator> - offset(<data structure type>, <member name>)
```

This can be translated into this macro
```C
#define container_of(addr, type, member) \
    ((type*) (((char*) addr) - offsetof(type, member)))
```

## Ambiorix Linked List API

The full API - structures, macros and functions - is documented. <br>

The documentation can be consulted:

- In the header file [amxc_llist.h](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc/-/blob/main/include/amxc/amxc_llist.h)
- As a web page [libamxc](https://prpl-foundation.gitlab.io/components/ambiorix/libraries/libamxc/doxygen) 

### Linked List structures

The Ambiorix library `libamxc` contains two structures and a set of functions with which you can create data independent linked lists.

The first structure is the `linked list iterator` structure:

```C
typedef struct _amxc_llist_it {
    struct _amxc_llist_it* next; /**< Pointer to the next item
                                      in the linked list */
    struct _amxc_llist_it* prev; /**< Pointer to the previous item
                                      in the linked list */
    struct _amxc_llist* llist;   /**< Pointer to the linked list */
} amxc_llist_it_t;
```

The second structure is the `linked list` main structure

```C
typedef struct _amxc_llist {
    amxc_llist_it_t* head; /**< Pointer to the first item in the linked list */
    amxc_llist_it_t* tail; /**< Pointer to the last item in the linked list */
} amxc_llist_t;

```

Although it is possible to manipulate the members of these structures it is recommenced to use the API functions to do that.

### Linked List Functions

In this tutorial not all available linked list API functions will be handled, only the most commonly used.

#### Constructing and initializing a linked list

```C
int amxc_llist_new(amxc_llist_t** llist);
int amxc_llist_init(amxc_llist_t* const llist);
```

When you need to allocate a linked list on the heap use `amxc_llist_new`, initializing a linked list on the stack is done by using `amxc_llist_init`. Please note that when allocating a linked list structure on the heap with `amxc_llist_new` it is already initialized, so there is no need to call `amxc_llist_init`.

#### Destructing and clean-up

```C
void amxc_llist_delete(amxc_llist_t** llist, amxc_llist_it_delete_t func);
void amxc_llist_clean(amxc_llist_t* const llist, amxc_llist_it_delete_t func);
```

Items (nodes) in the linked list can be allocated on the heap, and therefore the allocated memory of those items must be freed when not needed anymore. Often when cleaning up the linked list itself, the items must be cleaned-up as well.

It is possible to create a loop that does exactly that, remove each item (node) from the linked list and free memory as needed. As this is a very common use case and mostly all items (nodes) in the linked list are of the same type, a `delete` callback function can be provided. This callback function will be called for each item (node) in the linked list.  

#### Adding nodes

```C
int amxc_llist_append(amxc_llist_t* const llist, amxc_llist_it_t* const it);
int amxc_llist_prepend(amxc_llist_t* const llist, amxc_llist_it_t* const it);
int amxc_llist_set_at(amxc_llist_t* llist, const unsigned int index, amxc_llist_it_t* const it);
int amxc_llist_it_insert_before(amxc_llist_it_t* const reference, amxc_llist_it_t* const it);
int amxc_llist_it_insert_after(amxc_llist_it_t* const reference, amxc_llist_it_t* const it);
```

All of these function will add an item (node) to the linked list, by using a pointer to the iterator that has been put in the data structure. If the item (node) was already in a list (the same or another one), it will be removed from the list it is in and added to the list at the requested position.

#### Removing nodes

```C
void amxc_llist_it_take(amxc_llist_it_t* const it);
amxc_llist_it_t* amxc_llist_take_first(amxc_llist_t* const llist);
amxc_llist_it_t* amxc_llist_take_last(amxc_llist_t* const llist);
amxc_llist_it_t* amxc_llist_take_at(const amxc_llist_t* llist, const unsigned int index);
```

All of these functions will remove an item (node) from the linked list, but will not delete it from memory.

Keep in mind that it is not possible to directly address a node in a linked list by index. To get the node using an index in a in a linked list, the linked list needs to be traversed from the beginning until the node is reached that matches the given index.

#### Getting A Node

```C
amxc_llist_it_t* amxc_llist_get_first(const amxc_llist_t* const llist);
amxc_llist_it_t* amxc_llist_get_last(const amxc_llist_t* const llist);
amxc_llist_it_t* amxc_llist_it_get_next(const amxc_llist_it_t* const reference);
amxc_llist_it_t* amxc_llist_it_get_previous(const amxc_llist_it_t* const reference);
amxc_llist_it_t* amxc_llist_get_at(const amxc_llist_t* const llist, const unsigned int index);
```

Keep in mind that it is not possible to directly address a node in a linked list by index. To get the node using an index in a linked list, the linked list needs to be traversed from the beginning until the node is reached that matches the given index.

#### Iterating All Nodes

```C
#define amxc_llist_for_each(it, list)
#define amxc_llist_iterate(it, list)
```

These are macros that can be used to iterate over all nodes of a linked list.

Both macros should be used as a `for` loop:

```C
amxc_llist_iterate(ll_it, my_linked_list) {
    // do something here
}
```

The differences between both macros is that with `amxc_llist_for_each` it is possible to remove the current `it` from the linked list, but it is not possible to embed another `amxc_llist_for_each` in the loop.

The macro `amxc_llist_iterate` doesn't allow that the linked list is modified while iterating, but it is possible to embed others `amxc_llist_iterate` in the loop.

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
mkdir -p ~/workspace/ambiorix/tutorials/collections/
cd ~/workspace/ambiorix/tutorials/collections/
git clone git@gitlab.com:prpl-foundation/components/ambiorix/tutorials/collections/linked-lists.git
```

### Lab1 - Build A Linked List

For this lab all code is given. The code is building a linked list as shown in this figure.

>![](doc/images/Doubly-linked-list.png) 

A structure is defined which contains two members, a `number` and a `linked list iterator`.

```C
typedef struct _data {
    uint32_t number;
    amxc_llist_it_t ll_it;
} data_t;
```

In the main function a linked list is created (on the stack) and filled with three nodes, using a helper function to allocate a data structure. When the linked list is created and filled, the number of each node is printed to the standard output. Last but not least, the linked list and all nodes are deleted.

To build the code, use the installed compiler:

```bash
cd ~/workspace/ambiorix/tutorials/collections/linked-lists/labs/lab1
gcc main.c -lamxc -o lab1
```

And now run it:

```bash
$ ./lab1
Number = 12
Number = 99
Number = 17
```

### Lab2 - Split A linked List

Now let's extend the [lab1](#lab1-build-a-linked-list) code.

Additions done:

- At startup two arguments must be given a `seed` and the `number` of nodes.
- A pseudo-random generator is seeded with the `seed`
- A linked list is created with a `number` of items, each item containing a random integer.
- Two empty lists are created `odd_numbers` and `even_numbers`

It is now up to you to implement the function `lab2_split_llist` that must split the original linked list and puts the even integers nodes in the `even_numbers` linked list and the odd integers in the `odd_numbers` linked list.

The application can be compiled with this command:

```bash
cd ~/workspace/ambiorix/tutorials/collections/linked-lists/labs/lab2
gcc main.c -lamxc -o lab2
```

When the function is implemented the output should look like this:

```bash
$ ./lab2 101 10
All numbers:Number = 66
Number = 79
Number = 1
Number = 25
Number = 70
Number = 80
Number = 12
Number = 60
Number = 20
Number = 21
Odd numbers are:
Number = 79
Number = 1
Number = 25
Number = 21
Even numbers are:
Number = 66
Number = 70
Number = 80
Number = 12
Number = 60
Number = 20
```

The implementation of function `lab2_split_llist` should take about 8 lines of code.

### Lab3 - Build A Stack With Linked List

Using a linked list is an easy way to build a stack (or queue). In this lab you have to implement a push and pop function using linked list API, to push data on a stack and pop it from the stack. Do not forget to delete your data.

The application can be compiled with this command:

```bash
cd ~/workspace/ambiorix/tutorials/collections/linked-lists/labs/lab3
gcc main.c -lamxc -o lab3
```

When the functions are implemented the output should look like this:

```bash
$ ./lab3 
Popped number 3 from the stack
Popped number 64 from the stack
Popped number 12 from the stack
Popped number 17 from the stack
Popped number 99 from the stack
```

To implement the functions about 5 lines of code must be added.

The Ambiorix library provides a full implementation of a stack and a queue using a linked list, first implemented it  by yourself and then look in the documentation of [libamxc](https://prpl-foundation.gitlab.io/components/ambiorix/libraries/libamxc/master/).

- [amxc_lstack](https://prpl-foundation.gitlab.io/components/ambiorix/libraries/libamxc/master/dd/dae/a00163.html) - linked list based stack implementation. 
- [amxc_lqueue](https://prpl-foundation.gitlab.io/components/ambiorix/libraries/libamxc/master/db/d44/a00162.html) - linked list based queue implementation.

Or look at the source code to see how it is implemented in the library:

- [amxc_lstack.h](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc/-/blob/main/include/amxc/amxc_lstack.h)
- [amxc_lqueue.h](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc/-/blob/main/include/amxc/amxc_lqueue.h)

### Lab4 - List of Contacts

In all previous labs we used a simple data structure, one data field and a linked list iterator. In real world situations more complex data structures will be used. In this lab we are going to build a list of contacts, each contact will contain these fields:

- first_name - Contains the first name
- last_name - Contains the last name
- gender - Contains the gender (enum)
- age - The age as an integer
- email - one e-mail address
- phone - one phone number.

The records are provided as a string, containing comma separated values.

Example:

```Text
Violet,Elliott,Female,25,v.elliott@randatmail.com,896-3242-73
```

Using other Ambiorix functions these strings are splitted into a linked list, using "," as a separator. For each `person record` a structure is allocated and added to the linked list.

It is up to you to implement the search function and the clean-up callback function.

To implement the functions about 15 lines of code must be added.

---
>Note:<br>
>
>When working with more complex data structures it is possible that these structures contain allocated memory members, like in this structure where the fields `first_name`, `last_name`, `email`, `phone` are separate allocated blocks of memory. When implementing an iterator delete callback function (which can be passed to `amxc_llist_clean` or `amxc_llist_delete`) this function can be reused as your overall delete function if needed. All you need to do is call that function by yourself and pass the address of the linked list iterator.
>
>When using such a callback function as your delete function, also make sure that you remove the item form the linked list, otherwise the linked list gets invalid (dangling pointers).

---

The application can be compiled with this command:

```bash
cd ~/workspace/ambiorix/tutorials/collections/linked-lists/labs/lab4
gcc main.c -lamxc -o lab4
```

When the functions are implemented the output should look like this:

```bash
$ ./lab4 Elliott
Person : Violet Elliott
        Gender = Female
        Age    = 25
        E-Mail = v.elliott@randatmail.com
        Phone  = 896-3242-73

$ ./lab4  Cole
Person : Deanna Cole
        Gender = Female
        Age    = 30
        E-Mail = d.cole@randatmail.com
        Phone  = 929-0326-86
```

To verify that each allocated memory block is freed, run the application under valgrind:

```bash
$ valgrind ./lab4  Cole
==20857== Memcheck, a memory error detector
==20857== Copyright (C) 2002-2017, and GNU GPL d, by Julian Seward et al.
==20857== Using Valgrind-3.14.0 and LibVEX; rerun with -h for copyright info
==20857== Command: ./lab4 Cole
==20857==
Person : Deanna Cole
        Gender = Female
        Age    = 30
        E-Mail = d.cole@randatmail.com
        Phone  = 929-0326-86
==20857==
==20857== HEAP SUMMARY:
==20857==     in use at exit: 0 bytes in 0 blocks
==20857==   total heap usage: 250 allocs, 250 frees, 7,697 bytes allocated
==20857==
==20857== All heap blocks were freed -- no leaks are possible
==20857==
==20857== For counts of detected and suppressed errors, rerun with: -v
==20857== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0
```

## Example - amx-llist-contacts

A more complete example is available at<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/collections/llist_contacts

This example is documented and explained in the [README.md](https://gitlab.com/prpl-foundation/components/ambiorix/examples/collections/llist_contacts/-/blob/main/README.md) file.

## Conclusion

After completing this tutorial you have learned:

- How to use the Ambiorix Linked List API
- To build and manipulate a linked list
- Where to find the API documentation

It is not the intention of this tutorial to explain the Ambiorix API's in depth, it will provide you links and pointers that will help you to get more information, and of course all sources are available in gitlab.

## References

- Ambiorix Tutorials - Getting Started<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md
- Wikipedia - Doubly linked list<br> 
https://en.wikipedia.org/wiki/Doubly_linked_list
- Online Linux man pages - offesetof<br>
https://www.man7.org/linux/man-pages/man3/offsetof.3.html
- libamxc - API Documentation<br>
https://prpl-foundation.gitlab.io/components/ambiorix/libraries/libamxc/doxygen 
- amxc-llist-contacts example<br>
https://gitlab.com/prpl-foundation/components/ambiorix/examples/collections/llist_contacts
- Valgrind<br>
https://valgrind.org/
