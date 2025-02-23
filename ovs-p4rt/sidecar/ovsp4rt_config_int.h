// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_CONFIG_INT_H_
#define OVSP4RT_CONFIG_INT_H_

#include "client/ovsp4rt_client_interface.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

void ConfigFdbUpdateTunnelInfo(ClientInterface& client,
                               struct mac_learning_info& learn_info,
                               const ::p4::config::v1::P4Info& p4info);

}  // namespace ovsp4rt

#endif  // OVSP4RT_CONFIG_INT_H_
