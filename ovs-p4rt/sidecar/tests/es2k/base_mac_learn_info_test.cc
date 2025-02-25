// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "base_mac_learn_info_test.h"

#include <absl/status/status.h>
#include <arpa/inet.h>
#include <gtest/gtest.h>

#include "ovsp4rt/ovs-p4rt.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

namespace ovsp4rt {

void BaseMacLearnInfoTest::InitV4NativeTagged(
    struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.local_ip.family = AF_INET;
  fdb_info.tnl_info.remote_ip.family = AF_INET;
  fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
  fdb_info.tnl_info.vni = 0x1984U;
}

void BaseMacLearnInfoTest::InitV4NativeUntagged(
    struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.local_ip.family = AF_INET;
  fdb_info.tnl_info.remote_ip.family = AF_INET;
  fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
  fdb_info.tnl_info.vni = 0x1776U;
}

void BaseMacLearnInfoTest::InitV6NativeTagged(
    struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.local_ip.family = AF_INET6;
  fdb_info.tnl_info.remote_ip.family = AF_INET6;
  fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
  fdb_info.tnl_info.vni = 0xFACEU;
}

void BaseMacLearnInfoTest::InitV6NativeUntagged(
    struct mac_learning_info& fdb_info) {
  fdb_info.tnl_info.local_ip.family = AF_INET6;
  fdb_info.tnl_info.remote_ip.family = AF_INET6;
  fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
  fdb_info.tnl_info.vni = 0xCEDEU;
}

void BaseMacLearnInfoTest::InitP4Info(::p4::config::v1::P4Info* p4info) {
  auto status = stratum::ParseProtoFromString(P4INFO_TEXT, p4info);
  EXPECT_TRUE(status.ok()) << "ParseProtoFromString: "
                           << status.error_message();
}

}  // namespace ovsp4rt
