// Copyright 2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "tests/base/base_mac_map_info_test.h"

#include <arpa/inet.h>

#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

void BaseMacMapInfoTest::InitIpv4MapInfo(struct ip_mac_map_info map_info) {
  constexpr uint8_t SRC_MAC[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  constexpr char IPV4_SRC_ADDR[] = "10.20.30.40";
  constexpr int IPV4_PREFIX_LEN = 24;

  memcpy(map_info.src_mac_addr, SRC_MAC, sizeof(map_info.src_mac_addr));

  EXPECT_EQ(
      inet_pton(AF_INET, IPV4_SRC_ADDR, &map_info.src_ip_addr.ip.v4addr.s_addr),
      1)
      << "Error converting " << IPV4_SRC_ADDR;
  map_info.src_ip_addr.family = AF_INET;
  map_info.src_ip_addr.prefix_len = IPV4_PREFIX_LEN;
}

}  // namespace ovsp4rt
