// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_ENCODE_ACTIONS_H
#define OVSP4RT_ENCODE_ACTIONS_H

#include <nlohmann/json.hpp>

#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

//----------------------------------------------------------------------
// Return JSON representation of P4Runtime actions.
//----------------------------------------------------------------------

extern nlohmann::json EncodeReadRequest(const ::p4::v1::ReadRequest& request);

extern nlohmann::json EncodeReadResponse(
    const absl::StatusOr<::p4::v1::ReadResponse>& response);

extern nlohmann::json EncodeWriteRequest(const ::p4::v1::WriteRequest& request);

extern nlohmann::json EncodeWriteStatus(const absl::Status& status);

}  // namespace ovsp4rt

#endif  // OVSP4RT_ENCODE_ACTIONS_H
