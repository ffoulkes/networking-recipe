// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareTunnelTermTableEntry().

#include <gtest/gtest.h>

#include "core/common/ovsp4rt_entry_utils.h"
#include "core/es2k/p4_name_mapping.h"
#include "tunnel_info_test.h"

namespace ovsp4rt {

class PrepTunnelTermEntryTest : public TunnelInfoTest {
 public:
  PrepTunnelTermEntryTest() {}
  virtual ~PrepTunnelTermEntryTest() = default;

  static void AssertV4Tunnel(const p4::v1::TableEntry& table_entry,
                             const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, IPV4_TUNNEL_TERM_TABLE);
  }

  static void AssertV6Tunnel(const p4::v1::TableEntry& table_entry,
                             const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, IPV6_TUNNEL_TERM_TABLE);
  }
};

TEST_F(PrepTunnelTermEntryTest, configV4TunnelTermEntry) {
  ::p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  PrepareTunnelTermTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV4Tunnel(table_entry, p4info);
}

TEST_F(PrepTunnelTermEntryTest, configV6TunnelTermEntry) {
  ::p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV6TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  PrepareTunnelTermTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV6Tunnel(table_entry, p4info);
}

}  // namespace ovsp4rt
