/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

//#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
//#endif
	
extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}
extern struct list_head freequeue;
struct task_struct *idle_task;
void init_idle (void)
{
    struct list_head *e = list_first(&freequeue);
    list_del(e);
    struct task_struct *t = list_head_to_task_struct(e);
    t->PID=0;
    t->quantum = QUANTUM;
    t->info.user_ticks = 0;
    t->info.system_ticks = 0;
    t->info.blocked_ticks = 0;
    t->info.ready_ticks = 0;
    t->info.elapsed_total_ticks = 0;
    t->info.total_trans = 0;
    t->info.remaining_ticks = 0;
    allocate_DIR(t);
    union task_union *tu = (union task_union*)t;
    tu->stack[KERNEL_STACK_SIZE-1] = cpu_idle;
    tu->stack[KERNEL_STACK_SIZE-2] = 0;      
    tu->task.ebp_initial_value_pos = &tu->stack[KERNEL_STACK_SIZE-2];    
    idle_task = &tu->task;                
}


void init_task1(void)
{
	struct list_head * e = list_first(&freequeue); //Queue of a task union
	list_del(e);
	struct task_struct * t = list_head_to_task_struct(e);
	t->PID=1;
	t->quantum = QUANTUM;
	t->info.user_ticks = 0;
  t->info.system_ticks = 0;
  t->info.blocked_ticks = 0;
  t->info.ready_ticks = 0;
  t->info.elapsed_total_ticks = 0;
  t->info.total_trans = 0;
  t->info.remaining_ticks = 0;
	allocate_DIR(t);
	set_user_pages(t);
	union task_union *tu = (union task_union*)t;
	tss.esp0 = &tu->stack[KERNEL_STACK_SIZE];
	set_cr3(get_DIR(t));
}

void task_switch(union task_union *t) {
    asm("pushl %esi; pushl %edi; pushl %ebx");
    int tiqs = get_ticks();
    t->task.info.ready_ticks+=tiqs-t->task.info.elapsed_total_ticks;
		t->task.info.elapsed_total_ticks = tiqs;   
		t->task.info.total_trans++;  
    inner_task_switch(t);
    asm("popl %ebx; popl %edi; popl %esi");
}
 
void inner_task_switch(union task_union *t) {
    register int xd asm("ebp");
    current()->ebp_initial_value_pos = xd;
    tss.esp0 = &t->stack[KERNEL_STACK_SIZE];   
    set_cr3(get_DIR(&t->task));
    int newAddr = t->task.ebp_initial_value_pos;
    asm("movl %0, %%esp" : :"r"(newAddr));
    asm("popl %ebp");
    asm("ret");
}

unsigned int global_quantum;

void update_sched_data_rr(void) {
	global_quantum--;
	current()->info.remaining_ticks=global_quantum;
}

int needs_sched_rr (void) {
	return global_quantum == 0;
}
extern struct list_head readyqueue;

void update_process_state_rr (struct task_struct *t, struct list_head *dst_queue) {
	if (t == idle_task && list_empty(&readyqueue)); 
	else if (dst_queue == &readyqueue) {
		t->state = 2;//2 = READY
		list_add_tail(dst_queue,&t->list);
	}
	else if (dst_queue == &freequeue) {
		t->state = 0; //0 = ZOMBIE
		list_add_tail(dst_queue,&t->list);
	}
}


void sched_next_rr (void) {
	if (list_empty(&readyqueue) && current() != idle_task) {
		task_switch((union task_union*)idle_task);
	}
	else {
		struct list_head * e = list_first(&readyqueue);
		list_del(e);
		struct task_struct * t = list_head_to_task_struct(e);
		t->quantum = QUANTUM;
		task_switch((union task_union*)t);
	}
	global_quantum = QUANTUM;
} 

void init_sched(){
}

void scheduling() {
	update_sched_data_rr();
	if (needs_sched_rr()){
		update_process_state_rr(current(),&readyqueue);
		sched_next_rr();
	}
}

int get_quantum (struct task_struct *t) {
	return t->quantum;
}

void set_quantum (struct task_struct *t, int new_quantum) {
	t->quantum = new_quantum;
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

