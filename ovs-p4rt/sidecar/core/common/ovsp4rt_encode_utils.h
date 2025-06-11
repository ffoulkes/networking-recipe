// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Functions to encode values for P4Runtime requests.

#ifndef OVSP4RT_ENCODE_UTILS_H_
#define OVSP4RT_ENCODE_UTILS_H_

#include <arpa/inet.h>
#include <stdarg.h>
#include <stdint.h>

#include <string>

#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

extern std::string CanonicalizeIp(const uint32_t ipv4addr);

#if defined(ES2K_TARGET)
extern std::string CanonicalizeIpv6(const struct in6_addr ipv6addr);
#endif

extern std::string CanonicalizeMac(const uint8_t mac[6]);

extern std::string EncodeByteValue(int arg_count...);

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

}  // namespace ovsp4rt

#endif  // OVSP4RT_ENCODE_UTILS_H_
