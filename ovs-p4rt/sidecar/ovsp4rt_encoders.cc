// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_encoders.h"

#include <arpa/inet.h>
#include <stdint.h>

#include <string>

#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_util_int.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

#if defined(DPDK_TARGET)
#include "dpdk/p4_name_mapping.h"
#elif defined(ES2K_TARGET)
#include "es2k/p4_name_mapping.h"
#endif

namespace ovsp4rt {

#if defined(ES2K_TARGET)
static const std::string tunnel_v6_param_name[] = {
    ACTION_SET_TUNNEL_V6_PARAM_IPV6_1, ACTION_SET_TUNNEL_V6_PARAM_IPV6_2,
    ACTION_SET_TUNNEL_V6_PARAM_IPV6_3, ACTION_SET_TUNNEL_V6_PARAM_IPV6_4};
#endif

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

#if !defined(DPDK_TARGET)
std::string CanonicalizeIpv6(const struct in6_addr ipv6addr) {
  return EncodeByteValue(
      16, ipv6addr.__in6_u.__u6_addr8[0], ipv6addr.__in6_u.__u6_addr8[1],
      ipv6addr.__in6_u.__u6_addr8[2], ipv6addr.__in6_u.__u6_addr8[3],
      ipv6addr.__in6_u.__u6_addr8[4], ipv6addr.__in6_u.__u6_addr8[5],
      ipv6addr.__in6_u.__u6_addr8[6], ipv6addr.__in6_u.__u6_addr8[7],
      ipv6addr.__in6_u.__u6_addr8[8], ipv6addr.__in6_u.__u6_addr8[9],
      ipv6addr.__in6_u.__u6_addr8[10], ipv6addr.__in6_u.__u6_addr8[11],
      ipv6addr.__in6_u.__u6_addr8[12], ipv6addr.__in6_u.__u6_addr8[13],
      ipv6addr.__in6_u.__u6_addr8[14], ipv6addr.__in6_u.__u6_addr8[15]);
}
#endif

std::string CanonicalizeMac(const uint8_t mac[6]) {
  return EncodeByteValue(6, (mac[0] & 0xff), (mac[1] & 0xff), (mac[2] & 0xff),
                         (mac[3] & 0xff), (mac[4] & 0xff), (mac[5] & 0xff));
}

int GetTableId(const ::p4::config::v1::P4Info& p4info,
               const std::string& t_name) {
  for (const auto& table : p4info.tables()) {
    const auto& pre = table.preamble();
    if (pre.name() == t_name) return pre.id();
  }
  return -1;
}

int GetActionId(const ::p4::config::v1::P4Info& p4info,
                const std::string& a_name) {
  for (const auto& action : p4info.actions()) {
    const auto& pre = action.preamble();
    if (pre.name() == a_name) return pre.id();
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

#if defined(ES2K_TARGET)
void EncodeFdbSmacTableEntry(p4::v1::TableEntry* table_entry,
                             const struct mac_learning_info& learn_info,
                             const ::p4::config::v1::P4Info& p4info,
                             bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_SMAC_TABLE;
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_SMAC_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_SMAC_TABLE, L2_FWD_SMAC_TABLE_KEY_SA));
  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, L2_FWD_SMAC_TABLE,
                                       L2_FWD_SMAC_TABLE_KEY_BRIDGE_ID));
  match1->mutable_exact()->set_value(EncodeByteValue(1, learn_info.bridge_id));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, L2_FWD_SMAC_TABLE_ACTION_NO_ACTION));
  }
}
#endif  // ES2K_TARGET

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
#if defined(ES2K_TARGET)
  {
    // match-key: bridge_id
    auto match = table_entry->add_match();
    match->set_field_id(GetMatchFieldId(p4info, L2_FWD_TX_TABLE,
                                        L2_FWD_TX_TABLE_KEY_BRIDGE_ID));
    match->mutable_exact()->set_value(EncodeByteValue(1, learn_info.bridge_id));
  }
#endif
}

