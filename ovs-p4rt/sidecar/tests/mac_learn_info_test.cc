// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "mac_learn_info_test.h"

#include <absl/status/status.h>
#include <arpa/inet.h>
#include <gtest/gtest.h>

#include "core/ovsp4rt_util_int.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace ovsp4rt {

void MacLearnInfoTest::InitV4TunnelInfo(struct mac_learning_info& fdb_info) {
  constexpr char IPV4_SRC_ADDR[] = "10.20.30.40";
  constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
  constexpr int IPV4_PREFIX_LEN = 24;

  constexpr uint16_t SRC_PORT = 0x1066;
  constexpr uint16_t DST_PORT = 0x4224;
  constexpr uint16_t VNI = 0x1776;

  tunnel_info& tnl_info = fdb_info.tnl_info;

  EXPECT_EQ(
      inet_pton(AF_INET, IPV4_SRC_ADDR, &tnl_info.local_ip.ip.v4addr.s_addr), 1)
      << "Error converting " << IPV4_SRC_ADDR;
  tnl_info.local_ip.prefix_len = IPV4_PREFIX_LEN;
  tnl_info.local_ip.family = AF_INET;

  EXPECT_EQ(
      inet_pton(AF_INET, IPV4_DST_ADDR, &tnl_info.remote_ip.ip.v4addr.s_addr),
      1)
      << "Error converting " << IPV4_DST_ADDR;
  tnl_info.remote_ip.prefix_len = IPV4_PREFIX_LEN;
  tnl_info.remote_ip.family = AF_INET;

  tnl_info.src_port = SRC_PORT;
  tnl_info.dst_port = DST_PORT;
  tnl_info.vni = VNI;
};

void MacLearnInfoTest::InitV6TunnelInfo(struct mac_learning_info& fdb_info) {
  constexpr char IPV6_SRC_ADDR[] = "fe80::215:5dff:fefa";
  constexpr char IPV6_DST_ADDR[] = "fe80::215:192.168.17.5";
  constexpr int IPV6_PREFIX_LEN = 64;

  constexpr uint16_t SRC_PORT = 0x1984;
  constexpr uint16_t DST_PORT = 0x4224;
  constexpr uint16_t VNI = 0x1066;

  tunnel_info& tnl_info = fdb_info.tnl_info;

  EXPECT_EQ(inet_pton(AF_INET6, IPV6_SRC_ADDR,
                      &tnl_info.local_ip.ip.v6addr.__in6_u.__u6_addr32),
            1)
      << "Error converting " << IPV6_SRC_ADDR;
  tnl_info.local_ip.prefix_len = IPV6_PREFIX_LEN;
  tnl_info.local_ip.family = AF_INET6;

  EXPECT_EQ(inet_pton(AF_INET6, IPV6_DST_ADDR,
                      &tnl_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32),
            1)
      << "Error converting " << IPV6_DST_ADDR;
  tnl_info.remote_ip.prefix_len = IPV6_PREFIX_LEN;
  tnl_info.remote_ip.family = AF_INET6;

  tnl_info.src_port = SRC_PORT;
  tnl_info.dst_port = DST_PORT;
  tnl_info.vni = VNI;
};

void MacLearnInfoTest::InitV4NativeTagged(struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.local_ip.family = AF_INET;
  fdb_info.tnl_info.remote_ip.family = AF_INET;
  fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
  fdb_info.tnl_info.vni = 0x1984U;
}

void MacLearnInfoTest::InitV4NativeUntagged(
    struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.local_ip.family = AF_INET;
  fdb_info.tnl_info.remote_ip.family = AF_INET;
  fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
  fdb_info.tnl_info.vni = 0x1776U;
}

void MacLearnInfoTest::InitV6NativeTagged(struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.local_ip.family = AF_INET6;
  fdb_info.tnl_info.remote_ip.family = AF_INET6;
  fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
  fdb_info.tnl_info.vni = 0xFACEU;
}

void MacLearnInfoTest::InitV6NativeUntagged(
    struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.local_ip.family = AF_INET6;
  fdb_info.tnl_info.remote_ip.family = AF_INET6;
  fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
  fdb_info.tnl_info.vni = 0xCEDEU;
}

void MacLearnInfoTest::InitVxlanTagged(struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.tunnel_type = OVS_TUNNEL_VXLAN;
  fdb_info.tnl_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
  fdb_info.tnl_info.vni = 0x1066;
}

void MacLearnInfoTest::InitVxlanUntagged(struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.tunnel_type = OVS_TUNNEL_VXLAN;
  fdb_info.tnl_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
  fdb_info.tnl_info.vni = 0x1492;
}

void MacLearnInfoTest::InitGeneveTagged(struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.tunnel_type = OVS_TUNNEL_GENEVE;
  fdb_info.tnl_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
  fdb_info.tnl_info.vni = 0x1776;
}

void MacLearnInfoTest::InitGeneveUntagged(struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.tunnel_type = OVS_TUNNEL_GENEVE;
  fdb_info.tnl_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
  fdb_info.tnl_info.vni = 0x1984;
}

void MacLearnInfoTest::AssertTableId(const p4::v1::TableEntry& table_entry,
                                     const ::p4::config::v1::P4Info& p4info,
                                     const char* table_name) {
  auto expected_id = GetTableId(p4info, table_name);
  EXPECT_EQ(table_entry.table_id(), expected_id);
}

}  // namespace ovsp4rt
