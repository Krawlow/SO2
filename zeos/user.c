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
}int global;
int global2;

void func() {
int pid = getpid();
	write(1,"codi del clon\n",14);
	//fork();
	global2++;
	if (global2 == 7) {
	write(1,"semafor verd\n",strlen("semafor verd\n"));
	global = 1;
	}
	else while(1);
	int pid2 = getpid();
	if (pid2 != pid) write(1,"Fork fet per un clon\n",strlen("Fork fet per un clon\n"));
	exit();
}
char c2[200];

void keyboard() {
	
	read(0,c2,5);
	write(1,c2,5);
	exit();
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
global = 0;
	//runjp();
	//runjp_rank(25,25);
	
	
	//runjp_rank(18,18); //21 bad ?? sometimes bad sometimes good?
	//runjp_rank(27,27); //27 bad
	//runjp_rank(25,25); //25 bad
	
	
	char c[1000];
	int i, pid, fff, ccc;
	for(i=0;i<0;i++) {
	ccc=clone(func,&c[99]);
	ccc=clone(func,&c[99]);
	ccc=clone(func,&c[99]);
	ccc=clone(func,&c[99]);
	ccc=clone(func,&c[99]);
	ccc=clone(func,&c[99]);
	ccc=clone(func,&c[99]);
	ccc=clone(func,&c[99]);
	if(ccc<0) write(1,"unable to clone",strlen("unable to clone"));
	//fff=fork();
	if(fff<0) write(1,"unable to fork",strlen("unable to fork"));
	
	}
	/*if(getpid()==1)while(global == 0){
	itoa(global,c);
	write(1,c,strlen(c));
	}*/
	//write(1,"el pare ha sortit\n",strlen("el pare ha sortit\n"));
	//fork();
	//exit(); 
	clone(keyboard,&c[999]);
	clone(keyboard,&c[999]);
	while(1);
	return(0);


    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

}

