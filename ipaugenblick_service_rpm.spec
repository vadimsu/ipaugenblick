Summary: IPAugenblick service
Name: ipaugenblick_srv
Version: 1.0
Release: 1
License: GPL
Group: Applications/Networking
Packager: Vadim Suraev <vadim.suraev@gmail.com>

%description
User-space full IP stack integrated with DPDK
%prep
echo $RPM_BUILD_ROOT
echo $RPM_BUILD_DIR
echo $(pwd)
rm -rf $RPM_BUILD_ROOT/ipaugenblick
rm -rf $RPM_BUILD_DIR/ipaugenblick
%build
git clone https://github.com/vadimsu/ipaugenblick
cd ipaugenblick
git checkout dpdk-2.0
./buildall.sh
%install
mkdir -p $RPM_BUILD_ROOT/usr/lib/ipaugenblick
mkdir -p $RPM_BUILD_ROOT/usr/bin
cd $RPM_BUILD_DIR
cp $RPM_BUILD_DIR/ipaugenblick/dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/librte_eal.so* $RPM_BUILD_ROOT/usr/lib/ipaugenblick/.
cp $RPM_BUILD_DIR/ipaugenblick/dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/librte_timer.so* $RPM_BUILD_ROOT/usr/lib/ipaugenblick/.
cp $RPM_BUILD_DIR/ipaugenblick/dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/librte_ring.so* $RPM_BUILD_ROOT/usr/lib/ipaugenblick/.
cp $RPM_BUILD_DIR/ipaugenblick/dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/librte_mbuf.so* $RPM_BUILD_ROOT/usr/lib/ipaugenblick/.
cp $RPM_BUILD_DIR/ipaugenblick/dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/librte_mempool.so* $RPM_BUILD_ROOT/usr/lib/ipaugenblick/.
cp $RPM_BUILD_DIR/ipaugenblick/dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/librte_malloc.so* $RPM_BUILD_ROOT/usr/lib/ipaugenblick/.
cp $RPM_BUILD_DIR/ipaugenblick/dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/librte_pmd_ixgbe.so* $RPM_BUILD_ROOT/usr/lib/ipaugenblick/.
cp $RPM_BUILD_DIR/ipaugenblick/dpdk-2.0.0/x86_64-native-linuxapp-gcc/lib/libethdev.so* $RPM_BUILD_ROOT/usr/lib/ipaugenblick/.
cp $RPM_BUILD_DIR/ipaugenblick/build/libnetinet.so $RPM_BUILD_ROOT/usr/lib/ipaugenblick/.
cp $RPM_BUILD_DIR/ipaugenblick/service/build/ipaugenblick_srv $RPM_BUILD_ROOT/usr/bin/ipaugenblick_srv
cp $RPM_BUILD_DIR/ipaugenblick/service/ipaugenblick_app_api/build/libipaugenblickservice.so $RPM_BUILD_ROOT/usr/lib/ipaugenblick/.
%files
%defattr(-,root,root,-)
/usr/lib/ipaugenblick/librte_eal.so*
/usr/lib/ipaugenblick/librte_timer.so*
/usr/lib/ipaugenblick/librte_ring.so*
/usr/lib/ipaugenblick/librte_mbuf.so*
/usr/lib/ipaugenblick/librte_mempool.so*
/usr/lib/ipaugenblick/librte_malloc.so*
/usr/lib/ipaugenblick/librte_pmd_ixgbe.so*
/usr/lib/ipaugenblick/libethdev.so*
/usr/lib/ipaugenblick/libnetinet.so
/usr/lib/ipaugenblick/libipaugenblickservice.so
/usr/bin/ipaugenblick_srv
