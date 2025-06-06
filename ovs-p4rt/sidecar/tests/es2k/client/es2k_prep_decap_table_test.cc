// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "core/common/ovsp4rt_entry_utils.h"
#include "core/es2k/p4_name_mapping.h"
#include "tunnel_info_test.h"

namespace ovsp4rt {

class Es2kPrepDecapTableTest : public TunnelInfoTest {
 public:
  Es2kPrepDecapTableTest() {}
  virtual ~Es2kPrepDecapTableTest() = default;

  static void AssertV4VxlanTagged(const p4::v1::TableEntry& table_entry,
                                  const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, VXLAN_DECAP_MOD_TABLE);
  }

  static void AssertV4VxlanUntagged(const p4::v1::TableEntry& table_entry,
                                    const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, VXLAN_DECAP_AND_VLAN_PUSH_MOD_TABLE);
  }

  static void AssertV4GeneveTagged(const p4::v1::TableEntry& table_entry,
                                   const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, GENEVE_DECAP_MOD_TABLE);
  }

  static void AssertV4GeneveUntagged(const p4::v1::TableEntry& table_entry,
                                     const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, GENEVE_DECAP_AND_VLAN_PUSH_MOD_TABLE);
  }
};

// Es2kPrepareDecapTableEntry switches on port_vlan_mode.
// IP version and tunnel type are irrelevant.

TEST_F(Es2kPrepDecapTableTest, prepareDecapIpv4VxlanTagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  // tunnel_type == OVS_TUNNEL_VXLAN
  // port_vlan_mode == P4_PORT_VLAN_NATIVE_TAGGED
  Es2kPrepareDecapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV4VxlanTagged(table_entry, p4info);
}

TEST_F(Es2kPrepDecapTableTest, prepareDecapIpv4VxlanUntagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitVxlanUntagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  // tunnel_type == OVS_TUNNEL_VXLAN
  // port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED
  Es2kPrepareDecapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV4VxlanUntagged(table_entry, p4info);
}

TEST_F(Es2kPrepDecapTableTest, prepareDecapGeneveTagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitGeneveTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  // tunnel_type == OVS_TUNNEL_GENEVE
  // port_vlan_mode == P4_PORT_VLAN_NATIVE_TAGGED
  Es2kPrepareDecapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV4GeneveTagged(table_entry, p4info);
}

TEST_F(Es2kPrepDecapTableTest, prepareDecapGeneveUntagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitGeneveUntagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  // tunnel_type == OVS_TUNNEL_GENEVE
  // port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED
  Es2kPrepareDecapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV4GeneveUntagged(table_entry, p4info);
}

}  // namespace ovsp4rt
