// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for DPDK version of DoConfigFdbEntry().

#include <absl/status/status.h>
#include <arpa/inet.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

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
constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class DpdkConfigTunnelEntryTest : public ::testing::Test {
 protected:
  DpdkConfigTunnelEntryTest() {}
  ~DpdkConfigTunnelEntryTest() = default;

  void InitTunnelInfo(struct tunnel_info& tunnel_info, uint8_t tunnel_type) {
    constexpr char IPV4_SRC_ADDR[] = "10.20.30.40";
    constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
    constexpr int IPV4_PREFIX_LEN = 24;

    constexpr uint16_t SRC_PORT = 0x1066;
    constexpr uint16_t DST_PORT = 0x4224;
    constexpr uint16_t VNI = 0x1776;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_SRC_ADDR,
                        &tunnel_info.local_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_SRC_ADDR;
    tunnel_info.local_ip.prefix_len = IPV4_PREFIX_LEN;
    tunnel_info.local_ip.family = AF_INET;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR,
                        &tunnel_info.remote_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_DST_ADDR;
    tunnel_info.remote_ip.prefix_len = IPV4_PREFIX_LEN;
    tunnel_info.remote_ip.family = AF_INET;

    tunnel_info.src_port = SRC_PORT;
    tunnel_info.dst_port = DST_PORT;
    tunnel_info.vni = VNI;
    tunnel_info.tunnel_type = tunnel_type;
  };
};

//----------------------------------------------------------------------
// Setup test cases
//----------------------------------------------------------------------

/**
 * Exercises client.connect() error path.
 */
TEST_F(DpdkConfigTunnelEntryTest, connectFailure) {
  struct tunnel_info tunnel_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::InternalError("connect")));

  auto status =
      DoConfigTunnelEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "connect")
      << status.message();
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(DpdkConfigTunnelEntryTest, getPipelineConfigFailure) {
  struct tunnel_info tunnel_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError("getPipelineConfig")));

  auto status =
      DoConfigTunnelEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "getPipelineConfig")
      << status.message();
}

/**
 * Exercises ConfigEncapTableEntry() error path.
 */
TEST_F(DpdkConfigTunnelEntryTest, encapTableWriteFailure) {
  struct tunnel_info tunnel_info = {0};
  InitTunnelInfo(tunnel_info, OVS_TUNNEL_VXLAN);

  ::p4::config::v1::P4Info expected_p4info;
  {
    // Note: return value is stratum::Status, not absl::Status.
    auto status = stratum::ParseProtoFromString(P4INFO_TEXT, &expected_p4info);
    ASSERT_TRUE(status.ok())
        << "ParseProtoFromString: " << status.error_message();
  }

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillRepeatedly(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError("sendWriteRequest")));

  auto status =
      DoConfigTunnelEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "sendWriteRequest")
      << status.message();
}

/**
 * Exercises ConfigEncapTableEntry() happy path and
 * ConfigTunnelTermTableEntry() error path.
 */
TEST_F(DpdkConfigTunnelEntryTest, configTunnelTermTableWriteFailure) {
  struct tunnel_info tunnel_info = {0};
  InitTunnelInfo(tunnel_info, OVS_TUNNEL_VXLAN);

  ::p4::config::v1::P4Info expected_p4info;
  {
    // Note: return value is stratum::Status, not absl::Status.
    auto status = stratum::ParseProtoFromString(P4INFO_TEXT, &expected_p4info);
    ASSERT_TRUE(status.ok())
        << "ParseProtoFromString: " << status.error_message();
  }

  TestClientMock client;
  EXPECT_CALL(client, connect).WillRepeatedly(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillRepeatedly(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::OkStatus()))
      .WillOnce(Return(absl::InternalError("sendWriteRequest")));

  auto status =
      DoConfigTunnelEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "sendWriteRequest")
      << status.message();
}

/**
 * Exercises ConfigTunnelTermTableEntry() happy path.
 */
TEST_F(DpdkConfigTunnelEntryTest, tunnelTermTableWriteSuccess) {
  struct tunnel_info tunnel_info = {0};
  InitTunnelInfo(tunnel_info, OVS_TUNNEL_VXLAN);

  ::p4::config::v1::P4Info expected_p4info;
  {
    // Note: return value is stratum::Status, not absl::Status.
    auto status = stratum::ParseProtoFromString(P4INFO_TEXT, &expected_p4info);
    ASSERT_TRUE(status.ok())
        << "ParseProtoFromString: " << status.error_message();
  }

  TestClientMock client;
  EXPECT_CALL(client, connect).WillRepeatedly(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillRepeatedly(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillRepeatedly(Return(absl::OkStatus()));

  auto status =
      DoConfigTunnelEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_TRUE(status.ok()) << status.message();
}

}  // namespace ovsp4rt
