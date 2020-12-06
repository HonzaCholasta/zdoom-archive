#include "doomtype.h"
#include "v_video.h"
#include "m_swap.h"

#include "i_system.h"
#include "i_video.h"

// [RH] Stretch values for V_DrawPatchClean()
int CleanXfac, CleanYfac;
// [RH] Virtual screen sizes as perpetuated by V_DrawPatchClean()
int CleanWidth, CleanHeight;

fixed_t V_Fade = 0x8000;

// The current set of column drawers (set in V_SetResolution)
DCanvas::vdrawfunc *DCanvas::m_Drawfuncs;
DCanvas::vdrawsfunc *DCanvas::m_Drawsfuncs;


// Palettized versions of the column drawers
DCanvas::vdrawfunc DCanvas::Pfuncs[6] =
{
	DCanvas::DrawPatchP,
	DCanvas::DrawLucentPatchP,
	DCanvas::DrawTranslatedPatchP,
	DCanvas::DrawTlatedLucentPatchP,
	DCanvas::DrawColoredPatchP,
	DCanvas::DrawColorLucentPatchP,
};
DCanvas::vdrawsfunc DCanvas::Psfuncs[6] =
{
	DCanvas::DrawPatchSP,
	DCanvas::DrawLucentPatchSP,
	DCanvas::DrawTranslatedPatchSP,
	DCanvas::DrawTlatedLucentPatchSP,
	DCanvas::DrawColoredPatchSP,
	DCanvas::DrawColorLucentPatchSP
};

// Direct (true-color) versions of the column drawers
DCanvas::vdrawfunc DCanvas::Dfuncs[6] =
{
	DCanvas::DrawPatchD,
	DCanvas::DrawLucentPatchD,
	DCanvas::DrawTranslatedPatchD,
	DCanvas::DrawTlatedLucentPatchD,
	DCanvas::DrawColoredPatchD,
	DCanvas::DrawColorLucentPatchD,
};
DCanvas::vdrawsfunc DCanvas::Dsfuncs[6] =
{
	DCanvas::DrawPatchSD,
	DCanvas::DrawLucentPatchSD,
	DCanvas::DrawTranslatedPatchSD,
	DCanvas::DrawTlatedLucentPatchSD,
	(vdrawsfunc)DCanvas::DrawColoredPatchD,
	(vdrawsfunc)DCanvas::DrawColorLucentPatchD
};

byte *V_ColorMap;
int V_ColorFill;

// Palette lookup table for direct modes
unsigned int *V_Palette;


/*********************************/
/*								 */
/* The palletized column drawers */
/*								 */
/*********************************/

// Normal patch drawers
void DCanvas::DrawPatchP (const byte *source, byte *dest, int count, int pitch)
{
	do
	{
		*dest = *source++;
		dest += pitch;
	} while (--count);
}

void DCanvas::DrawPatchSP (const byte *source, byte *dest, int count, int pitch, int yinc)
{
	int c = 0;

	do
	{
		*dest = source[c >> 16]; 
		dest += pitch;
		c += yinc;
	} while (--count);
}


// Translucent patch drawers (amount is in V_Fade)
void DCanvas::DrawLucentPatchP (const byte *source, byte *dest, int count, int pitch)
{
	unsigned int *fg2rgb, *bg2rgb;

	{
		fixed_t fglevel, bglevel;

		fglevel = V_Fade & ~0x3ff;
		bglevel = FRACUNIT-fglevel;
		fg2rgb = Col2RGB8[fglevel>>10];
		bg2rgb = Col2RGB8[bglevel>>10];
	}

	do
	{
		unsigned int fg = *source++;
		unsigned int bg = *dest;

		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		*dest = RGB32k[0][0][fg & (fg>>15)];
		dest += pitch; 
	} while (--count);
}

void DCanvas::DrawLucentPatchSP (const byte *source, byte *dest, int count, int pitch, int yinc)
{
	unsigned int *fg2rgb, *bg2rgb;
	int c = 0;

	{
		fixed_t fglevel, bglevel;

		fglevel = V_Fade & ~0x3ff;
		bglevel = FRACUNIT-fglevel;
		fg2rgb = Col2RGB8[fglevel>>10];
		bg2rgb = Col2RGB8[bglevel>>10];
	}

	do
	{
		unsigned int fg = source[c >> 16];
		unsigned int bg = *dest;

		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		*dest = RGB32k[0][0][fg & (fg>>15)];
		dest += pitch;
		c += yinc;
	} while (--count);
}


// Translated patch drawers
void DCanvas::DrawTranslatedPatchP (const byte *source, byte *dest, int count, int pitch)
{
	do
	{
		*dest = V_ColorMap[*source++];
		dest += pitch; 
	} while (--count);
}

