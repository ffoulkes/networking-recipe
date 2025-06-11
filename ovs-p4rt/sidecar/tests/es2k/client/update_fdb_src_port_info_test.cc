// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for UpdateFdbSrcPortInfo() [ES2K]

#include <absl/status/status.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "basic_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "core/common/ovsp4rt_encode_utils.h"
#include "core/common/ovsp4rt_entry_utils.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"

using ::testing::InvokeWithoutArgs;
using ::testing::Return;

namespace ovsp4rt {

constexpr uint32_t PORT8 = 72;
constexpr uint32_t PORT16 = 0xBED;
constexpr uint32_t PORT32 = 0xACED;

class UpdateFdbSrcPortInfoTest : public BasicTest {
 protected:
  UpdateFdbSrcPortInfoTest() {}
  virtual ~UpdateFdbSrcPortInfoTest() = default;

  void InitLearnInfo(struct mac_learning_info& fdb_info) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 42;
    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(MAC_ADDR));
    fdb_info.bridge_id = BRIDGE_ID;
    fdb_info.is_tunnel = true;
  }

  static absl::StatusOr<::p4::v1::ReadResponse> VsiLookupResponse(
      const uint8_t port[4]) {
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
    param->set_value(EncodeByteValue(4, port[0], port[1], port[2], port[3]));

    return response;
  }

  static absl::StatusOr<::p4::v1::ReadResponse> Port8() {
    constexpr uint8_t src_port[4] = {0, 0, 0, PORT8};
    return VsiLookupResponse(src_port);
  }

  static absl::StatusOr<::p4::v1::ReadResponse> Port16() {
    constexpr uint8_t src_port[4] = {0, 0, (PORT16 >> 8) & 0xff, PORT16 & 0xff};
    return VsiLookupResponse(src_port);
  }

  static absl::StatusOr<::p4::v1::ReadResponse> Port32() {
    constexpr uint8_t src_port[4] = {(PORT32 >> 24) & 0xff,
                                     (PORT32 >> 16) & 0xff,
                                     (PORT32 >> 8) & 0xff, PORT32 & 0xff};
    return VsiLookupResponse(src_port);
  }
};

/**
 * UpdateFdbSrcPortInfo failure path.
 */
TEST_F(UpdateFdbSrcPortInfoTest, entryNotFoundInTable) {
  constexpr char VSI_LOOKUP_FAILED[] = "Not found in VSI table";

  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError(VSI_LOOKUP_FAILED)));

  auto status = UpdateFdbSrcPortInfo(client, learn_info, p4info);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsNotFound(status) && status.message() == VSI_LOOKUP_FAILED);
}

/**
 * UpdateFdbSrcPortInfo success (8-bit port number).
 */
TEST_F(UpdateFdbSrcPortInfoTest, entryFound8Bits) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest).WillOnce(InvokeWithoutArgs(Port8));

  auto status = UpdateFdbSrcPortInfo(client, learn_info, p4info);

  ASSERT_TRUE(status.ok()) << status;
  ASSERT_EQ(learn_info.src_port, PORT8);
}

/**
 * UpdateFdbSrcPortInfo success (16-bit port number).
 */
TEST_F(UpdateFdbSrcPortInfoTest, entryFound16Bits) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest).WillOnce(InvokeWithoutArgs(Port16));

  auto status = UpdateFdbSrcPortInfo(client, learn_info, p4info);

  ASSERT_TRUE(status.ok()) << status;
  ASSERT_EQ(learn_info.src_port, PORT16);
}

/**
 * UpdateFdbSrcPortInfo success (32-bit port number).
 */
TEST_F(UpdateFdbSrcPortInfoTest, entryFound32Bits) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest).WillOnce(InvokeWithoutArgs(Port32));

  auto status = UpdateFdbSrcPortInfo(client, learn_info, p4info);

  ASSERT_TRUE(status.ok()) << status;
  ASSERT_EQ(learn_info.src_port, PORT32);
}

}  // namespace ovsp4rt
