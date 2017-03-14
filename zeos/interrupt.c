/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

char char_map[] =
{
  'C','C','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','C','C',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','C','C','a','s',
  'd','f','g','h','j','k','l','ñ',
  'C','º','C','ç','z','x','c','v',
  'b','n','m',',','.','-','C','*',
  'C','C','C','C','C','C','C','C',
  'C','C','C','C','C','C','C','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','C','C','C','<','C',
  'C','C','C','C','C','C','C','C',
  'C','C'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void keyboard_handler ();

void clock_handler ();

void system_call_handler ();

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */

  setInterruptHandler(33, keyboard_handler, 0);

  setInterruptHandler(32, clock_handler, 0);

	setTrapHandler(0x80, system_call_handler, 3);

  set_idt_reg(&idtR);
}
extern int zeos_ticks;
void clock() {
	zeos_ticks++;
	zeos_show_clock();
}

void RSR() {
  Byte key = inb(0x60);
  if((key & 0x80) == 0x80) { //0x80
		int i = key & 0x7f;
		char c = char_map[i];
		printc_xy(0,0,c);
  }
}