void EncodeFdbTxVlanTableEntry(p4::v1::TableEntry* table_entry,
                               const struct mac_learning_info& learn_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_TX_TABLE;

  EncodeL2FwdTxTablePrologue(table_entry, learn_info, p4info);

#if defined(ES2K_TARGET)
  if (insert_entry) {
    /* Action param configured by user in TX_ACC_VSI_TABLE is used as port_id
     * We call GET api to fetch this value and pass it to FDB programming.
     */
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    if (learn_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
      action->set_action_id(
          GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_REMOVE_VLAN_AND_FWD));
      {
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_REMOVE_VLAN_AND_FWD,
                       ACTION_REMOVE_VLAN_AND_FWD_PARAM_PORT_ID));
        auto port_id = learn_info.src_port;
        // TODO(derek): port_id truncated to 8 bits. [es2k]
        // See https://github.com/ipdk-io/networking-recipe/issues/619
        param->set_value(EncodeByteValue(1, port_id));
      }
      {
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_REMOVE_VLAN_AND_FWD,
                       ACTION_REMOVE_VLAN_AND_FWD_PARAM_VLAN_PTR));
        // TODO(derek): port_vlan truncated to 8 bits. [es2k]
        // See https://github.com/ipdk-io/networking-recipe/issues/620
        param->set_value(EncodeByteValue(1, learn_info.vlan_info.port_vlan));
      }
    } else {
      action->set_action_id(GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD));
      {
        auto param = action->add_params();
        param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD,
                                       ACTION_L2_FWD_PARAM_PORT));
        auto port_id = learn_info.src_port;
        // TODO(derek): port_id truncated to 8 bits. [es2k]
        // See https://github.com/ipdk-io/networking-recipe/issues/619
        param->set_value(EncodeByteValue(1, port_id));
      }
    }
  }
#elif defined(DPDK_TARGET)
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
#else
#error "ASSERT: Unknown TARGET type!"
#endif
}

#if defined(ES2K_TARGET)

void EncodeFdbRxVlanTableEntry(p4::v1::TableEntry* table_entry,
                               const struct mac_learning_info& learn_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_RX_TABLE;
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_RX_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_RX_TABLE, L2_FWD_RX_TABLE_KEY_DST_MAC));
  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_RX_TABLE, L2_FWD_RX_TABLE_KEY_BRIDGE_ID));
  match1->mutable_exact()->set_value(EncodeByteValue(1, learn_info.bridge_id));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, L2_FWD_RX_TABLE_ACTION_L2_FWD));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_RX_TABLE_ACTION_L2_FWD,
                                     ACTION_L2_FWD_PARAM_PORT));
      auto port_id = learn_info.rx_src_port;
      // TODO(derek): port_id truncated to 8 bits. [es2k]
      // See https://github.com/ipdk-io/networking-recipe/issues/682
      param->set_value(EncodeByteValue(1, port_id));
    }
  }
}

#elif defined(DPDK_TARGET)

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

#else
#error "ASSERT: Unknown TARGET type!"
#endif

