// Copyright 2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_JOURNAL_H_
#define OVSP4RT_JOURNAL_H_

#include <stdbool.h>
#include <stdint.h>

#include <nlohmann/json.hpp>
#include <vector>

#include "ovsp4rt/ovs-p4rt.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

// Records the inputs and outputs of an API function.
class Journal {
 public:
  ~Journal() { saveEntry(); }

  void recordInputs(const char* func_name, const struct mac_learning_info& info,
                    bool insert_entry);

  void recordInputs(const char* func_name, const ip_mac_map_info& info,
                    bool insert_entry);

  void recordInputs(const char* func_name, const tunnel_info& info,
                    bool insert_entry);

  void recordInputs(const char* func_name, const src_port_info info,
                    bool insert_entry);

  void recordInputs(const char* func_name, uint16_t vlan_id, bool insert_entry);

  void recordReadRequest(const ::p4::v1::ReadRequest& request);

  void recordReadResponse(
      const absl::StatusOr<::p4::v1::ReadResponse>& response);

  void recordWriteRequest(const ::p4::v1::WriteRequest& request);

  void recordWriteStatus(const absl::Status& status);

  void saveEntry() {}

 protected:
  void journalInputs(nlohmann::json& json);

  void journalAction(nlohmann::json& json);

 private:
  nlohmann::json input_;
  std::vector<nlohmann::json> actions_;
};

}  // namespace ovsp4rt

#endif  // OVSP4RT_JOURNAL_H_
