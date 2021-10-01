/*
@file    EVE_config.h
@brief   configuration information for some TFTs
@version 5.0
@date    2021-04-04
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

*/

#pragma once

#ifndef EVE_CONFIG_H_
#define EVE_CONFIG_H_


/* select the settings for the TFT attached */
#define EVE_EVE3_43G

/* display timing parameters below */

/* untested */
/* EVE3-35A 320x240 3.5" Matrix Orbital, resistive, or non-touch, BT816 */
#if defined (EVE_EVE3_35)
#define Resolution_320x240

#define EVE_PCLK	(11L)
#define EVE_PCLKPOL	(0L)
#define EVE_SWIZZLE	(0L)
#define EVE_CSPREAD	(1L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#endif


/* EVE3-35G 320x240 3.5" Matrix Orbital, capacitive-touch, BT815 */
#if defined (EVE_EVE3_35G)
#define Resolution_320x240

#define EVE_PCLK	(11L)
#define EVE_PCLKPOL	(0L)
#define EVE_SWIZZLE	(0L)
#define EVE_CSPREAD	(1L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#define EVE_HAS_GT911	/* special treatment required for out-of-spec touch-controller */
#endif


/* EVE3-39G 480x128 3.9" Matrix Orbital, capacitive-touch, BT815 */
#if defined (EVE_EVE3_39G)
#define EVE_HSIZE	(480L)	/* Thd Length of visible part of line (in PCLKs) - display width */
#define EVE_VSIZE	(128L)	/* Tvd Number of visible lines (in lines) - display height */

#define EVE_VSYNC0	(8L)	/* Tvf Vertical Front Porch */
#define EVE_VSYNC1	(11L)	/* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define EVE_VOFFSET	(11L)	/* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define EVE_VCYCLE	(288L)	/* Tv Total number of lines (visible and non-visible) (in lines) */
#define EVE_HSYNC0	(28L)	 /* (40L)	// Thf Horizontal Front Porch */
#define EVE_HSYNC1	(44L)	/* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define EVE_HOFFSET	(44L)	/* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define EVE_HCYCLE 	(524L)	/* Th Total length of line (visible and non-visible) (in PCLKs) */

#define EVE_PCLK	(5L)
#define EVE_PCLKPOL	(1L)
#define EVE_SWIZZLE	(0L)
#define EVE_CSPREAD	(1L)
#define EVE_TOUCH_RZTHRESH (3200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#define EVE_HAS_GT911	/* special treatment required for out-of-spec touch-controller */
#endif


/* untested */
/* RVT35HHBxxxxx 320x240 3.5" Riverdi, various options, BT817 */
#if defined (EVE_RVT35H)
#define EVE_HSIZE	(320L)
#define EVE_VSIZE	(240L)

#define EVE_VSYNC0	(0L)
#define EVE_VSYNC1	(4L)
#define EVE_VOFFSET	(12L)
#define EVE_VCYCLE	(260L)
#define EVE_HSYNC0	(0L)
#define EVE_HSYNC1	(4L)
#define EVE_HOFFSET (43L)
#define EVE_HCYCLE 	(371L)
#define EVE_PCLK	(12L)
#define EVE_PCLKPOL (1L)
#define EVE_SWIZZLE (0L)
#define EVE_CSPREAD	(0L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 4
#endif


/* untested */
/* EVE3-43A 480x272 4.3" Matrix Orbital, resistive, or non-touch, BT816 */
#if defined (EVE_EVE3_43)
#define Resolution_480x272

#define EVE_PCLK	(7L)
#define EVE_PCLKPOL	(1L)
#define EVE_SWIZZLE	(0L)
#define EVE_CSPREAD	(1L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#endif


/* EVE3-43G 480x272 4.3" Matrix Orbital, capacitive-touch, BT815 */
#if defined (EVE_EVE3_43G)
#define Resolution_480x272