void EncodeFdbTableEntryforV4VxlanTunnel(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_TX_TABLE;

  EncodeL2FwdTxTablePrologue(table_entry, learn_info, p4info);

#if defined(DPDK_TARGET)
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
#elif defined(ES2K_TARGET)
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();

    if (learn_info.tnl_info.local_ip.family == AF_INET &&
        learn_info.tnl_info.remote_ip.family == AF_INET) {
      if (learn_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
        // IPv4, untagged vlan
        action->set_action_id(GetActionId(
            p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_VXLAN_UNDERLAY_V4));
        {
          auto param = action->add_params();
          param->set_param_id(GetParamId(
              p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_VXLAN_UNDERLAY_V4,
              ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      } else {
        // IPv4, tagged vlan
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V4));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V4,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      }
    } else if (learn_info.tnl_info.local_ip.family == AF_INET6 &&
               learn_info.tnl_info.remote_ip.family == AF_INET6) {
      if (learn_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
        // IPv6, untagged vlan
        action->set_action_id(GetActionId(
            p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_VXLAN_UNDERLAY_V6));
        {
          auto param = action->add_params();
          param->set_param_id(GetParamId(
              p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_VXLAN_UNDERLAY_V6,
              ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      } else {
        // IPv6, tagged vlan
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V6));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V6,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      }
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif
}

#ifdef ES2K_TARGET

// Never called when DPDK_TARGET is enabled.
void EncodeFdbTableEntryforV4GeneveTunnel(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_TX_TABLE;

  EncodeL2FwdTxTablePrologue(table_entry, learn_info, p4info);

#if defined(DPDK_TARGET)
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL,
                                     ACTION_SET_TUNNEL_PARAM_TUNNEL_ID));
      // note: 8-bit vni (dpdk)
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
#elif defined(ES2K_TARGET)
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();

    if (learn_info.tnl_info.local_ip.family == AF_INET &&
        learn_info.tnl_info.remote_ip.family == AF_INET) {
      if (learn_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
        // IPv4, untagged vlan
        action->set_action_id(GetActionId(
            p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_GENEVE_UNDERLAY_V4));
        {
          auto param = action->add_params();
          param->set_param_id(GetParamId(
              p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_GENEVE_UNDERLAY_V4,
              ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      } else {
        // IPv4, tagged vlan
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V4));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V4,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      }
    } else if (learn_info.tnl_info.local_ip.family == AF_INET6 &&
               learn_info.tnl_info.remote_ip.family == AF_INET6) {
      if (learn_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
        // IPv6, untagged vlan
        action->set_action_id(GetActionId(
            p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_GENEVE_UNDERLAY_V6));
        {
          auto param = action->add_params();
          param->set_param_id(GetParamId(
              p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_GENEVE_UNDERLAY_V6,
              ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      } else {
        // IPv6, tagged vlan
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V6));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V6,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      }
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif
}

void EncodeL2ToTunnelV4(p4::v1::TableEntry* table_entry,
                        const struct mac_learning_info& learn_info,
                        const ::p4::config::v1::P4Info& p4info,
                        bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_TO_TUNNEL_V4_TABLE;
  table_entry->set_table_id(GetTableId(p4info, L2_TO_TUNNEL_V4_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_TO_TUNNEL_V4_TABLE, L2_TO_TUNNEL_V4_KEY_DA));
  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, L2_TO_TUNNEL_V4_ACTION_SET_TUNNEL_V4));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     L2_TO_TUNNEL_V4_ACTION_SET_TUNNEL_V4,
                                     ACTION_SET_TUNNEL_V4_PARAM_DST_ADDR));
      std::string ip_address =
          CanonicalizeIp(learn_info.tnl_info.remote_ip.ip.v4addr.s_addr);
      param->set_value(ip_address);
    }
  }
}

void EncodeL2ToTunnelV6(p4::v1::TableEntry* table_entry,
                        const struct mac_learning_info& learn_info,
                        const ::p4::config::v1::P4Info& p4info,
                        bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_TO_TUNNEL_V6_TABLE;
  table_entry->set_table_id(GetTableId(p4info, L2_TO_TUNNEL_V6_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_TO_TUNNEL_V6_TABLE, L2_TO_TUNNEL_V6_KEY_DA));
  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, L2_TO_TUNNEL_V6_ACTION_SET_TUNNEL_V6));
    for (unsigned int i = 0; i < 4; i++) {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     L2_TO_TUNNEL_V6_ACTION_SET_TUNNEL_V6,
                                     tunnel_v6_param_name[i]));
      std::string ip_address = CanonicalizeIp(
          learn_info.tnl_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32[i]);
      param->set_value(ip_address);
    }
  }
}

#endif  // ES2K_TARGET

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
#if defined(ES2K_TARGET)
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP,
                                     ACTION_VXLAN_ENCAP_PARAM_SRC_PORT));

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, ((src_port >> 8) & 0xff), (src_port & 0xff)));
    }
#endif
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

#if defined(ES2K_TARGET)

/* GENEVE_ENCAP_MOD_TABLE */
// Ipv4, Tagged, Geneve
void EncodeGeneveEncapTableEntry(p4::v1::TableEntry* table_entry,
                                 const struct tunnel_info& tunnel_info,
                                 const ::p4::config::v1::P4Info& p4info,
                                 bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, GENEVE_ENCAP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, GENEVE_ENCAP_MOD_TABLE,
                      GENEVE_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_GENEVE_ENCAP));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP,
                                     ACTION_GENEVE_ENCAP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP,
                                     ACTION_GENEVE_ENCAP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP,
                                     ACTION_GENEVE_ENCAP_PARAM_SRC_PORT));

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, ((src_port >> 8) & 0xff), (src_port & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP,
                                     ACTION_GENEVE_ENCAP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, ((dst_port >> 8) & 0xff), (dst_port & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP,
                                     ACTION_GENEVE_ENCAP_PARAM_VNI));
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
}

