/*
 * alt_sys_init.c - HAL initialization source
 *
 * Machine generated for CPU 'cpu' in SOPC Builder design 'ECE423_QSYS'
 * SOPC Builder design path: ../../ECE423_QSYS.sopcinfo
 *
 * Generated: Wed Feb 22 14:14:02 EST 2017
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#include "system.h"
#include "sys/alt_irq.h"
#include "sys/alt_sys_init.h"

#include <stddef.h>

/*
 * Device headers
 */

#include "altera_nios2_gen2_irq.h"
#include "altera_avalon_jtag_uart.h"
#include "altera_avalon_sysid_qsys.h"
#include "altera_avalon_timer.h"
#include "altera_msgdma.h"

/*
 * Allocate the device storage
 */

ALTERA_NIOS2_GEN2_IRQ_INSTANCE ( CPU, cpu);
ALTERA_AVALON_JTAG_UART_INSTANCE ( JTAG_UART, jtag_uart);
ALTERA_AVALON_SYSID_QSYS_INSTANCE ( SYSID, sysid);
ALTERA_AVALON_TIMER_INSTANCE ( TIMER_0, timer_0);
ALTERA_AVALON_TIMER_INSTANCE ( TIMER_1, timer_1);
ALTERA_MSGDMA_CSR_DESCRIPTOR_SLAVE_INSTANCE ( CB_OUT, CB_OUT_CSR, CB_OUT_DESCRIPTOR_SLAVE, Cb_out);
ALTERA_MSGDMA_CSR_DESCRIPTOR_SLAVE_INSTANCE ( CR_OUT, CR_OUT_CSR, CR_OUT_DESCRIPTOR_SLAVE, Cr_out);
ALTERA_MSGDMA_CSR_DESCRIPTOR_SLAVE_INSTANCE ( FROM_IDCT_HWACEL, FROM_IDCT_HWACEL_CSR, FROM_IDCT_HWACEL_DESCRIPTOR_SLAVE, from_idct_hwacel);
ALTERA_MSGDMA_CSR_DESCRIPTOR_SLAVE_INSTANCE ( MSGDMA_0, MSGDMA_0_CSR, MSGDMA_0_DESCRIPTOR_SLAVE, msgdma_0);
ALTERA_MSGDMA_CSR_DESCRIPTOR_SLAVE_INSTANCE ( TO_IDCT_HWACEL, TO_IDCT_HWACEL_CSR, TO_IDCT_HWACEL_DESCRIPTOR_SLAVE, to_idct_hwacel);
ALTERA_MSGDMA_CSR_DESCRIPTOR_SLAVE_INSTANCE ( VIDEO_DMA, VIDEO_DMA_CSR, VIDEO_DMA_DESCRIPTOR_SLAVE, video_dma);
ALTERA_MSGDMA_CSR_DESCRIPTOR_SLAVE_INSTANCE ( Y_OUT, Y_OUT_CSR, Y_OUT_DESCRIPTOR_SLAVE, Y_out);

/*
 * Initialize the interrupt controller devices
 * and then enable interrupts in the CPU.
 * Called before alt_sys_init().
 * The "base" parameter is ignored and only
 * present for backwards-compatibility.
 */

void alt_irq_init ( const void* base )
{
    ALTERA_NIOS2_GEN2_IRQ_INIT ( CPU, cpu);
    alt_irq_cpu_enable_interrupts();
}

/*
 * Initialize the non-interrupt controller devices.
 * Called after alt_irq_init().
 */

void alt_sys_init( void )
{
    ALTERA_AVALON_TIMER_INIT ( TIMER_0, timer_0);
    ALTERA_AVALON_TIMER_INIT ( TIMER_1, timer_1);
    ALTERA_AVALON_JTAG_UART_INIT ( JTAG_UART, jtag_uart);
    ALTERA_AVALON_SYSID_QSYS_INIT ( SYSID, sysid);
    ALTERA_MSGDMA_INIT ( CB_OUT, Cb_out);
    ALTERA_MSGDMA_INIT ( CR_OUT, Cr_out);
    ALTERA_MSGDMA_INIT ( FROM_IDCT_HWACEL, from_idct_hwacel);
    ALTERA_MSGDMA_INIT ( MSGDMA_0, msgdma_0);
    ALTERA_MSGDMA_INIT ( TO_IDCT_HWACEL, to_idct_hwacel);
    ALTERA_MSGDMA_INIT ( VIDEO_DMA, video_dma);
    ALTERA_MSGDMA_INIT ( Y_OUT, Y_out);
}
