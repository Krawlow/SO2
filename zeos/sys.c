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
struct task_struct *actual = current();

void system_in() {
  unsigned int current_ticks = get_ticks();
	actual->user_ticks+=current_ticks-actual->elapsed_total_ticks;
	actual->elapsed_total_ticks = current_ticks;
}

void system_out() {
	unsigned int current_ticks = get_ticks();
	actual->system_ticks+=current_ticks-actual->elapsed_total_ticks;
	actual->elapsed_total_ticks = current_ticks;
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
		if(task[i]->PID == pid) {
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

int sys_fork()
{
	system_in();
	int PID=-1;
	
	extern struct list_head freequeue;
	if (list_empty(&freequeue)) {
		system_out();
		return -12; /*-61 ENODATA*/ /*-12 ENOMEM*/
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
	for(i=0;i<NUM_PAG_DATA;i++){
		page_number_data = alloc_frame();
		if (page_number_data==-1) {
			system_out()
			return -12; /*-61 ENODATA*/ /*-12 ENOMEM*/
		}
		set_ss_pag(pte,pte[PAG_LOG_INIT_DATA+i],page_number_data);
	}
	
	int pag_init_copia_data = NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA + 1;
	for(i=0;i<NUM_PAG_DATA;i++){
		set_ss_pag(ppte,ppte[pag_init_copia_data + i],pte[PAG_LOG_INIT_DATA+i].bits.pbase_addr);
		copy_data(ppte[PAG_LOG_INIT_DATA + i],ppte[pag_init_copia_data + i],sizeof(page_table_entry));
		del_ss_pag(ppte,ppte[pag_init_copia_data + i]);
	}
	set_cr3(get_DIR(parent));

	int newPID = t->PID * 2;
	t->PID = newPID;
	t->info = {0,0,0,0,0,0,0};
	PID = newPID;

	tu->stack[KERNEL_STACK_SIZE-19] = 0;
	tu->stack[KERNEL_STACK_SIZE-18] = ret_from_fork;
  tu->ebp_initial_pos_value = &tu->stack[KERNEL_STACK_SIZE-19];
	

	extern struct list_head readyqueue;
	list_add_tail(&t->list,&readyqueue);

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
