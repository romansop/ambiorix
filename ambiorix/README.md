# Ambiorix

## Introduction

Ambiorix project is a case study project. 

For years, desktop distributions have settled on D-Bus and systemd. In the embedded
world this is not yet the case. Many bus systems exist. Even in the opensource
world, there are already at least 2 busses used.

- ubus (OpenWrt)
- D-Bus (RDK-B)

And some proprietary software buses exist as well.

- PCB (Process Communication Bus)
- R-Bus (RDK BUS)

The goal of this study is to investigate whether

- it is possible to create a common C API on top of all these busses
- it is possible to exchange software components between the busses (if possible without any code change)

The initial focus will be on ubus and D-Bus.

Ambiorix (53BC) (Gaulish "king in all directions") was a leader of a Belgic tribe of north-eastern Gaul.
Many tribes in that region where fighting against each other. When the Romans came and conquered the region,
Ambiorix succeeded in joining all the different tribes and fight all together against the common enemy.

This project is not aimed in fighting all the different busses. It is all about making them
"work togheter", making it easier to re-use software parts in different projects, without the 
headaches of writing (again) the interface to another bus.

This project is a collection of re-usable libraries and tools:

- libamxc: Generic data containers
- libamxp: Generic pattern implementations
- libamxj: JSON parser/generator using yajl.
- libamxm: Modularty toolkit. Add mods to your apps
- libamxt: Generic tty functionaly. Adds interactive terminal interface to your apps


## Prerequisites

### Repo tool

You can follow [official installation](https://source.android.com/docs/setup/download#installing-repo) procedures.

#### Repo tool installation using official Debian package

```bash
sudo apt-get update
sudo apt-get install repo
```

#### Repo tool manual installation

```bash
export REPO=$(mktemp /tmp/repo.XXXXXXXXX)
curl -o ${REPO} https://storage.googleapis.com/git-repo-downloads/repo
gpg --recv-keys 8BB9AD793E8E6153AF0F9A4416530D5E920F5C65
curl -s https://storage.googleapis.com/git-repo-downloads/repo.asc | gpg --verify - ${REPO} && install -m 755 ${REPO} /usr/bin/repo
```

## Fetching all libs and tools

```bash
mkdir ~/ambiorix
cd ~/ambiorix

repo init -u git@gitlab.com:prpl-foundation/components/ambiorix/ambiorix.git
repo sync
repo forall -c "git checkout master"
```

If you encounter following error:

```bash
$ repo init -u git@gitlab.com:prpl-foundation/components/ambiorix/ambiorix.git
  File "/usr/bin/repo", line 51
    def print(self, *args, **kwargs):
            ^
SyntaxError: invalid syntax
```

Then your system is still probably using Python2 as a default Python
interpreter, so try to use Python3 directly instead as a workaround:

```bash
$ python3 /usr/bin/repo init -u git@gitlab.com:prpl-foundation/components/ambiorix/ambiorix.git
```