#define EVE_PCLK	(5L)
#define EVE_PCLKPOL	(1L)
#define EVE_SWIZZLE	(0L)
#define EVE_CSPREAD	(1L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#define EVE_HAS_GT911
#endif


/* 480x272 4.3" Riverdi, various options, BT815/BT816 */
#if defined (EVE_RiTFT43)
#define Resolution_480x272

#define EVE_PCLK	(7L)
#define EVE_PCLKPOL	(1L)
#define EVE_SWIZZLE	(0L)
#define EVE_CSPREAD	(1L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#endif


/* untested */
/* RVT43HLBxxxxx 480x272 4.3" Riverdi, various options, BT817 */
#if defined (EVE_RVT43H)
#define EVE_HSIZE	(480L)
#define EVE_VSIZE	(272L)

#define EVE_VSYNC0	(0L)
#define EVE_VSYNC1	(4L)
#define EVE_VOFFSET	(12L)
#define EVE_VCYCLE	(292L)
#define EVE_HSYNC0	(0L)
#define EVE_HSYNC1	(4L)
#define EVE_HOFFSET (43L)
#define EVE_HCYCLE 	(531L)
#define EVE_PCLK	(7L)
#define EVE_PCLKPOL (1L)
#define EVE_SWIZZLE (0L)
#define EVE_CSPREAD	(0L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 4
#endif


/* untested */ /* Matrix Orbital EVE3 modules EVE3-50A, EVE3-70A : 800x480 5.0" and 7.0" resistive, or no touch, BT816 */
/* PAF90B5WFNWC01 800x480 9.0" Panasys, BT815 */
#if defined (EVE_EVE3_50) || defined (EVE_EVE3_70) || defined (EVE_PAF90)
#define Resolution_800x480

#define EVE_PCLK	(2L)
#define EVE_PCLKPOL	(1L)
#define EVE_SWIZZLE	(0L)
#define EVE_CSPREAD	(0L)
#define EVE_TOUCH_RZTHRESH (1600L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#endif


/* Matrix Orbital EVE3 modules EVE3-50G, EVE3-70G : 800x480 5.0" and 7.0" capacitive touch, BT815 */
#if defined (EVE_EVE3_50G) || defined (EVE_EVE3_70G)
#define Resolution_800x480

#define EVE_PCLK	(2L)
#define EVE_PCLKPOL	(1L)
#define EVE_SWIZZLE	(0L)
#define EVE_CSPREAD	(0L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#define EVE_HAS_GT911
#endif


/* untested */
/* Bridgtek 800x480 5.0" BT816 */
#if defined (EVE_VM816C50AD)
#define Resolution_800x480

#define EVE_PCLK	(2L)
#define EVE_PCLKPOL	(1L)
#define EVE_SWIZZLE	(0L)
#define EVE_CSPREAD	(0L)
#define EVE_TOUCH_RZTHRESH (1800L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#endif



/* untested */
/* RVT50xQBxxxxx 800x480 5.0" Riverdi, various options, BT815/BT816 */
/* RVT70xQBxxxxx 800x480 7.0" Riverdi, various options, BT815/BT816 */
#if defined (EVE_RiTFT70) || defined (EVE_RiTFT50)
#define EVE_HSIZE	(800L)	/* Thd Length of visible part of line (in PCLKs) - display width */
#define EVE_VSIZE	(480L)	/* Tvd Number of visible lines (in lines) - display height */

#define EVE_VSYNC0	(0L)	/* Tvf Vertical Front Porch */
#define EVE_VSYNC1	(10L)	/* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define EVE_VOFFSET	(23L)	/* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define EVE_VCYCLE	(525L)	/* Tv Total number of lines (visible and non-visible) (in lines) */
#define EVE_HSYNC0	(0L)	/* Thf Horizontal Front Porch */
#define EVE_HSYNC1	(10L)	/* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define EVE_HOFFSET (46L)	/* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define EVE_HCYCLE 	(1056L)	/* Th Total length of line (visible and non-visible) (in PCLKs) */
#define EVE_PCLK	(2L)	/* 72MHz / REG_PCLK = PCLK frequency 30 MHz */
#define EVE_PCLKPOL (1L)	/* PCLK polarity (0 = rising edge, 1 = falling edge) */
#define EVE_SWIZZLE (0L)	/* Defines the arrangement of the RGB pins of the FT800 */
#define EVE_CSPREAD	(1L)
#define EVE_TOUCH_RZTHRESH (1800L)	/* touch-sensitivity */
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#endif

