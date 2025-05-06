# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]


## Release v5.2.0 - 2024-11-12(15:41:36 +0000)

### Other

- USP bus get_instances return

## Release v5.1.4 - 2024-10-17(12:18:37 +0000)

### Other

- - Ambiorix USP-BE threshold support
- - Ambiorix USP-BE threshold support

## Release v5.1.3 - 2024-09-20(12:14:41 +0000)

### Other

- CI: Disable squashing of open source commits

## Release v5.1.2 - 2024-09-19(13:25:01 +0000)

### Other

- - [USP][CDRouter][Random] Some datamodel path are missing in USP hl-api tests
- - [USP][CDRouter][Random] Some datamodel path are missing in USP hl-api tests

## Release v5.1.1 - 2024-07-30(12:02:34 +0000)

### Fixes

- [AMX][USP] Avoid double USP connections

## Release v5.1.0 - 2024-07-25(07:57:13 +0000)

### Fixes

- Remove legacy subscribe interface function in favor of subscribe_v2
- [USP] Double operation complete notifications

### Changes

- Make requires-device-prefix default value configurable

## Release v5.0.0 - 2024-07-17(14:55:54 +0000)

### Other

- Make USP backend more compliant with USP specification

## Release v4.6.0 - 2024-06-10(08:12:03 +0000)

### New

- Allow data model translations in USP backend

## Release v4.5.4 - 2024-06-06(19:12:42 +0000)

### Fixes

- Detection of bus context (USP) requires path to be dotted

## Release v4.5.3 - 2024-06-03(06:05:28 +0000)

## Release v4.5.2 - 2024-05-21(20:38:59 +0000)

### Other

- [Security][ambiorix]Some libraries are not compiled with Fortify-Source

## Release v4.5.1 - 2024-05-13(14:11:31 +0000)

### Fixes

- AMX USP backend must remove braces from operate name when received

## Release v4.5.0 - 2024-05-08(14:44:10 +0000)

### New

- Implement custom has function

### Changes

- Methods must be called with braces

## Release v4.4.1 - 2024-05-06(14:15:07 +0000)

### Other

- Update dependencies in .gitlab-ci.yml

## Release v4.4.0 - 2024-04-18(12:22:45 +0000)

### Changes

- Allow multiple registrations for the same bus context

## Release v4.3.0 - 2024-03-27(08:45:46 +0000)

### New

- [USP] Add register retry mechanism

## Release v4.2.2 - 2024-01-26(10:27:45 +0000)

### Fixes

- [USP] Issue with asynchronous calls

## Release v4.2.1 - 2024-01-12(10:40:55 +0000)

### Fixes

- [USP] amxb_usp_poll_response returns with error randomly

## Release v4.2.0 - 2024-01-08(16:08:14 +0000)

### New

- Allow creating subscriptions via LocalAgent.Subscription dm

## Release v4.1.3 - 2023-12-13(16:02:34 +0000)

## Release v4.1.2 - 2023-12-12(11:31:53 +0000)

### Fixes

- Deferred calls are cleaned up before everything is done

### Changes

- [USP] Backend must be able to handle interleaved messages

## Release v4.1.1 - 2023-11-27(14:19:25 +0000)

### Changes

- [USP] It must be possible to configure capabilities

## Release v4.1.0 - 2023-11-22(08:08:34 +0000)

### New

- [USP] Set up communication with obuspa

## Release v4.0.7 - 2023-11-07(13:10:16 +0000)

### Fixes

- Fix license headers in files

### Changes

- [USP] All plugins on host connect to the USP agent socket

## Release v4.0.6 - 2023-10-13(09:56:44 +0000)

### Fixes

- Block SIGALARM during USP socket polling to avoid interrupting it

## Release v4.0.5 - 2023-09-06(07:47:50 +0000)

### Other

- Update dependencies in .gitlab-ci.yml

## Release v4.0.4 - 2023-08-31(12:47:58 +0000)

### Fixes

- [USP] Remove handshake from connect

## Release v4.0.3 - 2023-08-29(11:01:52 +0000)

### Fixes

- USP UDS connection is complete when handshake is done

## Release v4.0.2 - 2023-08-18(09:49:56 +0000)

### Other

- Remove redundant line of code

## Release v4.0.1 - 2023-07-11(07:00:13 +0000)

### Other

- Update dependencies in .gitlab-ci.yml

## Release v4.0.0 - 2023-07-10(12:12:16 +0000)

### Breaking

- [IMTP] Implement IMTP communication as specified in TR-369

## Release v3.0.3 - 2023-05-22(09:06:33 +0000)

### Fixes

- [USP] Agent stuck doing blocking amxb_resolves

