# CMake build file for ovs-p4rt/sidecar/core/v2
#
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache 2.0
#

target_sources(ovs_sidecar_o PRIVATE
    newovsp4rt.cc
    ovsp4rt_api_utils.cc
    ovsp4rt_do_config_int.h
    ovsp4rt_encoders.cc
)

if(BUILD_JOURNAL)
    target_sources(ovs_sidecar_o PRIVATE ovsp4rt_journal_api.cc)
else()
    target_sources(ovs_sidecar_o PRIVATE ovsp4rt_standard_api.cc)
endif()
