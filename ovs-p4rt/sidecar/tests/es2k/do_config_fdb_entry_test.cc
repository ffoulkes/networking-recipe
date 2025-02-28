// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Minimal unit test for DoConfigFdbEntry().

#include <absl/status/status.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_doconfig_int.h"
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

class DoConfigFdbEntryTest : public ::testing::Test {
 protected:
  DoConfigFdbEntryTest() {}
  virtual ~DoConfigFdbEntryTest() = default;

  void InitP4Info(::p4::config::v1::P4Info* p4info) {
    auto status = stratum::ParseProtoFromString(P4INFO_TEXT, p4info);
    EXPECT_TRUE(status.ok())
        << "ParseProtoFromString: " << status;
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

/**
 * Exercises client.connect() error path.
 */
TEST_F(DoConfigFdbEntryTest, connectFailure) {
  constexpr char CONNECT_ERROR[] = "connect";

  struct mac_learning_info learn_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect)
      .WillOnce(Return(absl::InternalError(CONNECT_ERROR)));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == CONNECT_ERROR)
      << status;
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(DoConfigFdbEntryTest, getPipelineConfigFailure) {
  constexpr char PIPELINE_ERROR[] = "getPipelineConfig";

  struct mac_learning_info learn_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError(PIPELINE_ERROR)));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == PIPELINE_ERROR)
      << status;
}

}  // namespace ovsp4rt
