# CMake build file for ovs-p4rt/sidecar/core
#
# Copyright 2025 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

target_sources(ovs_sidecar_o PRIVATE
    # Internal interface headers.
    ovsp4rt_config_int.h
    ovsp4rt_encoders.h
    ovsp4rt_util_int.h
)

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
endif()

if(BUILD_CLIENT)
    # Revised (v2) implementation.
    add_subdirectory(v2)
else()
    # Original (v1) implementation.
    add_subdirectory(v1)
endif()
