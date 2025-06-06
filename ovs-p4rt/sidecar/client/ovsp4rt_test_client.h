// Copyright 2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_TEST_CLIENT_H_
#define OVSP4RT_TEST_CLIENT_H_

#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <stdint.h>

#include "ovsp4rt_client_interface.h"
#include "p4/v1/p4runtime.pb.h"
#include "session/ovsp4rt_session.h"

namespace ovsp4rt {

class TestClient : public ClientInterface {
 public:
  TestClient();
  virtual ~TestClient() = default;

  // Connects to the P4Runtime server.
  virtual absl::Status connect(const char* grpc_addr) override;

  // Gets the pipeline configuration from the P4Runtime server.
  virtual absl::Status getPipelineConfig(
      ::p4::config::v1::P4Info* p4info) override;

  //--------------------------------------------------------------------

  // Initializes a Read Table Entry request message.
  virtual ::p4::v1::TableEntry* initReadRequest(
      ::p4::v1::ReadRequest* request) override;

  // Sends a Read Table Entry request to the P4Runtime server.
  virtual absl::StatusOr<p4::v1::ReadResponse> sendReadRequest(
      const p4::v1::ReadRequest& request) override {
    return DoSendReadRequest(request);
  }

  //--------------------------------------------------------------------

  // Initializes an Insert Table Entry request message.
  virtual ::p4::v1::TableEntry* initInsertRequest(
      ::p4::v1::WriteRequest* request) override {
    return InitWriteRequest(request, ::p4::v1::Update::INSERT);
  }

  // Initializes a Modify Table Entry request message.
  virtual ::p4::v1::TableEntry* initModifyRequest(
      ::p4::v1::WriteRequest* request) override {
    return InitWriteRequest(request, ::p4::v1::Update::MODIFY);
  }

  // Initializes a Delete Table Entry request message.
  virtual ::p4::v1::TableEntry* initDeleteRequest(
      ::p4::v1::WriteRequest* request) override {
    return InitWriteRequest(request, ::p4::v1::Update::DELETE);
  }

  // Initializes an Insert Table Entry or Delete Table Entry request
  // message, depending on the value of the `insert_entry` parameter.
  virtual ::p4::v1::TableEntry* initWriteRequest(
      ::p4::v1::WriteRequest* request, bool insert_entry) override {
    if (insert_entry) {
      return initInsertRequest(request);
    } else {
      return initDeleteRequest(request);
    }
  }

  // Sends a Write Table Entry request to the P4Runtime server.
  virtual absl::Status sendWriteRequest(
      const p4::v1::WriteRequest& request) override {
    return DoSendWriteRequest(request);
  }

  //--------------------------------------------------------------------

 protected:
  // Implements sendReadRequest().
  absl::StatusOr<p4::v1::ReadResponse> DoSendReadRequest(
      const p4::v1::ReadRequest& request);

  // Implements sendWriteRequest();
  absl::Status DoSendWriteRequest(const p4::v1::WriteRequest& request);

  // Initializes a WriteRequest message.
  ::p4::v1::TableEntry* InitWriteRequest(::p4::v1::WriteRequest* request,
                                         ::p4::v1::Update_Type type_value);

 private:
  uint64_t device_id_;
  p4::v1::Uint128 election_id_;
  std::string role_name_;
};

}  // namespace ovsp4rt

#endif  // OVSP4RT_TEST_CLIENT_H_
