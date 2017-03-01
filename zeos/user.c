#include <libc.h>

char buff[24];

int pid;

int write (int fd, char * buffer, int size) {	//Write wrapper
	asm(	"movl -12(%ebp), %%ebx
		movl -8(%ebp), %%ecx
		movl -4(%ebp), %%edx");
	asm("movl $4, %%eax"); //Posa 4 a %eax
	int $0x80;
	int err;
	asm(	"movl %%eax, %%1"
		:"=r" (err)	//output
		:		//input
		:		//clobbered register
		);
	if (err < 0) {
		//update errno = err;		
		return -1;
	}
	else return err;	//S'ha de retornar el "resultat" si es positiu, si es negatiu s'ha de fer lu de errno i retornar -1
}

int gettime() {
	asm("movl $10, %%eax"); //Posa 10 a %eax
	int $0x80;
	int err;
	asm(	"movl %%eax, %%1"
		:"=r" (err)	//output
		:		//input
		:		//clobbered register
		);
	if (err < 0) {
		//update errno = err;		
		return -1;
	}
	else return err;
}

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
	char buffer[5]="hello";
	write(1,&buffer,5);
	while(1);
	return(0);


    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

}
