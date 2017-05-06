#include <keyboard.h>
#include <io.h>

struct list_head keyboardqueue;
extern struct list_head readyqueue;
struct cbuffer teclat_buff;
int buff_used(struct cbuffer *b) {
	int buff_used = b->inpt - b->oupt;
	if (buff_used <0) buff_used = CIR_SIZE + buff_used;
	return buff_used;
}
void block() {
	printk("bloquejo un procés a la cua de teclat\n");
	update_process_state_rr(current(),&keyboardqueue);
	sched_next_rr();
}
void blockP() {
	printk("bloquejo un procés a la cua de teclat amb prioritat\n");
	current()->state = 3; //3 = BLOCKED
	list_add(&current()->list,&keyboardqueue);
	sched_next_rr();
}
void unblock(struct task_struct *t) {
	printk("desbloquejo un procés a la cua de teclat\n");
	list_del(&t->list);
	update_process_state_rr(t,&readyqueue);
}
void init_circbuff(struct cbuffer *b) {
	b->inpt = 0;
	b->oupt = 0;
}
int circbuffadd(struct cbuffer *b, Byte key) {
	if ((b->inpt+1)%CIR_SIZE == b->oupt) return -1;
	printk("afegeixo un byte al buffer\n");
	b->circular[b->inpt] = key;
	b->inpt = (++b->inpt)%CIR_SIZE;
	return 0;
}
int circbuffsize(struct cbuffer *b) {
	return CIR_SIZE-1;
}
Byte circbuffdel(struct cbuffer *b) {
	if (b->inpt == b->oupt) return -1;
	printk("elimino un byte del buffer\n");
	Byte key = b->circular[b->oupt];
	b->oupt = (++b->oupt)%CIR_SIZE;
	return key;
}
int circbufffull(struct cbuffer *b) {
	return ((b->inpt+1)%CIR_SIZE == b->oupt);
}
int circbuffempty(struct cbuffer *b) {
	return (b->inpt == b->oupt);
}
