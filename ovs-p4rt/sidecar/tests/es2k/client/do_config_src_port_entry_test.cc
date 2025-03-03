// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Minimal unit test for DoConfigTunnelEntry().

#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <stdint.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "basic_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_do_config_int.h"
#include "ovsp4rt_util_int.h"  // EncodeByteValue
#include "p4/config/v1/p4info.pb.h"

using ::testing::Return;

namespace ovsp4rt {

constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class DoConfigSrcPortEntryTest : public BasicTest {
 protected:
  DoConfigSrcPortEntryTest() {}
  virtual ~DoConfigSrcPortEntryTest() = default;

  void InitSrcPortInfo(struct src_port_info& port_info) {
    port_info.bridge_id = 42;
    port_info.vlan_id = 0x123;
    port_info.src_port = 0x1066;
  }

  static absl::StatusOr<::p4::v1::ReadResponse> DummySendReadRequest() {
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

/**
 * Exercises client.connect() error path.
 */
TEST_F(DoConfigSrcPortEntryTest, connectFailure) {
  struct src_port_info port_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::InternalError("connect")));

  auto status =
      DoConfigSrcPortEntry(client, port_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "connect") << status;
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(DoConfigSrcPortEntryTest, getPipelineConfigFailure) {
  struct src_port_info port_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError("getPipelineConfig")));

  auto status =
      DoConfigSrcPortEntry(client, port_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "getPipelineConfig")
      << status;
}

}  // namespace ovsp4rt
