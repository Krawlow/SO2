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

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

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
	return -38; /*ENOSYS*/
}

int sys_getstats(int pid, struct stats *st) {
	if (pid < 0) {
		return -22; /*EINVAL*/
	}
	if (st == NULL) {
		return -14; /*EFAULT*/
	}
	if (st > 0x108000 + NUM_PAG_DATA*PAGE_SIZE || st < 0x108000) {
		return -14; /*EFAULT*/
	}
	int i;
	for(i=0;i<NR_TASKS;i++){
		if(task[i].task.PID == pid) {
			if (task[i].task.state == 0) return 0;
			else {
				int err = copy_to_user(&current()->info,st,sizeof(struct stats));
				return err;
			}
		}
	}
	return -3; /*ESRCH*/
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
	
	
	
	if (list_empty(&freequeue)) {
		return -12; 
	}
	struct list_head * e = list_first(&freequeue);
	list_del(e);
	struct task_struct * t = list_head_to_task_struct(e);

	union task_union *tu = (union task_union*)t;
	struct task_struct *parent = current();
	copy_data(parent,t,4096);

  	allocate_DIR(t);

	//?
	page_table_entry *pte = get_PT(t);
	page_table_entry *ppte = get_PT(parent);
	copy_data(ppte,pte,TOTAL_PAGES*sizeof(page_table_entry));
	
	int i;
	int page_number_data;
	int pag_init_copia_data = PAG_LOG_INIT_DATA + NUM_PAG_DATA;
	for(i=0;i<NUM_PAG_DATA;i++){
		page_number_data = alloc_frame();
		if (page_number_data==-1) {
			return -12; 
		}
		set_ss_pag(pte,PAG_LOG_INIT_DATA+i,page_number_data);
		set_ss_pag(ppte,pag_init_copia_data + i,pte[PAG_LOG_INIT_DATA+i].bits.pbase_addr);
		copy_data((void*)((PAG_LOG_INIT_DATA + i)*PAGE_SIZE), (void*)((PAG_LOG_INIT_DATA + NUM_PAG_DATA + i)*PAGE_SIZE), PAGE_SIZE);
		del_ss_pag(ppte,pag_init_copia_data + i);
	}
	set_cr3(get_DIR(parent));

	int newPID = t->PID * 2;
	t->PID = newPID;
	t->info.user_ticks = 0;
  t->info.system_ticks = 0;
  t->info.blocked_ticks = 0;
  t->info.ready_ticks = 0;
  t->info.elapsed_total_ticks = get_ticks();
  t->info.total_trans = 0;
  t->info.remaining_ticks = 0;
  t->state = 2;
	PID = newPID;

	tu->stack[KERNEL_STACK_SIZE-19] = 0;
	tu->stack[KERNEL_STACK_SIZE-18] = ret_from_fork;
  t->ebp_initial_value_pos = &tu->stack[KERNEL_STACK_SIZE-19];
	

	extern struct list_head readyqueue;
	update_process_state_rr(t,&readyqueue);
	
	//set_cr3(get_DIR(t));
	
	fork_task = t;
	
	return PID;
}

void sys_exit()
{  
	free_user_pages(current());
	update_process_state_rr(current(),&freequeue);
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
		return -14; //EFAULT
	}
	if (size < 0) {
		return -22; //EINVAL
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
