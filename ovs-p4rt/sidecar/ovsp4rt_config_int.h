// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_CONFIG_INT_H_
#define OVSP4RT_CONFIG_INT_H_

#include <absl/status/status.h>
#include <absl/status/statusor.h>

#include "client/ovsp4rt_client_interface.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

extern absl::Status ConfigFdbRxVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

absl::Status ConfigFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigFdbTxVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigL2TunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigTunnelTermTableEntry(
    ClientInterface& client, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

#if defined(ES2K_TARGET)

extern absl::Status ConfigDecapTableEntry(
    ClientInterface& client, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigDstIpMacMapTableEntry(
    ClientInterface& client, const struct ip_mac_map_info& ip_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigEncapTableEntry(
    ClientInterface& client, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigFdbSmacTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigFdbUpdateSrcPort(
    ClientInterface& client, struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern void ConfigFdbUpdateTunnelInfo(ClientInterface& client,
                                      struct mac_learning_info& learn_info,
                                      const ::p4::config::v1::P4Info& p4info);

extern absl::Status ConfigSrcIpMacMapTableEntry(
    ClientInterface& client, const struct ip_mac_map_info& ip_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigVlanPopTableEntry(
    ClientInterface& client, const uint16_t vlan_id,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigVlanPushTableEntry(
    ClientInterface& client, const uint16_t vlan_id,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::StatusOr<::p4::v1::ReadResponse> GetL2ToTunnelV4TableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

#endif  // ES2K_TARGET

}  // namespace ovsp4rt

#endif  // OVSP4RT_CONFIG_INT_H_
