// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ES2K version of DoConfigFdbEntry().

#include <absl/status/status.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

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

class Es2kConfigFdbEntryTest : public ::testing::Test {
 protected:
  Es2kConfigFdbEntryTest() {}
  ~Es2kConfigFdbEntryTest() = default;

  void InitLearnInfo(struct mac_learning_info& fdb_info, uint8_t tunnel_type) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(MAC_ADDR));
    fdb_info.bridge_id = 42;
    fdb_info.tnl_info.tunnel_type = tunnel_type;
  }

  void InitV4NativeTagged(struct mac_learning_info& fdb_info) {
    fdb_info.tnl_info.local_ip.family = AF_INET;
    fdb_info.tnl_info.remote_ip.family = AF_INET;
    fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    fdb_info.tnl_info.vni = 0x1984U;
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
TEST_F(Es2kConfigFdbEntryTest, connectFailure) {
  constexpr char CONNECT_ERROR[] = "connect";

  struct mac_learning_info learn_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect)
      .WillOnce(Return(absl::InternalError(CONNECT_ERROR)));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == CONNECT_ERROR)
      << status.message();
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(Es2kConfigFdbEntryTest, getPipelineConfigFailure) {
  constexpr char PIPELINE_ERROR[] = "getPipelineConfig";

  struct mac_learning_info learn_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError(PIPELINE_ERROR)));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == PIPELINE_ERROR)
      << status.message();
}

/**
 * Exercises GetFdbTunnelTableEntry insert path.
 */
TEST_F(Es2kConfigFdbEntryTest, insertVxlanTunnelTableEntry) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info, OVS_TUNNEL_VXLAN);
  InitV4NativeTagged(learn_info);
  learn_info.is_tunnel = true;

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError("GetFdbTunnelTableEntry")));
  EXPECT_CALL(client, sendWriteRequest)
      .WillRepeatedly(Return(absl::OkStatus()));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_TRUE(status.ok()) << status.message();
}

}  // namespace ovsp4rt
