# Ambiorix Tools - amxrt

Estimated time needed: 40 - 60 minutes.

[[_TOC_]]

## Introduction

The Ambiorix framework is designed to work in single-threaded event driven applications. Handling events is a very important aspect to make your applications run smoothly. 
The Ambiorix Run-Time, [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt), is a base application that can launch an Ambiorix based data model service or an Ambiorix based data model client.

The application by itself does not provide any service or functionality. It provides an eventloop (using libevent), can load one or more ODL files and it can load one or more Ambiorix bus back-ends.
The real functionality is implemented in amxrt plug-ins (add-ons), which are imported using the ODL file.

In the previous tutorials `amxrt` has been introduced briefly as a tool to run Ambiorix plug-ins. In this tutorial we will dive deeper into its functionality.

## Goal

The goal of this tutorial is to explain:

- How to run plug-ins using `amxrt`.
- What config options you can use and what their default values are.
- How to pass different options to `amxrt` from the command line.

## Prerequisites

- You finished the [Getting Started](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/getting-started/-/blob/main/README.md) tutorial
- You got a brief introduction to `amxrt` in the [Define And Publish](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/datamodels/server/define-publish/-/blob/main/README.md) tutorial
- You have a basic understanding of the data model concepts as defined by `Broadband Forum` standards
- You have a basic understanding of the Ambiorix variants
- You have basic C knowledge
- You have basic git knowledge
- You can work with docker containers

## amxrt

The Ambiorix Run-Time, `amxrt`, is at the heart of your application and can be seen as the engine that makes your application run. It opens and parses the provided ODL files (using libamxo) to build a data model, connects with the available software bus systems and provides your application with an event loop.
In the `Define And Publish` tutorial you have seen how you can use `amxrt` without any parameters (except for the daemonize option) and yet it was still able to register the phonebook.odl on the bus system. Remember that you did this by calling

```bash
amxrt -D phonebook.odl
```

with the following `phonebook.odl` file:

```
%define {
    object Phonebook {
        object Contact[] {
            string FirstName;
            string LastName;

            object PhoneNumber[] {
                string Phone;
            }

            object E-Mail[] {
                string E-Mail;
            }
        }
    }
}
```

All configuration options for `amxrt` can be added to the `%config` section of the ODL file, however the only thing present in the `phonebook.odl` file is a `%define` section for the datamodel. So how does `amxrt` know which back-end to use and how to connect to it? This information is passed from the default config options that are embedded in `amxrt`. We will start off by taking a closer look at them.

### Default ODL installation directories

---
> Note:<br>
> This section can be followed passively or interactively. It is recommended to follow the same steps interactively on your system to get a better understanding of the concepts. You can start off with the ODL file in the lab1 folder and extend it during the explanation. The makefile can be ignored for now.
>
> You can clone the repository to your workspace
> ```bash
> mkdir -p ~/workspace/ambiorix/tutorials/datamodels/tools
> cd ~/workspace/ambiorix/tutorials/datamodels/tools/
> git clone git@gitlab.com:prpl-foundation/components/ambiorix/tutorials/tools/amxrt.git
> cd amxrt
> ```
---

As mentioned before, amxrt parses ODL files to build a data model. If you provide the ODL files as a inputs, then it obviously has no difficulty finding the files. However it is often more convenient to pass only your main ODL file as input and include any other files from there. This can be done by using the `include` option in your main ODL file. We will demonstrate this by extending the `Phonebook` example with a `%config` section and we will split up the `%define` and `%config` sections over 2 different files.

We will add following file
- `phonebook.odl`: the main ODL file with a config section

This will be our main odl file, which will be passed to `amxrt`. The data model definition odl file is now `phonebook_definition.odl` and is already provided.

Let's start of with a very simple `phonebook.odl`:

```bash
%config {
}

include "phonebook_definition.odl";
```

It has an empty config section, so all default options will be taken. The last line of the file provides an include of `phonebook_definition.odl`, which contains the data model:

```bash
%define {
    object Phonebook {
        object Contact[] {
            string FirstName;
            string LastName;

            object PhoneNumber[] {
                string Phone;
            }

            object E-Mail[] {
                string E-Mail;
            }
        }
    }
}
```

You can now run the main odl file with `amxrt phonebook.odl` and verify with ubus or pcb_cli that you can still find the data model.

The included file is found based on the following default config options:

```
prefix = "";
cfg-dir = "/etc/amx";
name = "amxrt";
include-dirs = [
    ".",
    "${prefix}${cfg-dir}/${name}",
    "${prefix}${cfg-dir}/modules"
];
```

In this case the current directory as specified with `"."` in `include-dirs` is used to find `phonebook_definition.odl`. As an example you could override the `include-dirs` in your config section and try to run the ODL file again.

```
%config {
    include-dirs = "";
}
```

This should fail with an error `Include file not found` and print all used config variables to stdout. Let's remove this line again to return to the default config options.

The other default include directories are interesting when you install your ODL files to your system files. The default locations to install ODL files are `/etc/amx/${name}/` and `/etc/amx/modules/` (when no prefix is set). The default name doesn't necessarily correspond to `"amxrt"` as listed above, but it corresponds to argument zero when running amxrt. If you run `amxrt` explicitly as we have always done so far (`amxrt phonebook.odl`), then the zeroth argument will be `amxrt`. However you can also create a symbolic link of your plug-in to `amxrt` e.g.

```
ln -s /usr/bin/amxrt phonebook
```

If you now try to run `./phonebook` amxrt will look for the main odl file in `/etc/amx/phonebook/`. Because we haven't actually installed anything there, you will get an error. The key thing to take away from this is that you can get a nice separation of ODL files in `/etc/amx/<name>` for each of your installed Ambiorix plug-ins if you resort to the default directories. This also allows you to keep using the default config options, which makes your life easier and is thus the recommended way of working.

To get a working installation of our phonebook, a makefile has been prepared for you

```make
install: dir
	install phonebook.odl phonebook_definition.odl /etc/amx/phonebook
	ln -sf /usr/bin/amxrt phonebook

dir:
	mkdir -p /etc/amx/phonebook

uninstall:
	rm -rf /etc/amx/phonebook
	rm -f ./phonebook

```

You can use `sudo make install` to install the ODL files and create a symbolic link to your current working directory (you could also change this to `/usr/bin/phonebook` if desired). After installing them, verify that you can run `./phonebook` and can access your data model from ubus or pcb. Run `sudo make uninstall` to undo the installation.

The last default include directory to discuss is `/etc/amx/modules`. This directory is typically used to install ODL files for modules that extends your program's capabilities. An example of such a module is [mod_sahtrace](https://gitlab.com/prpl-foundation/components/core/modules/mod_sahtrace) which provides logging functionality to your application.

> Note: The concept of using the zeroth argument to change a programs behavior is nothing new. A good existing example of this concept is the functionality of BusyBox. BusyBox is a Swiss Army Knife of tools used in an embedded Linux system. It also has a lot of symbolic links to the main busybox binary and depending on which symbolic link you use to call the program, it changes its behavior.

### Import directories, shared objects and entry points

So far we have only looked at ODL files in this tutorial, but in practice you will write C code as well to implement your plug-in. Some C code has been provided for you in the `lab2` folder. The `phonebook.c` file contains 2 functions:

- `_phonebook_main`: this is the entry point of your program
- `print_info`: a small print function that will dump the contents of the ODL config section to stdout and display which back-end connections are made