/* untested */
/* RVT50HQBxxxxx 800x480 5.0" Riverdi, various options, BT817 */
#if defined (EVE_RVT50H)
#define EVE_HSIZE	(800L)
#define EVE_VSIZE	(480L)

#define EVE_VSYNC0	(0L)
#define EVE_VSYNC1	(4L)
#define EVE_VOFFSET	(8L)
#define EVE_VCYCLE	(496L)
#define EVE_HSYNC0	(0L)
#define EVE_HSYNC1	(4L)
#define EVE_HOFFSET (8L)
#define EVE_HCYCLE 	(816L)
#define EVE_PCLK	(3L)
#define EVE_PCLKPOL (1L)
#define EVE_SWIZZLE (0L)
#define EVE_CSPREAD	(0L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 4
#endif


/* untested */
/* RVT70HSBxxxxx 1024x600 7.0" Riverdi, various options, BT817 */
#if defined (EVE_RVT70H)
#define EVE_HSIZE	(1024L)
#define EVE_VSIZE	(600L)

#define EVE_VSYNC0	(0L)
#define EVE_VSYNC1	(10L)
#define EVE_VOFFSET	(23L)
#define EVE_VCYCLE	(635L)
#define EVE_HSYNC0	(0L)
#define EVE_HSYNC1	(70L)
#define EVE_HOFFSET (160L)
#define EVE_HCYCLE 	(1344L)
#define EVE_PCLK	(1L) /* 1 = use second PLL for pixel-clock in BT817 / BT818 */
#define EVE_PCLK_FREQ (51000000L) /* EVE_PCLK needs to be set to 1 for this to take effect */
#define EVE_PCLKPOL (1L)
#define EVE_SWIZZLE (0L)
#define EVE_CSPREAD	(0L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 4
#endif


#if defined (BT817_TEST1)
#define EVE_HSIZE	(1024L)	/* Thd Length of visible part of line (in PCLKs) - display width */
#define EVE_VSIZE	(600L)	/* Tvd Number of visible lines (in lines) - display height */

#define EVE_VSYNC0	(1L)	/* Tvf Vertical Front Porch */
#define EVE_VSYNC1	(2L)	/* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define EVE_VOFFSET	(25L)	/* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define EVE_VCYCLE	(626L)	/* Tv Total number of lines (visible and non-visible) (in lines) */
#define EVE_HSYNC0	(16L)	/* Thf Horizontal Front Porch */
#define EVE_HSYNC1	(17L)	/* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define EVE_HOFFSET (177L)	/* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define EVE_HCYCLE 	(1597L)	/* Th Total length of line (visible and non-visible) (in PCLKs) */
#define EVE_PCLKPOL (1L)	/* PCLK polarity (0 = rising edge, 1 = falling edge) */
#define EVE_SWIZZLE (3L)	/* Defines the arrangement of the RGB pins */
#define EVE_PCLK	(1L)	/* 1 = use second PLL for pixel-clock in BT817 / BT818 */
#define EVE_CSPREAD	(0L)	/* helps with noise, when set to 1 fewer signals are changed simultaneously, reset-default: 1 */
#define EVE_TOUCH_RZTHRESH (1200L)	/* touch-sensitivity */
#define EVE_HAS_CRYSTAL
#define EVE_GEN 4
//#define EVE_PCLK_FREQ (51000000L)	/* 51MHz - value for EVE_cmd_pclkfreq */
#endif


