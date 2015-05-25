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
#include <syslog.h>

void show_mib_stats(void)
{
	struct netns_mib *p_mib = &init_net.mib;
	syslog(LOG_INFO," IPSTATS_MIB_INPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INPKTS]);
	syslog(LOG_INFO," IPSTATS_MIB_INOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INOCTETS]);
	syslog(LOG_INFO," IPSTATS_MIB_INDELIVERS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INDELIVERS]);
	syslog(LOG_INFO," IPSTATS_MIB_OUTFORWDATAGRAMS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTFORWDATAGRAMS]);
	syslog(LOG_INFO," IPSTATS_MIB_OUTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTPKTS]);
	syslog(LOG_INFO," IPSTATS_MIB_OUTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTOCTETS]);
	syslog(LOG_INFO," IPSTATS_MIB_INHDRERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INHDRERRORS]);
	syslog(LOG_INFO," IPSTATS_MIB_INTOOBIGERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INTOOBIGERRORS]);
	syslog(LOG_INFO," IPSTATS_MIB_INNOROUTES %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INNOROUTES]);
	syslog(LOG_INFO," IPSTATS_MIB_INADDRERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INADDRERRORS]);
	syslog(LOG_INFO," IPSTATS_MIB_INUNKNOWNPROTOS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INUNKNOWNPROTOS]);
	syslog(LOG_INFO," IPSTATS_MIB_INTRUNCATEDPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INTRUNCATEDPKTS]);
	syslog(LOG_INFO," IPSTATS_MIB_INDISCARDS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INDISCARDS]);
	syslog(LOG_INFO," IPSTATS_MIB_OUTDISCARDS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTDISCARDS]);
	syslog(LOG_INFO," IPSTATS_MIB_OUTNOROUTES %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTNOROUTES]);
	syslog(LOG_INFO," IPSTATS_MIB_REASMTIMEOUT %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMTIMEOUT]);
	syslog(LOG_INFO," IPSTATS_MIB_REASMREQDS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMREQDS]);
	syslog(LOG_INFO," IPSTATS_MIB_REASMOKS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMOKS]);
	syslog(LOG_INFO," IPSTATS_MIB_REASMFAILS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMFAILS]);
	syslog(LOG_INFO," IPSTATS_MIB_FRAGOKS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGOKS]);
	syslog(LOG_INFO," IPSTATS_MIB_FRAGFAILS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGFAILS]);
	syslog(LOG_INFO," IPSTATS_MIB_FRAGCREATES %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGCREATES]);
	//syslog(LOG_INFO," IPSTATS_MIB_INMCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INMCASTPKTS]);
	//syslog(LOG_INFO," IPSTATS_MIB_OUTMCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTMCASTPKTS]);
	//syslog(LOG_INFO," IPSTATS_MIB_INBCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INBCASTPKTS]);
	//syslog(LOG_INFO," IPSTATS_MIB_OUTBCASTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTBCASTPKTS]);
	//syslog(LOG_INFO," IPSTATS_MIB_INMCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INMCASTOCTETS]);
	//syslog(LOG_INFO," IPSTATS_MIB_OUTMCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTMCASTOCTETS]);
	//syslog(LOG_INFO," IPSTATS_MIB_INBCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INBCASTOCTETS]);
	//syslog(LOG_INFO," IPSTATS_MIB_OUTBCASTOCTETS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTBCASTOCTETS]);
	syslog(LOG_INFO," IPSTATS_MIB_CSUMERRORS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_CSUMERRORS]);
	//syslog(LOG_INFO," IPSTATS_MIB_NOECTPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_NOECTPKTS]);
	//syslog(LOG_INFO," IPSTATS_MIB_ECT1PKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_ECT1PKTS]);
	//syslog(LOG_INFO," IPSTATS_MIB_ECT0PKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_ECT0PKTS]);
	syslog(LOG_INFO," IPSTATS_MIB_CEPKTS %"PRIu64"\n",(unsigned long)p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_CEPKTS]);

	syslog(LOG_INFO," LINUX_MIB_DELAYEDACKS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_DELAYEDACKS]);
	syslog(LOG_INFO," LINUX_MIB_DELAYEDACKLOST %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_DELAYEDACKLOST]);
	syslog(LOG_INFO," LINUX_MIB_TCPPREQUEUED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPREQUEUED]);
	syslog(LOG_INFO," LINUX_MIB_TCPDIRECTCOPYFROMBACKLOG %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDIRECTCOPYFROMBACKLOG]);
	syslog(LOG_INFO," LINUX_MIB_TCPDIRECTCOPYFROMPREQUEUE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDIRECTCOPYFROMPREQUEUE]);
	syslog(LOG_INFO," LINUX_MIB_TCPPREQUEUEDROPPED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPREQUEUEDROPPED]);
	syslog(LOG_INFO," LINUX_MIB_TCPHPHITS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPHITS]);
	syslog(LOG_INFO," LINUX_MIB_TCPHPHITSTOUSER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPHITSTOUSER]);
	syslog(LOG_INFO," LINUX_MIB_TCPPUREACKS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPUREACKS]);
	syslog(LOG_INFO," LINUX_MIB_TCPHPACKS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPACKS]);
	syslog(LOG_INFO," LINUX_MIB_TCPRENORECOVERY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENORECOVERY]);
	syslog(LOG_INFO," LINUX_MIB_TCPSACKRECOVERY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKRECOVERY]);
	syslog(LOG_INFO," LINUX_MIB_TCPSACKRENEGING %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKRENEGING]);
	syslog(LOG_INFO," LINUX_MIB_TCPFACKREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFACKREORDER]);
	syslog(LOG_INFO," LINUX_MIB_TCPSACKREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKREORDER]);
	syslog(LOG_INFO," LINUX_MIB_TCPRENOREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENOREORDER]);
	syslog(LOG_INFO," LINUX_MIB_TCPTSREORDER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTSREORDER]);
	syslog(LOG_INFO," LINUX_MIB_TCPFULLUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFULLUNDO]);
	syslog(LOG_INFO," LINUX_MIB_TCPPARTIALUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPARTIALUNDO]);
	syslog(LOG_INFO," LINUX_MIB_DELAYEDACKLOCKED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_DELAYEDACKLOCKED]);
	syslog(LOG_INFO," LINUX_MIB_TCPDSACKUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKUNDO]);
	syslog(LOG_INFO," LINUX_MIB_TCPLOSSUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSUNDO]);
	syslog(LOG_INFO," LINUX_MIB_TCPLOSTRETRANSMIT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSTRETRANSMIT]);
	syslog(LOG_INFO," LINUX_MIB_TCPRENOFAILURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENOFAILURES]);
	syslog(LOG_INFO," LINUX_MIB_TCPSACKFAILURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKFAILURES]);
	syslog(LOG_INFO," LINUX_MIB_TCPLOSSFAILURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSFAILURES]);
	syslog(LOG_INFO," LINUX_MIB_TCPFASTRETRANS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTRETRANS]);
	syslog(LOG_INFO," LINUX_MIB_TCPFORWARDRETRANS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFORWARDRETRANS]);
	syslog(LOG_INFO," LINUX_MIB_TCPSLOWSTARTRETRANS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSLOWSTARTRETRANS]);
	syslog(LOG_INFO," LINUX_MIB_TCPTIMEOUTS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTIMEOUTS]);
	syslog(LOG_INFO," LINUX_MIB_TCPLOSSPROBES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSPROBES]);
	syslog(LOG_INFO," LINUX_MIB_TCPLOSSPROBERECOVERY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSPROBERECOVERY]);
	syslog(LOG_INFO," LINUX_MIB_TCPSCHEDULERFAILED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSCHEDULERFAILED]);
	syslog(LOG_INFO," LINUX_MIB_TCPRCVCOLLAPSED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRCVCOLLAPSED]);
	syslog(LOG_INFO," LINUX_MIB_TCPDSACKOLDSENT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOLDSENT]);
	syslog(LOG_INFO," LINUX_MIB_TCPDSACKOFOSENT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOFOSENT]);
	syslog(LOG_INFO," LINUX_MIB_TCPDSACKRECV %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKRECV]);
	syslog(LOG_INFO," LINUX_MIB_TCPDSACKOFORECV %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOFORECV]);
	syslog(LOG_INFO," LINUX_MIB_TCPABORTONDATA %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONDATA]);
	syslog(LOG_INFO," LINUX_MIB_TCPABORTONCLOSE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONCLOSE]);
	syslog(LOG_INFO," LINUX_MIB_TCPABORTONMEMORY %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONMEMORY]);
	syslog(LOG_INFO," LINUX_MIB_TCPABORTONTIMEOUT %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONTIMEOUT]);
	syslog(LOG_INFO," LINUX_MIB_TCPABORTONLINGER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONLINGER]);
	syslog(LOG_INFO," LINUX_MIB_TCPABORTFAILED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTFAILED]);
	syslog(LOG_INFO," LINUX_MIB_TCPMEMORYPRESSURES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMEMORYPRESSURES]);
	syslog(LOG_INFO," LINUX_MIB_TCPSACKDISCARD %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKDISCARD]);
	syslog(LOG_INFO," LINUX_MIB_TCPDSACKIGNOREDOLD %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKIGNOREDOLD]);
	syslog(LOG_INFO," LINUX_MIB_TCPDSACKIGNOREDNOUNDO %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKIGNOREDNOUNDO]);
	syslog(LOG_INFO," LINUX_MIB_TCPSPURIOUSRTOS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSPURIOUSRTOS]);
	syslog(LOG_INFO," LINUX_MIB_TCPMD5NOTFOUND %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMD5NOTFOUND]);
	syslog(LOG_INFO," LINUX_MIB_TCPMD5UNEXPECTED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMD5UNEXPECTED]);
	syslog(LOG_INFO," LINUX_MIB_SACKSHIFTED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKSHIFTED]);
	syslog(LOG_INFO," LINUX_MIB_SACKMERGED %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKMERGED]);
	syslog(LOG_INFO," LINUX_MIB_SACKSHIFTFALLBACK %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKSHIFTFALLBACK]);
	syslog(LOG_INFO," LINUX_MIB_TCPBACKLOGDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPBACKLOGDROP]);
	syslog(LOG_INFO," LINUX_MIB_TCPMINTTLDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMINTTLDROP]);
	syslog(LOG_INFO," LINUX_MIB_TCPDEFERACCEPTDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDEFERACCEPTDROP]);
	syslog(LOG_INFO," LINUX_MIB_IPRPFILTER %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_IPRPFILTER]);
	syslog(LOG_INFO," LINUX_MIB_TCPTIMEWAITOVERFLOW %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTIMEWAITOVERFLOW]);
	syslog(LOG_INFO," LINUX_MIB_TCPREQQFULLDOCOOKIES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPREQQFULLDOCOOKIES]);
	syslog(LOG_INFO," LINUX_MIB_TCPREQQFULLDROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPREQQFULLDROP]);
	syslog(LOG_INFO," LINUX_MIB_TCPRETRANSFAIL %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRETRANSFAIL]);
	syslog(LOG_INFO," LINUX_MIB_TCPRCVCOALESCE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRCVCOALESCE]);
	syslog(LOG_INFO," LINUX_MIB_TCPOFOQUEUE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFOQUEUE]);
	syslog(LOG_INFO," LINUX_MIB_TCPOFODROP %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFODROP]);
	syslog(LOG_INFO," LINUX_MIB_TCPOFOMERGE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFOMERGE]);
	syslog(LOG_INFO," LINUX_MIB_TCPCHALLENGEACK %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPCHALLENGEACK]);
	syslog(LOG_INFO," LINUX_MIB_TCPSYNCHALLENGE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSYNCHALLENGE]);
	syslog(LOG_INFO," LINUX_MIB_TCPFASTOPENACTIVE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENACTIVE]);
	syslog(LOG_INFO," LINUX_MIB_TCPFASTOPENPASSIVE %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENPASSIVE]);
	syslog(LOG_INFO," LINUX_MIB_TCPFASTOPENPASSIVEFAIL %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENPASSIVEFAIL]);
	syslog(LOG_INFO," LINUX_MIB_TCPFASTOPENLISTENOVERFLOW %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENLISTENOVERFLOW]);
	syslog(LOG_INFO," LINUX_MIB_TCPFASTOPENCOOKIEREQD %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENCOOKIEREQD]);
	syslog(LOG_INFO," LINUX_MIB_TCPSPURIOUS_RTX_HOSTQUEUES %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSPURIOUS_RTX_HOSTQUEUES]);
	syslog(LOG_INFO," LINUX_MIB_BUSYPOLLRXPACKETS %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_BUSYPOLLRXPACKETS]);
	syslog(LOG_INFO," LINUX_MIB_TCPAUTOCORKING %"PRIu64"\n",(unsigned long)p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPAUTOCORKING]);

	syslog(LOG_INFO," TCP_MIB_RTOALGORITHM %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOALGORITHM]);
	syslog(LOG_INFO," TCP_MIB_RTOMIN %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOMIN]);
	syslog(LOG_INFO," TCP_MIB_RTOMAX %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOMAX]);
	syslog(LOG_INFO," TCP_MIB_MAXCONN %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_MAXCONN]);
	syslog(LOG_INFO," TCP_MIB_ACTIVEOPENS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_ACTIVEOPENS]);
	syslog(LOG_INFO," TCP_MIB_PASSIVEOPENS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_PASSIVEOPENS]);
	syslog(LOG_INFO," TCP_MIB_ATTEMPTFAILS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_ATTEMPTFAILS]);
	syslog(LOG_INFO," TCP_MIB_ESTABRESETS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_ESTABRESETS]);
	syslog(LOG_INFO," TCP_MIB_CURRESTAB %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_CURRESTAB]);
	syslog(LOG_INFO," TCP_MIB_INSEGS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_INSEGS]);
	syslog(LOG_INFO," TCP_MIB_OUTSEGS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_OUTSEGS]);
	syslog(LOG_INFO," TCP_MIB_RETRANSSEGS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_RETRANSSEGS]);
	syslog(LOG_INFO," TCP_MIB_INERRS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_INERRS]);
	syslog(LOG_INFO," TCP_MIB_OUTRSTS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_OUTRSTS]);
	syslog(LOG_INFO," TCP_MIB_CSUMERRORS %"PRIu64"\n",(unsigned long)p_mib->tcp_statistics[0]->mibs[TCP_MIB_CSUMERRORS]);
	syslog(LOG_INFO," UDP_MIB_INDATAGRAMS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_INDATAGRAMS]);
    	syslog(LOG_INFO," UDP_MIB_NOPORTS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_NOPORTS]);
    	syslog(LOG_INFO," UDP_MIB_INERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_INERRORS]);
    	syslog(LOG_INFO," UDP_MIB_OUTDATAGRAMS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_OUTDATAGRAMS]);
    	syslog(LOG_INFO," UDP_MIB_RCVBUFERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_RCVBUFERRORS]);
    	syslog(LOG_INFO," UDP_MIB_SNDBUFERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_SNDBUFERRORS]);
    	syslog(LOG_INFO," UDP_MIB_CSUMERRORS %"PRIu64"\n",(unsigned long)p_mib->udp_statistics[0]->mibs[UDP_MIB_CSUMERRORS]);
}
