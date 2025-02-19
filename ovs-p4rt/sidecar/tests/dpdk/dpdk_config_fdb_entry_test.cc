// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for DPDK version of DoConfigFdbEntry().

#include <absl/status/status.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "client/ovsp4rt_client_mock.h"
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

class DpdkConfigFdbEntryTest : public ::testing::Test {
 protected:
  DpdkConfigFdbEntryTest() {}
  ~DpdkConfigFdbEntryTest() = default;

  struct mac_learning_info learn_info_;
};

TEST_F(DpdkConfigFdbEntryTest, connectFailure) {
  ClientMock client;

  EXPECT_CALL(client, connect).WillOnce(Return(absl::InternalError("connect")));

  auto status = DoConfigFdbEntry(client, learn_info_, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
}

TEST_F(DpdkConfigFdbEntryTest, getPipelineConfigFailure) {
  ClientMock client;

  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(Return(absl::InternalError("getPipelineConfig")));

  auto status = DoConfigFdbEntry(client, learn_info_, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
}

#if false

TEST_F(DpdkConfigFdbEntryTest, configTunnelEntryFailure) {
  ClientMock client;
  ::p4::config::v1::P4Info expected_p4info;

  {
    // Note: return value is stratum::Status, not absl::Status.
    auto status = stratum::ParseProtoFromString(P4INFO_TEXT, &expected_p4info);
    ASSERT_TRUE(status.ok())
        << "Error parsing P4INFO_TEXT: " << status.error_message();
  }

  EXPECT_CALL(client, connect).WillOnce(Return(absl::OkStatus()));
  EXPECT_CALL(client, getPipelineConfig)
      .WillOnce(
          DoAll(SetArgPointee<0>(expected_p4info), Return(absl::OkStatus())));
  EXPECT_CALL(client, sendWriteRequest)
      .WillOnce(Return(absl::InternalError("sendWriteRequest")));

  auto status = DoConfigFdbEntry(client, learn_info_, INSERT_ENTRY, GRPC_ADDR);

  ASSERT_FALSE(status.ok());
}

TEST_F(DpdkConfigFdbEntryTest, configTunnelEntrySuccess) {
  ClientMock client;

  EXPECT_CALL(client, connect).WillByDefault(Return(absl::OkStatus()));

  auto status = DoConfigFdbEntry(client, learn_info_, INSERT_ENTRY, GRPC_ADDR);
}

TEST_F(DpdkConfigFdbEntryTest, configVlanTxEntryFailure) {
  ClientMock client;

  EXPECT_CALL(client, connect).WillByDefault(Return(absl::OkStatus()));

  auto status = DoConfigFdbEntry(client, learn_info_, INSERT_ENTRY, GRPC_ADDR);
}

TEST_F(DpdkConfigFdbEntryTest, configVlanTxEntrySuccess) {
  ClientMock client;

  EXPECT_CALL(client, connect).WillByDefault(Return(absl::OkStatus()));

  auto status = DoConfigFdbEntry(client, learn_info_, INSERT_ENTRY, GRPC_ADDR);
}

TEST_F(DpdkConfigFdbEntryTest, configVlanRxEntryFailure) {
  ClientMock client;

  EXPECT_CALL(client, connect).WillByDefault(Return(absl::OkStatus()));

  auto status = DoConfigFdbEntry(client, learn_info_, INSERT_ENTRY, GRPC_ADDR);
}

TEST_F(DpdkConfigFdbEntryTest, configVlanRxEntrySuccess) {
  ClientMock client;
  auto status = DoConfigFdbEntry(client, learn_info_, INSERT_ENTRY, GRPC_ADDR);
}

#endif  // false

}  // namespace ovsp4rt
