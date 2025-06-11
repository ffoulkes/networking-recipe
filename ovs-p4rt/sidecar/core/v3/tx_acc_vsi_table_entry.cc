// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Implements ReadTxAccVsiTableEntry() and GetTxAccVsiPort().

#if !defined(ES2K_TARGET)
#error "ASSERT: Target type must be ES2K_TARGET"
#endif

#include "tx_acc_vsi_table_entry.h"

#include <absl/status/status.h>

#include "client/ovsp4rt_client_interface.h"
#include "core/common/ovsp4rt_encoders.h"
#include "core/common/ovsp4rt_p4info_utils.h"  // GetParamId
#include "core/es2k/p4_name_mapping.h"

namespace ovsp4rt {

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

}  // namespace ovsp4rt
