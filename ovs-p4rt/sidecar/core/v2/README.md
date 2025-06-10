# OVSP4RT Core Logic V2

This version of the core has been refactored to make it more testable.
Some maintainability improvements have also been made.

To build with this version of the core, set the `BUILD_CLIENT` cmake
variable to `ON`, `TRUE`, etc.

## Changes from Version 1

1. Defined `ClientInterface` class to provide an abstract interface
   to the P4 server. Created a concrete `Client` subclass to encapsulate
   interactions with the `OvsP4RtSession` object.

2. Split each public API function into an external API and an
   internal API. Moved the external APIs to a separate file
   (`ovsp4rt_standard_api.cc`), to allow substitutions to be made
   at build time.

   - The external API (e.g. `ovsp4rt_config_tunnel_entry`)
     instantiates a `Client` object and passes it to the internal API.

   - The internal API (e.g. `DoConfigTunnelEntry`) does the actual
     processing.

3. Replaced the `OvsP4RtSession` object with `ClientInterface`
   throughout the core code.

4. Renamed functions to clarify their purpose.

   - Functions that encode a protobuf are now prefixed with `Encode`
     instead of `Prepare`.

   - Functions that call an encoder and issue a `WriteRequest`
     are now prefixed with `Write` instead of `Config`.

   - Functions that call an encoder and issue a `ReadRequest`
     are now prefixed with `Read` instead of `Get`.

5. Moved the `Encode` functions to a separate source file,
   `ovsp4rt_encoders.cc`. Renamed `ovsp4rt_internal.h` to
   `ovsp4rt_encoders.h`.

6. Refactored functions to reduce complexity and create seams for
   unit testing.

7. Defined `TestClient` and `TestClientMock` classes to facilitate
   unit testing.

8. Wrote additional tests to validate functions that are now
   testable.

## Source Files

### Original source tree

In the original release of version 2, the core files were laid out
as follows.

```text
sidecar/
├── newovsp4rt.cc
├── ovsp4rt_api_utils.cc
├── ovsp4rt_config_int.h
├── ovsp4rt_do_config_int.h
├── ovsp4rt_encoders.cc
├── ovsp4rt_encoders.h
├── ovsp4rt_journal_api.cc
├── ovsp4rt_standard_api.cc
├── ovsp4rt_util_int.h
```

### Revised source tree

When the files moved under the `core` directory for version 3,
the files were redistributed as follows. A number of them
were renamed.

```text
sidecar/core/
├── api
│   ├── ovsp4rt_internal_api.h
│   ├── ovsp4rt_journal_api.cc
│   ├── ovsp4rt_standard_api.cc
│   └── ovsp4rt_str_to_tunnel_type.cc
├── common
│   ├── ovsp4rt_core_utils.h
│   ├── ovsp4rt_encoders.h
│   └── ovsp4rt_entry_utils.h
├── v2
│   ├── ovsp4rt_encoders.cc
│   └── ovsp4rt_v2.cc
```

The mapping between the original and revised files is:

| Original file           | Revised file                      |
|-------------------------|-----------------------------------|
| newovsp4rt.cc           | core/v2/ovsp4rt_v2.cc             |
| ovsp4rt_api_utils.cc    | core/api/ovsp4rt_str_to_tunnel_type.cc |
| ovsp4rt_config_int.h    | core/common/ovsp4rt_entry_utils.h |
| ovsp4rt_do_config_int.h | core/api/ovsp4rt_internal_api.h   |
| ovsp4rt_encoders.cc     | core/common/ovsp4rt_encoders.h    |
| ovsp4rt_encoders.h      | core/common/ovsp4rt_encoders.h    |
| ovsp4rt_journal_api.cc  | core/api/ovsp4rt_journal_api.cc   |
| ovsp4rt_standard_api.cc | core/api/ovsp4rt_standard_api.cc  |
| ovsp4rt_util_int.h      | core/common/ovsp4rt_core_utils.h  |

## Unit Tests

Test coverage is as follows:

| Target | Num Tests | Lines | Functions |
|--------|:---------:|------:|----------:|
| DPDK   | 13        | 92.9% | 91.3%     |
| ES2K   | 67        | 92.4% | 92.2%     |
