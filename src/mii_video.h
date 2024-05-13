/*
 * mii_video.h
 *
 * Copyright (C) 2023 Michel Pollet <buserror@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/*
 * When 1, this will draw a heat map of the dirty lines alongside the video
 * This is useful to see if the video rendering is efficient.
 * This has no real use apart from debugging the video rendering.
 */
#define MII_VIDEO_DEBUG_HEAPMAP	0

// TODO move VRAM stuff to somewhere else
#define MII_VIDEO_WIDTH		(280 * 2)
#define MII_VIDEO_HEIGHT	(192 * 2)
// in case padding is needed, these can be changed

#if 0	// Loads of padding to align to power of 2 sizes
#define MII_VRAM_WIDTH		(1024 * 4)
#define MII_VRAM_HEIGHT		512
#else
#define MII_VRAM_WIDTH		(MII_VIDEO_WIDTH * 4)
#define MII_VRAM_HEIGHT		MII_VIDEO_HEIGHT
#endif

struct mii_t;
struct mii_video_t;

typedef void (*mii_video_line_drawing_cb)(
					mii_t *mii );
typedef void (*mii_video_line_check_cb)(
					struct mii_video_t *video,
					uint32_t 			sw,
					uint16_t 			addr);

typedef struct mii_video_cb_t {
	mii_video_line_drawing_cb	render;
	mii_video_line_check_cb		check;
} mii_video_cb_t;

typedef uint32_t mii_color_t;

/*
 * Color Lookup Table fro all the modes. They default to the color
 * version defined in mii_video.c, then get derived from that for
 * the green and amber modes.
 */
typedef union mii_video_clut_t {
	struct {
		mii_color_t 	lores[2][16];	// lores (main, and aux page DLORES)
		mii_color_t 	dhires[16];
		mii_color_t 	hires[10];
//		mii_color_t 	hires2[8];
		mii_color_t 	text[2];		// text
		mii_color_t 	mono[2];		// DHRES mono mode
	};
	mii_color_t 		colors[(2*16) + 16 + 10 /*+ 8*/ + 2 + 2];
} mii_video_clut_t;

typedef struct mii_video_t {
	void *				state;		// protothread state in mii_video.c
	uint8_t 			timer_id;	// timer id for the video thread
	uint8_t				line;		// current line for cycle timer
	uint16_t 			base_addr;	// current mode base address
	uint16_t			line_addr;	// VRAM address for the line we are on
	uint64_t 			timer_max;	// timer start value
	uint32_t			frame_count; // incremented every frame
	uint8_t 			color_mode;	// color palette index
	uint8_t   			monochrome;	// monochrome mode
	mii_video_clut_t 	clut;		// current color table
	mii_video_clut_t	clut_low; 	// low luminance version
	// function pointer to the line drawing function
	mii_video_cb_t		line_cb;
	uint8_t 			frame_dirty;
	uint32_t 			frame_seed; // changed when pixels have changed for this frame
	uint64_t 			lines_dirty[192 / 64]; // 192 lines / 64 bits

#if MII_VIDEO_DEBUG_HEAPMAP
	uint8_t 			video_hmap[192]
			__attribute__((aligned(32))) ; // line dirty heat map
#endif
	// alignment is required for vector extensions
	uint32_t 			pixels[MII_VRAM_WIDTH * MII_VRAM_HEIGHT]
			__attribute__((aligned(32)));
} mii_video_t;

bool
mii_access_video(
		struct mii_t *mii,
		uint16_t addr,
		uint8_t * byte,
		bool write);
void
mii_video_init(
		mii_t *mii);

void
mii_video_set_mode(
		mii_t *mii,
		uint8_t mode);
/* out of bounds write check. This allow SmartPort DMA drive to
 * pass down the range it writes buffers to, so the video gets
 * a chance to check if the addresses are in RAM, in case the
 * Prodos call is loading an image into VRAM proper */
void
mii_video_OOB_write_check(
		mii_t *mii,
		uint16_t addr,
		uint16_t size);
uint8_t
mii_video_get_vapor(
		mii_t *mii);
