// Copyright 2022-2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

// Private utility functions.

#ifndef OVSP4RT_UTIL_INT_H_
#define OVSP4RT_UTIL_INT_H_

#include <stdarg.h>

#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

//----------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------

extern std::string EncodeByteValue(int arg_count...);

extern int GetActionId(const ::p4::config::v1::P4Info& p4info,
                       const std::string& a_name);

extern int GetTableId(const ::p4::config::v1::P4Info& p4info,
                      const std::string& t_name);

}  // namespace ovsp4rt

#endif  // OVSP4RT_UTIL_INT_H_
