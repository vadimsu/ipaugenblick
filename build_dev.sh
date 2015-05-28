./build_version.sh
cd dpdk-2.0.0
make install T=x86_64-native-linuxapp-gcc
cd ..
#rm -rf build
make  CURRENT_DIR=$(pwd)/ clean
make CURRENT_DIR=$(pwd)/
echo 'install libaries'
sudo cp dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/*.so.* /usr/lib/.
sudo cp stack_and_service/service/ipaugenblick_app_api/build/libipaugenblickservice.so /usr/lib/.
echo 'install headers'
sudo cp stack_and_service/service/ipaugenblick_app_api/ipaugenblick_api.h /usr/include/.
