# es2k_mac_learn_info_tests.cmake
#
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0
#

#-----------------------------------------------------------------------
# es2k_mac_learn_info
#-----------------------------------------------------------------------
add_library(es2k_mac_learn_info STATIC
  base_mac_learn_info_test.cc
  base_mac_learn_info_test.h
  p4info_text.h
  test_main.cc
)

target_include_directories(es2k_mac_learn_info PUBLIC
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(es2k_mac_learn_info PUBLIC
  absl::flags_parse
  p4runtime_proto
  stratum_utils
)

#-----------------------------------------------------------------------
# define_es2k_mac_learn_info()
#-----------------------------------------------------------------------
macro(define_es2k_mac_learn_info TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    $<TARGET_OBJECTS:ovsp4rt_test_client_o>
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    es2k_mac_learn_info
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()


define_es2k_mac_learn_info(config_fdb_tunnel_table_entry_test)
define_es2k_mac_learn_info(es2k_config_l2_tunnel_table_entry_test)
define_es2k_mac_learn_info(es2k_prep_l2_tunnel_table_entry)
define_es2k_mac_learn_info(es2k_prep_fdb_table_v4_tunnel_test)
define_es2k_mac_learn_info(es2k_prep_fdb_tunnel_table_test)
define_es2k_mac_learn_info(get_fdb_tunnel_entry_test)
define_es2k_mac_learn_info(get_fdb_vlan_entry_test)
define_es2k_mac_learn_info(get_l2_to_tunnel_v6_entry_test)
