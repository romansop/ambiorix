# RBus Backend Requirements

## __Table of Contents__  

[[_TOC_]]

## Description

In this document the requirements for the ambiorix rbus backend are listed. The back-end covers two major parts, the provider side and the consumer side.

The provider side main responsibilities are:
1. Registering the data model to rbus as defined by the application.
1. Handling incoming requests and provide correct replies as it would be done by a native rbus data model provider implementation.

The consumer side main responsibilities are:
1. Translate amxb requests into rbus-requests, this includes support for USP like features (e.g. search-paths).
1. Translate the reply from rbus into ambiorix data structures.

All requirements must be implemented and covered by unit-tests. This document provides for each requirement which unit test implementation (directory) covers the requirement and which scenarios are covered.

---
> **NOTE**<br>
> Unit-tests are common good-practice but can never proof that an implementation is bug free or issue free. If issues are detected in the future, extra test scenarios must be added that covers the issue. The newly added test should fail initialy and when the issue is fixed it should succeed. Fixes without unit-tests will not be accepted.
---

## RBus Backend RunTime Configuration

---
> **CFG-01**: The rbus-backend must be able to provide the default RBus connection URI. If the provided configuration options don't provide the `uris` option, the rbus backend must add the default (currently `rbus:`), if the `uris` option is provided nothing should be added or updated.<br>

The unit-tests verifying this requirement is implemented in `test/amxb_rbus_test_config`. The handling of the set config requests is implemented in `src/amxb_rbus.c`.

The following scenarios are covered:
- Set an empty configuration and check if the backend provides the default uri: `test_amxb_rbus_provides_default_uri`
- Set a configuration where uri(s) are defined and check if the backend doesn't overwrite them: `test_amxb_rbus_does_not_overwrite_provided_uris`

---

---
> **CFG-02**: If the `uris` option is provided to the rbus-backend but is not a list, the back-end will silently discard the invalid option and provide the default `uris` as a list.<br>

The unit-tests verifying this requirement is implemented in `test/amxb_rbus_test_config`. The handling of the set config requests is implemented in `src/amxb_rbus.c`.

The following scenarios are covered:
- Set a configuration where the uris is not a list type: `test_amxb_rbus_does_not_accept_invalid_uris_type`

---

---
> **CFG-03**: The amxb-rbus back-end must be configurable and must accept configuration options that can configure when and what will be registered to the RBus daemon (rtrouted):
> - Register the suppported data model at application start (app:start event): `register-on-start-event`.
> - Skip registering of part of the top level objects in the supported data model: `skip-register`.
> - Do not use or register ambiorix internal (protected) data model methods: `use-amx-calls`.
> - The name that must be used to register to the RBus daemon: `register-name`.
> - Translate the object names provided by the ambiorix data model provided: `translate`

No specific unit-tests are implemented for this requirement. The amxb-rbus backend will accept any configuration option, but unknown options are ignored and not used. Unit-tests that verify that the defined options are correctly used are implemented in other unit-tests, (See Regstering & Discovery).

---

---
> **CFG-04**: The rbus-backend must indicate if ambiorix internal (protected) data model methods are supported. If the configuration option is not provided that rbus-backend must set the option `use-amx-calls` to the default (currently set to true).<br>

The unit-tests verifying this requirement is implemented in `test/amxb_rbus_test_config`. The handling of the set config requests is implemented in `src/amxb_rbus.c`.

The following scenarios are covered:
- When a configuration is provided that doesn't have `use-amx-calls` then the backend must provided it and set it to the default value (true): `test_amxb_rbus_indicates_if_amx_calls_are_supported`
- When a configuration is provided where `use-amx-calls` is set to false, the backend must accept this option: `test_amxb_rbus_accept_option_to_turn_off_amx_calls`

---

## Ambiorix Data Model Provider RBus Compatibility

An ambiorix data model provider must register its supported data model and available rows to rbus, the tests verifies that the correct rbus APIs are called to do the registration. A native rbus provider must be able to access these data models using the rbus APIs.

The following APIs are used in the tests or by an ambiorix data model provider:
- `rbus_open`
- `rbus_close`
- `rbus_regDataElements`
- `rbusTable_registerRow`
- `rbusTable_unregisterRow`
- `rbusMethod_SendAsyncResponse`
- `rbus_discoverComponentDataElements`
- `rbusTable_getRowNames`
- `rbus_discoverRegisteredComponents`
- `rbus_discoverComponentName`
- `rbusElementInfo_get`
- `rbus_get`
- `rbus_getExt`
- `rbus_getBoolean`
- `rbus_getInt`
- `rbus_getUint`
- `rbus_getStr`
- `rbus_set`
- `rbus_setBoolean`
- `rbus_setUInt`
- `rbus_setStr`
- `rbus_setMulti`
- `rbusTable_addRow`
- `rbusTable_removeRow`
- `rbus_discoverWildcardDestinations`

### Data Conversions

When data translation is needed from or to Rbus many of the Rbus API value functions are used as well. These are not mentioned here in detail.
See the rbus header file `rbus_value.h`. All data translation from ambiorix to rbus is implemented in `src/amxb_rbus_to_robject.c` and all data translation from rbus to ambiorix is implemented in `src/amxb_rbus_to_var`.
No individual tests are implemented for the data conversions, it is assumed that when the data conversions are not correct or fail, that it will result in failure of the top call.

If the need for individual data conversion tests is prove, they can be added.

### Connecting & Disconnecting

---
> **CON-01**: The rbus-backend must be able to connect to the RBus daemon (rtrouted) and must use the RBus api `rbus_open`. The amxb-rbus backend must be able to provide a registration mame if none is provided in the configuration. If a name is provided in the configuration that name must be used and passed to `rbus_open`. The return code of `rbus_open` must be propagated to the upper apis.<br>

The unit-tests that verifies if rbus_open is called with the correct arguments is implemented in `test/amxb_rbus_test_connect`. Opening a connection to the rbus daemon is implemented in `src/amxb_rbus.c`.

The following scenarios are covered:
- Call `amxb_connect` without providing any configuration options: `test_amxb_rbus_connects_using_rbus_open`
- Call `amxb_connect` after providing backend configuration options which contains a register-name: `test_amxb_rbus_connects_using_rbus_open_with_configured_name`
- Call `amxb_connect` and make `rbus_open` fail, checks that `amxb_connect` is reporting the failure in the return code: `test_amxb_rbus_can_handle_rbus_open_failure`
- Call `amxb_connect` and make the creation of the socket pair fail, checks that `amxb_connect` is reporting the failure in the return code: `test_amxb_rbus_connect_fails_when_no_socket_pair_can_be_created`

---

---
> **CON-02**: The rbus-backend must be able to disconnect from the RBus daemon (rtrouted). The amx-rbus backend must propagate any error code from `rbus_close`.<br>

The unit-tests that verifies if rbus_close is called with the correct arguments is implemented in `test/amxb_rbus_test_connect`. Disconnection from the rbus daemon is implemented in `src/amxb_rbus.c`.

The following scenarios are covered:
- Call `amxb_disconnect` with an amx_bus_ctx that represent a previously successful opened rbus connection: `test_amxb_rbus_can_disconnect_from_rbus_daemon`
- Call `amxb_disconnect` and let `rbus_close` fail, and verifies that the error code is propagated to the caller: `test_amxb_rbus_propagates_failure_to_disconnect`

---

---
> **CON-03**: The rbus-backend must provide a valid file-descriptor that can be added to the applications event-loop for an opened connection and -1 if the connection is closed.<br>

The unit-tests that verifies the backend provides a valid file descriptor when a connection is created is implemented in `test/amxb_rbus_test_connect`. Retrieving the file descriptor is implemented in `src/amxb_rbus.c`.

The following scenarios are covered:
- Open a connection to RBus and check if the file descriptor returned when calling `amxb_get_fd` is a valid file descriptor: `test_amxb_rbus_provides_valid_fd`.
- Close the connection and check that when calling `amxb_get_fd` returns -1: `test_amxb_rbus_provides_valid_fd`

---

### Registering & Discovery

---
> **RD-01**: An ambiorix data model provider that uses the rbus-backend must register its name to the RBus daemon. A native RBus data model consumer must be able to find the component name using the RBus APIs<br>
> This includes following RBus APIs:
> - `rbusCoreError_t rbus_discoverRegisteredComponents(int * count, char *** components)`
> - `rbusError_t rbus_discoverComponentName (rbusHandle_t handle, int numElements, char const** elementNames, int *numComponents, char ***componentName)`

Unit-tests are implemented that verify that the component name is discoverable using RBus APIs. The unit-tests are implemented in `test/amxb_rbus_test_register`. Registering the component name is done using the `rbus_connect` and is implemented in `src/amxb_rbus.c`.

The test scenarios covered are:
- The componenent names are available in the list returned when calling `rbus_discoverRegisteredComponents`. Two ambiorix data model providers are started, one using `amxrt` and the unit-test itself is registering a data model: `test_amxb_rbus_native_can_discover_component_name`
- When requesting the component name that provides an element, this is verified using `rbus_discoverComponentName`, for two amxbiorix data model providers: `test_amxb_rbus_native_can_discover_component_name_using_element`.

