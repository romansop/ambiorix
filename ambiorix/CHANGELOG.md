# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]


## Release v7.1.0 - 2024-11-25(10:44:13 +0000)

## Release v7.0.0 2024-05-14(05:47:06 +0000)

### Other

- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Take counter parameter attributes into account
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Fix instance counter position
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): The amxo parser struct contains unused fields and should be removed
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [amxd] amxd_trans_set_param with NULL value makes the amxd_trans_apply failIgnore null variants
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): Avoid g++ compiler warning for new .add_value variant function pointer
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Avoid g++ compiler warning for new .add_value variant function pointer
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Fix typo in documentation
- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj): Avoid g++ compiler warning for new .add_value variant function pointer
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Improve documentation of amxc_ts_to_tm functions

### New

- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): support setting absolute time in unittests
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Add `+=` syntax for appending lists in config sections
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): add amxc_var_add_value()

## Release v6.12.0 2024-04-19(19:25:43 +0000)

### New

- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Pass configuration to libamxb
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Make timeouts configurable.

### Other

- [dmtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/applications/dmtui): Opensource component
- [libamxtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/libraries/libamxtui): Make available on gitlab.com
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [ba-cli]When set fails the error must be printed instead of no data found
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Hosts tests NOK on Safran
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): After reboot all hosts are disconnected (AKA amb timeouts)
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Parameters must also be notated in xml using supported path notation
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): [AMX][Documentation] Allow to configure the proxied datamodel path in documentation
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): amxrt app exits immediately on LXC due to capdrop error
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Improve documentation on data model discovery
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Update reference to mod-dmext
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Update documentation on mutable unique keys
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Datamodel application generic performance improvement
- [libamxm](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxm): [AMXM] Implement amxm_so_error to debug failed amxm_so_open

### Changes

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Make amxb timeouts configurable
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): Make amxb timeouts configurable
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Allow multiple registrations for the same bus context

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [AMX] Eventing no longer works for mapped native pcb objects
- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): nano seconds calculated wrong when updating clock
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [AMX] Config variable not reset when backends are freed
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Fix test dependencies
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Fix nested list parsed as not-nested list
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): fix string literal with leading/trailing spaces parsed as without
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Expose getter and eval binary expression tree functions
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Fix inconsistency in converting integer signedness
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Fix crash on converting string variant without buffer to timestamp

## Release v6.11.0 2024-03-01(09:55:27 +0000)

### Other

- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): [AMX] Lua bindings should only connect to default sockets when the backend is loaded
- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Update DMObject to pass pamx connection as argument
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [ba-cli] "ubus-protected" not working anymore
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [ba-cli]ba-cli should connect to all available bus systems
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [cli]It must be possible to connect to ipv6 lla
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [amx][cli] Add syntax to filter parameters list
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [amx][cli] Add syntax to filter parameters list
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [amx][cli] Add syntax to filter parameters list
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [CI][HTTPAccess] Plugin not starting due to race condition with Device.Users
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [PCB] Add parameter_not_found error code
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): uspagent -D crash
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [ba-cli] Events are not showed when requested using pcb-cli with gsdm command
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [amx][cli] Add syntax to filter parameters list
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [amx][cli] Add syntax to filter parameters list
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [prpl] Authorization header uses 'bearer' instead of 'Bearer'
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [Prpl] Bad rpath makes build fail on Yocto LCM build
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [amx][cli] Add syntax to filter parameters list
- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): read json to variant
- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): Mock the clock
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Amxrt should connect to bus systems before daemonizing
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Improve save and load functionality
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [BaRt] Failed to handle multiple keys
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): tr181-device crashes when using combination of proxy and parameter  synchronistation
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [amx][cli] Add syntax to filter parameters list
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [cli]It must be possible to connect to ipv6 lla
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [amx][cli] Add syntax to filter parameters list - fix version check unit test
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [amx][cli] Add syntax to filter parameters list
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Update amxb_set documentation
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): update documentation for amxb_wait_for_object
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): update documentation for amxb_wait_for_object
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Crash pwhm on lb6
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [Ambiorix] Amxb_set and amxb_del should return more info about each parameter
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): call destroy callback on root object destruction
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): [cli]Always add last command at the bottom of the history
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): CLONE - Functional issues linked to parenthesis escaping in the data model
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Functional issues linked to parenthesis escaping in the data model
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): [amx-cli] Allow proper escaping of variable in cli for input and display

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [amx-cli] Allow proper escaping of variable in cli for input and display
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Do not create a subscription if an object is found and the notify flags are not set
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Crash in tr181-device
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Fix a memory growth
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Problems with getdebug on Nokia prpl mainline
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Error codes are lost when using the pcb mapper
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [PCB Backend] Incoming requests are handled while waiting for a response
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [amx]Datamodels accessed through pcb must respect the pcb notification request flags
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): Adapt description of ambiorix packages
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Correct loop detection for object-changed events
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): [Amxs] Apply sync direction to all child entries recursively
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): [amx-cli] Allow proper escaping of variable in cli for input and display
- [libamxm](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxm): Adapt description of ambiorix packages
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [WNC-CHR2][LCM] SoftwareModules cannot be found
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Adapt description of ambiorix packages
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): [amx-cli] Allow proper escaping of variable in cli for input and display

### Changes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Problems with getdebug on Nokia prpl mainline
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [AMX_PCB] subscribing before creating object, do not get events.
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [amx]Datamodels accessed through pcb must respect the pcb notification request flags
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): [ambiorix][libamxs]Reduce initial sync size to minimum depth for synchronization context
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): [ambiorix][synchronization]The synchronization library must also synchronize sub-objects when an instance is added
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [ambiorix][synchronization]The synchronization library must also synchronize sub-objects when an instance is added

### New

- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [AMX] Add ends with expression operator

## Release v6.10.1 2023-12-14(15:03:15 +0000)

### Other

- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Add example lua script

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [ba-cli] bus URIs duplicated
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Update ba-cli bus commands documentation
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): Fix the linker again, errors were still encountered with the previous one
- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): [amxut] use transaction to set dm parameters
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [amxrt] Need to load backends from multiple directories
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [AMX] Add extra NULL pointer checks
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Inconsistent status types in amxb_get_multiple and amxb_set_multiple
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Make it possible to _exec functions with braces

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Add and update documentation
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add and update documentation

### New

- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): ambiorix lua bindings must be extended
- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): ambiorix lua bindings must be extended
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Add parameter constraints to generated xml files
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Add parameter constraints to generated xml files

## Release v6.10.0 2023-11-30(07:18:04 +0000)

### Changes

- [dmtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/applications/dmtui): Add option to select which root objects are shown
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [ServiceID][WebUI] set correct acl settings for webui

### New

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Add posibility for action handlers to provide a description
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add posibility for action handlers to provide a description

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Modify libbart for PCB Components Compatibility with PCM-Manager
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [cli] Reduce syslog messages when starting ubus-cli/ba-cli/...
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [AMX] Cache size is not updated correctly
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): tr181-usermanagement segfault during startup
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Update and extend documentation
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Update and extend documentation

## Release v6.9.1 2023-11-22(06:50:41 +0000)

### Other

- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): Add the BSD-2-Clause-Patent license

### Fixes

- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): Add timestamp for events printed in AMX CLI
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): [AMX] Only return objects that contain the parameter
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): pwhm datamodel load failure after reboot
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Only return objects that contain the parameter
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Only return objects that contain the parameter
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): Ubus-cli does not start if no tty is available

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Add timestamp for events printed in AMX CLI

## Release v6.9.0 2023-11-15(07:45:38 +0000)

### Other

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP] [Add_msg] Wrong error code in the response when a Add message is requested with an invalid parameter or value.

### Changes

- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [amxrt] Need to load backends from multiple directories

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Wifi NOK on Safran prpl mainline
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Wifi NOK on Safran prpl mainline
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Full objects are replied when it is not needed
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [DataModel][TR-181]"Device.DeviceInfo.VendorConfigFile.Date" parameter type Not as expected
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [amxrt][no-root-user][capability drop] failed to add capabilities for a forked process
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [AMX] Only detect sockets for loaded backends
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [Amxo] Cannot create synchronization template without adding "{i}." to the objects

## Release v6.8.1 2023-11-09(10:44:13 +0000)

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [amx-cli]Completion on empty or root object is not working when using multiple connections

## Release v6.8.0 2023-11-09(06:58:39 +0000)

### Fixes

- [libamxtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/libraries/libamxtui): [dmtui]Common Improvements
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [amx-cli]Completion on empty or root object is not working when using multiple connections
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): [AMX] libamxo linker issue

### New

- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): Refactor libamxo - libamxp: move fd and connection management out of libamxo
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Refactor libamxo - libamxp: move fd and connection management out of libamxo
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Refactor libamxo - libamxp: move fd and connection management out of libamxo
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): - [prpl][libamxc] amxc_set_to_string only use space as separator

### Changes

- [dmtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/applications/dmtui): [dmtui]Common Improvements
- [mod-dm-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-dm-cli): Refactor libamxo - libamxp: move fd and connection management out of libamxo
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Refactor libamxo - libamxp: move fd and connection management out of libamxo
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): - [UI][LAN versus WAN admin] Allow the WAN and the LAN UI to have different ACL(users)
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Refactor libamxo - libamxp: move fd and connection management out of libamxo
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [CDROUTER][USP] usp_conformance_10_13 : Cannot add a new MQTT BulkData Profile
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): Refactor libamxo - libamxp: move fd and connection management out of libamxo
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): Refactor libamxo - libamxp: move fd and connection management out of libamxo

## Release v6.7.1 2023-10-31(06:09:33 +0000)

### Changes

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [ConnDB][WebUI] Create a webui for the connection database

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [amx-cli]List doesn't provide output when redirected to file
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [amx-cli]List doesn't provide output when redirected to file
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): ACL checking broken for add  without search path
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [libamxb]Possible memory leak when async request are open on local datamodel when exiting
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): [amx-cli]List doesn't provide output when redirected to file

### Other

- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [AMX] libamxo linker issue
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): - [prpl][libamxa] Create an object using a search path isn't allowed

## Release v6.7.0 2023-10-27(14:32:50 +0000)

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Extend help of get command `?`
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [doc]Describe how to access ubusd as non-root user
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Include extensions directory in ODL

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): pcb_cli parameter query returns error when query-ing amx parameter
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [amx-fcgi]Segfault detected with unit-tests
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Fix batch parameter sync direction
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [libamxb]Segfault when local async-call in progress and conection lost to bus system
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [libamxb]Segfault when local async-call in progress and conection lost to bus system

### New

- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): [libamxut] add dm support for boolean parameters
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Make it possible to declare synchronization templates in odl files
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Make it possible to declare synchronization templates in odl files

## Release v6.6.1 - 2023-10-20(06:27:36 +0000)

## Release v6.6.0 2023-10-20(06:04:07 +0000)

### Other

- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Issue ST-1184 [amxb][amxc][amxo][amxrt] Fix typos in documentation
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Issue ST-1184 [amxb][amxc][amxo][amxrt] Fix typos in documentation
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Issue ST-1184 [amxb][amxc][amxo][amxrt] Fix typos in documentation
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Issue ST-1184 [amxb][amxc][amxo][amxrt] Fix typos in documentation

### Fixes

- [mod-dm-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-dm-cli): Fix license headers in files
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Fix license headers in files
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Fix license headers in files
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): Fix license headers in files
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): Fix license headers in files
- [amxb-inspect](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxb-inspect): Fix license headers in files
- [amxo-xml-to](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-xml-to): Fix license headers in files
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Fix license headers in files
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): Fix license headers in files
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [libamxo]Make it possible to define object synchronisation in odl
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Set entry pointer to NULL if initialization fails
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [libamxb]Memory leak - closed pending asynchronous request are not freed when disconnecting
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [WiFi] Cannot use a WiFi password containing the character "
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Fix license headers in files
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [PRPL OJO NOKIA][GMAP] The GMap server crashes when a new wired device, like an IP printer or IP camera, is plugged in.
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [libamxp]Memory when parsing invalid list value