void DCanvas::DrawTranslatedPatchSP (const byte *source, byte *dest, int count, int pitch, int yinc)
{
	int c = 0;

	do
	{
		*dest = V_ColorMap[source[c >> 16]];
		dest += pitch;
		c += yinc;
	} while (--count);
}


// Translated, translucent patch drawers
void DCanvas::DrawTlatedLucentPatchP (const byte *source, byte *dest, int count, int pitch)
{
	unsigned int *fg2rgb, *bg2rgb;
	byte *colormap = V_ColorMap;

	{
		fixed_t fglevel, bglevel;

		fglevel = V_Fade & ~0x3ff;
		bglevel = FRACUNIT-fglevel;
		fg2rgb = Col2RGB8[fglevel>>10];
		bg2rgb = Col2RGB8[bglevel>>10];
	}

	do
	{
		unsigned int fg = colormap[*source++];
		unsigned int bg = *dest;

		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		*dest = RGB32k[0][0][fg & (fg>>15)];
		dest += pitch; 
	} while (--count);
}

void DCanvas::DrawTlatedLucentPatchSP (const byte *source, byte *dest, int count, int pitch, int yinc)
{
	int c = 0;
	unsigned int *fg2rgb, *bg2rgb;
	byte *colormap = V_ColorMap;

	{
		fixed_t fglevel, bglevel;

		fglevel = V_Fade & ~0x3ff;
		bglevel = FRACUNIT-fglevel;
		fg2rgb = Col2RGB8[fglevel>>10];
		bg2rgb = Col2RGB8[bglevel>>10];
	}

	do
	{
		unsigned int fg = colormap[source[c >> 16]];
		unsigned int bg = *dest;

		fg = fg2rgb[fg];
		bg = bg2rgb[bg];
		fg = (fg+bg) | 0x1f07c1f;
		*dest = RGB32k[0][0][fg & (fg>>15)];
		dest += pitch;
		c += yinc;
	} while (--count);
}


// Colored patch drawer
//
void DCanvas::DrawColoredPatchP (const byte *source, byte *dest, int count, int pitch)
{
	byte fill = (byte)V_ColorFill;

	do
	{
		*dest = fill;
		dest += pitch; 
	} while (--count);
}

void DCanvas::DrawColoredPatchSP (const byte *source, byte *dest, int count, int pitch, int yinc)
{
	byte fill = (byte)V_ColorFill;

	do
	{
		*dest = fill;
		dest += pitch; 
	} while (--count);
}


// Colored, translucent patch drawer
//
void DCanvas::DrawColorLucentPatchP (const byte *source, byte *dest, int count, int pitch)
{
	unsigned int *bg2rgb;
	unsigned int fg;

	{
		fixed_t fglevel;

		fglevel = V_Fade & ~0x3ff;
		bg2rgb = Col2RGB8[(FRACUNIT-fglevel)>>10];
		fg = Col2RGB8[fglevel>>10][V_ColorFill];
	}

	do
	{
		unsigned int bg;
		bg = (fg + bg2rgb[*dest]) | 0x1f07c1f;
		*dest = RGB32k[0][0][bg & (bg>>15)];
		dest += pitch; 
	} while (--count);
}

void DCanvas::DrawColorLucentPatchSP (const byte *source, byte *dest, int count, int pitch, int yinc)
{
	unsigned int *bg2rgb;
	unsigned int fg;

	{
		fixed_t fglevel;

		fglevel = V_Fade & ~0x3ff;
		bg2rgb = Col2RGB8[(FRACUNIT-fglevel)>>10];
		fg = Col2RGB8[fglevel>>10][V_ColorFill];
	}

	do
	{
		unsigned int bg;
		bg = (fg + bg2rgb[*dest]) | 0x1f07c1f;
		*dest = RGB32k[0][0][bg & (bg>>15)];
		dest += pitch; 
	} while (--count);
}


/**************************/
/*						  */
/* The RGB column drawers */
/*						  */
/**************************/

// Normal patch drawers
void DCanvas::DrawPatchD (const byte *source, byte *dest, int count, int pitch)
{
	do
	{
		*((unsigned int *)dest) = V_Palette[*source++];
		dest += pitch;
	} while (--count);
}

void DCanvas::DrawPatchSD (const byte *source, byte *dest, int count, int pitch, int yinc)
{
	int c = 0;

	do
	{
		*((unsigned int *)dest) = V_Palette[source[c >> 16]];
		dest += pitch;
		c += yinc;
	} while (--count);
}


