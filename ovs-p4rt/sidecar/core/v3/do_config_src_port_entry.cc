// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Implements DoConfigSrcPortEntry() (ES2K)

#if !defined(ES2K_TARGET)
#error "ASSERT: Target type must be ES2K_TARGET"
#endif

#include "client/ovsp4rt_client_interface.h"
#include "core/api/ovsp4rt_internal_api.h"
#include "core/common/ovsp4rt_encoders.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "tx_acc_vsi_table_entry.h"

namespace ovsp4rt {

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

absl::Status ConfigSrcPortEntry(ClientInterface& client,
                                struct src_port_info port_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  auto host_sp = GetTxAccVsiPort(client, p4info, port_info.src_port);
  if (!host_sp.ok()) return host_sp.status();
  port_info.src_port = host_sp.value();

  return WriteVsiSrcPortTableEntry(client, port_info, p4info, insert_entry);
}

//----------------------------------------------------------------------
// DoConfigSrcPortEntry
//
// vsi_sp is passed by value because this function makes local
// modifications to it.
//----------------------------------------------------------------------

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

}  // namespace ovsp4rt
