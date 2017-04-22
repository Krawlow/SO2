#ifndef SEMAFORS_H
#define SEMAFORS_H
#include <list.h>
#include <sched.h>
/* Structure used by 'sem' functions */
struct semafors
{
  unsigned int counter;
  struct task_struct *owner;
  struct list_head blockedqueue;
};

#endif /* !SEMAFORS_H */
