// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Implements DoConfigFdbEntry for DPDK.

#include "client/ovsp4rt_client_interface.h"
#include "core/api/ovsp4rt_internal_api.h"
#include "core/common/ovsp4rt_encoders.h"
#include "core/common/ovsp4rt_entry_utils.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "logging/ovsp4rt_logutils.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace ovsp4rt {

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

absl::Status WriteFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  EncodeFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                      insert_entry, detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

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
