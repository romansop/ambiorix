# USP Backend

[[_TOC_]]

## Introduction

USP back-end implementation for bus agnostic API.

This module can be loaded using the [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb) using function `amxb_be_load`.

This back-end implements the following methods:

- connect - connect to a unix domain socket
- disconnect - closes connection
- listen - creates a new listen socket
- accept - accepts an incoming connection on an open listen socket
- register - registers the data model on the connection
- get_fd - gets the file descriptor of the connection
- read - read data from the connection and parse it
- read_raw - read raw data from the connection without parsing it
- invoke - invokes a remote method and wait for result (blocking)
- free - frees the allocated memory for a connection
- subscribe - subscribes to datamodel events
- unsubscribe - unsubscribes from datamodel events
- get - gets a datamodel object recursively
- set - sets a datamodel parameter
- add - adds an instance of a multi-instance object
- del - deletes an instance of a multi-instance object
- get_supported - get the supported data model starting from a given object
- has - backend supports custom has function which will discover data models by sending a USP get request

## Configuration

The following few sections will give an overview of the different configuration options that can be used for the USP backend.

### Backend location

The backend will by default be installed in `/usr/bin/mods/usp/`, and this can be configured with the CONFIG_SAH_MOD_AMXB_USP_INSTALL_DIR configuration variable. It is not installed in `/usr/bin/mods/amxb/` by default, like the other backends, because the USP backend should not be loaded automatically by every service. Only the services that expose a TR-181 data model should be exposed via USP and with the `auto-connect` feature from ambiorix any service that loads the USP backend will connect to the USP Broker sockets. Installing the backend in a different directory ensures that no service will unintentionally connect to the USP Broker.

### Setting the EndpointID

