#include <keyboard.h>
#include <io.h>

#define CIR_SIZE 32
struct list_head keyboardqueue;
extern struct list_head readyqueue;
Byte circular[CIR_SIZE];
int inpt, oupt;
void block() {
	printk("jeje\n");
	update_process_state_rr(current(),&keyboardqueue);
	sched_next_rr();
}
int unblock(struct task_struct *t) {
	struct list_head aux = keyboardqueue;
	int found = -1;
	struct list_head * e = list_first(&keyboardqueue);
	list_for_each(e,&keyboardqueue) {
		struct task_struct *tsk = list_head_to_task_struct(e);
		if (t->PID == tsk->PID) {
			found = 0;
			list_del(e);
			update_process_state_rr(tsk,&readyqueue);
		}
	}
	return found;
}
void init_circbuff() {
	inpt = 0;
	oupt = 0;
}
int circbuffadd(Byte key) {
	if ((inpt+1)%32 == oupt) return -1;
	circular[inpt] = key;
	inpt = (inpt++)%CIR_SIZE;
	return 0;
}
int circbuffsize() {
	return CIR_SIZE-1;
}
Byte circbuffdel() {
	if (inpt == oupt) return -1;
	Byte key = circular[oupt];
	oupt = (oupt++)%CIR_SIZE;
	return key;
}
int circbufffull() {
	return ((inpt+1)%CIR_SIZE == oupt);
}
int circbuffempty() {
	return (inpt == oupt);
}
