//
//  main.c
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/23/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include "ece423_vid_ctl/ece423_vid_ctl.h"
#include "decoder/mjpeg423_decoder.h"
#include <system.h>
#include <sys/alt_irq.h>
#include <priv/alt_legacy_irq.h>
#include <altera_avalon_pio_regs.h>
#include "command.h"

//number of frames to encode
#define NUM_FRAMES 5
//starting bmp number for the encoder. I.e., if the first bmp to encode is "name0002.bmp", set it to 2.
#define START_BMP 0
//bmp input stride. The encoder encodes one bmp every BMP_STRIDE.
//I.e., if set to 1, every bmp is included; if set to 2, one bmp every 2; etc.
//This is useful to adjust the frame rate compared to the original bmp sequence.
#define BMP_STRIDE 1

//max separation between successive I frames. The encoder will insert an I frame at most every MAX_I_INTERVAL frames.
#define MAX_I_INTERVAL 24
//resolution: width
#define WIDTH 640
//resolution: height
#define HEIGHT 480

bool is_video_playing = TRUE;

volatile uint32_t ready_frames = 0;

volatile BUTTON_COMMANDS command = NO_CMD;

static void timer1_isr(void *context)
{
	//ack interupt
	IOWR(TIMER_1_BASE, 0, 0);

#ifdef PERIODIC_VIDEO
    if(is_video_playing && (disp != (void *)0) && ready_frames)
	{
    	ready_frames--;
	    ece423_video_display_switch_frames(disp);
	}
#endif
}

static void pio_isr(void *context)
{
	int buttons_pressed;
	//ack interupt
    IOWR(KEY_BASE, 3, 0);

    buttons_pressed = IORD(KEY_BASE, 0);

    if (command != NO_CMD)
    {
   	    return;
    }

    if((buttons_pressed & (1 << 0)) == 0)
    {
    	if(is_video_playing)
    	{
    		command = PAUSE;
    	}
    	else
    	{
    		command = START;
    	}
    }
    else if((buttons_pressed & (1 << 1)) == 0)
    {
    	command = CYCLE;
    }
    else if((buttons_pressed & (1 << 2)) == 0)
    {
    	command = REWIND;
    }
    else if((buttons_pressed & (1 << 3)) == 0)
    {
    	command = FORWARD;
    }
}

static void interrupts_init(void)
{
	  IOWR(KEY_BASE, 2, 0xF); // interupt on all buttons
	  alt_irq_register(KEY_IRQ, (void *)0, pio_isr);
	  alt_irq_enable(KEY_IRQ);

	  // Period (2 seconds)
	  //IOWR(TIMER_1_BASE, 3, 0x0EE6);
	  //IOWR(TIMER_1_BASE, 2, 0xB280);

	  //1 second
	  //IOWR(TIMER_1_BASE, 3, 0x0832);
	  //IOWR(TIMER_1_BASE, 2, 0x1560);

	  //0.7 seconds
	  IOWR(TIMER_1_BASE, 3, 0x0537);
	  IOWR(TIMER_1_BASE, 2, 0x24E0);

	 //                      START   CONT   ITO
	  IOWR(TIMER_1_BASE, 1, (1<<2) |(1<<1) | 1 );

	  alt_irq_register(TIMER_1_IRQ, (void *)0, timer1_isr);
	  alt_irq_enable(TIMER_1_IRQ);
}

int main (int argc, const char * argv[])
{
	  SDLIB_Init(SD_CONT_0_BASE);

	  FAT_HANDLE sd_fat_handle = Fat_Mount();

	  FAT_BROWSE_HANDLE browse_handle;

	  Fat_FileBrowseBegin(sd_fat_handle, &browse_handle);

	  int video_found_flag = FALSE;
	  FILE_CONTEXT file_context;

	  char *p_filename;

	  interrupts_init();

	  while(1)
	  {
		  if(Fat_FileBrowseNext(&browse_handle, &file_context) == FALSE)
			  Fat_FileBrowseBegin(sd_fat_handle, &browse_handle);

		if(Fat_CheckExtension(&file_context, ".MPG"))
		{
		    p_filename = Fat_GetFileName(&file_context);
			video_found_flag = TRUE;
		}


		if(video_found_flag)
		{
			mjpeg423_decode(p_filename, sd_fat_handle);
			if(command != CYCLE)
			{
				while(ready_frames != 0)
				{
				    if(!is_video_playing)
					{
				    	while(1);
						ready_frames = 0;
						ece423_video_display_switch_frames(disp);
					}
				}
				is_video_playing = FALSE;
			}
			else
			{
				disp->buffer_being_written = ((disp->buffer_being_displayed + 1)
						% disp->num_frame_buffers);
				ready_frames = 0;
			    command = NO_CMD;
			}
			video_found_flag = FALSE;

			//When cycling, if the video was paused, then cycle to the next video paused. And vice versa.

		}
	  }
}