### New

- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Add support for read-only parameters in a local datamodel
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Add support for named and search paths in sync ctx
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [libamxo]Make it possible to define object synchronisation in odl
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): [libamxc] Some characters in amxc string can have special purpose and it must be possible to escape them

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [mod-ba-cli]The cli is always resolving environment variables and configuration variables
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [libamxo]Make it possible to define object synchronisation in odl
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [libamxo]Update odl documentation and API regarding object synchronisation

## Release v6.5.3 2023-10-09(20:55:16 +0000)

### Fixes

- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Avoid infinite event loop in bidirectional parameter sync

## Release v6.5.2 2023-10-09(16:02:09 +0000)

### New

- [dmtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/applications/dmtui): [dmtui]Add support for data model RPC invoke

### Other

- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Remove duplicate CI variable
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Develop generic prpl voiceactivation module

### Changes

- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [ubus-cli/ba-cli] Read commands from stdin

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): It must be possible to call native ubus object methods
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): amxb_describe() returning non expected results over the pcb bus
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): High CPU and Memory usage
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [tr181-device] Crash during boot and shutdown
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Don't return listen context from who_has
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Improve and optimize some parts for speed
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): amxb_describe() returning non expected results over the pcb bus
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): [ubus-cli/ba-cli] Pasting multiple commands trims first letters
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): amx-cli crash when unhandled key strokes are pressed
- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj): Improve and omptimize some parts for speed

## Release v6.5.1 2023-09-26(09:02:46 +0000)

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [tr181-device] Crash during boot
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): amx path queries to pcb bus are missing entries
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Fix license headers in files
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): - [prpl][amx-fcgi] amx_fcgi_http_subscribe return wrong status code
- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): amxut new dm assert always fails on subobject
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Fix license headers in files
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Fix license headers in files
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Fix license headers in files
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [PRPL][SAFRAN] Wan is not up after reset, only after additional reboot
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Subscriptions on mib objects not correctly removed
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Fix license headers in files
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): [CHR2fA] ubus-cli console generate a segmentation fault
- [libamxm](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxm): Fix license headers in files
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Fix license headers in files
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Fix license headers in files

### Changes

- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [tr181-device] Crash during boot
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Create directory before user privilege and capability dropping

### Other

- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [Ambiorix] Build error on KPN SW2 caused by opensource_libcapng=gen_v0.8.2

## Release v6.5.0 2023-09-15(06:33:14 +0000)

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [tr181-device] Crash during boot
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): - [amx-fcgi] admin user cannot add to Device.Routing.Router.[Alias=='main'].IPv4Forwarding.
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [amx-cli]Redirecting output to file prints error
- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): [libamxut] Handle events before cleanup
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): - [ACL][libamxa] When a search path is allowed but not its fixed part amxa_get fail
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): amxp singal pipefds should be closed when spawning child processes

### Changes

- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [libamxp]Scheduler api should be more tolerant regarding spaces in days of week list
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): allow amxc_var_dump with FILE*

### New

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [amx-fcgi] Add reporting of number of unsuccessful login attempts
- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): Issue HOP-4307 [libamxut] Expand amxut functionality

## Release v6.4.1 2023-09-01(09:40:21 +0000)

### Fixes

- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): ODL parser fails to open file in root directory
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [libamxp] When enable or disable a scheduler it is possible that signals are not emitted or triggered

## Release v6.4.0 2023-08-23(10:34:13 +0000)

### Fixes

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): - [prpl][user-management] Users role paths are not tr181 paths

### Other

- [amx-async-call](https://gitlab.com/prpl-foundation/components/ambiorix/examples/baapi/async_call): use new AMXRT prefix macro
- [amx-greeter-app](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_app): - solve minor makefile errors and use AMXRT_ macro prefix
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [amx-fcgi][SWUpdate] Upload command should stream file directly to the disk
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): - Refactor libamxrt for compatibility with prplmesh

### New

- [amx-wait-for](https://gitlab.com/prpl-foundation/components/ambiorix/examples/baapi/wait_for): Use new AMXRT prefixed macros
- [amx-subscribe](https://gitlab.com/prpl-foundation/components/ambiorix/examples/baapi/subscribe): use new AMXRT prefix macro
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): Use new AMXRT prefixed macros

## Release v6.3.1 2023-08-21(06:11:32 +0000)

### Other

- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): Support (u)int8+(u)int16 datamodel parameter assert

### Fixes

- [dmtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/applications/dmtui): dmtui crashes when started
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Memory leak in ba-cli with auto-connect functionality
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Do not remove readme file when opesourcing
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Update - extend odl documentation
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add documentation about data model modules - extensions
- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj): casting empty jstring to any causes crash

## Release v6.3.0 2023-07-28(13:05:01 +0000)

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [ba-cli] output in 10s when ba-cli query invoked from console

### Other

- [dmtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/applications/dmtui): [AMX] Disable docgen for dmtui
- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): provide simple .odl loading and datamodel parameter asserts
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Make the AMX Bus Rust API safe-to-use

## Release v6.2.3 2023-07-26(14:42:50 +0000)

### Other

- [amx-wifi-scheduler](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/wifi-scheduler): Opensource example
- [amx-scheduler](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/amx-scheduler): Opensource example
- [amx-wait-for](https://gitlab.com/prpl-foundation/components/ambiorix/examples/baapi/wait_for): Opensource example
- [amx-async-call](https://gitlab.com/prpl-foundation/components/ambiorix/examples/baapi/async_call): Opensource example

## Release v6.2.2 2023-07-26(05:41:45 +0000)

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): bus agnostic cli must be able to auto detect backends and sockets
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): bus agnostic cli must be able to auto detect backends and sockets
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Update documentation for amxo_parser_get_config()

## Release v6.2.1 2023-07-25(11:10:02 +0000)

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): bus agnostic cli must be able to auto detect backends and sockets
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): bus agnostic cli must be able to auto detect backends and sockets
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Enforce coding style - no declarations after code
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Enforce coding style - no declarations after code
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Enforce coding style - no declarations after code
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Enforce coding style - no declarations after code
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): It is not possible to declare events in a mib
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): It is not possible to declare events in a mib
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): Enforce coding style - no declarations after code
- [libamxm](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxm): Enforce coding style - no declarations after code
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Expression function contains should be able to use a list of values
- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj): [libamxj]Add api doxygen documentation
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): When using GCC 12.2 extra compilation wanings pop-up

### Other

- [dmtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/applications/dmtui): Config option needs to match with sah name

## Release v6.2.0 2023-07-11(17:33:18 +0000)

### Changes

- [amx-greeter-plugin](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_plugin): [USP] GSDM should return whether commands are (a)sync
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [USP] GSDM should return whether commands are (a)sync
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP] GSDM should return whether commands are (a)sync

### Breaking

- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [USP] GSDM should return whether commands are (a)sync
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [USP] GSDM should return whether commands are (a)sync

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP] GSDM should return whether commands are (a)sync

## Release v6.1.0 - 2023-07-10(16:06:18 +0000)

## Release v6.0.2 2023-07-06(11:25:31 +0000)

### Changes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [AMX] Define default sockets in backends

### Fixes

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [amx-fcgi] Error 404 on REST API call on a empty table result
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [AMX] Add be data-uris to top level data-uris
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Plugins crashing because of segfault in ambiorix

### Other

- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [AMX] Define default sockets in backends

## Release v6.0.1 2023-06-28(17:30:08 +0000)

### Fixes

- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [AMX] Crash with new libamxrt

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Add requests with search paths are allowed
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Add requests with search paths are allowed

## Release v6.0.0 2023-06-27(20:27:12 +0000)

### New

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): use libamxrt
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): [amx][capabilities] Add support for reduced capabilities in ambiorix

### Fixes

- [amx-wifi-scheduler](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/amx-wifi-scheduler): Fix baf name
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Segmentation fault can occur when timers are added or deleted from within a timer callback

## Release v5.5.1 2023-06-21(14:01:32 +0000)

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [ubus-cli] extra rpc arguments not returned

## Release v5.5.0 2023-06-21(06:04:26 +0000)

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Remove destructor and use exit function
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): When using libamxrt a segfault can occur when closing the application
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): When a schedule item is changed and is currently started send a stop event

## Release v5.4.0 2023-06-19(07:50:33 +0000)

### New

- [amx-greeter-app](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_app): use libamxrt
- [amx-subscribe](https://gitlab.com/prpl-foundation/components/ambiorix/examples/baapi/subscribe): use libamxrt
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Add API documentation

## Release v5.3.0 2023-06-19(06:59:12 +0000)

### Other

- Add examples: wifi-scheduler, amx-scheduler, wait_for, async_call

## Release v5.2.2 2023-06-16(14:01:59 +0000)

### Fixes

- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): [amx][prpl]Implementation of the LANConfigSecurity module

### Other

- [libamxut](https://gitlab.com/prpl-foundation/components/ambiorix/tools/libraries/libamxut): [amx][prpl]Implementation of the LANConfigSecurity module
- [libamxrt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxrt): Opensource component

## Release v5.2.1 2023-06-15(11:06:34 +0000)

### Fixes

- [dmtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/applications/dmtui): Issue: Fix segmentation fault and improve fetching objects
- [libamxtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/libraries/libamxtui): Issue: Fix segmentation fault
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Add more unit tests
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Failing transactions not fully cleaned

### Other

- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Issue: ambiorix/libraries/libamxd#153 Sending an object event with object, eobject or path in the event data causes never ending loop

## Release v5.2.0 2023-06-02(08:11:13 +0000)

### Other

- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Update documentation

### New

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [AMX] Ambiorix should return the same error codes regardless of the used bus
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [AMX] Ambiorix should return the same error codes regardless of the used bus
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [TR181-DeviceInfo][FCGI] Add rpc to read/write a file
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [PCB] add option to pcb-cli to mimic operator (usp) access
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [USP] GSDM events need to be filtered out
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [AMX] Ambiorix should return the same error codes regardless of the used bus

### Fixes

- [mod-lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/mod-lua-amx): Add unit tests
- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Fix return values and throwing errors
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Regression in ubus back-end due to changes in error code passing.
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [TR181-DeviceInfo][FCGI] Add rpc to read/write a file
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Ambiorix should return the same error codes regardless of the used bus

## Release v5.1.1 2023-05-26(13:05:47 +0000)

### Fixes

- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): Link with libyajl
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Scheduler does not take correct duration into account when multiple schedules expire at the same moment

## Release v5.1.0 2023-05-25(16:34:03 +0000)

### Changes

- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [USP] Add specific error codes for get instances
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [USP] Must be able to call methods with named path

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [AMX] Get instances supports search paths
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [HTTPManager][Login][acl-manager] After firstboot acl group doesn't exist
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add documentation to default action implementations

### Other

- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): - [HTTPManager][Login][amx-fcgi] Create a session
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Documentation is missing for the get_instances operator
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Issue: ambiorix/libraries/libamxb#68 Documentation is missing for the get_instances operator

### New

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [amx-fcgi]Refactor sessions code to make it easier to run the provided web-ui example
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [Security][USP] Add ACLs for get instances to USP agent
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [USP] Add specific error codes for get instances
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [libamxp]Provide API for cron expression parsing and calculating next occurence

## Release v5.0.3 2023-05-15(05:48:15 +0000)

### Fixes

- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Use index paths when sending events after an odl file is parsed

### Other

- [libamxtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/libraries/libamxtui): Add unit tests to libamxtui
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): - [HTTPManager][Login] Increase unit test coverage
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): - [HTTPManager][Login][amx-fcgi] Create a session

## Release v5.0.2 2023-05-11(09:39:57 +0000)

### Fixes

- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Fix regression in amxd_path api
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [unit-tests] Complete and extend unit tests
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): [amxc] Fix missing semicolon

### Other

