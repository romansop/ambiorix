# Coding Guidelines

- [Introduction](#Introduction)
- [Naming convention](#naming-convention)
  - [General](#general)
  - [Variables](#variables)
  - [Functions](#functions)
  - [Others](#others)
- [Code Style](#code-style)
  - [Curly Brackets](#curly-brackets)
  - [Continuation](#continuation)
  - [Indentation](#indentation)
  - [Comments](#comments)
    - [In line comments](#in-line-comments)
    - [Libraries and API's](#libraries-and-apis)
  - [Code Line lengths](#code-line-lengths)
  - [Function lengths](#function-lengths)
- [Code Rules](#code-rules)
  - [Declaration and Initialization](#declaration-and-initialization)
  - [Switches and Cases](#switches-and-cases)
  - [Use of macros](#use-of-macros)
  - [Use of goto](#use-of-goto)
  - [Use standard types](#use-standard-types)
  - [Conditions](#conditions)
  - [Use of const](#use-of-const)
- [Coding Rules of Thumb](#coding-rules-of-thumb)]
  - [I like KISSES](#i-like-kisses)
  - [Keep the code DRY](#keep-the-code-dry)
  - [Keep the code COOL](#keep-the-code-cool)
  - [Getters are no setters](#getters-are-no-setters)
  - [Don't dig too deep](#dont-dig-too-deep)
  - [Validate the input](#validate-the-input)
  - [Constantly use const](#constantly-use-const)

## Introduction

As a start, it is recommended to read [Coding Standards and Guidelines](
https://wiki.softathome.com/mediawiki/images/5/59/2001_C%2B%2B_Coding_Standards_and_Guidelines.pdf) written by __Kosmas Karadimitriou,  Ph.D.__. Although it explains good coding standards and guidelines for C++, most of them apply also for C. This coding guideline is mainly based on this document.

Most of these coding guidelines are for code readability, with one big thought in mind

> Code is written once, read many times

It is very important that the code is readable, and understandable. Basically the code must be self-explaining.

> Be a software engineer – not a programmer!

A programmer is anybody who knows the syntax of a language and can " throw together ” some code that works. However, a software engineer is someone who not only writes code that works correctly, but also writes high quality code. High quality code has the following characteristics:

- Maintainability (easy to find and fix bugs, easy to add new features)
- Readability (easy to read)
- Understandability (easy to understand – this is not the same as readability, e.g. redundant and repetitive code may be very readable, but not necessarily easily understandable)
- Re-usability (easy to reuse parts of code, e.g. functions, classes, data structures, etc)
- Robustness (code that can handle unexpected inputs and conditions)
- Reliability (code that is unlikely to produce wrong results; it either works correctly, or reports errors)

Some of the guidelines defined here are personal preferences. If you don't agree with something here, that's perfectly fine. These rules aren't intended to be universal admonitions about quality.

## Naming convention

### General

- Use names that are clear and as concise as possible.
- Generally avoid names consisting of more than 4 words.
- Names should not be too long. Generally 20 characters should be considered a soft limit.
- All names in [snake case](https://en.wikipedia.org/wiki/Snake_case).

### Variables

- Variable names may be prefixed based on type or scope, but is not a must.
  - When prefixing variables, separate the prefix and the name with an underscore.
  - Following prefixes are suggested to be used:
    - g for global variables (g_my_var) (Avoid using global variables !!) 
    - s for static variables (s_my_var)
    - p for pointer variables (p_my_var)
    - l for local variables (l_my_var)

    These prefixes may be merged, e.g. sp_my_var

### Functions

- Function names should be prefixed with:
  - (for libraries only) The name of the library.
  - The name of the functional group.
- The name of the function should be a clear hint to what it is doing.

### Others

- Macros (#define) defining a constant must be all in upper case.
- Functional macros (#define with arguments) may be in lower case or upper case.
- Structs, enums, typedefs in lower case.
- Typedefs should be suffixed with `_t`.

## Code Style

This section is more about the visual aspect of the code and the readability.

Most of the rules is to have a single function fit on a single pc screen, without having to scroll up and down to understand the logic of the function.

Further, we want code to be as readable as possible, without overloading the system with too many words. Part of this readability is that the code flow should be clear at a glance.


### Curly brackets

- ALWAYS put curly brackets !!
- Open curly bracket should be on the same line as the code that precedes it. In case of multiple preceding lines, the open curly bracket goes on the last line, not on a separate line.
- After an open curly bracket, you always indent exactly one level.
- Before an open curly bracket, there must be exactly one space.

Rationale

- Using curly brackets clearly shows the beginning and ending of a block.
- Avoid the "forgot to add curly brackets" bug when later adding code.

```C
// wrong
if(a > 0) do_something();  // Missing curly brackets

if(a > 0)  // Missing curly brackets
    do_something();

for(int i =0; i < 10; i++) do_something();  // Missing curly brackets

for(int i =0; i < 10; i++)  // Missing curly brackets
    do_something();

void myfunction(void){ // no space before open curly bracket
    // do something
}

// correct
if(a > 0) { do_something(); }

if(a > 0) {
    do_something();
}

for(int i =0; i < 10; i++) { do_something() };

for(int i =0; i < 10; i++) {
    do_something();
}
```

### Continuation

- If a "single command" does not fit on a single line, then the next line (the continuation of the line) should be aligned with the open bracket. This applies to both function declarations, or commands.

Rationale

- Readability, aligning everything that is related to each other (like function arguments, conditional expressions in an if, ...) makes it easier to read and understand.

```C
//wrong
void my_long_function_name(int my_long_var_name1,
        int my_long_var_name2, int my_long_var_name3);

int the_retval = my_long_function_name(
    var1, var2, var3
);

if(some_var >= 120 || second_var <= 120 ||
        third_var != 0) {
    // do something here
}

// correct
void my_long_function_name(int my_long_var_name1, int my_long_var_name2, int my_long_var_name3);
// or
void my_long_function_name(int my_long_var_name1,
                           int my_long_var_name2,
                           int my_long_var_name3);

int the_retval = my_long_function_name(var1, var2, var3);
// or
int the_retval = my_long_function_name(var1,
                                       var2,
                                       var3);

if(some_var >= 120 || second_var <= 120 || third_var != 0) {
    // do something here
}
// or
if(some_var >= 120 || 
   second_var <= 120 ||
   third_var != 0) {
    // do something here
}

```

### Indentation

- Use spaces no tabs.
- Indent only after a curly bracket (with 4 spaces) or for continuation.

Rationale

The idea of tabs was that we'd use tabs for indentation levels, and spaces for alignment. This lets people choose an indentation width to their liking, without breaking alignment of columns.

```
int main( void ) {
|tab   |if( pigs_can_fly() == true ) {
|tab   ||tab   |developers_can_use_tabs( "and align columns "
|tab   ||tab   |                         "with spaces!" );
|tab   |}
}
```

But, alas, we (and our editors) rarely get it right. There are four main problems posed by using tabs and spaces:

- Tabs for indentation lead to inconsistencies between opinions on line lengths. Someone who uses a tab width of 8 will hit 80 characters much sooner than someone who uses a tab width of 2. The only way to avoid this is to require a tab-width, which eliminates the benefit of tabs.
- It's much harder to configure your editor to correctly handle tabs and spaces for each project, than it is to just handle spaces. See also: [Tabs vs Spaces: An Eternal Holy War](https://www.jwz.org/doc/tabs-vs-spaces.html)
- It's harder to align things using only the space bar. It's much easier to hit tab twice than to hold the space bar for eight characters. A developer on your project will make this mistake eventually. If you use spaces for indentation and alignment, you can hit the tab key in either situation, which is quick, easy and not prone to errors.
- Cut the complexity, and use spaces everywhere. You may have to adjust to someone else's indent width every now and then. Tough luck!

- Spaces have the same size everywhere, just one character, while tabs can have visually a different size, depending on the text editor or IDE used. It is true that in all good editors or IDE's you can configure the size of a tab character, but that is not always true for online viewers. It happens that you need to open the code with another editor than the one you are used to:

  - when you want to go through the code with someone else on that person's computer/laptop with another editor (we love team work)
  - in a (docker) container
  - maybe even on an embedded device
  - ...

- Using only tabs it is very hard to align the arguments of function declarations, function calls and so on (See Continuation paragraph). Using a combination of tabs and spaces is even more ugly.

It is nice that the code you wrote looks exactly the same wherever you open the code.

### Comments

#### In line comments

- Comment your code lightly.
- When adding comments, explain why you did something, not what the code is doing.

Rationale

The code should be self explaining. If you need to document what the code is doing, maybe it is too complex. Re-factor your code to make it clear and readable.

```C
// A very well documented function. Is all of this comment needed?

// this function is returning the character at the requested position
char get_char_from_alphabet(unsigned int pos) {
    //define the alphabet
    char the_alphabet[27] = "abcdefghijklmnopqrstuvwxyz";

    // when pos is higher then the length of the alphabet
    // the 0 char is returned, otherwise the character
    // at the position is returned
    if(pos > strlen(the_alphabet)) {
        // return the 0 character
        return 0x00;
    }

    // return the alphabet character
    return the_alphabet[pos];
}

/*
    Comments on the comments
    - The line above the function is not needed, the function name states clearly that it takes a character from the alphabet
    - The comment above the declaration of the variable is not needed, it is obvious that we define, declare and initialize a variable that contains the alphabet, more over the name of the variable states what is contains.
    - The lengthy comment above the if, is explaining what the code does, it would be better to write down why the check is there
    - The comment above the returns are not needed, it should be clear what is returned.
*/

// better

char get_char_from_alphabet(unsigned int pos) {
    char the_alphabet[27] = "abcdefghijklmnopqrstuvwxyz";

    // Respect boundaries
    if(pos > strlen(the_alphabet)) {
        return 0x00;
    }

    return the_alphabet[pos];
}

```

#### Libraries and API's

- Document you public API in the public header files.
- Explain for each public function, structure, enum what it is doing or what it is for and how to use it.
- Highlight possible wrong usages of the public API that could lead to possible memory leaks, performance impacts, crashes (segmentation faults, ...).

Rationale

- When creating a library or a public available API, the API must be usable from the beginning, without the need to dig in the implementation details. Libraries are mostly installed in binary form, with the header files. Software engineers using your library need to use the header files. If the documentation is at the same place it is very easy to access.

If possible provide clear examples or code snippets that show in a clear fashion how to use the API.

### Code Line lengths

- A soft limit of a line length (including indentation) of 80 characters is recommended but not a real limit. It is recommended not to go above 100 characters.

Rationale

- Welcome to the 21st century, we have graphical user interfaces, HD and even 4K monitors. Hard limiting to 80 characters is no longer a need. We can put many more characters on a single line, but keep in mind that putting more then 100 characters on a line will decrease readability and your code will be less easy to comprehend.
- In IDE's there are often side bars or other really helpful windows open. If the code lines are too long, there is no place left for the other informational windows. 

### Function lengths

- A soft limit of 40 lines is a recommendation. If needed, more lines are possible. Please, do not go over 50 lines.

Rationale

- To keep the code understandable, it should fit on one single screen height. Longer functions are also harder to understand.
- Long functions (more than 50 lines) tend to be complex and not readable anymore (need scrolling up and down). Often these functions are doing more than one single thing. Consider to rewrite the function into smaller entities.

## Code Rules

This section is about what to do and not to do in the code.

### Declaration and Initialization

- Write declaration and initialization on a single line and only one at a line.
- Declare and initialize at the beginning of the block, on the next line after the open curly bracket. Never in between the code.
- Reduce the scope of variables, use the smallest possible scope.

Rationale

- Where is a variable declared? At the beginning of the code block. No hassle to find the declaration.
- No long lists of variables at the beginning of a function. All variables only needed in a specific scope or only declared for that scope.
- Declaring more than one variable at a line is error prone and not readable.

```C
// wrong
void myfunc(void) {
    char* a, b = NULL; // multiple line declaration
                       // and b is not a char * in this case. Is this the intention?
                       // variable a only used in a smaller scope 
    int myvar;  // no initialization
    int some_var = 100;
    do_something(some_var);

    int another_var = 200; // declaration in between code
    do_something_else(another_var);

    if(myvar > 200) {
        a = malloc(100);
    }

    do_something_extra(b, myvar);
}

// correct
void myfunc(void) {
    char* b = NULL;
    int myvar = 0;
    int some_var = 100;
    int another_var = 200;
 
    do_something(some_var);
    do_something_else(another_var);

    if(myvar > 200) {
        char* a = NULL;
        a = malloc(100);
    }

    do_something_extra(b, myvar);
}
```

### Switches and Cases

- Avoid lengthy switch cases.
- Avoid fall-through.

Rationale

- Switch case statements are often used to handle all possible enums from an enum definition. This can lead to very lengthy functions. These kind of switch statements can be easily implemented with a function array. Each enumeration in the enum will then correspond with a function.
- If it is not possible to avoid a lengthy switch case, try to call a function for each of the cases. This keeps the case implementation small. 
- A fall-through (no break at the end of the case) is often the cause of bugs, or can later lead to bugs. Not when the code is written the first time, because you are writing it and probably know what you are doing, but often later when maintaining the code. If it can not be avoided, make sure you add a comment why there is no break and end the case with a big "// FALL THROUGH" comment, at the place where normally the break should have come.
- Cascading switches with code in between is a case where code flow can get confused, which is bad.

### Use of macros

- Try to avoid generator macros.
- Macros should not be depending on the existing of variables. Use macro arguments.
- Avoid setting values in macros.

Rationale

- Functional macros are not debugable. The generated functions (by the preprocessor) can not be found in the code, as they do not exist in the code itself. Functional macros are not maintainable. Often a functional macro can be replaced with a static inline function.
- Macros must be reusable, and help to make the code more readable.

```C
// wrong

// not reusable, variables x,a,b must exist
#define SET_MIN (x = (a>b)?b:a) 
#define SET_MAX (x = (a>b)?a:b)

// extract from real-life example - please never do this.
#define define_list(type) \
\
    struct _list_##type; \
    \
    typedef struct _list_elem_##type \
    { \
        type _data; \
        struct _list_elem_##type* _next; \
    } list_elem_##type; \
    \
    typedef struct _list_##type \
    { \
        size_t _size; \
        list_elem_##type* _first; \
        list_elem_##type* _last; \
        _list_functions_##type* _functions; \
    } List_##type; \
    \
    bool list_is_empty_##type(const List_##type* list) \
    { \
        return list->_size == 0; \
    } \
    \
    size_t list_size_##type(const List_##type* list) \
    { \
        return list->_size; \
    } \
    \
    const type list_front_##type(const List_##type* list) \
    { \
        return list->_first->_data; \
    }  

// correct
// reusable and can be used in assignment
#define SET_MIN(a,b) (a>b)?b:a)
#define SET_MAX(a,b) (a>b)?a:b)

// good functional macros
// they make the code more readable, understandable
#define when_null_goto(x,l) if(x == NULL) { goto l; }
#define when_true_goto(cond,l) if(cond) { goto l; }

int myfunction(char* buffer, int buf_len, int start, int len) {
    int retval = -1;
    when_null_goto(buffer, end);
    when_true_goto(start + len > buf_len, end);

    // do something here

    retval = 0;

end:
    return retval
}
```

### Use of goto

- Do not use goto, except for exception handling

Rationale

- As C has no exception handling, goto is an easy and simple way to have a kind of exception handling.

Example:

```C
int myfunction(char* buffer, int buf_len, int start, int len) {
    int retval = -1;

    if(buffer == NULL) {
        goto end;
    }

    if(start + len > buf_len) {
        goto end;
    }

    // do something here

    retval = 0;

end:
    return retval
}
```

- Suggestion, define macros that help make the exception throwing clear.

Example:

```C
#define when_null_goto(x,l) if(x == NULL) { goto l; }
#define when_true_goto(cond,l) if(cond) { goto l; }

int myfunction(char* buffer, int buf_len, int start, int len) {
    int retval = -1;
    when_null_goto(buffer, end);
    when_true_goto(start + len > buf_len, end);

    // do something here

    retval = 0;

end:
    return retval
}
```

- Any other use of goto is forbidden.

### Use Standard Types

- Use C99 standard types with size specification, contained in <stdint.h>.
- Need a boolean, use a `bool` and not an `int` type. `bool` is defined in C99 standard in <stdbool.h> as well as 'true' and 'false', use it.

Rationale

- Fixed size variables are easier portable then architecture dependent variables like `int`.
- Is the use of 'int', 'unsigned int', .. forbidden? No, absolutely not. Often you do not have a choice, but wherever possible use the types with size specification.

### Conditions

- Use conditions that evaluates to boolean (true or false).

Rationale

- Writing full conditional expressions makes the code more readable and understandable.

```C
// wrong
if(!myptr) {
    // do something
}

// correct
if(myptr == NULL) {
    // do something
}
```

## Coding Rules of Thumb

### I like KISSES

I like **K**eeping **I**t:

- **S**imple : (primary objective) Do not over complicate things. Write clear and readable code that everyone can understand.
- **S**tructured : (secondary objective) Try to create as much structure in code as possible. 
- **E**legant : Pleasingly ingenious and according to best practices.
- **S**tandard : Match your code with how other code is written. It has been shown that consistent code is easier to read, so easier to maintain.

### Keep the code DRY

**D**on't **R**epeat **Y**ourself

This can however mean several things:

- Avoid copy pasting blocks of code and just changing a variable somewhere. This is typically an indication that you should just make a function, and provide the changed elements as variables.
- If you see two blocks of code (5 lines +) that are pretty much identical, you should really consider making a function that encapsulates the behavior, and just parameterize it as needed. The bigger the block, and the more identical, the more urgent the need to just create a function that does it.
The logic here is that typically, when changes happen to such blocks of code (and they will happen!), if you made it a separate function, you only have to change the function and all the functionality is updated. If you duplicated your behavior 2, 3 or more times, you have 2, 3 or more times as much work, and are `VERY VERY` likely to forget it. So be lazy, don't create more work than necessary.
- Don't repeat data. Avoid having to store the same data in multiple places. This also applies to data that is easily retrievable. A good example here is the size of a linked list. For some reason, the system may need to regularly know the size of a given linked list. Avoid the temptation to store a variable "size" with this list, but rather use the `llist_size()` function. This way you avoid "forgetting" to update the list in case of certain operations leading to unexpected sizes.

### Keep the code COOL

**C**ohesive **O**bject **O**riented **L**ayers

This basically means:

- Write code with objects in mind. An object is basically a combination of data and functions, so no real reason to not use this in C. Just define a struct for the object data, and write the relevant functions in a file. Think of a C files as a C++ class implementation.
- Write cohesive objects. This basically means that your objects should not do too much, and should only do things that are related to each other. Avoid having "Do everything" objects, that basically contain everything. Split behavior, and make it cohesive, one object to do one job.
- Write layered code. Things are easier to model and remember if you write in layers. Have a top layer (and top object), and then gradually go down your object tree to get more into detail.

### Getters are no setters

Try to avoid mixing state introspection functions (i.e. functions that return an object or state, getters), with state modification functions (i.e. functions that write / change the internal state, setters). The general idea is that if the function returns anything else except an error code, it should not change internal state. Exception to this rules are:

- `get or add` functions, where we search for a certain object, and if we don't find it, create a new object.
- validate functions, where we match some external state with internal state, and change one of them if the system is out of sync. Although this can be considered a setter.

### Don't dig too deep.

Avoid adding levels of depth if it is not really necessary.

```C
//Bad example:
bool f(object_t* obj) {
    int stuff = 0
    int a_stuff = 100;
    int b_stuff = 200;

    if(obj != NULL) {
        object_do_stuff(obj, stuff);
        if(obj->isA != NULL) {
            object_doA_stuff(obj, a_stuff);
            if(obj->isB != NULL) {
                object_doB_stuff(obj, b_stuff);
            }
        }
    }
    return true;
}

//Good example:
bool f(object_t* obj) {
    int stuff = 0
    int a_stuff = 100;
    int b_stuff = 200;

    if(obj == NULL){
        return true;
    }
    object_do_stuff(obj, stuff);
    if(obj->isA == NULL) {
        return true;
    }
    object_doA_stuff(obj, a_stuff);
    if(obj->isB == NULL) {
        return true;
    }
    object_doB_stuff(obj, b_stuff);
    return true;
}

// or even use assert macros
#define when_null_goto(x,l) if(x == NULL) { goto l; }
bool f(object_t* obj) {
    int stuff = 0
    int a_stuff = 100;
    int b_stuff = 200;

    when_null_goto(obj, end);
    object_do_stuff(obj, stuff);
    when_null_goto(obj->isA, end);
    object_doA_stuff(obj, a_stuff);
    when_null_goto(obj->isB, end);
    object_doB_stuff(obj, b_stuff);

end:
    return true;
}
```

### Validate the input

Always test your function arguments before using them, preferably at the beginning of the function:

- Pointers should be checked for NULL.
- Numeric types should undergo range checking.

Mistakes happen a lot and often go by unnoticed. If wrong or invalid argument values are passed to your function, make sure you capture them and `throw` an error.  

## Constantly use const

Const improves compile-time correctness. It isn't only for documenting read-only pointers. It should be used for every read-only variable and pointee.

`const` helps the reader immensely in understanding a piece of functionality. If they can look at an initialization and be sure that that value won't change throughout the scope, they can reason about the rest of the scope much easier. Without `const`, everything is up in the air; the reader is forced to comprehend the entire scope to understand what is and isn't being modified. If you consistently use `const`, your reader will begin to trust you, and will be able to assume that a variable that isn't qualified with `const` is a signal that it will be changed at some point in the scope.

Using `const` everywhere you can also helps you, as a developer, reason about what's happening in the control flow of your program, and where mutability is spreading. It's amazing, when using `const`, how much more helpful the compiler is, especially regarding pointers and pointees. You always want the compiler on your side.
