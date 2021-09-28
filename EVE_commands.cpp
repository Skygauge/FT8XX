#include "EVE_commands.h"

#if EVE_GEN > 2
#include <stdarg.h>
#endif

/* EVE Memory Commands - used with memWritexx and memReadxx */
#define MEM_WRITE	0x80 /* EVE Host Memory Write */
#define MEM_READ	0x00 /* EVE Host Memory Read */

namespace EVE
{
	/*----------------------------------------------------------------------------------------------------------------------------*/
	/*---- helper functions ------------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------------------------------------*/

	void Display::cmdWrite(uint8_t command, uint8_t parameter)
	{
		port.cs_set();
		port.transmit(command);
		port.transmit(parameter);
		port.transmit(0x00);
		port.cs_clear();
	}


	uint8_t Display::memRead8(uint32_t ftAddress)
	{
		uint8_t ftData8;
		port.cs_set();
		port.transmit_32(0x00000000 + ((uint8_t)(ftAddress >> 16) | MEM_READ) + (ftAddress & 0x0000ff00) + ( (ftAddress & 0x000000ff) << 16) );
		ftData8 = port.receive(0x00); /* read data byte by sending another dummy byte */
		port.cs_clear();
		return ftData8;	/* return byte read */
	}


	uint16_t Display::memRead16(uint32_t ftAddress)
	{
		uint16_t ftData16 = 0;
		port.cs_set();
		port.transmit_32(0x00000000 + ((uint8_t)(ftAddress >> 16) | MEM_READ) + (ftAddress & 0x0000ff00) + ( (ftAddress & 0x000000ff) << 16) );
		ftData16 = (port.receive(0x00));	/* read low byte */
		ftData16 = (port.receive(0x00) << 8) | ftData16;	/* read high byte */
		port.cs_clear();
		return ftData16; /* return integer read */
	}


	uint32_t Display::memRead32(uint32_t ftAddress)
	{
		uint32_t ftData32= 0;
		port.cs_set();
		port.transmit_32(0x00000000 + ((uint8_t)(ftAddress >> 16) | MEM_READ) + (ftAddress & 0x0000ff00) + ( (ftAddress & 0x000000ff) << 16) );
		ftData32 = ((uint32_t)port.receive(0x00)); /* read low byte */
		ftData32 = ((uint32_t)port.receive(0x00) << 8) | ftData32;
		ftData32 = ((uint32_t)port.receive(0x00) << 16) | ftData32;
		ftData32 = ((uint32_t)port.receive(0x00) << 24) | ftData32; /* read high byte */
		port.cs_clear();
		return ftData32; /* return long read */
	}


	void Display::memWrite8(uint32_t ftAddress, uint8_t ftData8)
	{
		port.cs_set();
		port.transmit((uint8_t)(ftAddress >> 16) | MEM_WRITE);
		port.transmit((uint8_t)(ftAddress >> 8));
		port.transmit((uint8_t)(ftAddress));
		port.transmit(ftData8);
		port.cs_clear();
	}


	void Display::memWrite16(uint32_t ftAddress, uint16_t ftData16)
	{
		port.cs_set();
		port.transmit((uint8_t)(ftAddress >> 16) | MEM_WRITE); /* send Memory Write plus high address byte */
		port.transmit((uint8_t)(ftAddress >> 8)); /* send middle address byte */
		port.transmit((uint8_t)(ftAddress)); /* send low address byte */
		port.transmit((uint8_t)(ftData16)); /* send data low byte */
		port.transmit((uint8_t)(ftData16 >> 8));  /* send data high byte */
		port.cs_clear();
	}


	void Display::memWrite32(uint32_t ftAddress, uint32_t ftData32)
	{
		port.cs_set();
		port.transmit((uint8_t)(ftAddress >> 16) | MEM_WRITE); /* send Memory Write plus high address byte */
		port.transmit((uint8_t)(ftAddress >> 8)); /* send middle address byte */
		port.transmit((uint8_t)(ftAddress)); /* send low address byte */
		port.transmit((uint8_t)(ftData32)); /* send data low byte */
		port.transmit((uint8_t)(ftData32 >> 8));
		port.transmit((uint8_t)(ftData32 >> 16));
		port.transmit((uint8_t)(ftData32 >> 24)); /* send data high byte */
		port.cs_clear();
	}



	/* helper function, write a block of memory from the FLASH of the host controller to EVE */
	void Display::memWrite_flash_buffer(uint32_t ftAddress, const uint8_t *data, uint32_t len)
	{
		port.cs_set();
		port.transmit((uint8_t)(ftAddress >> 16) | MEM_WRITE);
		port.transmit((uint8_t)(ftAddress >> 8));
		port.transmit((uint8_t)(ftAddress));

		len = (len + 3)&(~3);

		port.transmit(data, len);

		port.cs_clear();
	}


	/* helper function, write a block of memory from the SRAM of the host controller to EVE */
	void Display::memWrite_sram_buffer(uint32_t ftAddress, const uint8_t *data, uint32_t len)
	{
		uint32_t count;

		port.cs_set();
		port.transmit((uint8_t)(ftAddress >> 16) | MEM_WRITE);
		port.transmit((uint8_t)(ftAddress >> 8));
		port.transmit((uint8_t)(ftAddress));

		len = (len + 3)&(~3);

		for(count=0;count<len;count++)
		{
			port.transmit(data[count]);
		}

		port.cs_clear();
	}


	/* Check if the graphics processor completed executing the current command list. */
	/* REG_CMDB_SPACE == 0xffc -> command fifo is empty */
	/* (REG_CMDB_SPACE & 0x03) != 0 -> we have a co-processor fault */
	uint8_t Display::busy(void)
	{
		uint16_t space;

		#if defined (EVE_DMA)
		if(port.dma_busy)
		{
			return 1;
		}
		#endif

		space = memRead16(REG_CMDB_SPACE);

		if((space & 0x3) != 0) /* we have a co-processor fault, make EVE play with us again */
		{
			Serial.println("!!! FAULT !!!");
			#if EVE_GEN > 2
			uint16_t copro_patch_pointer;
			uint32_t ftAddress;

			copro_patch_pointer = memRead16(REG_COPRO_PATCH_DTR);
			#endif

			memWrite8(REG_CPURESET, 1);   /* hold co-processor engine in the reset condition */
			memWrite16(REG_CMD_READ, 0);  /* set REG_CMD_READ to 0 */
			memWrite16(REG_CMD_WRITE, 0); /* set REG_CMD_WRITE to 0 */
			memWrite32(REG_CMD_DL, 0);    /* reset REG_CMD_DL to 0 as required by the BT81x programming guide, should not hurt FT8xx */
			memWrite8(REG_CPURESET, 0);  /* set REG_CMD_WRITE to 0 to restart the co-processor engine*/

			#if EVE_GEN > 2

			memWrite16(REG_COPRO_PATCH_DTR, copro_patch_pointer);
			DELAY_MS(5); /* just to be safe */
			ftAddress = REG_CMDB_WRITE;

			port.cs_set();
			port.transmit((uint8_t)(ftAddress >> 16) | MEM_WRITE); /* send Memory Write plus high address byte */
			port.transmit((uint8_t)(ftAddress >> 8)); /* send middle address byte */
			port.transmit((uint8_t)(ftAddress)); /* send low address byte */

			port.transmit_32(CMD_FLASHATTACH);
			port.transmit_32(CMD_FLASHFAST);
			port.cs_clear();

			memWrite8(REG_PCLK, EVE_PCLK); /* restore REG_PCLK in case it was set to zero by an error */
			DELAY_MS(5); /* just to be safe */

			#endif
		}

		if(space != 0xffc)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}


	/* deprecated, with using REG_CMDB_WRITE commands will be automatically excecuted on the rising edge of chip select */
	/* order the command co-processor to start processing its FIFO queue and do not wait for completion */
	void Display::cmd_start(void)
	{
	#if defined (EVE_DMA)
		if(port.dma_busy)
		{
			return; /* just do nothing if a dma transfer is in progress */
		}
	#endif
	}


	/* wait for the co-processor to complete the FIFO queue */
	void Display::cmd_execute(void)
	{
		while (busy());
	}


	/* begin a co-processor command, this is used for non-display-list and non-burst-mode commands */
	void Display::begin_cmd(uint32_t command)
	{
		uint32_t ftAddress;

		ftAddress = REG_CMDB_WRITE;
		port.cs_set();
		port.transmit((uint8_t)(ftAddress >> 16) | MEM_WRITE); /* send Memory Write plus high address byte */
		port.transmit((uint8_t)(ftAddress >> 8)); /* send middle address byte */
		port.transmit((uint8_t)(ftAddress)); /* send low address byte */
		port.transmit_32(command);
	}


	void Display::private_block_write(const uint8_t *data, uint16_t len)
	{
		uint8_t padding = len & 0x03; /* 0, 1, 2 or 3 */
		padding = 4-padding; /* 4, 3, 2 or 1 */
		padding &= 3; /* 3, 2 or 1 */

		port.transmit(data, len);
		
		while(padding > 0)
		{
			port.transmit(0);
			padding--;
		}
	}


	void Display::block_transfer(const uint8_t *data, uint32_t len)
	{
		uint32_t bytes_left;

		bytes_left = len;
		while(bytes_left > 0)
		{
			uint32_t block_len;
			uint32_t ftAddress;

			block_len = bytes_left>3840 ? 3840:bytes_left;

			ftAddress = REG_CMDB_WRITE;
			port.cs_set();
			port.transmit((uint8_t)(ftAddress >> 16) | MEM_WRITE); /* send Memory Write plus high address byte */
			port.transmit((uint8_t)(ftAddress >> 8)); /* send middle address byte */
			port.transmit((uint8_t)(ftAddress)); /* send low address byte */
			private_block_write(data,block_len);
			port.cs_clear();
			data += block_len;
			bytes_left -= block_len;
			while (busy());
		}
	}

	/*----------------------------------------------------------------------------------------------------------------------------*/
	/*---- co-processor commands that are not used in displays lists, these are not to be used with burst transfers --------------*/
	/*----------------------------------------------------------------------------------------------------------------------------*/


