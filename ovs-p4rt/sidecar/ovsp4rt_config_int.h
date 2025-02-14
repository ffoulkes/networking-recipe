// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Defines the internal interface to the C++ ConfigEntry functions.

#ifndef OVSP4RT_CONFIG_API_H_
#define OVSP4RT_CONFIG_API_H_

#include "absl/status/status.h"
#include "client/ovsp4rt_client_interface.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

extern absl::Status ConfigFdbTxVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

}  // namespace ovsp4rt

#endif  // OVSP4RT_CONFIG_API_H_