/* VXLAN_ENCAP_V6_MOD_TABLE */
// Ipv6, Tagged, Vxlan
void EncodeV6VxlanEncapTableEntry(p4::v1::TableEntry* table_entry,
                                  const struct tunnel_info& tunnel_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VXLAN_ENCAP_V6_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, VXLAN_ENCAP_V6_MOD_TABLE,
                      VXLAN_ENCAP_V6_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_VXLAN_ENCAP_V6));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6,
                                     ACTION_VXLAN_ENCAP_V6_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.local_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6,
                                     ACTION_VXLAN_ENCAP_V6_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6,
                                     ACTION_VXLAN_ENCAP_V6_PARAM_SRC_PORT));

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, (src_port >> 8) & 0xff, src_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6,
                                     ACTION_VXLAN_ENCAP_V6_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, (dst_port >> 8) & 0xff, dst_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6,
                                     ACTION_VXLAN_ENCAP_V6_PARAM_VNI));
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
}

/* GENEVE_ENCAP_V6_MOD_TABLE */
// Ipv6, Tagged, Geneve
void EncodeV6GeneveEncapTableEntry(p4::v1::TableEntry* table_entry,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, GENEVE_ENCAP_V6_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, GENEVE_ENCAP_V6_MOD_TABLE,
                      GENEVE_ENCAP_V6_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_GENEVE_ENCAP_V6));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_V6,
                                     ACTION_GENEVE_ENCAP_V6_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.local_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_V6,
                                     ACTION_GENEVE_ENCAP_V6_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_V6,
                                     ACTION_GENEVE_ENCAP_V6_PARAM_SRC_PORT));

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, (src_port >> 8) & 0xff, src_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_V6,
                                     ACTION_GENEVE_ENCAP_V6_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, (dst_port >> 8) & 0xff, dst_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_V6,
                                     ACTION_GENEVE_ENCAP_V6_PARAM_VNI));
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
}

/* VXLAN_ENCAP_VLAN_POP_MOD_TABLE */
void EncodeVxlanEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VXLAN_ENCAP_VLAN_POP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(
      p4info, VXLAN_ENCAP_VLAN_POP_MOD_TABLE,
      VXLAN_ENCAP_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP,
                     ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP,
                     ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP,
                     ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_SRC_PORT));

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, ((src_port >> 8) & 0xff), (src_port & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP,
                     ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, ((dst_port >> 8) & 0xff), (dst_port & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP,
                                     ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_VNI));
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
}

/* GENEVE_ENCAP_VLAN_POP_MOD_TABLE */
void EncodeGeneveEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, GENEVE_ENCAP_VLAN_POP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(
      p4info, GENEVE_ENCAP_VLAN_POP_MOD_TABLE,
      GENEVE_ENCAP_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP,
                     ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP,
                     ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP,
                     ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_SRC_PORT));

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, ((src_port >> 8) & 0xff), (src_port & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP,
                     ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, ((dst_port >> 8) & 0xff), (dst_port & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP,
                                     ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_VNI));
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
}

/* VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE */
// Ipv6, Untagged, Vxlan
void EncodeV6VxlanEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(
      p4info, VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE,
      VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP,
                     ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.local_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP,
                     ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP,
                     ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_SRC_PORT));

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, (src_port >> 8) & 0xff, src_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP,
                     ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, (dst_port >> 8) & 0xff, dst_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP,
                                     ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_VNI));
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
}

/* GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE */
// Ipv6, Untagged, Geneve
void EncodeV6GeneveEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(
      p4info, GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE,
      GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP,
                     ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.local_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP,
                     ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP,
                     ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_SRC_PORT));

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, (src_port >> 8) & 0xff, src_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP,
                     ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, (dst_port >> 8) & 0xff, dst_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP,
                     ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_VNI));
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
}

