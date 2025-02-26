// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef BASE_TUNNEL_INFO_TEST_H_
#define BASE_TUNNEL_INFO_TEST_H_

#include <gtest/gtest.h>

#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

class BaseTunnelInfoTest : public ::testing::Test {
 protected:
  BaseTunnelInfoTest(){};
  virtual ~BaseTunnelInfoTest() = default;

  static void InitV4TunnelInfo(struct tunnel_info& tunnel_info);
  static void InitV6TunnelInfo(struct tunnel_info& tunnel_info);

  static void InitVxlanTagged(struct tunnel_info& tunnel_info);
  static void InitVxlanUntagged(struct tunnel_info& tunnel_info);

  static void InitGeneveTagged(struct tunnel_info& tunnel_info);
  static void InitGeneveUntagged(struct tunnel_info& tunnel_info);

  static void InitP4Info(::p4::config::v1::P4Info* p4info);

  static void AssertTableId(const p4::v1::TableEntry& table_entry,
                            const ::p4::config::v1::P4Info& p4info,
                            const char* table_name);
};

}  // namespace ovsp4rt

#endif  // BASE_TUNNEL_INFO_TEST_H_
