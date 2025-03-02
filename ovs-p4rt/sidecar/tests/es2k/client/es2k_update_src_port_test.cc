// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ConfigFdbUpdateSrcPort() [ES2K]

#include <absl/status/status.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"
#include "ovsp4rt_util_int.h"
#include "p4/config/v1/p4info.pb.h"
#include "tests/base/basic_test.h"

using ::testing::InvokeWithoutArgs;
using ::testing::Return;

namespace ovsp4rt {

constexpr int SRC_PORT = 72;

class Es2kUpdateSrcPortTest : public BasicTest {
 protected:
  Es2kUpdateSrcPortTest() {}
  virtual ~Es2kUpdateSrcPortTest() = default;

  void InitLearnInfo(struct mac_learning_info& fdb_info) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 42;
    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(MAC_ADDR));
    fdb_info.bridge_id = BRIDGE_ID;
    fdb_info.is_tunnel = true;
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
    param->set_value(EncodeByteValue(4, 0, 0, 0, SRC_PORT));

    return response;
  }
};

/**
 * GetTxAccVsiTableEntry failure path.
 */
TEST_F(Es2kUpdateSrcPortTest, getTxAccVsiEntryFailure) {
  constexpr char VSI_LOOKUP_FAILED[] = "Not found in VSI table";

  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError(VSI_LOOKUP_FAILED)));

  auto status = ConfigFdbUpdateSrcPort(client, learn_info, p4info);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsNotFound(status) && status.message() == VSI_LOOKUP_FAILED);
}

/**
 * GetTxAccVsiTableEntry success path.
 */
TEST_F(Es2kUpdateSrcPortTest, getTxAccVsiEntrySuccess) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(InvokeWithoutArgs(VsiLookupResponse));

  auto status = ConfigFdbUpdateSrcPort(client, learn_info, p4info);

  ASSERT_TRUE(status.ok()) << status;
  ASSERT_EQ(learn_info.src_port, SRC_PORT);
}

}  // namespace ovsp4rt