void EncodeRxTunnelTableEntry(p4::v1::TableEntry* table_entry,
                              const struct tunnel_info& tunnel_info,
                              const ::p4::config::v1::P4Info& p4info,
                              bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, RX_IPV4_TUNNEL_SOURCE_PORT_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, RX_IPV4_TUNNEL_SOURCE_PORT_TABLE,
                      RX_IPV4_TUNNEL_SOURCE_PORT_TABLE_KEY_VNI));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, RX_IPV4_TUNNEL_SOURCE_PORT_TABLE,
                      RX_IPV4_TUNNEL_SOURCE_PORT_TABLE_KEY_IPV4_SRC));
  match1->mutable_exact()->set_value(
      CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(
        p4info, RX_IPV4_TUNNEL_SOURCE_PORT_TABLE_ACTION_SET_SRC_PORT));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, RX_IPV4_TUNNEL_SOURCE_PORT_TABLE_ACTION_SET_SRC_PORT,
          ACTION_SET_SRC_PORT));
      param->set_value(EncodeByteValue(2, ((tunnel_info.src_port >> 8) & 0xff),
                                       (tunnel_info.src_port & 0xff)));
    }
  }
}

void EncodeV6RxTunnelTableEntry(p4::v1::TableEntry* table_entry,
                                const struct tunnel_info& tunnel_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, RX_IPV6_TUNNEL_SOURCE_PORT_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, RX_IPV6_TUNNEL_SOURCE_PORT_TABLE,
                      RX_IPV6_TUNNEL_SOURCE_PORT_TABLE_KEY_VNI));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, RX_IPV6_TUNNEL_SOURCE_PORT_TABLE,
                      RX_IPV6_TUNNEL_SOURCE_PORT_TABLE_KEY_IPV6_SRC));
  match1->mutable_exact()->set_value(
      CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(
        p4info, RX_IPV6_TUNNEL_SOURCE_PORT_TABLE_ACTION_SET_SRC_PORT));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, RX_IPV6_TUNNEL_SOURCE_PORT_TABLE_ACTION_SET_SRC_PORT,
          ACTION_SET_SRC_PORT));
      param->set_value(EncodeByteValue(2, ((tunnel_info.src_port >> 8) & 0xff),
                                       (tunnel_info.src_port & 0xff)));
    }
  }
}

#endif  // ES2K_TARGET

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

#if defined(ES2K_TARGET)
  table_entry->set_table_id(GetTableId(p4info, IPV4_TUNNEL_TERM_TABLE));

  // TODO(derek): table does not have a bridge_id match field. [es2k]
  // See https://github.com/ipdk-io/networking-recipe/issues/617 for details.
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                      IPV4_TUNNEL_TERM_TABLE_KEY_BRIDGE_ID));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.bridge_id));

  // match vni
  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                       IPV4_TUNNEL_TERM_TABLE_KEY_VNI));
  match2->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));
#elif defined(DPDK_TARGET)
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
#else
#error "ASSERT: Unknown TARGET type!"
#endif

