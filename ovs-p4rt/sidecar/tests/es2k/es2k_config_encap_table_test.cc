// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "base_tunnel_info_test.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class Es2kConfigEncapTableTest : public BaseTunnelInfoTest {
 public:
  Es2kConfigEncapTableTest() {}
  virtual ~Es2kConfigEncapTableTest() = default;
};

//----------------------------------------------------------------------

// AF_INET && P4_PORT_VLAN_NATIVE_UNTAGGED && OVS_TUNNEL_VXLAN
TEST_F(Es2kConfigEncapTableTest, configEncapIpv4VxlanUntagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitVxlanUntagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  CheckV4TunnelInfo(table_entry);
  CheckVxlanUntagged(table_entry);
}

// AF_INET && P4_PORT_VLAN_NATIVE_UNTAGGED && OVS_TUNNEL_GENEVE
TEST_F(Es2kConfigEncapTableTest, configEncapIpv4GeneveUntagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitGeneveUntagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  CheckV4TunnelInfo(table_entry);
  CheckGeneveUntagged(table_entry);
}

//----------------------------------------------------------------------

// AF_INET && P4_PORT_VLAN_NATIVE_TAGGED && OVS_TUNNEL_VXLAN
TEST_F(Es2kConfigEncapTableTest, configEncapIpv4VxlanTagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  CheckV4TunnelInfo(table_entry);
  CheckVxlanTagged(table_entry);
}

// AF_INET && P4_PORT_VLAN_NATIVE_TAGGED && OVS_TUNNEL_GENEVE
TEST_F(Es2kConfigEncapTableTest, configEncapIpv4GeneveTagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV4TunnelInfo(tunnel_info);
  InitGeneveTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  CheckV4TunnelInfo(table_entry);
  CheckGeneveTagged(table_entry);
}

//----------------------------------------------------------------------

// AF_INET6 && P4_PORT_VLAN_NATIVE_UNTAGGED && OVS_TUNNEL_VXLAN
TEST_F(Es2kConfigEncapTableTest, configEncapIpv6VxlanUntagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV6TunnelInfo(tunnel_info);
  InitVxlanUntagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  CheckV6TunnelInfo(table_entry);
  CheckVxlanUntagged(table_entry);
}

// AF_INET6 && P4_PORT_VLAN_NATIVE_UNTAGGED && OVS_TUNNEL_GENEVE
TEST_F(Es2kConfigEncapTableTest, configEncapIpv6GeneveUntagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV6TunnelInfo(tunnel_info);
  InitGeneveUntagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  CheckV6TunnelInfo(table_entry);
  CheckGeneveUntagged(table_entry);
}

//----------------------------------------------------------------------

// AF_INET6 && P4_PORT_VLAN_NATIVE_TAGGED && OVS_TUNNEL_VXLAN
TEST_F(Es2kConfigEncapTableTest, configEncapIpv6VxlanTagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV6TunnelInfo(tunnel_info);
  InitVxlanTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  CheckV6TunnelInfo(table_entry);
  CheckVxlanTagged(table_entry);
}

// AF_INET6 && P4_PORT_VLAN_NATIVE_TAGGED && OVS_TUNNEL_GENEVE
TEST_F(Es2kConfigEncapTableTest, configEncapIpv6GeneveTagged) {
  p4::v1::TableEntry table_entry;

  struct tunnel_info tunnel_info = {0};
  InitV6TunnelInfo(tunnel_info);
  InitGeneveTagged(tunnel_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  CheckV6TunnelInfo(table_entry);
  CheckGeneveTagged(table_entry);
}

}  // namespace ovsp4rt