- [libamxtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/libraries/libamxtui): Issue: ambiorix/amxlab/tui/libraries/libamxtui#1 Wrong behavior and segfault when tryting to collapse tree item without sub-items
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Issue: ambiorix/applications/amx-fcgi#16 Document how to launch the example webui in a container
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): Create the voice interface between ubus and pcb (tr104i1/2 mapper)

## Release v5.0.1 2023-05-04(05:52:21 +0000)

### Fixes

- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Check if htables are initialized

## Release v5.0.0 2023-05-03(11:01:59 +0000)

### Fixes

- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Fixes tests, remove deprecated odl syntax
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): gsdm missing arguments for commands and events
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Fix unit-tests, update test odls remove deprecated syntax
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): has must fail on empty object lookup
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Fix unit tests, remove deprecated odl syntax
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Fixes unit tests, removed deprecated syntax in tests
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): [Amxs] Parameter callbacks are not called when an object instance is added
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): gsdm missing arguments for commands and events
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): ODL syntax documentation must be updated
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Changing optional parameters gives no events
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): access to DM LocalAgent. using amxb_get fail on EW

### Changes

- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): - [HTTPManager][Login][amx-fcgi] Create a session

### Breaking

- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Remove deprecated odl syntax and pcb compatibility syntax
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): gsdm missing arguments for commands and events

### New

- [dmtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/applications/dmtui): Make it possible to start dmtui in protected access mode
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): gsdm missing arguments for commands and events

## Release v4.5.0 2023-04-21(09:40:30 +0000)

### New

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/):  Implement capabilities and has function for object discovery

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [V12][USP] Push notification is not sent

## Release v4.4.6 2023-04-19(06:53:52 +0000)

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [V12][USP] Push notification is not sent

### Other

- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Fix a typo in the description of amxc_var_add_new_key_amxc_llist_t

## Release v4.4.5 2023-04-17(12:13:13 +0000)

### Fixes

- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [AMX] Unable to unsubscribe from search path

## Release v4.4.4 2023-04-09(05:47:14 +0000)

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [LCM] Error code is not forwarded correctly from LCM to USP in case of InstallDU using a non-existent EE
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP][AMX] GSDM needs a ValueChangeType
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [USP][AMX] GSDM needs a ValueChangeType
- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj): [AMX] Reduce write calls done in amxj_write

## Release v4.4.3 2023-04-07(18:51:19 +0000)

### Fixes

- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [TR069-manager] [wnc] No ManagementServer object for wnc

## Release v4.4.2 2023-04-03(14:20:10 +0000)

### Other

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [amx pcb]Not possible to retrieve list of parameters and functions of a template object
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP] Download() is handled synchronously

### New

- [dmtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/applications/dmtui): [TR181-EasterEgg] add snake as hidden easter egg
- [libamxtui](https://gitlab.com/prpl-foundation/components/ambiorix/amxlab/tui/libraries/libamxtui): [TR181-EasterEgg] add snake as hidden easter egg

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [Ambiorix] Implement function call with search path
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [amx pcb]Who has on search paths with pcb back-end is not working

## Release v4.4.1 2023-03-31(06:27:36 +0000)

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Onboarding Issue : Sometimes, PUBLISH is not sent
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Fix and complete unit tests
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): Listen socket connections are not removed from the event loop

## Release v4.4.0 2023-03-28(11:45:05 +0000)

### New

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [AMX] Take bus access into account for GSDM
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [AMX] Take bus access into account for GSDM
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): AMX : make it possible to to define event handlers directly under object
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Take bus access into account for GSDM

### Fixes

- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [AMX] gdbgui provides full path of cli module
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [uspagent] The 'Discovery' object in the dm cannot use gsdm

## Release v4.3.0 2023-03-27(11:55:18 +0000)

### Other

- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [KPN SW2][Security]Restrict ACL of guest user in Ambiorix datamodels
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [KPN SW2][Security]Restrict ACL of guest user in Ambiorix datamodels

### New

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [pcb] usp endpoint doesn't support pcb requests
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [pcb] usp endpoint doesn't support pcb requests

## Release v4.2.0 2023-03-18(12:20:47 +0000)

### Other

- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Create the voice interface between ubus and pcb (tr104i1/2 mapper)
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [odl] Regression conditional include does not take second if first is an empty directory

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [amxb_ubus]When calling a not existing method asynchronously no reply is given
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Improvements in amxp regarding amxp_dir_scan, timer documentation and slot disconnects

### New

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [amx-fcgi][SWUpdate] Add upload file functionality
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): Some control key sequences are incorrect defined

## Release v4.1.6 2023-03-13(12:55:18 +0000)

### New

- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [prpl][tr181-upnp] UPnP.Discovery and UPnP.Description are not implemented

### Other

- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [amxo] Saving and restoring the odl (config) section gives errors.
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): When odl includes an empty directory no error should be printed

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [Amxs][PCB] Amxs initial sync does not work with native pcb plugins
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Always execute the initial sync with protected access
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Mop test netmodel_02_check_interfaces_test.py::test_wan_isup_query_loop fails due to unexpected out argument

## Release v4.1.5 2023-03-02(10:20:18 +0000)

### Fixes

- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Protected objects should not be retrieved by gsdm

## Release v4.1.4 2023-02-28(15:12:14 +0000)

### Fixes

- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): amxo-cg fails to build correct include tree when parsing error occurs
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): Can write event must only be created once
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Object path verification should be done when subscribing
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [amxd] methods with amxd_aattr_out argument return NULL

### Changes

- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): [PRPL] amxo-cg does not compile with libxml2 version 2.10.2

## Release v4.1.3 2023-02-24(08:08:35 +0000)

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [ubus-cli] No alias paths in datamodel
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP] Get requests with search paths on pcb can fail

## Release v4.1.2 2023-02-22(19:30:45 +0000)

### Fixes

- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Ignore empty read filter
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): amxd_object_get_param_value should have its object parameter const
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): unbound does not start after reboot in tagged mode
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [CDROUTER][IPv6] Box sends 2 ICMPv6 RA when a RS is received on LAN

## Release v4.1.1 2023-02-21(19:12:17 +0000)

### Fixes

- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Fix conditions when `luaL_setfuncs` should be set
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): list operator using ubus backend without a path doesn't give a reply

## Release v4.1.0 2023-02-17(07:06:53 +0000)

### Fixes

- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Pylint issues with python3 bindings tests
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): When asking a parameter value in the cli, it returns more than expected.
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): list operator using ubus backend without a path doesn't give a reply
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): Issue:  HOP-2086 [WNC] Keyboard arrows not working within ubus-cli on serial
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): [WNC] Keyboard arrows not working within ubus-cli on serial
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [SSH][AMX] Processes spawned by dropbear instance managed SSH Manager ignores SIGINT

### Other

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Remove unneeded dependencies
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [amx-fcgi] generated datamodel documentation is empty

### New

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Add json output format to cli (ubus-cli, pcb-cli, ba-cli)

## Release v4.0.0 2023-02-14(08:40:46 +0000)

### Other

- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Add amxb_set_config support to lua bindings
- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Change python3 bindings tests to comply with usp spec
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): unblock a signal when disabling it with amxp_syssig_enable

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Replace openwrt ASCII art by prplOS one
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Issue NET-4423 [USP] Requirements for Get changed
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [USP] Requirements for Get changed
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [USP] Requirements for Get changed

### New

- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Add amxb_set_config support to python3 amx bindings
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [USP] Requirements for Get further clarified
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [USP] Requirements for Get further clarified

### Breaking

- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Improve logical expression parser

### Fixes

- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Fix Python3 amx bindings install target
- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): [amx] Error when we try a second reboot
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [AMXB] protected objects are listed for public connection
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP][CDROUTER] GetSupportedDM on Device.LocalAgent. using a single object, first_level_only true, all options presents no event
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP] Get requests starting with Device and containing a search path are failing on sop
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): amx-fastcgi crashes at boot when webui is installed in LCM container
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): amx-fastcgi crashes at boot when webui is installed in LCM container
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [AMX][CLI] Scripting: ubus-cli or ba-cli doesn't output anything without a pseudo-terminal
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): [MQTT][USP] Overlapping reconnects can cause a segmentation fault
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Data model functions arguments don't inherit attributes from mib
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): [AMX][CLI] Scripting: ubus-cli or ba-cli doesn't output anything without a pseudo-terminal
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Fix memory leakj wehn empty expression is used
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Issue: semgrep reports

## Release v3.2.6 2023-01-20(10:46:13 +0000)

### Fixes

- [amx-greeter-app](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_app): Issue: unit-test fails - Clean transaction object before re-using it
- [amx-greeter-plugin](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_plugin): Issue: unit-test failing - clean-up previous transaction before re-using the transaction object
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): An InstallDU call from USP is sometimes not called on SoftwareModules
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): [doc generation][NumberOfEntries field is not correctly put under the correct Object
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [AMXB] subscribing on different paths, still triggers all events cb
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [AMX][USP] A get on a protected parameter with public bus access must fail

### Other

- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected

## Release v3.2.5 2023-01-13(13:04:51 +0000)

### Fixes

- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [ambiorix] [regression] transaction time is dependent on the number of parameters within the object

## Release v3.2.4 2023-01-12(11:19:59 +0000)

### Fixes

- [amx-cpu-info](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/cpu-info): Unit tests fail due to SIGALRM
- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Path to sut.dm.IP.Interface['3'].IPv4Address['1'].proxy() doenst work
- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): [amx] Unable to get data model object after a reboot
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [AMX] ACL directory must be updated for mod-ba-cli
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [AMX][USP] Only filter objects when at least one parameter was filtered
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX][USP] A get on a protected parameter with public bus access must fail
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [ambiorix] transaction time is dependent on the number of parameters within the object
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): 0-timeout timer postponed when starting another longer timer

## Release v3.2.3 2022-12-14(17:53:18 +0000)

### Fixes

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): [amxrt] el_slot_wait_write_fd is added to late to signal `connection-wait-write`
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [multisettings] Using triggers is not effective

### Other

- [mod-lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/mod-lua-amx): Add debian packages for amx lua bindings
- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Add debian packages for amx lua bindings

## Release v3.2.2 2022-12-07(12:54:07 +0000)

### Fixes

- [mod-lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/mod-lua-amx): Starting should not fail if init script doesn't exist
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [USP] Allow invoking commands without braces
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add instance response is wrong when using key path notation on ubus
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): amxp_expr_buildf with 2 arguments only works in container, not on board

## Release v3.2.1 2022-11-28(14:52:48 +0000)

### Fixes

- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Example script can't find lamx_wait_for
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Fix wrong usage of function amxd_path_setf
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Fix wrong usage of function amxd_path_setf
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Fix wrong usage of function amxd_path_setf
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Issue #147 Add and update documentation for amxd_path API

### Other

- [mod-lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/mod-lua-amx): Change ldoc install to sudo
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Fix some typos in the documentation

## Release v3.2.0 2022-11-21(17:55:03 +0000)

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [AMX] Missing functions in GSDM response
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [AMX] Missing functions in GSDM response
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Invalid ACL file must result in an error
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Missing functions in GSDM response
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Missing functions in GSDM response
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Fix wrong comma in amxc_var_dump output
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Converting an empty string variant to a list should result in an empty list

### Other

- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add new error code amxd_status_not_supported
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Issue: ambiorix/libraries/libamxd#148 Add new error code amxd_status_not_supported

### New

- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [AMX] Extend libamxa for easier verification of RPC methods
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add function to retrieve parameter path

## Release v3.1.3 2022-11-17(07:26:54 +0000)

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Segmentation fault when stopping process
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Segmentation fault when stopping process
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Files passed with -i option are not handled as include files
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Get with search path returns too many results
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Update documentation of functions amxc_var_get_next, amxc_var_get_previous and amxc_var_get_parent

### New

- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Install object monitor scripts

## Release v3.1.2 2022-11-14(15:09:52 +0000)

### Fixes

- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Wait for objects in sequence causes wrong callback invoke
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Improve wait for and subscribe functionality
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Improve wait for and subscribe functionality
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Ignore deprecated declarations
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Investigate and fix klocwork reports for ambiorix libs and tools
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Make it possible to wait for instances
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Do not use expression to filter parameters on name or attributes
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Investigate and fix klocwork reports for ambiorix libs and tools
- [libamxm](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxm): Investigate and fix klocwork reports for ambiorix libs and tools
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Compile regular expressions and validate expressions only once
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Investigate and fix klocwork reports for ambiorix libs and tools
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Investigate and fix klocwork reports for ambiorix libs and tools

### Other

- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Improve documentation of amxd_object function with regard to events
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [AMX] Improve documentation of amxd_object function with regard to events

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [AMX] Dump output arguments of failed methods

## Release v3.1.1 2022-11-03(17:40:42 +0000)

### Fixes

- [mod-lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/mod-lua-amx): Depending on the build the lua header files are installed in different locations
- [mod-lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/mod-lua-amx): Rename sah_mod_lua_amx to sah_mod-lua-amx
- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Depending on the build the lua header files are installed in different locations
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [AMX] JSON string cannot be sent as event data
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Issue: Investigate and fix klocwork reports
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Write errors and warning to system log
- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj): [AMX] JSON string cannot be sent as event data

### Other

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): Implement reboot/upgrade persistence for Ambiorix objects
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Add ^= expression operator
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Add function to check if string is safe to build expressions with
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Issue: ambiorix/libraries/libamxc#69 Remove dead code and code cleanup
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Support appending formatted string with safety check on replacements

### Changes

- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Reduce the amount of amxb calls for copy parameters
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [ACS][V12] Setting of VOIP in a single SET does not enable VoiceProfile
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [ACS][V12] Setting of VOIP in a single SET does not enable VoiceProfile

## Release v3.1.0 2022-10-25(06:30:41 +0000)

### Changes

- [mod-lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/mod-lua-amx): Make it possible to write a data model plugin in lua
- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Make it possible to implement a data model in lua
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [ACS][V12] Setting of VOIP in a single SET does not enable VoiceProfile
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [ACS][V12] Setting of VOIP in a single SET does not enable VoiceProfile
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): It must be possible for a function resolver to known for which action an action callback function is needed

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Wrong configuration is passed to back-ends when connecting
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Issue: Fix failing unit tests for set multiple
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): Issue: verify return value of chown
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Parameter attributes are not correctly checked when adding the parameter to an object

### Other

- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Issue: ambiorix/applications/amxo-cg#21 Variant return type is not properly converted to doc

## Release v3.0.0 2022-10-14(06:26:09 +0000)

### Other

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [USP][CDROUTER] GetSupportedDM on Device.LocalAgent. using a single object, first_level_only true, all options presents no event
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Improve plugin boot order
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [USP][CDROUTER] GetSupportedDM on Device.LocalAgent. using a single object, first_level_only true, all options presents no event

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Init data before cleaning in amxb_ubus_func_handler
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP][CDROUTER] GetSupportedDM on Device.LocalAgent. using a single object, first_level_only true, all options presents no event
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [USP][CDROUTER] GetSupportedDM on Device.LocalAgent. using a single object, first_level_only true, all options presents no event
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Issue: Fix _describe RPC method definition

## Release v2.13.0 2022-10-10(06:11:47 +0000)

### Changes

- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Use amxp functions for scanning directories
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): Use amxp functions for creating and scanning directories

### Fixes

- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): Clean up code
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [USP][CDROUTER] GetSupportedDMResp presents wrong syntax of inner nested multi-instanceobject
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Apply change owner when uid or gid is different from zero
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): When signals are triggered in a recursive way it can lead to segfaults

### Other

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP][CDROUTER] GetSupportedDMResp presents wrong syntaxe of inner nested multi-instanceobject
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Remove configuration for lighttpd

### New

- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [ACLManager] Create ACL user to handle secure acl checking
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [ACLManager] Create ACL user to handle secure acl checking
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Add directory utility functions
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Add comparison implementation for htable variants
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc):  Add comparison implementation for linked list variants

## Release v2.12.3 2022-09-29(11:58:09 +0000)

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Regression - translation to _exec function is going wrong

## Release v2.12.2 2022-09-28(12:28:32 +0000)

### Changes

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): Auto detect usp sockets

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [USP][LCM] InstallDU called from the backend failed; Calling Device.SoftwareModules.InstallDU fails as well (but not SoftwareModules.InstallDU)
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Get request with invalid path should fail
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [ACLManager]Crash when accessing pcb with user anonymous

## Release v2.12.1 2022-09-22(11:16:14 +0000)

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [USP] Add requests with search paths will be allowed
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP] Add requests with search paths will be allowed
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Re-add demo/example web-ui
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [USP] Add requests with search paths will be allowed

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Key parameters must be added to add instance reply
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): GSDM should return read-only attribute for key parameters
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Issue: HOP-1897- [UI] UI broken on WNC config
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Asynchonous call on local deferred function does not fill retval
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): It mustbe possible to define empty object or empty array in config section
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): GSDM should return read-only attribute for key parameters

## Release v2.12.0 2022-09-13(06:45:04 +0000)

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP][CDROUTER] USP Agent never sent a notification to the controller
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP][CDROUTER] The agent sends the GetSupportedDMResp with missing "Device" when requested single object "Device.localAgent."

### Changes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Support Ambiorix access restriction from PCB bus, NET-3484 [PCB][AMX][ACL] It must be possible to apply ACLS on amx components in a PCB environment
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): [USP] Location of odl save files needs to change

### Other

- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): libamxo build failed because when_true_status() macro is redefined.
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Integrate Devolo Interference Mitigation (integration)

### New

- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Add public function to check if getting a parameter is allowed

## Release v2.11.9 2022-08-30(16:54:19 +0000)

### Fixes

- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj): Due to change in libamxc a unit test is failing

## Release v2.11.8 2022-08-30(12:51:05 +0000)

### Fixes

- [mod-amxb-jsonrpc](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [AMX] Allow back-ends to modify their config section
## Release v2.11.7 2022-08-30(12:39:03 +0000)

### Fixes

- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): amx gmap-client modules do not wait for "requires"

## Release v2.11.6 2022-08-29(13:19:46 +0000)

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [AMX] Allow back-ends to modify their config section
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [AMX] Allow back-ends to modify their config section
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [AMX] Allow back-ends to modify their config section

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Set config variant before connecting to back-end

## Release v2.11.5 2022-08-26(10:54:27 +0000)

### Fixes

- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Issues with datamodel during tests

## Release v2.11.4 2022-08-26(05:58:31 +0000)

### Fixes

- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): amxc_string_t does not handle empty strings properly

## Release v2.11.3 2022-08-24(14:03:54 +0000)

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP] TransferComplete! event does not have a Device. prefix
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Performing an amxb_async_call on a local deferred data model method doesn't return correctly
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [amx] custom param read handler called more often than expected
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [amx] custom param read handler called more often than expected

## Release v2.11.2 2022-08-19(06:34:47 +0000)

### Fixes

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Issue: Unit tests for send-events are failing
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [USP] MQTT IMTP connection cannot handle bus requests
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): It must be possible to extend composite config options
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): allow_partial is not set as an input argument for the set operation

### Changes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): Issue: Update ubus capabilities
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [GL-B1300] Various components failing to open Service in firewall due to high load and multiple interface toggling
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [GL-B1300] Various components failing to open Service in firewall due to high load and multiple interface toggling
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): amxc_string_split_to_llist not splitting text with newline sperator.

## Release v2.11.1 2022-08-05(12:26:58 +0000)

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus): [Ambiorix] Unit tests for amxb_ubus report memory leak
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt): amxrt fails to create folder
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [amx] certain NumberOfEntries fields not updated

## Release v2.11.0 2022-07-27(15:32:07 +0000)

### Changes

- [amxo-xml-to](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-xml-to): It must be possible to customize style sheet, title and copyright notice in html

### Fixes

- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Set amxd_object_free as public API method

### Other

- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Improve documentation
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Add when_failed_status macro

## Release v2.10.4 2022-07-14(08:36:07 +0000)

### Fixes

- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Invoke of method with out arguments on local data model creates wrong result variant

### Other

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): USP needs async userflags for functions
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Issue: ambiorix/libraries/libamxo#76 The object write  action is not called during parsing of odl files
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Issue: ambiorix/libraries/libamxd#141 Default object write action fails when only setting optional parameters

## Release v2.10.3 2022-07-11(11:04:39 +0000)

### Other

- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Issue: ambiorix/libraries/libamxb#63 Doxygen documentation tags must be added to back-end interface stuct and function signatures

## Release v2.10.2 2022-07-07(06:12:23 +0000)

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [USP] Unable to invoke FirmwareImage Download() command

## Release v2.10.1 2022-07-05(19:14:07 +0000)

### Fixes

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Plugins not starting at boot

## Release v2.10.0 2022-07-05(11:22:58 +0000)

### Fixes

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Changes for hop-1509 causes regressions

### New

- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx):  Add bus.wait_for, auto_connect, disconnect_all methods to lua bindings

### Changes

- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Adds lookup cache for amxb_who_has

## Release v2.9.5 2022-06-30(09:01:08 +0000)

### Other

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Sort objects before building GSDM response

## Release v2.9.4 2022-06-28(14:00:49 +0000)

### Fixes

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): When amxrt is stopped while waiting for required objects the entrypoints should not be called with reason STOP
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Reference following using key addressing fails
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Supported commands under multi-instance objects are not returned

### Changes

- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Make it possible to read hidden values depending on the access level

## Release v2.9.3 2022-06-22(12:20:21 +0000)

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Dump command must display mutable attribute when set
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Add support for mutable keys
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add support for mutable keys

### Fixes

- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): A quoted string must always be interpreted as a string

## Release v2.9.2 2022-06-15(10:58:23 +0000)

### Changes

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Plugins not starting at boot

## Release v2.9.1 2022-06-14(07:44:30 +0000)

### Other

- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [ACL manager] Update documentation for the acl manager in confluence

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Issue: # 19 Pcb and ubus config files should be installed by default
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Load order must be the same as save order
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): The default rpc _get must be able to support parameter paths

## Release v2.9.0 2022-06-02(08:30:12 +0000)

### Other

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Use amxa_get to avoid code duplications
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [amx] crash on amxp signal read

### Fixes

- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): amxa_get() should return -1 when no access rights
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [amxo-cg] segfault when parsing long comments

### New

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): When there are required objects events can appear before the entry points are called
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): It must be possible to suspend handling of signals for a specific signal manager

## Release v2.8.0 2022-05-23(18:16:49 +0000)

### Fixes

- [amx-variant-contacts](https://gitlab.com/prpl-foundation/components/ambiorix/examples/collections/variant_contacts): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [amx-htable-contacts](https://gitlab.com/prpl-foundation/components/ambiorix/examples/collections/htable_contacts): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [amx-llist-contacts](https://gitlab.com/prpl-foundation/components/ambiorix/examples/collections/llist_contacts): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [amx-ssh-server](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/ssh-server): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [amx-tr181-localagent-threshold](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/localagent_threshold): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [amx-cpu-info](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/cpu-info): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [amx-greeter-app](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_app): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [amx-subscribe](https://gitlab.com/prpl-foundation/components/ambiorix/examples/baapi/subscribe): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Improve error message when function call fails
- [mod-dm-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-dm-cli): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): The command parser does not parse embedded string correctly
- [libamxm](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxm): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when memory leaks are detected

### New

- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Make it possible to initialize a timestamp structure using struct tm

### Other

- [amx-greeter-plugin](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/greeter_plugin): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when...
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when...
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Use amxa_get to avoid code duplications
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Rework configuration to work with default lighttpd
- [amxo-xml-to](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-xml-to): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when...
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when...
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Issue: ambiorix/libraries/libamxa#24 Implement amxa_get
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): [Gitlab CI][Unit tests][valgrind] Pipeline doesn't stop when...
- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj): Issue: ambiorix/libraries/libamxj#15 Fix memory issue for out of bounds write in amxj_read() (fix)

### Changes

- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Use reference index when a reference path is provided
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add reference following for reference lists using indexes

## Release v2.7.0 2022-05-12(16:22:48 +0000)

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [Ambiorix] Implementation of reference following decorator
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): [Ambiorix] Implementation of reference following decorator
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Update path parser to be able to detect reference path

### New

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Add implementation for seBatch

### Other

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Add missing dependency to libpcb

## Release v2.6.0 2022-05-05(18:30:39 +0000)

### New

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Add get instances command
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Implement pcb backend function for get instances operator on PCB data models
- [amxb-inspect](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxb-inspect): Check get_instances function of back-end

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): When subscribing on root object of mapped pcb datamodel no events are received
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Incorrect check for get_instances back-end function

### Other

- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Change type to "utility"
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Issue: ambiorix/modules/amx_cli/mod-ba-cli#18 Update output of gsdm command [changed]

### Changes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Update get supported data model implementation according to USP specification 1.2
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Update get supported data model implementation according to USP specification 1.2
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): [MQTT] Topic must be writable after creation

## Release v2.5.0 2022-04-25(16:21:03 +0000)

### Other

- [mod-lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/mod-lua-amx): Add lua as a dependency
- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Add lua as a dependency

### New

- [mod-lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/mod-lua-amx): Add usage documentation to readme
- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): It must be possible to access the odl config options
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Implement amxb_get_multiple
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Add support for get_instances operator
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add internal data model RPC _get_instances

### Fixes

- [mod-lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/mod-lua-amx): Unused variables when compiling for arm target
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): ODL parser sometimes gets confused

## Release v2.4.0 2022-04-08(09:02:29 +0000)

### New

- Add lua bindings

### Fixes

- [lua-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/lua/lua-amx): Incorrect install location after adding config variable

## Release v2.3.3 2022-04-06(15:57:57 +0000)

### Changes

- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [GetDebugInformation] Add data model debuginfo in component services

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Send reply in case of an error
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): uBus does not always respect order of in-coming messages
- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): no-colors should be set to true by default
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Constructor functions that add custom expression function must be run at level higher then 101
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Remove macro IS_SET
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): It is not possible to add multiple times the same object action callback with different private data

## Release v2.3.2 2022-03-17(21:42:59 +0000)

### Other

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Update ubus dependency

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Segmentation fault when amxb_ubus_list is called with invalid path
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Segmentation fault when amxb_pcb_list is called with invalid path
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [ACL][USP] ACL files must be located in writable directory
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [ACL][USP] ACL files must be located in writable directory
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Use correct logic to determine if an instance exists
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Aliases containing dots causes problems when used in object paths
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Use dyncast to get the index out of a variant

## Release v2.3.1 2022-02-18(10:38:07 +0000)

### Fixes

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Plug-in name is not correctly passed to pcb back-end

## Release v2.3.0 2022-02-17(20:45:48 +0000)

### New

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Commandline options -o and -F must support configuration paths and json data
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Implement amxa_set_multiple
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add API to get applied mib names
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add permission denied status code
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Add implementation of amxc_var_set_path and amxc_var_set_pathf

### Other

- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Update documentation on AMXO_ODL_LOADED
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Issue: ambiorix/libraries/libamxo#72 Update documentation on AMXO_ODL_LOADED
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Issue: ambiorix/libraries/libamxd#125 Update documentation on return variant of transaction

### Fixes

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Link amxrt with libyajl
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Adds yajl as dependency in baf
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Fixes regression
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Run tests with sah-ci image
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Update implementation of amxo_parser_get_config, amxo_parser_set_config, amxo_parser_claim_config

## Release v2.2.1 2022-02-07(15:54:16 +0000)

### Fixes

- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Copybara replaces too many lib occurences
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Variant conversions to integer values is going wrong on mips target

## Release v2.2.0 2022-02-04(07:00:50 +0000)

### Breaking

- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): [prplOS][ambiorix] Several component failing to start on NEC mips xrx500 target

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): When using amxb_set_multiple without required parameters the return value is incorrect
- [amxo-xml-to](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-xml-to): Broken reference to mibs in html pages
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): Too much callbacks are called when multiple parameters with the same name are synced
- [libamxs](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxs): memory leak in amxs_sync_entry_build_opposite_path_entry
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Cannot load unknown instance parameter with "populate-behavior.unknown-parameter=add"

### New

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): It must be possible to show and access protected Parameters/Objects.

### Note

A major upstep of libamxo is done:

- Deprecated PCB compatible odl syntax is removed.

## Release v2.1.0 2022-01-25(15:21:54 +0000)

### New

- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Implement amxb_set_multiple

### Fixes

- [amx-tr181-localagent-threshold](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/localagent_threshold): Sleep in unit test can be too short
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Revert set partial option
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Make it possible to do partial sets on native PCB data models
- [amxo-xml-to](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-xml-to): Update the date in the html template to 2022
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Fixes test version cehcking
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Allow object write with only optional parameters
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Adding a valid MIB to an object with a transaction makes the transaction fail
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Objects added using a mib can not be addressed with search path or named path
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Correct allow partial for set
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): amxp_signal_has_slots only checks the first slot

## Release v2.0.2 2021-12-17(09:26:09 +0000)

### Changes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Make it possible to do partial sets on native PCB data models

## Release v2.0.1 2021-12-16(16:54:58 +0000)

### Fixes

- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): Command value parser ignores single or double quotes for values

## Release v2.0.0 2021-12-16(12:38:41 +0000)

### Breaking

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Update minimum and maximum version of supported libamxb
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Update set interface according to changes in libamxb
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Remove deprecated functions
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Add support for allow partial with set operator

### New

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Make it possible to do partial set
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Add support for allow partial in set operator

### Fixes

- [amx-webui](https://gitlab.com/prpl-foundation/components/ambiorix/examples/webui/webui): Update variables to work with amx-fcgi
- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Adjust tests to comply with libamxb 4.0.2
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Fixes version check test due to upstep of major version

### Note

A major upstep of libamxb is done:

- Deprecated functions are removed
  - amxb_set_v1
  - amxb_add_v1
  - amxb_del_v1

- Back-end interface for set operator has been changed, all back-ends implementing this interface must be updated.

## Release v1.15.4 2021-12-10(06:21:18 +0000)

### New

- Adds webui example

## Release v1.15.3 2021-12-09(07:28:18 +0000)

### Fixes

- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Fix Build warnings
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): amxb_who_has function must take local data model into account

### Changes

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Make it possible to handle data model events before app:start event is triggered
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): When a parameter is of csv or ssv type all individual values must be verified with check_enum or check_is_in
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Make it possible to set relative parameter references in validators

### Other

- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Issue: ambiorix/applications/amxo-cg#20 Add STAGINGDIR to CFLAGS and LDFLAGS

## Release v1.15.2 2021-12-02(13:06:31 +0000)

### Changes

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Code clean-up for search path subscriptions
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Code clean-up
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Adds support for event proxy
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Improve and refactor subscriptions

### Fixes

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Fixes segfault in test
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Subscriptions on non-existing objects must fail
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): No events received when subscribing on native ubus objects
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Fixes subscription on search paths
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Fixes segmentation fault when deleting subscription object
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Documentation mentions wrong type for object iterations macros
- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj): Potential memory leak in variant_json_init

### Other

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Opensource component

## Release v1.15.2 2021-11-25(15:48:34 +0000)

### New

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Add asynchronous command

### Fixes

- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): Issue: 11 fixed segmentation fault in variant to python conversion
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Memory leak in amxb_ubus_get_longest_path when invoked with a non existing path
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): Guest role should only have read access
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Improve default set action

### Changes

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Complete the README.md file

## Release v1.15.1 2021-11-18(16:16:47 +0000)

### Other

- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Issue: ambiorix/libraries/libamxb#47 When unsubscribing slot disconnect must be done on a specific signal

### Changes

- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Make it possible to set an action callback for all actions
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): The function name must be passed as private data to subscriptions taken from an odl file

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Use emit signal instead of trigger in amxb_ubus_wait_watcher
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Amxb_ubus_unsubscribe segfaults when object is not found
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Fixes regression due to adding any action
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Missing brackets in function resolver data causes segmentation fault

## Release v1.15.0 2021-11-11(08:04:41 +0000)

### Changes

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Add unit-tests for event streams and subscriptions
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): ODL parser should pass function type to resolvers
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): When signal is deleted in slot, the remaining slots must be called

### Other

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Issue: ambiorix/applications/amx-fcgi#11 Add unit tests for sending events

### New

- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): It must be possible to filter get_supported messages based on ACL filters

### Fixes

- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): amxo-cg crashes when trying to parse prplMesh ODL files
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Fixes compilation issue for g++
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Fixes test when daylight saving is off

## Release v1.14.0 2021-10-29(07:12:01 +0000)

### Changes

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Add startup order
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): It must be possible to invoke native commands
- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Unit tests must verify the returned status code in the responses

### New

- [amx-fcgi](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-fcgi): Add ACL verification
- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): [CLI][AMX] Add support for ACLs in the cli
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): It must be possible to connect to uris without registering a data model

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Fixes events from PCB mapper data models
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [AMX] ACL merged directory must be writable
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Infinite loop when removing parent object having underlying depth greater than 10
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Amxd_object_remove_mib removes mibs when they are not added
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Restarting timers can lead to early sigalrm

## Release v1.13.1 2021-10-22(11:23:33 +0000)

### Changes

- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Parse odl files in order given on commandline

### Fixes

- [python-amx](https://gitlab.com/prpl-foundation/components/ambiorix/bindings/python3): DMObject created with key notations paths have broken parameters
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): amxo-cg sometimes doesn't properly detect passed filename

## Release v1.13.0 2021-10-21(08:40:23 +0000)

### Other

- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [BAF] add support for amx docgen
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [BAF] add support for amx docgen

### Changes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Always return index path
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Always return index path

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Memory leak in amxb_ubus_has back-end interface implementation
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Amxd_path_get_type returns a bool
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Segmentation fault occurs when NULL pointer passed to amxc_var_dump or amxc_var_log

### New

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Listen to signals that indicate a wait-for-write fd must be added to the event loop
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Introduces function to add wait-for-write fd to event loop

## Release v1.12.0 2021-10-14(09:13:25 +0000)

### Changes

- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Resolve search paths for objects if parent instance exists
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Use longest possible path for bus operations
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): `_get` function must return multi-instance objects when access is protected
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Update implementation of ~= operator

### Other

- [amx-cli](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amx-cli): [CI] Update autogenerated files

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): ubus blob must be initialized right before usage
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Error checking must be applied when registering data model objects
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Use longest possible path known by ubusd
- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Updates README.md - adds missing ubus configuration option
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Parsing defaults values fails if parent is referenced by Alias
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Don't clean acls in amxa_resolve_search_paths
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Remove object if all parameters are filtered out
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Extend the back-end interface to make it possible for a back-end to announce its capabilities and provide object discovery
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Fixes version check tests

### New

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Use longest possible path known by ubusd
- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Update pcb back-end due to changes in libamxb
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [ACL] Add default ACL configuration per services
- [amxb-inspect](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxb-inspect): amxb inspect must verify functions `has` and `capabilities`
- [libamxb](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxb): Extend the back-end interface to make it possible for a back-end to announce its capabilities and provide object discovery
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Make it possible to get the local time timestamp

## Release v1.11.2 2021-10-05(10:47:01 +0000)

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Fixes acl get verification
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): When parameter paths are added to the filter list, they have a dot suffix
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Resolving search paths can fail for paths with a Device. prefix

## Release v1.11.1 2021-09-27(12:11:14 +0000)

### Fixes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): When using auto load with events turned off objects are not registered to bus
- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): When using auto load with events turned off objects are not registered to bus
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): It is possible to bypass acl verification in object tree
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [ACL] Make sure ACL verification works for Device path
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Add libamxo as an open source test dependency
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): Saved odl files with mib extensions can not be loaded
- [libamxo](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxo): it must be possible to indicate that an instance parameter must be saved in the header
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Key parameters must be validated when instance is created
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Instances with Alias parameter containing a dot can not be deleted
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): method amxd_object_for_all can be invoked on any object
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): It must be possible to indicate that amxc_var_get_path must not search positional if key is not found
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Unexpected behavior of amxc_var_get_path

