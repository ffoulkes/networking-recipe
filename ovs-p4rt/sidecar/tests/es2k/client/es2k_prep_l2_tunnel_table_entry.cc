// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "core/common/ovsp4rt_entry_utils.h"
#include "core/es2k/p4_name_mapping.h"
#include "mac_learn_info_test.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

class Es2kPrepL2TunnelEntryTest : public MacLearnInfoTest {
 public:
  Es2kPrepL2TunnelEntryTest() {}
  virtual ~Es2kPrepL2TunnelEntryTest() = default;

  static void InitFdbInfo(struct mac_learning_info& fdb_info) {
    constexpr uint8_t MAC_ADDR[] = {0xde, 0xad, 0xbe, 0xef, 0x00, 0xe};
    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
  }

  static void AssertV4TunnelTable(::p4::v1::TableEntry& table_entry,
                                  ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, L2_TO_TUNNEL_V4_TABLE);
  }

  static void AssertV6TunnelTable(::p4::v1::TableEntry& table_entry,
                                  ::p4::config::v1::P4Info& p4info) {
    AssertTableId(table_entry, p4info, L2_TO_TUNNEL_V6_TABLE);
  }
};

//----------------------------------------------------------------------

TEST_F(Es2kPrepL2TunnelEntryTest, prepareL2TunnelV4TableEntry) {
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;

  struct mac_learning_info learn_info = {0};
  InitFdbInfo(learn_info);
  InitV4TunnelInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  PrepareL2TunnelTableEntry(&table_entry, learn_info, p4info, INSERT_ENTRY,
                            detail);

  AssertV4TunnelTable(table_entry, p4info);
}

TEST_F(Es2kPrepL2TunnelEntryTest, prepareL2TunnelTableV6Entry) {
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;

  struct mac_learning_info learn_info = {0};
  InitFdbInfo(learn_info);
  InitV6TunnelInfo(learn_info);

  ::p4::config::v1::P4Info p4info;
  InitP4Info(&p4info);

  PrepareL2TunnelTableEntry(&table_entry, learn_info, p4info, INSERT_ENTRY,
                            detail);

  AssertV6TunnelTable(table_entry, p4info);
}

}  // namespace ovsp4rt