// Translucent patch drawers (always 50%)
void DCanvas::DrawLucentPatchD (const byte *source, byte *dest, int count, int pitch)
{
	do
	{
		*((unsigned int *)dest) = ((V_Palette[*source++] & 0xfefefe) >> 1) +
						 ((*((int *)dest) & 0xfefefe) >> 1);
		dest += pitch; 
	} while (--count);
}

void DCanvas::DrawLucentPatchSD (const byte *source, byte *dest, int count, int pitch, int yinc)
{
	int c = 0;

	do
	{
		*((unsigned int *)dest) = ((V_Palette[source[c >> 16]] & 0xfefefe) >> 1) +
						 ((*((int *)dest) & 0xfefefe) >> 1);
		dest += pitch;
		c += yinc;
	} while (--count);
}


// Translated patch drawers
void DCanvas::DrawTranslatedPatchD (const byte *source, byte *dest, int count, int pitch)
{
	do
	{
		*((unsigned int *)dest) = V_Palette[V_ColorMap[*source++]];
		dest += pitch; 
	} while (--count);
}

void DCanvas::DrawTranslatedPatchSD (const byte *source, byte *dest, int count, int pitch, int yinc)
{
	int c = 0;

	do
	{
		*((unsigned int *)dest) = V_Palette[V_ColorMap[source[c >> 16]]];
		dest += pitch;
		c += yinc;
	} while (--count);
}


// Translated, translucent patch drawers
void DCanvas::DrawTlatedLucentPatchD (const byte *source, byte *dest, int count, int pitch)
{
	do
	{
		*((unsigned int *)dest) = ((V_Palette[V_ColorMap[*source++]] & 0xfefefe) >> 1) +
						 ((*((int *)dest) & 0xfefefe) >> 1);
		dest += pitch; 
	} while (--count);
}

void DCanvas::DrawTlatedLucentPatchSD (const byte *source, byte *dest, int count, int pitch, int yinc)
{
	int c = 0;

	do
	{
		*((unsigned int *)dest) = ((V_Palette[V_ColorMap[source[c >> 16]]] & 0xfefefe) >> 1) +
						 ((*((int *)dest) & 0xfefefe) >> 1);
		dest += pitch;
		c += yinc;
	} while (--count);
}


// Colored patch drawer
//
// This routine is the same for the stretched version since we don't
// care about the patch's actual contents, just it's outline.
void DCanvas::DrawColoredPatchD (const byte *source, byte *dest, int count, int pitch)
{
	do
	{
		*((int *)dest) = V_ColorFill;
		dest += pitch; 
	} while (--count);
}


// Colored, translucent patch drawer
//
// This routine is the same for the stretched version since we don't
// care about the patch's actual contents, just it's outline.
void DCanvas::DrawColorLucentPatchD (const byte *source, byte *dest, int count, int pitch)
{
	int fill = (V_ColorFill & 0xfefefe) >> 1;

	do
	{
		*((int *)dest) = fill + ((*((int *)dest) & 0xfefefe) >> 1);
		dest += pitch; 
	} while (--count);
}



/******************************/
/*							  */
/* The patch drawing wrappers */
/*							  */
/******************************/

//
// V_DrawWrapper
// Masks a column based masked pic to the screen. 
//
void DCanvas::DrawWrapper (EWrapperCode drawer, const patch_t *patch, int x, int y) const
{
	int 		col;
	int			colstep;
	column_t*	column;
	byte*		desttop;
	int 		w;
	vdrawfunc	drawfunc;

	y -= SHORT(patch->topoffset);
	x -= SHORT(patch->leftoffset);
#ifdef RANGECHECK 
	if (x<0
		||x+SHORT(patch->width) > width
		|| y<0
		|| y+SHORT(patch->height)>height)
	{
	  // Printf (PRINT_HIGH, "Patch at %d,%d exceeds LFB\n", x,y );
	  // No I_Error abort - what is up with TNT.WAD?
	  DPrintf ("DCanvas::DrawWrapper: bad patch (ignored)\n");
	  return;
	}
#endif

	if (is8bit)
	{
		drawfunc = Pfuncs[drawer];
		colstep = 1;
	}
	else
	{
		drawfunc = Dfuncs[drawer];
		colstep = 4;
	}

	col = 0;
	desttop = buffer + y*pitch + x * colstep;

	w = SHORT(patch->width);

	for ( ; col<w ; x++, col++, desttop += colstep)
	{
		column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

		// step through the posts in a column
		while (column->topdelta != 0xff )
		{
			drawfunc ((byte *)column + 3,
					  desttop + column->topdelta * pitch,
					  column->length,
					  pitch);

			column = (column_t *)(	(byte *)column + column->length
									+ 4 );
		}
	}
}

