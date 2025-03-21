# CMake build file for ovs-p4rt/sidecar/tests
#
# Copyright 2024 Intel Corporation
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0
#

include(FindGTest)
mark_as_advanced(GTest_DIR)

include(cmake/set_test_properties.cmake)

option(TEST_COVERAGE OFF "Measure unit test code coverage")

# Set the "ovsp4rt" label on all tests in this directory.
# We can use this with ctest to filter the tests to be run.
set_property(DIRECTORY PROPERTY LABELS ovsp4rt)

set(TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#-----------------------------------------------------------------------
# ovsp4rt::p4info_helper
#-----------------------------------------------------------------------
add_library(ovsp4rt_p4info_helper INTERFACE p4info_helper.h)

target_include_directories(ovsp4rt_p4info_helper INTERFACE
  ${TESTS_SOURCE_DIR}
)

add_library(ovsp4rt::p4info_helper ALIAS ovsp4rt_p4info_helper)

#-----------------------------------------------------------------------
# ovsp4rt::p4info_text
#-----------------------------------------------------------------------
if(DPDK_TARGET)
  add_library(dpdk_p4info_text INTERFACE dpdk/p4info_text.h)
  add_library(ovsp4rt::p4info_text ALIAS dpdk_p4info_text)
elseif(ES2K_TARGET)
  add_library(es2k_p4info_text INTERFACE es2k/p4info_text.h)
  add_library(ovsp4rt::p4info_text ALIAS es2k_p4info_text)
endif()

#-----------------------------------------------------------------------
# ovsp4rt::test_main
#-----------------------------------------------------------------------
add_library(ovsp4rt_test_main_o OBJECT test_main.cc)
add_library(ovsp4rt::test_main ALIAS ovsp4rt_test_main_o)

#-----------------------------------------------------------------------
# Library and macro definitions
#-----------------------------------------------------------------------
include(cmake/basic_test.cmake)
include(cmake/mac_learn_info_test.cmake)
include(cmake/mac_map_info_test.cmake)
include(cmake/tunnel_info_test.cmake)

#-----------------------------------------------------------------------
# Common tests
#-----------------------------------------------------------------------
add_subdirectory(common)

#-----------------------------------------------------------------------
# Target-specific tests
#-----------------------------------------------------------------------
if(ES2K_TARGET)
  add_subdirectory(es2k)
elseif(DPDK_TARGET)
  add_subdirectory(dpdk)
endif()

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
