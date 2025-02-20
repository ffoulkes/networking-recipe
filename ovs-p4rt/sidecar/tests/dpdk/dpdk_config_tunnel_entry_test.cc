// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for DPDK version of DoConfigFdbEntry().

#include <absl/status/status.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_doconfig_int.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class DpdkConfigTunnelEntryTest : public ::testing::Test {
 protected:
  DpdkConfigTunnelEntryTest() {}
  ~DpdkConfigTunnelEntryTest() = default;
};

//----------------------------------------------------------------------
// Setup test cases
//----------------------------------------------------------------------

/**
 * Exercises client.connect() error path.
 */
TEST_F(DpdkConfigTunnelEntryTest, connectFailure) {
  struct tunnel_info tunnel_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::InternalError("connect")));

  auto status =
      DoConfigTunnelEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "connect")
      << status.message();
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(DpdkConfigTunnelEntryTest, getPipelineConfigFailure) {
  struct tunnel_info tunnel_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError("getPipelineConfig")));

  auto status =
      DoConfigTunnelEntry(client, tunnel_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "getPipelineConfig")
      << status.message();
}

}  // namespace ovsp4rt
