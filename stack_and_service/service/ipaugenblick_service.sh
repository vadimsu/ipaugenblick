#!/bin/bash
export LD_LIBRARY_PATH=/usr/lib/ipaugenblick
/usr/bin/ipaugenblick_srv -c 3 -n 4 -d librte_pmd_i40e.so -d librte_pmd_ixgbe.so -- -p 0x3 -l 0
#/usr/bin/ipaugenblick_srv -c 3 -n 1 -- -p 0x3
