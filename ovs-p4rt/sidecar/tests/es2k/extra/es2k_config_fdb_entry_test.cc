// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ES2K version of DoConfigFdbEntry().

#include <absl/status/status.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_do_config_int.h"
#include "ovsp4rt_util_int.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

using ::testing::DoAll;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr bool DELETE_ENTRY = false;
constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class Es2kConfigFdbEntryTest : public ::testing::Test {
 protected:
  Es2kConfigFdbEntryTest() {}
  virtual ~Es2kConfigFdbEntryTest() = default;

  void InitTunnelLearnInfo(struct mac_learning_info& fdb_info,
                           uint8_t tunnel_type) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 42;

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(MAC_ADDR));
    fdb_info.bridge_id = BRIDGE_ID;
    fdb_info.tnl_info.tunnel_type = tunnel_type;
    fdb_info.is_tunnel = true;
  }

  void InitVlanLearnInfo(struct mac_learning_info& fdb_info) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 99;
    constexpr uint32_t SRC_PORT = 0x42;

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
    fdb_info.bridge_id = BRIDGE_ID;
    fdb_info.rx_src_port = SRC_PORT;
    fdb_info.is_vlan = true;
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

  static absl::StatusOr<::p4::v1::ReadResponse> VsiLookupResponse() {
    constexpr int TABLE_ID = 42508227;   // tx_acc_vsi
    constexpr int ACTION_ID = 31624713;  // l2_fwd_and_bypass_bridge
    constexpr int PARAM_ID = 1;          // port

    ::p4::v1::ReadResponse response;
    auto entity = response.add_entities();

    auto table_entry = entity->mutable_table_entry();
    table_entry->set_table_id(TABLE_ID);

    auto table_action = table_entry->mutable_action();

    auto action = table_action->mutable_action();
    action->set_action_id(ACTION_ID);

    auto param = action->add_params();
    param->set_param_id(PARAM_ID);
    param->set_value(EncodeByteValue(4, 0, 0, 0, 72));

    return response;
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
 * Exercises the DoConfigFdbEntry delete path.
 *
 * Note that ConfigFdbUpdateTunnelInfo() has its own unit test.
 */
TEST_F(Es2kConfigFdbEntryTest, deleteVxlanTunnelTableEntry) {
  struct mac_learning_info learn_info = {0};
  InitTunnelLearnInfo(learn_info, OVS_TUNNEL_VXLAN);
  InitV4NativeTagged(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError("ReadFdbTunnelTableEntry failed")));
  EXPECT_CALL(client, sendWriteRequest)
      .WillRepeatedly(Return(absl::OkStatus()));

  auto status = DoConfigFdbEntry(client, learn_info, DELETE_ENTRY, GRPC_ADDR);

  ASSERT_TRUE(status.ok()) << status.message();
}

/**
 * Exercises the ConfigFdbTunnelEntry insert path.
 */
TEST_F(Es2kConfigFdbEntryTest, insertVxlanTunnelTableEntry) {
  struct mac_learning_info learn_info = {0};
  InitTunnelLearnInfo(learn_info, OVS_TUNNEL_VXLAN);
  InitV4NativeTagged(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError("ReadFdbTunnelTableEntry failed")));
  EXPECT_CALL(client, sendWriteRequest)
      .WillRepeatedly(Return(absl::OkStatus()));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_TRUE(status.ok()) << status.message();
}

/**
 * Exercises ConfigFdbVlanEntry with the ReadTxAccVsiTableEntry
 * failure path.
 */
TEST_F(Es2kConfigFdbEntryTest, insertVlanEntryVsiNotFound) {
  constexpr char VLAN_LOOKUP_FAILED[] = "ReadFdbVlanTableEntry failed";
  constexpr char VSI_LOOKUP_FAILED[] = "ReadTxAccVsiTableEntry failed";

  struct mac_learning_info learn_info = {0};
  InitVlanLearnInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError(VLAN_LOOKUP_FAILED)))
      .WillOnce(Return(absl::NotFoundError(VSI_LOOKUP_FAILED)));
  EXPECT_CALL(client, sendWriteRequest)
      .WillRepeatedly(Return(absl::OkStatus()));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsNotFound(status) && status.message() == VSI_LOOKUP_FAILED);
}

/**
 * Exercises ConfigFdbVlanEntry with the ReadTxAccVsiTableEntry
 * success path.
 */
TEST_F(Es2kConfigFdbEntryTest, insertVlanEntryVsiFound) {
  constexpr char VLAN_LOOKUP_FAILED[] = "ReadFdbVlanTableEntry failed";

  struct mac_learning_info learn_info = {0};
  InitVlanLearnInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError(VLAN_LOOKUP_FAILED)))
      .WillOnce(InvokeWithoutArgs(VsiLookupResponse));
  EXPECT_CALL(client, sendWriteRequest)
      .WillRepeatedly(Return(absl::OkStatus()));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_TRUE(status.ok()) << status;
}

}  // namespace ovsp4rt