	/* BT817 / BT818 */
	#if EVE_GEN > 3

	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* write "num" bytes from src in RAM_G to to the external flash on a BT81x board at address dest */
	/* note: dest must be 4096-byte aligned, src must be 4-byte aligned, num must be a multiple of 4096 */
	/* note: EVE will not do anything if the alignment requirements are not met */
	/* note: the address ptr is relative to the flash so the first address is 0x00000000 not 0x800000 */
	/* note: this looks exactly the same as cmd_flashupdate() but it needs the flash to be empty */
	void Display::cmd_flashprogram(uint32_t dest, uint32_t src, uint32_t num)
	{
		begin_cmd(CMD_FLASHPROGRAM);
		port.transmit_32(dest);
		port.transmit_32(src);
		port.transmit_32(num);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_fontcache(uint32_t font, int32_t ptr, uint32_t num)
	{
		begin_cmd(CMD_FONTCACHE);
		port.transmit_32(font);
		port.transmit_32(ptr);
		port.transmit_32(num);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_fontcachequery(uint32_t *total, int32_t *used)
	{
		uint16_t cmdoffset;

		begin_cmd(CMD_FONTCACHEQUERY);
		port.transmit_32(0);
		port.transmit_32(0);
		port.cs_clear();
		while (busy());
		cmdoffset = memRead16(REG_CMD_WRITE);  /* read the graphics processor write pointer */

		if(total)
		{
			*total = memRead32(EVE_RAM_CMD + ((cmdoffset - 8) & 0xfff));
		}
		if(used)
		{
			*used = memRead32(EVE_RAM_CMD + ((cmdoffset - 4) & 0xfff));
		}
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_getimage(uint32_t *source, uint32_t *fmt, uint32_t *width, uint32_t *height, uint32_t *palette)
	{
		uint16_t cmdoffset;

		begin_cmd(CMD_GETIMAGE);
		port.transmit_32(0);
		port.transmit_32(0);
		port.transmit_32(0);
		port.transmit_32(0);
		port.transmit_32(0);
		port.cs_clear();
		while (busy());
		cmdoffset = memRead16(REG_CMD_WRITE);  /* read the graphics processor write pointer */

		if(palette)
		{
			*palette = memRead32(EVE_RAM_CMD + ((cmdoffset - 4) & 0xfff));
		}
		if(height)
		{
			*height = memRead32(EVE_RAM_CMD + ((cmdoffset - 8) & 0xfff));
		}
		if(width)
		{
			*width = memRead32(EVE_RAM_CMD + ((cmdoffset - 12) & 0xfff));
		}
		if(fmt)
		{
			*fmt = memRead32(EVE_RAM_CMD + ((cmdoffset - 16) & 0xfff));
		}
		if(source)
		{
			*source = memRead32(EVE_RAM_CMD + ((cmdoffset - 20) & 0xfff));
		}
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_linetime(uint32_t dest)
	{
		begin_cmd(CMD_LINETIME);
		port.transmit_32(dest);
		port.cs_clear();
		while (busy());
	}

	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_newlist(uint32_t adr)
	{
		begin_cmd(CMD_NEWLIST);
		port.transmit_32(adr);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* This command sets REG_PCLK_FREQ to generate the closest possible frequency to the one requested. */
	/* Returns the frequency achieved or zero if no frequency was found. */
	uint32_t Display::cmd_pclkfreq(uint32_t ftarget, int32_t rounding)
	{
		uint16_t cmdoffset;

		begin_cmd(CMD_PCLKFREQ);
		port.transmit_32(ftarget);
		port.transmit_32(rounding);
		port.transmit_32(0);
		port.cs_clear();
		while (busy());
		cmdoffset = memRead16(REG_CMD_WRITE);  /* read the graphics processor write pointer */
		cmdoffset -= 4;
		cmdoffset &= 0x0fff;
		return (memRead32(EVE_RAM_CMD + cmdoffset));
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_wait(uint32_t us)
	{
		begin_cmd(CMD_WAIT);
		port.transmit_32(us);
		port.cs_clear();
		while (busy());
	}


	#endif /* EVE_GEN > 3 */


	/* BT815 / BT816 */
	#if EVE_GEN > 2

	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* this command clears the graphics systems flash cache and to do so it needs to be executed with empty display lists */
	/* note: looks like overkill to clear both display lists but this is taken from BRT sample code */
	void Display::cmd_clearcache(void)
	{
		cmd_dl(CMD_DLSTART);
		cmd_dl(CMD_SWAP);
		while (busy());

		cmd_dl(CMD_DLSTART);
		cmd_dl(CMD_SWAP);
		while (busy());

		cmd_dl(CMD_CLEARCACHE);
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* this is added for conveniance, using cmd_dl(CMD_FLASHATTACH); followed by cmd_execute(); would work as well */
	void Display::cmd_flashattach(void)
	{
		begin_cmd(CMD_FLASHATTACH);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* this is added for conveniance, using cmd_dl(CMD_FLASHDETACH); followed by cmd_execute(); would work as well */
	void Display::cmd_flashdetach(void)
	{
		begin_cmd(CMD_FLASHDETACH);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* this is added for conveniance, using cmd_dl(CMD_FLASHERASE); followed by cmd_execute(); would work as well */
	void Display::cmd_flasherase(void)
	{
		begin_cmd(CMD_FLASHERASE);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	uint32_t Display::cmd_flashfast(void)
	{
		uint16_t cmdoffset;

		begin_cmd(CMD_FLASHFAST);
		port.transmit_32(0);
		port.cs_clear();

		while (busy());
		cmdoffset = memRead16(REG_CMD_WRITE);  /* read the graphics processor write pointer */
		cmdoffset -= 4;
		cmdoffset &= 0x0fff;
		return (memRead32(EVE_RAM_CMD + cmdoffset));
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* this is added for conveniance, using cmd_dl(CMD_FLASHSPIDESEL); followed by cmd_execute(); would work as well */
	void Display::cmd_flashspidesel(void)
	{
		begin_cmd(CMD_FLASHSPIDESEL);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* write "num" bytes from src in the external flash on a BT81x board to dest in RAM_G */
	/* note: src must be 64-byte aligned, dest must be 4-byte aligned, num must be a multiple of 4 */
	/* note: EVE will not do anything if the alignment requirements are not met */
	/* note: the src pointer is relative to the flash so the first address is 0x00000000 not 0x800000 */
	void Display::cmd_flashread(uint32_t dest, uint32_t src, uint32_t num)
	{
		begin_cmd(CMD_FLASHREAD);
		port.transmit_32(dest);
		port.transmit_32(src);
		port.transmit_32(num);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_flashsource(uint32_t ptr)
	{
		begin_cmd(CMD_FLASHSOURCE);
		port.transmit_32(ptr);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* write "num" bytes from the BT81x SPI interface dest in RAM_G */
	/* note: raw direct access, not really useful for anything */
	void Display::cmd_flashspirx(uint32_t dest, uint32_t num)
	{
		begin_cmd(CMD_FLASHSPIRX);
		port.transmit_32(dest);
		port.transmit_32(num);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* write "num" bytes from *data to the BT81x SPI interface */
	/* note: raw direct access, not really useful for anything */
	void Display::cmd_flashspitx(uint32_t num, const uint8_t *data)
	{
		begin_cmd(CMD_FLASHSPITX);
		port.transmit_32(num);
		port.cs_clear();
		block_transfer(data, num);
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* write "num" bytes from src in RAM_G to to the external flash on a BT81x board at address dest */
	/* note: dest must be 4096-byte aligned, src must be 4-byte aligned, num must be a multiple of 4096 */
	/* note: EVE will not do anything if the alignment requirements are not met */
	/* note: the address ptr is relative to the flash so the first address is 0x00000000 not 0x800000 */
	void Display::cmd_flashupdate(uint32_t dest, uint32_t src, uint32_t num)
	{
		begin_cmd(CMD_FLASHUPDATE);
		port.transmit_32(dest);
		port.transmit_32(src);
		port.transmit_32(num);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* write "num" bytes from *data to the external flash on a BT81x board at address ptr */
	/* note: ptr must be 256 byte aligned, num must be a multiple of 256 */
	/* note: EVE will not do anything if the alignment requirements are not met */
	/* note: the address ptr is relative to the flash so the first address is 0x00000000 not 0x800000 */
	/* note: on AVR controllers this expects the data to be located in the controllers flash memory */
	void Display::cmd_flashwrite(uint32_t ptr, uint32_t num, const uint8_t *data)
	{
		begin_cmd(CMD_FLASHWRITE);
		port.transmit_32(ptr);
		port.transmit_32(num);
		port.cs_clear();
		if(data)
		{
			block_transfer(data, num);
		}
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_inflate2(uint32_t ptr, uint32_t options, const uint8_t *data, uint32_t len)
	{
		begin_cmd(CMD_INFLATE2);
		port.transmit_32(ptr);
		port.transmit_32(options);
		port.cs_clear();

		if(options == 0) /* direct data, not by Media-FIFO or Flash */
		{
			if(data)
			{
				block_transfer(data, len);
			}
		}
	}

	#endif /* EVE_GEN > 2 */


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* get the properties of an image after a CMD_LOADIMAGE operation and write the values to the variables that are supplied by pointers
		uint32 pointer, width, height;
		EVE_LIB_GetProps(&pointer, &width, &height);

		uint32 width, height;
		EVE_LIB_GetProps(0, &width, &height);
	*/
	void Display::cmd_getprops(uint32_t *pointer, uint32_t *width, uint32_t *height)
	{
		uint16_t cmdoffset;

		begin_cmd(CMD_GETPROPS);
		port.transmit_32(0);
		port.transmit_32(0);
		port.transmit_32(0);
		port.cs_clear();
		while (busy());
		cmdoffset = memRead16(REG_CMD_WRITE);  /* read the graphics processor write pointer */

		if(pointer)
		{
			*pointer = memRead32(EVE_RAM_CMD + ((cmdoffset - 12) & 0xfff));
		}
		if(width)
		{
			*width = memRead32(EVE_RAM_CMD + ((cmdoffset - 8) & 0xfff));
		}
		if(height)
		{
			*height = memRead32(EVE_RAM_CMD + ((cmdoffset - 4) & 0xfff));
		}
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* address = cmd_getpr(); */
	uint32_t Display::cmd_getptr(void)
	{
		uint16_t cmdoffset;

		begin_cmd(CMD_GETPTR);
		port.transmit_32(0);

		port.cs_clear();
		while (busy());
		cmdoffset = memRead16(REG_CMD_WRITE);  /* read the graphics processor write pointer */
		cmdoffset -= 4;
		cmdoffset &= 0x0fff;
		return (memRead32(EVE_RAM_CMD + cmdoffset));
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_inflate(uint32_t ptr, const uint8_t *data, uint32_t len)
	{
		begin_cmd(CMD_INFLATE);
		port.transmit_32(ptr);
		port.cs_clear();
		if(len)
		{
			block_transfer(data, len);
		}
	}


	/* this is meant to be called outside display-list building, does not support cmd-burst */
	void Display::cmd_interrupt(uint32_t ms)
	{
		begin_cmd(CMD_INTERRUPT);
		port.transmit_32(ms);
		port.cs_clear();
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_loadimage(uint32_t ptr, uint32_t options, const uint8_t *data, uint32_t len)
	{
		begin_cmd(CMD_LOADIMAGE);
		port.transmit_32(ptr);
		port.transmit_32(options);
		port.cs_clear();

	#if EVE_GEN > 2
		if( ((options & EVE_OPT_MEDIAFIFO) == 0) && ((options & EVE_OPT_FLASH) == 0) )/* direct data, neither by Media-FIFO or from Flash */
	#else
		if((options & EVE_OPT_MEDIAFIFO) == 0) /* direct data, not by Media-FIFO */
	#endif
		{
			if(len)
			{
				block_transfer(data, len);
			}
		}
	}


	/* this is meant to be called outside display-list building, does not support cmd-burst */
	void Display::cmd_mediafifo(uint32_t ptr, uint32_t size)
	{
		begin_cmd(CMD_MEDIAFIFO);
		port.transmit_32(ptr);
		port.transmit_32(size);
		port.cs_clear();
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_memcpy(uint32_t dest, uint32_t src, uint32_t num)
	{
		begin_cmd(CMD_MEMCPY);
		port.transmit_32(dest);
		port.transmit_32(src);
		port.transmit_32(num);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* crc32 = cmd_memcrc(my_ptr_to_some_memory_region, some_amount_of_bytes); */
	uint32_t Display::cmd_memcrc(uint32_t ptr, uint32_t num)
	{
		uint16_t cmdoffset;

		begin_cmd(CMD_MEMCRC);
		port.transmit_32(ptr);
		port.transmit_32(num);
		port.transmit_32(0);
		port.cs_clear();
		while (busy());
		cmdoffset = memRead16(REG_CMD_WRITE);  /* read the graphics processor write pointer */
		cmdoffset -= 4;
		cmdoffset &= 0x0fff;
		return (memRead32(EVE_RAM_CMD + cmdoffset));
	}


	/* this is meant to be called outside display-list building, does not support cmd-burst */
	void Display::cmd_memset(uint32_t ptr, uint8_t value, uint32_t num)
	{
		begin_cmd(CMD_MEMSET);
		port.transmit_32(ptr);
		port.transmit_32((uint32_t) value);
		port.transmit_32(num);
		port.cs_clear();
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* this is a pointless command, just use one of the memWrite* helper functions instead to directly write to EVEs memory */
	
	void Display::cmd_memwrite(uint32_t dest, const uint8_t *data, uint32_t num)
	{
		begin_cmd(CMD_MEMWRITE);
		port.transmit_32(dest);
		port.transmit_32(num);

		private_block_write(data, num);

		port.cs_clear();
		while (busy());
	}
	
	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_memzero(uint32_t ptr, uint32_t num)
	{
		begin_cmd(CMD_MEMZERO);
		port.transmit_32(ptr);
		port.transmit_32(num);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command, does not support cmd-burst */
	/* it does not wait for completion in order to allow the video to be paused or terminated by REG_PLAY_CONTROL */
	void Display::cmd_playvideo(uint32_t options, const uint8_t *data, uint32_t len)
	{
		begin_cmd(CMD_PLAYVIDEO);
		port.transmit_32(options);
		port.cs_clear();

		#if EVE_GEN > 2
		if( ((options & EVE_OPT_MEDIAFIFO) == 0) && ((options & EVE_OPT_FLASH) == 0) )/* direct data, neither by Media-FIFO or from Flash */
		#else
		if((options & EVE_OPT_MEDIAFIFO) == 0) /* direct data, not by Media-FIFO */
		#endif
		{
			if(data)
			{
				block_transfer(data, len);
			}
		}
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	/* regvalue = cmd_regread(ptr); */
	/* this seems to be completely pointless, there is no real use for it outside a display-list since the register could be read directly */
	/* and for what purpose would this be implemented to be used in a display list?? */
	uint32_t Display::cmd_regread(uint32_t ptr)
	{
		uint16_t cmdoffset;

		begin_cmd(CMD_REGREAD);
		port.transmit_32(ptr);
		port.transmit_32(0);
		port.cs_clear();
		while (busy());
		cmdoffset = memRead16(REG_CMD_WRITE);  /* read the graphics processor write pointer */
		cmdoffset -= 4;
		cmdoffset &= 0x0fff;
		return (memRead32(EVE_RAM_CMD + cmdoffset));
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_setrotate(uint32_t r)
	{
		begin_cmd(CMD_SETROTATE);
		port.transmit_32(r);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_snapshot(uint32_t ptr)
	{
		begin_cmd(CMD_SNAPSHOT);
		port.transmit_32(ptr);
		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_snapshot2(uint32_t fmt, uint32_t ptr, int16_t x0, int16_t y0, int16_t w0, int16_t h0)
	{
		begin_cmd(CMD_SNAPSHOT2);
		port.transmit_32(fmt);
		port.transmit_32(ptr);

		port.transmit((uint8_t)(x0));
		port.transmit((uint8_t)(x0 >> 8));
		port.transmit((uint8_t)(y0));
		port.transmit((uint8_t)(y0 >> 8));

		port.transmit((uint8_t)(w0));
		port.transmit((uint8_t)(w0 >> 8));
		port.transmit((uint8_t)(h0));
		port.transmit((uint8_t)(h0 >> 8));

		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_track(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t tag)
	{
		begin_cmd(CMD_TRACK);

		port.transmit((uint8_t)(x0));
		port.transmit((uint8_t)(x0 >> 8));
		port.transmit((uint8_t)(y0));
		port.transmit((uint8_t)(y0 >> 8));

		port.transmit((uint8_t)(w0));
		port.transmit((uint8_t)(w0 >> 8));
		port.transmit((uint8_t)(h0));
		port.transmit((uint8_t)(h0 >> 8));

		port.transmit((uint8_t)(tag));
		port.transmit((uint8_t)(tag >> 8));
		port.transmit(0);
		port.transmit(0);

		port.cs_clear();
		while (busy());
	}


	/* this is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst */
	void Display::cmd_videoframe(uint32_t dest, uint32_t result_ptr)
	{
		begin_cmd(CMD_VIDEOFRAME);
		port.transmit_32(dest);
		port.transmit_32(result_ptr);
		port.cs_clear();
		while (busy());
	}


	/*----------------------------------------------------------------------------------------------------------------------------*/
	/*------------- patching and initialisation ----------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------------------------------------*/

	#if EVE_GEN > 2

	/* switch the FLASH attached to a BT815/BT816 to full-speed mode, returns 0 for failing to do so */
	uint8_t Display::init_flash(void)
	{
		uint8_t timeout = 0;
		uint8_t status;

		status = memRead8(REG_FLASH_STATUS); /* should be 0x02 - FLASH_STATUS_BASIC, power-up is done and the attached flash is detected */

		while(status == 0) /* FLASH_STATUS_INIT - we are somehow still in init, give it a litte more time, this should never happen */
		{
			status = memRead8(REG_FLASH_STATUS);
			DELAY_MS(1);
			timeout++;
			if(timeout > 100) /* 100ms and still in init, lets call quits now and exit with an error */
			{
				return 0;
			}
		}

		if(status == 1) /* FLASH_STATUS_DETACHED - no flash was found during init, no flash present or the detection failed, but have hope and let the BT81x have annother try */
		{
			cmd_dl(CMD_FLASHATTACH);
			while (busy());
			status = memRead8(REG_FLASH_STATUS);
			if(status != 2) /* still not in FLASH_STATUS_BASIC, time to give up */
			{
				return 0;
			}
		}

		if(status == 2) /* FLASH_STATUS_BASIC - flash detected and ready for action, lets move it up to FLASH_STATUS_FULL */
		{
			uint32_t result;

			result = cmd_flashfast();

			if(result == 0) /* cmd_flashfast was successful */
			{
				return 1;
			}
			else /* room for improvement, cmd_flashfast provided an error code but there is no way to return it without returning a value that is FALSE all the same */
			{
				return 0;
			}
		}

		if(status == 3) /* FLASH_STATUS_FULL - we are already there, why has this function been called? */
		{
			return 1;
		}

		return 0;
	}

	#endif /* EVE_GEN > 2 */


	/* FT811 / FT813 binary-blob from FTDIs AN_336 to patch the touch-engine for Goodix GT911 / GT9271 touch controllers */
	#if defined (EVE_HAS_GT911) && (EVE_GEN < 3)

	#if	defined (__AVR__)
	#include <avr/pgmspace.h>
	#else
	#define PROGMEM
	#endif

	const uint16_t Display::GT911_len = 1184;
	const uint8_t Display::GT911_data[1184] PROGMEM =
	{
		26,255,255,255,32,32,48,0,4,0,0,0,2,0,0,0,
		34,255,255,255,0,176,48,0,120,218,237,84,221,111,84,69,20,63,51,179,93,160,148,101,111,76,5,44,141,123,111,161,11,219,154,16,9,16,17,229,156,75,26,11,13,21,227,3,16,252,184,179,
		45,219,143,45,41,125,144,72,67,100,150,71,189,113,18,36,17,165,100,165,198,16,32,17,149,196,240,128,161,16,164,38,54,240,0,209,72,130,15,38,125,48,66,82,30,76,19,31,172,103,46,
		139,24,255,4,227,157,204,156,51,115,102,206,231,239,220,5,170,94,129,137,75,194,216,98,94,103,117,115,121,76,131,177,125,89,125,82,123,60,243,58,142,242,204,185,243,188,118,156,
		227,155,203,238,238,195,251,205,229,71,92,28,169,190,184,84,143,113,137,53,244,103,181,237,87,253,113,137,233,48,12,198,165,181,104,139,25,84,253,155,114,74,191,0,54,138,163,
		12,62,131,207,129,23,217,34,91,31,128,65,246,163,175,213,8,147,213,107,35,203,94,108,3,111,40,171,83,24,15,165,177,222,116,97,23,188,140,206,150,42,102,181,87,78,86,182,170,134,
		215,241,121,26,243,252,2,76,115,217,139,222,206,173,136,132,81,61,35,185,39,113,23,46,199,76,178,54,151,183,224,0,40,189,28,149,182,58,131,79,152,30,76,34,98,234,162,216,133,141,
		102,39,170,40,192,101,53,201,146,191,37,77,44,177,209,74,211,5,206,187,5,6,216,47,53,96,123,22,50,103,251,192,84,17,74,227,185,56,106,51,91,161,96,182,163,48,171,141,139,65,152,
		66,66,11,102,43,158,75,36,80,147,184,147,139,112,17,235,216,103,111,239,245,92,10,175,194,40,44,58,125,5,59,112,50,103,245,4,78,192,5,156,194,51,60,191,134,75,110,173,237,46,192,
		121,156,192,115,184,218,120,67,63,115,46,11,102,10,97,232,50,235,114,182,148,118,178,41,188,12,135,77,202,124,12,96,238,35,161,234,189,129,23,249,212,139,230,25,53,48,205,52,93,
		163,117,53,154,170,81,85,163,178,70,69,66,167,241,14,46,241,1,226,136,152,179,197,59,184,148,254,49,132,48,15,176,137,192,76,131,196,105,104,162,86,81,160,165,255,26,173,162,137,
		86,145,210,183,192,55,175,194,211,60,91,120,230,184,174,27,41,131,155,40,224,29,87,179,232,16,55,55,7,165,147,81,23,165,49,101,54,224,75,180,81,108,18,29,226,69,225,110,175,224,
		42,212,25,47,130,193,110,234,192,215,252,56,74,162,24,46,251,174,54,106,68,245,14,9,155,160,22,120,207,104,240,29,90,178,140,28,24,220,47,166,112,61,251,208,192,111,56,239,238,
		93,255,251,62,99,32,193,75,61,190,235,123,229,110,218,194,85,79,225,59,98,20,238,227,235,220,11,221,149,25,180,116,194,159,111,96,192,24,213,59,139,179,156,215,69,230,19,24,35,
		135,117,206,171,206,162,67,129,234,61,235,11,104,103,84,64,223,167,254,40,163,101,92,84,43,150,46,249,219,205,7,116,11,91,104,61,57,75,223,8,48,25,28,119,252,222,113,49,86,249,
		74,180,211,156,181,61,215,168,157,7,251,199,150,242,250,91,58,132,94,121,7,53,151,139,98,6,165,153,69,214,32,110,211,100,101,31,89,45,81,98,23,205,205,197,209,109,186,198,35,
		141,191,249,25,60,132,223,153,251,98,20,239,146,139,20,217,250,41,250,137,58,177,90,57,79,51,108,233,20,253,194,187,49,222,205,114,141,96,48,175,219,107,54,111,138,22,154,103,
		108,79,58,252,179,178,79,164,195,2,153,36,39,170,199,201,167,197,85,106,8,59,177,81,46,56,2,230,75,114,17,55,112,188,65,208,137,77,114,10,115,55,58,208,197,173,122,87,6,140,
		110,42,208,124,163,70,108,241,104,18,245,98,214,187,134,53,42,221,22,182,133,211,116,148,177,194,209,192,85,90,199,58,55,203,2,229,19,137,187,161,228,154,112,203,145,125,244,
		188,220,118,228,41,201,181,41,195,144,215,183,51,80,250,21,217,16,217,200,235,109,227,188,122,218,142,60,170,224,112,240,184,130,229,224,113,5,223,148,163,80,165,183,130,187,
		132,116,64,238,161,85,220,115,139,205,98,227,244,29,102,125,7,37,243,123,223,11,26,92,63,243,116,61,191,138,123,244,160,84,186,74,31,5,174,247,119,135,199,248,253,135,242,97,
		102,145,190,144,14,85,238,221,231,193,158,48,205,25,120,248,15,220,29,158,9,70,185,30,103,229,33,254,23,237,160,172,62,193,90,222,224,232,14,200,56,90,104,142,227,120,110,6,
		21,211,203,65,150,99,151,220,247,87,164,50,159,49,239,234,58,142,0,109,108,123,18,79,227,36,100,248,222,205,96,127,120,26,171,228,69,63,36,17,252,200,17,116,242,187,227,88,143,
		247,2,75,191,6,130,59,188,11,55,240,31,243,122,152,226,183,207,154,73,188,39,219,43,105,222,87,41,143,141,140,175,73,112,184,252,61,184,16,90,250,35,168,82,119,176,57,116,94,
		200,150,22,190,179,44,104,12,235,84,149,102,252,89,154,193,99,228,106,242,125,248,64,194,255,223,127,242,83,11,255,2,70,214,226,128,0,0
	};
	#endif


	/* init, has to be executed with the SPI setup to 11 MHz or less as required by FT8xx / BT8xx */
	uint8_t Display::init(void)
	{
        port.init();

		#if defined (EVE_HAS_CRYSTAL)
		cmdWrite(EVE_CLKEXT,0);	/* setup EVE for external clock */
		#else
		cmdWrite(EVE_CLKINT,0);	/* setup EVE for internal clock */
		#endif

		#if EVE_GEN > 2
		cmdWrite(EVE_CLKSEL,0x46); /* set clock to 72 MHz */
		#endif

		cmdWrite(EVE_ACTIVE,0);	/* start EVE */

		uint8_t chipid = memRead8(REG_ID);;
		uint16_t timeout = 0;

		while(chipid != 0x7C) /* if chipid is not 0x7c, continue to read it until it is, EVE needs a moment for it's power on self-test and configuration */
		{
			DELAY_MS(1);
			chipid = memRead8(REG_ID);
			timeout++;
			if(timeout > 400)
			{
                Serial.println("display init timeout on id!");
            	return 0;
			}
		}

		timeout = 0;
		while (0x00 != (memRead8(REG_CPURESET) & 0x03)) /* check if EVE is in working status */
		{
			DELAY_MS(1);
			timeout++;
			if(timeout > 50) /* experimental, 10 was the lowest value to get the BT815 started with, the touch-controller was the last to get out of reset */
			{
                Serial.println("display init timeout on reset!");
				return 0;
			}
		}

		/* tell EVE that we changed the frequency from default to 72MHz for BT8xx */
		#if EVE_GEN > 2
		memWrite32(REG_FREQUENCY, 72000000);
		#endif

		/* we have a display with a Goodix GT911 / GT9271 touch-controller on it, so we patch our FT811 or FT813 according to AN_336 or setup a BT815 accordingly */
		#if defined (EVE_HAS_GT911)

		#if EVE_GEN > 2
			memWrite16(REG_TOUCH_CONFIG, 0x05d0); /* switch to Goodix touch controller */
		#else
			uint32_t ftAddress;

			ftAddress = REG_CMDB_WRITE;

			port.cs_set();
			port.transmit((uint8_t)(ftAddress >> 16) | MEM_WRITE); /* send Memory Write plus high address byte */
			port.transmit((uint8_t)(ftAddress >> 8)); /* send middle address byte */
			port.transmit((uint8_t)(ftAddress)); /* send low address byte */
			private_block_write(EVE_GT911_data, EVE_GT911_len);
			port.cs_clear();
			while (busy());

			memWrite8(REG_TOUCH_OVERSAMPLE, 0x0f); /* setup oversample to 0x0f as "hidden" in binary-blob for AN_336 */
			memWrite16(REG_TOUCH_CONFIG, 0x05D0); /* write magic cookie as requested by AN_336 */

			/* specific to the EVE2 modules from Matrix-Orbital we have to use GPIO3 to reset GT911 */
			memWrite16(REG_GPIOX_DIR,0x8008); /* Reset-Value is 0x8000, adding 0x08 sets GPIO3 to output, default-value for REG_GPIOX is 0x8000 -> Low output on GPIO3 */
			DELAY_MS(1); /* wait more than 100�s */
			memWrite8(REG_CPURESET, 0x00); /* clear all resets */
			DELAY_MS(110); /* wait more than 55ms - does not work with multitouch, for some reason a minimum delay of 108ms is required */
		#endif
		#endif
		
        memWrite16(REG_GPIOX, 0x800C);
		memWrite16(REG_GPIOX_DIR,0x800C); /* setting GPIO3 back to input, GPIO2 to output */
		/*	memWrite8(REG_PCLK, 0x00);	*/	/* set PCLK to zero - don't clock the LCD until later, line disabled because zero is reset-default and we just did a reset */

		#if defined (EVE_ADAM101)
		memWrite8(REG_PWM_DUTY, 0x80); /* turn off backlight for Glyn ADAM101 module, it uses inverted values */
		#else
		memWrite8(REG_PWM_DUTY, 0); /* turn off backlight for any other module */
		#endif

		/* Initialize Display */
		memWrite16(REG_HSIZE,   EVE_HSIZE);   /* active display width */
		memWrite16(REG_HCYCLE,  EVE_HCYCLE);  /* total number of clocks per line, incl front/back porch */
		memWrite16(REG_HOFFSET, EVE_HOFFSET); /* start of active line */
		memWrite16(REG_HSYNC0,  EVE_HSYNC0);  /* start of horizontal sync pulse */
		memWrite16(REG_HSYNC1,  EVE_HSYNC1);  /* end of horizontal sync pulse */
		memWrite16(REG_VSIZE,   EVE_VSIZE);   /* active display height */
		memWrite16(REG_VCYCLE,  EVE_VCYCLE);  /* total number of lines per screen, including pre/post */
		memWrite16(REG_VOFFSET, EVE_VOFFSET); /* start of active screen */
		memWrite16(REG_VSYNC0,  EVE_VSYNC0);  /* start of vertical sync pulse */
		memWrite16(REG_VSYNC1,  EVE_VSYNC1);  /* end of vertical sync pulse */
		memWrite8(REG_SWIZZLE,  EVE_SWIZZLE); /* FT8xx output to LCD - pin order */
		memWrite8(REG_PCLK_POL, EVE_PCLKPOL); /* LCD data is clocked in on this PCLK edge */
		memWrite8(REG_CSPREAD,	EVE_CSPREAD); /* helps with noise, when set to 1 fewer signals are changed simultaneously, reset-default: 1 */

		/* do not set PCLK yet - wait for just after the first display list */

		/* configure Touch */
		memWrite8(REG_TOUCH_MODE, EVE_TMODE_CONTINUOUS); /* enable touch */
		memWrite16(REG_TOUCH_RZTHRESH, EVE_TOUCH_RZTHRESH);	/* eliminate any false touches */

		/* disable Audio for now */
		//memWrite8(REG_VOL_PB, 0xFF); /* turn recorded audio volume down */
		//memWrite8(REG_VOL_SOUND, 0xFF); /* turn synthesizer volume off */
		//memWrite16(REG_SOUND, 0x6000); /* set synthesizer to mute */

		/* write a basic display-list to get things started */
		memWrite32(EVE_RAM_DL, DL_CLEAR_RGB);
		memWrite32(EVE_RAM_DL + 4, (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
		memWrite32(EVE_RAM_DL + 8, DL_DISPLAY);	/* end of display list */
		memWrite32(REG_DLSWAP, EVE_DLSWAP_FRAME);

		/* nothing is being displayed yet... the pixel clock is still 0x00 */

		#if EVE_GEN > 3
		#if defined (EVE_PCLK_FREQ)
		uint32_t frequency;
		frequency = cmd_pclkfreq(EVE_PCLK_FREQ, 0); /* setup the second PLL for the pixel-clock according to the define in EVE_config.h for the display, as close a match as possible */
		if(frequency == 0) /* this failed for some reason so we return with an error */
		{
			return 0;
		}
		#endif
		#endif
        
		/* send pre-recorded touch calibration values, depending on the display the code is compiled for */

		#if defined (EVE_CFAF240400C1_030SC)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x0000ed11);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0x00001139);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff76809);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0x00000000);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x00010690);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0xfffadf2e);
		#elif defined (EVE_CFAF320240F_035T)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x00005614);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0x0000009e);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff43422);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0x0000001d);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0xffffbda4);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0x00f8f2ef);
		#elif defined (EVE_CFAF480128A0_039TC)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x00010485);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0x0000017f);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xfffb0bd3);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0x00000073);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x0000e293);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0x00069904);
		#elif defined (EVE_CFAF800480E0_050SC)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x000107f9);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0xffffff8c);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff451ae);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0x000000d2);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x0000feac);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0xfffcfaaf);
		#elif defined (EVE_PAF90)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x00000159);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0x0001019c);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff93625);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0x00010157);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x00000000);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0x0000c101);
		#elif defined (EVE_RiTFT43)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x000062cd);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0xfffffe45);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff45e0a);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0x000001a3);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x00005b33);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0xFFFbb870);
		#elif defined (EVE_EVE2_38)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x00007bed);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0x000001b0);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xfff60aa5);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0x00000095);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0xffffdcda);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0x00829c08);
		#elif defined (EVE_EVE2_35G) ||  defined (EVE_EVE3_35G)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x000109E4);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0x000007A6);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xFFEC1EBA);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0x0000072C);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x0001096A);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0xFFF469CF);
		#elif defined (EVE_EVE2_43G) ||  defined (EVE_EVE3_43G)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x0000a1ff);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0x00000680);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xffe54cc2);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0xffffff53);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x0000912c);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0xfffe628d);
		#elif defined (EVE_EVE2_50G) || defined (EVE_EVE3_50G)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x000109E4);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0x000007A6);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xFFEC1EBA);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0x0000072C);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x0001096A);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0xFFF469CF);
		#elif defined (EVE_EVE2_70G)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x000105BC);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0xFFFFFA8A);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0x00004670);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0xFFFFFF75);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x00010074);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0xFFFF14C8);
		#elif defined (EVE_NHD_35)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x0000f78b);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0x00000427);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xfffcedf8);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0xfffffba4);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x0000f756);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0x0009279e);
		#elif defined (EVE_RVT70)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x000074df);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0x000000e6);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xfffd5474);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0x000001af);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x00007e79);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0xffe9a63c);
		#elif defined (EVE_FT811CB_HY50HD)
			memWrite32(REG_TOUCH_TRANSFORM_A, 66353);
			memWrite32(REG_TOUCH_TRANSFORM_B, 712);
			memWrite32(REG_TOUCH_TRANSFORM_C, 4293876677);
			memWrite32(REG_TOUCH_TRANSFORM_D, 4294966157);
			memWrite32(REG_TOUCH_TRANSFORM_E, 67516);
			memWrite32(REG_TOUCH_TRANSFORM_F, 418276);
		#elif defined (EVE_ADAM101)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x000101E3);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0x00000114);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0xFFF5EEBA);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0xFFFFFF5E);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x00010226);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0x0000C783);
		#elif defined (EVE_EVE3_39G)
			memWrite32(REG_TOUCH_TRANSFORM_A, 0x00010668);
			memWrite32(REG_TOUCH_TRANSFORM_B, 0xFFFFF9DD);
			memWrite32(REG_TOUCH_TRANSFORM_C, 0x000A43EF);
			memWrite32(REG_TOUCH_TRANSFORM_D, 0xFFFFFF3A);
			memWrite32(REG_TOUCH_TRANSFORM_E, 0x0000E691);
			memWrite32(REG_TOUCH_TRANSFORM_F, 0x0004E824);

		/* activate this if you are using a module for the first time or if you need to re-calibrate it */
		/* write down the numbers on the screen and either place them in one of the pre-defined blocks above or make a new block */
		#else
		
			#if defined (EVE_ADAM101)
			memWrite8(REG_PWM_DUTY, 0x00); /* turn on backlight to 25% for Glyn ADAM101 module, it uses inverted values */
			#else
			memWrite8(REG_PWM_DUTY, 0xFF); /* turn on backlight to 25% for any other module */
			#endif
			/* calibrate touch and displays values to screen */
			cmd_dl(CMD_DLSTART);
			cmd_dl(DL_CLEAR_RGB);
			cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
			cmd_text((EVE_HSIZE/2), 50, 26, EVE_OPT_CENTER, "Please tap on the dot.");
			cmd_calibrate();
			cmd_dl(DL_DISPLAY);
			cmd_dl(CMD_SWAP);
			cmd_execute();

			uint32_t touch_a, touch_b, touch_c, touch_d, touch_e, touch_f;

			touch_a = memRead32(REG_TOUCH_TRANSFORM_A);
			touch_b = memRead32(REG_TOUCH_TRANSFORM_B);
			touch_c = memRead32(REG_TOUCH_TRANSFORM_C);
			touch_d = memRead32(REG_TOUCH_TRANSFORM_D);
			touch_e = memRead32(REG_TOUCH_TRANSFORM_E);
			touch_f = memRead32(REG_TOUCH_TRANSFORM_F);

			cmd_dl(CMD_DLSTART);
			cmd_dl(DL_CLEAR_RGB);
			cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
			cmd_dl(TAG(0));

			cmd_text(5, 15, 26, 0, "TOUCH_TRANSFORM_A:");
			cmd_text(5, 30, 26, 0, "TOUCH_TRANSFORM_B:");
			cmd_text(5, 45, 26, 0, "TOUCH_TRANSFORM_C:");
			cmd_text(5, 60, 26, 0, "TOUCH_TRANSFORM_D:");
			cmd_text(5, 75, 26, 0, "TOUCH_TRANSFORM_E:");
			cmd_text(5, 90, 26, 0, "TOUCH_TRANSFORM_F:");

			cmd_setbase(16L);
			cmd_number(310, 15, 26, EVE_OPT_RIGHTX|8, touch_a);
			cmd_number(310, 30, 26, EVE_OPT_RIGHTX|8, touch_b);
			cmd_number(310, 45, 26, EVE_OPT_RIGHTX|8, touch_c);
			cmd_number(310, 60, 26, EVE_OPT_RIGHTX|8, touch_d);
			cmd_number(310, 75, 26, EVE_OPT_RIGHTX|8, touch_e);
			cmd_number(310, 90, 26, EVE_OPT_RIGHTX|8, touch_f);

			cmd_dl(DL_DISPLAY);	/* instruct the graphics processor to show the list */
			cmd_dl(CMD_SWAP); /* make this list active */
			cmd_execute();

			while(1);
		#endif

		    memWrite8(REG_GPIO, 0x80); /* enable the DISP signal to the LCD panel, it is set to output in REG_GPIO_DIR by default */
		    memWrite8(REG_PCLK, EVE_PCLK); /* now start clocking data to the LCD panel */

            port.set_speed();

            start_cmd_burst();                             //begin commandlist
            cmd_dl_burst(CMD_DLSTART);                     //start displaylist
            cmd_dl_burst(DL_CLEAR|CLR_COL|CLR_STN|CLR_TAG);//clean screen
            cmd_dl_burst(DL_DISPLAY);                      //render display
            cmd_dl_burst(CMD_SWAP);                        //scanout display
            end_cmd_burst();                               //close commandlist

            while(busy());

			#if defined (EVE_ADAM101)
			memWrite8(REG_PWM_DUTY, 0x00); /* turn on backlight to 25% for Glyn ADAM101 module, it uses inverted values */
			#else
			memWrite8(REG_PWM_DUTY, 0xFF); /* turn on backlight to 25% for any other module */
			#endif
		return 1;
	}


	/*----------------------------------------------------------------------------------------------------------------------------*/
	/*-------- functions for display lists ---------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------------------------------------*/

	/*
	These eliminate the overhead of transmitting the command-fifo address with every single command, just wrap a sequence of commands
	with these and the address is only transmitted once at the start of the block.
	Be careful to not use any functions in the sequence that do not address the command-fifo as for example any mem...() function.
	*/
	void Display::start_cmd_burst(void)
	{
		uint32_t ftAddress;

	#if defined (EVE_DMA)
		if(port.dma_busy)
		{
			while (busy()); /* this is a safe-guard to protect segmented display-list building with DMA from overlapping */
		}
	#endif

		cmd_burst = 42;
		ftAddress = REG_CMDB_WRITE;

		#if defined (EVE_DMA)
			/* 0x low mid hi 00 */
			port.buffer[0] = ((uint8_t)(ftAddress >> 16) | MEM_WRITE) | (ftAddress & 0x0000ff00) |  ((uint8_t)(ftAddress) << 16);
			port.buffer[0] = port.buffer[0] << 8;
			port.dma_buffer_index = 1;
		#else
			port.cs_set();
			port.transmit((uint8_t)(ftAddress >> 16) | MEM_WRITE); /* send Memory Write plus high address byte */
			port.transmit((uint8_t)(ftAddress >> 8)); /* send middle address byte */
			port.transmit((uint8_t)(ftAddress)); /* send low address byte */
		#endif
	}


	void Display::end_cmd_burst(void)
	{
		cmd_burst = 0;

		#if defined (EVE_DMA)
			port.dma_transfer(); /* begin DMA transfer */
		#else
			port.cs_clear();
		#endif
	}

	#if 0
	/* private function, begin a co-processor command, only used for non-burst commands */
	void Display::start_command(uint32_t command)
	{
		uint32_t ftAddress;

		ftAddress = REG_CMDB_WRITE;
		port.cs_set();
		port.transmit((uint8_t)(ftAddress >> 16) | MEM_WRITE); /* send Memory Write plus high address byte */
		port.transmit((uint8_t)(ftAddress >> 8)); /* send middle address byte */
		port.transmit((uint8_t)(ftAddress)); /* send low address byte */

		port.transmit_32(command);
	}
	#endif

	/* write a string to co-processor memory in context of a command: no chip-select, just plain SPI-transfers */
	void Display::private_string_write(const char *text)
	{
		uint8_t textindex = 0;
		uint8_t padding = 0;
		uint8_t *bytes = (uint8_t *) text; /* treat the array as bunch of bytes */

		if(cmd_burst)
		{
			uint32_t calc;
			uint8_t data;

			for(textindex = 0; textindex < 249;)
			{
				calc = 0;

				data = bytes[textindex++];
				if(data == 0)
				{
					port.transmit_burst(calc);
					break;
				}
				calc += (uint32_t) (data);

				data = bytes[textindex++];
				if(data == 0)
				{
					port.transmit_burst(calc);
					break;
				}
				calc += ((uint32_t) data) << 8;

				data = bytes[textindex++];
				if(data == 0)
				{
					port.transmit_burst(calc);
					break;
				}
				calc += ((uint32_t) data) << 16;

				data = bytes[textindex++];
				if(data == 0)
				{
					port.transmit_burst(calc);
					break;
				}
				calc += ((uint32_t) data) << 24;

				port.transmit_burst(calc);
			}
		}
		else
		{
			while(bytes[textindex] != 0)
			{
				port.transmit(bytes[textindex]);
				textindex++;
				if(textindex > 249) /* there appears to be no end for the "string", so leave */
				{
					break;
				}
			}

			/* we need to transmit at least one 0x00 byte and up to four if the string happens to be 4-byte aligned already */
			padding = textindex & 3;  /* 0, 1, 2 or 3 */
			padding = 4-padding; /* 4, 3, 2 or 1 */
			textindex += padding;

			while(padding > 0)
			{
				port.transmit(0);
				padding--;
			}
		}
	}



	/* BT817 / BT818 */
	#if EVE_GEN > 3

	void Display::cmd_animframeram(int16_t x0, int16_t y0, uint32_t aoptr, uint32_t frame)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_ANIMFRAMERAM);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit_32(aoptr);
			port.transmit_32(frame);

			port.cs_clear();
		}
	}


	void Display::cmd_animframeram_burst(int16_t x0, int16_t y0, uint32_t aoptr, uint32_t frame)
	{
		port.transmit_burst(CMD_ANIMFRAMERAM);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst(aoptr);
		port.transmit_burst(frame);
	}


	void Display::cmd_animstartram(int32_t ch, uint32_t aoptr, uint32_t loop)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_ANIMSTARTRAM);
			port.transmit_32(ch);
			port.transmit_32(aoptr);
			port.transmit_32(loop);
			port.cs_clear();
		}
	}


	void Display::cmd_animstartram_burst(int32_t ch, uint32_t aoptr, uint32_t loop)
	{
		port.transmit_burst(CMD_ANIMSTARTRAM);
		port.transmit_burst(ch);
		port.transmit_burst(aoptr);
		port.transmit_burst(loop);
	}


	void Display::cmd_apilevel(uint32_t level)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_APILEVEL);
			port.transmit_32(level);
			port.cs_clear();
		}
	}


	void Display::cmd_apilevel_burst(uint32_t level)
	{
		port.transmit_burst(CMD_APILEVEL);
		port.transmit_burst(level);
	}


	void Display::cmd_calibratesub(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_CALIBRATESUB);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(width));
			port.transmit((uint8_t)(width >> 8));
			port.transmit((uint8_t)(height));
			port.transmit((uint8_t)(height >> 8));

			port.cs_clear();
		}
	}


	void Display::cmd_calllist(uint32_t adr)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_CALLLIST);
			port.transmit_32(adr);
			port.cs_clear();
		}
	}


	void Display::cmd_calllist_burst(uint32_t adr)
	{
		port.transmit_burst(CMD_CALLLIST);
		port.transmit_burst(adr);
	}


	void Display::cmd_hsf(uint32_t hsf)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_HSF);
			port.transmit_32(hsf);
			port.cs_clear();
		}
	}


	void Display::cmd_hsf_burst(uint32_t hsf)
	{
		port.transmit_burst(CMD_HSF);
		port.transmit_burst(hsf);
	}


	void Display::cmd_runanim(uint32_t waitmask, uint32_t play)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_RUNANIM);
			port.transmit_32(waitmask);
			port.transmit_32(play);
			port.cs_clear();
		}
	}


	void Display::cmd_runanim_burst(uint32_t waitmask, uint32_t play)
	{
		port.transmit_burst(CMD_RUNANIM);
		port.transmit_burst(waitmask);
		port.transmit_burst(play);
	}


	#endif /* EVE_GEN > 3 */


	/* BT815 / BT816 */
	#if EVE_GEN > 2

	void Display::cmd_animdraw(int32_t ch)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_ANIMDRAW);
			port.transmit_32(ch);
			port.cs_clear();
		}
	}


	void Display::cmd_animdraw_burst(int32_t ch)
	{
		port.transmit_burst(CMD_ANIMDRAW);
		port.transmit_burst(ch);
	}


	void Display::cmd_animframe(int16_t x0, int16_t y0, uint32_t aoptr, uint32_t frame)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_ANIMFRAME);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit_32(aoptr);
			port.transmit_32(frame);
			port.cs_clear();
		}
	}


	void Display::cmd_animframe_burst(int16_t x0, int16_t y0, uint32_t aoptr, uint32_t frame)
	{
		port.transmit_burst(CMD_ANIMFRAME);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst(aoptr);
		port.transmit_burst(frame);
	}


	void Display::cmd_animstart(int32_t ch, uint32_t aoptr, uint32_t loop)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_ANIMSTART);

			port.transmit_32(ch);
			port.transmit_32(aoptr);
			port.transmit_32(loop);
			port.cs_clear();
		}
	}


	void Display::cmd_animstart_burst(int32_t ch, uint32_t aoptr, uint32_t loop)
	{
		port.transmit_burst(CMD_ANIMSTART);
		port.transmit_burst(ch);
		port.transmit_burst(aoptr);
		port.transmit_burst(loop);
	}


	void Display::cmd_animstop(int32_t ch)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_ANIMSTOP);
			port.transmit_32(ch);
			port.cs_clear();
		}
	}


	void Display::cmd_animstop_burst(int32_t ch)
	{
		port.transmit_burst(CMD_ANIMSTOP);
		port.transmit_burst(ch);
	}


	void Display::cmd_animxy(int32_t ch, int16_t x0, int16_t y0)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_ANIMXY);
			port.transmit_32(ch);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.cs_clear();
		}
	}


	void Display::cmd_animxy_burst(int32_t ch, int16_t x0, int16_t y0)
	{
		port.transmit_burst(CMD_ANIMXY);
		port.transmit_burst(ch);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
	}


	void Display::cmd_appendf(uint32_t ptr, uint32_t num)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_APPENDF);
			port.transmit_32(ptr);
			port.transmit_32(num);
			port.cs_clear();
		}
	}


	void Display::cmd_appendf_burst(uint32_t ptr, uint32_t num)
	{
		port.transmit_burst(CMD_APPENDF);
		port.transmit_burst(ptr);
		port.transmit_burst(num);
	}



	uint16_t Display::cmd_bitmap_transform(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t tx0, int32_t ty0, int32_t tx1, int32_t ty1, int32_t tx2, int32_t ty2)
	{
		if(!cmd_burst)
		{
			uint16_t cmdoffset;

			begin_cmd(CMD_BITMAP_TRANSFORM);
			port.transmit_32(x0);
			port.transmit_32(y0);
			port.transmit_32(x1);
			port.transmit_32(y1);
			port.transmit_32(x2);
			port.transmit_32(y2);
			port.transmit_32(tx0);
			port.transmit_32(ty0);
			port.transmit_32(tx1);
			port.transmit_32(ty1);
			port.transmit_32(tx2);
			port.transmit_32(ty2);
			port.transmit_32(0);
			port.cs_clear();
			while (busy());
			cmdoffset = memRead16(REG_CMD_WRITE);  /* read the graphics processor write pointer */
			cmdoffset -= 4;
			cmdoffset &= 0x0fff;
			return (memRead32(EVE_RAM_CMD + cmdoffset));
		}
		return 0;
	}


	/* note: as this is meant for use in burst-mode display-list generation the result parameter is ignored */
	void Display::cmd_bitmap_transform_burst( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t tx0, int32_t ty0, int32_t tx1, int32_t ty1, int32_t tx2, int32_t ty2)
	{
		port.transmit_burst(CMD_BITMAP_TRANSFORM);
		port.transmit_burst(x0);
		port.transmit_burst(y0);
		port.transmit_burst(x1);
		port.transmit_burst(y1);
		port.transmit_burst(x2);
		port.transmit_burst(y2);
		port.transmit_burst(tx0);
		port.transmit_burst(ty0);
		port.transmit_burst(tx1);
		port.transmit_burst(ty1);
		port.transmit_burst(tx2);
		port.transmit_burst(ty2);
		port.transmit_burst(0);
	}


	void Display::cmd_fillwidth(uint32_t s)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_FILLWIDTH);
			port.transmit_32(s);
			port.cs_clear();
		}
	}


	void Display::cmd_fillwidth_burst(uint32_t s)
	{
		port.transmit_burst(CMD_FILLWIDTH);
		port.transmit_burst(s);
	}


	void Display::cmd_gradienta(int16_t x0, int16_t y0, uint32_t argb0, int16_t x1, int16_t y1, uint32_t argb1)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_GRADIENTA);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit_32(argb0);

			port.transmit((uint8_t)(x1));
			port.transmit((uint8_t)(x1 >> 8));
			port.transmit((uint8_t)(y1));
			port.transmit((uint8_t)(y1 >> 8));

			port.transmit_32(argb1);

			port.cs_clear();
		}
	}


	void Display::cmd_gradienta_burst(int16_t x0, int16_t y0, uint32_t argb0, int16_t x1, int16_t y1, uint32_t argb1)
	{
		port.transmit_burst(CMD_GRADIENTA);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst(argb0);
		port.transmit_burst((uint32_t) x1 + ((uint32_t) y1 << 16));
		port.transmit_burst(argb1);
	}


	void Display::cmd_rotatearound(int32_t x0, int32_t y0, int32_t angle, int32_t scale)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_ROTATEAROUND);
			port.transmit_32(x0);
			port.transmit_32(y0);
			port.transmit_32(angle);
			port.transmit_32(scale);
			port.cs_clear();
		}
	}


	void Display::cmd_rotatearound_burst(int32_t x0, int32_t y0, int32_t angle, int32_t scale)
	{
		port.transmit_burst(CMD_ROTATEAROUND);
		port.transmit_burst(x0);
		port.transmit_burst(y0);
		port.transmit_burst(angle);
		port.transmit_burst(scale);
	}


	/* as the name implies, "num_args" is the number of arguments passed to this function as variadic arguments */
	void Display::cmd_button_var(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text, uint8_t num_args, ...)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_BUTTON);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(w0));
			port.transmit((uint8_t)(w0 >> 8));
			port.transmit((uint8_t)(h0));
			port.transmit((uint8_t)(h0 >> 8));

			port.transmit((uint8_t)(font));
			port.transmit((uint8_t)(font >> 8));
			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));

			private_string_write(text);

			if(options & EVE_OPT_FORMAT)
			{
				va_list arguments;
				uint8_t counter;
				uint32_t data;

				va_start(arguments, num_args);

				for(counter=0;counter<num_args;counter++)
				{
					data = (uint32_t) va_arg(arguments, int);
					port.transmit_32(data);
				}

				va_end(arguments);
			}
			port.cs_clear();
		}
	}


	void Display::cmd_button_var_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text, uint8_t num_args, ...)
	{
		port.transmit_burst(CMD_BUTTON);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst((uint32_t) font + ((uint32_t) options << 16));
		private_string_write(text);

		if(options & EVE_OPT_FORMAT)
		{
			va_list arguments;
			uint8_t counter;

			va_start(arguments, num_args);

			for(counter=0;counter<num_args;counter++)
			{
				port.transmit_burst((uint32_t) va_arg(arguments, int));
			}
			va_end(arguments);
		}
	}


	/* as the name implies, "num_args" is the number of arguments passed to this function as variadic arguments */
	void Display::cmd_text_var(int16_t x0, int16_t y0, int16_t font, uint16_t options, const char* text, uint8_t num_args, ...)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_TEXT);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(font));
			port.transmit((uint8_t)(font >> 8));
			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));

			private_string_write(text);

			if(options & EVE_OPT_FORMAT)
			{
				va_list arguments;
				uint8_t counter;
				uint32_t data;

				va_start(arguments, num_args);

				for(counter=0;counter<num_args;counter++)
				{
					data = (uint32_t) va_arg(arguments, int);
					port.transmit_32(data);
				}
				va_end(arguments);
			}
			port.cs_clear();
		}
	}


	void Display::cmd_text_var_burst(int16_t x0, int16_t y0, int16_t font, uint16_t options, const char* text, uint8_t num_args, ...)
	{
		port.transmit_burst(CMD_TEXT);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) font + ((uint32_t) options << 16));
		private_string_write(text);

		#if EVE_GEN > 2
		if(options & EVE_OPT_FORMAT)
		{
			va_list arguments;
			uint8_t counter;

			va_start(arguments, num_args);

			for(counter=0;counter<num_args;counter++)
			{
				port.transmit_burst((uint32_t) va_arg(arguments, int));
			}
			va_end(arguments);
		}
		#endif
	}


	/* as the name implies, "num_args" is the number of arguments passed to this function as variadic arguments */
	void Display::cmd_toggle_var(int16_t x0, int16_t y0, int16_t w0, int16_t font, uint16_t options, uint16_t state, const char* text, uint8_t num_args, ...)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_TOGGLE);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(w0));
			port.transmit((uint8_t)(w0 >> 8));
			port.transmit((uint8_t)(font));
			port.transmit((uint8_t)(font >> 8));

			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));
			port.transmit((uint8_t)(state));
			port.transmit((uint8_t)(state >> 8));

			private_string_write(text);

			if(options & EVE_OPT_FORMAT)
			{
				va_list arguments;
				uint8_t counter;
				uint32_t data;

				va_start(arguments, num_args);

				for(counter=0;counter<num_args;counter++)
				{
					data = (uint32_t) va_arg(arguments, int);
					port.transmit_32(data);
				}

				va_end(arguments);
			}
			port.cs_clear();
		}
	}


	void Display::cmd_toggle_var_burst(int16_t x0, int16_t y0, int16_t w0, int16_t font, uint16_t options, uint16_t state, const char* text, uint8_t num_args, ...)
	{
		port.transmit_burst(CMD_TOGGLE);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) font << 16));
		port.transmit_burst((uint32_t) options + ((uint32_t) state << 16));
		private_string_write(text);

		if(options & EVE_OPT_FORMAT)
		{
			va_list arguments;
			uint8_t counter;

			va_start(arguments, num_args);

			for(counter=0;counter<num_args;counter++)
			{
				port.transmit_burst((uint32_t) va_arg(arguments, int));
			}

			va_end(arguments);
		}
	}

	#endif /* EVE_GEN > 2 */



	/* generic function for all commands that have no arguments and all display-list specific control words */
	/*
	examples:
	cmd_dl(CMD_DLSTART);
	cmd_dl(CMD_SWAP);
	cmd_dl(CMD_SCREENSAVER);
	cmd_dl(LINE_WIDTH(1*16));
	cmd_dl(VERTEX2F(0,0));
	cmd_dl(DL_BEGIN | EVE_RECTS);
	*/
	void Display::cmd_dl(uint32_t command)
	{
		if(!cmd_burst)
		{
			begin_cmd(command);
			port.cs_clear();
		}
	}


	void Display::cmd_dl_burst(uint32_t command)
	{
		port.transmit_burst(command);
	}


	void Display::cmd_append(uint32_t ptr, uint32_t num)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_APPEND);
			port.transmit_32(ptr);
			port.transmit_32(num);
			port.cs_clear();
		}
	}


	void Display::cmd_append_burst(uint32_t ptr, uint32_t num)
	{
		port.transmit_burst(CMD_APPEND);
		port.transmit_burst(ptr);
		port.transmit_burst(num);
	}


	void Display::cmd_bgcolor(uint32_t color)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_BGCOLOR);
			port.transmit((uint8_t)(color));
			port.transmit((uint8_t)(color >> 8));
			port.transmit((uint8_t)(color >> 16));
			port.transmit(0x00);
			port.cs_clear();
		}
	}


	void Display::cmd_bgcolor_burst(uint32_t color)
	{
		port.transmit_burst(CMD_BGCOLOR);
		port.transmit_burst(color);
	}


	void Display::cmd_button(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_BUTTON);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));

			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(w0));
			port.transmit((uint8_t)(w0 >> 8));

			port.transmit((uint8_t)(h0));
			port.transmit((uint8_t)(h0 >> 8));

			port.transmit((uint8_t)(font));
			port.transmit((uint8_t)(font >> 8));

			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));

			private_string_write(text);
			port.cs_clear();
		}
	}


	void Display::cmd_button_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text)
	{
		port.transmit_burst(CMD_BUTTON);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst((uint32_t) font + ((uint32_t) options << 16));
		private_string_write(text);
	}


	void Display::cmd_calibrate(void)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_CALIBRATE);
			port.transmit_32(0);
			port.cs_clear();
		}
	}


	void Display::cmd_clock(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t hours, uint16_t minutes, uint16_t seconds, uint16_t millisecs)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_CLOCK);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(r0));
			port.transmit((uint8_t)(r0 >> 8));
			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));

			port.transmit((uint8_t)(hours));
			port.transmit((uint8_t)(hours >> 8));
			port.transmit((uint8_t)(minutes));
			port.transmit((uint8_t)(minutes >> 8));

			port.transmit((uint8_t)(seconds));
			port.transmit((uint8_t)(seconds >> 8));
			port.transmit((uint8_t)(millisecs));
			port.transmit((uint8_t)(millisecs >> 8));
			port.cs_clear();
		}
	}


	void Display::cmd_clock_burst(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t hours, uint16_t minutes, uint16_t seconds, uint16_t millisecs)
	{
		port.transmit_burst(CMD_CLOCK);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) r0 + ((uint32_t) options << 16));
		port.transmit_burst((uint32_t) hours + ((uint32_t) minutes << 16));
		port.transmit_burst((uint32_t) seconds + ((uint32_t) millisecs << 16));
	}


	void Display::cmd_dial(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t val)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_DIAL);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(r0));
			port.transmit((uint8_t)(r0 >> 8));
			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));

			port.transmit((uint8_t)(val));
			port.transmit((uint8_t)(val >> 8));
			port.transmit(0);
			port.transmit(0);

			port.cs_clear();
		}
	}


	void Display::cmd_dial_burst(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t val)
	{
		port.transmit_burst(CMD_DIAL);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) r0 + ((uint32_t) options << 16));
		port.transmit_burst(val);
	}


	void Display::cmd_fgcolor(uint32_t color)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_FGCOLOR);
			port.transmit((uint8_t)(color));
			port.transmit((uint8_t)(color >> 8));
			port.transmit((uint8_t)(color >> 16));
			port.transmit(0x00);
			port.cs_clear();
		}
	}


	void Display::cmd_fgcolor_burst(uint32_t color)
	{
		port.transmit_burst(CMD_FGCOLOR);
		port.transmit_burst(color);
	}


	void Display::cmd_gauge(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t major, uint16_t minor, uint16_t val, uint16_t range)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_GAUGE);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(r0));
			port.transmit((uint8_t)(r0 >> 8));
			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));

			port.transmit((uint8_t)(major));
			port.transmit((uint8_t)(major >> 8));
			port.transmit((uint8_t)(minor));
			port.transmit((uint8_t)(minor >> 8));

			port.transmit((uint8_t)(val));
			port.transmit((uint8_t)(val >> 8));
			port.transmit((uint8_t)(range));
			port.transmit((uint8_t)(range >> 8));

			port.cs_clear();
		}
	}


	void Display::cmd_gauge_burst(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t major, uint16_t minor, uint16_t val, uint16_t range)
	{
		port.transmit_burst(CMD_GAUGE);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) r0 + ((uint32_t) options << 16));
		port.transmit_burst((uint32_t) major + ((uint32_t) minor << 16));
		port.transmit_burst((uint32_t) val + ((uint32_t) range << 16));
	}


	/* this function is meant to be called  with display-list building, but it waits for completion */
	/* as this function returns values by writing to the command-fifo, it can not be used with cmd-burst */
	/* get the properties of the bitmap transform matrix and write the values to the variables that are supplied by pointers */
	void Display::cmd_getmatrix(int32_t *get_a, int32_t *get_b, int32_t *get_c, int32_t *get_d, int32_t *get_e, int32_t *get_f)
	{
		if(!cmd_burst)
		{
			uint16_t cmdoffset;

			begin_cmd(CMD_GETMATRIX);
			port.transmit_32(0);
			port.transmit_32(0);
			port.transmit_32(0);
			port.transmit_32(0);
			port.transmit_32(0);
			port.transmit_32(0);
			port.cs_clear();
			while (busy());
			cmdoffset = memRead16(REG_CMD_WRITE);  /* read the graphics processor write pointer */

			if(get_f)
			{
				*get_f = memRead32(EVE_RAM_CMD + ((cmdoffset - 4) & 0xfff));
			}
			if(get_e)
			{
				*get_e = memRead32(EVE_RAM_CMD + ((cmdoffset - 8) & 0xfff));
			}
			if(get_d)
			{
				*get_d = memRead32(EVE_RAM_CMD + ((cmdoffset - 12) & 0xfff));
			}
			if(get_c)
			{
				*get_c = memRead32(EVE_RAM_CMD + ((cmdoffset - 16) & 0xfff));
			}
			if(get_b)
			{
				*get_b = memRead32(EVE_RAM_CMD + ((cmdoffset - 20) & 0xfff));
			}
			if(get_a)
			{
				*get_a = memRead32(EVE_RAM_CMD + ((cmdoffset - 24) & 0xfff));
			}
		}
	}


	void Display::cmd_gradcolor(uint32_t color)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_GRADCOLOR);
			port.transmit((uint8_t)(color));
			port.transmit((uint8_t)(color >> 8));
			port.transmit((uint8_t)(color >> 16));
			port.transmit(0x00);
			port.cs_clear();
		}
	}


	void Display::cmd_gradcolor_burst(uint32_t color)
	{
		port.transmit_burst(CMD_GRADCOLOR);
		port.transmit_burst(color);
	}


	void Display::cmd_gradient(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_GRADIENT);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(rgb0));
			port.transmit((uint8_t)(rgb0 >> 8));
			port.transmit((uint8_t)(rgb0 >> 16));
			port.transmit(0x00);

			port.transmit((uint8_t)(x1));
			port.transmit((uint8_t)(x1 >> 8));
			port.transmit((uint8_t)(y1));
			port.transmit((uint8_t)(y1 >> 8));

			port.transmit((uint8_t)(rgb1));
			port.transmit((uint8_t)(rgb1 >> 8));
			port.transmit((uint8_t)(rgb1 >> 16));
			port.transmit(0x00);

			port.cs_clear();
		}
	}


	void Display::cmd_gradient_burst(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1)
	{
		port.transmit_burst(CMD_GRADIENT);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst(rgb0);
		port.transmit_burst((uint32_t) x1 + ((uint32_t) y1 << 16));
		port.transmit_burst(rgb1);
	}


	void Display::cmd_keys(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_KEYS);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(w0));
			port.transmit((uint8_t)(w0 >> 8));
			port.transmit((uint8_t)(h0));
			port.transmit((uint8_t)(h0 >> 8));

			port.transmit((uint8_t)(font));
			port.transmit((uint8_t)(font >> 8));
			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));

			private_string_write(text);
			port.cs_clear();
		}
	}


	void Display::cmd_keys_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text)
	{
		port.transmit_burst(CMD_KEYS);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst((uint32_t) font + ((uint32_t) options << 16));

		private_string_write(text);
	}


	void Display::cmd_number(int16_t x0, int16_t y0, int16_t font, uint16_t options, int32_t number)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_NUMBER);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(font));
			port.transmit((uint8_t)(font >> 8));
			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));

			port.transmit_32(number);
			port.cs_clear();
		}
	}


	void Display::cmd_number_burst(int16_t x0, int16_t y0, int16_t font, uint16_t options, int32_t number)
	{
		port.transmit_burst(CMD_NUMBER);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) font + ((uint32_t) options << 16));
		port.transmit_burst(number);
	}


	void Display::cmd_progress(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_PROGRESS);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(w0));
			port.transmit((uint8_t)(w0 >> 8));
			port.transmit((uint8_t)(h0));
			port.transmit((uint8_t)(h0 >> 8));

			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));
			port.transmit((uint8_t)(val));
			port.transmit((uint8_t)(val >> 8));

			port.transmit((uint8_t)(range));
			port.transmit((uint8_t)(range >> 8));
			port.transmit(0x00);	/* dummy byte for 4-byte alignment */
			port.transmit(0x00); /* dummy byte for 4-byte alignment */

			port.cs_clear();
		}
	}


	void Display::cmd_progress_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range)
	{
		port.transmit_burst(CMD_PROGRESS);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst((uint32_t) options + ((uint32_t) val << 16));
		port.transmit_burst(range);
	}


	void Display::cmd_romfont(uint32_t font, uint32_t romslot)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_ROMFONT);

			port.transmit((uint8_t)(font));
			port.transmit((uint8_t)(font >> 8));
			port.transmit(0x00);
			port.transmit(0x00);

			port.transmit((uint8_t)(romslot));
			port.transmit((uint8_t)(romslot >> 8));
			port.transmit(0x00);
			port.transmit(0x00);

			port.cs_clear();
		}
	}


	void Display::cmd_romfont_burst(uint32_t font, uint32_t romslot)
	{
		port.transmit_burst(CMD_ROMFONT);
		port.transmit_burst(font);
		port.transmit_burst(romslot);
	}


	void Display::cmd_rotate(int32_t angle)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_ROTATE);
			port.transmit_32(angle);
			port.cs_clear();
		}
	}


	void Display::cmd_rotate_burst(int32_t angle)
	{
		port.transmit_burst(CMD_ROTATE);
		port.transmit_burst(angle);
	}


	void Display::cmd_scale(int32_t sx, int32_t sy)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_SCALE);
			port.transmit_32(sx);
			port.transmit_32(sy);
			port.cs_clear();
		}
	}


	void Display::cmd_scale_burst(int32_t sx, int32_t sy)
	{
		port.transmit_burst(CMD_SCALE);
		port.transmit_burst(sx);
		port.transmit_burst(sy);
	}


	void Display::cmd_scrollbar(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t size, uint16_t range)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_SCROLLBAR);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(w0));
			port.transmit((uint8_t)(w0 >> 8));
			port.transmit((uint8_t)(h0));
			port.transmit((uint8_t)(h0 >> 8));

			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));
			port.transmit((uint8_t)(val));
			port.transmit((uint8_t)(val >> 8));

			port.transmit((uint8_t)(size));
			port.transmit((uint8_t)(size >> 8));
			port.transmit((uint8_t)(range));
			port.transmit((uint8_t)(range >> 8));

			port.cs_clear();
		}
	}


	void Display::cmd_scrollbar_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t size, uint16_t range)
	{
		port.transmit_burst(CMD_SCROLLBAR);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst((uint32_t) options + ((uint32_t) val << 16));
		port.transmit_burst((uint32_t) size + ((uint32_t) range << 16));
	}


	void Display::cmd_setbase(uint32_t base)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_SETBASE);
			port.transmit_32(base);
			port.cs_clear();
		}
	}


	void Display::cmd_setbase_burst(uint32_t base)
	{
		port.transmit_burst(CMD_SETBASE);
		port.transmit_burst(base);
	}


	void Display::cmd_setbitmap(uint32_t addr, uint16_t fmt, uint16_t width, uint16_t height)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_SETBITMAP);
			port.transmit_32(addr);

			port.transmit((uint8_t)(fmt));
			port.transmit((uint8_t)(fmt>> 8));
			port.transmit((uint8_t)(width));
			port.transmit((uint8_t)(width >> 8));

			port.transmit((uint8_t)(height));
			port.transmit((uint8_t)(height >> 8));
			port.transmit(0);
			port.transmit(0);

			port.cs_clear();
		}
	}


	void Display::cmd_setbitmap_burst(uint32_t addr, uint16_t fmt, uint16_t width, uint16_t height)
	{
		port.transmit_burst(CMD_SETBITMAP);
		port.transmit_burst(addr);
		port.transmit_burst((uint32_t) fmt + ((uint32_t) width << 16));
		port.transmit_burst(height);
	}


	void Display::cmd_setfont(uint32_t font, uint32_t ptr)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_SETFONT);
			port.transmit_32(font);
			port.transmit_32(ptr);
			port.cs_clear();
		}
	}


	void Display::cmd_setfont_burst(uint32_t font, uint32_t ptr)
	{
		port.transmit_burst(CMD_SETFONT);
		port.transmit_burst(font);
		port.transmit_burst(ptr);
	}


	void Display::cmd_setfont2(uint32_t font, uint32_t ptr, uint32_t firstchar)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_SETFONT2);
			port.transmit_32(font);
			port.transmit_32(ptr);
			port.transmit_32(firstchar);
			port.cs_clear();
		}
	}


	void Display::cmd_setfont2_burst(uint32_t font, uint32_t ptr, uint32_t firstchar)
	{
		port.transmit_burst(CMD_SETFONT2);
		port.transmit_burst(font);
		port.transmit_burst(ptr);
		port.transmit_burst(firstchar);
	}


	void Display::cmd_setscratch(uint32_t handle)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_SETSCRATCH);
			port.transmit_32(handle);
			port.cs_clear();
		}
	}


	void Display::cmd_setscratch_burst(uint32_t handle)
	{
		port.transmit_burst(CMD_SETSCRATCH);
		port.transmit_burst(handle);
	}


	void Display::cmd_sketch(int16_t x0, int16_t y0, uint16_t w0, uint16_t h0, uint32_t ptr, uint16_t format)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_SKETCH);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(w0));
			port.transmit((uint8_t)(w0 >> 8));
			port.transmit((uint8_t)(h0));
			port.transmit((uint8_t)(h0 >> 8));

			port.transmit_32(ptr);

			port.transmit((uint8_t)(format));
			port.transmit((uint8_t)(format >> 8));
			port.transmit(0);
			port.transmit(0);

			port.cs_clear();
		}
	}


	void Display::cmd_sketch_burst(int16_t x0, int16_t y0, uint16_t w0, uint16_t h0, uint32_t ptr, uint16_t format)
	{
		port.transmit_burst(CMD_SKETCH);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst(ptr);
		port.transmit_burst(format);
	}


	void Display::cmd_slider(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_SLIDER);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(w0));
			port.transmit((uint8_t)(w0 >> 8));
			port.transmit((uint8_t)(h0));
			port.transmit((uint8_t)(h0 >> 8));

			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));
			port.transmit((uint8_t)(val));
			port.transmit((uint8_t)(val >> 8));

			port.transmit((uint8_t)(range));
			port.transmit((uint8_t)(range >> 8));
			port.transmit(0x00); /* dummy byte for 4-byte alignment */
			port.transmit(0x00); /* dummy byte for 4-byte alignment */

			port.cs_clear();
		}
	}


	void Display::cmd_slider_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range)
	{
		port.transmit_burst(CMD_SLIDER);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst((uint32_t) options + ((uint32_t) val << 16));
		port.transmit_burst(range);
	}


	void Display::cmd_spinner(int16_t x0, int16_t y0, uint16_t style, uint16_t scale)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_SPINNER);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(style));
			port.transmit((uint8_t)(style >> 8));
			port.transmit((uint8_t)(scale));
			port.transmit((uint8_t)(scale >> 8));

			port.cs_clear();
		}
	}


	void Display::cmd_spinner_burst(int16_t x0, int16_t y0, uint16_t style, uint16_t scale)
	{
		port.transmit_burst(CMD_SPINNER);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) style + ((uint32_t) scale << 16));
	}


	void Display::cmd_text(int16_t x0, int16_t y0, int16_t font, uint16_t options, const char* text)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_TEXT);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(font));
			port.transmit((uint8_t)(font >> 8));
			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));

			private_string_write(text);
			port.cs_clear();
		}
	}


	void Display::cmd_text_burst(int16_t x0, int16_t y0, int16_t font, uint16_t options, const char* text)
	{
		port.transmit_burst(CMD_TEXT);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) font + ((uint32_t) options << 16));
		private_string_write(text);
	}


	void Display::cmd_toggle(int16_t x0, int16_t y0, int16_t w0, int16_t font, uint16_t options, uint16_t state, const char* text)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_TOGGLE);

			port.transmit((uint8_t)(x0));
			port.transmit((uint8_t)(x0 >> 8));
			port.transmit((uint8_t)(y0));
			port.transmit((uint8_t)(y0 >> 8));

			port.transmit((uint8_t)(w0));
			port.transmit((uint8_t)(w0 >> 8));
			port.transmit((uint8_t)(font));
			port.transmit((uint8_t)(font >> 8));

			port.transmit((uint8_t)(options));
			port.transmit((uint8_t)(options >> 8));
			port.transmit((uint8_t)(state));
			port.transmit((uint8_t)(state >> 8));

			private_string_write(text);
			port.cs_clear();
		}
	}


	void Display::cmd_toggle_burst(int16_t x0, int16_t y0, int16_t w0, int16_t font, uint16_t options, uint16_t state, const char* text)
	{
		port.transmit_burst(CMD_TOGGLE);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) font << 16));
		port.transmit_burst((uint32_t) options + ((uint32_t) state << 16));
		private_string_write(text);
	}


	void Display::cmd_translate(int32_t tx, int32_t ty)
	{
		if(!cmd_burst)
		{
			begin_cmd(CMD_TRANSLATE);
			port.transmit_32(tx);
			port.transmit_32(ty);
			port.cs_clear();
		}
	}


	void Display::cmd_translate_burst(int32_t tx, int32_t ty)
	{
		port.transmit_burst(CMD_TRANSLATE);
		port.transmit_burst(tx);
		port.transmit_burst(ty);
	}


	void Display::color_rgb(uint32_t color)
	{
		if(!cmd_burst)
		{
			begin_cmd(DL_COLOR_RGB | color);
			port.cs_clear();
		}
	}


	void Display::color_rgb_burst(uint32_t color)
	{
		port.transmit_burst(DL_COLOR_RGB | color);
	}


	/*---------------------------------------------------------------------------------------------------------------------------*/
	/*-------- special purpose functions ------------------- --------------------------------------------------------------------*/
	/*---------------------------------------------------------------------------------------------------------------------------*/


	/* this is meant to be called outside display-list building */
	/* this function displays an interactive calibration screen, calculates the calibration values and */
	/* writes the new values to the touch matrix registers of EVE */
	/* unlike the built-in cmd_calibrate() of EVE this also works with displays that are cut down from larger ones like EVE2-38A / EVE2-38G */
	/* the height is needed as parameter as EVE_VSIZE for the EVE2-38 is 272 but the visible size is only 116 */
	/* so the call would be EVE_calibrate_manual(116); for the EVE2-38A and EVE2-38G while for most other displays */
	/* using EVE_calibrate_manual(EVE_VSIZE) would work - but for normal displays the built-in cmd_calibrate would work as expected anyways */
	/* this code was taken from the MatrixOrbital EVE2-Library on Github, adapted and modified */
	void Display::calibrate_manual(uint16_t height)
	{
		uint32_t displayX[3], displayY[3];
		uint32_t touchX[3], touchY[3];
		uint32_t touchValue;
		int32_t tmp, k;
		int32_t TransMatrix[6];
		uint8_t count = 0;
		char num[2];
		uint8_t touch_lock = 1;

		/* these values determine where your calibration points will be drawn on your display */
		displayX[0] = (EVE_HSIZE * 0.15);
		displayY[0] = (height * 0.15);

		displayX[1] = (EVE_HSIZE * 0.85);
		displayY[1] = (height / 2);

		displayX[2] = (EVE_HSIZE / 2);
		displayY[2] = (height * 0.85);

		while (count < 3)
		{
			cmd_dl(CMD_DLSTART);
			cmd_dl(DL_CLEAR_RGB | 0x000000);
			cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);

			/* draw Calibration Point on screen */
			cmd_dl(DL_COLOR_RGB | 0x0000ff);
			cmd_dl(POINT_SIZE(20*16));
			cmd_dl((DL_BEGIN | EVE_POINTS));
			cmd_dl(VERTEX2F((uint32_t)(displayX[count]) * 16, (uint32_t)((displayY[count])) * 16));
			cmd_dl(DL_END);
			cmd_dl(DL_COLOR_RGB | 0xffffff);
			cmd_text((EVE_HSIZE/2), 50, 27, EVE_OPT_CENTER, "Please tap on the dot.");
			num[0] = count + 0x31; num[1] = 0; /* null terminated string of one character */
			cmd_text(displayX[count], displayY[count], 27, EVE_OPT_CENTER, num);

			cmd_dl(DL_DISPLAY);
			cmd_dl(CMD_SWAP);
			while (busy());

			while(1)
			{
				touchValue = memRead32(REG_TOUCH_DIRECT_XY); /* read for any new touch tag inputs */

				if(touch_lock)
				{
					if(touchValue & 0x80000000) /* check if we have no touch */
					{
						touch_lock = 0;
					}
				}
				else
				{
					if (!(touchValue & 0x80000000)) /* check if a touch is detected */
					{
						touchX[count] = (touchValue>>16) & 0x03FF; /* raw Touchscreen Y coordinate */
						touchY[count] = touchValue & 0x03FF; /* raw Touchscreen Y coordinate */
						touch_lock = 1;
						count++;
						break; /* leave while(1) */
					}
				}
			}
		}

		k = ((touchX[0] - touchX[2])*(touchY[1] - touchY[2])) - ((touchX[1] - touchX[2])*(touchY[0] - touchY[2]));

		tmp = (((displayX[0] - displayX[2]) * (touchY[1] - touchY[2])) - ((displayX[1] - displayX[2])*(touchY[0] - touchY[2])));
		TransMatrix[0] = ((int64_t)tmp << 16) / k;

		tmp = (((touchX[0] - touchX[2]) * (displayX[1] - displayX[2])) - ((displayX[0] - displayX[2])*(touchX[1] - touchX[2])));
		TransMatrix[1] = ((int64_t)tmp << 16) / k;

		tmp = ((touchY[0] * (((touchX[2] * displayX[1]) - (touchX[1] * displayX[2])))) + (touchY[1] * (((touchX[0] * displayX[2]) - (touchX[2] * displayX[0])))) + (touchY[2] * (((touchX[1] * displayX[0]) - (touchX[0] * displayX[1])))));
		TransMatrix[2] = ((int64_t)tmp << 16) / k;

		tmp = (((displayY[0] - displayY[2]) * (touchY[1] - touchY[2])) - ((displayY[1] - displayY[2])*(touchY[0] - touchY[2])));
		TransMatrix[3] = ((int64_t)tmp << 16) / k;

		tmp = (((touchX[0] - touchX[2]) * (displayY[1] - displayY[2])) - ((displayY[0] - displayY[2])*(touchX[1] - touchX[2])));
		TransMatrix[4] = ((int64_t)tmp << 16) / k;

		tmp = ((touchY[0] * (((touchX[2] * displayY[1]) - (touchX[1] * displayY[2])))) + (touchY[1] * (((touchX[0] * displayY[2]) - (touchX[2] * displayY[0])))) + (touchY[2] * (((touchX[1] * displayY[0]) - (touchX[0] * displayY[1])))));
		TransMatrix[5] = ((int64_t)tmp << 16) / k;

		memWrite32(REG_TOUCH_TRANSFORM_A, TransMatrix[0]);
		memWrite32(REG_TOUCH_TRANSFORM_B, TransMatrix[1]);
		memWrite32(REG_TOUCH_TRANSFORM_C, TransMatrix[2]);
		memWrite32(REG_TOUCH_TRANSFORM_D, TransMatrix[3]);
		memWrite32(REG_TOUCH_TRANSFORM_E, TransMatrix[4]);
		memWrite32(REG_TOUCH_TRANSFORM_F, TransMatrix[5]);
	}
}