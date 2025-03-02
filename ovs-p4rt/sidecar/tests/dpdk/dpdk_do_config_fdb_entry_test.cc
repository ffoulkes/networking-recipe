// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Minimal unit test for DoConfigFdbEntry().

#include <absl/status/status.h>

// clang-format off
#include <gtest/gtest.h>
#include <gmock/gmock.h>
// clang-format on

#include "client/ovsp4rt_test_client_mock.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_do_config_int.h"
#include "p4/config/v1/p4info.pb.h"
#include "tests/base/basic_test.h"

using ::testing::Return;

namespace ovsp4rt {

constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class DpdkDoConfigFdbEntryTest : public BasicTest {
 protected:
  DpdkDoConfigFdbEntryTest() {}
  ~DpdkDoConfigFdbEntryTest() = default;
};

/**
 * Exercises client.connect() error path.
 */
TEST_F(DpdkDoConfigFdbEntryTest, connectFailure) {
  struct mac_learning_info learn_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::InternalError("connect")));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "connect") << status;
}

/**
 * Exercises config.getPipelineConfig() error path.
 */
TEST_F(DpdkDoConfigFdbEntryTest, getPipelineConfigFailure) {
  struct mac_learning_info learn_info = {0};

  TestClientMock client;
  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError("getPipelineConfig")));

  auto status = DoConfigFdbEntry(client, learn_info, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
  ASSERT_TRUE(IsInternal(status) && status.message() == "getPipelineConfig")
      << status;
}

}  // namespace ovsp4rt