---

---
> **RD-02**: An ambiorix data model provider that uses the rbus-backend must use the RBus APIs to register its supported data model. All avaiable parameters, events and methods in the supported data model must be registered.<br>
> This includes following RBus APIs:
> - `rbusError_t rbus_regDataElements(rbusHandle_t handle, int numDataElements, rbusDataElement_t *elements)`

Using function wrapping, provided by the compiler (see [Linking Options](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_3.html)), the unit-test checks that all data elements are registered using calls to `rbus_regDataElements`. The unit-test is implemented in `test/amxb_rbus_test_register`. Registering the supported data model is implemented in `src/amxb_rbus_register.c`.

The test scenarios covered are:
- Load a data model definition from an odl file and check that the data model provider uses the rbus-backend and calls `rbus_regDataElements` for each data element in the defined data model (= supported data model): `test_amxb_rbus_provider_registers_supported_datamodel` (check is done in `__wrap_rbus_regDataElements`).

---

---
> **RD-03**: An ambiorix data model provider that uses the rbus-backend must use the RBus APIs to register available instances (rows) of the multi-instance objects (tables).<br>
> This includes following RBus APIs:
> - `rbusError_t rbusTable_registerRow(rbusHandle_t handle, char const* tableName, uint32_t instNum, char const* aliasName);`

Using function wrapping, provided by the compiler (see [Linking Options](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_3.html)), the unit-test checks that all rows that are available during registration process are added to rbus daemon using calls to `rbusTable_registerRow`. The unit-test is implemented in `test/amxb_rbus_test_register`. Registering the already available rows is implemented in `src/amxb_rbus_register.c`.

The test scenarios covered are:
- Load an odl file that contains instance creation and check that the rbus-backend is registering each of the new rows to rbus using `rbusTable_registerRow`: `test_amxb_rbus_provider_adds_rows` (check is done in `__wrap_rbus_regDataElements`)

---

---
> **RD-04**: When instances (rows) are added or deleted from within the ambiorix data model provider these rows must be registered to the RBus daemon using `rbusTable_registerRow` or `rbusTable_unregisterRow`.<br>

Using function wrapping, provided by the compiler (see [Linking Options](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_3.html)), the unit-tests checks that all newly added rows are made available to the rbus daemon using calls to `rbusTable_registerRow` or when all removed instances are removed from rbus daemon using calls to `rbusTable_unregisterRow`. The unit-test is implemented in `test/amxb_rbus_test_register`. Registering added rows or unregistering removed rows is implemented in `src/amxb_rbus_register.c`.

The test scenarios covered are:
- Create a new instances (rows) in the loaded data model and check if the rbus backend is registering each new added instance as a row to rbus using `rbusTable_registerRow`: `test_amxb_rbus_provider_adds_newly_added_instances` (check is done in `__wrap_rbus_regDataElements`)
- Delete existing instances (rows) in the data model and check if the rbus backend is unregistering each deleted row using `rbusTable_unregisterRow`: `test_amxb_rbus_provider_unregisters_removed_instances` (check is done in `__wrap_rbusTable_unregisterRow`)

---

---
> **RD-05**: An ambiorix data model provider that uses the amxb-rbus backend must be able to delay the registration of the data model until the `app:start` is triggered or emitted.<br>

The registration of the data model to the rbus daemon is implemented in `src/amxb_rbus_register.c`. A unit-test is available that sets the `register-on-start-event` configuration option and then calls `amxb_register`. It checks that the rbus APIs are called when the event is triggered. This unit-test is implemented in `test/amxb_rbus_test_register`.

Test scenarios covered are:
- Set the configuration option `register-on-start-event`, load a data model definition from an odl file and call `amxb_register`. Check that the supported data model is not registered. Trigger "app:start" even and check that the data model is registered after handling the event: `test_amxb_rbus_provider_can_delay_registration_until_app_start_event`
---

---
> **RD-06**: A RBus native data model consumer must be able get the supported data model or rows provided by an ambiorix data model provider using the RBus APIs.<br>

The following test are using `rbus_discoverComponentDataElements` to fetch the supported data model after it was registered to rbus:
- `test_amxb_rbus_provider_registers_supported_datamodel`
- `test_amxb_rbus_provider_can_delay_registration_until_app_start_event`

The following tests are using `rbusTable_getRowNames` to check if the rows are registered to rbus or unregistered from rbus:
- `test_amxb_rbus_provider_adds_rows`
- `test_amxb_rbus_provider_adds_newly_added_instances`
- `test_amxb_rbus_provider_unregisters_removed_instances`

---

---
> **RD-07**: An ambiorix data model provider must not provide a set handler for read-only parameters.<br>

After registering a data model the element info can be retrieved using `rbusElementInfo_get`. This retrieved information includes the access type and element type. The test `test_amxb_rbus_provider_sets_correct_property_handlers` verifies if the element type and access type is as expected.

Test scenario covered:
- Register a data model where at least on multi-instance object (table) is defined as read-only and at least one parameter is defined as read-only, Use `rbusElementInfo_get` to retrieve the element info and verify that the element type and element access type is as expected: `test_amxb_rbus_provider_sets_correct_property_handlers`

---

---
> **RD-08**: An ambiorix data model provider must not provide a tableAddRowHandler and tableRemoveRowHandler for read-only multi-instance objects (tables).<br>

See: RD-07, the same test is covering this requirement.

---

---
> **RD-09**: An ambiorix data model provider can load extra definitions after initial registration. New definition in the supported data model must be registered to rbus.<br>

When new object definitions are loaded in the supported data model, the ambiorix data model provider must register these new objects using `rbus_regDataElements`. The data model egine will emit events when new objects are added.

Test scenario covered:
- After start-up and initial registration load some extra data element definitions and verify that they are registered to rbus. This requires that the events are handled: `test_amxb_rbus_provider_registers_added_definitions_in_supported_datamodel`

---

---
> **RD-10**: An ambiorix data model provider can remove definitions in the supported data model at any time. Removed definitions must be removed from rbus.<br>

When object definitions in the supported data model are removed they must be unregistered from rbus using `rbus_unregDataElements`.

Test scenario covered:
- Delete objects from the supported data model and check that the corresponding data elements are unrigestered from rbus: `test_amxb_rbus_provider_unregisters_deleted_definitions_in_supported_datamodel`

---

### Data Model Access

#### Data Model Get & Set

---
> **DMA-01**: A RBus data model consumer must be able to fetch all parameter values from an ambiorix data model provider using the RBus APIs.<br>
> This includes following RBus APIs:
> - `rbusError_t rbus_get(rbusHandle_t handle, char const* name, rbusValue_t* value);`
> - `rbusError_t rbus_getExt(rbusHandle_t handle, int paramCount, char const** paramNames, int *numProps, rbusProperty_t* properties);`
> - `rbusError_t rbus_getBoolean(rbusHandle_t handle, char const* paramName, bool* paramVal);`
> - `rbusError_t rbus_getInt(rbusHandle_t handle, char const* paramName, int* paramVal);`
> - `rbusError_t rbus_getUint(rbusHandle_t handle, char const* paramName, unsigned int* paramVal);`
> - `rbusError_t rbus_getStr(rbusHandle_t handle, char const* paramName, char** paramVal);`

The unit-tests implemented in `test/amxb_rbus_test_native_get` verifies that the RBus get API's can be used on ambiorix data model providers.
The handling of the get requests is implemented in `src/amxb_rbus_handler_get.c`.

The following scenarios are covered

1. Use `rbus_get` on an existing string parameter and check that it succeeds, the returned type and value is as expected:  `test_native_rbus_get_string_value`
1. Use `rbus_getStr` on an existing string parameter and check that it succeeds and if the expected string value is set: `test_native_rbus_get_string_value`
1. Use `rbus_getStr` on an existing non string parameter and check that it fails: `test_native_rbus_get_string_value`
1. Use `rbus_get` on an existing boolean parameter and check that it succeeds, the returned type and value is as expected: `test_native_rbus_get_bool_value`
1. Use `rbus_getBoolean` on an existing boolean parameter and check that it succeeds and the return value is as expected: `test_native_rbus_get_bool_value`
1. Use `rbus_getBoolean` on an existing non boolean parameter and check that it fails: `test_native_rbus_get_bool_value`
1. Use `rbus_get` on an existing signed integer parameter and check that it succeeds, the returned type and value is as expected: `test_native_rbus_get_int_value`
1. Use `rbus_getInt` on an existing signed integer parameter and check that it succeeds and the return value is as expected: `test_native_rbus_get_int_value`
1. Use `rbus_getInt` on an existing non signed integer parameter and check that it fails: `test_native_rbus_get_int_value`
1. Use `rbus_get` on an existing unsigned integer parameter and check that it succeeds, the returned type and value is as expected: `test_native_rbus_get_uint_value`
1. Use `rbus_getUint` on an existing unsigned integer parameter and check that it succeeds and the return value is as expected: `test_native_rbus_get_uint_value`
1. Use `rbus_getUint` on an existing non unsigned integer parameter and check that it fails: `test_native_rbus_get_uint_value`
1. Use `rbus_getExt` on an existing partial path and check that it succeeds and all parameters are returned, and all of them are matching the expected type: `test_native_rbus_getext_partial_path`

