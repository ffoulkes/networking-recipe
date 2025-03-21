# Copyright 2024 Intel Corporation
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0

# Library and macro to define tests that use the BasicTest base
# class.

#-----------------------------------------------------------------------
# ovsp4rt::basic_test
#-----------------------------------------------------------------------
add_library(ovsp4rt_basic_test STATIC
  basic_test.cc
  basic_test.h
)

target_include_directories(ovsp4rt_basic_test PRIVATE
  ${CURRENT_SOURCE_DIR}
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_basic_test PUBLIC
  absl::flags_parse
  ovsp4rt::p4info_text
  ovsp4rt::test_main
  p4runtime_proto
  stratum_utils
)

add_library(ovsp4rt::basic_test ALIAS ovsp4rt_basic_test)

#-----------------------------------------------------------------------
# define_basic_test()
#-----------------------------------------------------------------------
macro(define_basic_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_include_directories(${TARGET} PUBLIC
    ${TESTS_SOURCE_DIR}
  )

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::basic_test
    ovsp4rt::test_client
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro()
