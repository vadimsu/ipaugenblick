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
#include <ipaugenblick_log.h>

void show_mib_stats(void)
{
	struct netns_mib *p_mib = &init_net.mib;
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INPKTS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INOCTETS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INDELIVERS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INDELIVERS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_OUTFORWDATAGRAMS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTFORWDATAGRAMS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_OUTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTPKTS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_OUTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTOCTETS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INHDRERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INHDRERRORS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INTOOBIGERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INTOOBIGERRORS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INNOROUTES %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INNOROUTES]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INADDRERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INADDRERRORS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INUNKNOWNPROTOS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INUNKNOWNPROTOS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INTRUNCATEDPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INTRUNCATEDPKTS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INDISCARDS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INDISCARDS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_OUTDISCARDS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTDISCARDS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_OUTNOROUTES %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTNOROUTES]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_REASMTIMEOUT %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMTIMEOUT]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_REASMREQDS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMREQDS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_REASMOKS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMOKS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_REASMFAILS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMFAILS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_FRAGOKS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGOKS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_FRAGFAILS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGFAILS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_FRAGCREATES %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGCREATES]);
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INMCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INMCASTPKTS]);
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_OUTMCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTMCASTPKTS]);
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INBCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INBCASTPKTS]);
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_OUTBCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTBCASTPKTS]);
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INMCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INMCASTOCTETS]);
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_OUTMCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTMCASTOCTETS]);
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_INBCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INBCASTOCTETS]);
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_OUTBCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTBCASTOCTETS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_CSUMERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_CSUMERRORS]);
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_NOECTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_NOECTPKTS]);
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_ECT1PKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_ECT1PKTS]);
	//ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_ECT0PKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_ECT0PKTS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," IPSTATS_MIB_CEPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_CEPKTS]);

	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_DELAYEDACKS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_DELAYEDACKS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_DELAYEDACKLOST %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_DELAYEDACKLOST]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPPREQUEUED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPREQUEUED]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPDIRECTCOPYFROMBACKLOG %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDIRECTCOPYFROMBACKLOG]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPDIRECTCOPYFROMPREQUEUE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDIRECTCOPYFROMPREQUEUE]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPPREQUEUEDROPPED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPREQUEUEDROPPED]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPHPHITS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPHITS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPHPHITSTOUSER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPHITSTOUSER]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPPUREACKS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPUREACKS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPHPACKS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPACKS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPRENORECOVERY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENORECOVERY]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPSACKRECOVERY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKRECOVERY]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPSACKRENEGING %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKRENEGING]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPFACKREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFACKREORDER]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPSACKREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKREORDER]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPRENOREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENOREORDER]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPTSREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTSREORDER]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPFULLUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFULLUNDO]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPPARTIALUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPARTIALUNDO]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_DELAYEDACKLOCKED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_DELAYEDACKLOCKED]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPDSACKUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKUNDO]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPLOSSUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSUNDO]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPLOSTRETRANSMIT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSTRETRANSMIT]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPRENOFAILURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENOFAILURES]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPSACKFAILURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKFAILURES]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPLOSSFAILURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSFAILURES]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPFASTRETRANS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTRETRANS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPFORWARDRETRANS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFORWARDRETRANS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPSLOWSTARTRETRANS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSLOWSTARTRETRANS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPTIMEOUTS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTIMEOUTS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPLOSSPROBES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSPROBES]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPLOSSPROBERECOVERY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSPROBERECOVERY]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPSCHEDULERFAILED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSCHEDULERFAILED]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPRCVCOLLAPSED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRCVCOLLAPSED]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPDSACKOLDSENT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOLDSENT]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPDSACKOFOSENT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOFOSENT]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPDSACKRECV %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKRECV]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPDSACKOFORECV %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOFORECV]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPABORTONDATA %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONDATA]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPABORTONCLOSE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONCLOSE]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPABORTONMEMORY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONMEMORY]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPABORTONTIMEOUT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONTIMEOUT]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPABORTONLINGER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONLINGER]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPABORTFAILED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTFAILED]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPMEMORYPRESSURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMEMORYPRESSURES]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPSACKDISCARD %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKDISCARD]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPDSACKIGNOREDOLD %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKIGNOREDOLD]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPDSACKIGNOREDNOUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKIGNOREDNOUNDO]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPSPURIOUSRTOS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSPURIOUSRTOS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPMD5NOTFOUND %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMD5NOTFOUND]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPMD5UNEXPECTED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMD5UNEXPECTED]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_SACKSHIFTED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKSHIFTED]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_SACKMERGED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKMERGED]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_SACKSHIFTFALLBACK %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKSHIFTFALLBACK]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPBACKLOGDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPBACKLOGDROP]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPMINTTLDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMINTTLDROP]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPDEFERACCEPTDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDEFERACCEPTDROP]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_IPRPFILTER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_IPRPFILTER]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPTIMEWAITOVERFLOW %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTIMEWAITOVERFLOW]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPREQQFULLDOCOOKIES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPREQQFULLDOCOOKIES]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPREQQFULLDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPREQQFULLDROP]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPRETRANSFAIL %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRETRANSFAIL]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPRCVCOALESCE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRCVCOALESCE]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPOFOQUEUE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFOQUEUE]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPOFODROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFODROP]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPOFOMERGE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFOMERGE]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPCHALLENGEACK %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPCHALLENGEACK]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPSYNCHALLENGE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSYNCHALLENGE]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPFASTOPENACTIVE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENACTIVE]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPFASTOPENPASSIVE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENPASSIVE]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPFASTOPENPASSIVEFAIL %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENPASSIVEFAIL]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPFASTOPENLISTENOVERFLOW %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENLISTENOVERFLOW]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPFASTOPENCOOKIEREQD %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENCOOKIEREQD]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPSPURIOUS_RTX_HOSTQUEUES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSPURIOUS_RTX_HOSTQUEUES]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_BUSYPOLLRXPACKETS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_BUSYPOLLRXPACKETS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," LINUX_MIB_TCPAUTOCORKING %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPAUTOCORKING]);

	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_RTOALGORITHM %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOALGORITHM]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_RTOMIN %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOMIN]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_RTOMAX %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOMAX]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_MAXCONN %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_MAXCONN]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_ACTIVEOPENS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_ACTIVEOPENS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_PASSIVEOPENS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_PASSIVEOPENS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_ATTEMPTFAILS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_ATTEMPTFAILS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_ESTABRESETS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_ESTABRESETS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_CURRESTAB %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_CURRESTAB]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_INSEGS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_INSEGS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_OUTSEGS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_OUTSEGS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_RETRANSSEGS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RETRANSSEGS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_INERRS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_INERRS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_OUTRSTS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_OUTRSTS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," TCP_MIB_CSUMERRORS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_CSUMERRORS]);
	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," UDP_MIB_INDATAGRAMS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_INDATAGRAMS]);
    	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," UDP_MIB_NOPORTS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_NOPORTS]);
    	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," UDP_MIB_INERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_INERRORS]);
    	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," UDP_MIB_OUTDATAGRAMS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_OUTDATAGRAMS]);
    	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," UDP_MIB_RCVBUFERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_RCVBUFERRORS]);
    	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," UDP_MIB_SNDBUFERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_SNDBUFERRORS]);
    	ipaugenblick_log(IPAUGENBLICK_LOG_INFO," UDP_MIB_CSUMERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_CSUMERRORS]);
}
