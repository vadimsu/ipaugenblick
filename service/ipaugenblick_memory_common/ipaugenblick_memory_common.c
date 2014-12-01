#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>

#define MB_1 (1024*1024)
#define MB_8 (8*MB_1)
#define IPAUGENBLICK_SHMEM_KEY 0xBCDE
static int shmid1;

void *ipaugenblick_shared_memory_init(int size)
{
  void *api_base_address;
  shmid1 = shmget(IPAUGENBLICK_SHMEM_KEY, MB_8, SHM_HUGETLB
         | IPC_CREAT | SHM_R
         | SHM_W);
  if ( shmid1 < 0 ) {
    perror("shmget");
    exit(1);
  }
  printf("HugeTLB shmid: 0x%x\n", shmid1);
  api_base_address = shmat(shmid1, 0, 0);
  if (api_base_address == (char *)-1) {
    perror("Shared memory attach failure");
    shmctl(shmid1, IPC_RMID, NULL);
    exit(2);
  }
  return api_base_address;
}


