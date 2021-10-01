#include "EVE_commands.h"

#include <stdarg.h>

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
            initialized = false;
            port.pdn_set();
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

		if( ((options & EVE_OPT_MEDIAFIFO) == 0) && ((options & EVE_OPT_FLASH) == 0) )/* direct data, neither by Media-FIFO or from Flash */
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

		if( ((options & EVE_OPT_MEDIAFIFO) == 0) && ((options & EVE_OPT_FLASH) == 0) )/* direct data, neither by Media-FIFO or from Flash */
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

	/* switch the FLASH attached to a BT815/BT816 to full-speed mode, returns 0 for failing to do so */
	uint8_t Display::init_flash(void)
	{
		uint8_t timeout = 0;
		uint8_t status;

		status = memRead8(REG_FLASH_STATUS); /* should be 0x02 - FLASH_STATUS_BASIC, power-up is done and the attached flash is detected */

		while(status == 0) /* FLASH_STATUS_INIT - we are somehow still in init, give it a litte more time, this should never happen */
		{
			status = memRead8(REG_FLASH_STATUS);
			delay(1);
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


    bool Display::is_initialized()
    {
        return initialized;
    }

	/* init, has to be executed with the SPI setup to 11 MHz or less as required by FT8xx / BT8xx */
	uint8_t Display::init(const uint * TouchCalibration)
	{
        if(initialized) return false;

        port.init();
        delay(3);
		#if defined (EVE_HAS_CRYSTAL)
		cmdWrite(EVE_CLKEXT,0);	/* setup EVE for external clock */
		#else
		cmdWrite(EVE_CLKINT,0);	/* setup EVE for internal clock */
		#endif
		cmdWrite(EVE_CLKSEL,0x46); /* set clock to 72 MHz */

		cmdWrite(EVE_ACTIVE,0);	/* start EVE */

        elapsedMicros timeout = 0;
		while(memRead8(REG_ID) != 0x7C) /* if chipid is not 0x7c, continue to read it until it is, EVE needs a moment for it's power on self-test and configuration */
		{			
            if(timeout > 50000) {
                Serial.printf("display init timeout on id, got %u!\r\n");
                port.pdn_set();
                return false;
            }
		}

        timeout = 0;
		while (0x00 != (memRead8(REG_CPURESET) & 0x03)) /* check if EVE is in working status */
		{
            if(timeout > 50000) {
                Serial.println("display init timeout on reset!");
                port.pdn_set();
                return false;
            }
		}

		/* tell EVE that we changed the frequency from default to 72MHz for BT8xx */
		memWrite32(REG_FREQUENCY, 72000000);
		
        port.set_speed();

		/* we have a display with a Goodix GT911 / GT9271 touch-controller on it, so we patch our FT811 or FT813 according to AN_336 or setup a BT815 accordingly */
		#if defined (EVE_HAS_GT911)		
        memWrite16(REG_TOUCH_CONFIG, 0x05d0); /* switch to Goodix touch controller */		
		#endif
		
        memWrite16(REG_GPIOX, 0x800C);
		memWrite16(REG_GPIOX_DIR,0x800C); /* setting GPIO3 back to input, GPIO2 to output */
		
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

		/* configure Touch */
		memWrite8(REG_TOUCH_MODE, EVE_TMODE_CONTINUOUS); /* enable touch */
		memWrite16(REG_TOUCH_RZTHRESH, EVE_TOUCH_RZTHRESH);	/* eliminate any false touches */

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

        memWrite32(REG_TOUCH_TRANSFORM_A, TouchCalibration[0]);
        memWrite32(REG_TOUCH_TRANSFORM_B, TouchCalibration[1]);
        memWrite32(REG_TOUCH_TRANSFORM_C, TouchCalibration[2]);
        memWrite32(REG_TOUCH_TRANSFORM_D, TouchCalibration[3]);
        memWrite32(REG_TOUCH_TRANSFORM_E, TouchCalibration[4]);
        memWrite32(REG_TOUCH_TRANSFORM_F, TouchCalibration[5]);

        memWrite8(REG_GPIO, 0x80); /* enable the DISP signal to the LCD panel, it is set to output in REG_GPIO_DIR by default */
        memWrite8(REG_PCLK, EVE_PCLK); /* now start clocking data to the LCD panel */
        
        initialized = true;

		return initialized;
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

	void Display::cmd_animdraw_burst(int32_t ch)
	{
		port.transmit_burst(CMD_ANIMDRAW);
		port.transmit_burst(ch);
	}

	void Display::cmd_animframe_burst(int16_t x0, int16_t y0, uint32_t aoptr, uint32_t frame)
	{
		port.transmit_burst(CMD_ANIMFRAME);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst(aoptr);
		port.transmit_burst(frame);
	}

	void Display::cmd_animstart_burst(int32_t ch, uint32_t aoptr, uint32_t loop)
	{
		port.transmit_burst(CMD_ANIMSTART);
		port.transmit_burst(ch);
		port.transmit_burst(aoptr);
		port.transmit_burst(loop);
	}

	void Display::cmd_animstop_burst(int32_t ch)
	{
		port.transmit_burst(CMD_ANIMSTOP);
		port.transmit_burst(ch);
	}

	void Display::cmd_animxy_burst(int32_t ch, int16_t x0, int16_t y0)
	{
		port.transmit_burst(CMD_ANIMXY);
		port.transmit_burst(ch);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
	}

	void Display::cmd_appendf_burst(uint32_t ptr, uint32_t num)
	{
		port.transmit_burst(CMD_APPENDF);
		port.transmit_burst(ptr);
		port.transmit_burst(num);
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

	void Display::cmd_fillwidth_burst(uint32_t s)
	{
		port.transmit_burst(CMD_FILLWIDTH);
		port.transmit_burst(s);
	}

	void Display::cmd_gradienta_burst(int16_t x0, int16_t y0, uint32_t argb0, int16_t x1, int16_t y1, uint32_t argb1)
	{
		port.transmit_burst(CMD_GRADIENTA);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst(argb0);
		port.transmit_burst((uint32_t) x1 + ((uint32_t) y1 << 16));
		port.transmit_burst(argb1);
	}

	void Display::cmd_rotatearound_burst(int32_t x0, int32_t y0, int32_t angle, int32_t scale)
	{
		port.transmit_burst(CMD_ROTATEAROUND);
		port.transmit_burst(x0);
		port.transmit_burst(y0);
		port.transmit_burst(angle);
		port.transmit_burst(scale);
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

	void Display::cmd_text_var_burst(int16_t x0, int16_t y0, int16_t font, uint16_t options, const char* text, uint8_t num_args, ...)
	{
		port.transmit_burst(CMD_TEXT);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
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

	void Display::cmd_append_burst(uint32_t ptr, uint32_t num)
	{
		port.transmit_burst(CMD_APPEND);
		port.transmit_burst(ptr);
		port.transmit_burst(num);
	}

	void Display::cmd_bgcolor_burst(uint32_t color)
	{
		port.transmit_burst(CMD_BGCOLOR);
		port.transmit_burst(color);
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

	void Display::cmd_clock_burst(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t hours, uint16_t minutes, uint16_t seconds, uint16_t millisecs)
	{
		port.transmit_burst(CMD_CLOCK);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) r0 + ((uint32_t) options << 16));
		port.transmit_burst((uint32_t) hours + ((uint32_t) minutes << 16));
		port.transmit_burst((uint32_t) seconds + ((uint32_t) millisecs << 16));
	}

	void Display::cmd_dial_burst(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t val)
	{
		port.transmit_burst(CMD_DIAL);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) r0 + ((uint32_t) options << 16));
		port.transmit_burst(val);
	}

	void Display::cmd_fgcolor_burst(uint32_t color)
	{
		port.transmit_burst(CMD_FGCOLOR);
		port.transmit_burst(color);
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

	void Display::cmd_gradcolor_burst(uint32_t color)
	{
		port.transmit_burst(CMD_GRADCOLOR);
		port.transmit_burst(color);
	}

	void Display::cmd_gradient_burst(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1)
	{
		port.transmit_burst(CMD_GRADIENT);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst(rgb0);
		port.transmit_burst((uint32_t) x1 + ((uint32_t) y1 << 16));
		port.transmit_burst(rgb1);
	}

	void Display::cmd_keys_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text)
	{
		port.transmit_burst(CMD_KEYS);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst((uint32_t) font + ((uint32_t) options << 16));

		private_string_write(text);
	}

	void Display::cmd_number_burst(int16_t x0, int16_t y0, int16_t font, uint16_t options, int32_t number)
	{
		port.transmit_burst(CMD_NUMBER);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) font + ((uint32_t) options << 16));
		port.transmit_burst(number);
	}


	void Display::cmd_progress_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range)
	{
		port.transmit_burst(CMD_PROGRESS);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst((uint32_t) options + ((uint32_t) val << 16));
		port.transmit_burst(range);
	}

	void Display::cmd_romfont_burst(uint32_t font, uint32_t romslot)
	{
		port.transmit_burst(CMD_ROMFONT);
		port.transmit_burst(font);
		port.transmit_burst(romslot);
	}

	void Display::cmd_rotate_burst(int32_t angle)
	{
		port.transmit_burst(CMD_ROTATE);
		port.transmit_burst(angle);
	}

	void Display::cmd_scale_burst(int32_t sx, int32_t sy)
	{
		port.transmit_burst(CMD_SCALE);
		port.transmit_burst(sx);
		port.transmit_burst(sy);
	}

	void Display::cmd_scrollbar_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t size, uint16_t range)
	{
		port.transmit_burst(CMD_SCROLLBAR);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst((uint32_t) options + ((uint32_t) val << 16));
		port.transmit_burst((uint32_t) size + ((uint32_t) range << 16));
	}

	void Display::cmd_setbase_burst(uint32_t base)
	{
		port.transmit_burst(CMD_SETBASE);
		port.transmit_burst(base);
	}

	void Display::cmd_setbitmap_burst(uint32_t addr, uint16_t fmt, uint16_t width, uint16_t height)
	{
		port.transmit_burst(CMD_SETBITMAP);
		port.transmit_burst(addr);
		port.transmit_burst((uint32_t) fmt + ((uint32_t) width << 16));
		port.transmit_burst(height);
	}

	void Display::cmd_setfont_burst(uint32_t font, uint32_t ptr)
	{
		port.transmit_burst(CMD_SETFONT);
		port.transmit_burst(font);
		port.transmit_burst(ptr);
	}

	void Display::cmd_setfont2_burst(uint32_t font, uint32_t ptr, uint32_t firstchar)
	{
		port.transmit_burst(CMD_SETFONT2);
		port.transmit_burst(font);
		port.transmit_burst(ptr);
		port.transmit_burst(firstchar);
	}

	void Display::cmd_setscratch_burst(uint32_t handle)
	{
		port.transmit_burst(CMD_SETSCRATCH);
		port.transmit_burst(handle);
	}

	void Display::cmd_sketch_burst(int16_t x0, int16_t y0, uint16_t w0, uint16_t h0, uint32_t ptr, uint16_t format)
	{
		port.transmit_burst(CMD_SKETCH);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst(ptr);
		port.transmit_burst(format);
	}

	void Display::cmd_slider_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range)
	{
		port.transmit_burst(CMD_SLIDER);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) h0 << 16));
		port.transmit_burst((uint32_t) options + ((uint32_t) val << 16));
		port.transmit_burst(range);
	}

	void Display::cmd_spinner_burst(int16_t x0, int16_t y0, uint16_t style, uint16_t scale)
	{
		port.transmit_burst(CMD_SPINNER);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) style + ((uint32_t) scale << 16));
	}

	void Display::cmd_text_burst(int16_t x0, int16_t y0, int16_t font, uint16_t options, const char* text)
	{
		port.transmit_burst(CMD_TEXT);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) font + ((uint32_t) options << 16));
		private_string_write(text);
	}

	void Display::cmd_toggle_burst(int16_t x0, int16_t y0, int16_t w0, int16_t font, uint16_t options, uint16_t state, const char* text)
	{
		port.transmit_burst(CMD_TOGGLE);
		port.transmit_burst((uint32_t) x0 + ((uint32_t) y0 << 16));
		port.transmit_burst((uint32_t) w0 + ((uint32_t) font << 16));
		port.transmit_burst((uint32_t) options + ((uint32_t) state << 16));
		private_string_write(text);
	}

	void Display::cmd_translate_burst(int32_t tx, int32_t ty)
	{
		port.transmit_burst(CMD_TRANSLATE);
		port.transmit_burst(tx);
		port.transmit_burst(ty);
	}

	void Display::color_rgb_burst(uint32_t color)
	{
		port.transmit_burst(DL_COLOR_RGB | color);
	}
}