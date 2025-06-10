# OVSP4RT Core Logic V3

This version of the ovsp4rt core moves the code into its own
directory and splits the primary source files into smaller files.

To build this version, set the cmake variables `BUILD_CLIENT` and
`OVSP4RT_V3` to a *True* value (`ON`, `TRUE`, etc.).

## Changes from Version 2

1. Created a new `core` folder and moved the source files in
   `sidecar`, together with the `dpdk` and `es2k` folders, into
   subfolders under `core`.

2. Split the files for `DoConfigFdbEntry` and `DoConfigTunnelSrcPortEntry`
   into two files each, one for DPDK and one for ES2K. Moved the API
   function, together with its unique supporting functions, to each file.

3. Split `ovsp4rt_encoders.cc` into two files, one for DPDK and
   one for ES2K.

4. Created a separate source file for `ReadTxAccVsiTableEntry` and
   `GetTxAccVsiPort`, which are common to two of the internal APIs.

In addition to making each internal API a cohesive unit, this structure
removes compile-time conditionals from the code.

## Source Files

The `core` source tree looks like this:

```text
core/
├── CMakeLists.txt
├── api
│   ├── CMakeLists.txt
│   ├── ovsp4rt_internal_api.h
│   ├── ovsp4rt_journal_api.cc
│   ├── ovsp4rt_standard_api.cc
│   └── ovsp4rt_str_to_tunnel_type.cc
├── common
│   ├── CMakeLists.txt
│   ├── ovsp4rt_core_utils.h
│   ├── ovsp4rt_encoders.h
│   └── ovsp4rt_entry_utils.h
├── dpdk
│   ├── CMakeLists.txt
│   └── p4_name_mapping.h
├── es2k
│   ├── CMakeLists.txt
│   └── p4_name_mapping.h
├── v1
│   ├── CMakeLists.txt
│   ├── README.md
│   └── ovsp4rt.cc
├── v2
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── ovsp4rt_encoders.cc
│   └── ovsp4rt_v2.cc
└── v3
    ├── CMakeLists.txt
    ├── README.md
    ├── do_config_ip_mac_map_entry.cc
    ├── do_config_rx_tunnel_src_entry.cc
    ├── do_config_src_port_entry.cc
    ├── do_config_tunnel_src_port_entry.cc
    ├── do_config_vlan_entry.cc
    ├── dpdk_config_fdb_entry.cc
    ├── dpdk_config_tunnel_entry.cc
    ├── dpdk_encoders.cc
    ├── es2k_config_fdb_entry.cc
    ├── es2k_config_tunnel_entry.cc
    ├── es2k_encoders.cc
    ├── tx_acc_vsi_table_entry.cc
    └── tx_acc_vsi_table_entry.h
```
