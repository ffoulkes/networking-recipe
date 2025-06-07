// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Table entry utility functions.

#ifndef OVSP4RT_ENTRY_UTILS_H_
#define OVSP4RT_ENTRY_UTILS_H_

#include <absl/status/status.h>
#include <absl/status/statusor.h>

#include "client/ovsp4rt_client_interface.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

//----------------------------------------------------------------------
// Get functions
//----------------------------------------------------------------------

#if defined(ES2K_TARGET)

extern absl::StatusOr<uint32_t> GetTxAccVsiPort(
    ClientInterface& client, const ::p4::config::v1::P4Info& p4info,
    uint32_t src_port);

#endif  // ES2K_TARGET

//----------------------------------------------------------------------
// Prepare functions
//----------------------------------------------------------------------

extern void PrepareL2TunnelTableEntry(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail);

#if defined(ES2K_TARGET)

extern void Es2kPrepareDecapTableEntry(::p4::v1::TableEntry* table_entry,
                                       const struct tunnel_info& tunnel_info,
                                       const ::p4::config::v1::P4Info& p4info,
                                       bool insert_entry);

extern void Es2kPrepareEncapTableEntry(::p4::v1::TableEntry* table_entry,
                                       const struct tunnel_info& tunnel_info,
                                       const ::p4::config::v1::P4Info& p4info,
                                       bool insert_entry);

extern void PrepareFdbTableV4TunnelEntry(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail, bool testing = false);

extern void Es2kPrepareFdbTunnelTableEntry(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail);

extern void Es2kPrepareTunnelTermTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareRxTunnelSrcPortTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

#endif  // ES2K_TARGET

//----------------------------------------------------------------------
// Read functions
//----------------------------------------------------------------------

extern absl::StatusOr<::p4::v1::ReadResponse> ReadFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool adding = false);

extern absl::StatusOr<::p4::v1::ReadResponse> ReadFdbVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool adding = false);

#if defined(ES2K_TARGET)

extern absl::StatusOr<::p4::v1::ReadResponse> ReadL2ToTunnelV4TableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern absl::StatusOr<::p4::v1::ReadResponse> ReadL2ToTunnelV6TableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern absl::StatusOr<::p4::v1::ReadResponse> ReadTxAccVsiTableEntry(
    ClientInterface& client, uint32_t sp,
    const ::p4::config::v1::P4Info& p4info);

extern absl::StatusOr<::p4::v1::ReadResponse> ReadVmDstTableEntry(
    ClientInterface& client, const struct ip_mac_map_info& ip_info,
    const ::p4::config::v1::P4Info& p4info);

extern absl::StatusOr<::p4::v1::ReadResponse> ReadVmSrcTableEntry(
    ClientInterface& client, struct ip_mac_map_info ip_info,
    const ::p4::config::v1::P4Info& p4info);

#endif  // ES2K_TARGET

//----------------------------------------------------------------------
// Update functions
//----------------------------------------------------------------------

#if defined(ES2K_TARGET)

extern absl::Status UpdateFdbSrcPortInfo(
    ClientInterface& client, struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern void UpdateFdbTunnelInfo(ClientInterface& client,
                                struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info);

#endif  // ES2K_TARGET

//----------------------------------------------------------------------
// Write functions
//----------------------------------------------------------------------

extern absl::Status WriteEncapTableEntry(ClientInterface& client,
                                         const struct tunnel_info& tunnel_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry);

extern absl::Status WriteFdbRxVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

absl::Status WriteFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status WriteFdbTxVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status WriteTunnelTermTableEntry(
    ClientInterface& client, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

#if defined(ES2K_TARGET)

extern absl::Status WriteDecapTableEntry(ClientInterface& client,
                                         const struct tunnel_info& tunnel_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry);

extern absl::Status WriteDstIpMacMapTableEntry(
    ClientInterface& client, const struct ip_mac_map_info& ip_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status WriteFdbSmacTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status WriteL2TunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status WriteRxTunnelSrcPortTableEntry(
    ClientInterface& client, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status WriteSrcIpMacMapTableEntry(
    ClientInterface& client, const struct ip_mac_map_info& ip_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status WriteTunnelSrcPortEntry(
    ClientInterface& client, const struct src_port_info& tnl_sp,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status WriteVlanPopTableEntry(
    ClientInterface& client, const uint16_t vlan_id,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status WriteVlanPushTableEntry(
    ClientInterface& client, const uint16_t vlan_id,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status WriteVsiSrcPortTableEntry(
    ClientInterface& client, const struct src_port_info& sp,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

#endif  // ES2K_TARGET

}  // namespace ovsp4rt

#endif  // OVSP4RT_ENTRY_UTILS_H_
