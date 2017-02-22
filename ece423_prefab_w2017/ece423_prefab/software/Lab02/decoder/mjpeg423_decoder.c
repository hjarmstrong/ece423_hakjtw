//
//  mjpeg423_decoder.c
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/24/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include "../common/mjpeg423_types.h"
#include "mjpeg423_decoder.h"
#include "../common/util.h"
#include "../ece423_sd/sd_lib.h"
#include "../ece423_vid_ctl/ece423_vid_ctl.h"
#include <system.h>
#ifdef PROFILEING
#include <sys/alt_timestamp.h>
#endif
#include "../command.h"

#include <stdio.h>
#include <stdint.h>
#include <io.h>
#include <sys/alt_cache.h>
#include <malloc.h>
#include <priv/alt_file.h>
#include <altera_msgdma.h>
#include <altera_msgdma_descriptor_regs.h>
#include <altera_msgdma_csr_regs.h>
#include "../common/mjpeg423_types.h"

#define NUM_BUFFERS 10

#define DESC_CONTROL ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK
#define DCT_BLOCK_SIZE (32)
#define COLOUR_BLOCK_SIZE (16)

volatile int mm_to_st_complete = 0;
volatile int st_to_mm_complete = 0;

//Stand-in DCAC type and color block
// uint32_t *p_dct_block_buffer = malloc(sizeof(uint32_t) * DCT_BLOCK_SIZE);
// uint32_t *p_colour_block_buffer = malloc(sizeof(uint32_t) * COLOUR_BLOCK_SIZE);

alt_msgdma_dev *mm_to_st_dma_dev, *st_to_mm_dma_dev;
alt_msgdma_standard_descriptor *mm_to_st_desc, *st_to_mm_desc;

bool init = FALSE;
ece423_video_display *disp = (void *)0;

static void mm_to_st_done (void *context)
{
	mm_to_st_complete = 1;
}

static void st_to_mm_done (void *context)
{
	st_to_mm_complete = 1;
}

