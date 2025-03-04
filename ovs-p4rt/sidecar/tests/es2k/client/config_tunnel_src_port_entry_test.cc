// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ConfigTunnelSrcPortEntry().

#include <absl/status/status.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "basic_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"
#include "p4/config/v1/p4info.pb.h"

using ::testing::Return;

namespace ovsp4rt {

constexpr char GRPC_ADDR[] = "172.54.19.3:5678";

class ConfigTunnelSrcPortTest : public BasicTest {
 protected:
  ConfigTunnelSrcPortTest() {}
  ~ConfigTunnelSrcPortTest() = default;

  void InitPortInfo(struct src_port_info& port_info) {
    port_info.bridge_id = 42;
    port_info.vlan_id = 0x123;
    port_info.src_port = 0x1066;
  }
};

TEST_F(ConfigTunnelSrcPortTest, configTunnelSrcPortFailure) {
  constexpr char ERROR_MESSAGE[] = "sendWriteRequest failed";

  struct src_port_info port_info = {0};
  InitPortInfo(port_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status =
      ConfigTunnelSrcPortEntry(client, port_info, p4info, INSERT_ENTRY);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE)
      << status;
}

TEST_F(ConfigTunnelSrcPortTest, configTunnelSrcPortSuccess) {
  struct src_port_info port_info = {0};
  InitPortInfo(port_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest).WillOnce(Return(absl::OkStatus()));

  auto status =
      ConfigTunnelSrcPortEntry(client, port_info, p4info, INSERT_ENTRY);

  ASSERT_TRUE(status.ok()) << status;
}

}  // namespace ovsp4rt
