# es2k_tunnel_info_tests.cmake
#
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0
#

#-----------------------------------------------------------------------
# define_es2k_tunnel_info_test()
#-----------------------------------------------------------------------
macro(define_es2k_tunnel_info_test TARGET)
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

define_es2k_tunnel_info_test(config_decap_table_entry_test)
define_es2k_tunnel_info_test(config_encap_table_entry_test)
define_es2k_tunnel_info_test(config_tunnel_term_entry_test)
define_es2k_tunnel_info_test(es2k_config_rx_tunnel_port_entry_test)
define_es2k_tunnel_info_test(es2k_prep_decap_table_test)
define_es2k_tunnel_info_test(es2k_prep_encap_table_test)
define_es2k_tunnel_info_test(es2k_prep_rx_tunnel_src_port_test)
define_es2k_tunnel_info_test(es2k_prep_tunnel_term_entry_test)