#if defined(DPDK_TARGET)
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
#elif defined(ES2K_TARGET)
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    if (tunnel_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
      if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
        action->set_action_id(GetActionId(
            p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR_AND_PUSH_VLAN));
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR_AND_PUSH_VLAN,
                       ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
        action->set_action_id(GetActionId(
            p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR_AND_PUSH_VLAN));
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR_AND_PUSH_VLAN,
                       ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else {
        std::cout << "Unsupported tunnel type" << std::endl;
      }
    } else {
      if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
        action->set_action_id(
            GetActionId(p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR));
        auto param = action->add_params();
        param->set_param_id(GetParamId(p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR,
                                       ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
        action->set_action_id(
            GetActionId(p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR));
        auto param = action->add_params();
        param->set_param_id(GetParamId(
            p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR, ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else {
        std::cout << "Unsupported tunnel type" << std::endl;
      }
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif
}

#if defined(ES2K_TARGET)

void EncodeV6TunnelTermTableEntry(p4::v1::TableEntry* table_entry,
                                  const struct tunnel_info& tunnel_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, IPV6_TUNNEL_TERM_TABLE));

  // TODO(derek): table does not have a bridge_id match field. [es2k]
  // See https://github.com/ipdk-io/networking-recipe/issues/617
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, IPV6_TUNNEL_TERM_TABLE,
                                      IPV6_TUNNEL_TERM_TABLE_KEY_BRIDGE_ID));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.bridge_id));

  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, IPV6_TUNNEL_TERM_TABLE,
                                       IPV6_TUNNEL_TERM_TABLE_KEY_IPV6_SRC));
  match1->mutable_exact()->set_value(
      CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));

  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, IPV6_TUNNEL_TERM_TABLE,
                                       IPV6_TUNNEL_TERM_TABLE_KEY_VNI));
  match2->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    if (tunnel_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
      if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
        action->set_action_id(GetActionId(
            p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR_AND_PUSH_VLAN));
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR_AND_PUSH_VLAN,
                       ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
        action->set_action_id(GetActionId(
            p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR_AND_PUSH_VLAN));
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR_AND_PUSH_VLAN,
                       ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else {
        std::cout << "Unsupported tunnel type" << std::endl;
      }
    } else {
      if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
        action->set_action_id(
            GetActionId(p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR));
        auto param = action->add_params();
        param->set_param_id(GetParamId(p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR,
                                       ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
        action->set_action_id(
            GetActionId(p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR));
        auto param = action->add_params();
        param->set_param_id(GetParamId(
            p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR, ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else {
        std::cout << "Unsupported tunnel type" << std::endl;
      }
    }
  }
}

// called-by: PrepareDecapModAndVlanPushTableEntry
void EncodeVxlanDecapModAndVlanPushTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, VXLAN_DECAP_AND_VLAN_PUSH_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, VXLAN_DECAP_AND_VLAN_PUSH_MOD_TABLE,
                      VXLAN_DECAP_AND_VLAN_PUSH_MOD_TABLE_KEY_MOD_BLOB_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, ACTION_VXLAN_DECAP_AND_PUSH_VLAN));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_DECAP_AND_PUSH_VLAN,
                     ACTION_VXLAN_DECAP_AND_PUSH_VLAN_PARAM_PCP));
      // note: magic number
      param->set_value(EncodeByteValue(1, 1));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_DECAP_AND_PUSH_VLAN,
                     ACTION_VXLAN_DECAP_AND_PUSH_VLAN_PARAM_DEI));
      // note: magic number
      param->set_value(EncodeByteValue(1, 0));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_DECAP_AND_PUSH_VLAN,
                     ACTION_VXLAN_DECAP_AND_PUSH_VLAN_PARAM_VLAN_ID));
      // TODO(derek): port_vlan truncated to 8 bits. [es2k]
      // See https://github.com/ipdk-io/networking-recipe/issues/678
      param->set_value(EncodeByteValue(1, tunnel_info.vlan_info.port_vlan));
    }
  }
}

// called-by: PrepareDecapModAndVlanPushTableEntry
void EncodeGeneveDecapModAndVlanPushTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, GENEVE_DECAP_AND_VLAN_PUSH_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, GENEVE_DECAP_AND_VLAN_PUSH_MOD_TABLE,
                      GENEVE_DECAP_AND_VLAN_PUSH_MOD_TABLE_KEY_MOD_BLOB_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, ACTION_GENEVE_DECAP_AND_PUSH_VLAN));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_DECAP_AND_PUSH_VLAN,
                     ACTION_GENEVE_DECAP_AND_PUSH_VLAN_PARAM_PCP));
      // note: magic number
      param->set_value(EncodeByteValue(1, 1));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_DECAP_AND_PUSH_VLAN,
                     ACTION_GENEVE_DECAP_AND_PUSH_VLAN_PARAM_DEI));
      // note: magic number
      param->set_value(EncodeByteValue(1, 0));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_DECAP_AND_PUSH_VLAN,
                     ACTION_GENEVE_DECAP_AND_PUSH_VLAN_PARAM_VLAN_ID));
      // TODO(derek): port_vlan truncated to 8 bits. [es2k]
      // See https://github.com/ipdk-io/networking-recipe/issues/679
      param->set_value(EncodeByteValue(1, tunnel_info.vlan_info.port_vlan));
    }
  }
}

void EncodeVlanPushTableEntry(p4::v1::TableEntry* table_entry,
                              const uint16_t vlan_id,
                              const ::p4::config::v1::P4Info& p4info,
                              bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VLAN_PUSH_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, VLAN_PUSH_MOD_TABLE,
                                      VLAN_PUSH_MOD_KEY_MOD_BLOB_PTR));
  // note: mod_blob_ptr is bit<24>, vlan_id is bit<12>, encoded value is bit<8>.
  match->mutable_exact()->set_value(EncodeByteValue(1, vlan_id));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH,
                                     ACTION_VLAN_PUSH_PARAM_PCP));
      // note: magic number
      // note: pcp is bit<3>
      param->set_value(EncodeByteValue(1, 1));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH,
                                     ACTION_VLAN_PUSH_PARAM_DEI));
      // note: magic number
      // note: dei is bit<1>
      param->set_value(EncodeByteValue(1, 0));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH,
                                     ACTION_VLAN_PUSH_PARAM_VLAN_ID));
      // note: vlan_id is bit<12>, encoded value is bit<8>
      param->set_value(EncodeByteValue(1, vlan_id));
    }
  }
}

