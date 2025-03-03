// Copyright 2025 Derek Foster
// SPDX-License-Identifier: Apache-2.0

#ifndef BASIC_TEST_H_
#define BASIC_TEST_H_

#include <gtest/gtest.h>

#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

class BasicTest : public ::testing::Test {
 protected:
  BasicTest(){};
  virtual ~BasicTest() = default;

  static void InitP4Info(::p4::config::v1::P4Info* p4info);
};

}  // namespace ovsp4rt

#endif  // BASIC_TEST_H_
