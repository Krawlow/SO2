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
	allocate_DIR(t);
	set_user_pages(t);
	union task_union *tu = (union task_union*)t;
	tss.esp0 = &tu->stack[KERNEL_STACK_SIZE];
	set_cr3(get_DIR(t));
}

void task_switch(union task_union *t) {
    asm("pushl %esi; pushl %edi; pushl %ebx");     
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

