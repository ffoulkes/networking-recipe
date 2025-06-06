// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for WriteFdbTunnelTableEntry().

// Core functionality is handled by EncodeFdbTableEntryforV4VxlanTunnel()
// or EncodeFdbTableEntryforV4GeneveTunnel, which are tested separately.
// This is a test of the non-core functionality.

#include <absl/status/status.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "client/ovsp4rt_test_client_mock.h"
#include "core/common/ovsp4rt_entry_utils.h"
#include "mac_learn_info_test.h"
#include "ovsp4rt/ovs-p4rt.h"

using ::testing::Return;

namespace ovsp4rt {

class WriteFdbTunnelTableEntryTest : public MacLearnInfoTest {
 public:
  WriteFdbTunnelTableEntryTest() {}
  virtual ~WriteFdbTunnelTableEntryTest() = default;

  static void InitLearnInfo(struct mac_learning_info& fdb_info) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 42;

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
    fdb_info.bridge_id = BRIDGE_ID;
  }
};

TEST_F(WriteFdbTunnelTableEntryTest, writeFdbTunnelEntryFailure) {
  constexpr char ERROR_MESSAGE[] = "sendWriteRequest failed";

  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);
  InitV4NativeTagged(learn_info);
  learn_info.tnl_info.tunnel_type = OVS_TUNNEL_VXLAN;

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status =
      WriteFdbTunnelTableEntry(client, learn_info, p4info, INSERT_ENTRY);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE)
      << status.message();
}

TEST_F(WriteFdbTunnelTableEntryTest, writeFdbTunnelEntrySuccess) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);
  InitV4NativeTagged(learn_info);
  learn_info.tnl_info.tunnel_type = OVS_TUNNEL_VXLAN;

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest).WillOnce(Return(absl::OkStatus()));

  auto status =
      WriteFdbTunnelTableEntry(client, learn_info, p4info, INSERT_ENTRY);

  ASSERT_TRUE(status.ok()) << status;
}

}  // namespace ovsp4rt
