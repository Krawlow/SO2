/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>


/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %ebx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %edx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %edx; \
      pushl %ecx; \
      pushl %ebx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

#define RESTORE_ALL \
      popl %gs; \
      popl %fs; \
      popl %es; \
      popl %ds; \
      popl %eax; \
      popl %ebp; \
      popl %edi; \
      popl %esi; \
      popl %edx; \
      popl %ecx; \
      popl %ebx

#define EOI \
      movb $0x20, %al; \
      outb %al, $0x20

ENTRY(keyboard_handler) 
      SAVE_ALL
      EOI
      call RSR
      RESTORE_ALL
      iret
ENTRY(system_call_handler)
	  SAVE_ALL
	  cmpl $0, %eax				//Negative syscall number
	  jl err
	  cpml $MAX_SYSCALL, %eax	//Out of bounds
	  jg err
	  call *sys_call_table(, %eax, 0x04);	//Calling the sys call
	  jmp fin
err:
	  movl $-ENOSYS, %eax		//Returns an error
fin:
	  movl %eax, 0x18(%esp) //push %eax
	  RESTORE_ALL
	  iret
ENTRY(pass_sys_params)
	  movl -12(%esp), %ebx
	  movl -8(%esp), %ecx
	  movl -4(%esp), %edx
	  iret
ENTRY(write_id)
	  movl $4, %eax
	  iret
ENTRY(sys_call_table)
	  .long sys_ni_syscall // 0
	  .long sys_ni_syscall // 1
	  .long sys_ni_syscall // 2
	  .long sys_ni_syscall // 3
	  .long sys_write // 4
	  .long sys_ni_syscall // 5
	  .long sys_ni_syscall // 6
	  .long sys_ni_syscall // 7
	  .long sys_ni_syscall // 8
	  .long sys_ni_syscall // 9
	  .long sys_gettime // 10
.globl MAX_SYSCALL
MAX_SYSCALL = (. - sys_call_table) / 4
ENTRY(clock_handler)
	  SAVE_ALL
	  EOI
	  call clock
	  RESTORE_ALL
	  iret




