# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0

# Library and macro to define tests that use the BaseTunnelInfoTest
# base class.

#-----------------------------------------------------------------------
# ovsp4rt::tunnel_info_test
#-----------------------------------------------------------------------
add_library(tunnel_info_test STATIC
  tunnel_info_test.cc
  tunnel_info_test.h
)

target_include_directories(tunnel_info_test PUBLIC
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
  ${TESTS_SOURCE_DIR}
)

target_link_libraries(tunnel_info_test PUBLIC
  ovsp4rt::basic_test
)

add_library(ovsp4rt::tunnel_info_test ALIAS tunnel_info_test)

#-----------------------------------------------------------------------
# define_tunnel_info_test()
#-----------------------------------------------------------------------
macro(define_tunnel_info_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_include_directories(${TARGET} PUBLIC
    ${TESTS_SOURCE_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::test_client
    ovsp4rt::tunnel_info_test
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()
