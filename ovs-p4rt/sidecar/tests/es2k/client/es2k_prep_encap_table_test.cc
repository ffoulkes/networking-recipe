// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "base_tunnel_info_test.h"
#include "es2k/p4_name_mapping.h"
#include "ovsp4rt_config_int.h"

namespace ovsp4rt {

class Es2kPrepEncapTableTest : public BaseTunnelInfoTest {
 public:
  Es2kPrepEncapTableTest() {}
  virtual ~Es2kPrepEncapTableTest() = default;

  static void AssertV4VxlanUntagged(const p4::v1::TableEntry& table_entry,
                                    const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, VXLAN_ENCAP_VLAN_POP_MOD_TABLE);
  }

  static void AssertV4GeneveUntagged(const p4::v1::TableEntry& table_entry,
                                     const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, GENEVE_ENCAP_VLAN_POP_MOD_TABLE);
  }

  static void AssertV4VxlanTagged(const p4::v1::TableEntry& table_entry,
                                  const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, VXLAN_ENCAP_MOD_TABLE);
  }

  static void AssertV4GeneveTagged(const p4::v1::TableEntry& table_entry,
                                   const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, GENEVE_ENCAP_MOD_TABLE);
  }

  static void AssertV6VxlanUntagged(const p4::v1::TableEntry& table_entry,
                                    const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE);
  }

  static void AssertV6GeneveUntagged(const p4::v1::TableEntry& table_entry,
                                     const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE);
  }

  static void AssertV6VxlanTagged(const p4::v1::TableEntry& table_entry,
                                  const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, VXLAN_ENCAP_V6_MOD_TABLE);
  }

  static void AssertV6GeneveTagged(const p4::v1::TableEntry& table_entry,
                                   const ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, GENEVE_ENCAP_V6_MOD_TABLE);
  }
};

//----------------------------------------------------------------------

TEST_F(Es2kPrepEncapTableTest, configEncapIpv4VxlanUntagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitVxlanUntagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV4VxlanUntagged(table_entry, p4info);
}

TEST_F(Es2kPrepEncapTableTest, configEncapIpv4GeneveUntagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitGeneveUntagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV4GeneveUntagged(table_entry, p4info);
}

//----------------------------------------------------------------------

TEST_F(Es2kPrepEncapTableTest, configEncapIpv4VxlanTagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV4VxlanTagged(table_entry, p4info);
}

TEST_F(Es2kPrepEncapTableTest, configEncapIpv4GeneveTagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitGeneveTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV4GeneveTagged(table_entry, p4info);
}

//----------------------------------------------------------------------

TEST_F(Es2kPrepEncapTableTest, configEncapIpv6VxlanUntagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV6TunnelInfo(tunnel_info);
  InitVxlanUntagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV6VxlanUntagged(table_entry, p4info);
}

TEST_F(Es2kPrepEncapTableTest, configEncapIpv6GeneveUntagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV6TunnelInfo(tunnel_info);
  InitGeneveUntagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV6GeneveUntagged(table_entry, p4info);
}

//----------------------------------------------------------------------

TEST_F(Es2kPrepEncapTableTest, configEncapIpv6VxlanTagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV6TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV6VxlanTagged(table_entry, p4info);
}

TEST_F(Es2kPrepEncapTableTest, configEncapIpv6GeneveTagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV6TunnelInfo(tunnel_info);
  InitGeneveTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  AssertV6GeneveTagged(table_entry, p4info);
}

}  // namespace ovsp4rt
