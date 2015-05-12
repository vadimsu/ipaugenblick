export LD_LIBRARY_PATH=../build/lib:../dpdk-1.8.0/x86_64-native-linuxapp-gcc/lib
./ipaugenblick_srv -c 3 -n 1 -d librte_pmd_ixgbe.so -- -p 1
