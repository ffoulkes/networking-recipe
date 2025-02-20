// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_test_client.h"

#include "absl/flags/flag.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ovsp4rt_client.h"
#include "ovsp4rt_client_flags.h"

namespace ovsp4rt {

TestClient::TestClient()
    : device_id_(absl::GetFlag(FLAGS_device_id)),
      role_name_(absl::GetFlag(FLAGS_role_name)) {}

absl::Status TestClient::connect(const char* grpc_addr) {
  return absl::OkStatus();
}

absl::Status TestClient::getPipelineConfig(::p4::config::v1::P4Info* p4info) {
  return absl::UnimplementedError("getPipelineConfig");
}

::p4::v1::TableEntry* TestClient::initReadRequest(
    ::p4::v1::ReadRequest* request) {
  request->set_device_id(device_id_);
  auto* entity = request->add_entities();
  return entity->mutable_table_entry();
}

absl::StatusOr<::p4::v1::ReadResponse> TestClient::DoSendReadRequest(
    const p4::v1::ReadRequest& request) {
  return absl::UnimplementedError("DoSendReadRequest");
}

absl::Status TestClient::DoSendWriteRequest(
    const p4::v1::WriteRequest& request) {
  return absl::UnimplementedError("DoSendWriteRequest");
}

::p4::v1::TableEntry* TestClient::InitWriteRequest(
    ::p4::v1::WriteRequest* request, ::p4::v1::Update_Type type_value) {
  request->set_device_id(device_id_);
  *request->mutable_election_id() = election_id_;
  auto* update = request->add_updates();
  update->set_type(type_value);
  return update->mutable_entity()->mutable_table_entry();
}

}  // namespace ovsp4rt
