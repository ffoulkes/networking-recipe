# CMake build file for ovs-p4rt/sidecar/tests/dpdk
#
# Copyright 2024 Intel Corporation
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0
#

#-----------------------------------------------------------------------
# dpdk_ovsp4rt_test
#-----------------------------------------------------------------------
add_library(dpdk_ovsp4rt_test INTERFACE
  base_table_test.h
)

target_include_directories(dpdk_ovsp4rt_test INTERFACE
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
)

target_link_libraries(dpdk_ovsp4rt_test INTERFACE
  absl::flags_parse
  ovsp4rt::p4info_helper
  ovsp4rt::p4info_text
  p4runtime_proto
  stratum_utils
)

#-----------------------------------------------------------------------
# define_dpdk_test()
#-----------------------------------------------------------------------
macro(define_dpdk_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    dpdk_ovsp4rt_test
    ovsp4rt::test_main
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro(define_dpdk_test)

#-----------------------------------------------------------------------
# DPDK PrepareTableEntry unit tests
#-----------------------------------------------------------------------
define_dpdk_test(dpdk_fdb_rx_vlan_test)
define_dpdk_test(dpdk_fdb_tx_vlan_test)
define_dpdk_test(dpdk_fdb_tx_vxlan_test)

define_dpdk_test(dpdk_tunnel_term_test)

define_dpdk_test(dpdk_vxlan_encap_test)

#-----------------------------------------------------------------------
# define_dpdk_config_test()
#-----------------------------------------------------------------------
macro(define_dpdk_config_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    absl::flags_parse
    ovsp4rt::test_client
    ovsp4rt::test_main
    p4runtime_proto
    stratum_utils
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro(define_dpdk_config_test)

#-----------------------------------------------------------------------
# DoConfigEntry tests
#-----------------------------------------------------------------------
if(BUILD_CLIENT)
  define_dpdk_config_test(dpdk_do_config_fdb_entry_test)
  define_dpdk_config_test(dpdk_do_config_tunnel_entry_test)
endif()

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
