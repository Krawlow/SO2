#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#include <list.h>
void block();
int unblock(struct task_struct *t);
void circbuffadd(Byte key);
void circbuffdel(Byte key);
#endif /* __KEYBOARD_H__ */
