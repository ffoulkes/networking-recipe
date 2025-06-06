// Copyright 2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Defines the internal interface to the C++ DoConfigEntry functions,
// which are invoked by the C API wrappers.

#ifndef OVSP4RT_DO_CONFIG_INT_H_
#define OVSP4RT_DO_CONFIG_INT_H_

#include "absl/status/status.h"
#include "client/ovsp4rt_client_interface.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace ovsp4rt {

extern absl::Status DoConfigFdbEntry(ClientInterface& client,
                                     struct mac_learning_info learn_info,
                                     bool insert_entry, const char* grpc_addr);

extern absl::Status DoConfigTunnelEntry(ClientInterface& client,
                                        const struct tunnel_info& tunnel_info,
                                        bool insert_entry,
                                        const char* grpc_addr);

#if defined(ES2K_TARGET)

extern absl::Status DoConfigIpMacMapEntry(ClientInterface& client,
                                          const struct ip_mac_map_info& ip_info,
                                          bool insert_entry,
                                          const char* grpc_addr);

extern absl::Status DoConfigRxTunnelSrcEntry(
    ClientInterface& client, const struct tunnel_info& tunnel_info,
    bool insert_entry, const char* grpc_addr);

extern absl::Status DoConfigSrcPortEntry(ClientInterface& client,
                                         struct src_port_info vsi_sp,
                                         bool insert_entry,
                                         const char* grpc_addr);

extern absl::Status DoConfigTunnelSrcPortEntry(
    ClientInterface& client, const struct src_port_info& tnl_sp,
    bool insert_entry, const char* grpc_addr);

extern absl::Status DoConfigVlanEntry(ClientInterface& client, uint16_t vlan_id,
                                      bool insert_entry, const char* grpc_addr);

#endif  // ES2K_TARGET

}  // namespace ovsp4rt

#endif  // OVSP4RT_DO_CONFIG_INT_H_