## Release v3.0.2 - 2023-05-09(11:24:46 +0000)

### Fixes

- [USP] Get with depth not working as expected

## Release v3.0.1 - 2023-05-03(12:21:16 +0000)

### Other

- Update dependencies in .gitlab-ci.yml

## Release v3.0.0 - 2023-05-02(11:20:52 +0000)

### Breaking

- Subscriptions with the USP backend no longer work

## Release v2.5.1 - 2023-01-13(07:35:23 +0000)

## Release v2.5.0 - 2023-01-11(15:55:28 +0000)

### New

- [USP][AMX] Add support for asynchronous invokes to USP backend

### Fixes

- Handle command output args for non-backend processes

### Changes

- [KPN][USP] max_depth has no effect on the Get Message

### Other

- Update dependencies in .gitlab-ci.yml

## Release v2.4.3 - 2022-11-25(08:15:39 +0000)

### Fixes

- [USP][AMX] Return variant of get_supported must be updated

## Release v2.4.2 - 2022-10-20(07:33:43 +0000)

### Fixes

- [USP] Fix unit tests of mod-amxb-usp after changes to libamxd

## Release v2.4.1 - 2022-09-06(06:46:26 +0000)

### Fixes

- [USP] Upstep version of libuspprotobuf

## Release v2.4.0 - 2022-09-05(11:43:56 +0000)

### New

- It must be possible to update EIDs after a signal

## Release v2.3.0 - 2022-08-29(14:02:59 +0000)

### New

- Update USP back-end with registrations

## Release v2.2.1 - 2022-07-08(07:05:00 +0000)

### Changes

- [USP] It must be possible to set string parameters starting with a plus

## Release v2.2.0 - 2022-06-07(06:56:58 +0000)

### New

- [USP Agent] Implement GetInstances message

### Other

- [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when...

## Release v2.1.2 - 2022-04-29(14:50:37 +0000)

### Other

- Enable auto opensourcing

## Release v2.1.1 - 2022-03-23(16:02:32 +0000)

### Fixes

- Macro IS_SET should be defined here

## Release v2.1.0 - 2022-02-14(14:32:18 +0000)

### New

- [USP Agent] Handle allow partial set

### Fixes

- Incoming messages could be read partially

### Other

- [USPAgent] The USP Agent must be opensourced to gitlab.com/soft.at.home

## Release v2.0.0 - 2021-12-16(12:42:59 +0000)

### Breaking

- Update set interface according to changes in libamxb

### Other

- Issue: ambiorix/modules/amxb_backends/amxb_usp#20 Back-ends should use versioned dependencies

## Release v1.1.0 - 2021-10-12(14:13:23 +0000)

### New

- USP back-end should not be used for object discovery

## Release v1.0.5 - 2021-08-24(09:16:18 +0000)

### Other

- Add debian package files

## Release v1.0.4 - 2021-08-03(13:23:37 +0000)

### Fixes

- openwrt build fails with stdio.h

### Changes

- usp backend can not support empty paths with invoke

## Release v1.0.3 - 2021-07-13(07:09:48 +0000)

### Fixes

- [tr181 plugins][makefile] Dangerous clean target for all tr181 components

### Changes

- Add wait_for function to back-end functions struct

## Release v1.0.2 - 2021-04-26(13:59:57 +0000)

### Changes

- Update set RPC to new libusp API

## Release 1.0.1 - 2021-04-15(10:03:19 +0000)

### Fixes

- VERSION_PREFIX is needed in buildfiles 

## Release 1.0.0 - 2021-04-09(07:15:11 +0000)

### Added

- Write a README
- Allow a TLV of type topic to be transmitted if one is present in the odl config options

### Changed

- Upstep minimum amxb version needed
- Build-in data model RPC methods can conflict with domain specific RPCs [new]
- When a client subscribes for events future events must be taken into account
- [yocto] non -dev/-dbg/nativesdk- package contains symlink .so

## Release 0.0.3 - 2021-03-16(14:58:48 +0000)

### Added

- Implement invoke

### Changed

- The return variant from an operation should be a list

## Release 0.0.2 - 2021-02-26(15:24:27 +0000)

### Added

- Implement subscribe and unsubscribe
- It must be possible to get the local EndpointID from the odl file

### Changed

- Adapt backend to USP add response variant format
- Add correct linking directory for compiling on a device
- Migrate to new licenses format (baf)
- Implement amxb_read_raw funtion
- Update baf.yml

## Release 0.0.1 - 2021-02-01(16:07:50 +0000)

### Added

- Initial implementation of the ambiorix USP backend
- This is the first implementation of the USP backend for amxb. Not all features will be supported yet. The currently implemented operators are: get, set, get supported, add, delete