#if defined (BT817_TEST2)
#define EVE_HSIZE	(1024L)	/* Thd Length of visible part of line (in PCLKs) - display width */
#define EVE_VSIZE	(600L)	/* Tvd Number of visible lines (in lines) - display height */

#define EVE_VSYNC0	(1L)	/* Tvf Vertical Front Porch */
#define EVE_VSYNC1	(2L)	/* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define EVE_VOFFSET	(25L)	/* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define EVE_VCYCLE	(626L)	/* Tv Total number of lines (visible and non-visible) (in lines) */
#define EVE_HSYNC0	(16L)	/* Thf Horizontal Front Porch */
#define EVE_HSYNC1	(17L)	/* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define EVE_HOFFSET (177L)	/* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define EVE_HCYCLE 	(1597L)	/* Th Total length of line (visible and non-visible) (in PCLKs) */
#define EVE_PCLKPOL (1L)	/* PCLK polarity (0 = rising edge, 1 = falling edge) */
#define EVE_SWIZZLE (3L)	/* Defines the arrangement of the RGB pins */
#define EVE_PCLK	(1L)	/* 1 = use second PLL for pixel-clock in BT817 / BT818 */
#define EVE_CSPREAD	(0L)	/* helps with noise, when set to 1 fewer signals are changed simultaneously, reset-default: 1 */
#define EVE_TOUCH_RZTHRESH (1200L)	/* touch-sensitivity */
#define EVE_HAS_CRYSTAL
#define EVE_GEN 4
#define EVE_PCLK_FREQ (51000000L)	/* 51MHz - value for EVE_cmd_pclkfreq */
#endif


#if defined (BT817_TEST3)
#define EVE_HSIZE	(1024L)	/* Thd Length of visible part of line (in PCLKs) - display width */
#define EVE_VSIZE	(600L)	/* Tvd Number of visible lines (in lines) - display height */

#define EVE_VSYNC0	(1L)	/* Tvf Vertical Front Porch */
#define EVE_VSYNC1	(2L)	/* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define EVE_VOFFSET	(25L)	/* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define EVE_VCYCLE	(626L)	/* Tv Total number of lines (visible and non-visible) (in lines) */
#define EVE_HSYNC0	(16L)	/* Thf Horizontal Front Porch */
#define EVE_HSYNC1	(17L)	/* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define EVE_HOFFSET (177L)	/* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define EVE_HCYCLE 	(1344L)	/* Th Total length of line (visible and non-visible) (in PCLKs) */
#define EVE_PCLKPOL (1L)	/* PCLK polarity (0 = rising edge, 1 = falling edge) */
#define EVE_SWIZZLE (3L)	/* Defines the arrangement of the RGB pins */
#define EVE_PCLK	(1L)	/* 1 = use second PLL for pixel-clock in BT817 / BT818 */
#define EVE_CSPREAD	(0L)	/* helps with noise, when set to 1 fewer signals are changed simultaneously, reset-default: 1 */
#define EVE_TOUCH_RZTHRESH (1200L)	/* touch-sensitivity */
#define EVE_HAS_CRYSTAL
#define EVE_GEN 4
#define EVE_PCLK_FREQ (51000000L)	/* 51MHz - value for EVE_cmd_pclkfreq */
#endif

/* ----------- 1280 x 800 ------------ */

