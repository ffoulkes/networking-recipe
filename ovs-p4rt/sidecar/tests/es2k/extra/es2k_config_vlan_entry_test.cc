// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ES2K version of DoConfigVlanEntry().

#include <absl/status/status.h>
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
constexpr uint16_t VLAN_ID = 0x1984;

class Es2kConfigVlanEntryTest : public ::testing::Test {
 protected:
  Es2kConfigVlanEntryTest() {}
  virtual ~Es2kConfigVlanEntryTest() = default;

  void InitP4Info(::p4::config::v1::P4Info* p4info) {
    auto status = stratum::ParseProtoFromString(P4INFO_TEXT, p4info);
    EXPECT_TRUE(status.ok())
        << "ParseProtoFromString: " << status.error_message();
  }
};

/**
 * Exercises client.connect() error path.
 */
TEST_F(Es2kConfigVlanEntryTest, connectFailure) {
  constexpr char CONNECT_ERROR[] = "connect";
  constexpr uint16_t vlan_id = VLAN_ID;

  TestClientMock client;
  EXPECT_CALL(client, connect)
      .WillOnce(Return(absl::InternalError(CONNECT_ERROR)));

  auto status = DoConfigVlanEntry(client, vlan_id, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == CONNECT_ERROR)
      << status.message();
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(Es2kConfigVlanEntryTest, getPipelineConfigFailure) {
  constexpr char PIPELINE_ERROR[] = "getPipelineConfig";
  constexpr uint16_t vlan_id = VLAN_ID;

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError(PIPELINE_ERROR)));

  auto status = DoConfigVlanEntry(client, vlan_id, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == PIPELINE_ERROR)
      << status.message();
}

/**
 * Exercises the ConfigVlanPushTableEntry error path.
 */
TEST_F(Es2kConfigVlanEntryTest, configVlanPushTableEntryFailure) {
  constexpr char PUSH_ENTRY_ERROR[] = "ConfigVlanPushTableEntry";
  constexpr uint16_t vlan_id = VLAN_ID;

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError(PUSH_ENTRY_ERROR)));

  auto status = DoConfigVlanEntry(client, vlan_id, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == PUSH_ENTRY_ERROR)
      << status.message();
}

/**
 * Exercises the ConfigVlanPopTableEntry error path.
 */
TEST_F(Es2kConfigVlanEntryTest, configVlanPopTableEntryFailure) {
  constexpr char POP_ENTRY_ERROR[] = "ConfigVlanPopTableEntry";
  constexpr uint16_t vlan_id = VLAN_ID;

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillRepeatedly(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::OkStatus()))  // ConfigVlanPushTableEntry
      .WillOnce(Return(absl::InternalError(POP_ENTRY_ERROR)));

  auto status = DoConfigVlanEntry(client, vlan_id, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == POP_ENTRY_ERROR)
      << status.message();
}

/**
 * Exercises the DoConfigVlanEntry happy path.
 */
TEST_F(Es2kConfigVlanEntryTest, doConfigVlanEntrySuccess) {
  constexpr uint16_t vlan_id = VLAN_ID;

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillRepeatedly(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillRepeatedly(Return(absl::OkStatus()));

  auto status = DoConfigVlanEntry(client, vlan_id, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_TRUE(status.ok()) << status.message();
}

}  // namespace ovsp4rt