//
// V_DrawSWrapper
// Masks a column based masked pic to the screen
// stretching it to fit the given dimensions.
//
extern void F_DrawPatchCol (int, const patch_t *, int, const DCanvas *);

void DCanvas::DrawSWrapper (EWrapperCode drawer, const patch_t *patch, int x0, int y0, int destwidth, int destheight) const
{
	column_t*	column; 
	byte*		desttop;
	vdrawsfunc	drawfunc;
	int			colstep;

	int			xinc, yinc, col, w, ymul, xmul;

	if (destwidth == SHORT(patch->width) && destheight == SHORT(patch->height))
	{
		DrawWrapper (drawer, patch, x0, y0);
		return;
	}

	if (destwidth == width && destheight == height &&
		SHORT(patch->width) == 320 && SHORT(patch->height) == 200
		&& drawer == EWrapper_Normal)
	{
		// Special case: Drawing a full-screen patch, so use
		// F_DrawPatchCol in f_finale.c, since it's faster.
		for (w = 0; w < 320; w++)
			F_DrawPatchCol (w, patch, w, this);
		return;
	}

	xinc = (SHORT(patch->width) << 16) / destwidth;
	yinc = (SHORT(patch->height) << 16) / destheight;
	xmul = (destwidth << 16) / SHORT(patch->width);
	ymul = (destheight << 16) / SHORT(patch->height);

	y0 -= (SHORT(patch->topoffset) * ymul) >> 16;
	x0 -= (SHORT(patch->leftoffset) * xmul) >> 16;

#ifdef RANGECHECK 
	if (x0<0
		|| x0+destwidth > width
		|| y0<0
		|| y0+destheight> height)
	{
		//Printf ("Patch at %d,%d exceeds LFB\n", x0,y0 );
		DPrintf ("DCanvas::DrawSWrapper: bad patch (ignored)\n");
		return;
	}
#endif

	if (is8bit) {
		drawfunc = Psfuncs[drawer];
		colstep = 1;
	} else {
		drawfunc = Dsfuncs[drawer];
		colstep = 4;
	}

	col = 0;
	desttop = buffer + y0*pitch + x0 * colstep;

	w = destwidth * xinc;

	for ( ; col<w ; col += xinc, desttop += colstep)
	{
		column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int top = column->topdelta * ymul;
			int bot = top + column->length * ymul;
			bot = (bot - top + 0x8000) >> 16;
			if (bot > 0)
			{
				top = (top) >> 16;
				drawfunc ((byte *)column + 3, desttop + top * pitch,
					bot, pitch, yinc);
				column = (column_t *)((byte *)column + column->length + 4);
			}
		}
	}
}

//
// V_DrawIWrapper
// Like V_DrawWrapper except it will stretch the patches as
// needed for non-320x200 screens.
//
void DCanvas::DrawIWrapper (EWrapperCode drawer, const patch_t *patch, int x0, int y0) const
{
	if (width == 320 && height == 200)
		DrawWrapper (drawer, patch, x0, y0);
	else
		DrawSWrapper (drawer, patch,
			 (width * x0) / 320, (height * y0) / 200,
			 (width * SHORT(patch->width)) / 320, (height * SHORT(patch->height)) / 200);
}

//
// V_DrawCWrapper
// Like V_DrawIWrapper, except it only uses integral multipliers.
//
void DCanvas::DrawCWrapper (EWrapperCode drawer, const patch_t *patch, int x0, int y0) const
{
	if (CleanXfac == 1 && CleanYfac == 1)
		DrawWrapper (drawer, patch, (x0-160) + (width/2), (y0-100) + (height/2));
	else
		DrawSWrapper (drawer, patch,
			(x0-160)*CleanXfac+(width/2), (y0-100)*CleanYfac+(height/2),
			SHORT(patch->width) * CleanXfac, SHORT(patch->height) * CleanYfac);
}

//
// V_DrawCNMWrapper
// Like V_DrawCWrapper, except it doesn't adjust the x and y coordinates.
//
void DCanvas::DrawCNMWrapper (EWrapperCode drawer, const patch_t *patch, int x0, int y0) const
{
	if (CleanXfac == 1 && CleanYfac == 1)
		DrawWrapper (drawer, patch, x0, y0);
	else
		DrawSWrapper (drawer, patch, x0, y0,
						SHORT(patch->width) * CleanXfac,
						SHORT(patch->height) * CleanYfac);
}


/********************************/
/*								*/
/* Other miscellaneous routines */
/*								*/
/********************************/

