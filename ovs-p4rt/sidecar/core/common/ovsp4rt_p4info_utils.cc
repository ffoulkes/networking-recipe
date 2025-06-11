// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "core/common/ovsp4rt_p4info_utils.h"

#include <string>

#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

int GetActionId(const ::p4::config::v1::P4Info& p4info,
                const std::string& a_name) {
  for (const auto& action : p4info.actions()) {
    const auto& pre = action.preamble();
    if (pre.name() == a_name) return pre.id();
  }
  return -1;
}

int GetMatchFieldId(const ::p4::config::v1::P4Info& p4info,
                    const std::string& t_name, const std::string& mf_name) {
  for (const auto& table : p4info.tables()) {
    const auto& pre = table.preamble();
    if (pre.name() != t_name) continue;
    for (const auto& mf : table.match_fields())
      if (mf.name() == mf_name) return mf.id();
  }
  return -1;
}

int GetParamId(const ::p4::config::v1::P4Info& p4info,
               const std::string& a_name, const std::string& param_name) {
  for (const auto& action : p4info.actions()) {
    const auto& pre = action.preamble();
    if (pre.name() != a_name) continue;
    for (const auto& param : action.params())
      if (param.name() == param_name) return param.id();
  }
  return -1;
}

int GetTableId(const ::p4::config::v1::P4Info& p4info,
               const std::string& t_name) {
  for (const auto& table : p4info.tables()) {
    const auto& pre = table.preamble();
    if (pre.name() == t_name) return pre.id();
  }
  return -1;
}

}  // namespace ovsp4rt
