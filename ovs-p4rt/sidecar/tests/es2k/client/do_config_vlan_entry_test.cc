// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Minimal test for DoConfigVlanEntry().

#include <absl/status/status.h>
#include <stdint.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "basic_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "core/v2/ovsp4rt_do_config_int.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"

using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace ovsp4rt {

constexpr char GRPC_ADDR[] = "1.2.3.4:5678";
constexpr uint16_t VLAN_ID = 0x1984;

class DoConfigVlanEntryTest : public ::testing::Test {
 protected:
  DoConfigVlanEntryTest() {}
  virtual ~DoConfigVlanEntryTest() = default;
};

/**
 * Exercises client.connect() error path.
 */
TEST_F(DoConfigVlanEntryTest, connectFailure) {
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
TEST_F(DoConfigVlanEntryTest, getPipelineConfigFailure) {
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

}  // namespace ovsp4rt
