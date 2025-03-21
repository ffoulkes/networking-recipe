// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef BASE_MAC_LEARN_INFO_TEST_H_
#define BASE_MAC_LEARN_INFO_TEST_H_

#include <gtest/gtest.h>

#include "basic_test.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

class MacLearnInfoTest : public BasicTest {
 protected:
  MacLearnInfoTest() {}
  virtual ~MacLearnInfoTest() = default;

  static void InitV4TunnelInfo(struct mac_learning_info& fdb_info);
  static void InitV6TunnelInfo(struct mac_learning_info& fdb_info);

  static void InitV4NativeTagged(struct mac_learning_info& fdb_info);
  static void InitV4NativeUntagged(struct mac_learning_info& fdb_info);
  static void InitV6NativeTagged(struct mac_learning_info& fdb_info);
  static void InitV6NativeUntagged(struct mac_learning_info& fdb_info);

  static void InitVxlanTagged(struct mac_learning_info& fdb_info);
  static void InitVxlanUntagged(struct mac_learning_info& fdb_info);
  static void InitGeneveTagged(struct mac_learning_info& fdb_info);
  static void InitGeneveUntagged(struct mac_learning_info& fdb_info);

  static void AssertTableId(const p4::v1::TableEntry& table_entry,
                            const ::p4::config::v1::P4Info& p4info,
                            const char* table_name);
};

}  // namespace ovsp4rt

#endif  // BASE_MAC_LEARN_INFO_TEST_H_
