/*
** r_drawt.cpp
** Faster column drawers for modern processors
**
**---------------------------------------------------------------------------
** Copyright 1998-2001 Randy Heit
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
** These functions stretch columns into a temporary buffer and then
** map them to the screen. On modern machines, this is faster than drawing
** them directly to the screen.
**
** Will I be able to even understand any of this if I come back to it later?
** Let's hope so. :-)
*/

#include "templates.h"
#include "doomtype.h"
#include "doomdef.h"
#include "r_defs.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_things.h"
#include "v_video.h"

// I should have commented this stuff better.
//
// dc_temp is the buffer R_DrawColumnHoriz writes into.
// dc_tspans points into it.
// dc_ctspan points into dc_tspans.
// But what is horizspan, and what is its relation with dc_ctspan?

byte dc_temp[MAXHEIGHT*4];
unsigned int dc_tspans[4][256];
unsigned int *dc_ctspan[4];
unsigned int *horizspan[4];

#ifndef USEASM
// Copies one span at hx to the screen at sx.
void rt_copy1col_c (int hx, int sx, int yl, int yh)
{
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4 + hx];
	pitch = dc_pitch;

	if (count & 1) {
		*dest = *source;
		source += 4;
		dest += pitch;
	}
	if (count & 2) {
		dest[0] = source[0];
		dest[pitch] = source[4];
		source += 8;
		dest += pitch*2;
	}
	if (!(count >>= 2))
		return;

	do {
		dest[0] = source[0];
		dest[pitch] = source[4];
		dest[pitch*2] = source[8];
		dest[pitch*3] = source[12];
		source += 16;
		dest += pitch*4;
	} while (--count);
}

// Copies all four spans to the screen starting at sx.
void rt_copy4cols_c (int sx, int yl, int yh)
{
	int *source;
	int *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	dest = (int *)(ylookup[yl] + sx);
	source = (int *)(&dc_temp[yl*4]);
	pitch = dc_pitch/sizeof(int);
	
	if (count & 1) {
		*dest = *source;
		source += 4/sizeof(int);
		dest += pitch;
	}
	if (!(count >>= 1))
		return;

	do {
		dest[0] = source[0];
		dest[pitch] = source[4/sizeof(int)];
		source += 8/sizeof(int);
		dest += pitch*2;
	} while (--count);
}

// Maps one span at hx to the screen at sx.
void rt_map1col_c (int hx, int sx, int yl, int yh)
{
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	colormap = dc_colormap;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4 + hx];
	pitch = dc_pitch;

	if (count & 1) {
		*dest = colormap[*source];
		source += 4;
		dest += pitch;
	}
	if (!(count >>= 1))
		return;

	do {
		dest[0] = colormap[source[0]];
		dest[pitch] = colormap[source[4]];
		source += 8;
		dest += pitch*2;
	} while (--count);
}

// Maps all four spans to the screen starting at sx.
void rt_map4cols_c (int sx, int yl, int yh)
{
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	colormap = dc_colormap;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4];
	pitch = dc_pitch;
	
	if (count & 1) {
		dest[0] = colormap[source[0]];
		dest[1] = colormap[source[1]];
		dest[2] = colormap[source[2]];
		dest[3] = colormap[source[3]];
		source += 4;
		dest += pitch;
	}
	if (!(count >>= 1))
		return;

	do {
		dest[0] = colormap[source[0]];
		dest[1] = colormap[source[1]];
		dest[2] = colormap[source[2]];
		dest[3] = colormap[source[3]];
		dest[pitch] = colormap[source[4]];
		dest[pitch+1] = colormap[source[5]];
		dest[pitch+2] = colormap[source[6]];
		dest[pitch+3] = colormap[source[7]];
		source += 8;
		dest += pitch*2;
	} while (--count);
}
#endif	/* !USEASM */

// Translates one span at hx to the screen at sx.
void rt_tlate1col (int hx, int sx, int yl, int yh)
{
	byte *translation;
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	translation = dc_translation;
	colormap = dc_colormap;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4 + hx];
	pitch = dc_pitch;

	do {
		*dest = colormap[translation[*source]];
		source += 4;
		dest += pitch;
	} while (--count);
}

