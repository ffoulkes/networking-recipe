# CMake build file for ovs-p4rt/sidecar/core/api
#
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache 2.0
#

target_sources(ovs_sidecar_o PRIVATE
    ovsp4rt_internal_api.h
    ovsp4rt_str_to_tunnel_type.cc
)

if(BUILD_JOURNAL)
    target_sources(ovs_sidecar_o PRIVATE ovsp4rt_journal_api.cc)
else()
    target_sources(ovs_sidecar_o PRIVATE ovsp4rt_standard_api.cc)
endif()
