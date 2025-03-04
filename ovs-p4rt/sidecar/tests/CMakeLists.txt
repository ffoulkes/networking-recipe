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
# P4INFO_TEXT
#-----------------------------------------------------------------------
if(DPDK_TARGET)
  set(P4INFO_TEXT dpdk/p4info_text.h)
elseif(ES2K_TARGET)
  set(P4INFO_TEXT es2k/p4info_text.h)
endif()

#-----------------------------------------------------------------------
# ovsp4rt_test_main_o
#-----------------------------------------------------------------------
add_library(ovsp4rt_test_main_o OBJECT test_main.cc)

#-----------------------------------------------------------------------
# ovsp4rt_basic_test
#-----------------------------------------------------------------------
add_library(ovsp4rt_basic_test STATIC
  basic_test.cc
  basic_test.h
  ${P4INFO_TEXT}
  $<TARGET_OBJECTS:ovsp4rt_test_main_o>
)

target_include_directories(ovsp4rt_basic_test PRIVATE
  ${CURRENT_SOURCE_DIR}
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_basic_test PUBLIC
  absl::flags_parse
  p4runtime_proto
  stratum_utils
)

add_library(ovsp4rt::basic_test ALIAS ovsp4rt_basic_test)

#-----------------------------------------------------------------------
# define_ovsp4rt_basic_test()
#-----------------------------------------------------------------------
macro(define_ovsp4rt_basic_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    $<TARGET_OBJECTS:ovsp4rt_test_client_o>
  )

  set_test_properties(${TARGET})

  target_include_directories(${TARGET} PUBLIC
    ${TESTS_SOURCE_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::basic_test
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

#-----------------------------------------------------------------------
# encode_host_port_value_test
#-----------------------------------------------------------------------
add_executable(encode_host_port_value_test
  encode_host_port_value_test.cc
)

set_test_properties(encode_host_port_value_test)

target_link_libraries(encode_host_port_value_test PUBLIC
  GTest::gtest_main
)

list(APPEND UNIT_TEST_NAMES encode_host_port_value_test)

#-----------------------------------------------------------------------
# ovsp4rt_tunnel_info_test
#-----------------------------------------------------------------------
add_library(ovsp4rt_tunnel_info_test STATIC
  base_tunnel_info_test.cc
  base_tunnel_info_test.h
)

target_include_directories(ovsp4rt_tunnel_info_test PUBLIC
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
  ${TESTS_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_tunnel_info_test PUBLIC
  ovsp4rt::basic_test
)

add_library(ovsp4rt::tunnel_info_test ALIAS ovsp4rt_tunnel_info_test)

#-----------------------------------------------------------------------
# define_ovsp4rt_tunnel_info_test()
#-----------------------------------------------------------------------
macro(define_ovsp4rt_tunnel_info_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    $<TARGET_OBJECTS:ovsp4rt_test_client_o>
  )

  set_test_properties(${TARGET})

  target_include_directories(${TARGET} PUBLIC
    ${TESTS_SOURCE_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::tunnel_info_test
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

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
