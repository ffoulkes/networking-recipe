// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ConfigFdbUpdateTunnelInfo() [ES2K]

#include <absl/status/status.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

using ::testing::InvokeWithoutArgs;
using ::testing::Return;

namespace ovsp4rt {

constexpr bool DELETE_ENTRY = false;
constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class Es2kUpdateTunnelInfoTest : public ::testing::Test {
 protected:
  Es2kUpdateTunnelInfoTest() {}
  virtual ~Es2kUpdateTunnelInfoTest() = default;

  // The UUT is called when deleting an entry.
  // The only field required is the mac address.
  void InitLearnInfo(struct mac_learning_info& learn_info) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    memcpy(learn_info.mac_addr, MAC_ADDR, sizeof(learn_info.mac_addr));
  }

  void InitP4Info(::p4::config::v1::P4Info* p4info) {
    auto status = stratum::ParseProtoFromString(P4INFO_TEXT, p4info);
    EXPECT_TRUE(status.ok())
        << "ParseProtoFromString: " << status.error_message();
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
TEST_F(Es2kUpdateTunnelInfoTest, EntryInIpv4TunnelTable) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(InvokeWithoutArgs(ReturnReadResponse));

  ConfigFdbUpdateTunnelInfo(client, learn_info, expected_p4info);

  ASSERT_TRUE(learn_info.is_tunnel);
}

/**
 * Entry is in the IPv6 tunnel table.
 */
TEST_F(Es2kUpdateTunnelInfoTest, EntryInIpv6TunnelTable) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError("not in IPv4 tunnel table")))
      .WillOnce(InvokeWithoutArgs(ReturnReadResponse));

  ConfigFdbUpdateTunnelInfo(client, learn_info, expected_p4info);

  ASSERT_TRUE(learn_info.is_tunnel);
}

/**
 * Entry is not in either tunnel table.
 */
TEST_F(Es2kUpdateTunnelInfoTest, EntryNotInTunnelTables) {
  struct mac_learning_info learn_info = {0};
  InitLearnInfo(learn_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError("not in IPv4 tunnel table")))
      .WillOnce(Return(absl::NotFoundError("not in IPv6 tunnel table")));

  ConfigFdbUpdateTunnelInfo(client, learn_info, expected_p4info);

  ASSERT_FALSE(learn_info.is_tunnel);
}

}  // namespace ovsp4rt
