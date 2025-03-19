// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_PRIVATE_H_
#define OVSP4RT_PRIVATE_H_

#include <absl/status/status.h>

#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

//----------------------------------------------------------------------
// Common functions
//----------------------------------------------------------------------

extern void EncodeFdbRxVlanTableEntry(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail);

extern void EncodeFdbTableEntryforV4GeneveTunnel(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail);

extern void EncodeFdbTableEntryforV4VxlanTunnel(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail);

extern void EncodeFdbTxVlanTableEntry(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail);

extern void EncodeL2FwdTxTablePrologue(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

// extracted from WriteL2TunnelTableEntry
extern void PrepareL2TunnelTableEntry(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail);

extern void EncodeVxlanEncapTableEntry(p4::v1::TableEntry* table_entry,
                                       const struct tunnel_info& tunnel_info,
                                       const ::p4::config::v1::P4Info& p4info,
                                       bool insert_entry);

extern void EncodeTunnelTermTableEntry(p4::v1::TableEntry* table_entry,
                                       const struct tunnel_info& tunnel_info,
                                       const ::p4::config::v1::P4Info& p4info,
                                       bool insert_entry);

//----------------------------------------------------------------------
// ES2K-specific functions
//----------------------------------------------------------------------

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

extern void EncodeDstIpMacMapTableEntry(p4::v1::TableEntry* table_entry,
                                        const struct ip_mac_map_info& ip_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry, DiagDetail& detail);

extern void EncodeFdbSmacTableEntry(p4::v1::TableEntry* table_entry,
                                    const struct mac_learning_info& learn_info,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry, DiagDetail& detail);

extern void EncodeSrcIpMacMapTableEntry(p4::v1::TableEntry* table_entry,
                                        const struct ip_mac_map_info& ip_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry, DiagDetail& detail);

extern void EncodeL2ToTunnelV4(p4::v1::TableEntry* table_entry,
                               const struct mac_learning_info& learn_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry, DiagDetail& detail);

extern void EncodeL2ToTunnelV6(p4::v1::TableEntry* table_entry,
                               const struct mac_learning_info& learn_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry, DiagDetail& detail);

extern void EncodeGeneveDecapModTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void EncodeGeneveDecapModAndVlanPushTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void EncodeGeneveEncapTableEntry(p4::v1::TableEntry* table_entry,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry);

extern void EncodeGeneveEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void EncodeV6GeneveEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void EncodeV6GeneveEncapTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareRxTunnelSrcPortTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void EncodeRxTunnelTableEntry(p4::v1::TableEntry* table_entry,
                                     const struct tunnel_info& tunnel_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry);

extern void EncodeV6RxTunnelTableEntry(p4::v1::TableEntry* table_entry,
                                       const struct tunnel_info& tunnel_info,
                                       const ::p4::config::v1::P4Info& p4info,
                                       bool insert_entry);

extern void EncodeSrcPortTableEntry(p4::v1::TableEntry* table_entry,
                                    const struct src_port_info& sp,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry);

extern void EncodeTxAccVsiTableEntry(p4::v1::TableEntry* table_entry,
                                     uint32_t sp,
                                     const ::p4::config::v1::P4Info& p4info);

extern void EncodeV6TunnelTermTableEntry(p4::v1::TableEntry* table_entry,
                                         const struct tunnel_info& tunnel_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry);

extern void EncodeVlanPopTableEntry(p4::v1::TableEntry* table_entry,
                                    const uint16_t vlan_id,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry);

extern void EncodeVlanPushTableEntry(p4::v1::TableEntry* table_entry,
                                     const uint16_t vlan_id,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry);

extern void EncodeVxlanDecapModTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void EncodeVxlanDecapModAndVlanPushTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void EncodeVxlanEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void EncodeV6VxlanEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void EncodeV6VxlanEncapTableEntry(p4::v1::TableEntry* table_entry,
                                         const struct tunnel_info& tunnel_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry);

#endif  // ES2K_TARGET

}  // namespace ovsp4rt

#endif  // OVSP4RT_PRIVATE_H_
