#include <io.h>
#include <utils.h>
#include <list.h>
#include <keyboard.h>

// Queue for blocked processes in I/O 
struct list_head blocked;

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
	if(!list_empty(&keyboardqueue)) {
		block();
	}
	else {
		int i;
		if (count < circbuffsize()) {
			for(i=0;i<count;i++) {
				buf[i]=circbuffdel();
			}
			return count;
		}
		else if (circbufffull()) {
			for(i=0;i<circbuffsize();i++) {
				buf[i]=circbuffdel();
			}
			current()->restants = count-circbuffsize();
			block();
		}
		else {
			current()->restants = count;
			block();
		}
	}
}
