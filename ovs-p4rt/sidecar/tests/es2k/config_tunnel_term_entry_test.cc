// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ConfigTunnelTermTableEntry (common)

// Core functionality is handled by PrepareL2ToTunnelV4(),
// which is tested separately. This is a test of the non-core
// functionality.

#include <absl/status/status.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "base_tunnel_info_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt_config_int.h"

using ::testing::Return;

namespace ovsp4rt {

class ConfigTunnelTermEntryTest : public BaseTunnelInfoTest {
 public:
  ConfigTunnelTermEntryTest() {}
  virtual ~ConfigTunnelTermEntryTest() = default;
};

TEST_F(ConfigTunnelTermEntryTest, configTunnelTermEntryFailure) {
  constexpr char ERROR_MESSAGE[] = "sendWriteRequest failed";
  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitVxlanUntagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status =
      ConfigTunnelTermTableEntry(client, tunnel_info, p4info, INSERT_ENTRY);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE)
      << status;
}

TEST_F(ConfigTunnelTermEntryTest, configTunnelTermEntrySuccess) {
  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest).WillOnce(Return(absl::OkStatus()));

  auto status =
      ConfigTunnelTermTableEntry(client, tunnel_info, p4info, INSERT_ENTRY);

  ASSERT_TRUE(status.ok()) << status;
}

}  // namespace ovsp4rt
