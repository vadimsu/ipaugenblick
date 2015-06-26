./build_version.sh
cd dpdk-2.0.0
make install T=x86_64-native-linuxapp-gcc
cd ..
#rm -rf build 
make  CURRENT_DIR=$(pwd)/ clean RTE_SDK=$(pwd)/dpdk-2.0.0 LD_LIBRARY_PATH='stack_and_service/stack_and_service/x86_64-native-linuxapp-gcc/lib'
make CURRENT_DIR=$(pwd)/ RTE_SDK=$(pwd)/dpdk-2.0.0 LD_LIBRARY_PATH='stack_and_service/stack_and_service/x86_64-native-linuxapp-gcc/lib'
sudo cp  stack_and_service/service/stack_and_service/service/x86_64-native-linuxapp-gcc/ipaugenblick_srv /usr/bin/.
echo 'install libaries'
sudo mkdir -p /usr/lib/ipaugenblick
sudo cp dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/*.so /usr/lib/ipaugenblick/.
sudo cp dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/*.so.* /usr/lib/ipaugenblick/.
sudo cp stack_and_service/service/ipaugenblick_app_api/stack_and_service/service/ipaugenblick_app_api/x86_64-native-linuxapp-gcc/lib/libipaugenblickservice.so /usr/lib/ipaugenblick/.
sudo cp stack_and_service/stack_and_service/x86_64-native-linuxapp-gcc/lib/libnetinet.so /usr/lib/ipaugenblick/.
echo 'install headers'
sudo cp stack_and_service/service/ipaugenblick_app_api/ipaugenblick_api.h /usr/include/.
sudo mkdir -p /etc/ipaugenblick
sudo cp stack_and_service/service/dpdk_ip_stack_config.txt /etc/ipaugenblick
