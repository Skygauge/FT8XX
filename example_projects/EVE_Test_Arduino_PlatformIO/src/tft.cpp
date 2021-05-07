/*
@file    tft.c / tft.cpp
@brief   TFT handling functions for EVE_Test project
@version 1.16
@date    2021-02-21
@author  Rudolph Riedel, modified by Linar Ismagilov
*/

#include "EVE.h"
#include "tft_data.h"

EVE::Display display;

/* some pre-definded colors */
#define RED		0xff0000UL
#define ORANGE	0xffa500UL
#define GREEN	0x00ff00UL
#define BLUE	0x0000ffUL
#define BLUE_1	0x5dade2L
#define YELLOW	0xffff00UL
#define PINK	0xff00ffUL
#define PURPLE	0x800080UL
#define WHITE	0xffffffUL
#define BLACK	0x000000UL

/* memory-map defines */
#define MEM_FONT 0x000f6000
#define MEM_LOGO 0x000f8000 /* start-address of logo, needs 6272 bytes of memory */
#define MEM_PIC1 0x000fa000 /* start of 100x100 pixel test image, ARGB565, needs 20000 bytes of memory */


#define MEM_DL_STATIC (EVE_RAM_G_SIZE - 4096) /* 0xff000 - start-address of the static part of the display-list, upper 4k of gfx-mem */

uint32_t num_dl_static; /* amount of bytes in the static part of our display-list */
uint8_t tft_active = 0;
uint16_t num_profile_a, num_profile_b;

#define LAYOUT_Y1 24


void initStaticBackground(void)
{
	display.cmd_dl(CMD_DLSTART); /* Start the display list */

	display.cmd_dl(TAG(0)); /* do not use the following objects for touch-detection */

	display.cmd_bgcolor(0); /* light grey */

	display.cmd_dl(VERTEX_FORMAT(0)); /* reduce precision for VERTEX2F to 1 pixel instead of 1/16 pixel default */

	/* draw a rectangle on top */
	display.cmd_dl(DL_BEGIN | EVE_RECTS);
	display.cmd_dl(LINE_WIDTH(1*16)); /* size is in 1/16 pixel */

	display.cmd_dl(DL_COLOR_RGB | BLUE_1);
	display.cmd_dl(VERTEX2F(0,0));
	display.cmd_dl(VERTEX2F(EVE_HSIZE,LAYOUT_Y1-2));
	display.cmd_dl(DL_END);

	/* display the logo */
	display.cmd_dl(DL_COLOR_RGB);
	display.cmd_dl(DL_BEGIN | EVE_BITMAPS);
	display.cmd_setbitmap(MEM_LOGO, EVE_ARGB1555, 56, 56);
	display.cmd_dl(VERTEX2F(EVE_HSIZE - 58, 5));
	display.cmd_dl(DL_END);

	/* draw a black line to separate things */
	display.cmd_dl(DL_COLOR_RGB | BLACK);
	display.cmd_dl(DL_BEGIN | EVE_LINES);
	display.cmd_dl(VERTEX2F(0,LAYOUT_Y1-2));
	display.cmd_dl(VERTEX2F(EVE_HSIZE,LAYOUT_Y1-2));
	display.cmd_dl(DL_END);

	/* add the static text to the list */
#if defined (EVE_DMA)
	display.cmd_text(10, EVE_VSIZE - 65, 26, 0, "Bytes:");
#endif
	display.cmd_text(10, EVE_VSIZE - 50, 26, 0, "DL-size:");
	display.cmd_text(10, EVE_VSIZE - 35, 26, 0, "Time1:");
	display.cmd_text(10, EVE_VSIZE - 20, 26, 0, "Time2:");

	display.cmd_text(125, EVE_VSIZE - 35, 26, 0, "us");
	display.cmd_text(125, EVE_VSIZE - 20, 26, 0, "us");

	while (display.busy());

	num_dl_static = display.memRead16(REG_CMD_DL);

	display.cmd_memcpy(MEM_DL_STATIC, EVE_RAM_DL, num_dl_static);
	while (display.busy());
}


void TFT_init(void)
{
	if(display.init() != 0)
	{
		Serial.println("EVE Init success!");
		tft_active = 1;

		display.memWrite8(REG_PWM_DUTY, 0xFF);	/* setup backlight, range is from 0 = off to 0x80 = max */
		//touch_calibrate();

		display.cmd_inflate(MEM_LOGO, logo, sizeof(logo)); /* load logo into gfx-memory and de-compress it */
		display.cmd_loadimage(MEM_PIC1, EVE_OPT_NODL, pic, sizeof(pic));

		initStaticBackground();

		Serial.println("Display Init");
	}
}


