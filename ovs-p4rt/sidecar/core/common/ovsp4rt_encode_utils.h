// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Functions to encode values for P4Runtime requests.

#ifndef OVSP4RT_ENCODE_UTILS_H_
#define OVSP4RT_ENCODE_UTILS_H_

#include <stdarg.h>

#include <string>

#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

extern std::string EncodeByteValue(int arg_count...);

}  // namespace ovsp4rt

#endif  // OVSP4RT_ENCODE_UTILS_H_
