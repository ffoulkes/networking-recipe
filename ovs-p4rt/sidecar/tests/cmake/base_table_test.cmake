# Copyright 2024 Intel Corporation
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0

# Definitions for tests that use the BasicTableTest base class.

#-----------------------------------------------------------------------
# ovsp4rt::table_test
#-----------------------------------------------------------------------
add_library(ovsp4rt_table_test STATIC
  base_table_test.cc
  base_table_test.h
)

target_include_directories(ovsp4rt_table_test PRIVATE
  ${SIDECAR_SOURCE_DIR}
  ${STRATUM_SOURCE_DIR}
)

target_link_libraries(ovsp4rt_table_test PUBLIC
  absl::flags_parse
  ovsp4rt::p4info_helper
  ovsp4rt::p4info_text
  p4runtime_proto
  stratum_utils
)

add_library(ovsp4rt::table_test ALIAS ovsp4rt_table_test)

#-----------------------------------------------------------------------
# define_table_test()
#-----------------------------------------------------------------------
macro(define_table_test TARGET)
  add_executable(${TARGET}
    ${TARGET}.cc
  )

  set_test_properties(${TARGET})

  target_link_libraries(${TARGET} PUBLIC
    ovsp4rt::table_test
    ovsp4rt::test_main
  )

  list(APPEND UNIT_TEST_NAMES ${TARGET})
endmacro(define_table_test)