---

---
> **DMA-02**: A RBus data model consumer must be able to set a single writable parameter of an ambiorix data model provider using the RBus APIs. With a single set the operation is commited immediately, when using method `rbus_set` either no `rbusSetOptions_t*` is provided or the commit is set to true.<br>
> This includes following RBus APIs:
> - `rbusError_t rbus_set(rbusHandle_t handle, char const* name, rbusValue_t value, rbusSetOptions_t* opts);`
> - `rbusError_t rbus_setBoolean(rbusHandle_t handle, char const* paramName, bool paramVal);`
> - `rbusError_t rbus_setInt(rbusHandle_t handle, char const* paramName, int paramVal);`
> - `rbusError_t rbus_setUInt(rbusHandle_t handle, char const* paramName, unsigned int paramVal);`
> - `rbusError_t rbus_setStr(rbusHandle_t handle, char const* paramName, char const* paramVal);`

The unit-tests implemented in `test/amxb_rbus_test_native_single_set` verifies that the RBus set API's can be used on a ambiorix data model providers.
The handling of the set requests is implemented in `src/rbus_handler_set.c`.

The following scenarios are covered:

1. Use `rbus_set` without options (NULL) on a string parameter that exists and verifies if the value has been changed in the data model: `test_native_rbus_single_set_string_value`
1. Use `rbus_setStr` on an existing string parameter and verifies if the the value has been changed in the data model: `test_native_rbus_single_set_string_value`
1. Use `rbus_setStr` on an existing none string parameter but data conversion to the correct type is possible, checks that the call succeeds and the new value has been applied: `test_native_rbus_single_set_string_value`
1. Use `rbus_setStr` on an existing none string parameter but data conversion to the correct type is not possible, checks that the call fails: `test_native_rbus_single_set_string_value`
1. Use `rbus_setStr` on a non-existing parameter and checks that the call fails: `test_native_rbus_single_set_string_value`
1. Use `rbus_setStr` on a non-mutable key parameter and checks that the call fails: `test_native_rbus_single_set_string_value`
1. Use `rbus_setStr` on a read-only parameter and checks that the call fails: `test_native_rbus_single_set_string_value`
1. Use `rbus_set` without options (NULL) on a bool parameter that exists and verifies if the value has been changed in the data model: `test_native_rbus_single_set_bool_value`
1. Use `rbus_setBoolean` on an existing bool parameter and verifies if the the value has been changed in the data model: `test_native_rbus_single_set_bool_value`
1. Use `rbus_setBoolean` on an existing none bool but data conversion to the correct type is possible, checks that the call succeeds and the new value has been applied: `test_native_rbus_single_set_bool_value`
1. Use `rbus_setBoolean` on an non-existing parameter and checks that the call fails: `test_native_rbus_single_set_bool_value`
1. Use `rbus_set` without options (NULL) on a signed integer parameter that exists and verifies if the value has been changed in the data model: `test_native_rbus_single_set_int_value`
1. Use `rbus_setInt` on an existing signed integer parameter and verifies if the the value has been changed in the data model: `test_native_rbus_single_set_int_value`
1. Use `rbus_setInt` on an existing none signed integer but data conversion to the correct type is possible, checks that the call succeeds and the new value has been applied: `test_native_rbus_single_set_int_value`
1. Use `rbus_setInt` on an non-existing parameter and checks that the call fails: `test_native_rbus_single_set_int_value`
1. Use `rbus_set` without options (NULL) on an unsigned integer parameter that exists and verifies if the value has been changed in the data model: `test_native_rbus_single_set_uint_value`
1. Use `rbus_setUInt` on an existing unsigned integer parameter and verifies if the the value has been changed in the data model: `test_native_rbus_single_set_uint_value`
1. Use `rbus_setUInt` on an existing none unsigned integer but data conversion to the correct type is possible, checks that the call succeeds and the new value has been applied: `test_native_rbus_single_set_uint_value`
1. Use `rbus_setUInt` on an non-existing parameter and checks that the call fails: `test_native_rbus_single_set_uint_value`

---

---
> **DMA-03**: A RBus data model consumer must be able to set multiple writable parameters of ambiorix data model providers at once using the RBus APIs.<br>
> This includes following RBus APIs:
> - `rbusError_t rbus_set(rbusHandle_t handle, char const* name, rbusValue_t value, rbusSetOptions_t* opts);`
> - `rbusError_t rbus_setMulti(rbusHandle_t handle, int numProps, rbusProperty_t properties, rbusSetOptions_t* opts);`

The unit-tests implemented in `test/amxb_rbus_test_native_multi_set` verifies that the RBus set API's can be used on a ambiorix data model providers.
The handling of the set requests is implemented in `src/rbus_handler_set.c`.

The following scenarios are covered:

1. Use `rbus_setMulti` without options (NULL) to set multiple values of the same object and verifies that all values have been changed: `test_native_rbus_multi_set_using_setMulti_on_one_provider_same_object`
1. Use `rbus_setMulti` without options (NULL) to set multiple values of the same object, provide at least one invalid value and verifies that the call fails and none of the values have been set: `test_native_rbus_multi_set_using_setMulti_on_one_provider_same_object_invalid_value`
1. Use `rbus_setMulti` without options (NULL) to set multiple values of different objects in the same provider and verifies that the values are applied: `test_native_rbus_multi_set_using_setMulti_on_one_provider_different_objects`
1. Use `rbus_setMulti` without options (NULL) to set multiple values of different objects in the same provider, make sure one of the values is invalid, verify that the call fails and none of the values are applied: `test_native_rbus_multi_set_using_setMulti_on_one_provider_different_objects_invalid_value`
1. Use `rbus_setMulti` without options (NULL) to set multiple values of different objects in different providers and verify that the call succeeds and all values are applied: `test_native_rbus_multi_set_using_setMulti_on_more_providers`
1. Use `rbus_setMulti` without options (NULL) to set multiple values of different objects in different providers, make sure one of the values is invalid `test_native_rbus_multi_set_using_setMulti_on_more_providers_invalid_value`<br>
   > The behavior of rbus is not as expected. The call `rbus_setMulti` seems to succeed (RBUS_ERROR_SUCCESS), but the invalid value is not accepted and not set. The ambiorix data model provider is returning an error to rbus, but it seems that the error is ignored by rbus. All values are set except the values for the failing provider.
1. Use `rbus_set` multiple times in sequence with options. Only the last set will contain the commit flag in the options. Verify that all values are only set after the commit: `test_native_rbus_multiple_single_sets_with_commit`
1. Use `rbus_set` multiple times in sequence with options. Never set the commit flag. Verify that none of the values are set: `test_native_rbus_multiple_single_sets_without_commit`

---

---
> **DMA-03-1**: When multiple parameters are set using `rbus_setMulti` and all of them are of the same data model provider, they should all succeed or not applied at all when an invalid value is provided.<br>

This requirement is tested by the unit-tests implemented in `test/amxb_rbus_test_native_multi_set`, for more details see **DMA-03**

---

---
> **DMA-03-2**: When multiple parameters are set using `rbus_setMulti` and multiple data model providers are used, the new values must be all applied, or when an invalid value is provided for one ot them, no changes should be done.<br>

This requirement is tested by the unit-tests implemented in `test/amxb_rbus_test_native_multi_set`, for more details see **DMA-03**<br>

> **NOTE**<br>
> **It seems that only the provider for which on invalid value is set is left unchanged, all others are applied. Is this the expected behavior?**
> **The test succeeds, but the behavior is not as expected, although the ambiorix data model provider is returning an error.**

---

#### Data Model Add & Delete Rows

---
> **DMA-04**: A RBus native data model consumer must be able to add rows on a ambiorix data model provider using the RBus APIs.<br>
> - `rbusError_t rbusTable_addRow(rbusHandle_t handle, char const* tableName, char const* aliasName, uint32_t* instNum);`

The unit-tests implemented in `test/amxb_rbus_test_native_add_delete_row` verifies that the RBus add API can be used on a ambiorix data model providers.
The handling of the add request is implemented in `src/rbus_handler_row.c`.

The following scenarios are covered:

- Add a row to a table and verifies that the individual parameters can be read after the row has been added, also verifies if the row is retured when using `rbusTable_getRowNames`: `test_native_rbus_add_row`
- Add anotherrow to the table and verify if it is returned when fetching the row names: `test_native_rbus_add_row`
- Try to add a row on a non-table object and verify that it fails: `test_native_rbus_add_row_on_non_tables_must_fail`.
---

---
> **DMA-05**: A RBus native data model consumer must be able to delete rows on a ambiorix data model provider using the RBus APIs.<br>
> - `rbusError_t rbusTable_removeRow(rbusHandle_t handle, char const* rowName); `

The unit-tests implemented in `test/amxb_rbus_test_native_add_delete_row` verifies that the RBus add API can be used on a ambiorix data model providers.
The handling of the delete request is implemented in `src/rbus_handler_row.c`.

