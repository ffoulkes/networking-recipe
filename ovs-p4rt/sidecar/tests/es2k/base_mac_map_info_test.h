// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef BASE_MAC_MAP_INFO_TEST_H
#define BASE_MAC_MAP_INFO_TEST_H

#include <gtest/gtest.h>

#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

class BaseMacMapInfoTest : public ::testing::Test {
 protected:
  BaseMacMapInfoTest() {}
  virtual ~BaseMacMapInfoTest() = default;

  static void InitIpv4MapInfo(struct ip_mac_map_info map_info);
  static void InitP4Info(::p4::config::v1::P4Info* p4info);
};

}  // namespace ovsp4rt

#endif  // BASE_MAC_MAP_INFO_TEST_H
