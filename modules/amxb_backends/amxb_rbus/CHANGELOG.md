# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## Release 1.3.4 - 2024-08-16(12:00:00 +0000)

### Fixes

- Fix: segmentation fault when deleting row using rbusTable_removeRow

## Release 1.3.3 - 2024-08-14(12:00:00 +0000)

### Fixes

- Fix: Correctly apply search path filter and depth
- Fix: Subscribing on multi-instance objects is not providing events
- Fix: Add instances on none-search paths is always failing

### Updates

- Doc: rbus-interal.md add documentation for all implementation files, include sequence diagrams

## Release 1.3.2 - 2024-07-09(12:00:00 +0000)

### Fixes

- Subscriptions:
  - Fix: do not recurse objects when subscription is taken on specific event
  - Fix: Rework subscription handling to make sure not too many events are published

### Added

- Add extra unit-test, verify if multiple subscriptions are correctly handled

### Updates

- Readme: Update readme.md and changelog.md
- CI Pipeline: Add gitlab ci/cd pipeline

## Release 1.3.1 - 2024-06-26(10:35:00 +0000)

### Changed

- Code cleanup - remove duplicate code

### New

- Adds extra unit-test:
  - Test all data conversions
  - Extra test cases for adding instances (rows)

## Release 1.3.0 - 2024-06-20(12:50:00 +0000)

### Milestone: Alpha-release

- feature complete - all needed features are available.

## Release v1.2.0 - 2024-05-30(09:34:11 +0000)

### Added:

- ambiorix (amxb) client (consumer) back-end interface functions:
  - amxb_rbus_gsdm
  - amxb_rbus_get_instances

### Fixes

- Race condition - not correctly creating the confitional wait (multithreading/concurency)
- Correctly handle (un)subscribe and eventing
- Timestamp conversions
- Error code reporting - improved error code conversions

## Release v1.1.0 - 2024-05-04(18:06:16 +0000)

### Added:

- ambiorix (amxb) client (consumer) back-end interface functions:
  - amxb_rbus_get
  - amxb_rbus_set
  - amxb_rbus_add
  - amxb_rbus_del
  - amxb_rbus_describe
  - amxb_rbus_list
  - amxb_rbus_wait_for
  - amxb_rbus_capabilities
  - amxb_rbus_has

### Changed

- Minimum required version of libamxb set to v4.7.0

## Release v1.0.1 - 2024-03-27(10:29:57 +0000)

### Added:

- Creating RBus connections
- Multithreading concurency handling
- Ambriorix Data Model registration to RBus
- Data Conversions from RBus to amx_var and from amx_var to Rbus
- Implementation of RBus callback handlers

## Older releases

- All release before v1.0.0 are considered legacy and shouldn't be used any more