// Translates all four spans to the screen starting at sx.
void rt_tlate4cols (int sx, int yl, int yh)
{
	byte *translation;
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	translation = dc_translation;
	count = yh-yl;
	if (count < 0)
		return;
	count++;

	colormap = dc_colormap;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4];
	pitch = dc_pitch;
	
	do {
		dest[0] = colormap[translation[source[0]]];
		dest[1] = colormap[translation[source[1]]];
		dest[2] = colormap[translation[source[2]]];
		dest[3] = colormap[translation[source[3]]];
		source += 4;
		dest += pitch;
	} while (--count);
}

// Adds one span at hx to the screen at sx without clamping.
void rt_add1col (int hx, int sx, int yl, int yh)
{
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	DWORD *fg2rgb = dc_srcblend;
	DWORD *bg2rgb = dc_destblend;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4 + hx];
	pitch = dc_pitch;
	colormap = dc_colormap;

	do {
		DWORD fg = colormap[*source];
		DWORD bg = *dest;

		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		*dest = RGB32k[0][0][fg & (fg>>15)];
		source += 4;
		dest += pitch;
	} while (--count);
}

// Adds all four spans to the screen starting at sx without clamping.
void rt_add4cols (int sx, int yl, int yh)
{
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	DWORD *fg2rgb = dc_srcblend;
	DWORD *bg2rgb = dc_destblend;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4];
	pitch = dc_pitch;
	colormap = dc_colormap;

	do {
		DWORD fg = colormap[source[0]];
		DWORD bg = dest[0];
		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		dest[0] = RGB32k[0][0][fg & (fg>>15)];

		fg = colormap[source[1]];
		bg = dest[1];
		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		dest[1] = RGB32k[0][0][fg & (fg>>15)];


		fg = colormap[source[2]];
		bg = dest[2];
		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		dest[2] = RGB32k[0][0][fg & (fg>>15)];

		fg = colormap[source[3]];
		bg = dest[3];
		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		dest[3] = RGB32k[0][0][fg & (fg>>15)];

		source += 4;
		dest += pitch;
	} while (--count);
}

// Translates and adds one span at hx to the screen at sx without clamping.
void rt_tlateadd1col (int hx, int sx, int yl, int yh)
{
	byte *translation;
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	DWORD *fg2rgb = dc_srcblend;
	DWORD *bg2rgb = dc_destblend;
	translation = dc_translation;
	colormap = dc_colormap;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4 + hx];
	pitch = dc_pitch;

	do {
		DWORD fg = colormap[translation[*source]];
		DWORD bg = *dest;

		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		*dest = RGB32k[0][0][fg & (fg>>15)];
		source += 4;
		dest += pitch;
	} while (--count);
}

// Translates and adds all four spans to the screen starting at sx without clamping.
void rt_tlateadd4cols (int sx, int yl, int yh)
{
	byte *translation;
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	DWORD *fg2rgb = dc_srcblend;
	DWORD *bg2rgb = dc_destblend;
	translation = dc_translation;
	colormap = dc_colormap;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4];
	pitch = dc_pitch;
	
	do {
		DWORD fg = colormap[translation[source[0]]];
		DWORD bg = dest[0];
		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		dest[0] = RGB32k[0][0][fg & (fg>>15)];

		fg = colormap[translation[source[1]]];
		bg = dest[1];
		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		dest[1] = RGB32k[0][0][fg & (fg>>15)];


		fg = colormap[translation[source[2]]];
		bg = dest[2];
		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		dest[2] = RGB32k[0][0][fg & (fg>>15)];

		fg = colormap[translation[source[3]]];
		bg = dest[3];
		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		dest[3] = RGB32k[0][0][fg & (fg>>15)];

		source += 4;
		dest += pitch;
	} while (--count);
}

