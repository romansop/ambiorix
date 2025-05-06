# Quick Start Guide

[[_TOC_]]

---
> For more detailed explanation please read the README.md
>
> Commands prefixed with `<host>$` must be run on the host, commands prefixed with `<container>$` must be run in the container terminal.
>
---

## Create Workspace Directory

```shell
<host>$ mkdir -p ~/workspace/
```

## Create Container

```shell
<host>$ docker pull registry.gitlab.com/soft.at.home/docker/oss-dbg:latest

<host>$ docker run -ti -d --name amxdev --restart always --cap-add=SYS_PTRACE --sysctl net.ipv6.conf.all.disable_ipv6=1 -e "USER=$USER" -e "UID=$(id -u)" -e "GID=$(id -g)" -v ~/workspace:/home/$USER/workspace registry.gitlab.com/soft.at.home/docker/oss-dbg:latest
```

### Open Container Terminal

```shell
<host>$ docker exec -ti -u $USER amxdev /bin/bash
```

---
> remove the option `-u $USER` to open a terminal as the container root user
> ```shell
> <host>$ docker exec -ti amxdev /bin/bash
> ```
---

### Configure Git In Container 

```shell
<container>$ ssh-keygen -N "" -t rsa -b 2048 -C "<YOUR EMAIL HERE>" -f ~/.ssh/id_rsa
```

---
> Add the generated public key (content of file `~/.ssh/id_rsa.pub`) to your gitlab account. See also https://docs.gitlab.com/ee/ssh/#adding-an-ssh-key-to-your-gitlab-account
---

Configure the global git settings.

```shell
<container>$ git config --global user.email "<YOUR EMAIL HERE>"
<container>$ git config --global user.name "<YOUR FULL NAME HERE>"
```

## Build Ambiorix

### Fetch Sources

```shell
<container>$ cd ~/workspace
<container>$ mkdir ambiorix
<container>$ cd ambiorix
<container>$ repo init -u git@gitlab.com:prpl-foundation/components/ambiorix/ambiorix.git
<container>$ repo sync
<container>$ source .repo/manifests/scripts/local_commands.sh
```

### Build And Install

```shell
<container>$ amx_rebuild_all
```

## Run Example

---

**NOTE**<br>
When using uBus make sure that the commands are run as `root` user in the container. 

---

### Launch Bus Systems

Launch PCB system bus

```shell
<container>$ pcb_sysbus
```

Launch uBus daemon

```shell
<container>$ ubusd &
```

### Launch Example

```shell
<container>$ greeter -D
```

## Bus Commands

### uBus

---

**NOTE**<br>
When using uBus make sure that the commands are run as `root` user in the container. 

---

List all uBus objects

```shell
<container>$ ubus list
```

Get parameters using uBus

```shell
<container>$ ubus call Greeter get '{"depth":999}`
```

Set parameter values:

```shell
<container>$ ubus call Greeter set '{"parameters":{"State":"Start"}}'
```

Call object method

```shell
<container>$ ubus call Greeter say '{"from":"Me", "message":"Hello world"}'
```

### PCB

List all PCB objects

```shell
<container>$ pcb_cli ls
```

Get parameters using PCB

```shell
<container>$ pcb ?
```

Set parameter value

```shell
<container>$ pcb_cli Greeter.State="Start"
```

Call object method

```shell
<container>$ pcb_cli 'Greeter.say(from:"Me", message:"Hello World")'
```