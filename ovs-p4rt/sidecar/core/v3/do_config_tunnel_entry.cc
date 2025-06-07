// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Implements DoConfigTunnelEntry() (dpdk, es2k)

#include <arpa/inet.h>

#include "client/ovsp4rt_client_interface.h"
#include "core/api/ovsp4rt_internal_api.h"
#include "core/common/ovsp4rt_encoders.h"
#include "core/common/ovsp4rt_entry_utils.h"
#include "logging/ovsp4rt_logutils.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace ovsp4rt {

//----------------------------------------------------------------------
// WriteEncapTableEntry
//----------------------------------------------------------------------

// Ipv4, Tagged
void PrepareEncapTableEntry(p4::v1::TableEntry* table_entry,
                            const struct tunnel_info& tunnel_info,
                            const ::p4::config::v1::P4Info& p4info,
                            bool insert_entry) {
#if defined(DPDK_TARGET)
  EncodeVxlanEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
#elif defined(ES2K_TARGET)
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    EncodeVxlanEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    EncodeGeneveEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }
#endif
}

#if defined(ES2K_TARGET)

// Ipv6, Tagged
void PrepareV6EncapTableEntry(p4::v1::TableEntry* table_entry,
                              const struct tunnel_info& tunnel_info,
                              const ::p4::config::v1::P4Info& p4info,
                              bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    EncodeV6VxlanEncapTableEntry(table_entry, tunnel_info, p4info,
                                 insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    EncodeV6GeneveEncapTableEntry(table_entry, tunnel_info, p4info,
                                  insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }
}

// Ipv4, Untagged
void PrepareEncapAndVlanPopTableEntry(p4::v1::TableEntry* table_entry,
                                      const struct tunnel_info& tunnel_info,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    EncodeVxlanEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                         insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    EncodeGeneveEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                          insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }
}

// Ipv6, Untagged
void PrepareV6EncapAndVlanPopTableEntry(p4::v1::TableEntry* table_entry,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    EncodeV6VxlanEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                           insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    EncodeV6GeneveEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                            insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }
}

void Es2kPrepareEncapTableEntry(::p4::v1::TableEntry* table_entry,
                                const struct tunnel_info& tunnel_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  if (tunnel_info.local_ip.family == AF_INET &&
      tunnel_info.remote_ip.family == AF_INET) {
    if (tunnel_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
      PrepareEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                       insert_entry);
    } else {
      PrepareEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
    }
  } else if (tunnel_info.local_ip.family == AF_INET6 &&
             tunnel_info.remote_ip.family == AF_INET6) {
    if (tunnel_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
      PrepareV6EncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                         insert_entry);
    } else {
      PrepareV6EncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
    }
  }
}

#endif  // ES2K_TARGET

absl::Status WriteEncapTableEntry(ClientInterface& client,
                                  const struct tunnel_info& tunnel_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

#if defined(DPDK_TARGET)
  PrepareEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
#elif defined(ES2K_TARGET)
  Es2kPrepareEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  return client.sendWriteRequest(write_request);
}

//----------------------------------------------------------------------
// WriteDecapTableEntry
//----------------------------------------------------------------------

#if defined(ES2K_TARGET)

// port_vlan_mode == P4_PORT_VLAN_NATIVE_TAGGED
void PrepareDecapModTableEntry(p4::v1::TableEntry* table_entry,
                               const struct tunnel_info& tunnel_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    EncodeVxlanDecapModTableEntry(table_entry, tunnel_info, p4info,
                                  insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    EncodeGeneveDecapModTableEntry(table_entry, tunnel_info, p4info,
                                   insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }
}

// port_vlan_mode == P4_PORT_VLAN_NATIVE_TAGGED
void PrepareDecapModAndVlanPushTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    EncodeVxlanDecapModAndVlanPushTableEntry(table_entry, tunnel_info, p4info,
                                             insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    EncodeGeneveDecapModAndVlanPushTableEntry(table_entry, tunnel_info, p4info,
                                              insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }
}

void Es2kPrepareDecapTableEntry(::p4::v1::TableEntry* table_entry,
                                const struct tunnel_info& tunnel_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  if (tunnel_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_TAGGED) {
    PrepareDecapModTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else {
    PrepareDecapModAndVlanPushTableEntry(table_entry, tunnel_info, p4info,
                                         insert_entry);
  }
}

absl::Status WriteDecapTableEntry(ClientInterface& client,
                                  const struct tunnel_info& tunnel_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  Es2kPrepareDecapTableEntry(table_entry, tunnel_info, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

#endif  // ES2K_TARGET

//----------------------------------------------------------------------
// WriteTunnelTermTableEntry
//----------------------------------------------------------------------

#if defined(ES2K_TARGET)

void Es2kPrepareTunnelTermTableEntry(p4::v1::TableEntry* table_entry,
                                     const struct tunnel_info& tunnel_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  if (tunnel_info.local_ip.family == AF_INET &&
      tunnel_info.remote_ip.family == AF_INET) {
    EncodeTunnelTermTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.local_ip.family == AF_INET6 &&
             tunnel_info.remote_ip.family == AF_INET6) {
    EncodeV6TunnelTermTableEntry(table_entry, tunnel_info, p4info,
                                 insert_entry);
  }
}

#endif  // ES2K_TARGET

absl::Status WriteTunnelTermTableEntry(ClientInterface& client,
                                       const struct tunnel_info& tunnel_info,
                                       const ::p4::config::v1::P4Info& p4info,
                                       bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

#if defined(DPDK_TARGET)
  EncodeTunnelTermTableEntry(table_entry, tunnel_info, p4info, insert_entry);
#elif defined(ES2K_TARGET)
  Es2kPrepareTunnelTermTableEntry(table_entry, tunnel_info, p4info,
                                  insert_entry);
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  return client.sendWriteRequest(write_request);
}

//----------------------------------------------------------------------
// ConfigTunnelEntry
//----------------------------------------------------------------------

absl::Status ConfigTunnelEntry(ClientInterface& client,
                               const struct tunnel_info& tunnel_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry) {
  absl::Status status;

  status = WriteEncapTableEntry(client, tunnel_info, p4info, insert_entry);
  if (!status.ok()) return status;

#if defined(ES2K_TARGET)
  status = WriteDecapTableEntry(client, tunnel_info, p4info, insert_entry);
  if (!status.ok()) return status;
#endif

  return WriteTunnelTermTableEntry(client, tunnel_info, p4info, insert_entry);
}

//----------------------------------------------------------------------
// DoConfigTunnelEntry
//----------------------------------------------------------------------

absl::Status DoConfigTunnelEntry(ClientInterface& client,
                                 const struct tunnel_info& tunnel_info,
                                 bool insert_entry, const char* grpc_addr) {
  absl::Status status;

  // Start a new client session.
  status = client.connect(grpc_addr);
  if (!status.ok()) return status;

  // Fetch P4Info object from server.
  ::p4::config::v1::P4Info p4info;
  status = client.getPipelineConfig(&p4info);
  if (!status.ok()) return status;

  // Update P4 tables.
  return ConfigTunnelEntry(client, tunnel_info, p4info, insert_entry);
}

}  // namespace ovsp4rt
