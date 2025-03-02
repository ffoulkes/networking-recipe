// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Minimal unit test for DoConfigRxTunnelSrcEntry().

#include <absl/status/status.h>
#include <arpa/inet.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_do_config_int.h"
#include "p4/config/v1/p4info.pb.h"
#include "tests/base/basic_test.h"

using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace ovsp4rt {

constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class DoConfigRxTunnelSrcPortTest : public BasicTest {
 protected:
  DoConfigRxTunnelSrcPortTest() {}
  ~DoConfigRxTunnelSrcPortTest() = default;

  void InitIpv4TunnelInfo(struct tunnel_info& tunnel_info) {
    constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
    constexpr int IPV4_PREFIX_LEN = 24;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR,
                        &tunnel_info.remote_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_DST_ADDR;
    tunnel_info.remote_ip.family = AF_INET;
    tunnel_info.remote_ip.prefix_len = IPV4_PREFIX_LEN;

    tunnel_info.vni = 0x123;
    tunnel_info.src_port = 99;
  }

  void InitIpv6TunnelInfo(struct tunnel_info& tunnel_info) {
    constexpr char IPV6_SRC_ADDR[] = "fe80::215:5dff:fefa";
    constexpr char IPV6_DST_ADDR[] = "fe80::215:192.168.17.5";
    constexpr int IPV6_PREFIX_LEN = 64;

    EXPECT_EQ(inet_pton(AF_INET6, IPV6_DST_ADDR,
                        &tunnel_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32),
              1)
        << "Error converting " << IPV6_DST_ADDR;
    tunnel_info.remote_ip.prefix_len = IPV6_PREFIX_LEN;

    tunnel_info.vni = 0x911;
    tunnel_info.src_port = 0xFADE;
  }
};

/**
 * Exercises client.connect() error path.
 */
TEST_F(DoConfigRxTunnelSrcPortTest, connectFailure) {
  constexpr char CONNECT_ERROR[] = "connect";

  struct tunnel_info tunnel_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect)
      .WillOnce(Return(absl::InternalError(CONNECT_ERROR)));

  auto status =
      DoConfigRxTunnelSrcEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == CONNECT_ERROR)
      << status;
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(DoConfigRxTunnelSrcPortTest, getPipelineConfigFailure) {
  constexpr char PIPELINE_ERROR[] = "getPipelineConfig";

  struct tunnel_info tunnel_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError(PIPELINE_ERROR)));

  auto status =
      DoConfigRxTunnelSrcEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == PIPELINE_ERROR)
      << status;
}

}  // namespace ovsp4rt
