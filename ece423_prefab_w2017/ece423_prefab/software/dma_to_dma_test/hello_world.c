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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <io.h>
#include <sys/alt_cache.h>
#include <malloc.h>
#include <priv/alt_file.h>
#include <system.h>
#include <altera_msgdma.h>
#include <altera_msgdma_descriptor_regs.h>
#include <altera_msgdma_csr_regs.h>

//#define DESC_CONTROL      (ALTERA_MSGDMA_DESCRIPTOR_CONTROL_PARK_READS_MASK | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_SOP_MASK | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MASK | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GO_MASK)  // Also set the park bit so that we can let the mSGDMA worry about the frame DUPLICATION

#define DESC_CONTROL ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK

volatile int got_mem_yet = 0;

static void callvack(void *context)
{
	got_mem_yet = 1;
}


int main()
{
  printf("Hello from Nios II!\n");

  volatile uint32_t *japan = malloc(sizeof(uint32_t) * 40); //{0xFFFBFFFF, 0xDEADCAFE};
  volatile uint32_t *japan_out = malloc(sizeof(uint32_t) * 40);

  japan[0] = 0xFFFBFFFF;
  japan[1] = 0xDEADCAFE;

  for(unsigned int i = 2; i< 40; i++)
	  japan[i] = i;

  for(unsigned int i = 0; i< 40; i++)
	  japan_out[i] = 0xFFFFFFFF;

  alt_dcache_flush_all();

  alt_msgdma_dev *to_dma, *from_dma;
  alt_msgdma_standard_descriptor *in_base, *out_base;

  in_base = malloc(sizeof(alt_msgdma_standard_descriptor));//(alt_msgdma_standard_descriptor*) memalign(32,
		  //3 * sizeof(alt_msgdma_standard_descriptor));
  out_base = malloc(sizeof(alt_msgdma_standard_descriptor));//(alt_msgdma_standard_descriptor*) memalign(32,
		  //3 * sizeof(alt_msgdma_standard_descriptor));

  to_dma = alt_msgdma_open(TO_IDCT_HWACEL_CSR_NAME);
  from_dma = alt_msgdma_open(FROM_IDCT_HWACEL_CSR_NAME);

  while(0 != alt_msgdma_construct_standard_mm_to_st_descriptor(to_dma,
  				out_base,
  				(alt_u32 *) japan, 1280,
  				DESC_CONTROL));

  while(0 != alt_msgdma_construct_standard_st_to_mm_descriptor(from_dma,
  				in_base,
  				(alt_u32 *) japan_out, 1280,
  				DESC_CONTROL));

  /*alt_msgdma_register_callback(
  	to_dma,
  	callvack,
  	ALTERA_MSGDMA_GLOBAL_INTERUPT_MASK,
  	0);

  alt_msgdma_register_callback(
  	from_dma,
  	callvack,
  	DESC_CONTROL,
  	0);*/

	while (alt_msgdma_standard_descriptor_sync_transfer(to_dma,
			out_base)
			!= 0);

	//while(!got_mem_yet);
	//got_mem_yet = 0;

	while (alt_msgdma_standard_descriptor_sync_transfer(from_dma,
			in_base)
			!= 0);

	//while(!got_mem_yet);
    //unsigned int guckjdfashjkde = japan_out[0];

	alt_dcache_flush_all();
	printf("%p ::: %p \n", japan, japan_out);

    for(uint32_t i = 0; i < 40; i++)
    {
    	alt_dcache_flush_all();
	    printf("goted\n [%u]: %X\n", i, japan_out[i]);
    }

    printf("\n");


  return 0;
}
