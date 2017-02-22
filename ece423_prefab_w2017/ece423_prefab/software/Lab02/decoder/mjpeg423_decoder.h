//
//  mjpeg423_decoder.h
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/24/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef mjpeg423app_mjpeg423_decoder_h
#define mjpeg423app_mjpeg423_decoder_h

#define PERIODIC_VIDEO

#include "../common/mjpeg423_types.h"
#include "../ece423_sd/sd_lib.h"
#include "../ece423_vid_ctl/ece423_vid_ctl.h"

void mjpeg423_decode(const char* filename_in, FAT_HANDLE sd_fat_handle);
void ycbcr_to_rgb(int h, int w, uint32_t w_size, pcolor_block_t Y, pcolor_block_t Cb, pcolor_block_t Cr, rgb_pixel_t* rgbblock);
void idct(dct_block_t DCAC, color_block_t block);
void lossless_decode(int num_blocks, void* bitstream, dct_block_t* DCACq, dct_block_t quant, bool P);

extern ece423_video_display *disp;

extern volatile uint32_t ready_frames;

#endif
