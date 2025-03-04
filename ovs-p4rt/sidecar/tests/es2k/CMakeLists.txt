# CMake build file for ovs-p4rt/sidecar/tests/es2k
#
# Copyright 2024 Intel Corporation
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0
#

#-----------------------------------------------------------------------
# base_mac_map_info_test
#-----------------------------------------------------------------------
add_library(base_mac_map_info_test STATIC
  base_mac_map_info_test.cc
  base_mac_map_info_test.h
)

target_include_directories(base_mac_map_info_test PUBLIC
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(base_mac_map_info_test PUBLIC
  p4runtime_proto
  stratum_utils
)

#-----------------------------------------------------------------------
# es2k_ovsp4rt_test
#-----------------------------------------------------------------------
add_library(es2k_ovsp4rt_test INTERFACE
  base_table_test.h
  p4info_helper.h
  p4info_text.h
)

target_include_directories(es2k_ovsp4rt_test INTERFACE
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
)

target_link_libraries(es2k_ovsp4rt_test INTERFACE
  absl::flags_parse
  p4runtime_proto
  stratum_utils
)

#-----------------------------------------------------------------------
# define_es2k_test()
#-----------------------------------------------------------------------
macro(define_es2k_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    test_main.cc
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    es2k_ovsp4rt_test
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro(define_es2k_test)

#-----------------------------------------------------------------------
# tunnel_test_utils
#-----------------------------------------------------------------------
add_library(tunnel_test_utils STATIC
  base_table_test.h
  ${TESTS_SOURCE_DIR}/basic_test.h
  ip_tunnel_test.h
  p4info_text.h
  test_main.cc
)

target_link_libraries(tunnel_test_utils PUBLIC
  absl::flags_parse
)

#-----------------------------------------------------------------------
# define_es2k_tunnel_test()
#-----------------------------------------------------------------------
macro(define_es2k_tunnel_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    tunnel_test_utils
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro(define_es2k_tunnel_test)

#-----------------------------------------------------------------------
# ES2K PrepareTableEntry unit tests
#-----------------------------------------------------------------------
define_es2k_test(fdb_rx_vlan_entry_test)
define_es2k_test(fdb_smac_entry_test)
define_es2k_test(fdb_tx_geneve_entry_test)
define_es2k_test(fdb_tx_vlan_entry_test)
define_es2k_test(fdb_tx_vxlan_entry_test)

define_es2k_test(geneve_decap_mod_table_test)
define_es2k_test(geneve_decap_mod_vlan_push_test)

define_es2k_tunnel_test(geneve_encap_v4_table_test)
define_es2k_tunnel_test(geneve_encap_v6_table_test)
define_es2k_tunnel_test(geneve_encap_v4_vlan_pop_test)
define_es2k_tunnel_test(geneve_encap_v6_vlan_pop_test)

define_es2k_test(src_ip_mac_map_table_test)
define_es2k_test(dst_ip_mac_map_table_test)

define_es2k_test(l2_to_v4_tunnel_test)
define_es2k_test(l2_to_v6_tunnel_test)

define_es2k_test(src_port_table_test)

define_es2k_test(rx_tunnel_v4_table_test)
define_es2k_test(rx_tunnel_v6_table_test)

define_es2k_test(tunnel_term_v4_table_test)
define_es2k_test(tunnel_term_v6_table_test)

define_es2k_test(tx_acc_vsi_table_test)

define_es2k_test(vlan_pop_mod_table_test)
define_es2k_test(vlan_push_mod_table_test)

define_es2k_test(vxlan_decap_mod_table_test)
define_es2k_test(vxlan_decap_mod_vlan_push_test)

define_es2k_tunnel_test(vxlan_encap_v4_table_test)
define_es2k_tunnel_test(vxlan_encap_v6_table_test)
define_es2k_tunnel_test(vxlan_encap_v4_vlan_pop_test)
define_es2k_tunnel_test(vxlan_encap_v6_vlan_pop_test)

#-----------------------------------------------------------------------
# client tests
#-----------------------------------------------------------------------
if(BUILD_CLIENT)
  add_subdirectory(client)
endif()

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
    tunnel_test_utils
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

#-----------------------------------------------------------------------
# define_es2k_mac_map_test()
#-----------------------------------------------------------------------
macro(define_es2k_mac_map_test TARGET)
  define_es2k_config_test(${TARGET})
  target_link_libraries(${TARGET} PRIVATE base_mac_map_info_test)
endmacro()

define_es2k_mac_map_test(es2k_config_dst_ip_mac_map_test)
define_es2k_mac_map_test(es2k_config_src_ip_mac_map_test)
define_es2k_mac_map_test(get_vm_dst_table_entry_test)
define_es2k_mac_map_test(get_vm_src_table_entry_test)

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
