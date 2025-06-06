# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]


## Release v0.5.2 - 2024-12-04(08:55:44 +0000)

### Other

- [amx-cli] missing include for basename

## Release v0.5.1 - 2024-02-10(08:37:48 +0000)

### Other

- [Prpl] Bad rpath makes build fail on Yocto LCM build

## Release v0.5.0 - 2024-01-29(13:36:23 +0000)

### Other

- [amx][cli] Add syntax to filter parameters list

## Release v0.4.2 - 2023-11-29(21:50:26 +0000)

### Fixes

- [cli] Reduce syslog messages when starting ubus-cli/ba-cli/...

## Release v0.4.1 - 2023-11-18(17:44:14 +0000)

### Fixes

- Add timestamp for events printed in AMX CLI

## Release v0.4.0 - 2023-11-06(09:21:33 +0000)

### New

- Refactor libamxo - libamxp: move fd and connection management out of libamxo

## Release v0.3.5 - 2023-10-30(16:36:21 +0000)

### Fixes

- [amx-cli]List doesn't provide output when redirected to file

### Other

- [AMX] libamxo linker issue

## Release v0.3.4 - 2023-10-12(16:37:10 +0000)

### Fixes

- Fix license headers in files

## Release v0.3.3 - 2023-10-09(08:24:55 +0000)

### Changes

- [ubus-cli/ba-cli] Read commands from stdin

## Release v0.3.2 - 2023-09-14(20:16:32 +0000)

### Fixes

- [amx-cli]Redirecting output to file prints error

## Release v0.3.1 - 2023-07-25(15:55:49 +0000)

### Fixes

- bus agnostic cli must be able to auto detect backends and sockets

## Release v0.3.0 - 2023-06-01(20:30:17 +0000)

### New

- [PCB] add option to pcb-cli to mimic operator (usp) access

## Release v0.2.28 - 2023-05-26(08:38:12 +0000)

### Fixes

- Link with libyajl

## Release v0.2.27 - 2023-05-25(06:10:15 +0000)

## Release v0.2.26 - 2023-05-24(18:20:41 +0000)

## Release v0.2.25 - 2023-03-28(08:55:37 +0000)

### Fixes

- [AMX] gdbgui provides full path of cli module

## Release v0.2.24 - 2023-02-16(11:49:12 +0000)

### Fixes

- Issue:  HOP-2086 [WNC] Keyboard arrows not working within ubus-cli on serial

## Release v0.2.23 - 2023-02-13(15:28:39 +0000)

### Fixes

- [AMX][CLI] Scripting: ubus-cli or ba-cli doesn't output anything without a pseudo-terminal

## Release v0.2.22 - 2022-05-23(14:42:26 +0000)

### Fixes

- [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected

## Release v0.2.21 - 2022-04-06(12:54:20 +0000)

### Fixes

- no-colors should be set to true by default

## Release v0.2.20 - 2021-11-18(13:22:55 +0000)

## Release v0.2.19 - 2021-10-29(05:21:50 +0000)

## Release v0.2.18 - 2021-10-13(12:43:09 +0000)

### Other

- [CI] Update autogenerated files

## Release v0.2.17 - 2021-08-23(11:41:36 +0000)

- Issue: ambiorix/applications/amx-cli#20 Exit on CTRL+D instead of CTRL+C

## Release v0.2.16 - 2021-08-02(13:27:24 +0000)

## Release v0.2.15 - 2021-07-22(12:28:22 +0000)

### Fixes

- SIGTERM must correctly be handled in eventloop

## Release v0.2.14 - 2021-06-28(14:27:43 +0000)

### Fixes

- Missing description in baf file

## Release v0.2.13 - 2021-06-15(09:44:15 +0000)

## Release v0.2.12 - 2021-06-11(13:41:20 +0000)

- Update README.md with test section

## Release v0.2.11 - 2021-06-08(18:04:03 +0000)

### Fixes

- [tr181 plugins][makefile] Dangerous clean target for all tr181 components

## Release v0.2.10 - 2021-06-03(14:01:07 +0000)

### Fixes

- Add linker flags for libamxd and libyajl

## Release v0.2.9 - 2021-06-03(09:34:34 +0000)

### Fixes

- The CFLAGS and LDFLAGS must be extended with sources from the STAGINGDIR

## Release v0.2.8 - 2021-06-03(08:06:06 +0000)

### Fixes

- Incorrect config variable prevents building in sah sop

## Release v0.2.7 - 2021-06-02(09:26:21 +0000)

### Fixes

- Incorrect config variable prevents building in sah sop

## Release v0.2.6 - 2021-05-31(11:24:09 +0000)

## Release v0.2.5 - 2021-05-09(21:09:35 +0000)

### Fixes

- Compilation error with bcm9xxx-aarch64-linux-4.19-gcc-9.2-glibc2.30 toolchain

### Changes

- Use common macros from libamxc

## Release v0.2.4 - 2021-04-25(17:19:15 +0000)

### Fixes

- Handle ampx events in playback

## Release v0.2.3 - 2021-04-21(10:30:31 +0000)

### Fixes

- Isse: #7 Call exit method of all cli modules at exit
-  when switching modules the deactivate method of the current must be called

## Release 0.2.2 - 2021-04-15(20:21:41 +0000)

### Changes

-  remove fakeroot dependency on host to build WRT image 

## Release 0.2.1 - 2021-04-15(11:10:09 +0000)

### Fixes

- VERSION_PREFIX is needed in buildfiles 

## Release 0.2.0 - 2021-04-14(13:22:24 +0000)

### New

- Add unit-test and documentation

## Release 0.1.0 - 2021-04-08(22:10:43 +0000)

### Changes

- Move copybara to baf

## Release 0.0.9 - 2021-03-15(20:53:58 +0000)

### Fixes

- Corrects readme

## Release 0.0.8 - 2021-02-26(17:55:26 +0000)

### Changes

- Migrate to new licenses format (baf)

## Release 0.0.7 - 2021-01-31(17:49:03 +0000)

### Fixes

- Application depends on libamxj, libamxt, libamxm

## Release 0.0.6 - 2021-01-19(07:56:06 +0000)

### New

- Auto generate make files using build agnostic file (baf)

### Fixes

- Failing commands should not return 0
- Recording of commands is not working when cli module uses sigalrm

## Release 0.0.5 - 2021-01-09(11:24:06 +0000)

### Fixes

- Listen sockets

### Changes

- Clear prompt on exit
- Asynchronous completion

## Release 0.0.4 - 2021-01-04(20:08:13 +0000)

### New

- Adds option wait for record playback
- Adds logging for commands and reply
- Adds silent command

### Changes

- Value parsing
- Option parsing

### Fixes

- Fixes completion and execution
- Fixes invalid memory access

## Release 0.0.3 - 2020-11-29(18:58:14 +0000)

### Changes

- Update readme
- Add dependency

### Fixes

- Fix install and release target
- Add missing files

## Release 0.0.2 - 2020-11-15(15:29:52 +0000)

### New

- Executes command given on command line

### Changes

- Changes playback behavior

## Release 0.0.1 - 2020-11-15(12:18:54 +0000)

### New

- fine-grained opt-out configuration options
- module amx - variables, aliases, and more
- module history - clear, load and save command history
- module record - record and playback commands
- module addon - load or remove amx cli add-ons
