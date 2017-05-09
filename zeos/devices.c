#include <io.h>
#include <utils.h>
#include <list.h>
#include <keyboard.h>

// Queue for blocked processes in I/O 
struct list_head blocked;
extern struct cbuffer teclat_buff;

int sys_write_console(char *buffer,int size)
{
  int i;

//printk("Arriba a fer el sys write console");
  
  for (i=0; i<size; i++)
    printc(buffer[i]);
  
  return size;
}
extern struct list_head keyboardqueue;
int sys_read_keyboard(char *buf,int count) {
	current()->restants = count;
	if(!list_empty(&keyboardqueue)) {
		printk("em fico a la cua\n");
		block();
	}
	int i,total = 0;
	char buff[100];
	while(current()->restants > 0) {
		if (current()->restants <= buff_used(&teclat_buff)) {
			//AND the distance between opt and ipt is the same amount we want to read (so we don't read before the buffer is filled completely
			for(i=0;i<current()->restants;i++) {
				buff[total]=circbuffdel(&teclat_buff);
				total++;
			}
			copy_to_user(&buff,buf,i);
			current()->restants -= i;
		}
		else {
			if (circbufffull(&teclat_buff)) {
				printk("buffer ple, el buido i vaig a la cua amb prioritat\n");
				for(i=0;i<circbuffsize(&teclat_buff);i++) {
					//char c[1] = {circbuffdel(&teclat_buff)};
					//copy_to_user(&c,&buf[i],sizeof(c));
					buff[total]=circbuffdel(&teclat_buff);
					total++;
				}
				copy_to_user(&buff,buf,i);
				current()->restants -= i;
				blockP();
			}
			else {
				printk("esperare a que s'ompli el buffer\n");
				blockP();
			}
		}
	}
	return total;
}
