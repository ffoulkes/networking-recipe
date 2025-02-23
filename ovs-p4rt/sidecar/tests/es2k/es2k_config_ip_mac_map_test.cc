// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for DoConfigIpMacMapEntry() [ES2K]

#include <absl/status/status.h>
#include <arpa/inet.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"
#include "ovsp4rt_doconfig_int.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class Es2kConfigIpMacMapTest : public ::testing::Test {
 protected:
  Es2kConfigIpMacMapTest() {}
  virtual ~Es2kConfigIpMacMapTest() = default;

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

  void InitP4Info(::p4::config::v1::P4Info* p4info) {
    auto status = stratum::ParseProtoFromString(P4INFO_TEXT, p4info);
    EXPECT_TRUE(status.ok())
        << "ParseProtoFromString: " << status.error_message();
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
TEST_F(Es2kConfigIpMacMapTest, connectFailure) {
  constexpr char CONNECT_ERROR[] = "connect";

  const struct ip_mac_map_info map_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect)
      .WillOnce(Return(absl::InternalError(CONNECT_ERROR)));

  auto status =
      DoConfigIpMacMapEntry(client, map_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == CONNECT_ERROR)
      << status.message();
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(Es2kConfigIpMacMapTest, getPipelineConfigFailure) {
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
      << status.message();
}

/**
 * Exercises the GetVmSrcTableEntry() both-found path.
 */
TEST_F(Es2kConfigIpMacMapTest, getVmSrcTableBothFound) {
  struct ip_mac_map_info map_info = {0};
  InitIpv4MapInfo(map_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(InvokeWithoutArgs(FoundReadRequest))
      .WillOnce(InvokeWithoutArgs(FoundReadRequest));

  auto status =
      DoConfigIpMacMapEntry(client, map_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_TRUE(status.ok()) << status;
}

/**
 * Exercises the GetVmSrcTableEntry() neither-found path.
 */
TEST_F(Es2kConfigIpMacMapTest, getVmSrcTableNeitherFound) {
  struct ip_mac_map_info map_info = {0};
  InitIpv4MapInfo(map_info);

  ::p4::config::v1::P4Info expected_p4info;
  InitP4Info(&expected_p4info);

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendReadRequest)
      .WillOnce(InvokeWithoutArgs(NotFoundReadRequest))
      .WillOnce(InvokeWithoutArgs(NotFoundReadRequest));
  EXPECT_CALL(client, sendWriteRequest)
      .WillRepeatedly(Return(absl::OkStatus()));

  auto status =
      DoConfigIpMacMapEntry(client, map_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_TRUE(status.ok()) << status;
}

/**
 * Exercises the ConfigSrcIpMacMapTableEntry() failure path.
 */
TEST_F(Es2kConfigIpMacMapTest, ConfigSrcIpMacMapTableEntryFailure) {
  constexpr char ERROR_MESSAGE[] = "sendReadRequest";

  struct ip_mac_map_info map_info = {0};
  InitIpv4MapInfo(map_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status =
      ConfigSrcIpMacMapTableEntry(client, map_info, p4info, INSERT_ENTRY);

  EXPECT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE);
}

/**
 * Exercises the ConfigDstIpMacMapTableEntry() failure path.
 */
TEST_F(Es2kConfigIpMacMapTest, configDstIpMacMapTableEntryFailure) {
  constexpr char ERROR_MESSAGE[] = "sendReadRequest";

  struct ip_mac_map_info map_info = {0};
  InitIpv4MapInfo(map_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  TestClientMock client;
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError(ERROR_MESSAGE)));

  auto status =
      ConfigDstIpMacMapTableEntry(client, map_info, p4info, INSERT_ENTRY);

  EXPECT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == ERROR_MESSAGE);
}

}  // namespace ovsp4rt
