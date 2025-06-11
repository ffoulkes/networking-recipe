# CMake build file for ovs-p4rt/sidecar/core/common
#
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache 2.0
#

target_sources(ovsp4rt_core_o PRIVATE
    # Internal interface headers.
    ovsp4rt_encode_utils.h
    ovsp4rt_encoders.h
    ovsp4rt_entry_utils.h
    ovsp4rt_p4info_utils.h
)
