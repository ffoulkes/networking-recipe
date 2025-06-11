// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include <arpa/inet.h>
#include <stdint.h>

#include <string>

#include "core/common/ovsp4rt_encode_utils.h"
#include "core/common/ovsp4rt_encoders.h"
#include "core/common/ovsp4rt_p4info_utils.h"
#include "core/dpdk/p4_name_mapping.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

std::string EncodeByteValue(int arg_count...) {
  std::string byte_value;
  va_list args;
  va_start(args, arg_count);

  for (int arg = 0; arg < arg_count; ++arg) {
    uint8_t byte = va_arg(args, int);
    byte_value.push_back(byte);
  }

  va_end(args);
  return byte_value;
}

// Encodes tunnel_info.vni as a "tunnel_id" action parameter,
// which is bit<20> in all cases except set_ipsec_tunnel.
static inline std::string EncodeTunnelId(uint32_t vni) {
  return EncodeByteValue(3, (vni >> 16) & 0x0F, (vni >> 8) & 0xFF, vni & 0xFF);
}

// Encodes tunnel_info.vni as a "vni" or "mod_blob_ptr" match
// field or action parameter, which are bit<24> in all cases.
static inline std::string EncodeVniValue(uint32_t vni) {
  return EncodeByteValue(3, (vni >> 16) & 0xFF, (vni >> 8) & 0xFF, vni & 0xFF);
}

std::string CanonicalizeIp(const uint32_t ipv4addr) {
  // note: low-to-high byte order
  return EncodeByteValue(4, (ipv4addr & 0xff), ((ipv4addr >> 8) & 0xff),
                         ((ipv4addr >> 16) & 0xff), ((ipv4addr >> 24) & 0xff));
}

std::string CanonicalizeMac(const uint8_t mac[6]) {
  return EncodeByteValue(6, (mac[0] & 0xff), (mac[1] & 0xff), (mac[2] & 0xff),
                         (mac[3] & 0xff), (mac[4] & 0xff), (mac[5] & 0xff));
}

void EncodeL2FwdTxTablePrologue(p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info) {
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_TX_TABLE));
  {
    // match-key: dst_mac
    auto match = table_entry->add_match();
    match->set_field_id(
        GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_DST_MAC));
    std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
    match->mutable_exact()->set_value(mac_addr);
  }
}

void EncodeFdbTxVlanTableEntry(p4::v1::TableEntry* table_entry,
                               const struct mac_learning_info& learn_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_TX_TABLE;

  EncodeL2FwdTxTablePrologue(table_entry, learn_info, p4info);

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD,
                                     ACTION_L2_FWD_PARAM_PORT));
      // TODO(derek) Questionable value semantics. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/689
      auto port_id = learn_info.vln_info.vlan_id - 1;
      // TODO(derek): vlan_id truncated to 8 bits. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/689
      param->set_value(EncodeByteValue(1, port_id));
    }
  }
}

void EncodeFdbRxVlanTableEntry(p4::v1::TableEntry* table_entry,
                               const struct mac_learning_info& learn_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_RX_WITH_TUNNEL_TABLE;
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_RX_WITH_TUNNEL_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, L2_FWD_RX_WITH_TUNNEL_TABLE,
                                      L2_FWD_TX_TABLE_KEY_DST_MAC));
  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_RX_TABLE_ACTION_L2_FWD,
                                     ACTION_L2_FWD_PARAM_PORT));
      // TODO(derek): questionable value semantics. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/683
      auto port_id = learn_info.vln_info.vlan_id - 1;
      // TODO(derek): vlan_id truncated to 8 bits. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/683
      param->set_value(EncodeByteValue(1, port_id));
    }
  }
}

void EncodeFdbTableEntryforV4VxlanTunnel(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_TX_TABLE;

  EncodeL2FwdTxTablePrologue(table_entry, learn_info, p4info);

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL,
                                     ACTION_SET_TUNNEL_PARAM_TUNNEL_ID));
      // TODO(derek): 8-bit value for 24-bit action parameter. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/677
      param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
    }

    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL,
                                     ACTION_SET_TUNNEL_PARAM_DST_ADDR));
      std::string ip_address =
          CanonicalizeIp(learn_info.tnl_info.remote_ip.ip.v4addr.s_addr);
      param->set_value(ip_address);
    }
  }
}

/* VXLAN_ENCAP_MOD_TABLE */
// Ipv4, Tagged, Vxlan
void EncodeVxlanEncapTableEntry(p4::v1::TableEntry* table_entry,
                                const struct tunnel_info& tunnel_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VXLAN_ENCAP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, VXLAN_ENCAP_MOD_TABLE,
                      VXLAN_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_VXLAN_ENCAP));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP,
                                     ACTION_VXLAN_ENCAP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP,
                                     ACTION_VXLAN_ENCAP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP,
                                     ACTION_VXLAN_ENCAP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, ((dst_port >> 8) & 0xff), (dst_port & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP, ACTION_VXLAN_ENCAP_PARAM_VNI));
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
}

void EncodeTunnelTermTableEntry(p4::v1::TableEntry* table_entry,
                                const struct tunnel_info& tunnel_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  // match remote ipv4 addr
  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                       IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC));
  match1->mutable_exact()->set_value(
      CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));

  table_entry->set_table_id(GetTableId(p4info, IPV4_TUNNEL_TERM_TABLE));

  // match vxlan tunnel type
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                      IPV4_TUNNEL_TERM_TABLE_KEY_TUNNEL_TYPE));
  match->mutable_exact()->set_value(EncodeByteValue(1, TUNNEL_TYPE_VXLAN));

  // match local ipv4 addr
  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                       IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST));
  match2->mutable_exact()->set_value(
      CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_DECAP_OUTER_IPV4));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_DECAP_OUTER_IPV4,
                                     ACTION_DECAP_OUTER_IPV4_PARAM_TUNNEL_ID));
      // TODO(derek): tunnel_id truncated to 8 bits. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/685
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }
}

}  // namespace ovsp4rt