// Shades one span at hx to the screen at sx.
void rt_shaded1col (int hx, int sx, int yl, int yh)
{
	DWORD *fgstart;
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	fgstart = &Col2RGB8[0][dc_color];
	colormap = dc_colormap;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4 + hx];
	pitch = dc_pitch;

	do {
		DWORD val = colormap[*source];
		DWORD fg = fgstart[val<<8];
		val = (Col2RGB8[64-val][*dest] + fg) | 0x1f07c1f;
		*dest = RGB32k[0][0][val & (val>>15)];
		source += 4;
		dest += pitch;
	} while (--count);
}

// Shades all four spans to the screen starting at sx.
void rt_shaded4cols (int sx, int yl, int yh)
{
	BYTE fill;
	DWORD *fgstart;
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	fgstart = &Col2RGB8[0][dc_color];
	colormap = dc_colormap;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4];
	pitch = dc_pitch;
	{
		DWORD val = fgstart[64<<8] | 0x1f07c1f;
		fill = RGB32k[0][0][val & (val>>15)];
	}
	
	do {
		DWORD val = colormap[source[0]];
		DWORD fg;
		if (val < 64)
		{
			fg = fgstart[val<<8];
			val = (Col2RGB8[64-val][dest[0]] + fg) | 0x1f07c1f;
			dest[0] = RGB32k[0][0][val & (val>>15)];
		}
		else
		{
			dest[0] = fill;
		}

		val = colormap[source[1]];
		if (val < 64)
		{
			fg = fgstart[val<<8];
			val = (Col2RGB8[64-val][dest[1]] + fg) | 0x1f07c1f;
			dest[1] = RGB32k[0][0][val & (val>>15)];
		}
		else
		{
			dest[1] = fill;
		}

		val = colormap[source[2]];
		if (val < 64)
		{
			fg = fgstart[val<<8];
			val = (Col2RGB8[64-val][dest[2]] + fg) | 0x1f07c1f;
			dest[2] = RGB32k[0][0][val & (val>>15)];
		}
		else
		{
			dest[2] = fill;
		}

		val = colormap[source[3]];
		if (val < 64)
		{
			fg = fgstart[val<<8];
			val = (Col2RGB8[64-val][dest[3]] + fg) | 0x1f07c1f;
			dest[3] = RGB32k[0][0][val & (val>>15)];
		}
		else
		{
			dest[3] = fill;
		}

		source += 4;
		dest += pitch;
	} while (--count);
}

// Adds one span at hx to the screen at sx with clamping.
void rt_addclamp1col (int hx, int sx, int yl, int yh)
{
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	DWORD *fg2rgb = dc_srcblend;
	DWORD *bg2rgb = dc_destblend;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4 + hx];
	pitch = dc_pitch;
	colormap = dc_colormap;

	do {
		DWORD a = fg2rgb[colormap[*source]] + bg2rgb[*dest];
		DWORD b = a;

		a |= 0x01f07c1f;
		b &= 0x40100400;
		a &= 0x3fffffff;
		b = b - (b >> 5);
		a |= b;
		*dest = RGB32k[0][0][(a>>15) & a];
		source += 4;
		dest += pitch;
	} while (--count);
}

// Adds all four spans to the screen starting at sx with clamping.
void rt_addclamp4cols (int sx, int yl, int yh)
{
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	DWORD *fg2rgb = dc_srcblend;
	DWORD *bg2rgb = dc_destblend;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4];
	pitch = dc_pitch;
	colormap = dc_colormap;

	do {
		DWORD a = fg2rgb[colormap[source[0]]] + bg2rgb[dest[0]];
		DWORD b = a;

		a |= 0x01f07c1f;
		b &= 0x40100400;
		a &= 0x3fffffff;
		b = b - (b >> 5);
		a |= b;
		dest[0] = RGB32k[0][0][(a>>15) & a];

		a = fg2rgb[colormap[source[1]]] + bg2rgb[dest[1]];
		b = a;
		a |= 0x01f07c1f;
		b &= 0x40100400;
		a &= 0x3fffffff;
		b = b - (b >> 5);
		a |= b;
		dest[1] = RGB32k[0][0][(a>>15) & a];

		a = fg2rgb[colormap[source[2]]] + bg2rgb[dest[2]];
		b = a;
		a |= 0x01f07c1f;
		b &= 0x40100400;
		a &= 0x3fffffff;
		b = b - (b >> 5);
		a |= b;
		dest[2] = RGB32k[0][0][(a>>15) & a];

		a = fg2rgb[colormap[source[3]]] + bg2rgb[dest[3]];
		b = a;
		a |= 0x01f07c1f;
		b &= 0x40100400;
		a &= 0x3fffffff;
		b = b - (b >> 5);
		a |= b;
		dest[3] = RGB32k[0][0][(a>>15) & a];

		source += 4;
		dest += pitch;
	} while (--count);
}

