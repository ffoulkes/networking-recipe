// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for ES2K version of DoConfigFdbEntry().

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

class Es2kConfigFdbEntryTest : public ::testing::Test {
 protected:
  Es2kConfigFdbEntryTest() {}
  ~Es2kConfigFdbEntryTest() = default;
};

//----------------------------------------------------------------------
// Setup test cases
//----------------------------------------------------------------------

/**
 * Exercises client.connect() error path.
 */
TEST_F(Es2kConfigFdbEntryTest, connectFailure) {
  constexpr char CONNECT_ERROR[] = "connect";

  struct mac_learning_info learn_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect)
      .WillOnce(Return(absl::InternalError(CONNECT_ERROR)));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == CONNECT_ERROR)
      << status.message();
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(Es2kConfigFdbEntryTest, getPipelineConfigFailure) {
  constexpr char PIPELINE_ERROR[] = "getPipelineConfig";

  struct mac_learning_info learn_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError(PIPELINE_ERROR)));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == PIPELINE_ERROR)
      << status.message();
}

}  // namespace ovsp4rt
