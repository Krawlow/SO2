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

int write (int fd, char * buffer, int size) {	//Write wrapper
	asm("movl -12(%ebp), %ebx;movl -8(%ebp), %ecx;movl -4(%ebp), %edx;");
	asm("movl $4, %eax"); //Posa 4 a %eax
	asm("int $0x80;");
	register int err asm("eax");
	if (err < 0) {
		errno = err;		
		return -1;
	}
	else return err;	//S'ha de retornar el "resultat" si es positiu, si es negatiu s'ha de fer lu de errno i retornar -1
}

int gettime() {
	asm("movl $10, %eax"); //Posa 10 a %eax
	asm("int $0x80;");
	register int err asm("eax");
	if (err < 0) {
		errno = err;		
		return -1;
	}
	else return err;
}

void perror() {
	write(1,errno,strlen(errno));
}