### Changes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Add acl verification for get
- [acl-manager](https://gitlab.com/prpl-foundation/components/ambiorix/applications/acl-manager): [ACL] [BAF] Configure default ACL directory variable in baf templates
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Resolving search paths is not needed if fixed part does not exist in acls
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): [ACL] [BAF] Configure default ACL directory variable in baf templates

### Other

- [amxrt](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxrt/): Issue: ambiorix/applications/amxrt#34 Make sure eventing is enabled before entry-points are called
- [libamxa](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxa): Opensource component

## Release v1.11.0 2021-09-14(11:57:39 +0000)

### Other

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Turns on unit tests and coverage reports
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): Generate junit xml files with unit-tests
- [libamxt](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxt): Issue: ambiorix/libraries/libamxt#6 Generate junit xml files with unit-tests
- [libamxm](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxm): Generate junit xml files with unit-tests
- [libamxm](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxm): Issue: ambiorix/libraries/libamxm#5 Generate junit xml files with unit-tests
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Generate junit xml files with unit-tests
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Issue: ambiorix/libraries/libamxp#36 Generate junit xml files with unit-tests
- [libamxj](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxj): Generate junit xml files with unit-tests
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Generate junit xml files with unit-tests
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Issue: ambiorix/libraries/libamxc#56 Generate junit xml files with unit-tests

### Changes

- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Adds support for bbf in operator ~=

### New

- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): Add functions to convert a string to capital/lower case.
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): [status macros] Add when_null_status macros in amxc

### Fixes

- [mod-amxb-pcb](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_pcb/): Support request paths containing '/' separators in pcb backend
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): GCC11 archlinux compiler warning
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): Disconnecting slots in slot callback function can lead to segmentation fault
- [libamxp](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxp): When child process is killed it stays in <defunc>

## Release v1.10.1 2021-08-24(14:33:25 +0000)

### Changes

- [amx-tr181-localagent-threshold](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/localagent_threshold): Correct example start-up in README.md

### New

