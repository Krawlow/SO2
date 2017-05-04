#include <keyboard.h>
#include <io.h>

struct list_head keyboardqueue;
extern struct list_head readyqueue;
struct cbuffer teclat_buff;
void block() {
	update_process_state_rr(current(),&keyboardqueue);
	sched_next_rr();
}
void unblock(struct task_struct *t) {
	list_del(&t->list);
	update_process_state_rr(t,&readyqueue);
}
void init_circbuff(struct cbuffer *b) {
	b->inpt = 0;
	b->oupt = 0;
}
int circbuffadd(struct cbuffer *b, Byte key) {
	if ((b->inpt+1)%CIR_SIZE == b->oupt) return -1;
	b->circular[b->inpt] = key;
	b->inpt = (b->inpt++)%CIR_SIZE;
	return 0;
}
int circbuffsize(struct cbuffer *b) {
	return CIR_SIZE-1;
}
Byte circbuffdel(struct cbuffer *b) {
	if (b->inpt == b->oupt) return -1;
	Byte key = b->circular[b->oupt];
	b->oupt = (b->oupt++)%CIR_SIZE;
	return key;
}
int circbufffull(struct cbuffer *b) {
	return ((b->inpt+1)%CIR_SIZE == b->oupt);
}
int circbuffempty(struct cbuffer *b) {
	return (b->inpt == b->oupt);
}
