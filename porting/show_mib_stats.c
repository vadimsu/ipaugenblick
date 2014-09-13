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
#include <specific_includes/net/arp.h>

#define IP_STAT_PRINT 1
#define TCP_STAT_PRINT 1
#define UDP_STAT_PRINT 0
#define NEIGH_STAT_PRINT 0

void show_mib_stats(void)
{
#if 0
	int cpu_idx;
	struct netns_mib *p_mib = &init_net.mib;
	for(cpu_idx = 0;cpu_idx < 3;cpu_idx++) {
#if IP_STAT_PRINT
		printf(" IPSTATS_MIB_INPKTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INPKTS],cpu_idx)));
		printf(" IPSTATS_MIB_INOCTETS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INOCTETS],cpu_idx)));
		printf(" IPSTATS_MIB_INDELIVERS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INDELIVERS],cpu_idx)));
		printf(" IPSTATS_MIB_OUTFORWDATAGRAMS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTFORWDATAGRAMS],cpu_idx)));
		printf(" IPSTATS_MIB_OUTPKTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTPKTS],cpu_idx)));
		printf(" IPSTATS_MIB_OUTOCTETS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTOCTETS],cpu_idx)));
		printf(" IPSTATS_MIB_INHDRERRORS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INHDRERRORS],cpu_idx)));
		printf(" IPSTATS_MIB_INTOOBIGERRORS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INTOOBIGERRORS],cpu_idx)));
		printf(" IPSTATS_MIB_INNOROUTES %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INNOROUTES],cpu_idx)));
		printf(" IPSTATS_MIB_INADDRERRORS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INADDRERRORS],cpu_idx)));
		printf(" IPSTATS_MIB_INUNKNOWNPROTOS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INUNKNOWNPROTOS],cpu_idx)));
		printf(" IPSTATS_MIB_INTRUNCATEDPKTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INTRUNCATEDPKTS],cpu_idx)));
		printf(" IPSTATS_MIB_INDISCARDS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INDISCARDS],cpu_idx)));
		printf(" IPSTATS_MIB_OUTDISCARDS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTDISCARDS],cpu_idx)));
		printf(" IPSTATS_MIB_OUTNOROUTES %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTNOROUTES],cpu_idx)));
		printf(" IPSTATS_MIB_REASMTIMEOUT %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMTIMEOUT],cpu_idx)));
		printf(" IPSTATS_MIB_REASMREQDS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMREQDS],cpu_idx)));
		printf(" IPSTATS_MIB_REASMOKS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMOKS],cpu_idx)));
		printf(" IPSTATS_MIB_REASMFAILS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_REASMFAILS],cpu_idx)));
		printf(" IPSTATS_MIB_FRAGOKS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGOKS],cpu_idx)));
		printf(" IPSTATS_MIB_FRAGFAILS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGFAILS],cpu_idx)));
		printf(" IPSTATS_MIB_FRAGCREATES %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_FRAGCREATES],cpu_idx)));
		//printf(" IPSTATS_MIB_INMCASTPKTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INMCASTPKTS],cpu_idx)));
		//printf(" IPSTATS_MIB_OUTMCASTPKTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTMCASTPKTS],cpu_idx)));
		//printf(" IPSTATS_MIB_INBCASTPKTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INBCASTPKTS],cpu_idx)));
		//printf(" IPSTATS_MIB_OUTBCASTPKTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTBCASTPKTS],cpu_idx)));
		//printf(" IPSTATS_MIB_INMCASTOCTETS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INMCASTOCTETS],cpu_idx)));
		//printf(" IPSTATS_MIB_OUTMCASTOCTETS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTMCASTOCTETS],cpu_idx)));
		//printf(" IPSTATS_MIB_INBCASTOCTETS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_INBCASTOCTETS],cpu_idx)));
		//printf(" IPSTATS_MIB_OUTBCASTOCTETS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_OUTBCASTOCTETS],cpu_idx)));
		printf(" IPSTATS_MIB_CSUMERRORS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_CSUMERRORS],cpu_idx)));
		//printf(" IPSTATS_MIB_NOECTPKTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_NOECTPKTS],cpu_idx)));
		//printf(" IPSTATS_MIB_ECT1PKTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_ECT1PKTS],cpu_idx)));
		//printf(" IPSTATS_MIB_ECT0PKTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_ECT0PKTS],cpu_idx)));
		printf(" IPSTATS_MIB_CEPKTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->ip_statistics[0]->mibs[IPSTATS_MIB_CEPKTS],cpu_idx)));
#endif
#if TCP_STAT_PRINT
		printf(" LINUX_MIB_DELAYEDACKS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_DELAYEDACKS],cpu_idx)));
		printf(" LINUX_MIB_DELAYEDACKLOST %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_DELAYEDACKLOST],cpu_idx)));
		printf(" LINUX_MIB_TCPPREQUEUED %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPREQUEUED],cpu_idx)));
		printf(" LINUX_MIB_TCPDIRECTCOPYFROMBACKLOG %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDIRECTCOPYFROMBACKLOG],cpu_idx)));
		printf(" LINUX_MIB_TCPDIRECTCOPYFROMPREQUEUE %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDIRECTCOPYFROMPREQUEUE],cpu_idx)));
		printf(" LINUX_MIB_TCPPREQUEUEDROPPED %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPREQUEUEDROPPED],cpu_idx)));
		printf(" LINUX_MIB_TCPHPHITS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPHITS],cpu_idx)));
		printf(" LINUX_MIB_TCPHPHITSTOUSER %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPHITSTOUSER],cpu_idx)));
		printf(" LINUX_MIB_TCPPUREACKS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPUREACKS],cpu_idx)));
		printf(" LINUX_MIB_TCPHPACKS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPHPACKS],cpu_idx)));
		printf(" LINUX_MIB_TCPRENORECOVERY %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENORECOVERY],cpu_idx)));
		printf(" LINUX_MIB_TCPSACKRECOVERY %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKRECOVERY],cpu_idx)));
		printf(" LINUX_MIB_TCPSACKRENEGING %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKRENEGING],cpu_idx)));
		printf(" LINUX_MIB_TCPFACKREORDER %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFACKREORDER],cpu_idx)));
		printf(" LINUX_MIB_TCPSACKREORDER %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKREORDER],cpu_idx)));
		printf(" LINUX_MIB_TCPRENOREORDER %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENOREORDER],cpu_idx)));
		printf(" LINUX_MIB_TCPTSREORDER %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTSREORDER],cpu_idx)));
		printf(" LINUX_MIB_TCPFULLUNDO %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFULLUNDO],cpu_idx)));
		printf(" LINUX_MIB_TCPPARTIALUNDO %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPPARTIALUNDO],cpu_idx)));
		printf(" LINUX_MIB_TCPDSACKUNDO %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKUNDO],cpu_idx)));
		printf(" LINUX_MIB_TCPLOSSUNDO %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSUNDO],cpu_idx)));
		printf(" LINUX_MIB_TCPLOSTRETRANSMIT %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSTRETRANSMIT],cpu_idx)));
		printf(" LINUX_MIB_TCPRENOFAILURES %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRENOFAILURES],cpu_idx)));
		printf(" LINUX_MIB_TCPSACKFAILURES %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKFAILURES],cpu_idx)));
		printf(" LINUX_MIB_TCPLOSSFAILURES %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSFAILURES],cpu_idx)));
		printf(" LINUX_MIB_TCPFASTRETRANS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTRETRANS],cpu_idx)));
		printf(" LINUX_MIB_TCPFORWARDRETRANS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFORWARDRETRANS],cpu_idx)));
		printf(" LINUX_MIB_TCPSLOWSTARTRETRANS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSLOWSTARTRETRANS],cpu_idx)));
		printf(" LINUX_MIB_TCPTIMEOUTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTIMEOUTS],cpu_idx)));
		printf(" LINUX_MIB_TCPLOSSPROBES %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSPROBES],cpu_idx)));
		printf(" LINUX_MIB_TCPLOSSPROBERECOVERY %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPLOSSPROBERECOVERY],cpu_idx)));
		printf(" LINUX_MIB_TCPSCHEDULERFAILED %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSCHEDULERFAILED],cpu_idx)));
		printf(" LINUX_MIB_TCPRCVCOLLAPSED %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRCVCOLLAPSED],cpu_idx)));
		printf(" LINUX_MIB_TCPDSACKOLDSENT %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOLDSENT],cpu_idx)));
		printf(" LINUX_MIB_TCPDSACKOFOSENT %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOFOSENT],cpu_idx)));
		printf(" LINUX_MIB_TCPDSACKRECV %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKRECV],cpu_idx)));
		printf(" LINUX_MIB_TCPDSACKOFORECV %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKOFORECV],cpu_idx)));
		printf(" LINUX_MIB_TCPABORTONDATA %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONDATA],cpu_idx)));
		printf(" LINUX_MIB_TCPABORTONCLOSE %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONCLOSE],cpu_idx)));
		printf(" LINUX_MIB_TCPABORTONMEMORY %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONMEMORY],cpu_idx)));
		printf(" LINUX_MIB_TCPABORTONTIMEOUT %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONTIMEOUT],cpu_idx)));
		printf(" LINUX_MIB_TCPABORTONLINGER %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTONLINGER],cpu_idx)));
		printf(" LINUX_MIB_TCPABORTFAILED %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPABORTFAILED],cpu_idx)));
		printf(" LINUX_MIB_TCPMEMORYPRESSURES %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMEMORYPRESSURES],cpu_idx)));
		printf(" LINUX_MIB_TCPSACKDISCARD %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSACKDISCARD],cpu_idx)));
		printf(" LINUX_MIB_TCPDSACKIGNOREDOLD %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKIGNOREDOLD],cpu_idx)));
		printf(" LINUX_MIB_TCPDSACKIGNOREDNOUNDO %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDSACKIGNOREDNOUNDO],cpu_idx)));
		printf(" LINUX_MIB_TCPSPURIOUSRTOS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSPURIOUSRTOS],cpu_idx)));
		printf(" LINUX_MIB_TCPMD5NOTFOUND %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMD5NOTFOUND],cpu_idx)));
		printf(" LINUX_MIB_TCPMD5UNEXPECTED %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMD5UNEXPECTED],cpu_idx)));
		printf(" LINUX_MIB_SACKSHIFTED %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKSHIFTED],cpu_idx)));
		printf(" LINUX_MIB_SACKMERGED %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKMERGED],cpu_idx)));
		printf(" LINUX_MIB_SACKSHIFTFALLBACK %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_SACKSHIFTFALLBACK],cpu_idx)));
		printf(" LINUX_MIB_TCPBACKLOGDROP %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPBACKLOGDROP],cpu_idx)));
		printf(" LINUX_MIB_TCPMINTTLDROP %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPMINTTLDROP],cpu_idx)));
		printf(" LINUX_MIB_TCPDEFERACCEPTDROP %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPDEFERACCEPTDROP],cpu_idx)));
		printf(" LINUX_MIB_IPRPFILTER %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_IPRPFILTER],cpu_idx)));
		printf(" LINUX_MIB_TCPTIMEWAITOVERFLOW %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPTIMEWAITOVERFLOW],cpu_idx)));
		printf(" LINUX_MIB_TCPREQQFULLDOCOOKIES %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPREQQFULLDOCOOKIES],cpu_idx)));
		printf(" LINUX_MIB_TCPREQQFULLDROP %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPREQQFULLDROP],cpu_idx)));
		printf(" LINUX_MIB_TCPRETRANSFAIL %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRETRANSFAIL],cpu_idx)));
		printf(" LINUX_MIB_TCPRCVCOALESCE %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPRCVCOALESCE],cpu_idx)));
		printf(" LINUX_MIB_TCPOFOQUEUE %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFOQUEUE],cpu_idx)));
		printf(" LINUX_MIB_TCPOFODROP %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFODROP],cpu_idx)));
		printf(" LINUX_MIB_TCPOFOMERGE %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPOFOMERGE],cpu_idx)));
		printf(" LINUX_MIB_TCPCHALLENGEACK %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPCHALLENGEACK],cpu_idx)));
		printf(" LINUX_MIB_TCPSYNCHALLENGE %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSYNCHALLENGE],cpu_idx)));
		printf(" LINUX_MIB_TCPFASTOPENACTIVE %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENACTIVE],cpu_idx)));
		printf(" LINUX_MIB_TCPFASTOPENPASSIVE %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENPASSIVE],cpu_idx)));
		printf(" LINUX_MIB_TCPFASTOPENPASSIVEFAIL %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENPASSIVEFAIL],cpu_idx)));
		printf(" LINUX_MIB_TCPFASTOPENLISTENOVERFLOW %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENLISTENOVERFLOW],cpu_idx)));
		printf(" LINUX_MIB_TCPFASTOPENCOOKIEREQD %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPFASTOPENCOOKIEREQD],cpu_idx)));
		printf(" LINUX_MIB_TCPSPURIOUS_RTX_HOSTQUEUES %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPSPURIOUS_RTX_HOSTQUEUES],cpu_idx)));
		printf(" LINUX_MIB_BUSYPOLLRXPACKETS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_BUSYPOLLRXPACKETS],cpu_idx)));
		printf(" LINUX_MIB_TCPAUTOCORKING %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->net_statistics[0]->mibs[LINUX_MIB_TCPAUTOCORKING],cpu_idx)));

		printf(" TCP_MIB_RTOALGORITHM %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOALGORITHM],cpu_idx)));
		printf(" TCP_MIB_RTOMIN %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOMIN],cpu_idx)));
		printf(" TCP_MIB_RTOMAX %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_RTOMAX],cpu_idx)));
		printf(" TCP_MIB_MAXCONN %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_MAXCONN],cpu_idx)));
		printf(" TCP_MIB_ACTIVEOPENS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_ACTIVEOPENS],cpu_idx)));
		printf(" TCP_MIB_PASSIVEOPENS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_PASSIVEOPENS],cpu_idx)));
		printf(" TCP_MIB_ATTEMPTFAILS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_ATTEMPTFAILS],cpu_idx)));
		printf(" TCP_MIB_ESTABRESETS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_ESTABRESETS],cpu_idx)));
		printf(" TCP_MIB_CURRESTAB %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_CURRESTAB],cpu_idx)));
		printf(" TCP_MIB_INSEGS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_INSEGS],cpu_idx)));
		printf(" TCP_MIB_OUTSEGS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_OUTSEGS],cpu_idx)));
		printf(" TCP_MIB_RETRANSSEGS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_RETRANSSEGS],cpu_idx)));
		printf(" TCP_MIB_INERRS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_INERRS],cpu_idx)));
		printf(" TCP_MIB_OUTRSTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_OUTRSTS],cpu_idx)));
		printf(" TCP_MIB_CSUMERRORS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->tcp_statistics[0]->mibs[TCP_MIB_CSUMERRORS],cpu_idx)));
#endif
#if UDP_STAT_PRINT
		printf(" UDP_MIB_INDATAGRAMS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->udp_statistics[0]->mibs[UDP_MIB_INDATAGRAMS],cpu_idx)));
		printf(" UDP_MIB_NOPORTS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->udp_statistics[0]->mibs[UDP_MIB_NOPORTS],cpu_idx)));
		printf(" UDP_MIB_INERRORS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->udp_statistics[0]->mibs[UDP_MIB_INERRORS],cpu_idx)));
		printf(" UDP_MIB_OUTDATAGRAMS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->udp_statistics[0]->mibs[UDP_MIB_OUTDATAGRAMS],cpu_idx)));
		printf(" UDP_MIB_RCVBUFERRORS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->udp_statistics[0]->mibs[UDP_MIB_RCVBUFERRORS],cpu_idx)));
		printf(" UDP_MIB_SNDBUFERRORS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->udp_statistics[0]->mibs[UDP_MIB_SNDBUFERRORS],cpu_idx)));
		printf(" UDP_MIB_CSUMERRORS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(&p_mib->udp_statistics[0]->mibs[UDP_MIB_CSUMERRORS],cpu_idx)));
#endif
#if NEIGH_STAT_PRINT
    	printf(" NEIGH_allocs %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->allocs,cpu_idx)));
    	printf(" NEIGH_destroys %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->destroys,cpu_idx)));
    	printf(" NEIGH_hash_grows %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->hash_grows,cpu_idx)));
    	printf(" NEIGH_res_failed %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->res_failed,cpu_idx)));
    	printf(" NEIGH_lookups %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->lookups,cpu_idx)));
    	printf(" NEIGH_hits %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->hits,cpu_idx)));
    	printf(" NEIGH_rcv_probes_mcast %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->rcv_probes_mcast,cpu_idx)));
    	printf(" NEIGH_rcv_probes_ucast %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->rcv_probes_ucast,cpu_idx)));
    	printf(" NEIGH_periodic_gc_runs %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->periodic_gc_runs,cpu_idx)));
    	printf(" NEIGH_forced_gc_runs %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->forced_gc_runs,cpu_idx)));
    	printf(" NEIGH_unres_discards %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->unres_discards,cpu_idx)));
    	printf(" NEIGH_VAR_LOCKTIME %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->allocs,cpu_idx)));
    	printf(" NEIGH_VAR_QUEUE_LEN %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->allocs,cpu_idx)));
    	printf(" NEIGH_VAR_RETRANS_TIME_MS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->allocs,cpu_idx)));
    	printf(" NEIGH_VAR_BASE_REACHABLE_TIME_MS %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->allocs,cpu_idx)));
    	printf(" NEIGH_VAR_GC_THRESH1 %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->allocs,cpu_idx)));
    	printf(" NEIGH_VAR_GC_THRESH2 %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->allocs,cpu_idx)));
    	printf(" NEIGH_VAR_GC_THRESH3 %"PRIu64"\n",*((unsigned long*)per_cpu_ptr(arp_tbl[0].stats->allocs,cpu_idx)));
#endif
    }
#endif
}
