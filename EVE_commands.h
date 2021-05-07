/*
@file    EVE_commands.h
@brief   contains FT8xx / BT8xx function prototypes
@version 5.0
@date    2020-04-19
@author  Rudolph Riedel

@section LICENSE

MIT License

Copyright (c) 2016-2021 Rudolph Riedel, modified by Linar Ismagilov

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once

#ifndef EVE_COMMANDS_H_
#define EVE_COMMANDS_H_

#include "EVE.h"

namespace EVE
{
	class Display
	{
	private:
	void private_string_write(const char *text);
	void block_transfer(const uint8_t *data, uint32_t len);
	void private_block_write(const uint8_t *data, uint16_t len);

	public:
	Port port;
	Display(){};
	
	volatile uint8_t cmd_burst = 0; /* flag to indicate cmd-burst is active */
	/*----------------------------------------------------------------------------------------------------------------------------*/
	/*---- helper functions ------------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------------------------------------*/
	void begin_cmd(uint32_t command);
	void cmdWrite(uint8_t command, uint8_t parameter);

	uint8_t  memRead8(uint32_t ftAddress);
	uint16_t memRead16(uint32_t ftAddress);
	uint32_t memRead32(uint32_t ftAddress);
	void memWrite8(uint32_t ftAddress, uint8_t ftData8);
	void memWrite16(uint32_t ftAddress, uint16_t ftData16);
	void memWrite32(uint32_t ftAddress, uint32_t ftData32);
	void memWrite_flash_buffer(uint32_t ftAddress, const uint8_t *data, uint32_t len);
	void memWrite_sram_buffer(uint32_t ftAddress, const uint8_t *data, uint32_t len);
	uint8_t  busy(void);
	void cmd_start(void);
	void cmd_execute(void);


	/*----------------------------------------------------------------------------------------------------------------------------*/
	/*---- commands and functions to be used outside of display-lists -------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------------------------------------*/


	/* EVE4: BT817 / BT818 */
	#if EVE_GEN > 3
	void cmd_flashprogram(uint32_t dest, uint32_t src, uint32_t num);
	void cmd_fontcache(uint32_t font, int32_t ptr, uint32_t num);
	void cmd_fontcachequery(uint32_t *total, int32_t *used);
	void cmd_getimage(uint32_t *source, uint32_t *fmt, uint32_t *width, uint32_t *height, uint32_t *palette);
	void cmd_linetime(uint32_t dest);
	uint32_t cmd_pclkfreq(uint32_t ftarget, int32_t rounding);
	void cmd_wait(uint32_t us);
	#endif /* EVE_GEN > 3 */

	/* EVE3: BT815 / BT816 */
	#if EVE_GEN > 2
	void cmd_clearcache(void);
	void cmd_flashattach(void);
	void cmd_flashdetach(void);
	void cmd_flasherase(void);
	uint32_t cmd_flashfast(void);
	void cmd_flashspidesel(void);
	void cmd_flashread(uint32_t dest, uint32_t src, uint32_t num);
	void cmd_flashsource(uint32_t ptr);
	void cmd_flashspirx(uint32_t dest, uint32_t num);
	void cmd_flashspitx(uint32_t num, const uint8_t *data);
	void cmd_flashupdate(uint32_t dest, uint32_t src, uint32_t num);
	void cmd_flashwrite(uint32_t ptr, uint32_t num, const uint8_t *data);
	void cmd_inflate2(uint32_t ptr, uint32_t options, const uint8_t *data, uint32_t len);
	#endif /* EVE_GEN > 2 */

	void cmd_getprops(uint32_t *pointer, uint32_t *width, uint32_t *height);
	uint32_t cmd_getptr(void);
	void cmd_inflate(uint32_t ptr, const uint8_t *data, uint32_t len);
	void cmd_interrupt(uint32_t ms);
	void cmd_loadimage(uint32_t ptr, uint32_t options, const uint8_t *data, uint32_t len);
	void cmd_mediafifo(uint32_t ptr, uint32_t size);
	void cmd_memcpy(uint32_t dest, uint32_t src, uint32_t num);
	uint32_t cmd_memcrc(uint32_t ptr, uint32_t num);
	void cmd_memset(uint32_t ptr, uint8_t value, uint32_t num);
	/*(void cmd_memwrite(uint32_t dest, uint32_t num, const uint8_t *data); */
	void cmd_memzero(uint32_t ptr, uint32_t num);
	void cmd_playvideo(uint32_t options, const uint8_t *data, uint32_t len);
	uint32_t cmd_regread(uint32_t ptr);
	void cmd_setrotate(uint32_t r);
	void cmd_snapshot(uint32_t ptr);
	void cmd_snapshot2(uint32_t fmt, uint32_t ptr, int16_t x0, int16_t y0, int16_t w0, int16_t h0);
	void cmd_track(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t tag);
	void cmd_videoframe(uint32_t dest, uint32_t result_ptr);

	/*----------------------------------------------------------------------------------------------------------------------------*/
	/*------------- patching and initialisation ----------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------------------------------------*/
	#if EVE_GEN > 2
	uint8_t  init_flash(void);
	#endif /* EVE_GEN > 2 */
	uint8_t  init(void);

	/*----------------------------------------------------------------------------------------------------------------------------*/
	/*-------- functions for display lists ---------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------------------------------------*/

	void start_cmd_burst(void);
	void end_cmd_burst(void);

	/* EVE4: BT817 / BT818 */
	#if EVE_GEN > 3

	void cmd_animframeram(int16_t x0, int16_t y0, uint32_t aoptr, uint32_t frame);
	void cmd_animframeram_burst(int16_t x0, int16_t y0, uint32_t aoptr, uint32_t frame);
	void cmd_animstartram(int32_t ch, uint32_t aoptr, uint32_t loop);
	void cmd_animstartram_burst(int32_t ch, uint32_t aoptr, uint32_t loop);
	void cmd_apilevel(uint32_t level);
	void cmd_apilevel_burst(uint32_t level);
	void cmd_calibratesub(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);
	void cmd_calllist(uint32_t adr);
	void cmd_calllist_burst(uint32_t adr);
	void cmd_hsf(uint32_t hsf);
	void cmd_hsf_burst(uint32_t hsf);
	void cmd_newlist(uint32_t adr);
	void cmd_newlist_burst(uint32_t adr);
	void cmd_runanim(uint32_t waitmask, uint32_t play);
	void cmd_runanim_burst(uint32_t waitmask, uint32_t play);


	#endif /* EVE_GEN > 3 */


	/* EVE3: BT815 / BT816 */
	#if EVE_GEN > 2

	void cmd_animdraw(int32_t ch);
	void cmd_animdraw_burst(int32_t ch);
	void cmd_animframe(int16_t x0, int16_t y0, uint32_t aoptr, uint32_t frame);
	void cmd_animframe_burst(int16_t x0, int16_t y0, uint32_t aoptr, uint32_t frame);
	void cmd_animstart(int32_t ch, uint32_t aoptr, uint32_t loop);
	void cmd_animstart_burst(int32_t ch, uint32_t aoptr, uint32_t loop);
	void cmd_animstop(int32_t ch);
	void cmd_animstop_burst(int32_t ch);
	void cmd_animxy(int32_t ch, int16_t x0, int16_t y0);
	void cmd_animxy_burst(int32_t ch, int16_t x0, int16_t y0);
	void cmd_appendf(uint32_t ptr, uint32_t num);
	void cmd_appendf_burst(uint32_t ptr, uint32_t num);
	uint16_t cmd_bitmap_transform( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t tx0, int32_t ty0, int32_t tx1, int32_t ty1, int32_t tx2, int32_t ty2);
	void cmd_bitmap_transform_burst( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t tx0, int32_t ty0, int32_t tx1, int32_t ty1, int32_t tx2, int32_t ty2);
	void cmd_fillwidth(uint32_t s);
	void cmd_fillwidth_burst(uint32_t s);
	void cmd_gradienta(int16_t x0, int16_t y0, uint32_t argb0, int16_t x1, int16_t y1, uint32_t argb1);
	void cmd_gradienta_burst(int16_t x0, int16_t y0, uint32_t argb0, int16_t x1, int16_t y1, uint32_t argb1);
	void cmd_rotatearound(int32_t x0, int32_t y0, int32_t angle, int32_t scale);
	void cmd_rotatearound_burst(int32_t x0, int32_t y0, int32_t angle, int32_t scale);

	void cmd_button_var(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text, uint8_t num_args, ...);
	void cmd_button_var_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text, uint8_t num_args, ...);
	void cmd_text_var(int16_t x0, int16_t y0, int16_t font, uint16_t options, const char* text, uint8_t numargs, ...);
	void cmd_text_var_burst(int16_t x0, int16_t y0, int16_t font, uint16_t options, const char* text, uint8_t numargs, ...);
	void cmd_toggle_var(int16_t x0, int16_t y0, int16_t w0, int16_t font, uint16_t options, uint16_t state, const char* text, uint8_t num_args, ...);
	void cmd_toggle_var_burst(int16_t x0, int16_t y0, int16_t w0, int16_t font, uint16_t options, uint16_t state, const char* text, uint8_t num_args, ...);

	#endif /* EVE_GEN > 2 */


	void cmd_dl(uint32_t command);
	void cmd_dl_burst(uint32_t command);

	void cmd_append(uint32_t ptr, uint32_t num);
	void cmd_append_burst(uint32_t ptr, uint32_t num);
	void cmd_bgcolor(uint32_t color);
	void cmd_bgcolor_burst(uint32_t color);
	void cmd_button(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text);
	void cmd_button_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text);
	void cmd_calibrate(void);
	void cmd_clock(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t hours, uint16_t minutes, uint16_t seconds, uint16_t millisecs);
	void cmd_clock_burst(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t hours, uint16_t minutes, uint16_t seconds, uint16_t millisecs);
	void cmd_dial(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t val);
	void cmd_dial_burst(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t val);
	void cmd_fgcolor(uint32_t color);
	void cmd_fgcolor_burst(uint32_t color);
	void cmd_gauge(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t major, uint16_t minor, uint16_t val, uint16_t range);
	void cmd_gauge_burst(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t major, uint16_t minor, uint16_t val, uint16_t range);
	void cmd_getmatrix(int32_t *get_a, int32_t *get_b, int32_t *get_c, int32_t *get_d, int32_t *get_e, int32_t *get_f);
	void cmd_gradcolor(uint32_t color);
	void cmd_gradcolor_burst(uint32_t color);
	void cmd_gradient(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1);
	void cmd_gradient_burst(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1);
	void cmd_keys(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text);
	void cmd_keys_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text);
	void cmd_number(int16_t x0, int16_t y0, int16_t font, uint16_t options, int32_t number);
	void cmd_number_burst(int16_t x0, int16_t y0, int16_t font, uint16_t options, int32_t number);
	void cmd_progress(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range);
	void cmd_progress_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range);
	void cmd_romfont(uint32_t font, uint32_t romslot);
	void cmd_romfont_burst(uint32_t font, uint32_t romslot);
	void cmd_rotate(int32_t angle);
	void cmd_rotate_burst(int32_t angle);
	void cmd_scale(int32_t sx, int32_t sy);
	void cmd_scale_burst(int32_t sx, int32_t sy);
	void cmd_scrollbar(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t size, uint16_t range);
	void cmd_scrollbar_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t size, uint16_t range);
	void cmd_setbase(uint32_t base);
	void cmd_setbase_burst(uint32_t base);
	void cmd_setbitmap(uint32_t addr, uint16_t fmt, uint16_t width, uint16_t height);
	void cmd_setbitmap_burst(uint32_t addr, uint16_t fmt, uint16_t width, uint16_t height);
	void cmd_setfont(uint32_t font, uint32_t ptr);
	void cmd_setfont_burst(uint32_t font, uint32_t ptr);
	void cmd_setfont2(uint32_t font, uint32_t ptr, uint32_t firstchar);
	void cmd_setfont2_burst(uint32_t font, uint32_t ptr, uint32_t firstchar);
	void cmd_setscratch(uint32_t handle);
	void cmd_setscratch_burst(uint32_t handle);
	void cmd_sketch(int16_t x0, int16_t y0, uint16_t w0, uint16_t h0, uint32_t ptr, uint16_t format);
	void cmd_sketch_burst(int16_t x0, int16_t y0, uint16_t w0, uint16_t h0, uint32_t ptr, uint16_t format);
	void cmd_slider(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range);
	void cmd_slider_burst(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range);
	void cmd_spinner(int16_t x0, int16_t y0, uint16_t style, uint16_t scale);
	void cmd_spinner_burst(int16_t x0, int16_t y0, uint16_t style, uint16_t scale);
	void cmd_text(int16_t x0, int16_t y0, int16_t font, uint16_t options, const char* text);
	void cmd_text_burst(int16_t x0, int16_t y0, int16_t font, uint16_t options, const char* text);
	void cmd_toggle(int16_t x0, int16_t y0, int16_t w0, int16_t font, uint16_t options, uint16_t state, const char* text);
	void cmd_toggle_burst(int16_t x0, int16_t y0, int16_t w0, int16_t font, uint16_t options, uint16_t state, const char* text);
	void cmd_translate(int32_t tx, int32_t ty);
	void cmd_translate_burst(int32_t tx, int32_t ty);

	void color_rgb(uint32_t color);
	void color_rgb_burst(uint32_t color);


	/*---------------------------------------------------------------------------------------------------------------------------*/
	/*-------- special purpose functions ------------------- --------------------------------------------------------------------*/
	/*---------------------------------------------------------------------------------------------------------------------------*/

	void calibrate_manual(uint16_t height);
	};
};
#endif /* EVE_COMMANDS_H_ */
