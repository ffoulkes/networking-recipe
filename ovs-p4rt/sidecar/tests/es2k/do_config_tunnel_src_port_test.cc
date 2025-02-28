// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for DoConfigSrcPortEntry().

#include <absl/status/status.h>
#include <stdint.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_doconfig_int.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr char GRPC_ADDR[] = "172.54.19.3:5678";

class DoConfigTunnelSrcPortTest : public ::testing::Test {
 protected:
  DoConfigTunnelSrcPortTest() {}
  ~DoConfigTunnelSrcPortTest() = default;

  void InitPortInfo(struct src_port_info& port_info) {
    port_info.bridge_id = 42;
    port_info.vlan_id = 0x123;
    port_info.src_port = 0x1066;
  }

  void InitP4Info(::p4::config::v1::P4Info* p4info) {
    auto status = stratum::ParseProtoFromString(P4INFO_TEXT, p4info);
    EXPECT_TRUE(status.ok()) << "ParseProtoFromString: " << status;
  }
};

/**
 * Exercises client.connect() error path.
 */
TEST_F(DoConfigTunnelSrcPortTest, connectFailure) {
  struct src_port_info port_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::InternalError("connect")));

  auto status =
      DoConfigTunnelSrcPortEntry(client, port_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "connect") << status;
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(DoConfigTunnelSrcPortTest, getPipelineConfigFailure) {
  struct src_port_info port_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError("getPipelineConfig")));

  auto status =
      DoConfigTunnelSrcPortEntry(client, port_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "getPipelineConfig")
      << status;
}

}  // namespace ovsp4rt
