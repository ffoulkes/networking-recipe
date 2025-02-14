// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_encode_actions.h"

#include <google/protobuf/util/json_util.h>

#include <nlohmann/json.hpp>

#include "absl/status/statusor.h"
#include "p4/v1/p4runtime.pb.h"

namespace {
constexpr uint32_t READ_REQUEST_VERSION = 0;
constexpr uint32_t READ_RESPONSE_VERSION = 0;
constexpr uint32_t WRITE_REQUEST_VERSION = 0;
constexpr uint32_t WRITE_STATUS_VERSION = 0;
}  // namespace

namespace ovsp4rt {

// These functions are only used by this module.
// They can be exposed if the need arises.

namespace {

nlohmann::json EncodeStatus(const absl::Status& status) {
  return {{"code", static_cast<int>(status.code())},
          {"message",
           // Convert absl::string_view to std::string
           std::string(status.message())}};
}

nlohmann::json EncodeProtobuf(const google::protobuf::Message& message) {
  std::string jsonString;
  auto status =
      google::protobuf::util::MessageToJsonString(message, &jsonString);
  if (status.ok()) {
    return nlohmann::json::parse(jsonString);
  } else {
    // There may be a better way to report the conversion error.
    // Try this for now.
    return {{"EncodeProtobuf", EncodeStatus(status)}};
  }
}

}  // namespace

nlohmann::json EncodeReadRequest(const ::p4::v1::ReadRequest& request) {
  nlohmann::json json;

  json["action"] = "ReadRequest";
  json["version"] = READ_REQUEST_VERSION;
  json["message"] = EncodeProtobuf(request);

  return json;
}

nlohmann::json EncodeReadResponse(
    const absl::StatusOr<::p4::v1::ReadResponse>& response) {
  nlohmann::json json;

  json["action"] = "ReadResponse";
  json["version"] = READ_RESPONSE_VERSION;

  if (response.ok()) {
    json["message"] = EncodeProtobuf(response.value());
  } else {
    json["status"] = EncodeStatus(response.status());
  }

  return json;
}

nlohmann::json EncodeWriteRequest(const ::p4::v1::WriteRequest& request) {
  nlohmann::json json;

  json["action"] = "WriteRequest";
  json["version"] = WRITE_REQUEST_VERSION;
  json["message"] = EncodeProtobuf(request);

  return json;
}

nlohmann::json EncodeWriteStatus(const absl::Status& status) {
  nlohmann::json json;

  json["action"] = "WriteStatus";
  json["version"] = WRITE_STATUS_VERSION;
  json["status"] = EncodeStatus(status);

  return json;
}

}  // namespace ovsp4rt
