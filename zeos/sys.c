/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <stats.h>

#include <errno.h>

#include <semafors.h>

#define LECTURA 0
#define ESCRIPTURA 1

struct semafors semf[20];

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF;
  if (permissions!=ESCRIPTURA) return -EACCES;
  return 0;
}

int global_PID = 1000;

void system_in() {
	asm("pushl %eax");
	struct task_struct *actual = current();
  	unsigned int current_ticks = get_ticks();
	actual->info.user_ticks+=current_ticks-actual->info.elapsed_total_ticks;
	actual->info.elapsed_total_ticks = current_ticks;
	asm("popl %eax");
}

void system_out() {
	asm("pushl %eax");
	struct task_struct *actual = current();
	unsigned int current_ticks = get_ticks();
	actual->info.system_ticks+=current_ticks-actual->info.elapsed_total_ticks;
	actual->info.elapsed_total_ticks = current_ticks;
	asm("popl %eax");
}

int sys_ni_syscall()
{
	return -ENOSYS;
}

extern unsigned int global_quantum;

int sys_getstats(int pid, struct stats *st) {

	if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT;
	/*if (st == NULL) {
		return -EFAULT;
	}
	if (st > 0x108000 + NUM_PAG_DATA*PAGE_SIZE || st < 0x108000) {
		return -EFAULT;
	}*/
	
	if (pid < 0) return -EINVAL;
	
	int i;
	for(i=0;i<NR_TASKS;i++){
		if(task[i].task.PID == pid) {
			task[i].task.info.remaining_ticks=global_quantum;
			copy_to_user(&task[i].task.info,st,sizeof(struct stats));
			return 0;
		}
	}
	return -ESRCH;
}

int sys_getpid()
{
	return current()->PID;
}

int ret_from_fork() {
	return 0;
}
extern struct list_head freequeue;
struct task_struct *fork_task;
int sys_fork()
{
	int PID=-1;
	
	if (list_empty(&freequeue)) return -ENOMEM;
	
	struct list_head * e = list_first(&freequeue);
	list_del(e);
	struct task_struct * t = list_head_to_task_struct(e);

	union task_union *tu = (union task_union*)t;
	struct task_struct *parent = current();
	copy_data(parent,t,4096);

  	if(allocate_DIR(t)==-1) {
  		return -ENOMEM;
  	}

	page_table_entry *pte = get_PT(t);
	page_table_entry *ppte = get_PT(parent);
	copy_data(ppte,pte,TOTAL_PAGES*sizeof(page_table_entry));
	
	int i;
	int page_number_data;
	int pag_init_copia_data = PAG_LOG_INIT_DATA + NUM_PAG_DATA;
	for(i=0;i<NUM_PAG_DATA;i++){
		page_number_data = alloc_frame();
		if (page_number_data==-1) {
			--dir_used[t->dir];
		//	printk("\n¡¡¡No hi ha frames fisics lliures per fer fork!!!\n");
			int j;
			for (j=0; j<i; j++)
      {
        free_frame(get_frame(pte, PAG_LOG_INIT_DATA+j));
        del_ss_pag(pte, PAG_LOG_INIT_DATA+j);
      }
      list_add_tail(e, &freequeue);
			return -EAGAIN; 
		}
		set_ss_pag(pte,PAG_LOG_INIT_DATA+i,page_number_data);
		set_ss_pag(ppte,pag_init_copia_data + i,pte[PAG_LOG_INIT_DATA+i].bits.pbase_addr);
		copy_data((void*)((PAG_LOG_INIT_DATA + i)*PAGE_SIZE), (void*)((PAG_LOG_INIT_DATA + NUM_PAG_DATA + i)*PAGE_SIZE), PAGE_SIZE);
		del_ss_pag(ppte,pag_init_copia_data + i);
	}
	set_cr3(get_DIR(parent));

	t->PID = ++global_PID;
	t->info.user_ticks = 0;
  t->info.system_ticks = 0;
  t->info.blocked_ticks = 0;
  t->info.ready_ticks = 0;
  t->info.elapsed_total_ticks = get_ticks();
  t->info.total_trans = 0;
  t->info.remaining_ticks = 0;
  t->state = 2;

	tu->stack[KERNEL_STACK_SIZE-19] = 0;
	tu->stack[KERNEL_STACK_SIZE-18] = ret_from_fork;
  t->ebp_initial_value_pos = &tu->stack[KERNEL_STACK_SIZE-19];
	

	extern struct list_head readyqueue;
	update_process_state_rr(t,&readyqueue);
	
	
	fork_task = t;
	
//	printk("forkejant...\n");
	
	return t->PID;
}