- [amx-cpu-info](https://gitlab.com/prpl-foundation/components/ambiorix/examples/datamodel/cpu-info): Make example open-source ready

## Release v1.10.0 2021-08-23(12:37:02 +0000)

### New

- [amxo-xml-to](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-xml-to): [docgen] Create amxo-xml-to template to create CSV for prpl jira import
- [amxo-cg](https://gitlab.com/prpl-foundation/components/ambiorix/applications/amxo-cg): Extra info needed in generated xml

### Fixes

- [mod-ba-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-ba-cli): Missing symlinks to /usr/bin/amx-cli on installation of the debian package
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Cannot remove last part from path if it ends with an asterisk
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Verifying if an object has a parameter can cause a segmentation fault

### Other

- [mod-dm-cli](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amx_cli/mod-dm-cli): Opensource component
- [libamxd](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxd): Issue: ambiorix/libraries/libamxd#102 Add macro to iterate over the content of objects that can be nested

### Changes

- [mod-amxb-ubus](https://gitlab.com/prpl-foundation/components/ambiorix/modules/amxb_backends/amxb_ubus/): Tests must be added
- [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc): no more Shadow warning if nesting of ...for_each... macros is used

## Release v1.9.2 2021-08-02(16:43:18 +0000)

### Changes

- libamxt: Auto detect file descriptor of terminal
- libamxd: Make amxd_function_are_args_valid function public
- libamxd: Destroy action callbacks must be called bottom-up
- libamxb: Make it possible to invoke RPC methods that are not under an object

### Fixes

- python-amx: Inconsistent naming between protected and async protected methods

## Release v1.9.1 2021-07-27(09:58:40 +0000)

### Fixes

- libamxj: Fixes streamed reading from fd
- libamxj: Fixes streaming parsing of json string
- mod-ba-cli: It must be possible to provide composite values to method arguments

### Changes

- amxrt: Adds simple rbus autodetect socket

## Release v1.9.0 2021-07-22(13:40:33 +0000)

### New

- libamxd: Add function to append amxd_path_t
- libamxo: Make it possible to declare required objects from remote processes
- libamxb: Update documentation
- libamxb: Add wait for feature to bus agnostic api
- amxrt: Updates run-time to have support for wait-for-objects feature
- mod-amxb-pcb: Add wait_for back-end implementation
- mod-amxb-ubus: Add wait_for back-end implementation
- amx-subscribe: Updates example to demonstrate wait for and auto resubscribe features

### Fixes

- libamxd: Applying a transaction without an object selected causes a segmentation fault
- libamxd: When removing part of a path, the type must be recalculated
- libamxb: Fixes version check tests
- amx-cli: SIGTERM must correctly be handled in eventloop

### Changes

- amxrt: Improve save functionality in amxrt
- amxb-inspect: amxb-inspect must verify wait_for function

## Release v1.8.1 2021-07-09(13:14:37 +0000)

### Fixes

- libamxc: Inconsistency in behavior of constcast and dyncast on string variants
- libamxd: Add unit test that shows bug
- libamxd: When providing a instance path to get supported it must fail
- libamxb: Add OBJECTS and INSTANCES flags for amxb_invoke_list
- amxo-cg: Not passing any input files results in exit code 0
- amxo-xml-to: When -x merge is used as input, the stylesheet should only be applied once

## Release v1.8.0 2021-07-05(08:49:11 +0000)

### New

- libamxc: Make it easy to convert an array of bytes to a hexbinary string and the other way around

### Changes

- libamxd: Makes it possible to store private data in function definitions
- libamxd: Makes it possible to enable or disable custom parameter and object actions without removal and re-adding

### Fixes

- libamxd: When amxd_trans_apply fails on enum string parameter it calls validation function from a different parameter
- libamxd: Length validation on csv or ssv string parameters is always failing
- libamxo: Generation of version.h files should not be .PHONY
- mod-amxb-ubus: amxb_ubus_list causes a memory leak when no objects found

## Release v1.7.1 2021-07-02(04:24:28 +0000)

### Fixes

- mod-amxb-pcb: Segmentation faults seen on target

## Release v1.7.0 2021-06-29(10:18:21 +0000)

### Fixes

- libamxc: Removes amxc_var_hfor_each
- libamxd: The data model  is sending to many events
- libamxo: ODL parser is sending the add (intstance) events in the wrong order
- libamxb: Generation of version.h files should not be .PHONY
- amxb-inspect: make clean does not clean the output directory
- amx-cli: Missing description in baf file
- mod-amxb-ubus: Automatically re-activate subscriptions on objects
- mod-amxb-ubus: Generation of version.h files should not be .PHONY
- python-amx: Fix naming of protected methods of DMObjects
- amx-tr181-localagent-threshold: Missing description in baf file
- libamxo: After loading post-includes data model eventing is disabled

### Changes

- libamxc: Make it easier to iterate over htable and list variants
- libamxo: Make it possible to load and keep modules when no symbols are resolved
- mod-amxb-pcb: Adapt pcb back-end to have support for deferred RPC methods
- mod-amxb-ubus: Adapt ubus back-end to have support for deferred RPC methods
- mod-ba-cli: Make it possible to push bus or backend specific configuration

### New

- libamxd: Make it possible for RPC methods to defer the result
- python-amx: Extend dmmngt with instances, objects and type_name. Improve DMObject __dict__
- amx-greeter-plugin: Add a deferred RPC method example

### Other

- mod-amxb-ubus: Update README

## Release v1.6.0 2021-06-22(09:40:44 +0000)

### New

- libamxp: Make it possible to defer function calls
- libamxb: It must be possible to store pending asynchronous invoke requests in a list

### Changes

- mod-amxb-pcb: Use function amxb_request_get_ctx to get the bus context of a pending request
- mod-amxb-ubus: Use function amxb_request_get_ctx to get the bus context of a pending request
- mod-ba-cli: Issue #10 Invoking data model methods must be done asynchronously

## Release v1.5.2 2021-06-19(08:27:33 +0000)

### New

- libamxc: Abstract data type set must be provided
- libamxp: When using an amxc_set_t it must be possible to evaluate a flag expression with it

### Changes

- libamxc: Make it possible to access htable variants by index
- libamxj: Update dependencies in .gitlab-ci.yml
- libamxp: Expressions with True or False should give matches
- libamxm: Update dependencies in .gitlab-ci.yml
- libamxt: Update dependencies in .gitlab-ci.yml
- libamxd: Update dependencies in .gitlab-ci.yml
- libamxo: Update dependencies in .gitlab-ci.yml
- libamxb: Update dependencies in .gitlab-ci.yml
- amxrt: Use command line option -o to override ODL config options
- mod-amxb-pcb: Update dependencies in .gitlab-ci.yml
- mod-amxb-ubus: Update dependencies in .gitlab-ci.yml

### Fixes

- libamxp: Issue: 21 Deleting signal manager from callback function causes segmentation fault
- libamxb: Memory leak in amxb_resolve in case of failure
- amxrt: ODL config option dm-eventing-enabled has no effect when changed in the odl file
- amxo-cg: Generation of version.h files should not be .PHONY
- mod-amxb-ubus: ubus server script does not work on rpi3, unkown socat option tcpwrap

### Other

- amxo-cg: Issue: ambiorix/applications/amxo-cg#14 Parameters defined using `counted with` are not present in XML output

## Release v1.5.1 2021-06-15(13:23:10 +0000)

### Fixes

- libamxd: When multiple add-inst action callbacks are added to an multi-instance object multiple instances are created
- libamxd: Destroy function will be called forever if it does not get an amxd_status_ok

### Changes

- libamxo: Update dependencies in .gitlab-ci.yml
- libamxb: Update dependencies in .gitlab-ci.yml
- mod-amxb-pcb: Update dependencies in .gitlab-ci.yml
- mod-amxb-ubus: Update dependencies in .gitlab-ci.yml

## Release v1.5.0 2021-06-11(14:14:19 +0000)

---
> **NOTE - BREAKING CHANGE IN BEHAVIOR**
> 
> Due to a misinterpretion of some of the requirements of TR-369 (USP) the behavior of the resolving functions and get operator in libamxd was not correctly implemented. From version 3.0.0 of libamxd this issue is fixed.
>
> The related TR-369 requirements are:
> ```
> R-GET.0 - If requested_path contains a Path Name that does not match any Object or Parameter in the Agent’s Supported Data Model, the Agent MUST use the 7026 - Invalid Path error in this RequestedPathResult.
> ```
> ```
> R-GET.1a - If the requested_path is valid (i.e., properly formatted and in the Agent’s supported data model) but did not resolve to any objects in the Agent’s instantiated data model, the resolved_path_results set MUST be empty and is not considered an error.
> ```
>  
> In previous versions of libamxd the resolve functions and get operators returned an error when no matching objects were found, even if the path was in the supported data model. According to TR-369 the get operator should return an empty data set if it could not be resolved to any objects, and an error if the given path is not in the supported data model.
---

To clarify this a small example:<br>
Assume this data model is available:
```
MyRoot.
MyRoot.Level1.1.
MyRoot.Level1.1.MoreText="Extra Text"
MyRoot.Level1.1.Text="Hallo"
MyRoot.Level1.1.Level2.1.
MyRoot.Level1.1.Level2.1.Flag=true
MyRoot.Level1.1.Level2.1.Number=0
MyRoot.Level1.1.Level2.1.Text=""
MyRoot.Level1.2.
MyRoot.Level1.2.MoreText=""
MyRoot.Level1.2.Text="Dummy"
MyRoot.Level1.2.Level2.1.
MyRoot.Level1.2.Level2.1.Flag=true
MyRoot.Level1.2.Level2.1.Number=3
MyRoot.Level1.2.Level2.1.Text="Found It"
MyRoot.Level1.2.Level2.2.
MyRoot.Level1.2.Level2.2.Flag=false
MyRoot.Level1.2.Level2.2.Number=4
MyRoot.Level1.2.Level2.2.Text="Bye"
```
Then the following gets will result in:
```
GET(MyRoot.Level1.*.Level2.2.)                                   -> Valid 
GET(MyRoot.Level1.*.Level2.2.Text)                               -> Valid
GET(MyRoot.Level1.*.Level2.)                                     -> Valid
GET(MyRoot.Level1.*.Level2.Text)                                 -> Invalid -> error
GET(MyRoot.Level1.*.Level2.*.Text)                               -> Valid
GET(MyRoot.Level1.*.Level2.3.)                                   -> Valid -> empty set (was previously an error -> not found)
GET(MyRoot.Level1.[Text == "Hallo"].Level2.*.)                   -> Valid
GET(MyRoot.Level1.[Text == "Hallo"].Level2.)                     -> Valid
GET(MyRoot.Level1.[Text == "NotAvailable"].Level2.)              -> Valid -> empty set (was perviously an error -> not found)
GET(MyRoot.Level1.[BlaBla == "NotAvailable"].Level2.)            -> Invalid -> error
GET(MyRoot.Level1.[Text == "Dummy"].Level2.[Flag == true].)      -> Valid
GET(MyRoot.Level1.[Text == "Dummy"].Level2.[Flag == true].Text)  -> Valid
GET(MyRoot.Level1.[Text == "Dummy"].Level2.[Number > 100].Text)  -> Valid -> empty set (was previously an error -> not found)
GET(MyRoot.Level1.[Text == "Dummy"].Levl2.[Number > 100].Text)   -> Invalid -> error
GET(MyRoot.Level1.*.Level2.[Flag == true].)                      -> Valid
GET(MyRoot.Level1.*.Level2.[Flag == true].Number)                -> Valid
```

### Fixes

- libamxd: Fixes checking of valid parameter names
- libamxd: Fixes del operator, due to change in behavior of amxd_object_resolve_... functions
- libamxd: GetSupportedDM does not return enough information when the first_level_only flag is set
- mod-amxb-ubus: Data model errors are not always correctly translated to ubus errors

### Breaking

- libamxd: Search paths should only be expanded if there is a match

### Changes

- mod-ba-cli: When a get request returns an empty data set a message must be printed

### New

- libamxo: A hook must be added for counter parameters
- mod-ba-cli: Add get supported data model command

## Release v1.4.1 2021-06-10(11:21:01 +0000)

### Fixes

- libamxc: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- libamxj: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- libamxp: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- libamxm: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- libamxt: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- libamxd: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- libamxo: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- libamxb: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amxrt: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amxo-cg: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- mod-amxb-pcb: Segmentation fault when data model returns a htable instead of a list for list operator
- mod-amxb-pcb: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- mod-amxb-ubus: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amx-subscribe: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amx-greeter-plugin: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amx-greeter-app: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amx-tr181-localagent-threshold: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amx-ssh-server: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amx-ssh-server: Updates readme, add underscores to ambiorix data model functions
- amx-llist-contacts: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amx-htable-contacts: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amx-variant-contacts: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amx-cli: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- amx-cli: Add linker flags for libamxd and libyajl
- amx-cli: The CFLAGS and LDFLAGS must be extended with sources from the STAGINGDIR
- amx-cli: Incorrect config variable prevents building in sah sop
- mod-ba-cli: [tr181 plugins][makefile] Dangerous clean target for all tr181 components
- mod-dm-cli: [tr181 plugins][makefile] Dangerous clean target for all tr181 components

### Changes

- libamxj: Update dependencies in .gitlab-ci.yml
- libamxp: Update dependencies in .gitlab-ci.yml
- libamxm: Update dependencies in .gitlab-ci.yml
- libamxt: Update dependencies in .gitlab-ci.yml
- libamxd: Update dependencies in .gitlab-ci.yml
- libamxo: Update dependencies in .gitlab-ci.yml
- libamxb: Update dependencies in .gitlab-ci.yml
- mod-amxb-pcb: Update dependencies in .gitlab-ci.yml
- mod-amxb-ubus: Update dependencies in .gitlab-ci.yml

### New

- mod-dm-cli: Component should have a debian package

## Release v1.4.0 2021-06-02(12:52:38 +0000)

### Changes

- libamxp: Extend starts with compare operator for expressions
- libamxt: Update dependencies in .gitlab-ci.yml
- libamxt: Update dependencies in .gitlab-ci.yml
- libamxd: Update dependencies in .gitlab-ci.yml
- libamxd: It must be possible to disable template information
- libamxd: Update dependencies in .gitlab-ci.yml
- libamxo: Update dependencies in .gitlab-ci.yml
- libamxo: Update dependencies in .gitlab-ci.yml
- libamxb: Update dependencies in .gitlab-ci.yml
- libamxb: List operator should take temple info flag into account
- libamxb: Update dependencies in .gitlab-ci.yml
- mod-amxb-pcb: Update dependencies in .gitlab-ci.yml
- mod-amxb-pcb: Operator lists takes template info flag into account
- mod-amxb-pcb: Update dependencies in .gitlab-ci.yml
- mod-amxb-ubus: Add the possibility to connect to tcp/ip sockets
- mod-amxb-ubus: Update dependencies in .gitlab-ci.yml
- mod-amxb-ubus: List operator takes template info flag into account
- mod-amxb-ubus: Update dependencies in .gitlab-ci.yml
- mod-ba-cli: Use new subscription API

### Fixes

- libamxd: Negative object indexes and names can occur
- amx-cli: Incorrect config variable prevents building in sah sop
- mod-ba-cli: It must be possible to subscribe for events using a search path

### Other

- mod-amxb-pcb: Disable cross-compilation

### New

- mod-ba-cli: Add resolve command
- mod-ba-cli: Adds who has cmd to connections

## Release v1.3.0 2021-05-25(11:40:53 +0000)

### Fixes

- libamxp: Fix typos in amxp_subproc.h
- libamxd: amxd_object_get_path with flag AMXD_OBJECT_SUPPORTED returns wrong path on instances
- libamxo: Recursive includes when parsing odl files can cause a segmentation fault
- libamxo: Extend event filter parser to resolve variables
- libamxb: amxb_subscribe and amxb_unsubscribe asymmetry
- libamxb: Improve who has functionality
- amxo-cg: Include tree is not build correctly

### Changes

- amxo-cg: add command line reset option
- amxo-cg: Update README
- amx-subscribe: Use oss-bus-tester container for CI
- amx-tr181-localagent-threshold: Re-enable tests

### New

- libamxb: It must be possible to keep track of subscriptions
- amx-greeter-plugin: Autogenerate data model documentation

### Other

- amx-tr181-localagent-threshold: Enable auto opensourcing

## Release 1.2.1 - 2021-05-10(08:43:00 +0000)

### New

- libamxp: Make it possible to use expressions in variant paths
- amxo-cg: Generate xml/html documentation from odl files
- mod-ba-cli: Component should have a debian package

### Fixes

- libamxj: libyajl development header should be installed with debian package
- amxo-cg: does not exit with status code different from 0 when odl contains errors
- amxo-cg: It must be possible to add saved files
- amx-cli: Compilation error with bcm9xxx-aarch64-linux-4.19-gcc-9.2-glibc2.30 toolchain

### Changes

- amx-cli: Use common macros from libamxc
- amxb-ubus: ubus and integer type handling
- mod-ba-cli: Use common macros from libamxc
- example subscribe: Datamodel operators must be prefixed with an underscore
- example greeter_plugin: Changes echo method
- example greeter_plugin: Data model documentation must be added to the odls
- example greeter_plugin: Use common macros from libamxc
- example greeter_app: Data model documentation must be added to the odls

## Release 1.2.0 - 2021-05-04(15:00:00 +0000)

### New

- libamxo: Comments in an odl file must be extracted and passed to a hook functions

### Fixes

- libamxo: ODL populate section does not resolve config variables
- amxb_ubus: Object deleted in ambiorix is still shown on ubus
- amxb_pcb: uint16 function argument handled as int64

### Changes

- libamxc: Provide public header file for common macros
- libamxc: Update variant GET macros
- libamxj: Use common macros from libamxc
- libamxp: Use common macros from libamxc
- libamxm: Use common macros from libamxc
- libamxt: Use common macros from libamxc
- libamxd: Use common macros from libamxc
- libamxo: Use common macros from libamxc
- libamxb: Use common macros from libamxc
- amxb_ubus: Use common macros from libamxc
- amxb_pcb: Use common macros from libamxc

## Release 1.1.0 - 2021-04-28(19:40:00 +0000)

### New

### Changes

- libamxd: It must be possible to add user flags to functions
- libamxo: Add configuration option to disable function resolving
- libamxo: It must be possible to add user flags to functions
- mod-ba-cli: Updates dump help text
- example greeter_plugin: Remove debug prints

### Fixes

- libamxc: using function amxc_var_take_it to remove a variant from a list or table segfaults when passing NULL pointer
- libamxj: Debian package does not depend on libyajl
- libamxd: Parameters with attribute %key are not immutable
- libamxd: enum_check fails if parameter has attribute %key
- libamxo: Save persistent protected and private parameters
- libamxt: Fixes multiline input and history
- libamxb: Loading a back-end multiple times should not fail
- amxb_ubus: When a new root object is added the sub-tree must be registered to ubusd
- amxb_pcb: When root objects are added or removed an event must be send to pcb_sysbus
- amx-cli: Call exit method of all cli modules at exit
- amx-cli: when switching modules the deactivate method of the current must be called
- amx-cli: Handle ampx events in playback
- amxo-cg: Disable function resolving

## Release 1.0.1 - 2021-04-15(11:10:00 +0000)

### New

### Changes

- all: remove fakeroot dependency on host to build WRT image 

### Fixes

- libamxp: Double free when deleting signal in slot callback function 
- libamxo: parsing multiple odl files with import causes a segmentation fault 
- libamxb: VERSION_PREFIX is needed in buildfiles
- amxb_pcb: VERSION_PREFIX is needed in buildfiles 
- amxb_ubus: VERSION_PREFIX is needed in buildfiles 
- amx-cli: VERSION_PREFIX is needed in buildfiles
- amxo-cg: VERSION_PREFIX is needed in buildfiles 
- amxo-cg: VERSION_PREFIX is needed in buildfiles 
- amxrt: VERSION_PREFIX is needed in buildfiles 
- amxrt: When one of the entry-points fail at startup all of them must be called when exiting 

## Release 1.0.0 - 2021-04-15(07:49:00 +0000)

### New

- libamxd: Adds function to build supported path from object or search path
- libamxd: Adds test case for supported path builder
- libamxd: Make it possible to detect that a given path is an instance object path
- amxrt: It must be possible to save a data model in an automatic manner
- example ssh-server: Add uci storage through config section

### Changes

- libamxd: Improve check if objects exists using describe RPC
- libamxd: Build-in data model RPC methods can conflict with domain specific RPCs
- libamxb:  Build-in data model RPC methods can conflict with domain specific RPCs
- amxb_ubus: Build-in data model RPC methods can conflict with domain specific RPCs
- amxb_pcb: Build-in data model RPC methods can conflict with domain specific RPCs
- example greeter_plugin: Update greeter stats actions
- example greeter_app: Update greeter stats actions

### Fixes

- libamxc: csv strings are not always correctly parsed
- libamxd: Memory leaks in read tests 
- libamxd: To many reads when object action read is invoked
- libamxo: Loading empty directory must return 2 (not found)
- libamxo: When saving an odl file all names must be quoted
- libamxb: Fixes amxb_list on local datamodel
- amxb_ubus:  Fix clean target in makefile
- amxb_ubus:  ubus backend segfaults when using ubus lookup
- amxb_ubus:  key path to index path translation with function invokation

### Notes

Major changes are done in the names of the default data model RPC methods.
All Ambiorix data model RPC methods are prefixed with an underscore `_`. This is done to avoid conflicts between domain specific get and set data model RPC methods.

An example of a service that provides domain specific getter and setter functions is gMap.

Because of this change, previous versions of libamxb or back-ends will not work anymore against the new version of libamxd. Make sure that when switching to version libamxd 2.0.0 that also the versions of libamxb and the bus back-ends are updated.

All your applications/services that depend on libamxb or libamxd must be rebuild (no code changes are needed)

## Release 0.9.0 - 2021-04-01(09:09:42 +0000)

### New

- libamxd: It must be possible to add tags or flags to parameters
- libamxd: Transactions must be able to add mib to object
- libamxo: it must be possible to include a directory
- libamxo: It must be possible to add and remove flags to parameters in odl files
- libamxo: Add amxo_parser_load_mib function
- libamxb: Add amxb_async_call

### Changes

- libamxo: Rename keyword flags to userflags
- libamxo: Always save persistent instance regardless of template object attributes
- libamxo: Make it possible to call entry-points in reverse order
- amxrt: - Calls entry-points in reverse order when stopping

### Fixes

- libamxc: amxc_var_get_path is not usable when keys contain dots
- libamxp: when right value of in comparator is a csv_string or a ssv_string they must function as an list
- libamxo: Clean-up resolvers after reading odl files
- libamxo: Send dm:instance-added events when creating instances from odl file
- libamxb: Subscription on native ubus objects fail
- amxb_pcb: When a client subscribes for events future events must be taken into account
- amxb_pcb: [yocto] non -dev/-dbg/nativesdk- package contains symlink .so
- amxb_ubus: When a client subscribes for events future events must be taken into account
- amxb_ubus: Missing link flag in makefile
- amxb_ubus: Remove subscription object when unsubscribing
- amxb_ubus: non -dev/-dbg/nativesdk- package contains symlink .so

## Release 0.8.1 - 2021-03-16(10:49:47 +0000)

### Fixes

- set correct version of htable contacts example in manifest

## Release 0.8.0 - 2021-03-15(10:40:33 +0000)

### New

- libamxp: Process controller - launch and monitor child processes and track children
- libamxp: Process info - fetch process information from /proc/<pid>/stat
- example ssh-server: Adds load and save functionality

### Changes

- libamxc: Extend string variant conversion to bool
- libamxj: Adding new cross compilation builds requires packages of dependencies on artifactory
- libamxp: It must be possible to limit number of instances with a parameter reference
- libamxp: It must be possible to filter parameters on attributes
- libaxmo: The function table resolve should provide a function name to default add instance action
- libamxo: Handle data model events before post includes
- amxrt: Add more command line start-up options
- example ssh-server: Use `or` instead of `||` in expressions for SAH build system
- example ssh-server: Disable print_event
- example ssh-server: Removes dropbear control implementation
- example ssh-server: Use amxp_proc_info as fallback to get dropbear child processes

### Fixes

- libamxp: Must block SIGINT for child processes
- amxrt: When auto-detect is set all back-ends in back-end dir should be loaded
- amxrt: .gitignore - ingores version.h
- example ssh-server: Check status before launching dropbear

## Release 0.7.0 - 2021-03-04(08:08:17 +0000)

### New

- libamxo: It must be possible to include odl files that are loaded after invoking the entry points
- amxb-inspect: Check if read-raw is available
- amxrt: Add the possibility to enable system signals through eventloop
- amxrt: Runtime must call the entry points with reason AMXO_ODL_LOADED (2) when post include files are loaded

### Changes

- all: Migrate to new licenses format (baf)
- libamxc: API documentation mentions AMXC_VAR_FLAG_XXXX instead of full names
- libamxc: API Documentation iterator APIs should be put in a sub-group
- libamxp: Launching a child process with arguments can be clumsy when using amxp_subproc_t
- libamxd: Add doxygen documentation to functions
- libamxd: Large inline functions should be moved to implementation files
- libamxb: Add missing error pragmas for dependencies
- amxb_pcb: Update back-end interface function table
- amxb_ubus: Update back-end interface function table

### Fixes

- libamxp: Removing a signal from within a slot causes a segmentation fault
- libamxd: There seems to be an inconsistency between parameters setters and getters for objects.

## Release 0.6.2 - 2021-02-17(10:54:00 +0000)

### Changes

- libamxp: Add doxygen documentation tags to all public APIs
- libamxb: Add doxygen documentation tags to all public APIs (except back-end interface part)

### Fixes

- libamxc: Documentation is not matching implementation (amxc_ts functions)
- libamxj: Converting a string variant with a NULL pointer as data to a variant of type json gives a segmentation fault
- amxb_ubus: Delete Instance Methods Does Not Cover Child Objects
- amxb_ubus: Calling a method on an instance using the name fails
- example tr181-localagent-threshold: Check reason code in `_threshold_instance_is_valid` action implementation
- example subscribe: README.md - Update README to use updated 'amx-subscribe' application name
- example variant_contacts: remove artifact from repository

## Release 0.6.1 - 2021-02-04(07:07:00 +0000)

### New

- libamxo: It must be possible to get a list of open listen sockets
- amxrt: should not exit when there are still open listen sockets

### Changes

- libamxc: Add doxygen documentation tags to all public APIs

### Fixes

- libamxd: When selecting a NULL object in a transaction, the transaction apply crashes
- example greeter_app: Only dump event data when available
- example greeter_app: Makefile - Fixes clean target
- example greeter_plugin: Makefile - Fixes clean target
- example tr181-localagent-threshold: README.md - install instructions

## Release 0.6.0 - 2021-01-20(06:53:00 +0000)

### New

- all: Auto generate make files using build agnostic file (baf)

### Changes

- libamxt: Adds and updates control key sequences
- libamxt: Re-organizes source files
- libamxb: Delete operator always must return an empty array when no instances are deleted, but multi-instance object exists
- libamxd: Delete function always must return an empty array when no instances are deleted, but multi-instance object exists
- libamxd: Instance add/delete events must provide the key parameters
- amxb_ubus: When RPC call result contains `retval` field, extract it and use as the return value of the RPC
- amxb_pcb: Check if the root object is an Ambiorix data model object instead of the full path
- amxb_pcb: Delete operator always must return an empty array when no instances are deleted, but multi-instance object exists

### Fixes

- libamxj: Fixes memory leak in move implementation if json_string_t variant
- libamxt: Do not auto-cast variant when value is text between quotes
- libamxt: Fixes seg-fault in amxt_tty_read
- libamxd: Sets correct type of parameter values in set function
- amx-cli: Failing commands should not return 0
- amx-cli: Recording of commands is not working when cli module uses sigalrm
- amxb_pcb: PCB function argument types are not correctly translated
- example htable_contacts: Fixes segmentation fault when no command line arguments are provided.

## Release 0.5.1 - 2021-01-11(07:04:00 +0000)

### Changes

- example greeter_app - disable package deployment (examples do not generate packages)

## Release 0.5.0 - 2021-01-11(07:04:00 +0000)

### New

- libamxb: Support in API and back-end interface for listen and accept sockets (`amxb_listen`, `amxb_accept`).
- libamxb: Introspection functions added (`amxb_be_get_info`, `amxb_be_list`).
- libamxb: Execute a function for all listen sockets (`amxb_be_for_all_listeners`).
- libamxb: Change data model access (public/protected) for a bus context (`amxb_set_access`).
- libamxc: New APIs: `amxc_string_set`, `amxc_var_cast`
- libamxc: String variants (AMXC_VAR_ID_CSTRING, cstring_t) can be auto converted to other types. (Use AMXC_VAR_ID_ANY).
- libamxd: Add default data model function `exec`. Can execute function using `search paths`.
- libamxt: Command parser generic helper functions
- libamxt: Completion generic helper functions
- amxb_pcb: Support for listen and accept sockets
- amxb_pcb: Implements set, add, del, list back-end interface functions
- amxb_ubus: Implements list back-end interface function
- amxb_ubus: Data model first level only registration (experimental).
- amxrt: Support for listen and accept sockets.
- amxrt: Added unit-tests.
- example greeter_app: Added unit-tests.
- example greeter_plugin: Support for PCB listen socket

### Changes

- libamxb: Major API changes to better support/match USP interfaces and specifications (See notes 1).
- libamxb: Major back-end interface changes (See notes 2).
- libamxd: Describe action (default implementation) can provide list of child-objects or instances
- libamxd: All default data model RPC methods have an extra argument `rel_path` which can contain the filter or wildcard part of a path.
- amxb_pcb: Updates back-end interface implementation (see libamxb).
- amxb_pcb: Re-factor of unit-tests
- amxb_ubus: Updates back-end interface implementation (see libamxb).
- example greeter_plugin: extended unit-tests.

### Fixes

- libamxc: Variant logging to syslog (using API `amxc_var_log`) is improved.
- libamxc: Improved indentation with `amxc_var_dump`.
- libamxp: Fixed subprocess fork PID checking (`amxp_subproc_vstart`).
- libamxd: Fixes index versus key path auto-detection.
- libamxo: Fixes incompatibilities with PCB odl parser.
- libamxt: Silent mode (no output to terminal).
- amxb_pcb: Fixes PCB index versus key path detection

### Notes

1. Major up-step of libamxb API

Some API's have been changed to be more compatible with the USP (TR-369) specification.
The following functions have been update and are not backwards compatible:

- amxb_set
- amxb_add
- amxb_del
- amxb_get_supported
- amxb_describe

To aid the transition the old versions of the functions are still available with a changed name:

- amxb_set_v1
- amxb_add_v1
- amxb_del_v1

All boolean arguments in the functions `amxb_get_supported` and `amxb_describe` are now replaced
with a `flags` argument which is a bit map.

2. Major up-step of libamxb back-end interface

The following back-end interfaces are changed.

- amxb_be_get_t
- amxb_be_set_t
- amxb_be_add_t
- amxb_be_del_t
- amxb_be_get_supported_t
- amxb_be_describe_t'

The paths can be passed as two arguments, the first (object) will contain the fixed part, while
the second (search_path) can contain filters or wild-cards. Not all existing bus systems
can handle filters and/or wildcards. This part of the path can now be passed as an argument to the
corresponding RPC method.


## Release 0.4.0 - 2020-12-02(18:19:03 +0000)

### New

- libamxc: It must be possible to move the content of one variant into another variant
- libamxc: Add amxc_string_replace function
- libamxc: Add doxygen documentation
- libamxp: Add doxygen documentation
- libamxb: It must be possible to retrieve a bus context
- libamxo: Add function to lookup connection context by file descriptor
- libamxt: Terminal API - Easy to use APi to create interactive terminal applications
- libamxm: Modularity - load and add functionality to your applications (add-on system)
- Data Collection Examples: llist_contacts, htable_contacts, variant_contacts. 

### Changes

- amxrt: ubus.sock directory changed
- amxb_ubus: path == NULL should be supported
- libamxd: Make the default data model RPC functions more USP compliant
- libamxd: The get supported datamodel arguments should be more in line with USP
- libamxd: Modify implementation of default data model get function
- All: updates done in the readme files

### Fixes

- libamxc: a trimmed amxc_string does not print correctly
- libamxd: Invalid object retrieval using partial name
- libamxd: Virtual root object does not contain any functions
- libamxd: When describing instances the path should not contain the index
- amxb_pcb: When invoking a method the backend must detect if the path is an index or key path
- amxb_pcb: When describing instances the path should not contain the index

### Notes

1. Major up-step of libamxc 
The variant interface has been changed. This makes libamxc binary incompatible. When updating to this new major version all applications, libraries depending on libamxc must be rebuilded.

2. Following libs need at least version v1.0.0 of libamxc
- libamxp v0.6.1
- libamxt v0.1.1

3. Data model `get_supported` functions (libamxd - libamxb)
In this function one of the arguments has changed name and meaning. The argument `recursive` is renamed to `first_level_only`. This to be more in line with the USP specification.
This change can effect the use of the related functions. Where previously `true` was used for the `recursive` argument, now `false` must be used for `first_level_only` argument to have the same result.