// Translates and adds one span at hx to the screen at sx with clamping.
void rt_tlateaddclamp1col (int hx, int sx, int yl, int yh)
{
	byte *translation;
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	DWORD *fg2rgb = dc_srcblend;
	DWORD *bg2rgb = dc_destblend;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4 + hx];
	pitch = dc_pitch;
	colormap = dc_colormap;
	translation = dc_translation;

	do {
		DWORD a = fg2rgb[colormap[translation[*source]]] + bg2rgb[*dest];
		DWORD b = a;

		a |= 0x01f07c1f;
		b &= 0x40100400;
		a &= 0x3fffffff;
		b = b - (b >> 5);
		a |= b;
		*dest = RGB32k[0][0][(a>>15) & a];
		source += 4;
		dest += pitch;
	} while (--count);
}

// Translates and adds all four spans to the screen starting at sx with clamping.
void rt_tlateaddclamp4cols (int sx, int yl, int yh)
{
	byte *translation;
	byte *colormap;
	byte *source;
	byte *dest;
	int count;
	int pitch;

	count = yh-yl;
	if (count < 0)
		return;
	count++;

	DWORD *fg2rgb = dc_srcblend;
	DWORD *bg2rgb = dc_destblend;
	dest = ylookup[yl] + sx;
	source = &dc_temp[yl*4];
	pitch = dc_pitch;
	colormap = dc_colormap;
	translation = dc_translation;

	do {
		DWORD a = fg2rgb[colormap[translation[source[0]]]] + bg2rgb[dest[0]];
		DWORD b = a;

		a |= 0x01f07c1f;
		b &= 0x40100400;
		a &= 0x3fffffff;
		b = b - (b >> 5);
		a |= b;
		dest[0] = RGB32k[0][0][(a>>15) & a];

		a = fg2rgb[colormap[translation[source[1]]]] + bg2rgb[dest[1]];
		b = a;
		a |= 0x01f07c1f;
		b &= 0x40100400;
		a &= 0x3fffffff;
		b = b - (b >> 5);
		a |= b;
		dest[1] = RGB32k[0][0][(a>>15) & a];

		a = fg2rgb[colormap[translation[source[2]]]] + bg2rgb[dest[2]];
		b = a;
		a |= 0x01f07c1f;
		b &= 0x40100400;
		a &= 0x3fffffff;
		b = b - (b >> 5);
		a |= b;
		dest[2] = RGB32k[0][0][(a>>15) & a];

		a = fg2rgb[colormap[translation[source[3]]]] + bg2rgb[dest[3]];
		b = a;
		a |= 0x01f07c1f;
		b &= 0x40100400;
		a &= 0x3fffffff;
		b = b - (b >> 5);
		a |= b;
		dest[3] = RGB32k[0][0][(a>>15) & a];

		source += 4;
		dest += pitch;
	} while (--count);
}

