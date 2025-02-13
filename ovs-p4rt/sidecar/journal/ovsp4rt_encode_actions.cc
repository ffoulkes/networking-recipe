// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_encode_actions.h"

#include <nlohmann/json.hpp>

#include "p4/v1/p4runtime.pb.h"

namespace {
constexpr uint32_t READ_REQUEST_VERSION = 0;
constexpr uint32_t READ_RESPONSE_VERSION = 0;
constexpr uint32_t WRITE_REQUEST_VERSION = 0;
constexpr uint32_t WRITE_STATUS_VERSION = 0;
}  // namespace

namespace ovsp4rt {

nlohmann::json EncodeReadRequest(const ::p4::v1::ReadRequest& request) {
  nlohmann::json json;

  json["action"] = "ReadRequest";
  json["version"] = READ_REQUEST_VERSION;

  auto& payload = json["request"];
  // TODO(derek): encode request

  return json;
}

nlohmann::json EncodeReadResponse(
    const absl::StatusOr<::p4::v1::ReadResponse>& response) {
  nlohmann::json json;

  json["action"] = "ReadResponse";
  json["version"] = READ_RESPONSE_VERSION;

  auto& payload = json["response"];
  // TODO(derek): encode response.status();
  // TODO(derek): encode response.value();

  return json;
}

nlohmann::json EncodeWriteRequest(const ::p4::v1::WriteRequest& request) {
  nlohmann::json json;

  json["action"] = "WriteRequest";
  json["version"] = WRITE_REQUEST_VERSION;

  auto& payload = json["request"];
  // TODO(derek): encode request

  return json;
}

nlohmann::json EncodeWriteStatus(const absl::Status& status) {
  nlohmann::json json;

  json["action"] = "WriteStatus";
  json["version"] = WRITE_STATUS_VERSION;

  auto& payload = json["status"];
  // TODO(derek): encode status

  return json;
}

}  // namespace ovsp4rt
