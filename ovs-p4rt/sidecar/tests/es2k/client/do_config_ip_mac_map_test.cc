// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Minimal unit test for DoConfigIpMacMapEntry().

#include <absl/status/status.h>
#include <arpa/inet.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"
#include "ovsp4rt_do_config_int.h"
#include "p4/config/v1/p4info.pb.h"
#include "tests/base/basic_test.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace ovsp4rt {

constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class DoConfigIpMacMapTest : public BasicTest {
 protected:
  DoConfigIpMacMapTest() {}
  virtual ~DoConfigIpMacMapTest() = default;

  void InitIpv4MapInfo(struct ip_mac_map_info& map_info) {
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

  static absl::StatusOr<::p4::v1::ReadResponse> FoundReadRequest() {
    ::p4::v1::ReadResponse response;
    return response;
  }

  static absl::StatusOr<::p4::v1::ReadResponse> NotFoundReadRequest() {
    return absl::NotFoundError("Entry not found");
  }
};

/**
 * Exercises client.connect() error path.
 */
TEST_F(DoConfigIpMacMapTest, connectFailure) {
  constexpr char CONNECT_ERROR[] = "connect";

  const struct ip_mac_map_info map_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect)
      .WillOnce(Return(absl::InternalError(CONNECT_ERROR)));

  auto status =
      DoConfigIpMacMapEntry(client, map_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == CONNECT_ERROR)
      << status;
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(DoConfigIpMacMapTest, getPipelineConfigFailure) {
  constexpr char PIPELINE_ERROR[] = "getPipelineConfig";

  const struct ip_mac_map_info map_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError(PIPELINE_ERROR)));

  auto status =
      DoConfigIpMacMapEntry(client, map_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == PIPELINE_ERROR)
      << status;
}

}  // namespace ovsp4rt
