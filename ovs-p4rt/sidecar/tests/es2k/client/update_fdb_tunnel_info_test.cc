// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for UpdateFdbTunnelInfo() [ES2K]

#include <absl/status/status.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "basic_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"
#include "p4/config/v1/p4info.pb.h"

using ::testing::InvokeWithoutArgs;
using ::testing::Return;

namespace ovsp4rt {

constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class UpdateFdbTunnelInfoTest : public BasicTest {
 protected:
  UpdateFdbTunnelInfoTest() {}
  virtual ~UpdateFdbTunnelInfoTest() = default;

  // The UUT is called when deleting an entry.
  // The only field required is the mac address.
  void InitLearnInfo(struct mac_learning_info& learn_info) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    memcpy(learn_info.mac_addr, MAC_ADDR, sizeof(learn_info.mac_addr));
  }

  // The UUT only cares that it receives a response.
  // It never examines the return value.
  static absl::StatusOr<::p4::v1::ReadResponse> ReturnReadResponse() {
    ::p4::v1::ReadResponse response;
    return response;
  }
};

/**
 * Entry is in the IPv4 tunnel table.
 */
TEST_F(UpdateFdbTunnelInfoTest, entryInIpv4TunnelTable) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(InvokeWithoutArgs(ReturnReadResponse));

  UpdateFdbTunnelInfo(client, learn_info, expected_p4info);

  ASSERT_TRUE(learn_info.is_tunnel);
}

/**
 * Entry is in the IPv6 tunnel table.
 */
TEST_F(UpdateFdbTunnelInfoTest, entryInIpv6TunnelTable) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError("not in IPv4 tunnel table")))
      .WillOnce(InvokeWithoutArgs(ReturnReadResponse));

  UpdateFdbTunnelInfo(client, learn_info, expected_p4info);

  ASSERT_TRUE(learn_info.is_tunnel);
}

/**
 * Entry is not in either tunnel table.
 */
TEST_F(UpdateFdbTunnelInfoTest, entryNotInTunnelTables) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError("not in IPv4 tunnel table")))
      .WillOnce(Return(absl::NotFoundError("not in IPv6 tunnel table")));

  UpdateFdbTunnelInfo(client, learn_info, expected_p4info);

  ASSERT_FALSE(learn_info.is_tunnel);
}

}  // namespace ovsp4rt