The following scenarios are covered:
- delete an exising row using `rbusTable_removeRow` and verify that the row is not available any more and it is not possible to fetch a value from it: `test_native_rbus_del_row`
- delete a row from a none table and verify that the call fails: `test_native_rbus_del_non_rows_must_fail`.
- delete a non-existing row and verify that the call fails: `test_native_rbus_del_non_existing_row`

---

---
> **DMA-06**: A RBus native data model consumer must be able to get the list of the available rows from a ambiorix data model provider using the RBus APIs.<br>
> - `rbusError_t rbusTable_getRowNames(rbusHandle_t handle, char const* tableName, rbusRowName_t** rowNames);`

Different unit-tests use the `rbusTable_getRowNames` to verify if the expected rows are available. A common function is made available that is used by multiple unit-tests that needs to verify if the correct rownames are available:

Unit-tests in `test/amxb_rbus_test_native_add_delete_row`
- `test_native_rbus_add_row`
- `test_native_rbus_del_row`
- `test_native_rbus_del_non_existing_row`

Unit-tests in `test/test_amxb_rbus_register`
- `test_amxb_rbus_provider_adds_rows`
- `test_amxb_rbus_provider_adds_newly_added_instances`
- `test_amxb_rbus_provider_unregisters_removed_instances`
---

---
> **DMA-07**: An ambiorix data model provider can fail creation or deletion of rows in tables. The RBus native data model consumer must get an error back.<br>
> - `rbusError_t rbusTable_getRowNames(rbusHandle_t handle, char const* tableName, rbusRowName_t** rowNames);`

When adding a row in table provided by an ambiorix data model provider or deleting a row from a table provided by an ambiorix data model provider fails an error must be returned. Two unit-tests are implemented in `test/amxb_rbus_native_add_delete_row`.

Test scenarios covered:
- Verifies if the table exists and already has at least one row available, try to add a new one that will fail and verifies if no new row has been added: `test_native_rbus_gets_error_when_adding_row_fails`
- Verifies if the table exists and already has at least one row available, try to delete an existing one that will fail and verifies if all 
rows are still available: `test_native_rbus_gets_error_when_deleting_row_fails`

---

#### Data Model Method Invoke

Handling of method calls is implemented in `src/amxb_rbus_handler_call.c`. Because of the multi-threaded nature of RBus a choice has been made to always handle the method calls asynchronous. Therefor the registered callback function to RBus, which will be called when a data model consumers invokes the data model method, is always returning `RBUS_ERROR_ASYNC_RESPONSE`. The requested method call is added to the rbus items queue and will be handled in the main thread. When the method call is finished the response (result) is send back using `rbusMethod_SendAsyncResponse`. 

Ambiorix data model providers will always return an array that contains 3 items for each method call done:

```
[
    <THE RETURN VALUE>,
    {
        <THE OUT ARGUMENTS>
    },
    <AMXD STATUS CODE>
]
```

Example: (in JSON notation)

```JSON
[
    0,
    {
        "data": {
            "HostObject.1.": {
                "Destination": "testDestination",
                "EnvVariable.1.": {
                    "Key": "testKey",
                    "ModuleVersion": "testVersion",
                    "Value": "testValue"
                },
                "Options": "Option1,Option2",
                "Source": "testSource"
            }
        },
        "message": "Hello World"
    },
    0
]
```

In above example the return value of the method call is `0` (integer), the out arguments are `data` and `message`, where data is a table and message is a string, and the amxd status code was `0` (integer).

Note that the return value can be anything, als complex data types like an array, or table, where each individual element can be anything.

---
> **DMA-08**: A Rbus native data model consumer must be able to call ambiorix data model provider methods using the RBus APIs.<br>
> - `rbusError_t rbusMethod_Invoke(rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusObject_t* outParams);`
> - `rbusError_t rbusMethod_InvokeAsync(rbusHandle_t handle, char const* methodName, rbusObject_t inParams, rbusMethodAsyncRespHandler_t callback, int timeout);`

The unit-tests implemented in `test/amxb_rbus_test_native_native_call` verifies that the RBus invoke APIs can be used on a ambiorix data model providers.
The handling of the call requests is implemented in `src/rbus_handler_call.c`. The tests are converting the returned data into ambiorix libamxc variants, to make it easier to verify the returned data. A test data model provider is created for these tests and is partially implemented with LUA and partially using C. The definition and implementation of this test data model provider can be found in `test/common/odl/method_calls.odl` and `test/common/test_mod/test_mod_main.c`

The covered scenarios are:
- Call a data model method from an ambiorix data model provider and verify that the returned data is as expected using `rbusMethod_Invoke`: `test_native_rbus_call_method_synchronous`, `test_native_rbus_call_deferred_method_synchronous`, 
`test_native_rbus_call_method_synchronous_get_out_args`, 
`test_native_rbus_call_deferred_method_synchronous_get_out_args`
- Call a data model method from an ambiorix data model provider and verify that the returned data is as expected using `rbusMethod_SendAsyncResponse`: `test_native_rbus_call_method_asynchronous`,

---

---
> **DMA-09**: An ambiorix data model provider must be able to handle method calls asynchronous and send the response using `rbusMethod_SendAsyncResponse`.<br>

For details and more information about the implemented test, see **DMA-08**

The covered scenarios are:

- Call a data model method on an ambiorix data model provider were the implementation is not returning the data immediately, but send the response later (deferred call). Typically this is useful when lengthy I/O operations are done that can take some time. The control is given back to the bus system and later the response is send. In the tests a timer is used, when the timer expires the response is send: `test_native_rbus_call_deferred_method_synchronous`, `test_native_rbus_call_deferred_method_synchronous_get_out_args`
- Call a data model method from an ambiorix data model provider where the implementation is not returning the data immediately, but sends the response later. If the consumers uses `rbusMethod_InvokeAsync` it must be possible to send another request:

---

---
> **DMA-10**: An ambriorix data model provider must be able to respond with complex data structures or arrays using the RBus data structures (rbusObject_t and rbusValue_t).<br>

For details and more information about the implemented test, see **DMA-08**

The covered scenarios are:

- Call a data model method on an ambiorix data model provider that returns a complex data structure and verify that the returned data is as expected: `test_native_rbus_call_deferred_method_synchronous_get_out_args`, `test_native_rbus_call_method_synchronous_get_out_args`

---

---
> **DMA-11**: An ambriorix data model provider must be able handle different methods in sequence and ensure the data model integrity at all times.<br>

Multiple sources can call methods or perform actions on the data model. The actions must be handled in sequence after each handled actions the data model must be in a known state. As RBus uses threads it is possible that internal action handling and actions requested by other data model consumers are received in parallel. The data model provider must be able to handle this situation and ensure that the data model is in a known state after each action.

The covered scenarios are:

- Call a data model method from an ambiorix data model provider that starts an internal repetitive action on the data model, then call many times a method that acts on the same data elements in the data model and check that the result of the combination of these actions is as expected: `test_native_rbus_call_start_timer` and `test_native_rbus_reset_counter`

As this is a complex scenario to test, the test itself is divided over two test functions, the first `test_native_rbus_call_start_timer` calls a method of the data model provider that starts a timer and increases a counter in the data model at regular intervals. This first functions tests that the counter is increased as expected. The second test functions, `test_native_rbus_reset_counter`,  calls a method that resets the counter to 0 at a faster rate then the one that is increasing the counter. This test checks that the counter is reset as expected. 

---

## Ambiorix Data Model Consumer Compatibility

The ambiorix top-level API for interacting with data model providers is based on the USP specification. The most common operators on a data model are:

- GET
- SET
- ADD
- DELETE
- CALL (ASYNC CALL)
- RESOLVE (find instances using wildcard or search paths)

The amxb-rbus backend implements two different ways to interact with the objects in the data model:
1. Using amx-calls - this will invoke a method (`_get`, `_set`, `_add`, `_del`, ...) on the object. This feature can be turned off using the amxb-rbus backend configuration option `use-amx-call`. When this is enabled the amxb-rbus backend will first verify if the `remote` (data model provider) supports these methods, if not the amxb-rbus backend will fall back to using the RBus API.
2. Using rbus API calls. As RBus is not providing search path functionality, and many of the USP operators allow the usage of search paths, the amxb-rbus backend implementations object filtering using the RBus APIs. The object filtering will fetch the full tree, starting from the path before the search expression, and then filters out the not matching objects. It will provide the output in a amxc_var_t as expected by the top-level api (amxb_get, amxb_set, amxb_add, ...). The implementation of the object filtering can be found in `src/amxb_rbus_resolve.c`

The functions top level amxb functions tested in the unit tests are:

