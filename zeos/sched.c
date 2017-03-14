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
	struct task_struct *t = list_head_to_task_struct(e); //Task_struct of idle
	t->PID=0;
	allocate_DIR(&t);
	union task_union tu;
	tu.task = *t;					//Task_union where it's task is idle
	tu.stack[KERNEL_STACK_SIZE-1] = cpu_idle;	//Code we want idle task to execute
	tu.stack[KERNEL_STACK_SIZE-2] = 0;		//garbage ebp (it's going to be popped)
	tu.task.ebp_initial_value_pos = &tu.stack[KERNEL_STACK_SIZE-2];	//idle's ebp's initial value position in the stack(esp)
	idle_task = &tu.task;				//global variable with the adress of the idle task
}

void init_task1(void)
{
	struct list_head * e = list_first(&freequeue); //Queue of a task union
	list_del(e);
	struct task_struct * t = list_head_to_task_struct(e);
	t->PID=1;
	allocate_DIR(&t);
	set_cr3(get_DIR(&t));		//The directory is now this one
	set_user_pages(&t);
	union task_union tu;			//Creating a task_union (allocating a stack)
	tu.task = *t;				//Task_union's task = init task
	tss.esp0 = &tu.stack[KERNEL_STACK_SIZE];//The stack is now this stack
	printk("I am now at init_task1!\n");
	printk("Changing to idle!\n");
	task_switch(idle_task); //Li he de passar una task_union de l'idle però només en tinc l'adreça del task_struct...
}

void task_switch(union task_union *t) {
	asm("pushl %esi; pushl %edi; pushl %ebx");	
	inner_task_switch(t);
	asm("popl %ebx; popl %edi; popl %esi");
}

void inner_task_switch(union task_union *t) {
	set_cr3(get_DIR(&t->task));
	tss.esp0 = &t->stack[KERNEL_STACK_SIZE];	
	int newAddr = t->task.ebp_initial_value_pos;
	asm("movl %0, %%esp" : "=r"(newAddr) :);
	asm("popl %ebp");
	asm("RET");
}


void init_sched(){

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

