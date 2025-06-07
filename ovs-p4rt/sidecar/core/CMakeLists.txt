# CMake build file for ovs-p4rt/sidecar/core
#
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache 2.0
#

option(OVSP4RT_V3 "Build experimental version" OFF)

add_subdirectory(common)

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
endif()

if(OVSP4RT_V3)
    # Experimental (v3) version.
    add_subdirectory(api)
    add_subdirectory(v3)
elseif(BUILD_CLIENT)
    # Revised (v2) implementation.
    add_subdirectory(api)
    add_subdirectory(v2)
else()
    # Original (v1) implementation.
    add_subdirectory(v1)
endif()
