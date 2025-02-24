# CMake build file for ovs-p4rt/sidecar/tests/es2k/extra
#
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0
#

include_directories(..)

#-----------------------------------------------------------------------
# define_es2k_extra_test()
#-----------------------------------------------------------------------
macro(define_es2k_extra_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    test_main.cc
    $<TARGET_OBJECTS:ovsp4rt_test_client_o>
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    absl::flags_parse
    p4runtime_proto
    stratum_utils
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro(define_es2k_extra_test)

#-----------------------------------------------------------------------
# define_es2k_extra_test()
#-----------------------------------------------------------------------
define_es2k_extra_test(es2k_config_fdb_entry_test)
define_es2k_extra_test(es2k_config_ip_mac_map_test)
define_es2k_extra_test(es2k_config_rx_tunnel_src_port_test)
define_es2k_extra_test(es2k_config_src_port_entry_test)
define_es2k_extra_test(es2k_config_tunnel_entry_test)
define_es2k_extra_test(es2k_config_tunnel_src_port_test)
define_es2k_extra_test(es2k_config_vlan_entry_test)

#define_es2k_extra_test(es2k_update_tunnel_info_test)

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
