// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ReadVmDstTableEntry().

// Core functionality is handled by EncodeDstIpMacMapTableEntry(),
// which is tested separately. This is a test of the non-core
// functionality.

#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <arpa/inet.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "mac_map_info_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"

using ::testing::InvokeWithoutArgs;
using ::testing::Return;

namespace ovsp4rt {

class ReadVmDstTableEntryTest : public MacMapInfoTest {
 public:
  ReadVmDstTableEntryTest() {}
  virtual ~ReadVmDstTableEntryTest() = default;

  static absl::StatusOr<::p4::v1::ReadResponse> GoodReadResponse() {
    ::p4::v1::ReadResponse response;
    return response;
  }
};

TEST_F(ReadVmDstTableEntryTest, readVmDstEntryFailure) {
  constexpr char REQUEST_FAILED[] = "sendReadRequest failed";

  struct ip_mac_map_info map_info = {0};
  InitIpv4MapInfo(map_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError(REQUEST_FAILED)));

  auto response = ReadVmDstTableEntry(client, map_info, p4info);
  auto status = response.status();

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsNotFound(status) && status.message() == REQUEST_FAILED);
}

TEST_F(ReadVmDstTableEntryTest, readVmDstEntrySuccess) {
  struct ip_mac_map_info map_info = {0};
  InitIpv4MapInfo(map_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(InvokeWithoutArgs(GoodReadResponse));

  auto response = ReadVmDstTableEntry(client, map_info, p4info);

  ASSERT_TRUE(response.ok()) << response.status();
}

}  // namespace ovsp4rt
