/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

int fork (void) {
	asm("movl $2, %eax"); //Posa 2 a %eax
	asm("int $0x80;");
	register int err asm("eax");
	if (err < 0) {
		errno = -err;		
		return -1;
	}
	else return err;
}

int getpid(void) {
	asm("movl $20, %eax"); //Posa 20 a %eax
	asm("int $0x80;");
	register int err asm("eax");
	if (err < 0) {
		errno = -err;		
		return -1;
	}
	else return err;
}

int write (int fd, char * buffer, int size) {	//Write wrapper
	asm("pushl %ebx;"); //ebx,esi,edi
	asm("movl 8(%ebp), %ebx;movl 12(%ebp), %ecx;movl 16(%ebp), %edx;");
	asm("movl $4, %eax"); //Posa 4 a %eax
	asm("int $0x80;");
	asm("popl %ebx;");
	register int err asm("eax");
	if (err < 0) {
		errno = -err;		
		return -1;
	}
	else return err;	//S'ha de retornar el "resultat" si es positiu, si es negatiu s'ha de fer lu de errno i retornar -1
}

void exit(void) {
	asm("movl $1, %eax"); //Posa 1 a %eax
	asm("int $0x80;");
	register int err asm("eax");
	if (err < 0) {
		errno = -err;		
		return -1;
	}
	else return err;
}

int gettime() {
	asm("movl $10, %eax"); //Posa 10 a %eax
	asm("int $0x80;");
	register int err asm("eax");
	if (err < 0) {
		errno = -err;		
		return -1;
	}
	else return err;
}

int get_stats (int pid, struct stats *st) {
	asm("pushl %ebx;");
	asm("movl 8(%ebp), %ebx;movl 12(%ebp), %ecx;");
	asm("movl $35, %eax"); //Posa 35 a %eax
	asm("int $0x80;");
	asm("popl %ebx;");
	register int err asm("eax");
	if (err < 0) {
		errno = -err;		
		return -1;
	}
	else return 0;
}

void perror() {
	char b[20];
	itoa(errno,b);
	write(1,b,strlen(b));
}

