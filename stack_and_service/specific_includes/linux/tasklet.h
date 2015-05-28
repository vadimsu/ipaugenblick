
#ifndef __LINUX_TASKLET_H_
#define __LINUX_TASKLET_H_
struct tasklet_struct
{
         struct tasklet_struct *next;
         unsigned long state;
         atomic_t count;
         void (*func)(unsigned long);
         unsigned long data;
};

void tasklet_init(struct tasklet_struct *t,
                   void (*func)(unsigned long), unsigned long data);
void tasklet_schedule(struct tasklet_struct *t);

#define DECLARE_TASKLET(name, func, data) \
struct tasklet_struct name = { NULL, 0, ATOMIC_INIT(0), func, data }
 
#define DECLARE_TASKLET_DISABLED(name, func, data) \
struct tasklet_struct name = { NULL, 0, ATOMIC_INIT(1), func, data }

#endif /* __LINUX_TASKLET_H_ */
