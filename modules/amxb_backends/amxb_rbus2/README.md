# Rbus baapi back-end  

[[_TOC_]]

## Introduction

RBUS implementation for the bus agnostic api. This back-end can be loaded using the `amxb_be_load` function which is implemented in [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb).

This back-end is an adaptor that can be used toghether with the libamxb abstraction layer and will provide functionality to make ambiorix based application or service work on RBus.

## Building and installing

### Building with yocto

For RDK-B meta-layers for yocto are avaiable [meta-amx](https://gitlab.com/prpl-foundation/prplrdkb/metalayers/meta-amx).

For the moment it is recommended to use this branch: https://gitlab.com/prpl-foundation/prplrdkb/metalayers/meta-amx/-/tree/feature/PPM-2739-align-with-prplos-rbus2

### Building with vagrant

TODO

### Building in docker container

1. Install docker

    Docker must be installed on your system.

    If you have no clue how to do this here are some links that could help you:

    - [Get Docker Engine - Community for Ubuntu](https://docs.docker.com/install/linux/docker-ce/ubuntu/)
    - [Get Docker Engine - Community for Debian](https://docs.docker.com/install/linux/docker-ce/debian/)
    - [Get Docker Engine - Community for Fedora](https://docs.docker.com/install/linux/docker-ce/fedora/)
    - [Get Docker Engine - Community for CentOS](https://docs.docker.com/install/linux/docker-ce/centos/)  <br /><br />
    
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
    mkdir -p ~/workspaces/ambiorix_oss
    ```

    Launch the container:

    ```bash
    docker run -ti -d --name ambiorix_oss --restart always --cap-add=SYS_PTRACE --sysctl net.ipv6.conf.all.disable_ipv6=1 -e "USER=$USER" -e "UID=$(id -u)" -e "GID=$(id -g)" -v ~/workspaces/ambiorix_oss:/home/$USER/ambiorix/ -v ~/.ssh:/home/$USER/.ssh registry.gitlab.com/soft.at.home/docker/oss-dbg:latest
    ```

    You can open as many terminals/consoles as you like:

    Using `docker exec`
    ```bash
    docker exec -ti --user $USER ambiorix_oss /bin/bash
    ```

    All following steps should be executed from within the container.

1. Configure the container:

   Configure git and make sure permissions/ownership are set correct (fill in correct user e-mail and name)

   Execute in docker container:
   ```
   sudo chown $USER ambiorix 
   git config --global user.email "you@example.com"
   git config --global user.name "Your Name"
   ```

1. Fetch/build/install dependencies

   Ambiorix components: (execute in docker container)

   ```
   cd ~/ambiorix
   repo init -u git@gitlab.com:prpl-foundation/components/ambiorix/ambiorix.git
   repo sync
   source .repo/manifests/scripts/local_commands.sh
   amx_rebuild_all
   ```

   Rbus: (execute in docker container)

   ```
   cd ~/
   git clone https://github.com/rdkcentral/rbus.git 
   cd rbus                                          
   git checkout v2.0.5                              
   mkdir build                                      
   cd build
   cmake -DBUILD_RTMESSAGE_LIB=ON -DBUILD_RTMESSAGE_SAMPLE_APP=ON -DBUILD_FOR_DESKTOP=ON \
         -DBUILD_DATAPROVIDER_LIB=ON -DCMAKE_BUILD_TYPE=release \
         -DCMAKE_INSTALL_PREFIX=/usr \
         -DCMAKE_C_FLAGS="-Wno-stringop-truncation -Wno-stringop-overflow" .. 
    sudo make                                             
    sudo make install                                    
    cd ../../                                        
   ```
1. Fetch/Build/Install this backend

   Execute in docker container:
   ```
   cd ~/ambiorix/bus_adaptors/
   git clone git@gitlab.com:prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_rbus2.git
   cd amxb_rbus2/
   make
   sudo make install
   ```

## Using the Rbus backend

### Using docker container

When everything is builded and installed in the docker container, it is possible to run and use the amxb-rbus backend.

Create this file in your home directory in the container

`vi ~/test.odl`
```
%define {
  object TestObject {
    string Text;
    uint32 Number;
  }
}
```

Execute in docker container:

```
sudo rtrouted
sudo amxrt -D ~/test.odl
```

Now a data model should be accessible using rbuscli

```
~$ rbuscli -i
rbuscli> get TestObject.
Parameter  1:
              Name  : TestObject.Text
              Type  : string
              Value : 
Parameter  2:
              Name  : TestObject.Number
              Type  : uint32
              Value : 0
rbuscli> 
```

Or using ba-cli:

```
~$ ba-cli
$USER - * - [bus-cli] (0)
 > TestObject.?
TestObject.
TestObject.Number=0
TestObject.Text=""
```

Using ba-cli it is possible to verify which back-ends are loaded:

```
~$ ba-cli
$USER - * - [bus-cli] (0)
 > !addon select mod_ba backend 

2024-05-13T08:36:39Z - $USER - [mod_ba backend] (0)
 > list

|   Name   |  Version  |              Description              |
----------------------------------------------------------------
| dummy    | 01.00.03  | AMXB Dummy Backend for testing        |
| rbus     | 01.01.02  | AMXB Backend for RBUS (RDK-B)         |
| ubus     | 03.03.02  | AMXB Backend for UBUS (Openwrt/Prplwr |

2024-05-13T08:36:42Z - $USER - [mod_ba backend] (0)
 > mode-cli
> !addon select mod_ba cli

$USER - * - [bus-cli] (0)
 > 

```

## Run Time Configuration

All ambiorix based data model providers or data model consumers that are started using the ambiorix runtime application (amxrt) can provide amxb-rbus configuration options at command line or in a config section of an odl file, to configure the behaviour of this back-end.

All rbus backend configuration options can be places in the odl's configuration section and must be set in a table with name of the backend (in this case `rbus`). Following options are available:

- `use-amx-calls`: (boolean)
  When set to true for a data model provider, the protected (interal) ambiorix data model methods (starting with `_`) will be registered and can be used by any data model consumer, when set to false these methods will not be registered.
  When set to true for a data model consumer, the back-end will verify if the protected amxbiorix data model methods can be used and are supported by the data model provider, if set to false this check will skipped and the native RBus API's will be used. 

- `register-on-start-event`: (boolean)
  When set to true register the data elements (objects, properties, methods and events) when the `app:start` event is triggered. If set to false, the data elements are registered to Rbus as soon as the `amxb_register` function is called.

- `register-name`: (string)
  If a value is provided for this configuration option, then this value is used to register the component to Rbus (when creating the connection to Rbus), when this option is not set the value of `name` is used as register name.

- `skip-register`: (string)
  Must be an object path, this path will not be registered to Rbus, but all sub-objects (or data elements) underneath that path are registered. Most of the time this option is not needed or used when a `translate` option is defined.
  Example: When `skip-register`="Device." and the data model provider contains `Device.Time.` then the object `Device.` will not be registered for the data model provider, but `Device.Time.` will.

- `translate`: (table)
  Typically ambiorix data model providers do not define the `Device.` object, but most of the time it is desirable that the `Device.` is prepended to the element paths. The translate option will instruct the amxb-rbus backend to translate the amxbiorix data model element path before registering. It will do the oposite translation on incomming requests.

ODL config example:
```
%config {
    rbus = {
        translate = {
            "'SoftwareModules.'" = "Device.SoftwareModules"
        },
        use-amx-calls = false,
        register-name = "tr181-softwaremodules",
        register-on-start-event = true
    }
}
```

All these options can be passed at commandline as well:

Command line example:
```
amxrt -o 'rbus.use-amx-calls=false' -o 'register-on-start-event=true' /etc/amx/softwaremodules/softwaremodules.odl
```

The odl config options always take precedence to the command line options, using `-F` flag instead of `-o` will force the command line option to be used.

Example command line overiding the config options in the odl files:

```
amxrt -F `rbus.use-amx-calls=true`
```
