// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef TX_ACC_VSI_TABLE_ENTRY_H_
#define TX_ACC_VSI_TABLE_ENTRY_H_

#include <absl/status/statusor.h>

#include "client/ovsp4rt_client_interface.h"
#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

extern absl::StatusOr<uint32_t> GetTxAccVsiPort(
    ClientInterface& client, const ::p4::config::v1::P4Info& p4info,
    uint32_t src_port);

extern absl::StatusOr<::p4::v1::ReadResponse> ReadTxAccVsiTableEntry(
    ClientInterface& client, uint32_t sp,
    const ::p4::config::v1::P4Info& p4info);

}  // namespace ovsp4rt

#endif  // TX_ACC_VSI_TABLE_ENTRY_H_
