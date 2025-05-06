# Rbus baapi back-end  

[[_TOC_]]

## Introduction

RBUS implementation for the bus agnostic api. This back-end can be loaded using the `amxb_be_load` function which is implemented in [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb).

This back-end is an adaptor that can be used toghether with the libamxb abstraction layer and will provide functionality to make ambiorix based application or service work on RBus.

The current implementation is tested against version 2.0.5 of RBus implementation. When using other versions of RBus it is possible that some modifications are needed in the implementation of the amxb-rbus back-end.

The amxb-rbus is always tested with the latest versions of the ambiorix components that are relevant for this back-end, each time a change is done. Besides testing with the latest versions of ambiorix components at the time of a change of this back-end, it is mainly focused to be used with these versions of the ambiorix components:

- libamxc v1.10.0
- libamxp v1.3.1
- libamxd v6.1.0
- libamxj v0.3.63
- libamxb v4.7.21

Versions lower then the ones specified here could not be supported.

## Building and installing

### Building with yocto

For RDK-B meta-layers for yocto are avaiable [meta-amx](https://gitlab.com/prpl-foundation/prplrdkb/metalayers/meta-amx).

### Building with vagrant

A fully documented vagrant example is available in the [lcm project](https://gitlab.com/prpl-foundation/lcm/development/vagrant)

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

- `skip-verify-type`: (boolean)
  Wildcards '*' or search-expressions can only be used on tables (multi-instance objects). This back-end implemention verifies when a wildcard or search-expression is encountered that the object on which it is used is a table. Some rbus data model providers don't correctly register the objects as tables and rbus will report them as a normal (singleton) object. When that is the case amxb-rbus will return an error. This configuration option disables the check to get arround the wrongly registered objects.

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

## Limitations

### Key Parameters

1. Ambiorix data model providers can define key parameters for multi-instance objects (tables), these key parameters (functional and non-functional) can be used to identify the instances in a unique way. Most of these parameters are write-once (or set by the provider itself). To make it possible for rbus native data model consumers to set these write-once key parameters, they are registered to rbus as writable. It is however not possible to change the access type once an instance is created. Although these parameters are read-only in the instances, they are still reported as writable by rbus. Trying to change the value of these parameters will result in an error. 

1. Key parameters can only be set in ambiorix data model providers when creating an instance. The rbus API can not create an instance and set the values with one single call (see `rbusTable_addRow`). This could prevent native rbus data model consumers to create instances in an ambiorix data model provider. However when `use-amx-calls` is enabled in the ambiorix data model provider a native rbus data model consumer can get arround this by invoking the method `_add` on the parent object of the multi-instance object (table) with as relative path the name of the multi-instance object (table). An rbus native data model provider can call methods by using `rbusMethod_Invoke` or `rbusMethod_InvokeAsync`.

1. Key parameters should be returned in the response of an add instance request. The rbus method `rbusTable_addRow` doesn't provide the key parameters. The same work-around for this limitation can be used as described in the topic above.

1. An ambiorix data model consumer the invokes the `get-instances` will get the list of instances available in the table. When it is invoked on a rbus native data model implementation, the key parameters will not be available as rbus is not adding them. When the ambiorix data model consumer invokes it on an ambiorix data model provider an both have the `use-amx-calls` set, the full and correct respons will be provided, including the key parameters and their values.

### Get Supported Data Model (GSDM)

When an ambiorix data model provider invokes the `get-supported-datamodel` the rbus back-end implementation will use the RBus API functions `rbus_discoverWildcardDestinations` and `rbus_discoverComponentDataElements` in combination to find all supported data elements. Rbus provides the list of the supported data elements, but only the the supported paths of these elements without any extra information. Due to the lack of the extra meta-data the amxb-rbus backend will use the type "string" for all parameters and will indicate all data elements as writable.

Example using ba-cli using rbus:
```
root - rbus: - [bus-cli] (0)
 > gsdm -pl Device.SoftwareModules.ExecutionUnit.
MAD (Object      ) Device.SoftwareModules.ExecutionUnit.{i}.
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.VendorConfigList
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.MemoryInUse
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.RunLevel
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.AvailableMemory
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.AutoStart
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.ExecEnvLabel
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.ExecutionFaultMessage
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.VendorLogList
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.Status
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.AllocatedDiskSpace
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.ExecutionEnvRef
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.Name
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.AssociatedProcessList
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.ExecutionFaultCode
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.AvailableDiskSpace
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.Vendor
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.Description
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.AllocatedMemory
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.DiskSpaceInUse
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.AllocatedCPUPercent
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.EUID
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.References
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.Alias
.RW (cstring_t   ) Device.SoftwareModules.ExecutionUnit.{i}.Version
MAD (Object      ) Device.SoftwareModules.ExecutionUnit.{i}.EnvVariable.{i}.
... (Object      ) Device.SoftwareModules.ExecutionUnit.{i}.Extensions.
MAD (Object      ) Device.SoftwareModules.ExecutionUnit.{i}.HostObject.{i}.
... (Object      ) Device.SoftwareModules.ExecutionUnit.{i}.NetworkConfig.
```

The same using ba-cli using ubus:
```
root - ubus:/var/run/ubus/ubus.sock - [bus-cli] (0)
 > gsdm -pl SoftwareModules.ExecutionUnit.
MAD (Object      ) SoftwareModules.ExecutionUnit.{i}.
.R. (csv_string_t) SoftwareModules.ExecutionUnit.{i}.VendorConfigList
.R. (int32_t     ) SoftwareModules.ExecutionUnit.{i}.MemoryInUse
.RW (uint16_t    ) SoftwareModules.ExecutionUnit.{i}.RunLevel
.R. (int32_t     ) SoftwareModules.ExecutionUnit.{i}.AvailableMemory
.RW (bool        ) SoftwareModules.ExecutionUnit.{i}.AutoStart
.R. (cstring_t   ) SoftwareModules.ExecutionUnit.{i}.ExecEnvLabel
.R. (cstring_t   ) SoftwareModules.ExecutionUnit.{i}.ExecutionFaultMessage
.R. (csv_string_t) SoftwareModules.ExecutionUnit.{i}.VendorLogList
.R. (cstring_t   ) SoftwareModules.ExecutionUnit.{i}.Status
.R. (int32_t     ) SoftwareModules.ExecutionUnit.{i}.AllocatedDiskSpace
.R. (cstring_t   ) SoftwareModules.ExecutionUnit.{i}.ExecutionEnvRef
.R. (cstring_t   ) SoftwareModules.ExecutionUnit.{i}.Name
.R. (csv_string_t) SoftwareModules.ExecutionUnit.{i}.AssociatedProcessList
.R. (cstring_t   ) SoftwareModules.ExecutionUnit.{i}.ExecutionFaultCode
.R. (int32_t     ) SoftwareModules.ExecutionUnit.{i}.AvailableDiskSpace
.R. (cstring_t   ) SoftwareModules.ExecutionUnit.{i}.Vendor
.R. (cstring_t   ) SoftwareModules.ExecutionUnit.{i}.Description
.R. (int32_t     ) SoftwareModules.ExecutionUnit.{i}.AllocatedMemory
.R. (int32_t     ) SoftwareModules.ExecutionUnit.{i}.DiskSpaceInUse
.R. (int32_t     ) SoftwareModules.ExecutionUnit.{i}.AllocatedCPUPercent
.R. (cstring_t   ) SoftwareModules.ExecutionUnit.{i}.EUID
.R. (csv_string_t) SoftwareModules.ExecutionUnit.{i}.References
.R. (cstring_t   ) SoftwareModules.ExecutionUnit.{i}.Alias
.R. (cstring_t   ) SoftwareModules.ExecutionUnit.{i}.Version
M.. (Object      ) SoftwareModules.ExecutionUnit.{i}.EnvVariable.{i}.
... (Object      ) SoftwareModules.ExecutionUnit.{i}.Extensions.
M.. (Object      ) SoftwareModules.ExecutionUnit.{i}.HostObject.{i}.
... (Object      ) SoftwareModules.ExecutionUnit.{i}.NetworkConfig.
```

### Unknown and Infinite Time

Date time values are mostly correctly passed over RBus excpet TR181 `unknown time` and TR181 `inifinite time` as specified in TR-106 (https://www.broadband-forum.org/pdfs/tr-106-1-13-0.pdf) section 3.2.1 Date and Time Rules.

When unknown time or infinite time are passed to RBus they become a valid time!

Example:
Unknown time "0001-01-01T00:00:00Z" becomes in RBus "1970-01-01T00:00:00Z".