In a USP ecosystem each service must have a unique ID that will be used in every message sent from or to that service. This ID is the USP EndpointID and it can be configured using the `usp.EndpointID` configuration option. The USP backend does not impose any restriction on what this ID can be, but other services (or the USP Broker) might, so it is recommended to always use [standard EndpointID schemes](https://usp.technology/specification/#sec:endpoint-identifier).

If no EndpointID is configured by the USP service, one will be generated automatically by the USP backend, because this is required to be able to communicate. The format for the generated EndpointID will be `proto::${name}` where `${name}` is the name of the process as configured in the odl config section of the plugin, or the name of the symlink used to run amxrt. When no `name` is set, the default EndpointID will be `proto::<random-integer>`.

When connecting to another USP service, EndpointIDs from both sides will be exchanged using [handshake messages](https://usp.technology/specification/#sec:handshaking-with-unix-domain-sockets) before any other communication. Once this is done, the processes can start communicating using USP messages.

### Retrying the connection

In an ideal world, every USP service can connect to all USP sockets it wants to and will stay connected for as long as needed. Even though Unix Domain Sockets are very reliable, there are still some situations that need to be taken into account when deploying a system in the real world.

- The process that needs to connect to the USP socket could start up before the process that needs to create the socket.
- The socket connection could be broken for some reason.

To solve these problems a connection retry mechanism can be used. When a connection is made, its file descriptor typically needs to be added to an event loop. For ambiorix applications this is done by `libamxrt`, so this is also where the connection retry mechanism is implemented. For more information regarding the retry mechanism, refer to the [libamxrt documentation](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt). The retry mechanism can be toggled on or off for each backend. For the USP backend, it can be toggled with:

- `usp.connect-retry`: (bool, default = false) enable or disable the amxrt connection retry mechanism for the USP backend.

### Registering the data model

The USP backend currently has 2 methods for registering the data model:

- Registering data model roots (default): Registers the root data model object(s) for the component
- Custom registration: registers whatever is added by the user to the list of registrations

For the custom registration, the USP config section must be updated with the list of data model paths that need to be registered e.g.

```odl
%config {
    usp.registrations = [
        "Device.Buttons.",
        "Device.Bridging.",
        "Device.BulkData."
    ]
}
```

This will build a USP Register message with the above 3 data model paths and sends the message on the connection.

It is possible that something goes wrong during registration of the data model. For example when using `amxrt`, `amxb_register` will be called pretty soon after `amxb_connect`. During the connect, a handshake message will be sent to the other side of the connection. To avoid blocking behavior, the backend will not wait for a handshake response before continuing. When `amxb_register` is called a short time later, it is possible that the handshake message from the other side hasn't arrived yet, in which case it won't be possible to build a USP Register message as this requires the target EndpointID which is provided in the handshake message. This means the registration will fail.

It is possible to retry the registration by adding 2 configuration options to the USP config section

- `usp.register-retry`: (boolean, default = false) when set to true, a timer will be started to retry the registration in case it fails.
- `usp.register-retry-time`: (integer, default = 10000) the registration retry time in milliseconds before the registration is retried.

Most backends don't allow registering the data model multiple times, however the USP backend does allow this when custom registrations are used. This can be useful when the entire data model is not yet available on startup of the plugin, for example when the plugin serves as a proxy to a different bus system. In this case it is allowed to call `amxb_register` multiple times. Before each `amxb_register` call, the `usp.registrations` list in the config section can be appended with the paths that are available at that point in time. The USP backend will empty this list whenever the USP Register message is successfully sent on the connection to the other side. This means you only need to append more items to the list and call `amxb_register` again if you need to do the registration in multiple parts.

### Data model translations

To be compliant with the TR-181 specifications, all object paths must start with a `Device.` prefix. Ambiorix processes typically don't start with the `Device.` root object, but start one level lower in the data model tree (or sometimes have a custom root object), which means they are not compliant with TR-181. To circumvent this, a translation table can be added to the odl config settings of the service, for example for the `MQTT` data model:

```odl
    usp = {
        translate = {
            "'MQTT.'" = "Device.MQTT."
        }
    };
```

This will make sure that any incoming USP message will have its paths translated from `Device.MQTT.` to `MQTT.` such that the request can be invoked on the local `MQTT` data model and each outgoing USP message will have the opposite conversions applied.

> NOTE  
> When a translation table is configured, the translations will always be applied. In case of the MQTT example this means that an incoming request for the `MQTT.` data model will be translated to a request for the `Device.MQTT.` data model before it is invoked on the local data model. Since the service can't handle requests for `Device.MQTT.` on its local data model (its root data model starts at MQTT), the request will fail. In other words, if a translation table is used, all incoming requests MUST always arrive with converted path.

### Strict TR-181 compliance

As mentioned in the previous section, TR-181 data models must always start with a `Device.` prefix. To ensure that no requests are sent on a path that does not start with `Device.` an extra check is added to the USP backend to reject all messages that don't start with `Device.`. This will avoid unnecessary communication overhead when the message would be rejected on the other side anyway. The behavior can be disabled with the following config option:

`usp.requires-device-prefix`: (bool, default = compile time configurable) when set to true, each backend operation (get/set/add/...) will return with `amxd_status_object_not_found` if the provided path does not start with `Device.`. When set to false, this check is skipped and the message is sent as is.

The default value can be configured with the CONFIG_SAH_MOD_AMXB_USP_REQUIRES_DEVICE_PREFIX configuration option. This will set the default value for each plugin that loads the USP backend and this value can be overwritten by each individual plugin using the odl config option `usp.requires-device-prefix`.

A good use case for this feature is the following. Ambiorix services often use `amxb_be_who_has` to discover which object can be found on which backend connection. In case of the USP backend, `amxb_be_who_has` will cause a USP Get message to be sent on the connection with the path provided to the function as the requested path of the Get. When using a distributed architecture where many services are connected to a USP Broker and the Broker does not accept messages for objects that don't start with a `Device.` prefix, this can cause a lot of unnecessary communication overhead. The Broker will never respond with a successful Get response, so there is no point in sending the Get in the first place.

### The LocalAgent data model

To comply with the USP specifications, each service that uses the USP backend needs to implement part of the `Device.LocalAgent.` data model. The implementation for (part of) this data model can be found in the USP backend. The `Device.LocalAgent.` data model needs to be loaded by each service using the USP backend, so a single odl file is added to the system at a default location, which can be changed with the following config option:

`usp.local-agent-dm`: (string, default = "/etc/amx/modules/usp/local-agent.odl") path to the location of the `Device.LocalAgent.` data model odl file.

## Building, installing and testing

### Docker container

You could install all tools needed for testing and devoloping on your local machine, but it is easier to just use a pre-configured environment. Such an environment is already prepared for you as a docker container.

1. Install docker

    Docker must be installed on your system.

    If you have no clue how to do this here are some links that could help you:

    - [Get Docker Engine - Community for Ubuntu](https://docs.docker.com/install/linux/docker-ce/ubuntu/)
    - [Get Docker Engine - Community for Debian](https://docs.docker.com/install/linux/docker-ce/debian/)
    - [Get Docker Engine - Community for Fedora](https://docs.docker.com/install/linux/docker-ce/fedora/)
    - [Get Docker Engine - Community for CentOS](https://docs.docker.com/install/linux/docker-ce/centos/)<br /><br />
    
    Make sure you user id is added to the docker group:

    ```
    sudo usermod -aG docker $USER
    ```

1. Fetch the container image

    To get access to the pre-configured environment, all you need to do is pull the image and launch a container.

    Pull the image:

    ```bash
    docker pull registry.gitlab.com/soft.at.home/docker/oss-dbg:latest
    ```

    Before launching the container, you should create a directory which will be shared between your local machine and the container.

    ```bash
    mkdir -p ~/workspace/modules/amxb_backends/
    ```

    Launch the container:

    ```bash
    docker run -ti -d --name amxdev --restart always --cap-add=SYS_PTRACE --sysctl net.ipv6.conf.all.disable_ipv6=1 -e "USER=$USER" -e "UID=$(id -u)" -e "GID=$(id -g)" -v ~/workspace/:/home/$USER/workspace/ registry.gitlab.com/soft.at.home/docker/oss-dbg:latest
    ```

    If you are using the vpn, you may need to add `--dns 192.168.16.10 --dns 192.168.16.11` to the run command.

    The `-v` option bind mounts the local directory for the ambiorix project in the container, at the exact same place.
    The `-e` options create environment variables in the container. These variables are used to create a user name with exactly the same user id and group id in the container as on your local host (user mapping).

    You can open as many terminals/consoles as you like:

    ```bash
    docker exec -ti --user $USER amxdev /bin/bash
    ```

### Building

#### Prerequisites

- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc) - Generic C api for common data containers
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp) - Common patterns implementation
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd) - Data model C-API
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb) - Bus agnostic C API (mediator)
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo) - Ambiorix Object Definition Language library
- [libimtp](https://gitlab.com/soft.at.home/usp/libraries/libimtp) - Generic implementation for the internal message protocol
- [libusp](https://gitlab.com/soft.at.home/usp/libraries/libusp) - Library for encoding and decoding USP protobuf messages.
- [libuspi](https://gitlab.com/soft.at.home/usp/libraries/libuspi) - USP helper library for utility and subscriptions

#### Build amxb_usp

1. Clone the git repository

    To be able to build it, you need the source code. So open the directory just created for the ambiorix project and clone this library in it (on your local machine).

    ```bash
    cd ~/workspace/modules/amxb_backends
    git clone git@gitlab.com:prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_usp.git
    ``` 

1. Install dependencies

    Although the container will contain all tools needed for building, it does not contain the libraries needed for building `amxb_usp`. To be able to build `amxb_usp` you need `libamxc`, `libamxp`, `libamxd`, `libamxb`, `libamxo`, `libimtp`, `libusp` and `libuspi`. These libraries can be installed in the container by executing the following commands. 

    ```bash
    sudo apt update
    sudo apt install libamxc libamxp libamxd libamxb libamxo libimtp protobuf-c-compiler libprotobuf-c-dev libusp libuspi
    ```

1. Build it

    When using the internal gitlab, you must define an environment variable `VERSION_PREFIX` before building.

    ```bash
    export VERSION_PREFIX="master_"
    ```

    After the variable is set, you can build the package.

    ```bash
    cd ~/workspace/modules/amxb_backends/amxb_usp
    make
    ```

### Installing

#### Using make target install

You can install your own compiled version easily in the container by running the install target.

```bash
cd ~/workspace/modules/amxb_backends/amxb_usp
sudo -E make install
```

### Testing

#### Prerequisites

- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj) - JSON parser & generator using yajl and libamxc variants
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo) - Ambiorix Object Definition Language library

#### Run tests

1. Install dependencies

    Most of the packages needed for testing are already installed in the container. To be able to test `amxb_usp` you need to install a few more libraries. They can easily be installed with `apt install`:

    ```bash
    sudo apt update
    sudo apt install -y libamxj amxrt lua-amx mod-lua-amx mod-dmproxy libamxut libssl-dev libcurl4-openssl-dev libsqlite3-dev libz-dev autoconf automake libtool pkg-config libmosquitto-dev
    ```

    Note that several of these libraries are needed for testing `amxb_usp` against `obuspa`. When running the tests, `obuspa` will be compiled from source and will be installed. While you don't need root privileges to run any of the tests, you do need root permissions to install `obuspa` in `/usr/local/bin/`, so it is possible that you need to run `sudo make test` once with root privileges and afterwards you can run `make test` without root privileges (after a `sudo make clean`). Alternatively, you can already install `obuspa` in advance with:

    ```bash
    git clone https://github.com/BroadbandForum/obuspa.git
    cd obuspa/

    # Would prefer to checkout a tag, but there is no recent tag
    git checkout c5554313d5ea77edfdfe24e31cd786399609304b

    # Optionally export PKG_CONFIG_PATH, it may not be needed depending on your environment
    export PKG_CONFIG_PATH="/usr/lib/pkgconfig:/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig"
    autoreconf --force --install

    # It's not needed to disable all of this, but you may need to install more dependencies if some `--disable` flags are left out. Check the obuspa documentation for more info.
    ./configure --disable-coap --disable-websockets --disable-stomp --disable-bulkdata
    make

    # The only step where you need sudo
    sudo make install
    ```

1. Run tests

    You can run the tests by executing the following command.

    ```bash
    cd ~/workspace/modules/amxb_backends/amxb_usp/tests
    make
    ```

    Or this command if you also want the coverage tests to run:

    ```bash
    cd ~/workspace/modules/amxb_backends/amxb_usp/tests
    make run coverage
    ```

#### Coverage reports

The coverage target will generate coverage reports using [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html) and [gcovr](https://gcovr.com/en/stable/guide.html).

A summary for each file (*.c files) is printed in your console after the tests are run.
A HTML version of the coverage reports is also generated. These reports are available in the output directory of the compiler used.
Example: using native gcc
When the output of `gcc -dumpmachine` is `x86_64-linux-gnu`, the HTML coverage reports can be found at `~/workspace/modules/amxb_backends/amxb_usp/output/x86_64-linux-gnu/coverage/report.`

You can easily access the reports in your browser.
In the container start a python3 http server in background.

```bash
cd ~/workspace/
python3 -m http.server 8080 &
```

Use the following url to access the reports `http://<IP ADDRESS OF YOUR CONTAINER>:8080/modules/amxb_backends/amxb_usp/output/<MACHINE>/coverage/report`
You can find the ip address of your container by using the `ip` command in the container.

Example:

```bash
USER@<CID>:~/workspace/modules/amxb_backends/amxb_usp$ ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
173: eth0@if174: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:ac:11:00:07 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 172.17.0.7/16 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 2001:db8:1::242:ac11:7/64 scope global nodad 
       valid_lft forever preferred_lft forever
    inet6 fe80::42:acff:fe11:7/64 scope link 
       valid_lft forever preferred_lft forever
```

in this case the ip address of the container is `172.17.0.7`.
So the uri you should use is: `http://172.17.0.7:8080/modules/amxb_backends/amxb_usp/output/x86_64-linux-gnu/coverage/report/`
