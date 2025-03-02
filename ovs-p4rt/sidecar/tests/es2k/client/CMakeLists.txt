# CMake build file for ovs-p4rt/sidecar/tests/es2k/client
#
# Copyright 2024 Intel Corporation
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0
#

#-----------------------------------------------------------------------
# define_basic_test()
#-----------------------------------------------------------------------
macro(define_basic_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    $<TARGET_OBJECTS:ovsp4rt_test_client_o>
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::basic_test
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

#-----------------------------------------------------------------------
# define_es2k_config_test()
#-----------------------------------------------------------------------
macro(define_es2k_config_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    $<TARGET_OBJECTS:ovsp4rt_test_client_o>
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::tunnel_info_test
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

#-----------------------------------------------------------------------
# Basic tests
#-----------------------------------------------------------------------
# ::ovsp4rt::BasicTest subclasses
define_es2k_config_test(config_fdb_smac_table_entry_test)
define_es2k_config_test(config_vlan_pop_table_entry_test)
define_es2k_config_test(config_vlan_push_table_entry_test)
define_es2k_config_test(config_vsi_src_port_entry_test)
define_es2k_config_test(get_l2_to_tunnel_v4_entry_test)

# ::testing::Test subclasses
define_es2k_config_test(do_config_fdb_entry_test)
define_es2k_config_test(do_config_ip_mac_map_test)
define_es2k_config_test(do_config_rx_tunnel_src_port_test)
define_es2k_config_test(do_config_src_port_entry_test)
define_es2k_config_test(do_config_tunnel_entry_test)
define_es2k_config_test(do_config_tunnel_src_port_test)
define_es2k_config_test(do_config_vlan_entry_test)

define_es2k_config_test(es2k_update_src_port_test)
define_es2k_config_test(es2k_update_tunnel_info_test)

#-----------------------------------------------------------------------
# define_es2k_mac_map_test()
#-----------------------------------------------------------------------
macro(define_es2k_mac_map_test TARGET)
  define_es2k_config_test(${TARGET})
  target_link_libraries(${TARGET} PRIVATE ovsp4rt::mac_map_info_test)
endmacro()

define_es2k_mac_map_test(es2k_config_dst_ip_mac_map_test)
define_es2k_mac_map_test(es2k_config_src_ip_mac_map_test)
define_es2k_mac_map_test(get_vm_dst_table_entry_test)
define_es2k_mac_map_test(get_vm_src_table_entry_test)

#-----------------------------------------------------------------------
# es2k_mac_learn_info_tests
#-----------------------------------------------------------------------
include(es2k_mac_learn_info_tests.cmake)

#-----------------------------------------------------------------------
# es2k_tunnel_info_tests
#-----------------------------------------------------------------------
include(es2k_tunnel_info_tests.cmake)

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
