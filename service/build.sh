rm -f bm
cd ipaugenblick_memory_common
./build_ipaugenblick_memory_common.sh
cd ..
cd ipaugenblick_memory_service
./build_ipaugenblick_memory_service.sh
cd ..
cd ipaugenblick_app_api
./build_ipaugenblick_api.sh
cd ..
gcc ipaugenblick_main.c build/libipaugenblickservice.a  ipaugenblick_memory_service/libipaugenblick_memory_service.a ipaugenblick_memory_common/libipaugenblick_memory_common.a ../build/libnetinet.a ../dpdk_libs/libdpdk.a  -lpthread -lrt -ldl -o ipaugenblick_service
