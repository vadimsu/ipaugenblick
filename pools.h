/*
 * pools.h
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains memory pools size

 */

#ifndef __POOLS_H_
#define __POOLS_H_

#define NUMBER_OF_HEAD_SKBS 16384*2

#define NUMBER_OF_FCLONE_SKBS 16384*2

/* On the transmit path, the headers are placed in HEADERS,
 * on the receive path, both data and headers are placed in HEADERS
 */
#define NUMBER_OF_HEADER_SEGMENTS ((NUMBER_OF_HEAD_SKBS > NUMBER_OF_FCLONE_SKBS) ? NUMBER_OF_HEAD_SKBS : NUMBER_OF_FCLONE_SKBS)

#define NUMBER_OF_SHINFO_BLOCKS ((NUMBER_OF_HEAD_SKBS > NUMBER_OF_FCLONE_SKBS) ? NUMBER_OF_HEAD_SKBS : NUMBER_OF_FCLONE_SKBS)

#define TOTAL_NUMBER_OF_SKBS (NUMBER_OF_FCLONE_SKBS+NUMBER_OF_FCLONE_SKBS)
/* This is a pool for the PMD  driver on rx path.
 * It should correlate with rx sockets pool interval,
 * too long interval relatively to too small RX pool will result in rx stuck
 */
#define MBUFS_PER_RX_QUEUE 4096*256

#define APP_MBUFS_POOL_SIZE 16384

#endif /* __POOLS_H_ */
