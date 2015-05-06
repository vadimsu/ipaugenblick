/*
 * show_mib_stats.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains functions to show Linux kernel TCP/IP MIB stats
 *  for the Linux TCP/IP ported to userland and integrated with DPDK 1.6
 */
#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
#include <specific_includes/net/net_namespace.h>

void show_mib_stats(void)
{
	struct netns_mib *p_mib = &init_net.mib;
	printf(" IPSTATS_MIB_INPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INPKTS]);
	printf(" IPSTATS_MIB_INOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INOCTETS]);
	printf(" IPSTATS_MIB_INDELIVERS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INDELIVERS]);
	printf(" IPSTATS_MIB_OUTFORWDATAGRAMS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTFORWDATAGRAMS]);
	printf(" IPSTATS_MIB_OUTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTPKTS]);
	printf(" IPSTATS_MIB_OUTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTOCTETS]);
	printf(" IPSTATS_MIB_INHDRERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INHDRERRORS]);
	printf(" IPSTATS_MIB_INTOOBIGERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INTOOBIGERRORS]);
	printf(" IPSTATS_MIB_INNOROUTES %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INNOROUTES]);
	printf(" IPSTATS_MIB_INADDRERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INADDRERRORS]);
	printf(" IPSTATS_MIB_INUNKNOWNPROTOS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INUNKNOWNPROTOS]);
	printf(" IPSTATS_MIB_INTRUNCATEDPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INTRUNCATEDPKTS]);
	printf(" IPSTATS_MIB_INDISCARDS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INDISCARDS]);
	printf(" IPSTATS_MIB_OUTDISCARDS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTDISCARDS]);
	printf(" IPSTATS_MIB_OUTNOROUTES %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTNOROUTES]);
	printf(" IPSTATS_MIB_REASMTIMEOUT %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMTIMEOUT]);
	printf(" IPSTATS_MIB_REASMREQDS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMREQDS]);
	printf(" IPSTATS_MIB_REASMOKS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMOKS]);
	printf(" IPSTATS_MIB_REASMFAILS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMFAILS]);
	printf(" IPSTATS_MIB_FRAGOKS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGOKS]);
	printf(" IPSTATS_MIB_FRAGFAILS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGFAILS]);
	printf(" IPSTATS_MIB_FRAGCREATES %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGCREATES]);
	//printf(" IPSTATS_MIB_INMCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INMCASTPKTS]);
	//printf(" IPSTATS_MIB_OUTMCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTMCASTPKTS]);
	//printf(" IPSTATS_MIB_INBCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INBCASTPKTS]);
	//printf(" IPSTATS_MIB_OUTBCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTBCASTPKTS]);
	//printf(" IPSTATS_MIB_INMCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INMCASTOCTETS]);
	//printf(" IPSTATS_MIB_OUTMCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTMCASTOCTETS]);
	//printf(" IPSTATS_MIB_INBCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INBCASTOCTETS]);
	//printf(" IPSTATS_MIB_OUTBCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTBCASTOCTETS]);
	printf(" IPSTATS_MIB_CSUMERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_CSUMERRORS]);
	//printf(" IPSTATS_MIB_NOECTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_NOECTPKTS]);
	//printf(" IPSTATS_MIB_ECT1PKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_ECT1PKTS]);
	//printf(" IPSTATS_MIB_ECT0PKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_ECT0PKTS]);
	printf(" IPSTATS_MIB_CEPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_CEPKTS]);

	printf(" LINUX_MIB_DELAYEDACKS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_DELAYEDACKS]);
	printf(" LINUX_MIB_DELAYEDACKLOST %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_DELAYEDACKLOST]);
	printf(" LINUX_MIB_TCPPREQUEUED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPREQUEUED]);
	printf(" LINUX_MIB_TCPDIRECTCOPYFROMBACKLOG %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDIRECTCOPYFROMBACKLOG]);
	printf(" LINUX_MIB_TCPDIRECTCOPYFROMPREQUEUE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDIRECTCOPYFROMPREQUEUE]);
	printf(" LINUX_MIB_TCPPREQUEUEDROPPED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPREQUEUEDROPPED]);
	printf(" LINUX_MIB_TCPHPHITS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPHITS]);
	printf(" LINUX_MIB_TCPHPHITSTOUSER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPHITSTOUSER]);
	printf(" LINUX_MIB_TCPPUREACKS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPUREACKS]);
	printf(" LINUX_MIB_TCPHPACKS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPACKS]);
	printf(" LINUX_MIB_TCPRENORECOVERY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENORECOVERY]);
	printf(" LINUX_MIB_TCPSACKRECOVERY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKRECOVERY]);
	printf(" LINUX_MIB_TCPSACKRENEGING %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKRENEGING]);
	printf(" LINUX_MIB_TCPFACKREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFACKREORDER]);
	printf(" LINUX_MIB_TCPSACKREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKREORDER]);
	printf(" LINUX_MIB_TCPRENOREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENOREORDER]);
	printf(" LINUX_MIB_TCPTSREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTSREORDER]);
	printf(" LINUX_MIB_TCPFULLUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFULLUNDO]);
	printf(" LINUX_MIB_TCPPARTIALUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPARTIALUNDO]);
	printf(" LINUX_MIB_DELAYEDACKLOCKED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_DELAYEDACKLOCKED]);
	printf(" LINUX_MIB_TCPDSACKUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKUNDO]);
	printf(" LINUX_MIB_TCPLOSSUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSUNDO]);
	printf(" LINUX_MIB_TCPLOSTRETRANSMIT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSTRETRANSMIT]);
	printf(" LINUX_MIB_TCPRENOFAILURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENOFAILURES]);
	printf(" LINUX_MIB_TCPSACKFAILURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKFAILURES]);
	printf(" LINUX_MIB_TCPLOSSFAILURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSFAILURES]);
	printf(" LINUX_MIB_TCPFASTRETRANS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTRETRANS]);
	printf(" LINUX_MIB_TCPFORWARDRETRANS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFORWARDRETRANS]);
	printf(" LINUX_MIB_TCPSLOWSTARTRETRANS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSLOWSTARTRETRANS]);
	printf(" LINUX_MIB_TCPTIMEOUTS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTIMEOUTS]);
	printf(" LINUX_MIB_TCPLOSSPROBES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSPROBES]);
	printf(" LINUX_MIB_TCPLOSSPROBERECOVERY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSPROBERECOVERY]);
	printf(" LINUX_MIB_TCPSCHEDULERFAILED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSCHEDULERFAILED]);
	printf(" LINUX_MIB_TCPRCVCOLLAPSED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRCVCOLLAPSED]);
	printf(" LINUX_MIB_TCPDSACKOLDSENT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOLDSENT]);
	printf(" LINUX_MIB_TCPDSACKOFOSENT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOFOSENT]);
	printf(" LINUX_MIB_TCPDSACKRECV %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKRECV]);
	printf(" LINUX_MIB_TCPDSACKOFORECV %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOFORECV]);
	printf(" LINUX_MIB_TCPABORTONDATA %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONDATA]);
	printf(" LINUX_MIB_TCPABORTONCLOSE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONCLOSE]);
	printf(" LINUX_MIB_TCPABORTONMEMORY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONMEMORY]);
	printf(" LINUX_MIB_TCPABORTONTIMEOUT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONTIMEOUT]);
	printf(" LINUX_MIB_TCPABORTONLINGER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONLINGER]);
	printf(" LINUX_MIB_TCPABORTFAILED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTFAILED]);
	printf(" LINUX_MIB_TCPMEMORYPRESSURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMEMORYPRESSURES]);
	printf(" LINUX_MIB_TCPSACKDISCARD %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKDISCARD]);
	printf(" LINUX_MIB_TCPDSACKIGNOREDOLD %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKIGNOREDOLD]);
	printf(" LINUX_MIB_TCPDSACKIGNOREDNOUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKIGNOREDNOUNDO]);
	printf(" LINUX_MIB_TCPSPURIOUSRTOS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSPURIOUSRTOS]);
	printf(" LINUX_MIB_TCPMD5NOTFOUND %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMD5NOTFOUND]);
	printf(" LINUX_MIB_TCPMD5UNEXPECTED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMD5UNEXPECTED]);
	printf(" LINUX_MIB_SACKSHIFTED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKSHIFTED]);
	printf(" LINUX_MIB_SACKMERGED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKMERGED]);
	printf(" LINUX_MIB_SACKSHIFTFALLBACK %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKSHIFTFALLBACK]);
	printf(" LINUX_MIB_TCPBACKLOGDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPBACKLOGDROP]);
	printf(" LINUX_MIB_TCPMINTTLDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMINTTLDROP]);
	printf(" LINUX_MIB_TCPDEFERACCEPTDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDEFERACCEPTDROP]);
	printf(" LINUX_MIB_IPRPFILTER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_IPRPFILTER]);
	printf(" LINUX_MIB_TCPTIMEWAITOVERFLOW %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTIMEWAITOVERFLOW]);
	printf(" LINUX_MIB_TCPREQQFULLDOCOOKIES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPREQQFULLDOCOOKIES]);
	printf(" LINUX_MIB_TCPREQQFULLDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPREQQFULLDROP]);
	printf(" LINUX_MIB_TCPRETRANSFAIL %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRETRANSFAIL]);
	printf(" LINUX_MIB_TCPRCVCOALESCE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRCVCOALESCE]);
	printf(" LINUX_MIB_TCPOFOQUEUE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFOQUEUE]);
	printf(" LINUX_MIB_TCPOFODROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFODROP]);
	printf(" LINUX_MIB_TCPOFOMERGE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFOMERGE]);
	printf(" LINUX_MIB_TCPCHALLENGEACK %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPCHALLENGEACK]);
	printf(" LINUX_MIB_TCPSYNCHALLENGE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSYNCHALLENGE]);
	printf(" LINUX_MIB_TCPFASTOPENACTIVE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENACTIVE]);
	printf(" LINUX_MIB_TCPFASTOPENPASSIVE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENPASSIVE]);
	printf(" LINUX_MIB_TCPFASTOPENPASSIVEFAIL %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENPASSIVEFAIL]);
	printf(" LINUX_MIB_TCPFASTOPENLISTENOVERFLOW %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENLISTENOVERFLOW]);
	printf(" LINUX_MIB_TCPFASTOPENCOOKIEREQD %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENCOOKIEREQD]);
	printf(" LINUX_MIB_TCPSPURIOUS_RTX_HOSTQUEUES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSPURIOUS_RTX_HOSTQUEUES]);
	printf(" LINUX_MIB_BUSYPOLLRXPACKETS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_BUSYPOLLRXPACKETS]);
	printf(" LINUX_MIB_TCPAUTOCORKING %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPAUTOCORKING]);

	printf(" TCP_MIB_RTOALGORITHM %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOALGORITHM]);
	printf(" TCP_MIB_RTOMIN %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOMIN]);
	printf(" TCP_MIB_RTOMAX %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOMAX]);
	printf(" TCP_MIB_MAXCONN %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_MAXCONN]);
	printf(" TCP_MIB_ACTIVEOPENS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_ACTIVEOPENS]);
	printf(" TCP_MIB_PASSIVEOPENS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_PASSIVEOPENS]);
	printf(" TCP_MIB_ATTEMPTFAILS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_ATTEMPTFAILS]);
	printf(" TCP_MIB_ESTABRESETS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_ESTABRESETS]);
	printf(" TCP_MIB_CURRESTAB %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_CURRESTAB]);
	printf(" TCP_MIB_INSEGS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_INSEGS]);
	printf(" TCP_MIB_OUTSEGS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_OUTSEGS]);
	printf(" TCP_MIB_RETRANSSEGS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RETRANSSEGS]);
	printf(" TCP_MIB_INERRS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_INERRS]);
	printf(" TCP_MIB_OUTRSTS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_OUTRSTS]);
	printf(" TCP_MIB_CSUMERRORS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_CSUMERRORS]);
	printf(" UDP_MIB_INDATAGRAMS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_INDATAGRAMS]);
    printf(" UDP_MIB_NOPORTS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_NOPORTS]);
    printf(" UDP_MIB_INERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_INERRORS]);
    printf(" UDP_MIB_OUTDATAGRAMS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_OUTDATAGRAMS]);
    printf(" UDP_MIB_RCVBUFERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_RCVBUFERRORS]);
    printf(" UDP_MIB_SNDBUFERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_SNDBUFERRORS]);
    printf(" UDP_MIB_CSUMERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_CSUMERRORS]);
}
