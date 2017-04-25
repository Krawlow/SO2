#include <keyboard.h>

struct list_head keyboardqueue;
Byte circular[32];
int *inpt, *outpt;
void block() {
	update_process_state_rr(current(),&keyboardqueue);
}
int unblock(struct task_struct *t) {
	struct list_head aux = keyboardqueue;
	int found = -1;
	while(found<0 && !list_empty(&aux)) {
		struct list_head * e = list_first(&aux);
		struct list_head * f = list_first(&keyboardqueue);
		list_del(e);
		struct task_struct *tsk = list_head_to_task_struct(e);
		if (t->PID == tsk->PID) {
			found = 0;
			list_del(f);
			update_process_state_rr(tsk,&readyqueue);
		}
	}
	return found;
}
void init_circbuff() {
	inpt = circular;
	outpt = circular;
}

void circbuffadd(Byte key) {
	*inpt = key;
	inpt++;
	inpt%=32;
}
Byte circbuffdel() {
	Byte key = *outpt;
	oupt++;
	oupt%=32;
	return *(outpt-1);
}
