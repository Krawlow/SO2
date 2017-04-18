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

void func() {
	write(1,"codi del clon\n",14);
	exit();
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
	//fork();
	//write(1,buff,strlen(buff));
	//perror();
	//while(zeos_ticks < 1000);
	//task_switch(idle);
//	char *p;
//	if (fork() ==0) exit();
//	else exit();
//while(1){write(1,p,strlen(p));};
	//runjp();
	//runjp_rank(18,18); //21 bad ?? sometimes bad sometimes good?
	//runjp_rank(27,27); //27 bad
	//runjp_rank(25,25); //25 bad
	
	
//	int it = 1000000;
//	while(0!=it--){write(1,p,strlen(p));}
//	if (p == "fill")exit();
	char c[100];
	int i, pid, fff, ccc;
	for(i=0;i<6;i++) {
	fff=fork();
	if(fff<0) write(1,"unable to fork|||",strlen("unable to fork|||"));
	ccc=clone(func,&c[99]);
	if(ccc<0) write(1,"unable to clone|||",strlen("unable to clone|||"));
	}
	exit(); //while(1);
	return(0);


    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

}

