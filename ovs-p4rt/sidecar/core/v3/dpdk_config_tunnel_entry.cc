// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Implements DoConfigTunnelEntry() (DPDK)

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
void PrepareV4EncapTableEntry(p4::v1::TableEntry* table_entry,
                              const struct tunnel_info& tunnel_info,
                              const ::p4::config::v1::P4Info& p4info,
                              bool insert_entry) {
  EncodeVxlanEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
}

absl::Status WriteEncapTableEntry(ClientInterface& client,
                                  const struct tunnel_info& tunnel_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareV4EncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

//----------------------------------------------------------------------
// WriteDecapTableEntry
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// WriteTunnelTermTableEntry
//----------------------------------------------------------------------

absl::Status WriteTunnelTermTableEntry(ClientInterface& client,
                                       const struct tunnel_info& tunnel_info,
                                       const ::p4::config::v1::P4Info& p4info,
                                       bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  EncodeTunnelTermTableEntry(table_entry, tunnel_info, p4info, insert_entry);

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