/* untested */
/* note: timing parameters from Matrix Orbital, does not use the second pll, 58,64 FPS */
/* EVE4-101G 1280x800 10.1" Matrix Orbital, capacitive touch, BT817 */
#if defined (EVE_EVE4_101G)
#define EVE_HSIZE	(1280L)	/* Thd Length of visible part of line (in PCLKs) - display width */
#define EVE_VSIZE	(800L)	/* Tvd Number of visible lines (in lines) - display height */
#define EVE_VSYNC0	(11L)	/* Tvf Vertical Front Porch */
#define EVE_VSYNC1	(12L)	/* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define EVE_VOFFSET	(22L)	/* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define EVE_VCYCLE	(823L)	/* Tv Total number of lines (visible and non-visible) (in lines) */
#define EVE_HSYNC0	(78L)	/* Thf Horizontal Front Porch */
#define EVE_HSYNC1	(80L)	/* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define EVE_HOFFSET (158L)	/* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define EVE_HCYCLE 	(1440L)	/* Th Total length of line (visible and non-visible) (in PCLKs) */
#define EVE_PCLK	(1L)	/* 1 = use second PLL for pixel-clock in BT817 / BT818 */
#define EVE_PCLK_FREQ (71000000L)	/* 71MHz - value for EVE_cmd_pclkfreq */
#define EVE_PCLKPOL (0L)	/* PCLK polarity (0 = rising edge, 1 = falling edge) */
#define EVE_SWIZZLE (3L)	/* Defines the arrangement of the RGB pins */
#define EVE_CSPREAD	(0L)	/* helps with noise, when set to 1 fewer signals are changed simultaneously, reset-default: 1 */
#define EVE_TOUCH_RZTHRESH (1200L)	/* touch-sensitivity */
#define EVE_HAS_CRYSTAL
#define EVE_GEN 4
#define EVE_HAS_GT911
#endif


/* untested */
/* RVT101HVBxxxxx 1280x600 7.0" Riverdi, various options, BT817 */
#if defined (EVE_RVT101H)
#define EVE_HSIZE	(1280L)
#define EVE_VSIZE	(800L)

#define EVE_VSYNC0	(0L)
#define EVE_VSYNC1	(10L)
#define EVE_VOFFSET	(23L)
#define EVE_VCYCLE	(838L)
#define EVE_HSYNC0	(0L)
#define EVE_HSYNC1	(20L)
#define EVE_HOFFSET (88L)
#define EVE_HCYCLE 	(1440L)
#define EVE_PCLK	(1L) /* 1 = use second PLL for pixel-clock in BT817 / BT818 */
#define EVE_PCLK_FREQ (72000000L) /* EVE_PCLK needs to be set to 1 for this to take effect */
#define EVE_PCLKPOL (1L)
#define EVE_SWIZZLE (0L)
#define EVE_CSPREAD	(0L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 4
#endif


/* untested */
/* EVE3x-39A 480x128 3.9" 1U Matrix Orbital, resistive touch, BT816 */
#if defined (EVE_EVE3x_39)
#define EVE_HSIZE	(480L)
#define EVE_VSIZE	(272L)

#define EVE_VSYNC0	(8L)
#define EVE_VSYNC1	(11L)
#define EVE_VOFFSET	(15L)
#define EVE_VCYCLE	(288L)
#define EVE_HSYNC0	(44L)
#define EVE_HSYNC1	(28L)
#define EVE_HOFFSET	(44L)
#define EVE_HCYCLE 	(524L)
#define EVE_PCLKPOL	(1L)
#define EVE_SWIZZLE	(0L)
#define EVE_PCLK	(7L)
#define EVE_CSPREAD	(1L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#endif


/* untested */
/* EVE3x-39G 480x128 3.9" 1U Matrix Orbital, capacitive touch, BT815 */
#if defined (EVE_EVE3x_39G)
#define EVE_HSIZE	(480L)
#define EVE_VSIZE	(272L)

#define EVE_VSYNC0	(8L)
#define EVE_VSYNC1	(11L)
#define EVE_VOFFSET	(15L)
#define EVE_VCYCLE	(288L)
#define EVE_HSYNC0	(44L)
#define EVE_HSYNC1	(28L)
#define EVE_HOFFSET	(44L)
#define EVE_HCYCLE 	(524L)
#define EVE_PCLKPOL	(1L)
#define EVE_SWIZZLE	(0L)
#define EVE_PCLK	(7L)
#define EVE_CSPREAD	(1L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 3
#define EVE_HAS_GT911
#endif


