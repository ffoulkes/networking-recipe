# es2k_mac_learn_info_tests.cmake
#
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0
#

#-----------------------------------------------------------------------
# define_mac_learn_info_test()
#-----------------------------------------------------------------------
macro(define_mac_learn_info_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    $<TARGET_OBJECTS:ovsp4rt_test_client_o>
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::mac_learn_info_test
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

#-----------------------------------------------------------------------
# mac_learn_info tests
#-----------------------------------------------------------------------
define_mac_learn_info_test(config_fdb_tunnel_table_entry_test)
define_mac_learn_info_test(es2k_config_l2_tunnel_table_entry_test)
define_mac_learn_info_test(es2k_prep_l2_tunnel_table_entry)
define_mac_learn_info_test(es2k_prep_fdb_table_v4_tunnel_test)
define_mac_learn_info_test(es2k_prep_fdb_tunnel_table_test)
define_mac_learn_info_test(get_fdb_tunnel_entry_test)
define_mac_learn_info_test(get_fdb_vlan_entry_test)
define_mac_learn_info_test(get_l2_to_tunnel_v6_entry_test)
