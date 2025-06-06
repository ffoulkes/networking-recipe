// Copyright 2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Unit test for EncodeL2FwdTxTablePrologue()

#include <gtest/gtest.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "core/common/ovsp4rt_encoders.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace ovsp4rt {

class L2FwdTxPrologueTest : public BaseTableTest {
 protected:
  L2FwdTxPrologueTest() {}

  void SetUp() { SelectTable("l2_fwd_tx_table"); }

  //----------------------------
  // Initialization
  //----------------------------

  void InitFdbInfo() {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 86;

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
    fdb_info.bridge_id = BRIDGE_ID;
  }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    constexpr char BRIDGE_ID_KEY[] = "user_meta.pmeta.bridge_id";
    constexpr char DST_MAC_KEY[] = "dst_mac";

    const int MFID_BRIDGE_ID = GetMatchFieldId(BRIDGE_ID_KEY);
    const int MFID_DST_MAC = GetMatchFieldId(DST_MAC_KEY);

    // number of match fields
    ASSERT_EQ(table_entry.match_size(), 2);

    for (const auto& match : table_entry.match()) {
      auto field_id = match.field_id();
      if (field_id == MFID_DST_MAC) {
        CheckMacAddrMatch(match);
      } else if (field_id == MFID_BRIDGE_ID) {
        CheckBridgeIdMatch(match);
      } else {
        FAIL() << "Unexpected field_id (" << field_id << ")";
      }
    }
  }

  void CheckMacAddrMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int MAC_ADDR_SIZE = 6;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), MAC_ADDR_SIZE);

    for (int i = 0; i < MAC_ADDR_SIZE; i++) {
      EXPECT_EQ(match_value[i], fdb_info.mac_addr[i])
          << "mac_addr[" << i << "] is incorrect";
    }
  }

  void CheckBridgeIdMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int BRIDGE_ID_SIZE = 1;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), BRIDGE_ID_SIZE);

    // widen so values will be treated as ints
    ASSERT_EQ(uint32_t(match_value[0]), uint32_t(fdb_info.bridge_id));
  }

  //----------------------------
  // CheckTableEntry()
  //----------------------------

  void CheckTableEntry() const {
    ASSERT_TRUE(HasTable());
    EXPECT_EQ(table_entry.table_id(), TableId());
  }

  //----------------------------
  // Protected member data
  //----------------------------

  struct mac_learning_info fdb_info = {0};
};

//----------------------------------------------------------------------
// EncodeL2FwdTxTablePrologue()
//----------------------------------------------------------------------

TEST_F(L2FwdTxPrologueTest, encodePrologue) {
  // Arrange
  InitFdbInfo();

  // Act
  EncodeL2FwdTxTablePrologue(&table_entry, fdb_info, p4info);

  // Assert
  CheckTableEntry();
  CheckMatches();
}

}  // namespace ovsp4rt
