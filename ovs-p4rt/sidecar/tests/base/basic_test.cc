// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "tests/base/basic_test.h"

#include "p4/config/v1/p4info.pb.h"
#include "stratum/lib/utils.h"

#if defined(DPDK_TARGET)
#include "tests/dpdk/p4info_text.h"
#elif defined(ES2K_TARGET)
#include "tests/es2k/p4info_text.h"
#endif

namespace ovsp4rt {

void BasicTest::InitP4Info(::p4::config::v1::P4Info* p4info) {
  auto status = stratum::ParseProtoFromString(P4INFO_TEXT, p4info);
  if (!status.ok()) {
    std::cerr << "ParseProtoFromString: " << status << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

}  // namespace ovsp4rt
