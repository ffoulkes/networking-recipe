# Copyright 2025 Derek Foster
# SPDX-License-Identifier: Apache-2.0

#-----------------------------------------------------------------------
# basic tests
#-----------------------------------------------------------------------
define_ovsp4rt_basic_test(config_fdb_smac_table_entry_test)
define_ovsp4rt_basic_test(config_fdb_rx_vlan_table_entry_test)
define_ovsp4rt_basic_test(config_fdb_tx_vlan_table_entry_test)
define_ovsp4rt_basic_test(config_vlan_pop_table_entry_test)
define_ovsp4rt_basic_test(config_vlan_push_table_entry_test)
define_ovsp4rt_basic_test(config_vsi_src_port_entry_test)
define_ovsp4rt_basic_test(get_l2_to_tunnel_v4_entry_test)

define_ovsp4rt_basic_test(do_config_fdb_entry_test)
define_ovsp4rt_basic_test(do_config_ip_mac_map_test)
define_ovsp4rt_basic_test(do_config_rx_tunnel_src_port_test)
define_ovsp4rt_basic_test(do_config_src_port_entry_test)
define_ovsp4rt_basic_test(do_config_tunnel_entry_test)
define_ovsp4rt_basic_test(do_config_tunnel_src_port_test)
define_ovsp4rt_basic_test(do_config_vlan_entry_test)

define_ovsp4rt_basic_test(es2k_update_src_port_test)
define_ovsp4rt_basic_test(es2k_update_tunnel_info_test)

#-----------------------------------------------------------------------
# mac_learn_info tests
#-----------------------------------------------------------------------
define_mac_learn_info_test(config_fdb_tunnel_table_entry_test)
define_mac_learn_info_test(es2k_config_l2_tunnel_table_entry_test)
define_mac_learn_info_test(es2k_prep_l2_tunnel_table_entry)
define_mac_learn_info_test(es2k_prep_fdb_table_v4_tunnel_test)
define_mac_learn_info_test(es2k_prep_fdb_tunnel_table_test)
define_mac_learn_info_test(get_fdb_tunnel_entry_test)
define_mac_learn_info_test(get_fdb_vlan_entry_test)
define_mac_learn_info_test(get_l2_to_tunnel_v6_entry_test)

#-----------------------------------------------------------------------
# mac_map_info tests
#-----------------------------------------------------------------------
define_mac_map_info_test(es2k_config_dst_ip_mac_map_test)
define_mac_map_info_test(es2k_config_src_ip_mac_map_test)
define_mac_map_info_test(get_vm_dst_table_entry_test)
define_mac_map_info_test(get_vm_src_table_entry_test)

#-----------------------------------------------------------------------
# tunnel_info tests
#-----------------------------------------------------------------------
define_tunnel_info_test(config_decap_table_entry_test)
define_tunnel_info_test(config_encap_table_entry_test)
define_tunnel_info_test(config_tunnel_term_entry_test)
define_tunnel_info_test(es2k_config_rx_tunnel_port_entry_test)
define_tunnel_info_test(es2k_prep_decap_table_test)
define_tunnel_info_test(es2k_prep_encap_table_test)
define_tunnel_info_test(es2k_prep_rx_tunnel_src_port_test)
define_tunnel_info_test(es2k_prep_tunnel_term_entry_test)

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
