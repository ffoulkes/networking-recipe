// Copyright 2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_journal.h"

#include "ovsp4rt_encode_actions.h"
#include "ovsp4rt_encode_inputs.h"

namespace ovsp4rt {

// mac_learning_info
void Journal::recordInputs(const char* func_name,
                           const struct mac_learning_info& info,
                           bool insert_entry) {
  auto inputs = EncodeMacLearningInfo(func_name, info, insert_entry);
  journalInputs(inputs);
}

// ip_mac_map_info
void Journal::recordInputs(const char* func_name, const ip_mac_map_info& info,
                           bool insert_entry) {
  auto inputs = EncodeIpMacMapInfo(func_name, info, insert_entry);
  journalInputs(inputs);
}

// tunnel_info
void Journal::recordInputs(const char* func_name, const tunnel_info& info,
                           bool insert_entry) {
  auto inputs = EncodeTunnelInfo(func_name, info, insert_entry);
  journalInputs(inputs);
}

// src_port_info
void Journal::recordInputs(const char* func_name, const src_port_info info,
                           bool insert_entry) {
  auto inputs = EncodeSrcPortInfo(func_name, info, insert_entry);
  journalInputs(inputs);
}

// vlan_id
void Journal::recordInputs(const char* func_name, uint16_t vlan_id,
                           bool insert_entry) {
  auto inputs = EncodeVlanId(func_name, vlan_id, insert_entry);
  journalInputs(inputs);
}

void Journal::recordReadRequest(const ::p4::v1::ReadRequest& request) {
  // TODO(derek): Add func_name parameter?
  auto action = EncodeReadRequest(request);
  journalAction(action);
}

void Journal::recordReadResponse(
    const absl::StatusOr<::p4::v1::ReadResponse>& response) {
  // TODO(derek): Add func_name parameter?
  auto action = EncodeReadResponse(response);
  journalAction(action);
}

void Journal::recordWriteRequest(const ::p4::v1::WriteRequest& request) {
  // TODO(derek): Add func_name parameter?
  auto action = EncodeWriteRequest(request);
  journalAction(action);
}

void Journal::recordWriteStatus(const absl::Status& status) {
  // TODO(derek): Add func_name parameter?
  auto action = EncodeWriteStatus(status);
  journalAction(action);
}

void Journal::journalInputs(nlohmann::json& json) {
  // TODO(derek): to be implemented.
}

void Journal::journalAction(nlohmann::json& json) {
  // TODO(derek): to be implemented.
}

}  // namespace ovsp4rt
