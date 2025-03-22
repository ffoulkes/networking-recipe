// Copyright 2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef BASE_TABLE_TEST_H_
#define BASE_TABLE_TEST_H_

#include <arpa/inet.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "p4info_helper.h"

namespace ovsp4rt {

constexpr uint32_t VNI_VALUE_MASK = 0x00ffffffU;  // bit<24>
constexpr uint32_t TUNNEL_ID_MASK = 0x000fffffU;  // bit<20>

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

// P4Info object describing the pipeline configuration.
static ::p4::config::v1::P4Info p4info;

class BaseTableTest : public ::testing::Test {
 protected:
  BaseTableTest();

  // Initializes the p4info object.
  static void SetUpTestSuite() { InitP4Info(&p4info); }

  static void InitP4Info(::p4::config::v1::P4Info* p4info);

  //----------------------------
  // P4Info lookup methods
  //----------------------------

  inline int GetActionId(const std::string& action_name) const {
    return helper.GetActionId(action_name);
  }

  inline int GetMatchFieldId(const std::string& mf_name) const {
    return helper.GetMatchFieldId(mf_name);
  }

  inline int GetParamId(const std::string& param_name) const {
    return helper.GetParamId(param_name);
  }

  inline void SelectAction(const std::string& action_name) {
    helper.SelectAction(action_name);
  }

  inline void SelectTable(const std::string& table_name) {
    helper.SelectTable(table_name);
  }

  inline int ActionId() const { return helper.action_id(); }

  inline bool HasTable() const { return helper.has_table(); }

  inline int TableId() const { return helper.table_id(); }

  //----------------------------
  // Utility methods
  //----------------------------

  // Decodes a port value and returns it as an unsigned integer.
  static uint16_t DecodePortValue(const std::string& string_value) {
    uint16_t port_value = DecodeWordValue(string_value) & 0xffff;
    // Port values are encoded low byte first.
    return ntohs(port_value);
  }

  // Decodes a tunnel_id value and returns it as an unsigned integer.
  static uint32_t DecodeTunnelId(const std::string& string_value) {
    return DecodeWordValue(string_value) & TUNNEL_ID_MASK;
  }

  // Decodes a vni value and returns it as an unsigned integer.
  static uint32_t DecodeVniValue(const std::string& string_value) {
    return DecodeWordValue(string_value) & VNI_VALUE_MASK;
  }

  // Decodes a parameter value and returns it as an unsigned integer.
  static uint32_t DecodeWordValue(const std::string& string_value) {
    uint32_t word_value = 0;
    for (int i = 0; i < string_value.size(); i++) {
      word_value = (word_value << 8) | (string_value[i] & 0xff);
    }
    return word_value;
  }

  // Dumps table_entry in JSON.
  void DumpTableEntry() const;

  //----------------------------
  // Protected member data
  //----------------------------

  // Table entry to be inserted or removed. Output of the UUT.
  ::p4::v1::TableEntry table_entry;

  // Facilitates use of the p4info object.
  P4InfoHelper helper;

 private:
  //----------------------------
  // Private member data
  //----------------------------

  // Whether DumpTableEntry() is enabled.
  bool dump_json_;
};

}  // namespace ovsp4rt

#endif  // BASE_TABLE_TEST_H_
