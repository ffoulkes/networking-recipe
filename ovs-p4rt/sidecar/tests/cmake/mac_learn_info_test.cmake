# Copyright 2024 Intel Corporation
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0

# Library and macro to define tests that use the BaseMacLearnInfoTest
# base class.

#-----------------------------------------------------------------------
# ovsp4rt::mac_learn_info_test
#-----------------------------------------------------------------------
add_library(mac_learn_info_test STATIC
  mac_learn_info_test.cc
  mac_learn_info_test.h
)

target_include_directories(mac_learn_info_test PUBLIC
  ${OVSP4RT_INCLUDE_DIR}
  ${SIDECAR_SOURCE_DIR}
  ${TESTS_SOURCE_DIR}
)

target_link_libraries(mac_learn_info_test PUBLIC
  ovsp4rt::basic_test
)

add_library(ovsp4rt::mac_learn_info_test ALIAS mac_learn_info_test)

#-----------------------------------------------------------------------
# define_mac_learn_info_test()
#-----------------------------------------------------------------------
macro(define_mac_learn_info_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_include_directories(${TARGET} PUBLIC
    ${TESTS_SOURCE_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::mac_learn_info_test
    ovsp4rt::test_client
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()
