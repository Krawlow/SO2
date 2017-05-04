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
	if(!list_empty(&keyboardqueue)) {
		unblock(list_head_to_task_struct(list_first(&keyboardqueue)));
		block();
	}
	else {
		int i;
		if (count < circbuffsize(&teclat_buff)) {
			for(i=0;i<count;i++) {
				buf[i]=circbuffdel(&teclat_buff);
			}
			return count;
		}
		else if (circbufffull(&teclat_buff)) {
			for(i=0;i<circbuffsize(&teclat_buff);i++) {
				buf[i]=circbuffdel(&teclat_buff);
			}
			current()->restants = count-circbuffsize(&teclat_buff);
			block();
			return i-1;
		}
		else {
			current()->restants = count;
			block();
		}
		return 0;
	}
}
