// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ES2K version of DoConfigTunnelEntry().

#include <absl/status/status.h>
#include <arpa/inet.h>
#include <stdint.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_do_config_int.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class Es2kConfigTunnelEntryTest : public ::testing::Test {
 protected:
  Es2kConfigTunnelEntryTest() {}
  virtual ~Es2kConfigTunnelEntryTest() = default;

  void InitIpv4TunnelInfo(struct tunnel_info& tunnel_info) {
    constexpr char IPV4_SRC_ADDR[] = "10.20.30.40";
    constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
    constexpr int IPV4_PREFIX_LEN = 24;

    constexpr uint16_t SRC_PORT = 0x1066;
    constexpr uint16_t DST_PORT = 0x4224;

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
  };

  void InitVxlanTagged(struct tunnel_info& tunnel_info) {
    tunnel_info.tunnel_type = OVS_TUNNEL_VXLAN;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    tunnel_info.vni = 0x1066;
  }

  void InitVxlanUntagged(struct tunnel_info& tunnel_info) {
    tunnel_info.tunnel_type = OVS_TUNNEL_VXLAN;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    tunnel_info.vni = 0x1492;
  }

  void InitGeneveTagged(struct tunnel_info& tunnel_info) {
    tunnel_info.tunnel_type = OVS_TUNNEL_GENEVE;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    tunnel_info.vni = 0x1776;
  }

  void InitGeneveUntagged(struct tunnel_info& tunnel_info) {
    tunnel_info.tunnel_type = OVS_TUNNEL_GENEVE;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    tunnel_info.vni = 0x1984;
  }

  void InitP4Info(::p4::config::v1::P4Info* p4info) {
    auto status = stratum::ParseProtoFromString(P4INFO_TEXT, p4info);
    EXPECT_TRUE(status.ok())
        << "ParseProtoFromString: " << status.error_message();
  }
};

//----------------------------------------------------------------------
// Setup test cases
//----------------------------------------------------------------------

/**
 * Exercises client.connect() error path.
 */
TEST_F(Es2kConfigTunnelEntryTest, connectFailure) {
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
TEST_F(Es2kConfigTunnelEntryTest, getPipelineConfigFailure) {
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

//----------------------------------------------------------------------
// WriteEncapTableEntry
//----------------------------------------------------------------------

/**
 * Exercises WriteEncapTableEntry() error path.
 */
TEST_F(Es2kConfigTunnelEntryTest, encapTableWriteFailure) {
  constexpr char ERROR_MESSAGE[] = "WriteEncapTableEntry";

  struct tunnel_info tunnel_info = {0};
  InitIpv4TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillRepeatedly(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status =
      DoConfigTunnelEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE)
      << status.message();
}

/**
 * Exercises WriteEncapTableEntry() happy path and
 * WriteDecapTableEntry() error path.
 */
TEST_F(Es2kConfigTunnelEntryTest, decapTableWriteFailure) {
  constexpr char ERROR_MESSAGE[] = "WriteDecapTableEntry";

  struct tunnel_info tunnel_info = {0};
  InitIpv4TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillRepeatedly(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillRepeatedly(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::OkStatus()))  // WriteEncapTableEntry
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status =
      DoConfigTunnelEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE)
      << status.message();
}

//----------------------------------------------------------------------
// WriteDecapTableEntry
//----------------------------------------------------------------------

/**
 * Exercises WriteDecapTableEntry() happy path and
 * WriteTunnelTermTableEntry() error path.
 */
TEST_F(Es2kConfigTunnelEntryTest, tunnelTermTableWriteFailure) {
  constexpr char ERROR_MESSAGE[] = "WriteTunnelTermTableEntry";

  struct tunnel_info tunnel_info = {0};
  InitIpv4TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillRepeatedly(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillRepeatedly(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::OkStatus()))  // WriteEncapTableEntry
      .WillOnce(Return(absl::OkStatus()))  // WriteDecapTableEntry
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status =
      DoConfigTunnelEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE)
      << status.message();
}

//----------------------------------------------------------------------
// WriteTunnelTermTableEntry
//----------------------------------------------------------------------

/**
 * Exercises WriteTunnelTermTableEntry() happy path.
 */
TEST_F(Es2kConfigTunnelEntryTest, tunnelTermTableWriteSuccess) {
  struct tunnel_info tunnel_info = {0};
  InitIpv4TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

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
