/*
@file    EVE_target.h
@brief   target specific includes, definitions and functions
@version 5.0
@date    2021-04-03
@author  Rudolph Riedel

@section LICENSE

MIT License

Copyright (c) 2016-2021 Rudolph Riedel, Modified by Linar Ismagilov

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

@section History

*/

#ifndef EVE_TARGET_H_
#define EVE_TARGET_H_

/* While the following lines make things a lot easier like automatically compiling the code for the target you are compiling for, */
/* a few things are expected to be taken care of beforehand. */
/* - setting the Chip-Select and Power-Down pins to Output, Chip-Select = 1 and Power-Down = 0 */
/* - setting up the SPI which may or not include things like
       - setting the pins for the SPI to output or some alternate I/O function or mapping that functionality to that pin
	   - if that is an option with the controller your are using you probably should set the drive-strength for the SPI pins to high
	   - setting the SS pin on AVRs to output in case it is not used for Chip-Select or Power-Down
	   - setting SPI to mode 0
	   - setting SPI to 8 bit with MSB first
	   - setting SPI clock to no more than 11 MHz for the init - if the display-module works as high

  For the SPI transfers single 8-Bit transfers are used with busy-wait for completion.
  While this is okay for AVRs that run at 16MHz with the SPI at 8 MHz and therefore do one transfer in 16 clock-cycles,
  this is wasteful for any 32 bit controller even at higher SPI speeds.

  If the define "EVE_DMA" is set the spi_transmit_async() is changed at compile time to write in a buffer instead directly to SPI.
  EVE_init() calls EVE_init_dma() which sets up the DMA channel and enables an IRQ for end of DMA.
  EVE_start_cmd_burst() resets the DMA buffer instead of transferring the first bytes by SPI.
  EVE_end_cmd_burst() just calls EVE_start_dma_transfer() which triggers the transfer of the SPI buffer by DMA.
  EVE_cmd_start() just instantly returns if there is an active DMA transfer.
  EVE_busy() does nothing but to report that EVE is busy if there is an active DMA transfer.
  At the end of the DMA transfer an IRQ is executed which clears the DMA active state, calls EVE_cs_clear() and EVE_cmd_start().

*/

#pragma once

#include <Arduino.h>
#include <stdio.h>
#include <SPI.h>

#define EVE_CS 		10
#define EVE_PDN		15
#define EVE_DMA
#define DELAY_MS(ms) delay(ms)

namespace EVE
{
	class Port
	{
	public:
		void dma_done();

		Port();

		EventResponder spi_event;
		uint32_t buffer[1025];
		volatile uint16_t dma_buffer_index;
		volatile uint8_t dma_busy;

		void dma_transfer(void);

		void cs_set(void)
		{
			digitalWriteFast(EVE_CS, LOW); /* make EVE listen */
		}

		void cs_clear(void)
		{
			digitalWriteFast(EVE_CS, HIGH); /* tell EVE to stop listen */
		}

		void transmit(uint8_t data)
		{
			SPI.transfer(data);
		}

		void transmit(uint8_t * data, uint len)
		{
			SPI.transfer(data, len);
		}

		void transmit_32(uint32_t data)
		{
			transmit((uint8_t)(data));
			transmit((uint8_t)(data >> 8));
			transmit((uint8_t)(data >> 16));
			transmit((uint8_t)(data >> 24));
		}

		/* spi_transmit_burst() is only used for cmd-FIFO commands so it *always* has to transfer 4 bytes */
		inline void transmit_burst(uint32_t data)
		{
			buffer[dma_buffer_index++] = data;
		}

		uint8_t receive(uint8_t data)
		{
			return SPI.transfer(data);
		}

		void pdn_set(void)
		{
			digitalWriteFast(EVE_PDN, LOW); /* go into power-down */
		}

		void pdn_clear(void)
		{
			digitalWriteFast(EVE_PDN, HIGH); /* power up */
		}
	};
};


#endif /* EVE_TARGET_H_ */