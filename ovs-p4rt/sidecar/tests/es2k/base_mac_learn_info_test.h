// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef BASE_MAC_LEARN_INFO_TEST_H_
#define BASE_MAC_LEARN_INFO_TEST_H_

#include <gtest/gtest.h>

#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

class BaseMacLearnInfoTest : public ::testing::Test {
 protected:
  BaseMacLearnInfoTest() {}
  virtual ~BaseMacLearnInfoTest() = default;

  static void InitV4NativeTagged(struct mac_learning_info& fdb_info);
  static void InitV4NativeUntagged(struct mac_learning_info& fdb_info);
  static void InitV6NativeTagged(struct mac_learning_info& fdb_info);
  static void InitV6NativeUntagged(struct mac_learning_info& fdb_info);

  static void InitP4Info(::p4::config::v1::P4Info* p4info);
};

}  // namespace ovsp4rt

#endif  // BASE_MAC_LEARN_INFO_TEST_H_