// Copies all spans in all four columns to the screen starting at sx.
// sx should be longword-aligned.
void rt_draw4cols (int sx)
{
	int x, bad;
	unsigned int maxtop, minbot, minnexttop;

	// Place a dummy "span" in each column. These don't get
	// drawn. They're just here to avoid special cases in the
	// max/min calculations below.
	for (x = 0; x < 4; ++x)
	{
		dc_ctspan[x][0] = viewheight+1;
		dc_ctspan[x][1] = viewheight;
	}

	for (;;)
	{
		// If a column is out of spans, mark it as such
		bad = 0;
		minnexttop = 0xffffffff;
		for (x = 0; x < 4; ++x)
		{
			if (horizspan[x] >= dc_ctspan[x])
			{
				bad |= 1 << x;
			}
			else if ((horizspan[x]+2)[0] < minnexttop)
			{
				minnexttop = (horizspan[x]+2)[0];
			}
		}
		// Once all columns are out of spans, we're done
		if (bad == 15)
		{
			return;
		}

		// Find the largest shared area for the spans in each column
		maxtop = MAX (MAX (horizspan[0][0], horizspan[1][0]),
					  MAX (horizspan[2][0], horizspan[3][0]));
		minbot = MIN (MIN (horizspan[0][1], horizspan[1][1]),
					  MIN (horizspan[2][1], horizspan[3][1]));

		// If there is no shared area with these spans, draw each span
		// individually and advance to the next spans until we reach a shared area.
		// However, only draw spans down to the highest span in the next set of
		// spans. If we allow the entire height of a span to be drawn, it could
		// prevent any more shared areas from being drawn in these four columns.
		//
		// Example: Suppose we have the following arrangement:
		//			A CD
		//			A CD
		//			 B D
		//			 B D
		//			aB D
		//			aBcD
		//			aBcD
		//			aBc
		//
		// If we draw the entire height of the spans, we end up drawing this first:
		//			A CD
		//			A CD
		//			 B D
		//			 B D
		//			 B D
		//			 B D
		//			 B D
		//			 B D
		//			 B
		//
		// This leaves only the "a" and "c" columns to be drawn, and they are not
		// part of a shared area, but if we can include B and D with them, we can
		// get a shared area. So we cut off everything in the first set just
		// above the "a" column and end up drawing this first:
		//			A CD
		//			A CD
		//			 B D
		//			 B D
		//
		// Then the next time through, we have the following arrangement with an
		// easily shared area to draw:
		//			aB D
		//			aBcD
		//			aBcD
		//			aBc
		if (bad != 0 || maxtop > minbot)
		{
			for (x = 0; x < 4; ++x)
			{
				if (!(bad & 1))
				{
					if (horizspan[x][1] < minnexttop)
					{
						hcolfunc_post1 (x, sx+x, horizspan[x][0], horizspan[x][1]);
						horizspan[x] += 2;
					}
					else if (minnexttop > horizspan[x][0])
					{
						hcolfunc_post1 (x, sx+x, horizspan[x][0], minnexttop-1);
						horizspan[x][0] = minnexttop;
					}
				}
				bad >>= 1;
			}
			continue;
		}

		// Draw any span fragments above the shared area.
		for (x = 0; x < 4; ++x)
		{
			if (maxtop > horizspan[x][0])
			{
				hcolfunc_post1 (x, sx+x, horizspan[x][0], maxtop-1);
			}
		}

		// Draw the shared area.
		hcolfunc_post4 (sx, maxtop, minbot);

		// For each column, if part of the span is past the shared area,
		// set its top to just below the shared area. Otherwise, advance
		// to the next span in that column.
		for (x = 0; x < 4; ++x)
		{
			if (minbot < horizspan[x][1])
			{
				horizspan[x][0] = minbot+1;
			}
			else
			{
				horizspan[x] += 2;
			}
		}
	}
}

// Before each pass through a rendering loop that uses these routines,
// call this function to set up the span pointers.
void rt_initcols (void)
{
	int y;

	for (y = 3; y >= 0; y--)
		horizspan[y] = dc_ctspan[y] = &dc_tspans[y][0];
}