int sys_clone(void (*function)(void), void *stack) {
	int PID=-1;
	
	if (list_empty(&freequeue)) return -ENOMEM;
	
	if (!access_ok(VERIFY_WRITE, stack, 4) || !access_ok(VERIFY_READ, function, 4)) return -EFAULT;
	
	struct list_head * e = list_first(&freequeue);
	list_del(e);
	struct task_struct * t = list_head_to_task_struct(e);

	union task_union *tu = (union task_union*)t;
	struct task_struct *parent = current();
	copy_data(parent,t,4096);
	
  char c[50];
//  itoa(t->dir,c);
	//printk("\nPosició que se li assigna al clon: ");
//	printk(c);
	//printk("\n");
  ++dir_used[t->dir];

	t->PID = ++global_PID;
	t->info.user_ticks = 0;
  t->info.system_ticks = 0;
  t->info.blocked_ticks = 0;
  t->info.ready_ticks = 0;
  t->info.elapsed_total_ticks = get_ticks();
  t->info.total_trans = 0;
  t->info.remaining_ticks = 0;
  t->state = 2;

	tu->stack[KERNEL_STACK_SIZE-19] = 0;
	tu->stack[KERNEL_STACK_SIZE-18] = ret_from_fork;
	tu->stack[KERNEL_STACK_SIZE-5] = function;
	tu->stack[KERNEL_STACK_SIZE-2] = stack;
  t->ebp_initial_value_pos = &tu->stack[KERNEL_STACK_SIZE-19];
	

	extern struct list_head readyqueue;
	update_process_state_rr(t,&readyqueue);
	
	
//	printk("clonant...\n");
	return t->PID;
} 

void sys_exit()
{  
	int i;
	for (i=0;i<20;i++)if(semf[i].owner==current())sys_sem_destroy(i);
	update_process_state_rr(current(),&freequeue);
	i = current()->dir;/*((unsigned long)current()->dir_pages_baseAddr - (unsigned long)&dir_pages)/(unsigned long)sizeof(page_table_entry);*/
	--dir_used[i];
//	char c[50];
	//printk("\n-----\nPos: ");
//	itoa(i,c);
	//printk(c);
//	printk("\nReferències: ");
//	itoa(dir_used[i],c);
	//printk(c);
	if(dir_used[i] <= 0) {
		//printk("\nS'allibera l'espai de l'usuari\n");
		free_user_pages(current());
	}
//	printk("\n-----\n");
	current()->PID=-1;
	sched_next_rr();
}
extern int zeos_ticks;
int sys_gettime() {
	return zeos_ticks;
}

int sys_write(int fd, char * buffer, int size) {
	int err = check_fd(fd,ESCRIPTURA);
	if (check_fd(fd,ESCRIPTURA) != 0) {
		return err;
	}
	if (buffer == NULL) {
		return -EFAULT;
	}
	if (size < 0) {
		return -EINVAL;
	}
	char c[10];
	err = 0;
	for (int i = 0; i < size; i+=10) {
		/*for (int j = 0; j < 10; j++) {
			c[j] = buffer[j+i*j];		//copy data from/to
		}*/
		if (size - i < 10) {
			copy_from_user(&buffer[i],&c,size-i);
			err += sys_write_console(c,size-i);
		}
		else {
			copy_from_user(&buffer[i],&c,10);
			err += sys_write_console(c,10);
		}
	}
	return err;	
}

extern struct list_head readyqueue;

//int semaphores[20];

int sys_sem_init (int n_sem, unsigned int value) {
	if (n_sem < 0 || n_sem >19) return -EINVAL;
	if (semf[n_sem].owner!=NULL) return -EBUSY;
	INIT_LIST_HEAD(&semf[n_sem].blockedqueue);
	semf[n_sem].counter = value;
	semf[n_sem].owner = current();
	return 0;
}
int sys_sem_wait(int n_sem) {
	if (n_sem < 0 || n_sem >19) return -EINVAL;
	if (semf[n_sem].owner == NULL) return -EINVAL;
	if (semf[n_sem].counter > 0) --semf[n_sem].counter;
	else {
		current()->sem = n_sem;
		update_process_state_rr(current(),&semf[n_sem].blockedqueue);
		sched_next_rr();
	}
	return current()->sem;
}

int sys_sem_signal(int n_sem) {
	if (n_sem < 0 || n_sem >19) return -EINVAL;
	if (semf[n_sem].owner == NULL) return -EINVAL;
	if(list_empty(&semf[n_sem].blockedqueue)) {
		++semf[n_sem].counter;
	}
	else {
		struct list_head *e = list_first(&semf[n_sem].blockedqueue);
		list_del(e);
		struct task_struct * t = list_head_to_task_struct(e);
		update_process_state_rr(t,&readyqueue);
	}
	return 0;
}
int sys_sem_destroy(int n_sem) {
	if (n_sem < 0 || n_sem >19) return -EINVAL;
	if (semf[n_sem].owner == NULL) return -EINVAL;
	if (current() != semf[n_sem].owner) return -1;
	semf[n_sem].counter = NULL;
	semf[n_sem].owner = NULL;
	while(!list_empty(&semf[n_sem].blockedqueue)) {
		struct list_head *e = list_first(&semf[n_sem].blockedqueue);
		list_del(e);
		struct task_struct * t = list_head_to_task_struct(e);
		t->sem = -1;
		update_process_state_rr(t,&readyqueue);
	}
	return 0;	
}
