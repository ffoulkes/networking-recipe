// Copyright 2024 Intel Corporation
// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#include "base_table_test.h"

#include <absl/flags/flag.h>
#include <absl/status/status.h>
#include <google/protobuf/util/json_util.h>
#include <gtest/gtest.h>

#include <iostream>

#include "p4/config/v1/p4info.pb.h"
#include "stratum/lib/utils.h"

#if defined(DPDK_TARGET)
#include "dpdk/p4info_text.h"
#elif defined(ES2K_TARGET)
#include "es2k/p4info_text.h"
#endif

ABSL_FLAG(bool, dump_json, false, "Dump table_entry in JSON");

namespace ovsp4rt {

using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::MessageToJsonString;

BaseTableTest::BaseTableTest() : helper(p4info) {
  dump_json_ = absl::GetFlag(FLAGS_dump_json);
}

void BaseTableTest::InitP4Info(::p4::config::v1::P4Info* p4info) {
  auto status = stratum::ParseProtoFromString(P4INFO_TEXT, p4info);
  if (!status.ok()) {
    std::cerr << "ParseProtoFromString: " << status << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

void BaseTableTest::DumpTableEntry() const {
  if (dump_json_) {
    JsonPrintOptions options;
    options.add_whitespace = true;
    options.preserve_proto_field_names = true;
    std::string output;
    auto status = MessageToJsonString(table_entry, &output, options);
    if (!status.ok()) {
      std::cerr << "MessageToJsonString: " << status << std::endl;
    } else {
      std::cout << output << std::endl;
    }
  }
}

}  // namespace ovsp4rt
