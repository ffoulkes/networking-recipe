// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareFdbTableV4TunnelEntry().

#include <nlohmann/json.hpp>

#include "es2k/p4_name_mapping.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "ovsp4rt_util_int.h"  // GetActionId
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "tests/base/base_mac_learn_info_test.h"

namespace ovsp4rt {

class Es2kPrepFdbTableV4TunnelTest : public BaseMacLearnInfoTest {
 public:
  Es2kPrepFdbTableV4TunnelTest() {}
  virtual ~Es2kPrepFdbTableV4TunnelTest() = default;

  static void InitFdbInfo(struct mac_learning_info& fdb_info) {
    constexpr uint8_t MAC_ADDR[] = {0xde, 0xad, 0xbe, 0xef, 0x00, 0xe};
    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
  }

  static void AssertActionId(const ::p4::v1::TableEntry& table_entry,
                             const ::p4::config::v1::P4Info& p4info,
                             const char* action_name) {
    int expected_action_id = GetActionId(p4info, action_name);
    auto table_action = table_entry.action();
    auto action = table_action.action();
    EXPECT_EQ(action.action_id(), expected_action_id);
  }

  static void AssertV4VxlanTunnel(const ::p4::v1::TableEntry& table_entry,
                                  const ::p4::config::v1::P4Info& p4info) {
    // tunnel_type == OVS_TUNNEL_VXLAN
    AssertActionId(table_entry, p4info,
                   L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V4);
  }

  static void AssertV4GeneveTunnel(const ::p4::v1::TableEntry& table_entry,
                                   const ::p4::config::v1::P4Info& p4info) {
    // tunnel_type == OVS_TUNNEL_GENEVE
    AssertActionId(table_entry, p4info,
                   L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V4);
  }

  static constexpr int TESTING = true;
};

//----------------------------------------------------------------------

TEST_F(Es2kPrepFdbTableV4TunnelTest, prepareFdbTableV4VxlanEntry) {
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;

  struct mac_learning_info learn_info = {0};
  InitFdbInfo(learn_info);
  InitV4TunnelInfo(learn_info);
  InitVxlanTagged(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  // We specify the optional 'testing' parameter so the UUT provides the
  // feedback we need for the Assert step.
  PrepareFdbTableV4TunnelEntry(&table_entry, learn_info, p4info, INSERT_ENTRY,
                               detail, TESTING);

  AssertV4VxlanTunnel(table_entry, p4info);
}

TEST_F(Es2kPrepFdbTableV4TunnelTest, prepareFdbTableV4GeneveEntry) {
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;

  struct mac_learning_info learn_info = {0};
  InitFdbInfo(learn_info);
  InitV4TunnelInfo(learn_info);
  InitGeneveTagged(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  // We specify the optional 'testing' parameter so the UUT provides the
  // feedback we need for the Assert step.
  PrepareFdbTableV4TunnelEntry(&table_entry, learn_info, p4info, INSERT_ENTRY,
                               detail, TESTING);

  AssertV4GeneveTunnel(table_entry, p4info);
}

}  // namespace ovsp4rt