//
// V_CopyRect 
// 
void DCanvas::CopyRect (int srcx, int srcy, int _width, int _height,
						int destx, int desty, DCanvas *destscrn)
{
#ifdef RANGECHECK
	if (srcx<0
		||srcx+_width > width
		|| srcy<0
		|| srcy+_height> height
		||destx<0||destx+_width > destscrn->width
		|| desty<0
		|| desty+_height > destscrn->height)
	{
		I_Error ("Bad DCanvas::CopyRect");
	}
#endif

	Blit (srcx, srcy, _width, _height, destscrn, destx, desty, _width, _height);
}

//
// V_DrawPatchFlipped
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
//
// Like V_DrawIWrapper except it only uses one drawing function and draws
// the patch flipped horizontally.
//
void DCanvas::DrawPatchFlipped (const patch_t *patch, int x0, int y0) const
{
	column_t*	column; 
	byte*		desttop;
	vdrawsfunc	drawfunc;
	int			colstep;
	int			destwidth, destheight;

	int			xinc, yinc, col, w, ymul, xmul;

	x0 = (width * x0) / 320;
	y0 = (height * y0) / 200;
	destwidth = (width * SHORT(patch->width)) / 320;
	destheight = (height * SHORT(patch->height)) / 200;

	xinc = (SHORT(patch->width) << 16) / destwidth;
	yinc = (SHORT(patch->height) << 16) / destheight;
	xmul = (destwidth << 16) / SHORT(patch->width);
	ymul = (destheight << 16) / SHORT(patch->height);

	y0 -= (SHORT(patch->topoffset) * ymul) >> 16;
	x0 -= (SHORT(patch->leftoffset) * xmul) >> 16;

#ifdef RANGECHECK 
	if (x0<0
		|| x0+destwidth > width
		|| y0<0
		|| y0+destheight> height)
	{
		//Printf ("Patch at %d,%d exceeds LFB\n", x0,y0 );
		DPrintf ("DCanvas::DrawPatchFlipped: bad patch (ignored)\n");
		return;
	}
#endif

	if (is8bit) {
		drawfunc = Psfuncs[EWrapper_Normal];
		colstep = 1;
	} else {
		drawfunc = Dsfuncs[EWrapper_Normal];
		colstep = 4;
	}

	w = destwidth * xinc;
	col = w - xinc;
	desttop = buffer + y0*pitch + x0 * colstep;

	for ( ; col >= 0 ; col -= xinc, desttop += colstep)
	{
		column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> 16]));

		// step through the posts in a column
		while (column->topdelta != 0xff )
		{
			drawfunc ((byte *)column + 3,
					  desttop + (((column->topdelta * ymul)) >> 16) * pitch,
					  (column->length * ymul) >> 16,
					  pitch,
					  yinc);
			column = (column_t *)(	(byte *)column + column->length
									+ 4 );
		}
	}
}


//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//
void DCanvas::DrawBlock (int x, int y, int _width, int _height, const byte *src) const
{
	byte *dest;

#ifdef RANGECHECK
	if (x<0
		||x+_width > width
		|| y<0
		|| y+_height>height)
	{
		I_Error ("Bad DCanvas::DrawBlock");
	}
#endif

	x <<= (is8bit) ? 0 : 2;
	_width <<= (is8bit) ? 0 : 2;

	dest = buffer + y*pitch + x;

	do
	{
		memcpy (dest, src, _width);
		src += _width;
		dest += pitch;
	} while (--_height);
}

void DCanvas::DrawPageBlock (const byte *src) const
{
	if (width == 320 && height == 200)
	{
		DrawBlock (0, 0, 320, 200, src);
		return;
	}

	byte *dest;
	fixed_t acc;
	fixed_t xinc, yinc;
	int h;

	dest = buffer;
	xinc = 320 * FRACUNIT / width;
	yinc = 200 * FRACUNIT / height;
	acc = 0;
	h = height;

	const byte *copysrc = NULL;

	do
	{
		if (copysrc)
		{
			memcpy (dest, copysrc, width);
			dest += pitch;
		}
		else
		{
			int w = width;
			fixed_t xfrac = 0;
			do
			{
				*dest++ = src[xfrac >> FRACBITS];
				xfrac += xinc;
			} while (--w);
			dest += pitch - width;
		}
		acc += yinc;
		if (acc >= FRACUNIT)
		{
			src += (acc >> FRACBITS) * 320;
			acc &= FRACUNIT-1;
			copysrc = NULL;
		}
		else
		{
			copysrc = dest - pitch;
		}
	} while (--h);
}