void EncodeVlanPopTableEntry(p4::v1::TableEntry* table_entry,
                             const uint16_t vlan_id,
                             const ::p4::config::v1::P4Info& p4info,
                             bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VLAN_POP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, VLAN_POP_MOD_TABLE,
                                      VLAN_POP_MOD_KEY_MOD_BLOB_PTR));
  // TODO(derek): vlan_id truncated to 8 bits. [es2k]
  // See https://github.com/ipdk-io/networking-recipe/issues/684
  match->mutable_exact()->set_value(EncodeByteValue(1, vlan_id));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, VLAN_POP_MOD_ACTION_VLAN_POP));
  }
}

// called-by: DoConfigTunnelSrcPortEntry (es2k)
void EncodeSrcPortTableEntry(p4::v1::TableEntry* table_entry,
                             const struct src_port_info& sp,
                             const ::p4::config::v1::P4Info& p4info,
                             bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, SOURCE_PORT_TO_BRIDGE_MAP_TABLE));

  auto match = table_entry->add_match();
  table_entry->set_priority(1);
  match->set_field_id(
      GetMatchFieldId(p4info, SOURCE_PORT_TO_BRIDGE_MAP_TABLE,
                      SOURCE_PORT_TO_BRIDGE_MAP_TABLE_KEY_SRC_PORT));
  match->mutable_ternary()->set_value(
      EncodeByteValue(2, ((sp.src_port >> 8) & 0xff), (sp.src_port & 0xff)));
  match->mutable_ternary()->set_mask(EncodeByteValue(2, 0xff, 0xff));

  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, SOURCE_PORT_TO_BRIDGE_MAP_TABLE,
                      SOURCE_PORT_TO_BRIDGE_MAP_TABLE_KEY_VID));
  match1->mutable_ternary()->set_value(
      EncodeByteValue(2, ((sp.vlan_id >> 8) & 0x0f), (sp.vlan_id & 0xff)));
  match1->mutable_ternary()->set_mask(EncodeByteValue(2, 0x0f, 0xff));
  // match1->mutable_ternary()->set_mask(EncodeByteValue(1, 0xff));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(
        p4info, SOURCE_PORT_TO_BRIDGE_MAP_TABLE_ACTION_SET_BRIDGE_ID));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, SOURCE_PORT_TO_BRIDGE_MAP_TABLE_ACTION_SET_BRIDGE_ID,
          ACTION_SET_BRIDGE_ID_PARAM_BRIDGE_ID));
      param->set_value(EncodeByteValue(1, sp.bridge_id));
    }
  }
}

void EncodeSrcIpMacMapTableEntry(p4::v1::TableEntry* table_entry,
                                 const struct ip_mac_map_info& ip_info,
                                 const ::p4::config::v1::P4Info& p4info,
                                 bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_SRC_IP_MAC_MAP_TABLE;
  table_entry->set_table_id(GetTableId(p4info, SRC_IP_MAC_MAP_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, SRC_IP_MAC_MAP_TABLE,
                                      SRC_IP_MAC_MAP_TABLE_KEY_SRC_IP));
  match->mutable_exact()->set_value(
      CanonicalizeIp(ip_info.src_ip_addr.ip.v4addr.s_addr));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, SRC_IP_MAC_MAP_TABLE_ACTION_SMAC_MAP));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     SRC_IP_MAC_MAP_TABLE_ACTION_SMAC_MAP,
                                     ACTION_SET_SRC_MAC_HIGH));
      std::string mac_high =
          EncodeByteValue(2, (ip_info.src_mac_addr[0] & 0xff),
                          (ip_info.src_mac_addr[1] & 0xff));
      param->set_value(mac_high);
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     SRC_IP_MAC_MAP_TABLE_ACTION_SMAC_MAP,
                                     ACTION_SET_SRC_MAC_MID));
      std::string mac_mid = EncodeByteValue(2, (ip_info.src_mac_addr[2] & 0xff),
                                            (ip_info.src_mac_addr[3] & 0xff));
      param->set_value(mac_mid);
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     SRC_IP_MAC_MAP_TABLE_ACTION_SMAC_MAP,
                                     ACTION_SET_SRC_MAC_LOW));
      std::string mac_low = EncodeByteValue(2, (ip_info.src_mac_addr[4] & 0xff),
                                            (ip_info.src_mac_addr[5] & 0xff));
      param->set_value(mac_low);
    }
  }
}

