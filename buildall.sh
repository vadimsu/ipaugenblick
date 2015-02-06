cd dpdk-1.6.0r2
make install T=x86_64-default-linuxapp-gcc
cd ..
mkdir -p dpdk_libs
cp ./dpdk-1.6.0r2/x86_64-default-linuxapp-gcc/lib/* ./dpdk_libs/.
cd dpdk_libs
rm -f libdpdk.a
DPDK_LIBS=$(ls *.a)
echo $DPDK_LIBS
for file in *.a; do
    ar -x "$file"
done
ar -rcs libdpdk.a *.o
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