//
// V_GetBlock
// Gets a linear block of pixels from the view buffer.
//
void DCanvas::GetBlock (int x, int y, int _width, int _height, byte *dest) const
{
	const byte *src;

#ifdef RANGECHECK 
	if (x<0
		||x+_width > width
		|| y<0
		|| y+_height>height)
	{
		I_Error ("Bad V_GetBlock");
	}
#endif

	x <<= (is8bit) ? 0 : 2;
	_width <<= (is8bit) ? 0 : 2;

	src = buffer + y*pitch + x;

	while (_height--)
	{
		memcpy (dest, src, _width);
		src += pitch;
		dest += _width;
	}
}

//
// 
void DCanvas::DrawMaskedBlock (int x, int y, int _width, int _height,
	const byte *src, const byte *colors) const
{
	byte *dest;
	int span;

#ifdef RANGECHECK
	if ( x < 0 || x + _width > width
		|| y<0 || y+_height>height)
	{
		return;
	}
#endif

	dest = buffer + y*pitch + x;
	span = pitch - _width;

	if (colors != NULL)
	{
		do
		{
			int i;

			for (i = _width; i > 0; i--, dest++)
			{
				byte val = *src++;
				if (val != 255)
				{
					*dest = colors[val];
				}
			}
			dest += span;
		}
		while (--_height);
	}
	else
	{
		do
		{
			int i;

			for (i = _width; i > 0; i--, dest++)
			{
				byte val = *src++;
				if (val != 255)
				{
					*dest = val;
				}
			}
			dest += span;
		}
		while (--_height);
	}
}

void DCanvas::ScaleMaskedBlock (int x, int y, int _width, int _height,
	int dwidth, int dheight, const byte *src, const byte *colors) const
{
	byte *dest;
	int span;
	fixed_t err;
	fixed_t xinc, yinc;

	if (dwidth == _width && dheight == _height)
	{
		DrawMaskedBlock (x, y, _width, _height, src, colors);
		return;
	}

#ifdef RANGECHECK
	if ( x < 0 || x + dwidth > width
		|| y<0 || y+dheight>height)
	{
		I_Error ("Bad DCanvas::ScaleMaskedBlock");
	}
#endif

	dest = buffer + y*pitch + x;
	span = pitch - dwidth;
	err = 0;
	xinc = (_width << FRACBITS) / dwidth;
	yinc = (_height << FRACBITS) / dheight;

	if (colors != NULL)
	{
		do
		{
			int i, x;

			for (i = dwidth, x = 0; i > 0; i--, dest++, x += xinc)
			{
				byte in = src[x >> FRACBITS];
				if (in != 255)
				{
					*dest = colors[in];
				}
			}
			dest += span;
			err += yinc;
			while (err >= FRACUNIT)
			{
				src += _width;
				err -= FRACUNIT;
			}
		}
		while (--dheight);
	}
	else
	{
		do
		{
			int i, x;

			for (i = dwidth, x = 0; i > 0; i--, dest++, x += xinc)
			{
				byte in = src[x >> FRACBITS];
				if (in != 255)
				{
					*dest = in;
				}
			}
			dest += span;
			err += yinc;
			while (err >= FRACUNIT)
			{
				src += _width;
				err -= FRACUNIT;
			}
		}
		while (--dheight);
	}
}

void DCanvas::DrawTranslucentMaskedBlock (int x, int y, int _width, int _height,
	const byte *src, const byte *colors, fixed_t fade) const
{
	byte *dest;
	int span;

#ifdef RANGECHECK
	if ( x < 0 || x + _width > width
		|| y<0 || y+_height>height)
	{
		return;
	}
#endif

	unsigned int *fg2rgb, *bg2rgb;

	{
		fixed_t fglevel, bglevel;

		fglevel = fade & ~0x3ff;
		bglevel = FRACUNIT - fglevel;
		fg2rgb = Col2RGB8[fglevel>>10];
		bg2rgb = Col2RGB8[bglevel>>10];
	}

	dest = buffer + y*pitch + x;
	span = pitch - _width;

	if (colors != NULL)
	{
		do
		{
			int i;

			for (i = _width; i > 0; i--, dest++)
			{
				byte val = *src++;
				if (val != 255)
				{
					unsigned int fg;
					fg = (fg2rgb[colors[val]] + bg2rgb[*dest]) | 0x1f07c1f;
					*dest = RGB32k[0][0][fg & (fg>>15)];
				}
			}
			dest += span;
		}
		while (--_height);
	}
	else
	{
		do
		{
			int i;

			for (i = _width; i > 0; i--, dest++)
			{
				byte val = *src++;
				if (val != 255)
				{
					unsigned int fg;
					fg = (fg2rgb[val] + bg2rgb[*dest]) | 0x1f07c1f;
					*dest = RGB32k[0][0][fg & (fg>>15)];
				}
			}
			dest += span;
		}
		while (--_height);
	}
}

