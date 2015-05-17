cd dpdk-2.0.0
make install T=x86_64-native-linuxapp-gcc
cd ..
rm -rf build
make  CURRENT_DIR=$(pwd)/ clean
make CURRENT_DIR=$(pwd)/
cd service
rm -rf build
make CURRENT_DIR=$(pwd)/ clean
make CURRENT_DIR=$(pwd)/
cp build/ipaugenblick_srv .
cd ipaugenblick_app_api
rm -rf build
make CURRENT_DIR=$(pwd)/ clean
make CURRENT_DIR=$(pwd)/
cd ../..
echo 'install libaries'
sudo cp dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/*.so.* /usr/lib/.
sudo cp service/ipaugenblick_app_api/build/libipaugenblickservice.so /usr/lib/.
echo 'install headers'
sudo cp service/ipaugenblick_app_api/ipaugenblick_api.h /usr/include/.