When launching your plug-in with `amxrt`, the ODL files are parsed, the back-end connections are made, the event loop is created and the entry points are called. The Ambiorix run time will call the entry points with `reason` 0 on startup and `reason` 1 on exit. This gives the implementor of the plug-in the ability to provide a function that can do initialization and clean-up. Other applications that use the ODL parser library can invoke the entry points for different reasons. More information can be found in the [ODL documentation](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md#define-entry-points).

Which entry point needs to be called on startup and exit is defined in the `%define` section of the ODL file. The `phonebook.odl` file has been extended to show how this works

```
%define {
    entry-point phonebook.phonebook_main;
}
```

The import resolver will use the above statement to look for a shared object `phonebook` with a function `_phonebook_main` which will be used as entry point. This brings us to the other additions that have been made to the `phonebook.odl` file. The `%config` section has been extended with a name for our phonebook plug-in and a shared object with that name will be imported.

```
%config {
    name = "phonebook";
}

import "${name}.so" as "${name}";
```

The import is also modified and now defines an alias for the shared object. 

---

> **Note**<br>
 The alias needs to be identical to the part before the dot in our entry point definition i.e. `phonebook`.

---

--- 

> **Note**<br>
When using a symbolic link to `amxrt` the `name` configuration option can be omitted in the configuration section if the name is the same as the symbolic link name.
`amxrt` will used the zeroth argument as the name by default, if no name is defined in the configuration section

---

Now that everything is connected properly, we just need to build the shared object. But before doing that, you should try to run our ODL file again with

```bash
amxrt phonebook.odl
```

This will fail with an error `Import file not found` and will print the ODL `%config` section. Take a closer look at the `import-dirs` section

```
import-dirs = [
    ".",
    "${prefix}${plugin-dir}/${name}",
    "${prefix}${plugin-dir}/modules",
    "${prefix}/usr/local/lib/amx/${name}",
    "${prefix}/usr/local/lib/amx/modules"
],
plugin-dir = "/usr/lib/amx",
```

This lists the five default import directories where the `import` resolver will look for shared objects. None of these directories contain `phonebook.so` because we haven't built and installed it yet, which is why the parsing fails. Also note that the `${name}` parameter that we defined earlier in the ODL file is used to define some of the directories. In this tutorial we will use the directory `/usr/lib/amx/phonebook` to install our phonebook plug-in.

The `makefile` from lab1 has been extended to allow the code to be compiled to a shared object.

> **Note**:<br>
 This tutorial will not explain how makefiles work. The provided makefile should be basic enough for you to understand. If you have trouble understanding it, you can refer to some references at the end of the tutorial.


You can now run `make` to build the `phonebook.so` file in the `output` directory. Run `sudo make install` to install the shared object in `/usr/lib/amx/phonebook`. This also creates a symbolic link of `/usr/bin/phonebook` to `/usr/bin/amxrt`. If you have your `PATH` set up properly (this will be the case if you are using the recommended docker container), you can now run the phonebook example using this symbolic link:

```
~/workspace/ambiorix/tutorials/tools/amxrt/labs/lab2$ phonebook
ERROR -- Failed to create pidfile
****************************************
*      Phonebook tutorial started      *
****************************************
dm = 0x5652b0fdf5c0
config = 
{
    auto-connect = true,
    auto-detect = true
    auto-resolver-order = [
        "ftab",
        "import",
        "*"
    ],
    backends = [
        "/usr/bin/mods/amxb/mod-amxb-pcb.so",
        "/usr/bin/mods/amxb/mod-amxb-ubus.so"
    ],
    cfg-dir = "/etc/amx",
    daemon = false,
    import-dbg = false,
    import-dirs = [
        ".",
        "${prefix}${plugin-dir}/${name}",
        "${prefix}${plugin-dir}/modules",
        "${prefix}/usr/local/lib/amx/${name}",
        "${prefix}/usr/local/lib/amx/modules"
    ],
    import-pcb-compat = false,
    include-dirs = [
        ".",
        "${prefix}${cfg-dir}/${name}",
        "${prefix}${cfg-dir}/modules"
    ],
    listen = [
    ],
    mib-dirs = [
        "${prefix}${cfg-dir}/${name}/mibs"
    ],
    name = "phonebook",
    pcb = {
        register-name = "phonebook"
    },
    pid-file = true,
    plugin-dir = "/usr/lib/amx",
    prefix = "",
    priority = 0,
    uris = [
        "pcb:/var/run/pcb_sys",
        "ubus:/var/run/ubus.sock"
    ],
}
uri = pcb:/var/run/pcb_sys (fd = 8)
uri = ubus:/var/run/ubus.sock (fd = 12)
```

Depending on which back-ends you have installed, your output might be slightly different. For example if you don't have the pcb back-end installed, you will not see it listed under `backends` and you won't see its socket under `uris`. When you exit the program (using CTRL+C) you will see the prints from the exit case of the entry point

```
^C****************************************
*      Phonebook tutorial stopped      *
****************************************
```

As a simple exercise, try to override the `import-dirs` config variable such that it imports the shared object from the `output` directory. You can clean up the `output` directory and undo the installation with

```bash
make clean
sudo make uninstall
```

### The remaining config variables

Most of the config variables have been discussed by now, but a few still remain. Almost all of them are explained in the [Predefined Configuration Options](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md#odl-parser-configuration-options) part of the ODL documentation. As a last exercise you can play around a bit with these variables. Some small exercises are listed below:

- Try to set `pid-file` to false, to remove the error message `ERROR -- Failed to create pidfile` that you get when running the `phonebook` plug-in. This will stop `amxrt` from trying to create pid files in the `/var/run` directory. Another way to avoid this issue is running `phonebook` with sudo privileges.
- Set `auto-connect` to false, to prevent automatically connecting to the bus system sockets using the loaded back-end. This will remove the connection prints that you see after the variant dump of the config section.
- Set `daemon` to true to run the application in the background. Run `killall phonebook` to kill the background process.

Some bus systems or protocols provide support for end-to-end communication. When using end-to-end communication two processes are connected to each other directly and no broker or dispatcher application is in between them. Typically one of the processes is listening for incoming connections using a listen socket. As soon as such a listen socket is created, other processes can connect to it.

In the `config` section, you can define the `listen` socket

- If you are using the pcb or usp back-end try to create a listen socket on `/tmp/phonebook.sock` and check if it is created correctly using `ls /tmp/phonebook.sock`. Remember to use the correct URI scheme for the socket connection. Hint: take a look at the existing `uris` if you're having trouble.

---
> **Note**
> The ability to create end-to-end connections as described highly depends on the bus system or protocol you are using. If the bus system does not support end-to-end connections (like ubus) it will not be possible to define a listen socket.
---

### Adding custom config variables

The list of possible config variables is not fixed, you can add your own variables and access these in your code. The odl parser library (libamxo) provides API functions to read or change the values of these config variables. You can also access the full set of config variables directly using `&parser->config` as shown in the entry point function which is implemented in `phonebook.c`. 

As you can see this is just a `amxc htable variant`, so you can access the content of this variant also using the `amxc variant` API as explained in the [collections - variants](https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/collections/variants) tutorial.

As an exercise you can define your own config variable in the `%config` section of `phonebook.odl` and re-launch the `phonebook` data model, your config variable should be printed at start-up.

### Passing command line arguments to amxrt

A lot of command line arguments can be passed to amxrt for setting certain options. These are explained in the introduction of the amxrt [README.md](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/-/blob/main/README.md) file. They provide the exact same functionality as the config variable definitions in the ODL file and will not be repeated here. Just know that they exist, because they can come in handy when you want to test something on the fly without changing an ODL file.

## Conclusion

After finishing this tutorial you have learned:

- How to run plug-ins using `amxrt`.
- What config options you can use and what their default values are.
- How to pass different options to `amxrt` from the command line.

## References

- The Ambiorix Runtime repository<br>
https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt
- Object Definition Language:<br>
https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo/-/blob/main/doc/odl.md
- Ambiorix sahtrace module:<br>
https://gitlab.com/prpl-foundation/components/core/modules/mod_sahtrace
- BusyBox:<br>
https://busybox.net/about.html
- Makefile tutorial<br>
https://makefiletutorial.com/
- A Simple Makefile Tutorial<br>
https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/
- An explanation of shared libraries<br>
https://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html
- Tutorial Collections - Variants<br>
https://gitlab.com/prpl-foundation/components/ambiorix/tutorials/collections/variants
