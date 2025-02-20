// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_CLIENT_FLAGS_H
#define OVSP4RT_CLIENT_FLAGS_H

#include <absl/flags/declare.h>
#include <stdint.h>

#include <string>

ABSL_DECLARE_FLAG(uint64_t, device_id);
ABSL_DECLARE_FLAG(std::string, role_name);

#endif  // OVSP4RT_CLIENT_FLAGS_H
