// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Implements DoConfigFdbEntry for ES2K.

#include <arpa/inet.h>

#include "client/ovsp4rt_client_interface.h"
#include "core/api/ovsp4rt_internal_api.h"
#include "core/common/ovsp4rt_encoders.h"
#include "core/common/ovsp4rt_entry_utils.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "logging/ovsp4rt_logutils.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "tx_acc_vsi_table_entry.h"

namespace ovsp4rt {

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

absl::Status WriteFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  Es2kPrepareFdbTunnelTableEntry(table_entry, learn_info, p4info, insert_entry,
                                 detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

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

absl::StatusOr<::p4::v1::ReadResponse> ReadFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool adding) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  PrepareFdbTableV4TunnelEntry(table_entry, learn_info, p4info, false, detail);

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

// Called when deleting an FDB entry because we don't know if it's
// a Tunnel learn entry or a regular VSI learn entry. Find out and
// update learn_info accordingly.
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

absl::Status UpdateFdbSrcPortInfo(ClientInterface& client,
                                  struct mac_learning_info& learn_info,
                                  const ::p4::config::v1::P4Info& p4info) {
  auto host_sp = GetTxAccVsiPort(client, p4info, learn_info.src_port);
  if (!host_sp.ok()) return host_sp.status();
  learn_info.src_port = host_sp.value();
  return absl::OkStatus();
}

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

// learn_info is passed by value because this function may make local
// modifications to it.
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

//----------------------------------------------------------------------
// DoConfigFdbEntry
//
// learn_info is passed by value because this api may make local
// modifications to it.
//----------------------------------------------------------------------
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

}  // namespace ovsp4rt