void EncodeDstIpMacMapTableEntry(p4::v1::TableEntry* table_entry,
                                 const struct ip_mac_map_info& ip_info,
                                 const ::p4::config::v1::P4Info& p4info,
                                 bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_DST_IP_MAC_MAP_TABLE;
  table_entry->set_table_id(GetTableId(p4info, DST_IP_MAC_MAP_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, DST_IP_MAC_MAP_TABLE,
                                      DST_IP_MAC_MAP_TABLE_KEY_DST_IP));
  match->mutable_exact()->set_value(
      CanonicalizeIp((ip_info.dst_ip_addr.ip.v4addr.s_addr)));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, DST_IP_MAC_MAP_TABLE_ACTION_DMAC_MAP));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     DST_IP_MAC_MAP_TABLE_ACTION_DMAC_MAP,
                                     ACTION_SET_DST_MAC_HIGH));
      std::string mac_high =
          EncodeByteValue(2, (ip_info.dst_mac_addr[0] & 0xff),
                          (ip_info.dst_mac_addr[1] & 0xff));
      param->set_value(mac_high);
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     DST_IP_MAC_MAP_TABLE_ACTION_DMAC_MAP,
                                     ACTION_SET_DST_MAC_MID));
      std::string mac_mid = EncodeByteValue(2, (ip_info.dst_mac_addr[2] & 0xff),
                                            (ip_info.dst_mac_addr[3] & 0xff));
      param->set_value(mac_mid);
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     DST_IP_MAC_MAP_TABLE_ACTION_DMAC_MAP,
                                     ACTION_SET_DST_MAC_LOW));
      std::string mac_low = EncodeByteValue(2, (ip_info.dst_mac_addr[4] & 0xff),
                                            (ip_info.dst_mac_addr[5] & 0xff));
      param->set_value(mac_low);
    }
  }
}

void EncodeTxAccVsiTableEntry(p4::v1::TableEntry* table_entry, uint32_t sp,
                              const ::p4::config::v1::P4Info& p4info) {
  table_entry->set_table_id(GetTableId(p4info, TX_ACC_VSI_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, TX_ACC_VSI_TABLE, TX_ACC_VSI_TABLE_KEY_VSI));
  // TODO(derek): sp value truncated to 8 bits. [es2k]
  // See https://github.com/ipdk-io/networking-recipe/issues/680 for details.
  match->mutable_exact()->set_value(
      EncodeByteValue(1, (sp - ES2K_VPORT_ID_OFFSET)));

#if 0
  /* unused match key of 0, code is added for reference */
  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, TX_ACC_VSI_TABLE,
                      TX_ACC_VSI_TABLE_KEY_ZERO_PADDING));
  match->mutable_exact()->set_value(EncodeByteValue(1, 0));
#endif
}

void EncodeVxlanDecapModTableEntry(p4::v1::TableEntry* table_entry,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VXLAN_DECAP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, VXLAN_DECAP_MOD_TABLE,
                                      VXLAN_DECAP_MOD_TABLE_KEY_MOD_BLOB_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    {
      action->set_action_id(GetActionId(p4info, ACTION_VXLAN_DECAP_OUTER_HDR));
    }
  }
}

void EncodeGeneveDecapModTableEntry(p4::v1::TableEntry* table_entry,
                                    const struct tunnel_info& tunnel_info,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, GENEVE_DECAP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, GENEVE_DECAP_MOD_TABLE,
                                      GENEVE_DECAP_MOD_TABLE_KEY_MOD_BLOB_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    {
      action->set_action_id(GetActionId(p4info, ACTION_GENEVE_DECAP_OUTER_HDR));
    }
  }
}

#endif  // ES2K_TARGET

}  // namespace ovsp4rt
