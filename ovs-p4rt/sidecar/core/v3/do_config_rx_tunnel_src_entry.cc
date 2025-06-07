// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Implements DoConfigRxTunnelSrcPortEntry()

#if !defined(ES2K_TARGET)
#error "ASSERT: Target type must be ES2K_TARGET"
#endif

#include <arpa/inet.h>

#include "client/ovsp4rt_client_interface.h"
#include "core/api/ovsp4rt_internal_api.h"
#include "core/common/ovsp4rt_encoders.h"
#include "core/common/ovsp4rt_entry_utils.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace ovsp4rt {

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

//----------------------------------------------------------------------
// DoConfigRxTunnelSrcEntry
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

}  // namespace ovsp4rt
