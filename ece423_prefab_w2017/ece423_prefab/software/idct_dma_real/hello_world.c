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
#include "common/mjpeg423_types.h"

//#define DESC_CONTROL      (ALTERA_MSGDMA_DESCRIPTOR_CONTROL_PARK_READS_MASK | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_SOP_MASK | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MASK | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GO_MASK)  // Also set the park bit so that we can let the mSGDMA worry about the frame DUPLICATION

#define DESC_CONTROL ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK
#define DCT_BLOCK_SIZE (32)
#define COLOUR_BLOCK_SIZE (16)

volatile int got_mem_yet = 0;

static void dma_done_cb (void *context)
{
	got_mem_yet = 1;
}


int main()
{
  printf("Hello from Nios II!\n");

  //uint32_t *japan = malloc(sizeof(uint32_t) * 40); //{0xFFFBFFFF, 0xDEADCAFE};
  //uint32_t *japan_out = malloc(sizeof(uint32_t) * 40);

  //Stand-in DCAC type and color block
  uint32_t *p_dct_block_buffer = malloc(sizeof(uint32_t) * DCT_BLOCK_SIZE);
  uint32_t *p_colour_block_buffer = malloc(sizeof(uint32_t) * COLOUR_BLOCK_SIZE);


  //japan[0] = 0xFFFBFFFF;
  //japan[1] = 0xDEADCAFE;

//  uint32_t t;
//  scanf("%d", &t);
//  for(unsigned int i = 2; i < 40; i++)
//  {
//	  japan[i] = ~(i+t);
////	  IOWR_32DIRECT(japan,i*4, i);
////	  alt_dcache_flush_all();
//  }

//  for(unsigned int i = 0; i < 40; i++)
//  {
//	  IOWR_32DIRECT(japan_out,i*4, 0xCAFECAFE); //japan_out[i] = 0xCAFECAFE;
////	  alt_dcache_flush_all();
//  }

  //japan[38] = 0xFFFFFFFF;

  alt_dcache_flush_all();

  alt_msgdma_dev *mm_to_st_dma_dev, *st_to_mm_dma_dev;
  alt_msgdma_standard_descriptor *mm_to_st_desc, *st_to_mm_desc;

  mm_to_st_desc = malloc(sizeof(alt_msgdma_standard_descriptor));//(alt_msgdma_standard_descriptor*) memalign(32,
		  //3 * sizeof(alt_msgdma_standard_descriptor));
  st_to_mm_desc = malloc(sizeof(alt_msgdma_standard_descriptor));//(alt_msgdma_standard_descriptor*) memalign(32,
		  //3 * sizeof(alt_msgdma_standard_descriptor));

  mm_to_st_dma_dev = alt_msgdma_open(TO_IDCT_HWACEL_CSR_NAME);
  st_to_mm_dma_dev = alt_msgdma_open(FROM_IDCT_HWACEL_CSR_NAME);

  while(0 != alt_msgdma_construct_standard_mm_to_st_descriptor(mm_to_st_dma_dev,
  				mm_to_st_desc,
  				(alt_u32 *) p_dct_block_buffer , sizeof(uint32_t) * DCT_BLOCK_SIZE,
  				DESC_CONTROL));

  while(0 != alt_msgdma_construct_standard_st_to_mm_descriptor(st_to_mm_dma_dev,
  				st_to_mm_desc,
  				(alt_u32 *) p_colour_block_buffer, sizeof(uint32_t) * COLOUR_BLOCK_SIZE,
  				DESC_CONTROL));

  alt_msgdma_register_callback(
  	mm_to_st_dma_dev,
  	dma_done_cb,
  	ALTERA_MSGDMA_CSR_GLOBAL_INTERRUPT_MASK,
  	0);

  alt_msgdma_register_callback(
  	st_to_mm_dma_dev,
  	dma_done_cb,
  	ALTERA_MSGDMA_CSR_GLOBAL_INTERRUPT_MASK,
  	0);


	while (alt_msgdma_standard_descriptor_async_transfer(mm_to_st_dma_dev,
			mm_to_st_desc)
			!= 0);

	while(!got_mem_yet);
	got_mem_yet = 0;

	while (alt_msgdma_standard_descriptor_async_transfer(st_to_mm_dma_dev,
			st_to_mm_desc)
			!= 0);


	while(!got_mem_yet);

	alt_dcache_flush_all();
//	printf("%p ::: %p \n", japan, japan_out);
//
//    for(uint32_t i = 0; i < 40; i++)
//    {
//    	alt_dcache_flush_all();
//    	uint32_t real_talk = IORD_32DIRECT(japan_out,i*4);
//	    printf("goted\n [%u]: %X\n", i, real_talk);//japan_out[i]);
//    }
//
//    for(unsigned int i = 2; i < 40; i++)
//    {
//  	  printf("%X - ", ~japan[i]);
//  //	  IOWR_32DIRECT(japan,i*4, i);
//  //	  alt_dcache_flush_all();
//    }
//    printf("\n");


  return 0;
}
