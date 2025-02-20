// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_client_flags.h"

#include <absl/flags/flag.h>

#define DEFAULT_ROLE_NAME "ovs-p4rt"

ABSL_FLAG(uint64_t, device_id, 1, "P4Runtime device ID.");

ABSL_FLAG(std::string, role_name, DEFAULT_ROLE_NAME, "P4 config role name.");
