/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <semafors.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
	
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
	int i;
	char c[50];

	for (i=0;i<NR_TASKS;++i){
		if (dir_used[i] == 0) {
		//itoa(i,c);
		//printk("\nPosició que se li assigna: ");
//		printk(c);
	//	printk("\n");
			t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[i];
			++dir_used[i];
			t->dir = i;
			return 1;
		}
	}
	return -1;
	
	/*int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;*/
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
  t->info.remaining_ticks = QUANTUM;
	allocate_DIR(t);
	set_user_pages(t);
	t->program_break = NULL;
	t->heap_pages=0;

	union task_union *tu = (union task_union*)t;
	tss.esp0 = &tu->stack[KERNEL_STACK_SIZE];
	set_cr3(get_DIR(t));
}

void task_switch(union task_union *t) {
    asm("pushl %esi; pushl %edi; pushl %ebx");
   // printk("\n\ntask switch\n\n");
    inner_task_switch(t);
    asm("add $0x4,%esp; popl %ebx; popl %edi; popl %esi");
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
	if (t == idle_task); 
	else if (dst_queue == &readyqueue) {
		t->state = 2;//2 = READY
	}
	else if (dst_queue == &freequeue) {
		t->state = 0; //0 = ZOMBIE
	}
	else {
		t->state = 3; //3 = BLOCKED
	}	
	list_add_tail(&t->list,dst_queue);
}


void sched_next_rr (void) {
	struct task_struct * t;
	if (list_empty(&readyqueue)) {
		t = idle_task;
	//	printk("idle");
	}
	else {
		struct list_head * e = list_first(&readyqueue);
		list_del(e);
		t = list_head_to_task_struct(e);
	}
	int tiqs = get_ticks();
	t->info.ready_ticks+=tiqs-t->info.elapsed_total_ticks;
	t->info.elapsed_total_ticks = tiqs;   
	t->info.total_trans++; 
	t->info.remaining_ticks = get_quantum(t);
	t->state = 1; //1 = RUN
	global_quantum = get_quantum(t);
	task_switch((union task_union*)t);
} 
extern struct semafors semf[20];
void init_sched(){
	global_quantum = QUANTUM;
	int i;
	for (i = 0; i< NR_TASKS; i++)dir_used[i] = 0;
	for (i=0;i<20;i++) {
		semf[i].counter = NULL;
		semf[i].owner = NULL;		
	}
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

