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
	printk("Va a una pos de la taula de syscalls invalida");
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}
extern zeos_ticks;
void sys_gettime() {
	printk("Arriba a fer la syscall de gettime");
	return zeos_ticks;
}

void sys_write(int fd, char * buffer, int size) {
	//asm("movl -12(%ebp), %ebx;movl -8(%ebp), %ecx;movl -4(%ebp), %edx;");
	if (check_fd(fd,ESCRIPTURA) != 0 || buffer == NULL || size < 0) return -1;
	//			copy data from/to
	
	int err = sys_write_console(buffer,size);
	if (err < 0) {
		//errno = -err;
		return -1;
	}
	else return err;
}
