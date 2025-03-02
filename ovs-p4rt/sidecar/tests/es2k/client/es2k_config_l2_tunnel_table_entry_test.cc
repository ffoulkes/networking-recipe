// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ConfigL2TunnelTableEntry().

// Core functionality is provided by PrepareL2TunnelTableEntry(),
// which is tested separately. This is a test of the non-core
// functionality.

#include <absl/status/status.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "tests/base/base_mac_learn_info_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"

using ::testing::Return;

namespace ovsp4rt {

class ConfigL2TunnelTableEntryTest : public BaseMacLearnInfoTest {
 public:
  ConfigL2TunnelTableEntryTest() {}
  virtual ~ConfigL2TunnelTableEntryTest() = default;
};

TEST_F(ConfigL2TunnelTableEntryTest, ConfigL2TunnelTableEntryFailure) {
  constexpr char ERROR_MESSAGE[] = "sendWriteRequest failed";

  struct mac_learning_info learn_info = {0};
  InitV4TunnelInfo(learn_info);
  InitVxlanTagged(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status =
      ConfigL2TunnelTableEntry(client, learn_info, p4info, INSERT_ENTRY);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE)
      << status.message();
}

TEST_F(ConfigL2TunnelTableEntryTest, ConfigL2TunnelTableEntrySuccess) {
  struct mac_learning_info learn_info = {0};
  InitV4TunnelInfo(learn_info);
  InitVxlanTagged(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest).WillOnce(Return(absl::OkStatus()));

  auto status =
      ConfigL2TunnelTableEntry(client, learn_info, p4info, INSERT_ENTRY);

  ASSERT_TRUE(status.ok()) << status;
}

}  // namespace ovsp4rt
