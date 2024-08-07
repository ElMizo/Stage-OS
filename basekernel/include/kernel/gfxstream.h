/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef KERNEL_GFXSTREAM_H
#define KERNEL_GFXSTREAM_H

typedef enum {
	GRAPHICS_END = 0,
	GRAPHICS_FGCOLOR,
	GRAPHICS_BGCOLOR,
	GRAPHICS_LINE,
	GRAPHICS_RECT,
	GRAPHICS_CLEAR,
	GRAPHICS_TEXT
} graphics_command_t;

#endif
