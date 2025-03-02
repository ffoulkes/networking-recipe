# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0

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
# Tests that require TestClientMock
#-----------------------------------------------------------------------
if(BUILD_CLIENT)

#-----------------------------------------------------------------------
# define_basic_test()
#-----------------------------------------------------------------------
macro(define_basic_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    $<TARGET_OBJECTS:ovsp4rt_test_client_o>
  )

  set_test_properties(${TARGET})

  target_include_directories(${TARGET} PRIVATE
    ${TESTS_SOURCE_DIR}/base
  )

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::basic_test
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

#-----------------------------------------------------------------------
# basic tests
#-----------------------------------------------------------------------
define_basic_test(config_fdb_rx_vlan_table_entry_test)
define_basic_test(config_fdb_tx_vlan_table_entry_test)

#-----------------------------------------------------------------------
# define_tunnel_info_test()
#-----------------------------------------------------------------------
macro(define_tunnel_info_test TARGET)
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
# tunnel_info tests
#-----------------------------------------------------------------------
define_tunnel_info_test(config_encap_table_entry_test)
define_tunnel_info_test(config_tunnel_term_entry_test)

endif(BUILD_CLIENT)

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
