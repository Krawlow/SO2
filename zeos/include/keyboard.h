#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#include <list.h>
#include <types.h>
#include <sched.h>
void block();
void init_circbuff();
int unblock(struct task_struct *t);
int circbuffadd(Byte key);
Byte circbuffdel();
int circbufffull();
int circbuffempty();
int circbuffsize();
#endif /* __KEYBOARD_H__ */
