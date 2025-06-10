# OVSP4RT Core Logic V1

This folder contains the legacy version of the ovsp4rt core,
corresponding roughly to IPDK release `mev-ts-1.9`.

This version of the core is built by default.

## Source Files

The core logic is contained in a single source file,
`ovsp4rt.cc`. A header file exposes prototypes for a number
of the functions in this file, to support unit testing.

## Unit Tests

Most of the core logic is tightly coupled to the network session,
making it unsuitable for unit testing. The chief exception is the
protobuf encoders.

A suite of unit tests was added after the fact, to validate the
encoder functions.

Test coverage is as follows:

| Target | Num Tests | Lines | Functions |
|--------|:---------:|------:|----------:|
| DPDK   | 6         | 54.6% | 44.8%     |
| ES2K   | 30        | 64.0% | 52.0%     |

## Changes from MEV-TS-1.9

The following changes have been made since `mev-ts-1.9`, to permit
this version to coexist with newer versions, and to make it easier
to do side-by-side comparisons between versions.

### Functions renamed

Backported from v2:

- Functions that encode a protobuf are now prefixed with `Encode`
  instead of `Prepare`.
- Functions that call an encoder and issue a `WriteRequest`
  are now prefixed with `Write` instead of `Config`.
- Functions that call an encoder and issue a `ReadRequest`
  are now prefixed with `Read` instead of `Get`.

### Files renamed

Backported from v2:

- The file that declares function prototypes for the encoder
  functions is now named `ovsp4rt_encoders.h` instead of
  `ovsp4rt_internal.h`.

### Files moved

Backported from v3:

- Moved `ovsp4rt.cc` to the `core/v1` folder.
