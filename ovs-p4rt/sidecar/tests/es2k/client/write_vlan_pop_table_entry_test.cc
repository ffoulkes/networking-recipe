// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for WriteVlanPopTableEntry().

// Core functionality is handled by EncodeVlanPopTableEntry(),
// which is tested separately. This is a test of the non-core
// functionality.

#include <absl/status/status.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "basic_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "core/common/ovsp4rt_entry_utils.h"
#include "ovsp4rt/ovs-p4rt.h"

using ::testing::Return;

namespace ovsp4rt {

class WriteVlanPopTableEntryTest : public BasicTest {
 public:
  WriteVlanPopTableEntryTest() {}
  virtual ~WriteVlanPopTableEntryTest() = default;
};

TEST_F(WriteVlanPopTableEntryTest, writeVlanPopEntryFailure) {
  constexpr char ERROR_MESSAGE[] = "sendWriteRequest failed";
  constexpr uint16_t VLAN_ID = 0x1776;

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status = WriteVlanPopTableEntry(client, VLAN_ID, p4info, INSERT_ENTRY);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE)
      << status.message();
}

TEST_F(WriteVlanPopTableEntryTest, writeVlanPopEntrySuccess) {
  constexpr uint16_t VLAN_ID = 0x1492;

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest).WillOnce(Return(absl::OkStatus()));

  auto status = WriteVlanPopTableEntry(client, VLAN_ID, p4info, INSERT_ENTRY);

  ASSERT_TRUE(status.ok()) << status;
}

}  // namespace ovsp4rt
