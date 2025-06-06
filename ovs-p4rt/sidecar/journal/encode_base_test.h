// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef ENCODE_BASE_TEST_H_
#define ENCODE_BASE_TEST_H_

#include <absl/flags/flag.h>
#include <gtest/gtest.h>
#include <stdbool.h>

ABSL_FLAG(bool, dump_json, false, "Dump JSON output");

namespace ovsp4rt {

class EncodeBaseTest : public ::testing::Test {
 protected:
  EncodeBaseTest() { dump_json_ = absl::GetFlag(FLAGS_dump_json); };
  bool dump_json_;
};

}  // namespace ovsp4rt

#endif  // ENCODE_BASE_TEST_H_
