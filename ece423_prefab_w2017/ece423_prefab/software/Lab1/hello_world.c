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
#include <system.h>
#include "ece423_sd/sd_lib.h"
#include "ece423_vid_ctl/ece423_vid_ctl.h"

int main()
{
  printf("Hello from Nios II!\n");

  /*SDLIB_Init(SD_CONT_0_BASE);

  FAT_HANDLE sd_fat_handle = Fat_Mount();

  FAT_BROWSE_HANDLE browse_handle;

  Fat_FileBrowseBegin(sd_fat_handle, &browse_handle);

  bool video_found_flag = FALSE;
  FILE_CONTEXT file_context;

  while(!video_found_flag)
  {
	  Fat_FileBrowseNext(&browse_handle, &file_context);
	  printf(Fat_GetFileName(&file_context));
	  printf("\n");
  }*/


  ece423_video_display *disp = ece423_video_display_init(VIDEO_DMA_CSR_NAME,640,480,2);
  alt_u32 *frame1 = ece423_video_display_get_buffer(disp);

  for(int i =0; i < 10000; i++)
  {
	  frame1[i] = 0x77F4390;
  }

  ece423_video_display_register_written_buffer(disp);

  ece423_video_display_switch_frames(disp);

  while(1);

  return 0;
}
