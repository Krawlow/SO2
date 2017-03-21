/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	//printk("Va a una pos de la taula de syscalls invalida");
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
	int PID=-1;

	if (list_empty(&freequeue)) return -12; /*-61 ENODATA*/ /*-12 ENOMEM*/
	struct list_head * e = list_first(&freequeue);
	list_del(e);
	struct task_struct * t = list_head_to_task_struct(e);

	union task_union *tu = (union task_union*)t;
	struct task_struct *parent = current();
	copy_data(parent,t,4096);

  	allocate_DIR(t);

	int page_number_data = alloc_frame();
	if (page_number==-1) return -12; /*-61 ENODATA*/ /*-12 ENOMEM*/

	page_table_entry *pte = get_PT(t);
	page_table_entry *ppte = get_PT(parent);
	copy_data(ppte,pte,TOTAL_PAGES*sizeof(page_table_entry));
	
	*pte.entry = 0;
	*pte.bits.pbase_addr = page_number_data;
	*pte.bits.user = 1;
	*pte.bits.present = 1;
	*pte.bits.rw = 1;

	copy_data(*ppte.bits.pbase_addr,*pte.bits.pbase_addr,/*size*/);
	
	
	return PID;
}

void sys_exit()
{  
}
extern int zeos_ticks;
int sys_gettime() {
	//printk("Arriba a fer la syscall de gettime");
	return zeos_ticks;
}

int sys_write(int fd, char * buffer, int size) {
	int err = check_fd(fd,ESCRIPTURA);
	if (check_fd(fd,ESCRIPTURA) != 0) return err;
	if (buffer == NULL) return -14; //EFAULT
	if (size < 0) return -22; //EINVAL
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
