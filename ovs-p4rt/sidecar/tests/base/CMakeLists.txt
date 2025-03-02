# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0

#-----------------------------------------------------------------------
# P4INFO_TEXT
#-----------------------------------------------------------------------
if(DPDK_TARGET)
  set(P4INFO_TEXT ${TESTS_SOURCE_DIR}/dpdk/p4info_text.h)
elseif(ES2K_TARGET)
  set(P4INFO_TEXT ${TESTS_SOURCE_DIR}/es2k/p4info_text.h)
endif()

#-----------------------------------------------------------------------
# ovsp4rt_basic_test
#-----------------------------------------------------------------------
add_library(ovsp4rt_basic_test STATIC
  basic_test.cc
  basic_test.h
  test_main.cc
  ${P4INFO_TEXT}
)

target_include_directories(ovsp4rt_basic_test PRIVATE
  ${CURRENT_SOURCE_DIR}
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_basic_test PUBLIC
  absl::flags_parse
  p4runtime_proto
  stratum_utils
)

add_library(ovsp4rt::basic_test ALIAS ovsp4rt_basic_test)

#-----------------------------------------------------------------------
# ovsp4rt_mac_learn_info_test
#-----------------------------------------------------------------------
add_library(ovsp4rt_mac_learn_info_test STATIC
  base_mac_learn_info_test.cc
  base_mac_learn_info_test.h
)

target_include_directories(ovsp4rt_mac_learn_info_test PUBLIC
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_mac_learn_info_test PUBLIC
  ovsp4rt::basic_test
)

add_library(ovsp4rt::mac_learn_info_test ALIAS ovsp4rt_mac_learn_info_test)

#-----------------------------------------------------------------------
# ovsp4rt_mac_map_info_test
#-----------------------------------------------------------------------
add_library(ovsp4rt_mac_map_info_test STATIC
  base_mac_map_info_test.cc
  base_mac_map_info_test.h
)

target_include_directories(ovsp4rt_mac_map_info_test PUBLIC
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_mac_map_info_test PUBLIC
  ovsp4rt::basic_test
)

add_library(ovsp4rt::mac_map_info_test ALIAS ovsp4rt_mac_map_info_test)

#-----------------------------------------------------------------------
# ovsp4rt_tunnel_info_test
#-----------------------------------------------------------------------
add_library(ovsp4rt_tunnel_info_test STATIC
  base_tunnel_info_test.cc
  base_tunnel_info_test.h
)

target_include_directories(ovsp4rt_tunnel_info_test PUBLIC
  ${CURRENT_SOURCE_DIR}
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_tunnel_info_test PUBLIC
  ovsp4rt::basic_test
)

add_library(ovsp4rt::tunnel_info_test ALIAS ovsp4rt_tunnel_info_test)
