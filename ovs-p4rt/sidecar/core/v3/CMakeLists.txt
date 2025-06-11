# CMake build file for ovs-p4rt/sidecar/core/v3
#
# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache 2.0
#

if(DPDK_TARGET)
    target_sources(ovsp4rt_core_o PRIVATE
        dpdk_config_fdb_entry.cc
        dpdk_config_tunnel_entry.cc
        dpdk_encoders.cc
        ovsp4rt_encode_utils.cc
        ovsp4rt_p4info_utils.cc
    )
elseif(ES2K_TARGET)
    target_sources(ovsp4rt_core_o PRIVATE
        do_config_ip_mac_map_entry.cc
        do_config_rx_tunnel_src_entry.cc
        do_config_src_port_entry.cc
        do_config_tunnel_src_port_entry.cc
        do_config_vlan_entry.cc
        es2k_config_fdb_entry.cc
        es2k_config_tunnel_entry.cc
        es2k_encoders.cc
        ovsp4rt_encode_utils.cc
        ovsp4rt_p4info_utils.cc
        tx_acc_vsi_table_entry.cc
        tx_acc_vsi_table_entry.h
    )
endif()
