// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Implements DoConfigIpMacMapEntry() (ES2K)

#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <arpa/inet.h>

#include "client/ovsp4rt_client_interface.h"
#include "core/api/ovsp4rt_internal_api.h"
#include "core/common/ovsp4rt_encoders.h"
#include "core/common/ovsp4rt_entry_utils.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "logging/ovsp4rt_logutils.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace ovsp4rt {

static inline int32_t ValidIpAddr(uint32_t nw_addr) {
  return (nw_addr && nw_addr != INADDR_ANY && nw_addr != INADDR_LOOPBACK &&
          nw_addr != 0xffffffff);
}

absl::StatusOr<::p4::v1::ReadResponse> ReadVmSrcTableEntry(
    ClientInterface& client, struct ip_mac_map_info ip_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  EncodeSrcIpMacMapTableEntry(table_entry, ip_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> ReadVmDstTableEntry(
    ClientInterface& client, const struct ip_mac_map_info& ip_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  EncodeDstIpMacMapTableEntry(table_entry, ip_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

absl::Status WriteDstIpMacMapTableEntry(ClientInterface& client,
                                        const struct ip_mac_map_info& ip_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  EncodeDstIpMacMapTableEntry(table_entry, ip_info, p4info, insert_entry,
                              detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailure(insert_entry, detail.getLogTableName());
  }
  return status;
}

absl::Status WriteSrcIpMacMapTableEntry(ClientInterface& client,
                                        const struct ip_mac_map_info& ip_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  EncodeSrcIpMacMapTableEntry(table_entry, ip_info, p4info, insert_entry,
                              detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailure(insert_entry, detail.getLogTableName());
  }
  return status;
}

absl::Status ConfigIpMacMapEntry(ClientInterface& client,
                                 const struct ip_mac_map_info& ip_info,
                                 const ::p4::config::v1::P4Info& p4info,
                                 bool insert_entry) {
  if (insert_entry) {
    auto status_or_read_response = ReadVmSrcTableEntry(client, ip_info, p4info);
    if (status_or_read_response.ok()) {
      goto try_dstip;
    }
  }

  if (ValidIpAddr(ip_info.src_ip_addr.ip.v4addr.s_addr)) {
    // Ignores errors (why?)
    (void)WriteSrcIpMacMapTableEntry(client, ip_info, p4info, insert_entry);
  }

try_dstip:
  if (insert_entry) {
    auto status_or_read_response = ReadVmDstTableEntry(client, ip_info, p4info);
    if (status_or_read_response.ok()) {
      return status_or_read_response.status();
    }
  }

  if (ValidIpAddr(ip_info.src_ip_addr.ip.v4addr.s_addr)) {
    // Ignores errors (why?)
    (void)WriteDstIpMacMapTableEntry(client, ip_info, p4info, insert_entry);
  }
  return absl::OkStatus();
}

//----------------------------------------------------------------------
// DoConfigIpMacMapEntry
//----------------------------------------------------------------------

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

}  // namespace ovsp4rt
