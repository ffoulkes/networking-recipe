// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for WriteVsiSrcPortTableEntry().

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

class WriteVsiSrcPortEntryTest : public BasicTest {
 public:
  WriteVsiSrcPortEntryTest() {}
  virtual ~WriteVsiSrcPortEntryTest() = default;

  static void InitPortInfo(struct src_port_info& port_info) {
    port_info.bridge_id = 42;
    port_info.vlan_id = 0x123;
    port_info.src_port = 0x1066;
  }
};

TEST_F(WriteVsiSrcPortEntryTest, writeVsiSrcPortEntryFailure) {
  constexpr char ERROR_MESSAGE[] = "sendWriteRequest failed";

  struct src_port_info port_info = {0};
  InitPortInfo(port_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status =
      WriteVsiSrcPortTableEntry(client, port_info, p4info, INSERT_ENTRY);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE)
      << status.message();
}

TEST_F(WriteVsiSrcPortEntryTest, writeVsiSrcPortEntrySuccess) {
  struct src_port_info port_info = {0};
  InitPortInfo(port_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest).WillOnce(Return(absl::OkStatus()));

  auto status =
      WriteVsiSrcPortTableEntry(client, port_info, p4info, INSERT_ENTRY);

  ASSERT_TRUE(status.ok()) << status;
}

}  // namespace ovsp4rt