//main decoder function
void mjpeg423_decode(const char* filename_in, FAT_HANDLE sd_fat_handle)
{
    //header and payload info
    uint32_t num_frames, w_size, h_size, num_iframes, payload_size;
    uint32_t Ysize, Cbsize, frame_size, frame_type;
    uint32_t file_header[5], frame_header[4];

    //file streams(changed to fat handle)
    FAT_FILE_HANDLE file_in;

    mm_to_st_desc = malloc(sizeof(alt_msgdma_standard_descriptor));//(alt_msgdma_standard_descriptor*) memalign(32,
        //3 * sizeof(alt_msgdma_standard_descriptor));
    st_to_mm_desc = malloc(sizeof(alt_msgdma_standard_descriptor));//(alt_msgdma_standard_descriptor*) memalign(32,
        //3 * sizeof(alt_msgdma_standard_descriptor));

    mm_to_st_dma_dev = alt_msgdma_open(TO_IDCT_HWACEL_CSR_NAME);
    st_to_mm_dma_dev = alt_msgdma_open(FROM_IDCT_HWACEL_CSR_NAME);

    if((file_in = Fat_FileOpen(sd_fat_handle, filename_in)) == NULL) error_and_exit("cannot open input file");
    // char* filename_out = malloc(strlen(filenamebase_out)+1);
    // strcpy(filename_out, filenamebase_out);

    //read header
    if(!Fat_FileRead(file_in, file_header, 5*sizeof(uint32_t))) error_and_exit("cannot read input file");
    num_frames      = file_header[0];
    w_size          = file_header[1];
    h_size          = file_header[2];
    num_iframes     = file_header[3];
    payload_size    = file_header[4];

    DEBUG_PRINT_ARG("Decoder start. Num frames #%u\n", num_frames)
    DEBUG_PRINT_ARG("Width %u\n", w_size)
    DEBUG_PRINT_ARG("Height %u\n", h_size)
    DEBUG_PRINT_ARG("Num i frames %u\n", num_iframes)

    int hCb_size = h_size/8;           //number of chrominance blocks
    int wCb_size = w_size/8;
    int hYb_size = h_size/8;           //number of luminance blocks. Same as chrominance in the sample app
    int wYb_size = w_size/8;

    //trailer structure
    iframe_trailer_t* trailer = malloc(sizeof(iframe_trailer_t)*num_frames);

    //main data structures. See lab manual for explanation
    rgb_pixel_t* rgbblock;
    if((rgbblock = malloc(w_size*h_size*sizeof(rgb_pixel_t)))==NULL) error_and_exit("cannot allocate rgbblock");
    color_block_t* Yblock;
    if((Yblock = malloc(hYb_size * wYb_size * 64))==NULL) error_and_exit("cannot allocate Yblock");
    color_block_t* Cbblock;
    if((Cbblock = malloc(hCb_size * wCb_size * 64))==NULL) error_and_exit("cannot allocate Cbblock");
    color_block_t* Crblock;
    if((Crblock = malloc(hCb_size * wCb_size * 64))==NULL) error_and_exit("cannot allocate Crblock");;
    dct_block_t* YDCAC;
    if((YDCAC = malloc(hYb_size * wYb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate YDCAC");
    dct_block_t* CbDCAC;
    if((CbDCAC = malloc(hCb_size * wCb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate CbDCAC");
    dct_block_t* CrDCAC;
    if((CrDCAC = malloc(hCb_size * wCb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate CrDCAC");
    //Ybitstream is assigned a size sufficient to hold all bistreams
    //the bitstream is then read from the file into Ybitstream
    //the remaining pointers simply point to the beginning of the Cb and Cr streams within Ybitstream
    uint8_t* Ybitstream;
    if((Ybitstream = malloc(hYb_size * wYb_size * 64 * sizeof(DCTELEM) + 2 * hCb_size * wCb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate bitstream");
    uint8_t* Cbbitstream;
    uint8_t* Crbitstream;

    //read trailer. Note: the trailer information is not used in the sample decoder app
    //set file to beginning of trailer
    if(!Fat_FileSeek(file_in,  SEEK_SET, 5 * sizeof(uint32_t) + payload_size)) error_and_exit("cannot seek into file");
    for(int count = 0; count < num_iframes; count++){
        if(!Fat_FileRead(file_in,&(trailer[count].frame_index), sizeof(uint32_t))) error_and_exit("cannot read input file");
        if(!Fat_FileRead(file_in,&(trailer[count].frame_position), sizeof(uint32_t))) error_and_exit("cannot read input file");
        DEBUG_PRINT_ARG("I frame index %u, ", trailer[count].frame_index)
        DEBUG_PRINT_ARG("position %u\n", trailer[count].frame_position)
    }
    //set it back to beginning of payload
    if(!Fat_FileSeek(file_in, SEEK_SET, 5 * sizeof(uint32_t))) error_and_exit("cannot seek into file");

// Callback Functions for DMA
    alt_msgdma_register_callback(
    	mm_to_st_dma_dev,
    	mm_to_st_done,
    	ALTERA_MSGDMA_CSR_GLOBAL_INTERRUPT_MASK,
    	0);

    alt_msgdma_register_callback(
    	st_to_mm_dma_dev,
    	st_to_mm_done,
    	ALTERA_MSGDMA_CSR_GLOBAL_INTERRUPT_MASK,
    	0);

    //encode and write frames
    for(int frame_index = 0; frame_index < num_frames; frame_index ++){
        DEBUG_PRINT_ARG("\nFrame #%u\n",frame_index)

        //read frame payload
        if(!Fat_FileRead(file_in, frame_header, 4*sizeof(uint32_t))) error_and_exit("cannot read input file");
        frame_size  = frame_header[0];
        frame_type  = frame_header[1];
        Ysize       = frame_header[2];
        Cbsize      = frame_header[3];

        DEBUG_PRINT_ARG("Frame_size %u\n",frame_size)
        DEBUG_PRINT_ARG("Frame_type %u\n",frame_type)

        if(!Fat_FileRead(file_in, Ybitstream, frame_size - 4 * sizeof(uint32_t)))
            error_and_exit("cannot read input file");
        //set the Cb and Cr bitstreams to point to the right location
        Cbbitstream = Ybitstream + Ysize;
        Crbitstream = Cbbitstream + Cbsize;

#ifdef PROFILEING
        if(alt_timestamp_start() != 0) error_and_exit("Can't start timestamp");
#endif
        //lossless decoding
        lossless_decode(hYb_size*wYb_size, Ybitstream, YDCAC, Yquant, frame_type);
        lossless_decode(hCb_size*wCb_size, Cbbitstream, CbDCAC, Cquant, frame_type);
        lossless_decode(hCb_size*wCb_size, Crbitstream, CrDCAC, Cquant, frame_type);

        //idct
        for(int b = 0; b < hYb_size*wYb_size; b++)
        {
          alt_dcache_flush_all();
          //idct(YDCAC[b], Yblock[b]);
          while(0 != alt_msgdma_construct_standard_mm_to_st_descriptor(mm_to_st_dma_dev,
          mm_to_st_desc,
          (alt_u32 *) YDCAC[b] , sizeof(uint32_t) * DCT_BLOCK_SIZE,
          DESC_CONTROL));

          while (alt_msgdma_standard_descriptor_async_transfer(mm_to_st_dma_dev,
                 mm_to_st_desc)
                 != 0);

          while(!mm_to_st_complete);
          mm_to_st_complete = 0;

          while(0 != alt_msgdma_construct_standard_st_to_mm_descriptor(st_to_mm_dma_dev,
          st_to_mm_desc,
          (alt_u32 *) Yblock[b], sizeof(uint32_t) * COLOUR_BLOCK_SIZE,
          DESC_CONTROL));

          while (alt_msgdma_standard_descriptor_async_transfer(st_to_mm_dma_dev,
                 st_to_mm_desc)
                 != 0);

          while(!st_to_mm_complete);
          st_to_mm_complete = 0;
        }

        for(int b = 0; b < hCb_size*wCb_size; b++)
        {
          alt_dcache_flush_all();
          // idct(CbDCAC[b], Cbblock[b]);
          while(0 != alt_msgdma_construct_standard_mm_to_st_descriptor(mm_to_st_dma_dev,
          mm_to_st_desc,
          (alt_u32 *) CbDCAC[b] , sizeof(uint32_t) * DCT_BLOCK_SIZE,
          DESC_CONTROL));

          while(0 != alt_msgdma_construct_standard_st_to_mm_descriptor(st_to_mm_dma_dev,
          st_to_mm_desc,
          (alt_u32 *) Cbblock[b], sizeof(uint32_t) * COLOUR_BLOCK_SIZE,
          DESC_CONTROL));

					while (alt_msgdma_standard_descriptor_async_transfer(mm_to_st_dma_dev,
                 mm_to_st_desc)
                 != 0);

          while(!mm_to_st_complete);
          mm_to_st_complete = 0;

          while (alt_msgdma_standard_descriptor_async_transfer(st_to_mm_dma_dev,
                 st_to_mm_desc)
                 != 0);

          while(!st_to_mm_complete);
          st_to_mm_complete = 0;
        }

        for(int b = 0; b < hCb_size*wCb_size; b++)
        {
          alt_dcache_flush_all();
          // idct(CrDCAC[b], Crblock[b]);
          while(0 != alt_msgdma_construct_standard_mm_to_st_descriptor(mm_to_st_dma_dev,
          mm_to_st_desc,
          (alt_u32 *) CrDCAC[b] , sizeof(uint32_t) * DCT_BLOCK_SIZE,
          DESC_CONTROL));

          while(0 != alt_msgdma_construct_standard_st_to_mm_descriptor(st_to_mm_dma_dev,
          st_to_mm_desc,
          (alt_u32 *) Crblock[b], sizeof(uint32_t) * COLOUR_BLOCK_SIZE,
          DESC_CONTROL));

					while (alt_msgdma_standard_descriptor_async_transfer(mm_to_st_dma_dev,
                 mm_to_st_desc)
                 != 0);

          while(!mm_to_st_complete);
          mm_to_st_complete = 0;

          while (alt_msgdma_standard_descriptor_async_transfer(st_to_mm_dma_dev,
                 st_to_mm_desc)
                 != 0);

          while(!st_to_mm_complete);
          st_to_mm_complete = 0;
        }

#ifdef PROFILEING
        printf("it took %u cycles at a frequency of %uHz to decode this frame\n", alt_timestamp(), alt_timestamp_freq());
#endif

        if(!init)
        {
            disp = ece423_video_display_init(VIDEO_DMA_CSR_NAME,640,480,NUM_BUFFERS);
            init = TRUE;
        }

        alt_u32 *frame1 = ece423_video_display_get_buffer(disp);

        //ybcbr to rgb conversion
        while(ece423_video_display_buffer_is_available(disp) || (ready_frames > NUM_BUFFERS-5));

        for (int h = 0; h < hCb_size; h++)
            for (int w = 0; w < wCb_size; w++) {
                int b = h * wCb_size + w;

                ycbcr_to_rgb(h << 3, w << 3, w_size, Yblock[b], Cbblock[b], Crblock[b], (void *)frame1);
            }

        ece423_video_display_register_written_buffer(disp);
        ready_frames++;
// Displays the first frame of a new section when the video is paused
#ifdef PERIODIC_VIDEO
        if(!is_video_playing)
#endif
        {
        	ready_frames--;
            ece423_video_display_switch_frames(disp);
        }
        do
        {
			if(command != NO_CMD)
			{
				int32_t desired_skip_frame;
					switch(command)
					{
						case START:
							is_video_playing = TRUE;
							break;
						case PAUSE:
							is_video_playing = FALSE;
							break;
						case CYCLE:
							goto Exit;
							break;
						case REWIND:
							desired_skip_frame = frame_index - 120;
							goto SkipIt;
							/*for(int count = num_iframes; count >= 0; count--)
							{
							    if(trailer[count].frame_index < frame_index || count == 0)
							    {
							    	frame_index = trailer[count].frame_index;
							        if(!Fat_FileSeek(file_in, SEEK_SET, trailer[count].frame_position)) error_and_exit("cannot seek into file");
							        break;
							    }
							}
							command = NO_CMD;
							goto EndSkip;*/
							break;
						case FORWARD:
							desired_skip_frame = frame_index + 120;
							/*for(int count = 0; count < num_iframes; count++)
							{
							    if(trailer[count].frame_index > frame_index || count == (num_iframes - 1))
							    {
							    	frame_index = trailer[count].frame_index;
							        if(!Fat_FileSeek(file_in, SEEK_SET, trailer[count].frame_position)) error_and_exit("cannot seek into file");
							        break;
							    }
							}*/
							SkipIt:
							if(desired_skip_frame <= 0)
							{
								frame_index = trailer[0].frame_index;
								if(!Fat_FileSeek(file_in, SEEK_SET, trailer[0].frame_position)) error_and_exit("cannot seek into file");
								ready_frames = 0;
								disp->buffer_being_written = ((disp->buffer_being_displayed + 1)
										% disp->num_frame_buffers);

								command = NO_CMD;
								goto EndSkip;
							}
							else if(desired_skip_frame >= num_frames - 1)
							{
								//desired_skip_frame = num_frames - 1;
								disp->buffer_being_written = ((disp->buffer_being_displayed + 1)
										% disp->num_frame_buffers);
								ready_frames = 0;
								command = NO_CMD;
								goto Exit;
							}

							int japan_high = num_iframes - 1;
							int japan_low = 0;
							int middle = 0;
							while(japan_low <= japan_high)
							{
                                int middle = (japan_high + japan_low)/2;
                                if(trailer[middle].frame_index == desired_skip_frame)
                                {
                                	break;
                                }
                                else if(trailer[middle].frame_index < desired_skip_frame)
                                {
                                    japan_low = middle + 1;
                                }
                                else
                                {
                                	japan_high = middle - 1;
                                }
							}
							frame_index = trailer[japan_high].frame_index;
							if(!Fat_FileSeek(file_in, SEEK_SET, trailer[japan_high].frame_position)) error_and_exit("cannot seek into file");
							ready_frames = 0;
							disp->buffer_being_written = ((disp->buffer_being_displayed + 1)
									% disp->num_frame_buffers);

							command = NO_CMD;
							goto EndSkip;
							break;
						default:
							error_and_exit("Invalid State");
							break;
					}
					command = NO_CMD;

			}
		} while(!is_video_playing);

        EndSkip: ;

    } //end frame iteration
    DEBUG_PRINT("\nDecoder done.\n\n\n")

Exit:
    //close down
    Fat_FileClose(file_in);
    free(rgbblock);
    free(Yblock);
    free(Cbblock);
    free(Crblock);
    free(YDCAC);
    free(CbDCAC);
    free(CrDCAC);
    free(Ybitstream);
    free(trailer);
}