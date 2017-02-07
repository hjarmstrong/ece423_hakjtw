/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */


#include <stdio.h>
#include <limits.h>
#include "system.h"
#include <sys/alt_irq.h>
#include <priv/alt_legacy_irq.h>
#include "altera_avalon_pio_regs.h"

volatile alt_u32 comand = 0;

static void timer1_isr(void *context)
{
	//ack interupt
	IOWR(TIMER_1_BASE, 0, 0);

    comand = IORD(KEY_BASE, 0);
}

static void pio_isr(void *context)
{
	//ack interupt
    IOWR(KEY_BASE, 3, 0);

    comand = IORD(KEY_BASE, 0);
}

int main(void)
{
  printf("Hello Japan!\n");

  IOWR(KEY_BASE, 2, 0xF); // interupt on all buttons
  alt_irq_register(KEY_IRQ, (void *)0, pio_isr);
  alt_irq_enable(KEY_IRQ);

  // Period
  IOWR(TIMER_1_BASE, 3, 0xFEE6);
  IOWR(TIMER_1_BASE, 2, 0xB280);
 //                      START   CONT   ITO
  IOWR(TIMER_1_BASE, 1, (1<<2) |(1<<1) | 1 );

  alt_irq_register(TIMER_1_IRQ, (void *)0, timer1_isr);
  alt_irq_enable(TIMER_1_IRQ);

  while(1)
  {
		printf("Command is %d \n", comand);
	/*	for(int i =0; i <INT_MAX; i++)
		{
			for(int j =0; i <INT_MAX; i++)
			{
				comand = IORD(KEY_BASE, 0);
			}
			// Please don't optimize me out!!!
		}*/
		while(1)
		{
		    int k = comand;
		}
  }
}
