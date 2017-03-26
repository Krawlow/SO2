#include <libc.h>

char buff[24];

int pid;

long inner(long n)
{
	int i;
	long suma;
	suma = 0;
	for(i=0; i<n; i++) suma = suma + i;
	return suma;
}

long outer(long n)
{
	int i;
	long acum;
	acum = 0;
	for(i=0; i<n; i++) acum = acum + inner(i);
	return acum;
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
	long count, acum;
	count = 75;
	acum = 0;
	acum = outer(count);
	char buffer[]="hello";
	//gettime();
	//write(1,buffer,strlen(buffer));
//	pid=getpid();
	//itoa(pid,buff);
//	write(1,buff,strlen(buff));
	//while(gettime() < 100);
//	pid=getpid();
	//itoa(pid,buff);
	//write(1,buff,strlen(buff));
	//perror();
	//while(zeos_ticks < 1000);
	//task_switch(idle);
	//fork();
	runjp();
	while(1);
	return(0);


    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

}
