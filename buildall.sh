cd dpdk-1.6.0r2
make install T=x86_64-default-linuxapp-gcc
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