// Stretches a column into a temporary buffer which is later
// drawn to the screen along with up to three other columns.
void R_DrawColumnHorizP_C (void)
{
	int count = dc_yh - dc_yl;
	byte *dest;
	fixed_t fracstep;
	fixed_t frac;

	if (count < 0)
		return;
	++count;

	{
		int x = dc_x & 3;
		unsigned int **span;
		
		span = &dc_ctspan[x];
		(*span)[0] = dc_yl;
		(*span)[1] = dc_yh;
		*span += 2;
		dest = &dc_temp[x + 4*dc_yl];
	}
	fracstep = dc_iscale;
	frac = dc_texturefrac;

	{
		byte *source = dc_source;

		if (count & 1) {
			*dest = source[frac>>FRACBITS];
			dest += 4;
			frac += fracstep;
		}
		if (count & 2) {
			dest[0] = source[frac>>FRACBITS];
			frac += fracstep;
			dest[4] = source[frac>>FRACBITS];
			frac += fracstep;
			dest += 8;
		}
		if (count & 4) {
			dest[0] = source[frac>>FRACBITS];
			frac += fracstep;
			dest[4] = source[frac>>FRACBITS];
			frac += fracstep;
			dest[8] = source[frac>>FRACBITS];
			frac += fracstep;
			dest[12] = source[frac>>FRACBITS];
			frac += fracstep;
			dest += 16;
		}
		count >>= 3;
		if (!count) return;

		do
		{
			dest[0] = source[frac>>FRACBITS];
			frac += fracstep;
			dest[4] = source[frac>>FRACBITS];
			frac += fracstep;
			dest[8] = source[frac>>FRACBITS];
			frac += fracstep;
			dest[12] = source[frac>>FRACBITS];
			frac += fracstep;
			dest[16] = source[frac>>FRACBITS];
			frac += fracstep;
			dest[20] = source[frac>>FRACBITS];
			frac += fracstep;
			dest[24] = source[frac>>FRACBITS];
			frac += fracstep;
			dest[28] = source[frac>>FRACBITS];
			frac += fracstep;
			dest += 32;
		} while (--count);
	}
}

// [RH] Just fills a column with a given color
void R_FillColumnHorizP (void)
{
	int count = dc_yh - dc_yl;
	byte color = dc_color;
	byte *dest;

	if (count++ < 0)
		return;

	count++;
	{
		int x = dc_x & 3;
		unsigned int **span = &dc_ctspan[x];

		(*span)[0] = dc_yl;
		(*span)[1] = dc_yh;
		*span += 2;
		dest = &dc_temp[x + 4*dc_yl];
	}

	if (count & 1) {
		*dest = color;
		dest += 4;
	}
	if (!(count >>= 1))
		return;
	do {
		dest[0] = color;
		dest[4] = color;
		dest += 8;
	} while (--count);
}

// Same as R_DrawMaskedColumn() except that it always uses
// R_DrawColumnHoriz().

void R_DrawMaskedColumnHoriz (column_t *column)
{
	int top = 0;

	while (column->topdelta != 0xff)
	{
		if (column->topdelta <= top)
		{
			top += column->topdelta;
		}
		else
		{
			top = column->topdelta;
		}
		if (column->length == 0)
		{
			goto nextpost;
		}
		// calculate unclipped screen coordinates for post
		dc_yl = (sprtopscreen + spryscale * top) >> FRACBITS;
		dc_yh = (sprtopscreen + spryscale * (top + column->length) - FRACUNIT) >> FRACBITS;

		if (sprflipvert)
		{
			swap (dc_yl, dc_yh);
		}

		if (dc_yh >= mfloorclip[dc_x])
		{
			dc_yh = mfloorclip[dc_x] - 1;
		}
		if (dc_yl < mceilingclip[dc_x])
		{
			dc_yl = mceilingclip[dc_x];
		}

		if (dc_yl <= dc_yh)
		{
			if (sprflipvert)
			{
				dc_texturefrac = (dc_yl*dc_iscale) - (top << FRACBITS)
					- FixedMul (centeryfrac, dc_iscale) - dc_texturemid;
				const fixed_t maxfrac = column->length << FRACBITS;
				while (dc_texturefrac >= maxfrac)
				{
					if (++dc_yl > dc_yh)
						goto nextpost;
					dc_texturefrac += dc_iscale;
				}
				fixed_t endfrac = dc_texturefrac + (dc_yh-dc_yl)*dc_iscale;
				while (endfrac < 0)
				{
					if (--dc_yh < dc_yl)
						goto nextpost;
					endfrac -= dc_iscale;
				}
			}
			else
			{
				dc_texturefrac = dc_texturemid - (top << FRACBITS)
					+ (dc_yl*dc_iscale) - FixedMul (centeryfrac-FRACUNIT, dc_iscale);
				while (dc_texturefrac < 0)
				{
					if (++dc_yl > dc_yh)
						goto nextpost;
					dc_texturefrac += dc_iscale;
				}
				fixed_t endfrac = dc_texturefrac + (dc_yh-dc_yl)*dc_iscale;
				const fixed_t maxfrac = column->length << FRACBITS;
				while (endfrac >= maxfrac)
				{
					if (--dc_yh < dc_yl)
						goto nextpost;
					endfrac -= dc_iscale;
				}
			}
			dc_source = (byte *)column + 3;
			hcolfunc_pre ();
		}
nextpost:
		column = (column_t *)((byte *)column + column->length + 4);
	}

	if (sprflipvert)
	{
		unsigned int *front = horizspan[dc_x&3];
		unsigned int *back = dc_ctspan[dc_x&3] - 2;

		// Reorder the posts so that they get drawn top-to-bottom
		// instead of bottom-to-top.
		while (front < back)
		{
			swap (front[0], back[0]);
			swap (front[1], back[1]);
			front += 2;
			back -= 2;
		}
	}
}