For more information about these functions, read the documentation: [libamxb API](https://prpl-foundation.gitlab.io/components/ambiorix/libraries/libamxb/doxygen/)

- `amxb_be_load`
- `amxb_set_config`
- `amxb_connect`
- `amxb_set_access`
- `amxb_free`
- `amxb_be_remove_all`
- `amxb_call`
- `amxb_async_call`
- `amxb_wait_for_request`
- `amxb_read`
- `amxb_close_request`
- `amxb_describe`
- `amxb_get`
- `amxb_get_multiple`
- `amxb_set`
- `amxb_set_multiple`
- `amxb_resolve`
- `amxb_be_who_has`
- `amxb_add`
- `amxb_del`

When needed these functions will call the correct method in the amxb-rbus backend to execute the request on RBus. 

### Data Model Discovery

#### LIST

Getting a list of available data elements is not an USP compliant function and is mainly used by application that needs the list of the available data elements, like a interactive command line interface with TAB completion.

---
> **ADML-01**: An ambiorix data model consumer must be able to get list of available root objects. To get a list of the available root objects an empty or NULL path must be provided.<br>

This functionality is typically used to discover which root objects are available. When only using TR-181 data models this will return only one object `Device.`. This functionality is implemented in `src/amxb_rbus_list.c`. The ambiorix RBus backend only uses RBus API to build the list of available objects on the root. The RBus API's used are:
- `rbus_discoverRegisteredComponents`
- `rbus_discoverComponentDataElements`

The covered scenarios are:

- Call `amxb_list` with an empty path and check that the returned list only contains the expected root objects: `test_amxb_rbus_list_root`
- Call `amxb_list` with a NULL path and check that the returned list only contains the expected root objects: `test_amxb_rbus_list_root`

---

---
> **ADML-02**: An ambiorix data model consumer must be able to get list of available child-objects or instances of an object.<br>

This functionality is typically used to discover which (child-)objects are available. This functionality is implemented in `src/amxb_rbus_list.c`. The ambiorix RBus backend only uses RBus API to build the list of available objects on the root. The RBus API's used are:
- `rbusElementInfo_get`

The covered scenarios are:

- Call `amxb_list` on a root object and check that the expected sub-objects are returned: `test_amxb_rbus_list_root_object`
- Call `amxb_list` on an object and check that the expected data elements are returned: `test_amxb_rbus_list_object`
- Call `amxb_list` on a table object and check that the expected instances are returned: `test_amxb_rbus_list_instances`
- Call `amxb_list` on an instance object and check that the expected date elements are returned: `test_amxb_rbus_list_instance_items`

---

---
> **ADML-03**: An ambiorix data model consumer must be able to get list of available parameter, methods and events of an object.<br>

This functionality is typically used to discover which data elements are available in an object. This functionality is implemented in `src/amxb_rbus_list.c`. The ambiorix RBus backend only uses RBus API to build the list of available objects on the root. The RBus API's used are:
- `rbusElementInfo_get`

The covered scenarios are:

- Call `amxb_list` on an object and check that the expected data elements are returned: `test_amxb_rbus_list_object`
- Call `amxb_list` on an instance object and check that the expected date elements are returned: `test_amxb_rbus_list_instance_items`

---

---
> **ADML-04**: An ambiorix data model consumer must be able to get list of specific data elements available in an object.<br>

This functionality is typically used to discover specific data elements that are available in an object. This functionality is implemented in `src/amxb_rbus_list.c`. The ambiorix RBus backend only uses RBus API to build the list of available objects on the root. The RBus API's used are:
- `rbusElementInfo_get`

The covered scenarios are:

- Call `amxb_list` on a table object and only request the list of instances. Check that the returned list only contains the expected instances: `test_amxb_rbus_list_objects_instances_only`
- Call `amxb_list` on an object that contains events and only request the list of events. Check that the returned list only contains the expected events: `test_amxb_rbus_list_events_only`
- Call `amxb_list` on an object that contains parameters and only request the list of parameters. Check that the returned list only contains the expected parameters: `test_amxb_rbus_list_parameters_only`
- Call `amxb_list` on an object that contains methods and only request the list of methods. Check that the returned list only contains the expected methods: `test_amxb_rbus_list_methods_only`

---

---

---
> **ADML-05**: An ambiorix data model consumer should only see the internal methods when using protected access.<br>

Internal data model methods can be used to get more information about data elements or the be more USP compliant. 

The covered scenarios are:

- Call `amxb_list` on an object and request all data elements of that object in protected access mode. Check that the returned list contains all the expected elements, including the internal methods: `test_amxb_rbus_list_instance_items_protected_access`

---

---
> **ADML-05**: An ambiorix data model consumer should get an empty list when requesting the list of elements on non-existing objects.<br>

The covered scenarios are:

- Call `amxb_list` using a non-existing object path, and check the returned list is empty: `test_amxb_rbus_list_non_existing_object`.
- Call `amxb_list` without providing a callback function on existing objects or using an empty path and check if the returned list is empty: `test_amxb_rbus_list_no_callback`.

---

#### GET SUPPORTED DATA MODEL

Get supported data model is a USP required data model operation.

The information returned by the RBus APIs regarding the supported data model is very limited, therefor all parameter types are returned as a sting type, all parameters and multi-instance objects are returned as read-write. This is not reflecting the real state of the parameters and objects. To get the correct information RBus must be adapted to return more information. 

The backend only uses RBus APIs to implement the `get supported data model`, no fallback is added to call the internal `_get_supported` data model method. The RBus APIs used to implement the `get supported data model` are:
- `rbus_discoverWildcardDestinations`
- `rbus_discoverComponentDataElements`

---
> **ADMGS-01**: An ambiorix data model consumer get the full supported data model, starting from a certain object, containing all parameter, method, event and sub-object names.<br>

The covered scenarios are:

- Call `amxb_get_supported` on objects and verify that all expected data elements are returned: `test_amxb_rbus_get_supported_data_model`.

---

---
> **ADMGS-02**: An ambiorix data model consumer get the full supported data model, starting from a certain object and only of that object (first level only)<br>

The covered scenarios are:

- Call `amxb_get_supported` on objects with the first level only flag set and verify that all expected data elements are returned: `test_amxb_rbus_get_supported_data_model_first_level_only`.

---

> **ADMGS-03**: An ambiorix data model consumer must be able to only get the list of supported objects in the data model without information of the data elements (parameters, methods and events)<br>

The covered scenarios are:

- Call `amxb_get_supported` on objects with no flags set and verify that all object paths are returned: `test_amxb_rbus_get_supported_data_model_objects_only`.

---


> **ADMGS-04**: Fetching the supported data model on a non-existing object path must result in an empty result.<br>

The covered scenarios are:

- Call `amxb_get_supported` on a non-existing object and verify that the returned result is empty: `test_amxb_rbus_get_supported_data_model_on_not_provided_object`.

---

#### GET INSTANCES

Get instances is a USP required data model operation.

As RBus has no support for key parameters it is assumed that when an instance (row) contains an `Alias` parameter it is a key. All other parameters are not considered a key. When amx-calls is turned on and the data model accessed is an ambiorix based data model, the internal data model method `_get_instances` will be used to retireve the correct information.

---
> **ADMGI-01**: An ambiorix data model consumer must be able to get a list of instances with the key parameters and their values.<br>

The covered scenarios are:

- Call `amxb_get_instances` on table objects (multi-instances) and verifiy that all expected instances are returned including the key parameters: `test_amxb_rbus_get_instances`.

---

---
> **ADMGI-02**: Fetching the instances of a non-table object (singleton or instance) should fail and an error must be returned.<br>

The covered scenarios are:

- Call `amxb_get_instances` on singleton objects or instance objects and verify that an error is returned: `test_amxb_rbus_get_instances_on_non_table`.

---

---
> **ADMGI-03**: When fetching the instances using a depth the depth must be taken into account.<br>

The covered scenarios are:

- Call `amxb_get_instances` on table objects with different depths and verify that the correct information is returned: `test_amxb_rbus_get_instances_with_depth`.

---

---
> **ADMGI-04**: When fetching the instances of non-existing objects an error must be returned.<br>

The covered scenarios are:

- Call `amxb_get_instances` on non-existing objects and verify that an error is returned: `test_amxb_rbus_get_instances_on_non_existing`.

---

### Data Model Access 

#### RESOLVE

---
> **ADMR-01**: An ambiorix data model consumer must be able to get a hint on which connection a data model object could be provided, the backend must be able to check if an object can be provided on a rbus connection.<br>

This functionality is typically used by data model consumers that are connected to multiple bus system or have multiple connections open on the bus system. The implementation can be found in file `src/amxb_rbus_has.c`. The test implementation can be found in `test/amxb_rbus_test_amx_resolve/`.

The covered scenarios are:

- Call `amxb_be_who_has` using a non-existing object path, but for which a partial match is provided. Verify that the amxb-rbus connection is returned: `test_amxb_rbus_get_bus_ctx_using_who_has`.
- Call `amxb_be_who_has` using an object path. Verify that the amxb-rbus connection is returned: `test_amxb_rbus_get_bus_ctx_using_who_has`.
- Call `amxb_be_who_has` using an object path for which no partial match can be found. Verify that NULL is returned: `test_amxb_rbus_get_bus_ctx_using_who_has`.
- Call `amxb_be_who_has` using an an empty path. Verify that NULL is returned: `test_amxb_rbus_get_bus_ctx_using_who_has`.

---

---
> **ADMR-02**: An ambiorix data model consumer must be able get the list of objects using wildcard symbol `*` at the place of the indexes. (See USP specification [Searching by Wildcard](https://usp.technology/specification/index.htm#sec:searching-by-wildcard)).<br>

Resolving object paths to real objects paths is implemented in `src/amxb_rbus_resolve.c`. The tests for path resolving using the wildcard symbol are implemented in `test/amxb_rbus_test_amx_resolve/`.

**NOTE**<br>
RBus itself can use wildcards in object paths, but the semantics of the RBus usage of the wildcard symbol is different to the one defined in the USP specification. In RBus data elements paths the wild card can be used at any place, in USP paths it can be only used on places where normally only an index can be used.

The covered scenarios are:

- Call `amxb_resolve` using an existing object path to a table (multi-instance object) and use the `*` symbol were normally an index can be used, verify that all existing instance paths are returned: `test_amxb_rbus_amxb_can_resolve_wildcard_paths`.  
- Call `amxb_resolve` use a wildcard symbol on a place were it is not allowed according to the USP specification (but it is allowed by rbus) and verify an error is returned: `test_amxb_rbus_amxb_can_resolve_wildcard_paths`.

---

---
> **ADMR-03**: An ambiorix data model consumer must be able get the list of objects using a search expression at the place of the indexes. (See USP specification [Searching with Expressions](https://usp.technology/specification/index.htm#sec:searching-with-expressions)).<br>

Resolving object paths to real objects paths is implemented in `src/amxb_rbus_resolve.c`. The tests for path resolving using a search expressions are implemented in `test/amxb_rbus_test_amx_resolve/`.

The covered scenarios are:

- Call `amxb_resolve` using an existing object path to a table (multi-instance object) and use a search expression were normally an index can be used, verify that all matching instance paths are returned: `test_amxb_rbus_amxb_can_resolve_search_paths`.  
- Call `amxb_resolve` use a search expression on a place were it is not allowed according to the USP specification and verify an error is returned: `test_amxb_rbus_amxb_can_resolve_wildcard_paths`.

---

**NOTE**<br>
As most data model operations have support for wildcard and search paths (GET, SET, ...) all the corresponding requests passed to the amxb-rbus backend will use this resolving implementation.

#### GET

---
> **ADMG-01**: An ambiorix data model consumer must be able to get the object hierarchy starting from a specific object and with a maximum depth specified.<br>

The implementation can be found in file `src/amxb_rbus_get.c` and `src/amxb_rbus_resolve.c` 

All tests use JSON files to verify that the returned data structure is as expected. The JSON file is read and converted to a amxc_var_t. The result of the `get` is then compared with the variant created from the JSON file. 

The covered scenarios are:

- Fetch the full data model object tree starting from the root node, with unlimited depth: `test_amxb_rbus_amxb_get_all`.
- Fetch the data model object tree starting from the root node, with specified depth: `test_amxb_rbus_amxb_get_with_depth`.
- Fetch the data model object tree starting from the a specific object, with unlimited depth: `test_amxb_rbus_amxb_get_specific_recursive`.

---

---
> **ADMG-02**: An ambiorix data model consumer must be able use wildcard symbol '*' in object paths instead of instance numbers as described in USP specification ([2.5.5 Searching by Wildcard](https://usp.technology/specification/index.htm#sec:searching-by-wildcard)) to perform a get.<br>

The implementation can be found in file `src/amxb_rbus_get.c` and `src/amxb_rbus_resolve.c` . 

All tests use JSON files to verify that the returned data structure is as expected. The JSON file is read and converted to a amxc_var_t. The result of the `get` is then compared with the variant created from the JSON file. 

The covered scenarios are:

- Fetch all instances of a table (multi-instance object) with unlimited depth: `test_amxb_rbus_amxb_get_specific_recursive_wild_card`
- Fetch the all sub-objects of all rows in a table in the data model with unlimited depth: `test_amxb_rbus_amxb_get_specific_recursive_wild_card_sub_objects`.
- Fetch all instances of hierarchical tables (at least two tables) where for each table all instances (rows) are requested using the wildcard symbol: `test_amxb_rbus_amxb_get_specific_recursive_wild_card_sub_objects`
- Fetch all instances using a search path followed by a wildcard: `test_amxb_rbus_amxb_get_specific_recursive_search_path`
- Fetch a specific parameter, but of all instances using a wildcard: `test_amxb_rbus_amxb_get_specific_parameter`

---

---
> **ADMG-03**: An ambiorix data model consumer must be able to use search expressions instead of instance numbers as described in USP specification ([2.5.4 Searching with Expressions](https://usp.technology/specification/index.htm#sec:searching-with-expressions)) to perform a get.<br>

The implementation can be found in file `src/amxb_rbus_get.c` and `src/amxb_rbus_resolve.c` . 

All tests use JSON files to verify that the returned data structure is as expected. The JSON file is read and converted to a amxc_var_t. The result of the `get` is then compared with the variant created from the JSON file. 

The covered scenarios are:

- Fetch all instances (rows) matching a search expression from table: `test_amxb_rbus_amxb_get_specific_recursive_search_path`
- Fetch instances where a value of a sub-object is used in the search expression and fetch a sub-object which is a table and use a wildcard: `test_amxb_rbus_amxb_get_specific_recursive_wild_card_sub_objects`.
- Fetch a specific parameter of all instances matching a search expression: `test_amxb_rbus_amxb_get_specific_parameter`

---

---
> **ADMG-04**: An ambiorix data model consumer must be able use parameter paths as described in the USP specification ((2.5 Path Names)[https://usp.technology/specification/index.htm#sec:path-names]) to get a specific parameter. These paths may contain wildcards or search expressions at the place where instance numbers can be specified.<br>

The implementation can be found in file `src/amxb_rbus_get.c` and `src/amxb_rbus_resolve.c` . 

All tests use JSON files to verify that the returned data structure is as expected. The JSON file is read and converted to a amxc_var_t. The result of the `get` is then compared with the variant created from the JSON file. 

The covered scenarios are:

- Fetch a parameter in a specific row: `test_amxb_rbus_amxb_get_specific_parameter`
- Fetch all parameters using a search and wild card path: `test_amxb_rbus_amxb_get_specific_parameter`.

---

---
> **ADMG-05**: An ambiorix data model consumer must get an error code back when the get operation failed. Search paths or wildcard paths that are not returning data must return success with an empty data set.<br>

The implementation can be found in file `src/amxb_rbus_get.c` and `src/amxb_rbus_resolve.c` . 

All tests use JSON files to verify that the returned data structure is as expected. The JSON file is read and converted to a amxc_var_t. The result of the `get` is then compared with the variant created from the JSON file. 

The covered scenarios are:

- Fetch non-existing objects: `test_amxb_rbus_amxb_get_non_existing_must_fail`
- Fetch a non-exising row: `test_amxb_rbus_amxb_get_non_existing_must_fail`
- Fetch instances using a search expression but no instance is matching: `test_amxb_rbus_amxb_get_empty_result`.

---

---
> **ADMG-06**: An ambiorix data model consumer must be able to get multiple objects at once using `amxb_get_multiple`. It must be able to provide a list of paths (object paths, search paths, wildcard paths, parameter paths) and get the response in one data structure for each requested path.<br>

The implementation can be found in file `src/amxb_rbus_get.c` and `src/amxb_rbus_resolve.c` . 

All tests use JSON files to verify that the returned data structure is as expected. The JSON file is read and converted to a amxc_var_t. The result of the `get multiple` is then compared with the variant created from the JSON file. 

The covered scenarios are:

- Fetch multiple objects using different path types and verify that for each requested path the correct response is available in the returned data: `test_amxb_rbus_amxb_get_multiple`.
- Check if an empty table is returned for search paths where no matching objects is found: `test_amxb_rbus_amxb_get_multiple`

---

---
> **ADMG-06**: An ambiorix data model consumer must be able to use reference following paths in get requests as specified in USP specification ([2.6.1 Reference Following](https://usp.technology/specification/index.htm#sec:reference-following)).<br>

The implementation can be found in file `src/amxb_rbus_get.c` and `src/amxb_rbus_resolve.c`. 

**Note:**
Reference following using `*` (follow all) is currently not supported by ambiorix implementation.

All tests use JSON files to verify that the returned data structure is as expected. The JSON file is read and converted to a amxc_var_t. The result of the `get multiple` is then compared with the variant created from the JSON file. 

The covered scenarios are:

- Fetch the value of a parameter containing a reference and verify that the value is returned: `test_amxb_rbus_amxb_get_reference_following`
- Use reference following to fetch the object that is referenced by the parameter value and verify that the reference object is returned: `test_amxb_rbus_amxb_get_reference_following`

---

#### SET

The implementation can be found in file `src/amxb_rbus_set.c`, when using search paths and wildcard paths the path resolving is done using the functions implemented in `src/amxb_rbus_resolve.c`. 

All tests use JSON files to verify that the returned data structure is as expected. The JSON file is read and converted to a amxc_var_t. The result of the `amxb_set` or `amxb_set_multiple` is then compared with the variant created from the JSON file.

More information about SET and SET MULTIPLE using allow partial and optional parameters can be found in the USP specification.

---
> **ADMS-01**: An ambiorix data model consumer must be able to set a single parameter of a specific object(s) using object path, wildcard path or search path.<br>

Covered scenarios:
- Set a specific parameter of a specific object and verify the returned result, also verifies if the value has been applied: `test_amxb_rbus_amxb_set_single_parameter`.
- Set a specific parameter of a all instances (using wildcard path) and verify the returned result, also verifies if all values has been applied: `test_amxb_rbus_amxb_set_single_parameter`.
- Set a specific parameter of a some instances (using search path) and verify the returned result, also verifies if all values has been applied: `test_amxb_rbus_amxb_set_single_parameter`.

---

---
> **ADMS-02**: An ambiorix data model consumer must be able to set a multiple parameters of a specific object(s) using object path. wildcard path or search path.<br>

Covered scenarios:
- Set a multiple parameters of a specific object and verify the returned result, also verifies if the value has been applied: `test_amxb_rbus_amxb_set_multiple_parameters_same_object`.
- Set a specific parameter of a some instances (using search path) and verify the returned result, also verifies if all values has been applied: `test_amxb_rbus_amxb_set_single_parameter`.

---

---
> **ADMS-03**: An ambiorix data model consumer must be able to set a multiple parameters of different objects with allow partial set to true or false and get the correct response back.<br>

Covered scenarios:
- Set a multiple parameters of objects with partial allowed and verify the returned result, also verifies if the value has been applied: `test_amxb_rbus_amxb_set_multi_partial_allowed`.
- Set a multiple parameters of objects no partial allowed and verify the returned result, also verifies if the value has been applied: `test_amxb_rbus_amxb_set_multi_no_partial_allowed_valid_values`, `test_amxb_rbus_amxb_set_multi_no_partial_allowed_valid_values`, `test_amxb_rbus_amxb_set_multi_no_partial_allowed_invalid_values`.

---

---
> **ADMS-04**: An ambiorix data model consumer must be able to set a multiple parameters of different objects with optional parameters.<br>

Covered scenarios:
- Set a multiple parameters of objects with optional parameters and verify the returned result, also verifies if the value has been applied: `test_amxb_rbus_amxb_set_multi_partial_allowed_optional_params`.

---

---
> **ADMS-05**: An ambiorix data model consumer must be able to call `amxb_set` without providing parameter values.<br>

Covered scenarios:
- Call `amxb_set` using an empty parameter table and verify that the call succeeds but a NULL result is returned: `test_amxb_rbus_amxb_set_without_values`.

---

---
> **ADMS-06**: An ambiorix data model consumer must get an error back when setting non-existing parameters or setting a parameter on a non-existing object.<br>

Covered scenarios:
- Call `amxb_set` on a non-existing object path or with a non-existing parameter and verify that the returned status is an error code: `test_amxb_rbus_amxb_set_non_existing`.

---

---
> **ADMS-07**: An ambiorix data model consumer that sets multiple parameters on different objects with at least one invalid value with allow partial set to false, must get the correct response and no data is changed.<br>

Covered scenarios:
- Set a multiple parameters of objects no partial allowed with invalid values and verify the returned result, also verifies if the value has not been applied: `test_amxb_rbus_amxb_set_multi_no_partial_allowed_invalid_values`.

---

---
> **ADMS-08**: An ambiorix data model consumer that sets multiple parameters on different objects with at least one invalid value as optional parameter with allow partial set to false, must get the correct response and all parameters are applied except the invalid optional parameter.<br>

---

#### ADD

The implementation can be found in file `src/amxb_rbus_add.c`, when using search paths and wildcard paths the path resolving is done using the functions implemented in `src/amxb_rbus_resolve.c`. 

All tests that needs to verify a larger data set use JSON files to verify that the returned data structure is as expected. The JSON file is read and converted to a amxc_var_t. 

All `amxb_add` tests are implemented in `test/amxb_rbus_test_amx_add`, all verify data in json format is available in `test/amxb_rbus_test_amx_add/verify_data`

---
> **NOTES ON USP COMPATIBILITY**<br>
> According to the USP specification when adding instances the key parameters and theire values must be returned of the newly created instance(s). As rbus is not returning these it will not always be possible to add this to the reply. When RBus APIs are used the returned key parameters will be an empty table.
---

---
> **ADMA-01**: An ambiorix data model consumer must be able to add a new instance (row) to a multi-instance object (table) and must recieve at least the index of the newly added instance.<br>

Covered scenarios:
- Call `amxb_add` on an existing table and check if the returned data is as expected, verifies if the newly created instance can be retrieved  using `amxb_get`: `test_amxb_rbus_add_instance`

---

---
> **ADMA-02**: An ambiorix data model consumer must be able to add a new instance (row) to a multi-instance object (table) using a specific index.<br>

Covered scenarios:
- Call `amxb_add` on an existing table and provide the request index and check if the returned data is as expected, verifies if the newly created instance can be retrieved  using `amxb_get`. The requested index is ignored when using RBus APIs, this is covered by the test as well:`test_amxb_rbus_add_instance_request_index`

---

---
> **NOTE**<br>
> When the rbus api is used it will not be possible to request that an instances is created/added with a specific index. It is not possible to pass a requested index to the provider. The provider will just add the new row using the next available index.
---

---
> **ADMA-03**: An ambiorix data model consumer must be able to add a new instance (row) to a multi-instance object (table) and provide parameter values for the new instance.<br>

Covered scenarios:
- Call `amxb_add` on an existing table and provide parameter values for the new instance and check if the returned data is as expected, verifies if the newly created instance can be retrieved  using `amxb_get` and contain the correct values for the parameters: `test_amxb_rbus_add_instance_and_set_param_values`

---

---
> **ADMA-04**: An ambiorix data model consumer must get an error back when adding an instance and providing invalid parameter values. The instance should not be created.<br>

Covered scenarios:
- Call `amxb_add` on an existing table and provide parameter values for the new instance and at least one value is invalid. Check that an error is returned and verify that no rows were added: `test_amxb_rbus_add_instance_with_invalid_values`.

---

---
> **NOTE ADMA-03 & ADMA-04**<br>
> When the rbus api is used it will not be possible to pass parameter values with the add row request, the amxb-rbus backend solves this by doing two requests, the first to add the row and the second to set the parameter values. When setting the parameter values is failing the row will be deleted. So using the rbus api adding a row with parameter values can not be considered as an atomic operation on the data model.
> <br>
> For some ambiorix data model providers this could be a problem as these data models are mainly based on the usp specifications. Instance objects which contain key parameters could be problematic.
---

---
> **ADMA-05**: An ambiorix data model consumer must be able to add a new instances (rows) using a search or wild-card path.<br>

Covered scenarios:
- Call `amxb_add` on an existing tables using a wildcard path and check that multiple instances are added, also verifies that all rows including the new ones can be retrieved using `amxb_get`: `test_amxb_rbus_add_instances_using_wildcard_path`
- Call `amxb_add` on an existing tables using a search path and check that instances are added on the correct objects, also verifies that all rows including the new ones can be retrieved using `amxb_get`: `test_amxb_rbus_add_instances_using_search_path`
- Call `amxb_add` on an existing tables using a search path with no matching objects and check no error is returned (empty set) and no rows were added: `test_amxb_rbus_add_instances_using_search_path_no_matches`

---

---
> **ADMA-06**: An ambiorix data model consumer must be able to add a new instance (row) and provide an alias value.<br>

Covered scenarios:
- Call `amxb_add` on an existing table and provide an alias (name), check that the returned data is as expected, verify that the newly added row can be retrieved and contains the correct Alias value: `test_amxb_rbus_add_instance_with_alias`

---

---
> **NOTE ADMA-06**<br>
> According to the USP specification, when no Alias is provided when creating the instance, the provider can generate it's own alias. All ambiorix data model providers will do this. When the rbus api is used by a consumer and no alias is provided, the ambiorix data model provider can not register the generated alias to rbus. Doing that causes that the same row is available in rbus twice.
---

---
> **ADMA-07**: An ambiorix data model consumer that adds a row with a duplicate alias must get an error back and no row must be added.<br>

Covered scenarios:
- Call `amxb_add` on an existing table and provide an alias (name) that already exists, check if an error is returned and verify that no row has been added: `test_amxb_rbus_add_instance`

---

---
> **ADMA-08**: An ambiorix data model consumer that adds a row on a non-existing table or non-table object should recieve an error.<br>

Covered scenarios:
- Call `amxb_add` on a non-existing object and check that it returns an error: `test_amxb_rbus_add_instance_on_not_existing_table`
- Call `amxb_add` on a non-table object and check that it returns an error: `test_amxb_rbus_add_instance_on_non_table`

---

#### DELETE

The implementation can be found in file `src/amxb_rbus_del.c`, when using search paths and wildcard paths the path resolving is done using the functions implemented in `src/amxb_rbus_resolve.c`. 

All `amxb_del` tests are implemented in `test/amxb_rbus_test_amx_del`.

---
> **ADMD-01**: An ambiorix data model consumer must be able to delete a single instance (row) of a multi-instance object (table) and must recieve the list of deleted objects and sub-objects.<br>

Covered scenarios:
- Call `amxb_del` on an existing instance and check if the returned data is as expected, verifies if the deleted instance can not be retrieved anymore using `amxb_get`: `test_amxb_rbus_delete_single_instance`, `test_amxb_rbus_all_removed_subobjects_are_returned`

---

---
> **ADMD-02**: An ambiorix data model consumer must be able to delete all instances of rows of a multi-instance objects using wild-card or search-paths and must recieve the list of deleted objects and sub-objects.<br>

Covered scenarios:
- Call `amxb_del` using a wildcard path or search path and verifies if all instances that are matching are deleted and the deleted instances can not be retrieved anymore using `amxb_get`: `test_amxb_rbus_delete_all_instances`, `test_amxb_rbus_all_removed_subobjects_are_returned`
- Call `amxb_del` using a wildcard path or search path and verifies if all instances that are matching are deleted and the deleted instances can not be retrieved anymore using `amxb_get`: `test_amxb_rbus_delete_no_instances_found`, `test_amxb_rbus_delete_no_index_no_alias`, `test_amxb_rbus_all_removed_subobjects_are_returned`

---

---
> **ADMD-03**: An ambiorix data model consumer must recieve an error when deletion of an instance fails. If multiple instances are matching and one fails to be deleted no instance should be deleted<br>

Covered scenarios:
- Call `amxb_del` on a path for which deletion will fail verify if an error is returned: `test_amxb_rbus_deletion_fails`

---

---
> **NOTE ADMD-03**<br>
> When the RBus api is used, a deletion for all matching instances must be done one by one. If one of the instances can not be deleted and and error is returned, the already deleted instances will be deleted and can not be reverted. The amxb-rbus backend will continue and delete as much as possible. This is a different behavior then when using amx data model calls or with some other bus systems. 
---


#### CALL & ASYNCHRONOUS CALL

The implementation can be found in file `src/amxb_rbus_invoke.c`, when using search paths and wildcard paths the path resolving is done using the functions implemented in `src/amxb_rbus_resolve.c`. 

All `amxb_call` and `amxb_async_call` tests are implemented in `test/amxb_rbus_test_amx_call`.

---
> **ADMC-01**: An ambiorix data model consumer must be able to call a data model method synchronously and providing a timeout and input arguments and must get the return value and output arguments back.<br>

Covered scenarios:
- Call `amxb_call` on an existing data model method providing an input argument and verify that the return value is as expected: `test_amxb_rbus_call_method_synchronous`
- Call `amxb_call` on an existing data model method providing no input arguments and verify that the returned complex out argument is as expected: `test_amxb_rbus_call_method_with_out_args`

---

---
> **ADMC-02**: An ambiorix data model consumer must be able to call a data model method asynchronously and later wait for the response with a specified time-out. When the wait time expires a timeo-out error must be given but the method call must not be canceled (unless `amxb_close_request` is called) <br>

Covered scenarios:
- Call `amxb_async_call` on an existing data model method and use `amxb_wait_for_request` to wait until the method is finished and verify the return value: `test_amxb_rbus_call_method_asynchronous_and_wait`
- Call `amxb_async_call` on an existing data model method and call `amxb_wait_for_request` after the function was already finished, verify the return value before and after the wait: `test_amxb_rbus_call_method_asynchronous_and_wait_after_done`
- Call `amxb_async_call` on an existing data model method that takes a long time to finish and call `amxb_wait_for_request` with a shorter time and verifies a timeout is returned but the method calls is not canceled : `test_amxb_rbus_call_method_asynchronous_and_wait_timeout`

---

---
> **ADMC-03**: An ambiorix data model consumer must be able to call a data model method asynchronously and close the request immediately (fire and forget)<br>

Covered scenarios:
- Call `amxb_async_call` on an existing data model and close the request immediately using `amxb_close_request`: `test_amxb_rbus_call_method_asynchronous_and_close`
  Suggestion - Some extra validation functionality must be added to verify that the function was executed (by using an event?)

---

---
> **ADMC-04**: An ambiorix data model consumer must be able to call a data model method asynchronously without providing a done callback function, but still must be able to wait until it is complete and get the return value or out arguments<br>

Covered scenarios:
- Call `amxb_async_call` on an existing data model without providing a done callback function, wiat until it is done using `amxb_wait_for_request` and verify the return value: `test_amxb_rbus_call_method_asynchronous_no_callback`

---

---
> **ADMC-05**: An ambiorix data model consumer get an error when the method can not be called or when the method fails to start asynchronously.<br>

Covered scenarios:
- Call `amxb_async_call` on a non-existing object or call a non-existing method of an existing object and verify that an error is returned: `test_amxb_rbus_call_non_existing_method`
- Call `amxb_async_call` and make the creation of the thread fail and verify if an error is returned: `test_amxb_rbus_call_method_asynchronous_and_pthread_create_fails`

---

### Data Model Introspection

---
> **ADMI-01**: An ambiorix data model consumer must be able to get the full meta data of an object. This includes:<br>
> - all the defined parameters of an object with the correct parameter type and at least the read-only attribute for each of them.
> - all events defined events of an object
> - all defined functions, if possible with the correct return type and all the input and output arguments, if possible with the correct type.
> - all its instance indexs and names (for multi-instance objects)
> - all its child objects

The implementation can be found in file `src/amxb_rbus_describe.c`. 

The tests are implemented in `test/amxb_rbus_test_amx_describe.c`. The tests are done using amx call `_describe` and also without the amx call, just using the RBus API. The output for a `describe` request can be different when using the amx call then using the RBus API. The information that can be retrieved using RBus API is limited. 
Because of the difference in the returned meta-data the tests only verify that the minimum information is returned. This information includes:
- All parameters, for each of them the parameter name, parameter type (type name and type id), the current value, and the read-only attribute.
- All events, for each of them the event name
- All functions, for each of them the function name, the function return type (type name and type id). Note that when using RBus API the type name and type id will always be the `NULL` type as RBus doesn't provide that information. The function arguments are aslo not checked as RBus API doesn't provide that information. 
- The object information itself, index (if applicable) and name, the path, object type (template, instance or singleton), the list of instances (if applicable) and list of child objects.


Test scenarios covered:
- Get full description of an instance object using amx call `_describe` and verify that the minimum meta data is returned: `test_amxb_rbus_describe_instance_full_using_amx_calls` 
- Get full description of an instance object using RBus API and verify that the minimum meta data is returned: `test_amxb_rbus_describe_instance_full_not_using_amx_calls` and `test_amxb_rbus_describe_instance_full`
- Get description of an instance object only using amx call `_describe` and verify that the minimum meta data is returned and no object elements are included (parameters, events, ...): `test_amxb_rbus_describe_instance_only_using_amx_calls` 
- Get description of an instance object only using RBus API and verify that the minimum meta data is returned and no object elements are included (parameters, events, ...): `test_amxb_rbus_describe_instance_only_not_using_amx_calls` and `test_amxb_rbus_describe_instance_only`
- Get full description of a table (multi-instance) object using amx call  `_describe` and verify that the minimum meta data is returned: `test_amxb_rbus_describe_table_full_using_amx_calls` 
- Get full description of a table (multi-instance) object using RBus API and verify that the minimum meta data is returned: `test_amxb_rbus_describe_table_full_without_amx_calls` and `test_amxb_rbus_describe_table_full`
- Get description of a table (multi-instance) object only using amx call  `_describe` and verify that the minimum meta data is returned and no object elements are included (parameters, events, ...): `test_amxb_rbus_describe_table_only_using_amx_calls` 
- Get description of a table (multi-instance) object only using RBus API and verify that the minimum meta data is returned: `test_amxb_rbus_describe_table_only_without_amx_calls` and `test_amxb_rbus_describe_table_only`
- Get full description of a singleton object using amx call  `_describe` and verify that the minimum meta data is returned: `test_amxb_rbus_describe_singelton_full_using_amx_calls` 
- Get full description of a singleton object using RBus API and verify that the minimum meta data is returned: `test_amxb_rbus_describe_singelton_full_without_amx_calls` and `test_amxb_rbus_describe_singleton_full`
- Get object description only of a singleton object using amx call  `_describe` and verify that the minimum meta data is returned and no object elements are included (parameters, events, ...): `test_amxb_rbus_describe_singelton_object_only_with_amx_calls` 
- Get object description only of a singleton object using RBus API and verify that the minimum meta data is returned and no object elements are included (parameters, events, ...): `test_amxb_rbus_describe_singelton_object_only_without_amx_calls` and `test_amxb_rbus_describe_object_only`
- Get full description of a none existing objects and verify if an empty table is returned: `test_amxb_rbus_describe_on_none_existing_returns_empty_table`

---

### Data Model Eventing

#### WAIT FOR OBJECT

The implementation can be found in file `src/amxb_rbus_wait_for.c`.
All `amxb_wait_for_object` tests are implemented in `test/amxb_rbus_wait_for`.

---
> **ADMWF-01**: An ambiorix data model consumer must be able wait until an object becomes available in the data model and must get informed when an object is avaialble through an event.<br>

Covered scenarios:
- Call `amxb_wait_for_object` on an existing data model object and checks if the event is recieved immediately: `test_amx_setup_wait_for_already_available_object`
- Call `amxb_wait_for_object` on an non-existing data model object and check if an event is recieved as soon as the object becomes available: `test_amx_setup_wait_unavailable_object`
- Call `amxb_wait_for_object` on an multiple non-existing data model objects and check if an event is recieved as soon as all objects becomes available: `test_amx_setup_wait_multiple_unavailable_objects`

---
