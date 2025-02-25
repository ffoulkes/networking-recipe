// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ConfigSrcIpMacMapTableEntry(). [es2k]

#include <absl/status/status.h>
#include <arpa/inet.h>
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

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

class Es2kConfigSrcIpMacMapTest : public ::testing::Test {
 protected:
  Es2kConfigSrcIpMacMapTest() {}
  virtual ~Es2kConfigSrcIpMacMapTest() = default;

  void InitIpv4MapInfo(struct ip_mac_map_info map_info) {
    constexpr uint8_t SRC_MAC[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr char IPV4_SRC_ADDR[] = "10.20.30.40";
    constexpr int IPV4_PREFIX_LEN = 24;

    memcpy(map_info.src_mac_addr, SRC_MAC, sizeof(map_info.src_mac_addr));

    EXPECT_EQ(inet_pton(AF_INET, IPV4_SRC_ADDR,
                        &map_info.src_ip_addr.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_SRC_ADDR;
    map_info.src_ip_addr.family = AF_INET;
    map_info.src_ip_addr.prefix_len = IPV4_PREFIX_LEN;
  }

  void InitP4Info(::p4::config::v1::P4Info* p4info) {
    auto status = stratum::ParseProtoFromString(P4INFO_TEXT, p4info);
    EXPECT_TRUE(status.ok())
        << "ParseProtoFromString: " << status.error_message();
  }
};

TEST_F(Es2kConfigSrcIpMacMapTest, configSrcIpMacMapWriteFailure) {
  constexpr char ERROR_MESSAGE[] = "sendWriteRequest failed";

  struct ip_mac_map_info map_info = {0};
  InitIpv4MapInfo(map_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status =
      ConfigSrcIpMacMapTableEntry(client, map_info, p4info, INSERT_ENTRY);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE)
      << status;
}

TEST_F(Es2kConfigSrcIpMacMapTest, configSrcIpMacMapWriteSuccess) {
  struct ip_mac_map_info map_info = {0};
  InitIpv4MapInfo(map_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest).WillOnce(Return(absl::OkStatus()));

  auto status =
      ConfigSrcIpMacMapTableEntry(client, map_info, p4info, REMOVE_ENTRY);

  ASSERT_TRUE(status.ok()) << status;
}

}  // namespace ovsp4rt
