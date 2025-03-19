// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ReadVmSrcTableEntry().

// Core functionality is handled by EncodeSrcIpMacMapTableEntry(),
// which is tested separately. This is a test of the non-core
// functionality.

#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <arpa/inet.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "base_mac_map_info_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"

using ::testing::InvokeWithoutArgs;
using ::testing::Return;

namespace ovsp4rt {

class ReadVmSrcTableEntryTest : public BaseMacMapInfoTest {
 public:
  ReadVmSrcTableEntryTest() {}
  virtual ~ReadVmSrcTableEntryTest() = default;

  static absl::StatusOr<::p4::v1::ReadResponse> GoodReadResponse() {
    ::p4::v1::ReadResponse response;
    return response;
  }
};

TEST_F(ReadVmSrcTableEntryTest, readVmSrcEntryFailure) {
  constexpr char REQUEST_FAILED[] = "sendReadRequest failed";

  struct ip_mac_map_info map_info = {0};
  InitIpv4MapInfo(map_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError(REQUEST_FAILED)));

  auto response = ReadVmSrcTableEntry(client, map_info, p4info);
  auto status = response.status();

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsNotFound(status) && status.message() == REQUEST_FAILED);
}

TEST_F(ReadVmSrcTableEntryTest, readVmSrcEntrySuccess) {
  struct ip_mac_map_info map_info = {0};
  InitIpv4MapInfo(map_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(InvokeWithoutArgs(GoodReadResponse));

  auto response = ReadVmSrcTableEntry(client, map_info, p4info);

  ASSERT_TRUE(response.ok()) << response.status();
}

}  // namespace ovsp4rt
