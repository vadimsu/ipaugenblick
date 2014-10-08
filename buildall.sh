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
cd benchmark_app
rm -rf build
make CURRENT_DIR=$(pwd)/ clean
make CURRENT_DIR=$(pwd)/
./build_test.sh
cd ../benchmark_app2
rm -rf build
make CURRENT_DIR=$(pwd)/ clean 
make CURRENT_DIR=$(pwd)/
./build_test.sh
cd ../benchmark_app3
rm -rf build
make CURRENT_DIR=$(pwd)/ clean
make CURRENT_DIR=$(pwd)/
cd ../benchmark_app_udp_1
rm -rf build
make CURRENT_DIR=$(pwd)/ clean
make CURRENT_DIR=$(pwd)/
cd ../benchmark_app_udp_2
rm -rf build
make CURRENT_DIR=$(pwd)/ clean
make CURRENT_DIR=$(pwd)/
./build_test.sh
cd ../benchmark_app4
rm -rf build
make CURRENT_DIR=$(pwd)/ clean
make CURRENT_DIR=$(pwd)/
./build_test.sh
cd ../libuv_app
rm -rf build
make CURRENT_DIR=$(pwd)/ clean
make CURRENT_DIR=$(pwd)/
rm -rf libuv_api_libs
mkdir -p libuv_api_libs
cp ./build/libuv_app.a ./libuv_api_libs/.
cp ../dpdk-1.6.0r2/x86_64-default-linuxapp-gcc/lib/* ./libuv_api_libs/.
cp ../build/libnetinet.a ./libuv_api_libs/.
cp libuv.a ./libuv_api_libs/.
cd libuv_api_libs
rm -f libdpdk.a
for file in *.a; do
    ar -x "$file"
done
ar -rcs libdpdk.a *.o
cp libdpdk.a ../.
cd ..
./build_test.sh
echo "DONE"
