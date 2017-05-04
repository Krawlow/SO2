#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#include <list.h>
#include <types.h>
#include <sched.h>
#define CIR_SIZE 32
struct cbuffer {
	Byte circular[CIR_SIZE];
	int inpt, oupt;
};
void block();
void init_circbuff(struct cbuffer *b);
void unblock(struct task_struct *t);
int circbuffadd(struct cbuffer *b, Byte key);
Byte circbuffdel(struct cbuffer *b);
int circbufffull(struct cbuffer *b);
int circbuffempty(struct cbuffer *b);
int circbuffsize(struct cbuffer *b);
#endif /* __KEYBOARD_H__ */
