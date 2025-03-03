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

# Export list of unit tests.
set(UNIT_TEST_NAMES "${UNIT_TEST_NAMES}" PARENT_SCOPE)
