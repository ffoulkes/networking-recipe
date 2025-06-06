// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ReadL2ToTunnelV6TableEntry().

// Core functionality is handled by EncodeL2ToTunnelV6(),
// which is tested separately. This is a test of the non-core
// functionality.

#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <arpa/inet.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "client/ovsp4rt_test_client_mock.h"
#include "core/common/ovsp4rt_entry_utils.h"
#include "mac_learn_info_test.h"
#include "ovsp4rt/ovs-p4rt.h"

using ::testing::InvokeWithoutArgs;
using ::testing::Return;

namespace ovsp4rt {

class ReadL2ToTunnelV6EntryTest : public MacLearnInfoTest {
 public:
  ReadL2ToTunnelV6EntryTest() {}
  virtual ~ReadL2ToTunnelV6EntryTest() = default;

  static void InitFdbInfo(struct mac_learning_info& fdb_info) {
    constexpr uint8_t MAC_ADDR[] = {0xde, 0xad, 0xbe, 0xef, 0x00, 0xe};
    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
  }

  static absl::StatusOr<::p4::v1::ReadResponse> GoodReadResponse() {
    ::p4::v1::ReadResponse response;
    return response;
  }
};

TEST_F(ReadL2ToTunnelV6EntryTest, readL2ToTunnelV6EntryFailure) {
  constexpr char REQUEST_FAILED[] = "sendReadRequest failed";
  struct mac_learning_info learn_info = {0};
  InitFdbInfo(learn_info);
  InitV6TunnelInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError(REQUEST_FAILED)));

  auto response = ReadL2ToTunnelV6TableEntry(client, learn_info, p4info);
  auto status = response.status();

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsNotFound(status) && status.message() == REQUEST_FAILED);
}

TEST_F(ReadL2ToTunnelV6EntryTest, readL2ToTunnelV6EntrySuccess) {
  struct mac_learning_info learn_info = {0};
  InitFdbInfo(learn_info);
  InitV6TunnelInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(InvokeWithoutArgs(GoodReadResponse));

  auto response = ReadL2ToTunnelV6TableEntry(client, learn_info, p4info);

  ASSERT_TRUE(response.ok()) << response.status();
}

}  // namespace ovsp4rt
