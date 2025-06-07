// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include <arpa/inet.h>

#include <string>

#include "client/ovsp4rt_client_interface.h"
#include "core/api/ovsp4rt_internal_api.h"
#include "core/common/ovsp4rt_core_utils.h"
#include "core/common/ovsp4rt_encoders.h"
#include "core/common/ovsp4rt_entry_utils.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "logging/ovsp4rt_logging.h"
#include "logging/ovsp4rt_logutils.h"
#include "ovsp4rt/ovs-p4rt.h"

#if defined(DPDK_TARGET)
#include "core/dpdk/p4_name_mapping.h"
#elif defined(ES2K_TARGET)
#include "core/es2k/p4_name_mapping.h"
#endif

namespace ovsp4rt {

#ifdef ES2K_TARGET

absl::Status WriteFdbSmacTableEntry(ClientInterface& client,
                                    const struct mac_learning_info& learn_info,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  EncodeFdbSmacTableEntry(table_entry, learn_info, p4info, insert_entry,
                          detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

// extracted from WriteL2TunnelTableEntry
void PrepareL2TunnelTableEntry(p4::v1::TableEntry* table_entry,
                               const struct mac_learning_info& learn_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry, DiagDetail& detail) {
  if (learn_info.tnl_info.local_ip.family == AF_INET6 &&
      learn_info.tnl_info.remote_ip.family == AF_INET6) {
    EncodeL2ToTunnelV6(table_entry, learn_info, p4info, insert_entry, detail);
  } else {
    EncodeL2ToTunnelV4(table_entry, learn_info, p4info, insert_entry, detail);
  }
}

absl::Status WriteL2TunnelTableEntry(ClientInterface& client,
                                     const struct mac_learning_info& learn_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
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
absl::Status WriteFdbTxVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  EncodeFdbTxVlanTableEntry(table_entry, learn_info, p4info, insert_entry,
                            detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

// called-by: ConfigFdbVlanEntry (dpdk, es2k)
absl::Status WriteFdbRxVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  EncodeFdbRxVlanTableEntry(table_entry, learn_info, p4info, insert_entry,
                            detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

#if defined(ES2K_TARGET)
// extracted from WriteFdbTunnelTableEntry
void Es2kPrepareFdbTunnelTableEntry(p4::v1::TableEntry* table_entry,
                                    const struct mac_learning_info& learn_info,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry, DiagDetail& detail) {
  if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    EncodeFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                        insert_entry, detail);
  } else if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    EncodeFdbTableEntryforV4GeneveTunnel(table_entry, learn_info, p4info,
                                         insert_entry, detail);
  } else if (!insert_entry) {
    // Just specify the prologue when deleting an entry.
    EncodeL2FwdTxTablePrologue(table_entry, learn_info, p4info);
  }
}
#endif  // ES2K_TARGET

absl::Status WriteFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

#if defined(DPDK_TARGET)
  EncodeFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
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

#if defined(ES2K_TARGET)

absl::StatusOr<::p4::v1::ReadResponse> ReadL2ToTunnelV4TableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  EncodeL2ToTunnelV4(table_entry, learn_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> ReadL2ToTunnelV6TableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  EncodeL2ToTunnelV6(table_entry, learn_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

// extracted from ReadFdbTunnelTableEntry
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
    EncodeFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                        testing, detail);
  } else if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    EncodeFdbTableEntryforV4GeneveTunnel(table_entry, learn_info, p4info,
                                         testing, detail);
  }
}

// called-by: ConfigFdbTunnelEntry
absl::StatusOr<::p4::v1::ReadResponse> ReadFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool adding) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

#if defined(DPDK_TARGET)
  EncodeFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info, false,
                                      detail);