void DCanvas::ScaleTranslucentMaskedBlock (int x, int y, int _width, int _height,
	int dwidth, int dheight, const byte *src, const byte *colors, fixed_t fade) const
{
	byte *dest;
	int span;
	fixed_t err;
	fixed_t xinc, yinc;

	if (dwidth == _width && dheight == _height)
	{
		DrawTranslucentMaskedBlock (x, y, _width, _height, src, colors, fade);
		return;
	}

#ifdef RANGECHECK
	if ( x < 0 || x + dwidth > width
		|| y<0 || y+dheight>height)
	{
		return;
	}
#endif

	unsigned int *fg2rgb, *bg2rgb;

	{
		fixed_t fglevel, bglevel;

		fglevel = fade & ~0x3ff;
		bglevel = FRACUNIT - fglevel;
		fg2rgb = Col2RGB8[fglevel>>10];
		bg2rgb = Col2RGB8[bglevel>>10];
	}

	dest = buffer + y*pitch + x;
	span = pitch - dwidth;
	err = 0;
	xinc = (_width << FRACBITS) / dwidth;
	yinc = (_height << FRACBITS) / dheight;

	if (colors != NULL)
	{
		do
		{
			int i, x;

			for (i = dwidth, x = 0; i > 0; i--, dest++, x += xinc)
			{
				byte in = src[x >> FRACBITS];
				if (in != 255)
				{
					unsigned int fg;
					fg = (fg2rgb[colors[in]] + bg2rgb[*dest]) | 0x1f07c1f;
					*dest = RGB32k[0][0][fg & (fg>>15)];
				}
			}
			dest += span;
			err += yinc;
			while (err >= FRACUNIT)
			{
				src += _width;
				err -= FRACUNIT;
			}
		}
		while (--dheight);
	}
	else
	{
		do
		{
			int i, x;

			for (i = dwidth, x = 0; i > 0; i--, dest++, x += xinc)
			{
				byte in = src[x >> FRACBITS];
				if (in != 255)
				{
					unsigned int fg;
					fg = (fg2rgb[in] + bg2rgb[*dest]) | 0x1f07c1f;
					*dest = RGB32k[0][0][fg & (fg>>15)];
				}
			}
			dest += span;
			err += yinc;
			while (err >= FRACUNIT)
			{
				src += _width;
				err -= FRACUNIT;
			}
		}
		while (--dheight);
	}
}

void DCanvas::DrawShadowedMaskedBlock (int x, int y, int _width, int _height,
	const byte *src, const byte *colors, fixed_t shade) const
{
	byte *dest;
	int span;

#ifdef RANGECHECK
	if ( x < 0 || x + _width > width
		|| y<0 || y+_height>height)
	{
		return;
	}
#endif

	unsigned int fg, *bg2rgb;

	{
		fixed_t fglevel, bglevel;

		fglevel = shade & ~0x3ff;
		bglevel = FRACUNIT - fglevel;
		fg = Col2RGB8[fglevel>>10][0];
		bg2rgb = Col2RGB8[bglevel>>10];
	}

	dest = buffer + y*pitch + x;
	span = pitch - _width;

	if (colors != NULL)
	{
		do
		{
			int i;

			for (i = _width; i > 0; i--, dest++)
			{
				byte val = *src++;
				if (val != 255)
				{
					unsigned int bg = bg2rgb[*(dest+pitch*2+2)];
					bg = (fg + bg) | 0x1f07c1f;
					*(dest+pitch*2+2) = RGB32k[0][0][bg & (bg>>15)];
					*dest = colors[val];
				}
			}
			dest += span;
		}
		while (--_height);
	}
	else
	{
		do
		{
			int i;

			for (i = _width; i > 0; i--, dest++)
			{
				byte val = *src++;
				if (val != 255)
				{
					unsigned int bg = bg2rgb[*(dest+pitch*2+2)];
					bg = (fg + bg) | 0x1f07c1f;
					*(dest+pitch*2+2) = RGB32k[0][0][bg & (bg>>15)];
					*dest = val;
				}
			}
			dest += span;
		}
		while (--_height);
	}
}

