// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for DPDK version of DoConfigFdbEntry().

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

class DpdkConfigFdbEntryTest : public ::testing::Test {
 protected:
  DpdkConfigFdbEntryTest() {}
  ~DpdkConfigFdbEntryTest() = default;

  void InitFdbTunnelInfo(struct mac_learning_info& learn_info) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

    memcpy(learn_info.mac_addr, MAC_ADDR, sizeof(learn_info.mac_addr));
    learn_info.is_tunnel = true;
  }

  void InitFdbVlanInfo(struct mac_learning_info& learn_info) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    // Note: 8-bit vlan_id.
    constexpr uint32_t VLAN_ID = 42;

    memcpy(learn_info.mac_addr, MAC_ADDR, sizeof(learn_info.mac_addr));
    learn_info.vln_info.vlan_id = VLAN_ID;
    learn_info.is_vlan = true;
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
TEST_F(DpdkConfigFdbEntryTest, connectFailure) {
  struct mac_learning_info learn_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::InternalError("connect")));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "connect")
      << status.message();
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(DpdkConfigFdbEntryTest, getPipelineConfigFailure) {
  struct mac_learning_info learn_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError("getPipelineConfig")));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "getPipelineConfig")
      << status.message();
}

//----------------------------------------------------------------------
// Tunnel test cases
//----------------------------------------------------------------------

/**
 * Exercises ConfigFdbTunnelTableEntry() error path.
 */
TEST_F(DpdkConfigFdbEntryTest, configFdbTunnelEntryWriteFailure) {
  struct mac_learning_info learn_info = {0};
  InitFdbTunnelInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError("sendWriteRequest")));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "sendWriteRequest")
      << status.message();
}

/**
 * Exercises tunnel configuration happy path.
 */
TEST_F(DpdkConfigFdbEntryTest, configFdbTunnelEntryWriteSuccess) {
  struct mac_learning_info learn_info = {0};
  InitFdbTunnelInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest).WillOnce(Return(absl::OkStatus()));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_TRUE(status.ok()) << status.message();
}

//----------------------------------------------------------------------
// Vlan test cases
//----------------------------------------------------------------------

/**
 * Exercises ConfigFdbTxVlanTableEntry() failure path.
 */
TEST_F(DpdkConfigFdbEntryTest, configVlanTxEntryWriteFailure) {
  struct mac_learning_info learn_info = {0};
  InitFdbVlanInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError("sendWriteRequest")));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "sendWriteRequest")
      << status.message();
}

/**
 * Exercises ConfigFdbRxVlanTableEntry() failure path.
 */
TEST_F(DpdkConfigFdbEntryTest, configFdbRxVlanRxEntryWriteFailure) {
  struct mac_learning_info learn_info = {0};
  InitFdbVlanInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      // ConfigFdbTxVlanTableEntry() succeeds
      .WillOnce(Return(absl::OkStatus()))
      // ConfigFdbRxVlanTableEntry() fails
      .WillOnce(Return(absl::InternalError("sendWriteRequest")));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "sendWriteRequest")
      << status.message();
}

/**
 * Exercises vlan configuration happy path.
 */
TEST_F(DpdkConfigFdbEntryTest, configVlanRxEntryWriteSuccess) {
  struct mac_learning_info learn_info = {0};
  InitFdbVlanInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillRepeatedly(Return(absl::OkStatus()));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_TRUE(status.ok()) << status.message();
}

}  // namespace ovsp4rt
