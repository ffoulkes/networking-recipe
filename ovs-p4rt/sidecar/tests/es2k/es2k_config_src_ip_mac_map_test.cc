// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ConfigSrcIpMacMapTableEntry(). [es2k]

#include <absl/status/status.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "base_mac_map_info_test.h"
#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"

using ::testing::Return;

namespace ovsp4rt {

class Es2kConfigSrcIpMacMapTest : public BaseMacMapInfoTest {
 protected:
  Es2kConfigSrcIpMacMapTest() {}
  virtual ~Es2kConfigSrcIpMacMapTest() = default;
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