void DCanvas::ScaleShadowedMaskedBlock (int x, int y, int _width, int _height,
	int dwidth, int dheight, const byte *src, const byte *colors, fixed_t shade) const
{
	byte *dest;
	int span;
	fixed_t err;
	fixed_t xinc, yinc;

	if (dwidth == _width && dheight == _height)
	{
		DrawShadowedMaskedBlock (x, y, _width, _height, src, colors, shade);
		return;
	}

#ifdef RANGECHECK
	if ( x < 0 || x + dwidth > width
		|| y<0 || y+dheight>height)
	{
		return;
	}
#endif

	unsigned int fg, *bg2rgb;

	{
		fixed_t fglevel, bglevel;

		fglevel = shade & ~0x3ff;
		bglevel = FRACUNIT - fglevel;
		fg = Col2RGB8[fglevel>>10][0];
		bg2rgb = Col2RGB8[bglevel>>10];
	}

	dest = buffer + y*pitch + x;
	span = pitch - dwidth;
	err = 0;
	xinc = (_width << FRACBITS) / dwidth;
	yinc = (_height << FRACBITS) / dheight;

	if (colors != NULL)
	{
		do
		{
			int i, x;

			for (i = dwidth, x = 0; i > 0; i--, dest++, x += xinc)
			{
				byte in = src[x >> FRACBITS];
				if (in != 255)
				{
					unsigned int bg;
					bg = (fg + bg2rgb[*(dest+pitch*2+2)]) | 0x1f07c1f;
					*(dest+pitch*2+2) = RGB32k[0][0][bg & (bg>>15)];
					*dest = colors[in];
				}
			}
			dest += span;
			err += yinc;
			while (err >= FRACUNIT)
			{
				src += _width;
				err -= FRACUNIT;
			}
		}
		while (--dheight);
	}
	else
	{
		do
		{
			int i, x;

			for (i = dwidth, x = 0; i > 0; i--, dest++, x += xinc)
			{
				byte in = src[x >> FRACBITS];
				if (in != 255)
				{
					unsigned int bg;
					bg = (fg + bg2rgb[*(dest+pitch*2+2)]) | 0x1f07c1f;
					*(dest+pitch*2+2) = RGB32k[0][0][bg & (bg>>15)];
					*dest = in;
				}
			}
			dest += span;
			err += yinc;
			while (err >= FRACUNIT)
			{
				src += _width;
				err -= FRACUNIT;
			}
		}
		while (--dheight);
	}
}

void DCanvas::DrawAlphaMaskedBlock (int x, int y, int _width, int _height,
	const byte *src, int color) const
{
#ifdef RANGECHECK
	if ( x < 0 || x + _width > width
		|| y<0 || y+_height>height)
	{
		return;
	}
#endif

	unsigned int *fgstart = &Col2RGB8[0][color];
	byte *dest = buffer + y*pitch + x;
	int span = pitch - _width;

	do
	{
		int i;

		for (i = _width; i > 0; i--, dest++)
		{
			unsigned int val = *src++;
			if (val != 255)
			{
				val = (val + 2) >> 2;
				unsigned int fg = fgstart[val<<8];
				val = Col2RGB8[64-val][*dest];
				val = (val + fg) | 0x1f07c1f;
				*dest = RGB32k[0][0][val & (val>>15)];
			}
		}
		dest += span;
	}
	while (--_height);
}

void DCanvas::ScaleAlphaMaskedBlock (int x, int y, int _width, int _height,
	int dwidth, int dheight, const byte *src, int color) const
{

	if (dwidth == _width && dheight == _height)
	{
		DrawAlphaMaskedBlock (x, y, _width, _height, src, color);
		return;
	}

#ifdef RANGECHECK
	if ( x < 0 || x + dwidth > width
		|| y<0 || y+dheight>height)
	{
		return;
	}
#endif

	unsigned int *fgstart = &Col2RGB8[0][color];
	byte *dest = buffer + y*pitch + x;
	int span = pitch - _width;
	fixed_t err;
	fixed_t xinc, yinc;

	dest = buffer + y*pitch + x;
	span = pitch - dwidth;
	err = 0;
	xinc = (_width << FRACBITS) / dwidth;
	yinc = (_height << FRACBITS) / dheight;

	do
	{
		int i, x;

		for (i = dwidth, x = 0; i > 0; i--, dest++, x += xinc)
		{
			unsigned int val = src[x >> FRACBITS];
			if (val != 255)
			{
				val = (val + 2) >> 2;
				unsigned int fg = fgstart[val<<8];
				val = Col2RGB8[64-val][*dest];
				val = (fg + val) | 0x1f07c1f;
				*dest = RGB32k[0][0][val & (val>>15)];
			}
		}
		dest += span;
		err += yinc;
		while (err >= FRACUNIT)
		{
			src += _width;
			err -= FRACUNIT;
		}
	}
	while (--dheight);
}