/* untested */
/* note: timing parameters from Matrix Orbital, does not use the second pll, 58,64 FPS */
/* EVE4-40G 720x720 4.0" Matrix Orbital, capacitive touch, BT817 */
#if defined (EVE_EVE4_40G)
#define EVE_HSIZE	(720L)
#define EVE_VSIZE	(720L)

#define EVE_VSYNC0	(16)
#define EVE_VSYNC1	(18L)
#define EVE_VOFFSET	(35L)
#define EVE_VCYCLE	(756L)
#define EVE_HSYNC0	(46L)
#define EVE_HSYNC1	(48L)
#define EVE_HOFFSET	(91)
#define EVE_HCYCLE 	(812L)
#define EVE_PCLK	(2L)
#define EVE_PCLKPOL	(1L)
#define EVE_SWIZZLE	(0L)
#define EVE_CSPREAD	(0L)
#define EVE_TOUCH_RZTHRESH (1200L)
#define EVE_HAS_CRYSTAL
#define EVE_GEN 4
#define EVE_HAS_GT911
#endif


/* ------ Common Timings ------ */

#if defined (Resolution_320x240)
#define EVE_HSIZE	(320L)	/* Thd Length of visible part of line (in PCLKs) - display width */
#define EVE_VSIZE	(240L)	/* Tvd Number of visible lines (in lines) - display height */

#define EVE_VSYNC0	(0L)	/* Tvf Vertical Front Porch */
#define EVE_VSYNC1	(2L)	/* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define EVE_VOFFSET	(18L)	/* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define EVE_VCYCLE	(262L)	/* Tv Total number of lines (visible and non-visible) (in lines) */
#define EVE_HSYNC0	(0L)	 /* (40L)	// Thf Horizontal Front Porch */
#define EVE_HSYNC1	(10L)	/* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define EVE_HOFFSET	(70L)	/* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define EVE_HCYCLE 	(408L)	/* Th Total length of line (visible and non-visible) (in PCLKs) */
#endif

#if defined (Resolution_480x272)
#define EVE_HSIZE	(480L)	/* Thd Length of visible part of line (in PCLKs) - display width */
#define EVE_VSIZE	(272L)	/* Tvd Number of visible lines (in lines) - display height */

#define EVE_VSYNC0	(0L)	/* Tvf Vertical Front Porch */
#define EVE_VSYNC1	(10L)	/* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define EVE_VOFFSET	(12L)	/* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define EVE_VCYCLE	(292L)	/* Tv Total number of lines (visible and non-visible) (in lines) */
#define EVE_HSYNC0	(0L)	 /* (40L)	// Thf Horizontal Front Porch */
#define EVE_HSYNC1	(41L)	/* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define EVE_HOFFSET	(43L)	/* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define EVE_HCYCLE 	(548L)	/* Th Total length of line (visible and non-visible) (in PCLKs) */
#endif

#if defined (Resolution_800x480)
#define EVE_HSIZE	(800L)	/* Thd Length of visible part of line (in PCLKs) - display width */
#define EVE_VSIZE	(480L)	/* Tvd Number of visible lines (in lines) - display height */

#define EVE_VSYNC0	(0L)	/* Tvf Vertical Front Porch */
#define EVE_VSYNC1	(3L)	/* Tvf + Tvp Vertical Front Porch plus Vsync Pulse width */
#define EVE_VOFFSET	(32L)	/* Tvf + Tvp + Tvb Number of non-visible lines (in lines) */
#define EVE_VCYCLE	(525L)	/* Tv Total number of lines (visible and non-visible) (in lines) */
#define EVE_HSYNC0	(0L)	 /* (40L)	// Thf Horizontal Front Porch */
#define EVE_HSYNC1	(48L)	/* Thf + Thp Horizontal Front Porch plus Hsync Pulse width */
#define EVE_HOFFSET	(88L)	/* Thf + Thp + Thb Length of non-visible part of line (in PCLK cycles) */
#define EVE_HCYCLE 	(928L)	/* Th Total length of line (visible and non-visible) (in PCLKs) */
#endif


#endif /* EVE_CONFIG_H */