void R_DrawMaskedColumnHoriz2 (column2_t *column)
{
	while (column->Length != 0)
	{
		const int length = column->Length;
		const int top = column->TopDelta;

		// calculate unclipped screen coordinates for post
		dc_yl = (sprtopscreen + spryscale * top) >> FRACBITS;
		dc_yh = (sprtopscreen + spryscale * (top + length) - FRACUNIT) >> FRACBITS;

		if (sprflipvert)
		{
			swap (dc_yl, dc_yh);
		}

		if (dc_yh >= mfloorclip[dc_x])
		{
			dc_yh = mfloorclip[dc_x] - 1;
		}
		if (dc_yl < mceilingclip[dc_x])
		{
			dc_yl = mceilingclip[dc_x];
		}

		if (dc_yl <= dc_yh)
		{
			if (sprflipvert)
			{
				dc_texturefrac = (dc_yl*dc_iscale) - (top << FRACBITS)
					- FixedMul (centeryfrac, dc_iscale) - dc_texturemid;
				const fixed_t maxfrac = length << FRACBITS;
				while (dc_texturefrac >= maxfrac)
				{
					if (++dc_yl > dc_yh)
						goto nextpost;
					dc_texturefrac += dc_iscale;
				}
				fixed_t endfrac = dc_texturefrac + (dc_yh-dc_yl)*dc_iscale;
				while (endfrac < 0)
				{
					if (--dc_yh < dc_yl)
						goto nextpost;
					endfrac -= dc_iscale;
				}
			}
			else
			{
				dc_texturefrac = dc_texturemid - (top << FRACBITS)
					+ (dc_yl*dc_iscale) - FixedMul (centeryfrac-FRACUNIT, dc_iscale);
				while (dc_texturefrac < 0)
				{
					if (++dc_yl > dc_yh)
						goto nextpost;
					dc_texturefrac += dc_iscale;
				}
				fixed_t endfrac = dc_texturefrac + (dc_yh-dc_yl)*dc_iscale;
				const fixed_t maxfrac = length << FRACBITS;
				while (endfrac >= maxfrac)
				{
					if (--dc_yh < dc_yl)
						goto nextpost;
					endfrac -= dc_iscale;
				}
			}
			dc_source = (byte *)column + 4;
			hcolfunc_pre ();
		}
nextpost:
		column = (column2_t *)((byte *)column + length + 4);
	}

	if (sprflipvert)
	{
		unsigned int *front = horizspan[dc_x&3];
		unsigned int *back = dc_ctspan[dc_x&3] - 2;

		// Reorder the posts so that they get drawn top-to-bottom
		// instead of bottom-to-top.
		while (front < back)
		{
			swap (front[0], back[0]);
			swap (front[1], back[1]);
			front += 2;
			back -= 2;
		}
	}
}