uint16_t toggle_state = 0;
uint16_t display_list_size = 0;

/* check for touch events and setup vars for TFT_display() */
void TFT_touch(void)
{
	uint8_t tag;
	static uint8_t toggle_lock = 0;
	
	if(display.busy()) /* is EVE still processing the last display list? */
	{
		return;
	}

	display_list_size = display.memRead16(REG_CMD_DL); /* debug-information, get the size of the last generated display-list */

	tag = display.memRead8(REG_TOUCH_TAG); /* read the value for the first touch point */

	switch(tag)
	{
		case 0:
			toggle_lock = 0;
			break;

		case 10: /* use button on top as on/off radio-switch */
			if(toggle_lock == 0)
			{
				toggle_lock = 42;
				if(toggle_state == 0)
				{
					toggle_state = EVE_OPT_FLAT;
				}
				else
				{
					toggle_state = 0;
				}
			}
			break;
	}
}


/*
	dynamic portion of display-handling, meant to be called every 20ms or more
*/
void TFT_display(void)
{
	static int32_t rotate = 0;
	

	if(tft_active != 0)
	{
		#if defined (EVE_DMA)
			uint16_t cmd_fifo_size;
			cmd_fifo_size = display.port.dma_buffer_index*4; /* without DMA there is no way to tell how many bytes are written to the cmd-fifo */
		#endif

		display.start_cmd_burst(); /* start writing to the cmd-fifo as one stream of bytes, only sending the address once */

		display.cmd_dl_burst(CMD_DLSTART); /* start the display list */
		display.cmd_dl_burst(DL_CLEAR_RGB | WHITE); /* set the default clear color to white */
		display.cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG); /* clear the screen - this and the previous prevent artifacts between lists, Attributes are the color, stencil and tag buffers */
		display.cmd_dl_burst(TAG(0));

		display.cmd_append_burst(MEM_DL_STATIC, num_dl_static); /* insert static part of display-list from copy in gfx-mem */
		/* display a button */
		display.cmd_dl_burst(DL_COLOR_RGB | WHITE);
		display.cmd_fgcolor_burst(0x00c0c0c0); /* some grey */
		display.cmd_dl_burst(TAG(10)); /* assign tag-value '10' to the button that follows */
		display.cmd_button_burst(20,20,80,30, 28, toggle_state,"Touch!");
		display.cmd_dl_burst(TAG(0)); /* no touch */

		/* display a picture and rotate it when the button on top is activated */
		display.cmd_setbitmap_burst(MEM_PIC1, EVE_RGB565, 100, 100);

		display.cmd_dl_burst(CMD_LOADIDENTITY);
		display.cmd_translate_burst(65536 * 70, 65536 * 50); /* shift off-center */
		display.cmd_rotate_burst(rotate);
		display.cmd_translate_burst(65536 * -70, 65536 * -50); /* shift back */
		display.cmd_dl_burst(CMD_SETMATRIX);

		if(toggle_state != 0)
		{
			rotate += 256;
		}

		display.cmd_dl_burst(DL_BEGIN | EVE_BITMAPS);
		display.cmd_dl_burst(VERTEX2F(EVE_HSIZE - 100, (LAYOUT_Y1)));
		display.cmd_dl_burst(DL_END);

		display.cmd_dl_burst(RESTORE_CONTEXT()); /* reset the transformation matrix to default values */

		/* print profiling values */
		display.cmd_dl_burst(DL_COLOR_RGB | BLACK);

		#if defined (EVE_DMA)
		display.cmd_number_burst(120, EVE_VSIZE - 65, 26, EVE_OPT_RIGHTX, cmd_fifo_size); /* number of bytes written to the cmd-fifo */
		#endif
		display.cmd_number_burst(120, EVE_VSIZE - 50, 26, EVE_OPT_RIGHTX, display_list_size); /* number of bytes written to the display-list by the command co-pro */
		display.cmd_number_burst(120, EVE_VSIZE - 35, 26, EVE_OPT_RIGHTX|5, num_profile_a); /* duration in �s of TFT_loop() for the touch-event part */
		display.cmd_number_burst(120, EVE_VSIZE - 20, 26, EVE_OPT_RIGHTX|5, num_profile_b); /* duration in �s of TFT_loop() for the display-list part */

		display.cmd_dl_burst(DL_DISPLAY); /* instruct the graphics processor to show the list */
		display.cmd_dl_burst(CMD_SWAP); /* make this list active */

		display.end_cmd_burst(); /* stop writing to the cmd-fifo, the cmd-FIFO will be executed automatically after this or when DMA is done */
	}
}
