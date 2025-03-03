// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include <arpa/inet.h>

#include <string>

#include "absl/flags/flag.h"
#include "client/ovsp4rt_client.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "logging/ovsp4rt_logging.h"
#include "logging/ovsp4rt_logutils.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"
#include "ovsp4rt_do_config_int.h"
#include "ovsp4rt_private.h"
#include "ovsp4rt_util_int.h"

#if defined(DPDK_TARGET)
#include "dpdk/p4_name_mapping.h"
#elif defined(ES2K_TARGET)
#include "es2k/p4_name_mapping.h"
#endif

namespace ovsp4rt {

static inline int32_t ValidIpAddr(uint32_t nw_addr) {
  return (nw_addr && nw_addr != INADDR_ANY && nw_addr != INADDR_LOOPBACK &&
          nw_addr != 0xffffffff);
}

#ifdef ES2K_TARGET

absl::Status ConfigFdbSmacTableEntry(ClientInterface& client,
                                     const struct mac_learning_info& learn_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareFdbSmacTableEntry(table_entry, learn_info, p4info, insert_entry,
                           detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

// extracted from ConfigL2TunnelTableEntry
void PrepareL2TunnelTableEntry(p4::v1::TableEntry* table_entry,
                               const struct mac_learning_info& learn_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry, DiagDetail& detail) {
  if (learn_info.tnl_info.local_ip.family == AF_INET6 &&
      learn_info.tnl_info.remote_ip.family == AF_INET6) {
    PrepareL2ToTunnelV6(table_entry, learn_info, p4info, insert_entry, detail);
  } else {
    PrepareL2ToTunnelV4(table_entry, learn_info, p4info, insert_entry, detail);
  }
}

absl::Status ConfigL2TunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareL2TunnelTableEntry(table_entry, learn_info, p4info, insert_entry,
                            detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

#endif  // ES2K_TARGET

// called-by: ConfigFdbVlanEntry (dpdk, es2k)
absl::Status ConfigFdbTxVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareFdbTxVlanTableEntry(table_entry, learn_info, p4info, insert_entry,
                             detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

// called-by: ConfigFdbVlanEntry (dpdk, es2k)
absl::Status ConfigFdbRxVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareFdbRxVlanTableEntry(table_entry, learn_info, p4info, insert_entry,
                             detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

#if defined(ES2K_TARGET)
// extracted from ConfigFdbTunnelTableEntry
void Es2kPrepareFdbTunnelTableEntry(p4::v1::TableEntry* table_entry,
                                    const struct mac_learning_info& learn_info,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry, DiagDetail& detail) {
  if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                         insert_entry, detail);
  } else if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareFdbTableEntryforV4GeneveTunnel(table_entry, learn_info, p4info,
                                          insert_entry, detail);
  } else {
    if (!insert_entry) {
      // Tunnel type doesn't matter for delete. So calling one of the functions
      // to prepare the entry
      PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                           insert_entry, detail);
    }
  }
}
#endif  // ES2K_TARGET

absl::Status ConfigFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

#if defined(DPDK_TARGET)
  PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                       insert_entry, detail);
#elif defined(ES2K_TARGET)
  Es2kPrepareFdbTunnelTableEntry(table_entry, learn_info, p4info, insert_entry,
                                 detail);
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

// Ipv4, Tagged
void PrepareEncapTableEntry(p4::v1::TableEntry* table_entry,
                            const struct tunnel_info& tunnel_info,
                            const ::p4::config::v1::P4Info& p4info,
                            bool insert_entry) {
#if defined(DPDK_TARGET)
  PrepareVxlanEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
#elif defined(ES2K_TARGET)
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareVxlanEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareGeneveEncapTableEntry(table_entry, tunnel_info, p4info,
                                 insert_entry);
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
    PrepareV6VxlanEncapTableEntry(table_entry, tunnel_info, p4info,
                                  insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareV6GeneveEncapTableEntry(table_entry, tunnel_info, p4info,
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
    PrepareVxlanEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                          insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareGeneveEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
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
    PrepareV6VxlanEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                            insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareV6GeneveEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                             insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }
}

#endif  // ES2K_TARGET

#if defined(ES2K_TARGET)
// called-by: ConfigEncapTableEntry
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

// called-by: DoConfigTunnelEntry (common)
absl::Status ConfigEncapTableEntry(ClientInterface& client,
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

#if defined(ES2K_TARGET)

// called-by: Es2kPrepareDecapTableEntry
// port_vlan_mode == P4_PORT_VLAN_NATIVE_TAGGED
void PrepareDecapModTableEntry(p4::v1::TableEntry* table_entry,
                               const struct tunnel_info& tunnel_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareVxlanDecapModTableEntry(table_entry, tunnel_info, p4info,
                                   insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareGeneveDecapModTableEntry(table_entry, tunnel_info, p4info,
                                    insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }
}

// called-by: Es2kPrepareDecapTableEntry
// port_vlan_mode == P4_PORT_VLAN_NATIVE_TAGGED
void PrepareDecapModAndVlanPushTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareVxlanDecapModAndVlanPushTableEntry(table_entry, tunnel_info, p4info,
                                              insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareGeneveDecapModAndVlanPushTableEntry(table_entry, tunnel_info, p4info,
                                               insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }
}

// called-by: ConfigDecapTableEntry
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

// called-by: DoConfigTunnelEntry (es2k)
absl::Status ConfigDecapTableEntry(ClientInterface& client,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  Es2kPrepareDecapTableEntry(table_entry, tunnel_info, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

// called-by: DoConfigVlanEntry (es2k)
absl::Status ConfigVlanPushTableEntry(ClientInterface& client,
                                      const uint16_t vlan_id,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareVlanPushTableEntry(table_entry, vlan_id, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

// called-by: DoConfigVlanEntry (es2k)
absl::Status ConfigVlanPopTableEntry(ClientInterface& client,
                                     const uint16_t vlan_id,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareVlanPopTableEntry(table_entry, vlan_id, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetL2ToTunnelV4TableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  PrepareL2ToTunnelV4(table_entry, learn_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetL2ToTunnelV6TableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  PrepareL2ToTunnelV6(table_entry, learn_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

// extracted from GetFdbTunnelTableEntry
void PrepareFdbTableV4TunnelEntry(p4::v1::TableEntry* table_entry,
                                  const struct mac_learning_info& learn_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry, DiagDetail& detail,
                                  bool testing) {
  // We ignore the 'insert_entry' parameter and specify 'false' when
  // calling the selected function. get-entry, like remove-entry, only
  // needs the match fields.
  //
  // The optional 'testing' parameter (which defaults to 'false') allows
  // the unit test to override this behavior.
  if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                         testing, detail);
  } else if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareFdbTableEntryforV4GeneveTunnel(table_entry, learn_info, p4info,
                                          testing, detail);
  }
}

absl::StatusOr<::p4::v1::ReadResponse> GetFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool adding) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

#if defined(DPDK_TARGET)
  PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info, false,
                                       detail);
#elif defined(ES2K_TARGET)
  PrepareFdbTableV4TunnelEntry(table_entry, learn_info, p4info, false, detail);
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  return client.sendReadRequest(read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetFdbVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool adding) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  PrepareFdbTxVlanTableEntry(table_entry, learn_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

// called-by: DoConfigIpMacMapEntry (es2k)
absl::StatusOr<::p4::v1::ReadResponse> GetVmSrcTableEntry(
    ClientInterface& client, struct ip_mac_map_info ip_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  PrepareSrcIpMacMapTableEntry(table_entry, ip_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

// called-by: DoConfigIpMacMapEntry (es2k)
absl::StatusOr<::p4::v1::ReadResponse> GetVmDstTableEntry(
    ClientInterface& client, const struct ip_mac_map_info& ip_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  PrepareDstIpMacMapTableEntry(table_entry, ip_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

// called-by: DoConfigSrcPortEntry, ConfigFdbUpdateSrcPort
absl::StatusOr<::p4::v1::ReadResponse> GetTxAccVsiTableEntry(
    ClientInterface& client, uint32_t sp,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initReadRequest(&read_request);

  PrepareTxAccVsiTableEntry(table_entry, sp, p4info);

  return client.sendReadRequest(read_request);
}

// called-by: DoConfigSrcPortEntry (es2k)
absl::Status ConfigVsiSrcPortTableEntry(ClientInterface& client,
                                        const struct src_port_info& sp,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareSrcPortTableEntry(table_entry, sp, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

// extracted from ConfigRxTunnelSrcPortTableEntry
void PrepareRxTunnelSrcPortTableEntry(p4::v1::TableEntry* table_entry,
                                      const struct tunnel_info& tunnel_info,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry) {
  if (tunnel_info.local_ip.family == AF_INET &&
      tunnel_info.remote_ip.family == AF_INET) {
    PrepareRxTunnelTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.local_ip.family == AF_INET6 &&
             tunnel_info.remote_ip.family == AF_INET6) {
    PrepareV6RxTunnelTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  }
}

// called-by: DoConfigRxTunnelSrcEntry (es2k)
absl::Status ConfigRxTunnelSrcPortTableEntry(
    ClientInterface& client, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareRxTunnelSrcPortTableEntry(table_entry, tunnel_info, p4info,
                                   insert_entry);

  return client.sendWriteRequest(write_request);
}

// called-by: ConfigTunnelTermTableEntry
void Es2kPrepareTunnelTermTableEntry(p4::v1::TableEntry* table_entry,
                                     const struct tunnel_info& tunnel_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  if (tunnel_info.local_ip.family == AF_INET &&
      tunnel_info.remote_ip.family == AF_INET) {
    PrepareTunnelTermTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.local_ip.family == AF_INET6 &&
             tunnel_info.remote_ip.family == AF_INET6) {
    PrepareV6TunnelTermTableEntry(table_entry, tunnel_info, p4info,
                                  insert_entry);
  }
}

#endif  // ES2K_TARGET

// called-by: DoConfigTunnelEntry (common)
absl::Status ConfigTunnelTermTableEntry(ClientInterface& client,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

#if defined(DPDK_TARGET)
  PrepareTunnelTermTableEntry(table_entry, tunnel_info, p4info, insert_entry);
#elif defined(ES2K_TARGET)
  Es2kPrepareTunnelTermTableEntry(table_entry, tunnel_info, p4info,
                                  insert_entry);
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  return client.sendWriteRequest(write_request);
}

#if defined(ES2K_TARGET)

// called-by: DoConfigIpMacMapEntry (es2k)
absl::Status ConfigDstIpMacMapTableEntry(ClientInterface& client,
                                         const struct ip_mac_map_info& ip_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareDstIpMacMapTableEntry(table_entry, ip_info, p4info, insert_entry,
                               detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailure(insert_entry, detail.getLogTableName());
  }
  return status;
}

// called-by: DoConfigIpMacMapEntry (es2k)
absl::Status ConfigSrcIpMacMapTableEntry(ClientInterface& client,
                                         const struct ip_mac_map_info& ip_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareSrcIpMacMapTableEntry(table_entry, ip_info, p4info, insert_entry,
                               detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailure(insert_entry, detail.getLogTableName());
  }
  return status;
}

// Called when deleting an FDB entry because we don't know if it's
// a Tunnel learn entry or a regular VSI learn entry. Find out and
// update learn_info accordingly.
//
// called-by: DoConfigFdbEntry (es2k)
void ConfigFdbUpdateTunnelInfo(ClientInterface& client,
                               struct mac_learning_info& learn_info,
                               const ::p4::config::v1::P4Info& p4info) {
  // Matching entry in IPv4 tunnel table?
  auto status_or_read_response =
      GetL2ToTunnelV4TableEntry(client, learn_info, p4info);
  if (status_or_read_response.ok()) {
    // Yes, we're deleting an IPv4 tunnel.
    learn_info.is_tunnel = true;
  }

  if (!learn_info.is_tunnel) {
    // Matching entry in IPv6 tunnel table?
    status_or_read_response =
        GetL2ToTunnelV6TableEntry(client, learn_info, p4info);
    if (status_or_read_response.ok()) {
      // We're deleting an IPv6 tunnel.
      learn_info.is_tunnel = true;
      learn_info.tnl_info.local_ip.family = AF_INET6;
      learn_info.tnl_info.remote_ip.family = AF_INET6;
    }
  }
}

// called-by: DoConfigFdbEntry (es2k)
absl::Status ConfigFdbUpdateSrcPort(ClientInterface& client,
                                    struct mac_learning_info& learn_info,
                                    const ::p4::config::v1::P4Info& p4info) {
  auto response_or_status =
      GetTxAccVsiTableEntry(client, learn_info.src_port, p4info);
  if (!response_or_status.ok()) {
    return response_or_status.status();
  }

  ::p4::v1::ReadResponse read_response = std::move(response_or_status).value();

  int param_id =
      GetParamId(p4info, TX_ACC_VSI_TABLE_ACTION_L2_FWD_AND_BYPASS_BRIDGE,
                 ACTION_L2_FWD_AND_BYPASS_BRIDGE_PARAM_PORT);

  uint32_t host_sp = 0;
  for (const auto& entity : read_response.entities()) {
    p4::v1::TableEntry table_entry_1 = entity.table_entry();
    auto* table_action = table_entry_1.mutable_action();
    auto* action = table_action->mutable_action();
    for (const auto& param : action->params()) {
      if (param_id == param.param_id()) {
        const std::string& s1 = param.value();
        std::string s2 = s1;
        for (int param_bytes = 0; param_bytes < 4; param_bytes++) {
          host_sp = host_sp << 8 | int(s2[param_bytes]);
        }
        break;
      }
    }
  }

  learn_info.src_port = host_sp;
  return absl::OkStatus();
}

// called-by: DoConfigFdbEntry (es2k) (not testable)
absl::Status ConfigFdbTunnelEntry(ClientInterface& client,
                                  const struct mac_learning_info& learn_info,
                                  bool insert_entry,
                                  const ::p4::config::v1::P4Info& p4info) {
  if (insert_entry) {
    auto status_or_read_response =
        GetFdbTunnelTableEntry(client, learn_info, p4info, true);
    if (status_or_read_response.ok()) {
      // Return if entry already exists.
      return absl::OkStatus();
    }
  }

  // Ignores status (why?)
  (void)ConfigFdbTunnelTableEntry(client, learn_info, p4info, insert_entry);

  // Ignores status (why?)
  (void)ConfigL2TunnelTableEntry(client, learn_info, p4info, insert_entry);

  // Ignores status (why?)
  (void)ConfigFdbSmacTableEntry(client, learn_info, p4info, insert_entry);

  return absl::OkStatus();
}

// called-by: DoConfigFdbEntry (es2k) (not testable)
absl::Status ConfigFdbVlanEntry(ClientInterface& client,
                                struct mac_learning_info& learn_info,
                                bool insert_entry,
                                const ::p4::config::v1::P4Info& p4info) {
  absl::Status status;

  if (insert_entry) {
    auto status_or_read_response =
        GetFdbVlanTableEntry(client, learn_info, p4info, true);
    if (status_or_read_response.ok()) {
      // Return if entry already exists.
      return absl::OkStatus();
    }

    // Ignores status (why?)
    (void)ConfigFdbRxVlanTableEntry(client, learn_info, p4info, insert_entry);

    status = ConfigFdbUpdateSrcPort(client, learn_info, p4info);
    if (!status.ok()) return status;
  }

  // Ignores status (why?)
  (void)ConfigFdbTxVlanTableEntry(client, learn_info, p4info, insert_entry);

  // Ignores status (why?)
  (void)ConfigFdbSmacTableEntry(client, learn_info, p4info, insert_entry);

  return absl::OkStatus();
}

//----------------------------------------------------------------------
// C++ functions that implement the public API.
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// DoConfigFdbEntry (ES2K)
//
// learn_info is passed by value because this function may make local
// modifications to it.
//----------------------------------------------------------------------

// extracted from DoConfigFdbEntry (not testable)
absl::Status ConfigFdbEntry(ClientInterface& client,
                            struct mac_learning_info learn_info,
                            const ::p4::config::v1::P4Info& p4info,
                            bool insert_entry) {
  if (!insert_entry) {
    // updates learn_info
    ConfigFdbUpdateTunnelInfo(client, learn_info, p4info);
  }

  if (learn_info.is_tunnel) {
    return ConfigFdbTunnelEntry(client, learn_info, insert_entry, p4info);
  } else {
    return ConfigFdbVlanEntry(client, learn_info, insert_entry, p4info);
  }
}

absl::Status DoConfigFdbEntry(ClientInterface& client,
                              struct mac_learning_info learn_info,
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
  return ConfigFdbEntry(client, learn_info, p4info, insert_entry);
}

//----------------------------------------------------------------------
// DoConfigRxTunnelSrcEntry (ES2K)
//----------------------------------------------------------------------

absl::Status DoConfigRxTunnelSrcEntry(ClientInterface& client,
                                      const struct tunnel_info& tunnel_info,
                                      bool insert_entry,
                                      const char* grpc_addr) {
  absl::Status status;

  // Start a new client session.
  status = client.connect(grpc_addr);
  if (!status.ok()) return status;

  // Fetch P4Info object from server.
  ::p4::config::v1::P4Info p4info;
  status = client.getPipelineConfig(&p4info);
  if (!status.ok()) return status;

  // Update P4 tables.
  return ConfigRxTunnelSrcPortTableEntry(client, tunnel_info, p4info,
                                         insert_entry);
}

//----------------------------------------------------------------------
// DoConfigTunnelSrcPortEntry (ES2K)
//----------------------------------------------------------------------

// extracted from DoConfigTunnelSrcPortEntry (testable)
absl::Status ConfigTunnelSrcPortEntry(ClientInterface& client,
                                      const struct src_port_info& tnl_sp,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareSrcPortTableEntry(table_entry, tnl_sp, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

absl::Status DoConfigTunnelSrcPortEntry(ClientInterface& client,
                                        const struct src_port_info& tnl_sp,
                                        bool insert_entry,
                                        const char* grpc_addr) {
  absl::Status status;

  // Start a new client session.
  status = client.connect(grpc_addr);
  if (!status.ok()) return status;

  // Fetch P4Info object from server.
  ::p4::config::v1::P4Info p4info;
  status = client.getPipelineConfig(&p4info);
  if (!status.ok()) return status;

  // Update P4 tables.
  return ConfigTunnelSrcPortEntry(client, tnl_sp, p4info, insert_entry);
}

//----------------------------------------------------------------------
// DoConfigSrcPortEntry (ES2K)
//
// vsi_sp is passed by value because this function makes local
// modifications to it.
//----------------------------------------------------------------------

// extracted from DoConfigSrcPortEntry (testable)
absl::Status ConfigSrcPortEntry(ClientInterface& client,
                                struct src_port_info vsi_sp,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  // TODO(derek): refactor (extract method)
  auto response_or_status =
      GetTxAccVsiTableEntry(client, vsi_sp.src_port, p4info);
  if (!response_or_status.ok()) return response_or_status.status();

  ::p4::v1::ReadResponse read_response = std::move(response_or_status).value();
  std::vector<::p4::v1::TableEntry> table_entries;

  table_entries.reserve(read_response.entities().size());

  int param_id =
      GetParamId(p4info, TX_ACC_VSI_TABLE_ACTION_L2_FWD_AND_BYPASS_BRIDGE,
                 ACTION_L2_FWD_AND_BYPASS_BRIDGE_PARAM_PORT);

  uint32_t host_sp = 0;
  for (const auto& entity : read_response.entities()) {
    p4::v1::TableEntry table_entry_1 = entity.table_entry();
    auto* table_action = table_entry_1.mutable_action();
    auto* action = table_action->mutable_action();
    for (const auto& param : action->params()) {
      if (param_id == param.param_id()) {
        const std::string& s1 = param.value();
        std::string s2 = s1;
        for (int param_bytes = 0; param_bytes < 4; param_bytes++) {
          host_sp = host_sp << 8 | int(s2[param_bytes]);
        }
        break;
      }
    }
  }

  vsi_sp.src_port = host_sp;
  // end of refactoring

  return ConfigVsiSrcPortTableEntry(client, vsi_sp, p4info, insert_entry);
}

absl::Status DoConfigSrcPortEntry(ClientInterface& client,
                                  struct src_port_info vsi_sp,
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
  return ConfigSrcPortEntry(client, vsi_sp, p4info, insert_entry);
}

//----------------------------------------------------------------------
// DoConfigVlanEntry (ES2K)
//----------------------------------------------------------------------

// extracted from DoConfigVlanEntry (not testable)
absl::Status ConfigVlanEntry(ClientInterface& client, uint16_t vlan_id,
                             const ::p4::config::v1::P4Info& p4info,
                             bool insert_entry) {
  absl::Status status;

  status = ConfigVlanPushTableEntry(client, vlan_id, p4info, insert_entry);
  if (!status.ok()) return status;

  return ConfigVlanPopTableEntry(client, vlan_id, p4info, insert_entry);
}

absl::Status DoConfigVlanEntry(ClientInterface& client, uint16_t vlan_id,
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
  return ConfigVlanEntry(client, vlan_id, p4info, insert_entry);
}

#elif defined(DPDK_TARGET)

//----------------------------------------------------------------------
// DoConfigFdbEntry (DPDK)
//----------------------------------------------------------------------

// extracted from DoConfigFdbEntry (not testable)
absl::Status ConfigFdbEntry(ClientInterface& client,
                            const struct mac_learning_info& learn_info,
                            const ::p4::config::v1::P4Info& p4info,
                            bool insert_entry) {
  if (learn_info.is_tunnel) {
    return ConfigFdbTunnelTableEntry(client, learn_info, p4info, insert_entry);
  } else if (learn_info.is_vlan) {
    auto status =
        ConfigFdbTxVlanTableEntry(client, learn_info, p4info, insert_entry);
    if (!status.ok()) return status;

    return ConfigFdbRxVlanTableEntry(client, learn_info, p4info, insert_entry);
  } else {
    // TODO(Derek): return error status?
    return absl::OkStatus();
  }
}

absl::Status DoConfigFdbEntry(ClientInterface& client,
                              struct mac_learning_info learn_info,
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
  return ConfigFdbEntry(client, learn_info, p4info, insert_entry);
}

#endif  // DPDK_TARGET

//----------------------------------------------------------------------
// DoConfigTunnelEntry (common)
//----------------------------------------------------------------------

// extracted from DoConfigTunnelEntry (not testable)
absl::Status ConfigTunnelEntry(ClientInterface& client,
                               const struct tunnel_info& tunnel_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry) {
  absl::Status status;

  status = ConfigEncapTableEntry(client, tunnel_info, p4info, insert_entry);
  if (!status.ok()) return status;

#if defined(ES2K_TARGET)
  status = ConfigDecapTableEntry(client, tunnel_info, p4info, insert_entry);
  if (!status.ok()) return status;
#endif

  return ConfigTunnelTermTableEntry(client, tunnel_info, p4info, insert_entry);
}

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

#if defined(ES2K_TARGET)

//----------------------------------------------------------------------
// DoConfigIpMacMapEntry (ES2K)
//----------------------------------------------------------------------

// extracted from DoConfigIpMacMapEntry (not testable)
absl::Status ConfigIpMacMapEntry(ClientInterface& client,
                                 const struct ip_mac_map_info& ip_info,
                                 const ::p4::config::v1::P4Info& p4info,
                                 bool insert_entry) {
  if (insert_entry) {
    auto status_or_read_response = GetVmSrcTableEntry(client, ip_info, p4info);
    if (status_or_read_response.ok()) {
      goto try_dstip;
    }
  }

  if (ValidIpAddr(ip_info.src_ip_addr.ip.v4addr.s_addr)) {
    // Ignores errors (why?)
    (void)ConfigSrcIpMacMapTableEntry(client, ip_info, p4info, insert_entry);
  }

try_dstip:
  if (insert_entry) {
    auto status_or_read_response = GetVmDstTableEntry(client, ip_info, p4info);
    if (status_or_read_response.ok()) {
      return status_or_read_response.status();
    }
  }

  if (ValidIpAddr(ip_info.src_ip_addr.ip.v4addr.s_addr)) {
    // Ignores errors (why?)
    (void)ConfigDstIpMacMapTableEntry(client, ip_info, p4info, insert_entry);
  }
  return absl::OkStatus();
}

absl::Status DoConfigIpMacMapEntry(ClientInterface& client,
                                   const struct ip_mac_map_info& ip_info,
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
  return ConfigIpMacMapEntry(client, ip_info, p4info, insert_entry);
}

#endif  // ES2K_TARGET

}  // namespace ovsp4rt
