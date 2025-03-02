# CMake build file for ovs-p4rt/sidecar/tests
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

include(FindGTest)
mark_as_advanced(GTest_DIR)

include(set_test_properties.cmake)

option(TEST_COVERAGE OFF "Measure unit test code coverage")

# Set the "ovsp4rt" label on all tests in this directory.
# We can use this with ctest to filter the tests to be run.
set_property(DIRECTORY PROPERTY LABELS ovsp4rt)

set(TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#-----------------------------------------------------------------------
# base test test libraries
#-----------------------------------------------------------------------
add_subdirectory(base)

#-----------------------------------------------------------------------
# common tests
#-----------------------------------------------------------------------
add_subdirectory(common)

#-----------------------------------------------------------------------
# Target-specific unit tests
#-----------------------------------------------------------------------
if(ES2K_TARGET)
  add_subdirectory(es2k)
elseif(DPDK_TARGET)
  add_subdirectory(dpdk)
endif()

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
