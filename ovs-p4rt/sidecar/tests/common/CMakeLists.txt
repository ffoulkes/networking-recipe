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
# Client-mode tests
#-----------------------------------------------------------------------
if(BUILD_CLIENT)

define_basic_test(config_fdb_rx_vlan_table_entry_test)
define_basic_test(config_fdb_tx_vlan_table_entry_test)

define_mac_learn_info_test(config_fdb_tunnel_table_entry_test)

define_tunnel_info_test(config_encap_table_entry_test)
define_tunnel_info_test(config_tunnel_term_entry_test)

endif(BUILD_CLIENT)

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
