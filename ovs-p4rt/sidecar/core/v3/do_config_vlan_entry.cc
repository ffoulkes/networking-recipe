// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Implements DoConfigVlanEntry() (ES2K)

#include "client/ovsp4rt_client_interface.h"
#include "core/api/ovsp4rt_internal_api.h"
#include "core/common/ovsp4rt_encoders.h"
#include "core/common/ovsp4rt_entry_utils.h"
#include "logging/ovsp4rt_logutils.h"

namespace ovsp4rt {

absl::Status WriteVlanPushTableEntry(ClientInterface& client,
                                     const uint16_t vlan_id,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  EncodeVlanPushTableEntry(table_entry, vlan_id, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

absl::Status WriteVlanPopTableEntry(ClientInterface& client,
                                    const uint16_t vlan_id,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  EncodeVlanPopTableEntry(table_entry, vlan_id, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

absl::Status ConfigVlanEntry(ClientInterface& client, uint16_t vlan_id,
                             const ::p4::config::v1::P4Info& p4info,
                             bool insert_entry) {
  absl::Status status;

  status = WriteVlanPushTableEntry(client, vlan_id, p4info, insert_entry);
  if (!status.ok()) return status;

  return WriteVlanPopTableEntry(client, vlan_id, p4info, insert_entry);
}

//----------------------------------------------------------------------
// DoConfigVlanEntry
//----------------------------------------------------------------------

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

}  // namespace ovsp4rt
