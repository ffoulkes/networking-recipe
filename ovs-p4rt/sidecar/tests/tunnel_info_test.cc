// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "tunnel_info_test.h"

#include <arpa/inet.h>
#include <gtest/gtest.h>

#include "core/common/ovsp4rt_core_utils.h"  // GetTableId
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

void TunnelInfoTest::InitV4TunnelInfo(struct tunnel_info& tunnel_info) {
  constexpr char IPV4_SRC_ADDR[] = "10.20.30.40";
  constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
  constexpr int IPV4_PREFIX_LEN = 24;

  constexpr uint16_t SRC_PORT = 0x1066;
  constexpr uint16_t DST_PORT = 0x4224;
  constexpr uint16_t VNI = 0x1776;

  EXPECT_EQ(
      inet_pton(AF_INET, IPV4_SRC_ADDR, &tunnel_info.local_ip.ip.v4addr.s_addr),
      1)
      << "Error converting " << IPV4_SRC_ADDR;
  tunnel_info.local_ip.prefix_len = IPV4_PREFIX_LEN;
  tunnel_info.local_ip.family = AF_INET;

  EXPECT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR,
                      &tunnel_info.remote_ip.ip.v4addr.s_addr),
            1)
      << "Error converting " << IPV4_DST_ADDR;
  tunnel_info.remote_ip.prefix_len = IPV4_PREFIX_LEN;
  tunnel_info.remote_ip.family = AF_INET;

  tunnel_info.src_port = SRC_PORT;
  tunnel_info.dst_port = DST_PORT;
  tunnel_info.vni = VNI;
};

void TunnelInfoTest::InitV6TunnelInfo(struct tunnel_info& tunnel_info) {
  constexpr char IPV6_SRC_ADDR[] = "fe80::215:5dff:fefa";
  constexpr char IPV6_DST_ADDR[] = "fe80::215:192.168.17.5";
  constexpr int IPV6_PREFIX_LEN = 64;

  constexpr uint16_t SRC_PORT = 0x1984;
  constexpr uint16_t DST_PORT = 0x4224;
  constexpr uint16_t VNI = 0x1066;

  EXPECT_EQ(inet_pton(AF_INET6, IPV6_SRC_ADDR,
                      &tunnel_info.local_ip.ip.v6addr.__in6_u.__u6_addr32),
            1)
      << "Error converting " << IPV6_SRC_ADDR;
  tunnel_info.local_ip.prefix_len = IPV6_PREFIX_LEN;
  tunnel_info.local_ip.family = AF_INET6;

  EXPECT_EQ(inet_pton(AF_INET6, IPV6_DST_ADDR,
                      &tunnel_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32),
            1)
      << "Error converting " << IPV6_DST_ADDR;
  tunnel_info.remote_ip.prefix_len = IPV6_PREFIX_LEN;
  tunnel_info.remote_ip.family = AF_INET6;

  tunnel_info.src_port = SRC_PORT;
  tunnel_info.dst_port = DST_PORT;
  tunnel_info.vni = VNI;
};

void TunnelInfoTest::InitVxlanTagged(struct tunnel_info& tunnel_info) {
  tunnel_info.tunnel_type = OVS_TUNNEL_VXLAN;
  tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
  tunnel_info.vni = 0x1066;
}

void TunnelInfoTest::InitVxlanUntagged(struct tunnel_info& tunnel_info) {
  tunnel_info.tunnel_type = OVS_TUNNEL_VXLAN;
  tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
  tunnel_info.vni = 0x1492;
}

void TunnelInfoTest::InitGeneveTagged(struct tunnel_info& tunnel_info) {
  tunnel_info.tunnel_type = OVS_TUNNEL_GENEVE;
  tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
  tunnel_info.vni = 0x1776;
}

void TunnelInfoTest::InitGeneveUntagged(struct tunnel_info& tunnel_info) {
  tunnel_info.tunnel_type = OVS_TUNNEL_GENEVE;
  tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
  tunnel_info.vni = 0x1984;
}

void TunnelInfoTest::AssertTableId(const p4::v1::TableEntry& table_entry,
                                   const ::p4::config::v1::P4Info& p4info,
                                   const char* table_name) {
  auto expected_id = GetTableId(p4info, table_name);
  EXPECT_EQ(table_entry.table_id(), expected_id);
}

}  // namespace ovsp4rt
