# es2k_tunnel_info_tests.cmake
#
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0
#

#-----------------------------------------------------------------------
# es2k_tunnel_info
#-----------------------------------------------------------------------
add_library(es2k_tunnel_info STATIC
  base_tunnel_info_test.cc
  base_tunnel_info_test.h
  p4info_text.h
  test_main.cc
)

target_include_directories(es2k_tunnel_info PUBLIC
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
  ${OVSP4RT_INCLUDE_DIR}
)

target_link_libraries(es2k_tunnel_info PUBLIC
  absl::flags_parse
  p4runtime_proto
  stratum_utils
)

#-----------------------------------------------------------------------
# define_es2k_tunnel_info_test()
#-----------------------------------------------------------------------
macro(define_es2k_tunnel_info_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
    $<TARGET_OBJECTS:ovsp4rt_test_client_o>
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    es2k_tunnel_info
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()

define_es2k_tunnel_info_test(config_encap_table_entry_test)
define_es2k_tunnel_info_test(es2k_prep_decap_table_test)
define_es2k_tunnel_info_test(es2k_prep_encap_table_test)

