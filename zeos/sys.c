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
	struct task_struct *actual = current();
  unsigned int current_ticks = get_ticks();
	actual->info.user_ticks+=current_ticks-actual->info.elapsed_total_ticks;
	actual->info.elapsed_total_ticks = current_ticks;
}

void system_out() {
	struct task_struct *actual = current();
	unsigned int current_ticks = get_ticks();
	actual->info.system_ticks+=current_ticks-actual->info.elapsed_total_ticks;
	actual->info.elapsed_total_ticks = current_ticks;
}

int sys_ni_syscall()
{
	system_in();
	system_out();
	return -38; /*ENOSYS*/
}

int sys_getstats(int pid, struct stats *st) {
	if (pid < 0) {
		system_out();
		return -9; /*EBADF*/
	}
	int i;
	for(i=0;i<NR_TASKS;i++){
		if(task[i].task.PID == pid) {
			int err = copy_to_user(&current()->info,st,sizeof(struct stats));
			system_out();
			return err;
		}
	}
	system_out();
	return -9; //EBADF
}

int sys_getpid()
{
	system_in();
	system_out();
	return current()->PID;
}

void ret_from_fork() {
	system_in();
	system_out();
	return 0;
}
extern struct list_head freequeue;
struct task_struct *fork_task;
int sys_fork()
{
	
	system_in();
	int PID=-1;
	
	
	
	if (list_empty(&freequeue)) {
		system_out();
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
			system_out();
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
	list_add_tail(&t->list,&readyqueue);
	
	//set_cr3(get_DIR(t));
	
	fork_task = t;

	system_out();
	
	return PID;
}

void sys_exit()
{  
	system_in();
	free_user_pages(current());
	update_process_state_rr(current(),&freequeue);
	sched_next_rr();
}
extern int zeos_ticks;
int sys_gettime() {
	system_in();
	system_out();
	return zeos_ticks;
}

int sys_write(int fd, char * buffer, int size) {
	int err = check_fd(fd,ESCRIPTURA);
	if (check_fd(fd,ESCRIPTURA) != 0) {
		system_out();
		return err;
	}
	if (buffer == NULL) {
		system_out();
		return -14; //EFAULT
	}
	if (size < 0) {
		system_out();
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
	system_out();
	return err;	
}
