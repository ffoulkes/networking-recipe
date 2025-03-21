# CMake build file for ovs-p4rt/sidecar/tests
#
# Copyright 2024 Intel Corporation
# Copyright 2025 Derek Foster
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
# ovsp4rt::basic_test
#-----------------------------------------------------------------------
add_library(ovsp4rt_basic_test STATIC
  basic_test.cc
  basic_test.h
)

target_include_directories(ovsp4rt_basic_test PRIVATE
  ${CURRENT_SOURCE_DIR}
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_basic_test PUBLIC
  absl::flags_parse
  ovsp4rt::p4info_text
  ovsp4rt::test_main
  p4runtime_proto
  stratum_utils
)

add_library(ovsp4rt::basic_test ALIAS ovsp4rt_basic_test)

#-----------------------------------------------------------------------
# define_basic_test()
#-----------------------------------------------------------------------
macro(define_basic_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_include_directories(${TARGET} PUBLIC
    ${TESTS_SOURCE_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::basic_test
    ovsp4rt::test_client
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

#-----------------------------------------------------------------------
# base_mac_learn_info_test
#-----------------------------------------------------------------------
add_library(base_mac_learn_info_test STATIC
  base_mac_learn_info_test.cc
  base_mac_learn_info_test.h
)

target_include_directories(base_mac_learn_info_test PUBLIC
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
  ${TESTS_SOURCE_DIR}
)

target_link_libraries(base_mac_learn_info_test PUBLIC
  ovsp4rt::basic_test
)

add_library(ovsp4rt::mac_learn_info_test ALIAS base_mac_learn_info_test)

#-----------------------------------------------------------------------
# define_mac_learn_info_test()
#-----------------------------------------------------------------------
macro(define_mac_learn_info_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_include_directories(${TARGET} PUBLIC
    ${TESTS_SOURCE_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::mac_learn_info_test
    ovsp4rt::test_client
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

#-----------------------------------------------------------------------
# base_mac_map_info_test
#-----------------------------------------------------------------------
add_library(base_mac_map_info_test STATIC
  base_mac_map_info_test.cc
  base_mac_map_info_test.h
)

target_include_directories(base_mac_map_info_test PUBLIC
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
  ${TESTS_SOURCE_DIR}
)

target_link_libraries(base_mac_map_info_test PUBLIC
  ovsp4rt::basic_test
)

add_library(ovsp4rt::mac_map_info_test ALIAS base_mac_map_info_test)

#-----------------------------------------------------------------------
# define_mac_map_info_test()
#-----------------------------------------------------------------------
macro(define_mac_map_info_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_include_directories(${TARGET} PUBLIC
    ${TESTS_SOURCE_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::mac_map_info_test
    ovsp4rt::test_client
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

#-----------------------------------------------------------------------
# base_tunnel_info_test
#-----------------------------------------------------------------------
add_library(base_tunnel_info_test STATIC
  base_tunnel_info_test.cc
  base_tunnel_info_test.h
)

target_include_directories(base_tunnel_info_test PUBLIC
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
  ${TESTS_SOURCE_DIR}
)

target_link_libraries(base_tunnel_info_test PUBLIC
  ovsp4rt::basic_test
)

add_library(ovsp4rt::tunnel_info_test ALIAS base_tunnel_info_test)

#-----------------------------------------------------------------------
# define_tunnel_info_test()
#-----------------------------------------------------------------------
macro(define_tunnel_info_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_include_directories(${TARGET} PUBLIC
    ${TESTS_SOURCE_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::test_client
    ovsp4rt::tunnel_info_test
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

#-----------------------------------------------------------------------
# Common tests
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
