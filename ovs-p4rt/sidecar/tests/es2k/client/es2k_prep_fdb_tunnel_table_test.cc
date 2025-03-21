// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for Es2kPrepareFdbTunnelTableEntry().

#include <absl/status/status.h>
#include <gtest/gtest.h>

#include "es2k/p4_name_mapping.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "mac_learn_info_test.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_config_int.h"
#include "ovsp4rt_util_int.h"
#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

constexpr char GRPC_ADDR[] = "1.2.3.4:5678";

class Es2kPrepFdbTunnelTableTest : public MacLearnInfoTest {
 protected:
  Es2kPrepFdbTunnelTableTest() {}
  virtual ~Es2kPrepFdbTunnelTableTest() = default;

  static void InitTunnelLearnInfo(struct mac_learning_info& fdb_info,
                                  uint8_t tunnel_type) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 42;

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(MAC_ADDR));
    fdb_info.bridge_id = BRIDGE_ID;
    fdb_info.tnl_info.tunnel_type = tunnel_type;
    fdb_info.is_tunnel = true;
  }

  static void CheckActionId(const ::p4::v1::TableEntry& table_entry,
                            const ::p4::config::v1::P4Info& p4info,
                            int tunnel_type) {
    std::string action_name;
    if (tunnel_type == OVS_TUNNEL_VXLAN) {
      action_name = L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V4;
    } else if (tunnel_type == OVS_TUNNEL_GENEVE) {
      action_name = L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V4;
    } else {
      EXPECT_TRUE(false) << "Invalid tunnel_type (" << tunnel_type << ")";
    }

    int expected_action_id = GetActionId(p4info, action_name);

    auto table_action = table_entry.action();
    auto action = table_action.action();
    EXPECT_EQ(action.action_id(), expected_action_id);
  }

  static void CheckRemoveAction(const ::p4::v1::TableEntry& table_entry) {
    EXPECT_FALSE(table_entry.has_action());
  }
};

TEST_F(Es2kPrepFdbTunnelTableTest, insertV4VxlanTunnelEntry) {
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;

  struct mac_learning_info learn_info = {0};
  InitTunnelLearnInfo(learn_info, OVS_TUNNEL_VXLAN);
  InitV4NativeTagged(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareFdbTunnelTableEntry(&table_entry, learn_info, p4info, INSERT_ENTRY,
                                 detail);

  CheckActionId(table_entry, p4info, OVS_TUNNEL_VXLAN);
}

TEST_F(Es2kPrepFdbTunnelTableTest, insertV4GeneveTunnelEntry) {
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;

  struct mac_learning_info learn_info = {0};
  InitTunnelLearnInfo(learn_info, OVS_TUNNEL_GENEVE);
  InitV4NativeTagged(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareFdbTunnelTableEntry(&table_entry, learn_info, p4info, INSERT_ENTRY,
                                 detail);

  CheckActionId(table_entry, p4info, OVS_TUNNEL_GENEVE);
}

TEST_F(Es2kPrepFdbTunnelTableTest, removeV4TunnelEntry) {
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;

  struct mac_learning_info learn_info = {0};
  InitTunnelLearnInfo(learn_info, OVS_TUNNEL_UNKNOWN);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  Es2kPrepareFdbTunnelTableEntry(&table_entry, learn_info, p4info, REMOVE_ENTRY,
                                 detail);

  CheckRemoveAction(table_entry);
}

}  // namespace ovsp4rt
