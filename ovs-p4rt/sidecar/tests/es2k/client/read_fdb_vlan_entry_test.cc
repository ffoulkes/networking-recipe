// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ReadFdbVlanTableEntry().

// Core functionality is handled by EncodeFdbTxVlanTableEntry(),
// which is tested separately. This is a test of the non-core
// functionality.

#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <arpa/inet.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "mac_learn_info_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"

using ::testing::InvokeWithoutArgs;
using ::testing::Return;

namespace ovsp4rt {

class ReadFdbVlanEntryTest : public MacLearnInfoTest {
 public:
  ReadFdbVlanEntryTest() {}
  virtual ~ReadFdbVlanEntryTest() = default;

  static void InitFdbInfo(struct mac_learning_info& fdb_info) {
    constexpr uint8_t MAC_ADDR[] = {0xde, 0xad, 0xbe, 0xef, 0x00, 0xe};
    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
  }

  static void InitTunnelInfo(struct mac_learning_info& fdb_info) {
    constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
    constexpr int IPV4_PREFIX_LEN = 24;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR,
                        &fdb_info.tnl_info.remote_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_DST_ADDR;
    fdb_info.tnl_info.remote_ip.family = AF_INET;
    fdb_info.tnl_info.remote_ip.prefix_len = IPV4_PREFIX_LEN;
  }

  static absl::StatusOr<::p4::v1::ReadResponse> GoodReadResponse() {
    ::p4::v1::ReadResponse response;
    return response;
  }
};

TEST_F(ReadFdbVlanEntryTest, readFdbVlanEntryFailure) {
  constexpr char REQUEST_FAILED[] = "sendReadRequest failed";
  struct mac_learning_info learn_info = {0};
  InitFdbInfo(learn_info);
  InitTunnelInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(Return(absl::NotFoundError(REQUEST_FAILED)));

  auto response = ReadFdbVlanTableEntry(client, learn_info, p4info);
  auto status = response.status();

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsNotFound(status) && status.message() == REQUEST_FAILED);
}

TEST_F(ReadFdbVlanEntryTest, readFdbVlanEntrySuccess) {
  struct mac_learning_info learn_info = {0};
  InitFdbInfo(learn_info);
  InitTunnelInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(InvokeWithoutArgs(GoodReadResponse));

  auto response = ReadFdbVlanTableEntry(client, learn_info, p4info);

  ASSERT_TRUE(response.ok()) << response.status();
}

}  // namespace ovsp4rt