#elif defined(ES2K_TARGET)
  PrepareFdbTableV4TunnelEntry(table_entry, learn_info, p4info, false, detail);
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  return client.sendReadRequest(read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> ReadFdbVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool adding) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  EncodeFdbTxVlanTableEntry(table_entry, learn_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

// called-by: DoConfigSrcPortEntry, UpdateFdbSrcPortInfo
absl::StatusOr<::p4::v1::ReadResponse> ReadTxAccVsiTableEntry(
    ClientInterface& client, uint32_t sp,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initReadRequest(&read_request);

  EncodeTxAccVsiTableEntry(table_entry, sp, p4info);

  return client.sendReadRequest(read_request);
}

absl::StatusOr<uint32_t> GetTxAccVsiPort(ClientInterface& client,
                                         const ::p4::config::v1::P4Info& p4info,
                                         uint32_t src_port) {
  auto response_or_status = ReadTxAccVsiTableEntry(client, src_port, p4info);
  auto status = response_or_status.status();
  if (!status.ok()) return status;

  ::p4::v1::ReadResponse read_response = std::move(response_or_status).value();

  int param_id =
      GetParamId(p4info, TX_ACC_VSI_TABLE_ACTION_L2_FWD_AND_BYPASS_BRIDGE,
                 ACTION_L2_FWD_AND_BYPASS_BRIDGE_PARAM_PORT);

  for (const auto& entity : read_response.entities()) {
    const p4::v1::TableEntry table_entry = entity.table_entry();
    const auto& table_action = table_entry.action();
    const auto& action = table_action.action();
    for (const auto& param : action.params()) {
      if (param.param_id() == param_id) {
        const std::string val = param.value();
        uint32_t host_sp = 0;
        for (int i = 0; i < val.size(); i++) {
          host_sp = ((host_sp << 8) | (val[i] & 0xff));
        }
        return host_sp;
      }
    }
  }
  return absl::InternalError("Missing port parameter");
}

// called-by: DoConfigSrcPortEntry (es2k)
absl::Status WriteVsiSrcPortTableEntry(ClientInterface& client,
                                       const struct src_port_info& port_info,
                                       const ::p4::config::v1::P4Info& p4info,
                                       bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  EncodeSrcPortTableEntry(table_entry, port_info, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

// extracted from WriteRxTunnelSrcPortTableEntry
void PrepareRxTunnelSrcPortTableEntry(p4::v1::TableEntry* table_entry,
                                      const struct tunnel_info& tunnel_info,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry) {
  if (tunnel_info.local_ip.family == AF_INET &&
      tunnel_info.remote_ip.family == AF_INET) {
    EncodeRxTunnelTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.local_ip.family == AF_INET6 &&
             tunnel_info.remote_ip.family == AF_INET6) {
    EncodeV6RxTunnelTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  }
}

// called-by: DoConfigRxTunnelSrcEntry (es2k)
absl::Status WriteRxTunnelSrcPortTableEntry(
    ClientInterface& client, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareRxTunnelSrcPortTableEntry(table_entry, tunnel_info, p4info,
                                   insert_entry);

  return client.sendWriteRequest(write_request);
}

#endif  // ES2K_TARGET

#if defined(ES2K_TARGET)

// Called when deleting an FDB entry because we don't know if it's
// a Tunnel learn entry or a regular VSI learn entry. Find out and
// update learn_info accordingly.
//
// called-by: DoConfigFdbEntry (es2k)
void UpdateFdbTunnelInfo(ClientInterface& client,
                         struct mac_learning_info& learn_info,
                         const ::p4::config::v1::P4Info& p4info) {
  // Matching entry in IPv4 tunnel table?
  auto status_or_read_response =
      ReadL2ToTunnelV4TableEntry(client, learn_info, p4info);
  if (status_or_read_response.ok()) {
    // Yes, we're deleting an IPv4 tunnel.
    learn_info.is_tunnel = true;
  }

  if (!learn_info.is_tunnel) {
    // Matching entry in IPv6 tunnel table?
    status_or_read_response =
        ReadL2ToTunnelV6TableEntry(client, learn_info, p4info);
    if (status_or_read_response.ok()) {
      // We're deleting an IPv6 tunnel.
      learn_info.is_tunnel = true;
      learn_info.tnl_info.local_ip.family = AF_INET6;
      learn_info.tnl_info.remote_ip.family = AF_INET6;
    }
  }
}

// called-by: DoConfigFdbEntry (es2k)
absl::Status UpdateFdbSrcPortInfo(ClientInterface& client,
                                  struct mac_learning_info& learn_info,
                                  const ::p4::config::v1::P4Info& p4info) {
  auto host_sp = GetTxAccVsiPort(client, p4info, learn_info.src_port);
  if (!host_sp.ok()) return host_sp.status();
  learn_info.src_port = host_sp.value();
  return absl::OkStatus();
}

// called-by: DoConfigFdbEntry (es2k) (not testable)
absl::Status ConfigFdbTunnelEntry(ClientInterface& client,
                                  const struct mac_learning_info& learn_info,
                                  bool insert_entry,
                                  const ::p4::config::v1::P4Info& p4info) {
  if (insert_entry) {
    auto status_or_read_response =
        ReadFdbTunnelTableEntry(client, learn_info, p4info, true);
    if (status_or_read_response.ok()) {
      // Return if entry already exists.
      return absl::OkStatus();
    }
  }

  // Ignores status (why?)
  (void)WriteFdbTunnelTableEntry(client, learn_info, p4info, insert_entry);

  // Ignores status (why?)
  (void)WriteL2TunnelTableEntry(client, learn_info, p4info, insert_entry);

  // Ignores status (why?)
  (void)WriteFdbSmacTableEntry(client, learn_info, p4info, insert_entry);

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
        ReadFdbVlanTableEntry(client, learn_info, p4info, true);
    if (status_or_read_response.ok()) {
      // Return if entry already exists.
      return absl::OkStatus();
    }

    // Ignores status (why?)
    (void)WriteFdbRxVlanTableEntry(client, learn_info, p4info, insert_entry);

    status = UpdateFdbSrcPortInfo(client, learn_info, p4info);
    if (!status.ok()) return status;
  }

  // Ignores status (why?)
  (void)WriteFdbTxVlanTableEntry(client, learn_info, p4info, insert_entry);

  // Ignores status (why?)
  (void)WriteFdbSmacTableEntry(client, learn_info, p4info, insert_entry);

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
    UpdateFdbTunnelInfo(client, learn_info, p4info);
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
  return WriteRxTunnelSrcPortTableEntry(client, tunnel_info, p4info,
                                        insert_entry);
}

//----------------------------------------------------------------------
// DoConfigTunnelSrcPortEntry (ES2K)
//----------------------------------------------------------------------

// extracted from DoConfigTunnelSrcPortEntry (testable)
absl::Status WriteTunnelSrcPortEntry(ClientInterface& client,
                                     const struct src_port_info& port_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  EncodeSrcPortTableEntry(table_entry, port_info, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

absl::Status DoConfigTunnelSrcPortEntry(ClientInterface& client,
                                        const struct src_port_info& port_info,
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
  return WriteTunnelSrcPortEntry(client, port_info, p4info, insert_entry);
}

//----------------------------------------------------------------------
// DoConfigSrcPortEntry (ES2K)
//
// vsi_sp is passed by value because this function makes local
// modifications to it.
//----------------------------------------------------------------------

// extracted from DoConfigSrcPortEntry (testable)
absl::Status ConfigSrcPortEntry(ClientInterface& client,
                                struct src_port_info port_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  auto host_sp = GetTxAccVsiPort(client, p4info, port_info.src_port);
  if (!host_sp.ok()) return host_sp.status();
  port_info.src_port = host_sp.value();

  return WriteVsiSrcPortTableEntry(client, port_info, p4info, insert_entry);
}

absl::Status DoConfigSrcPortEntry(ClientInterface& client,
                                  struct src_port_info port_info,
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
  return ConfigSrcPortEntry(client, port_info, p4info, insert_entry);
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
    return WriteFdbTunnelTableEntry(client, learn_info, p4info, insert_entry);
  } else if (learn_info.is_vlan) {
    auto status =
        WriteFdbTxVlanTableEntry(client, learn_info, p4info, insert_entry);
    if (!status.ok()) return status;

    return WriteFdbRxVlanTableEntry(client, learn_info, p4info, insert_entry);
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

}  // namespace ovsp4rt
