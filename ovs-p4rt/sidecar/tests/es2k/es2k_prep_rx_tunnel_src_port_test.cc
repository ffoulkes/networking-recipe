// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareRxTunnelSrcPortTableEntry().

#include <gtest/gtest.h>

#include "base_tunnel_info_test.h"
#include "es2k/p4_name_mapping.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class Es2kPrepRxTunnelSrcPortTest : public BaseTunnelInfoTest {
 public:
  Es2kPrepRxTunnelSrcPortTest() {}
  virtual ~Es2kPrepRxTunnelSrcPortTest() = default;

  static void AssertV4Table(const p4::v1::TableEntry& table_entry,
                             const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, RX_IPV4_TUNNEL_SOURCE_PORT_TABLE);
  }

  static void AssertV6Table(const p4::v1::TableEntry& table_entry,
                             const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, RX_IPV6_TUNNEL_SOURCE_PORT_TABLE);
  }
};

TEST_F(Es2kPrepRxTunnelSrcPortTest, configV4TunnelTermEntry) {
  ::p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  PrepareRxTunnelSrcPortTableEntry(&table_entry, tunnel_info, p4info,
                                   INSERT_ENTRY);

  AssertV4Table(table_entry, p4info);
}

TEST_F(Es2kPrepRxTunnelSrcPortTest, configV6TunnelTermEntry) {
  ::p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV6TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  PrepareRxTunnelSrcPortTableEntry(&table_entry, tunnel_info, p4info,
                                   INSERT_ENTRY);

  AssertV6Table(table_entry, p4info);
}

}  // namespace ovsp4rt
