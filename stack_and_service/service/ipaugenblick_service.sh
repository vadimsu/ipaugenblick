#!/bin/bash
export LD_LIBRARY_PATH=/usr/lib/ipaugenblick
#/usr/bin/ipaugenblick_srv -c 3 -n 1 --no-huge -d librte_pmd_ring.so -- -p 0x3 
/usr/bin/ipaugenblick_srv -c 3 -n 1 -- -p 0x3
