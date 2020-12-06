#include <ctype.h>

#include "v_text.h"

#include "i_system.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "w_wad.h"
#include "z_zone.h"
#include "m_swap.h"

#include "doomstat.h"


extern patch_t *hu_font[HU_FONTSIZE];


static byte *ConChars;

byte *WhiteMap;
byte *Ranges;

// Convert the CONCHARS patch into the internal format used by
// the console font drawer.
void V_InitConChars (byte transcolor)
{
	byte *d, *s, v, *src;
	patch_t *chars;
	int x, y, z, a;
	screen_t temp;

	temp.impdata = NULL;
	if (!V_AllocScreen (&temp, 128, 128, 8))
		I_FatalError ("Could not create console characters");

	V_LockScreen (&temp);

	chars = W_CacheLumpName ("CONCHARS", PU_CACHE);
	{
		long *screen, fill;

		fill = (transcolor << 24) | (transcolor << 16) | (transcolor << 8) | transcolor;
		for (y = 0; y < 128; y++) {
			screen = (long *)(temp.buffer + temp.pitch * y);
			for (x = 0; x < 128/4; x++) {
				*screen++ = fill;
			}
		}
		V_DrawPatch (0, 0, &temp, chars);
	}
	src = temp.buffer;
	if ( (ConChars = Z_Malloc (256*8*8*2, PU_STATIC, 0)) ) {
		d = ConChars;
		for (y = 0; y < 16; y++) {
			for (x = 0; x < 16; x++) {
				s = src + x * 8 + (y * 8 * temp.pitch);
				for (z = 0; z < 8; z++) {
					for (a = 0; a < 8; a++) {
						v = s[a];
						if (v == transcolor) {
							d[a] = 0x00;
							d[a+8] = 0xff;
						} else {
							d[a] = v;
							d[a+8] = 0x00;
						}
					}
					d += 16;
					s += temp.pitch;
				}
			}
		}
	}

	V_UnlockScreen (&temp);
	V_FreeScreen (&temp);
}


//
// V_PrintStr
// Print a line of text using the console font
//

extern void STACK_ARGS PrintChar1P (long *charimg, byte *dest, int screenpitch);
extern void STACK_ARGS PrintChar2P_MMX (long *charimg, byte *dest, int screenpitch);

void V_PrintStr (int x, int y, byte *str, int count)
{
	byte *temp;
	long *charimg;
	
	if (y > (screens[0].height - 8))
		return;

	if (x < 0) {
		int skip;

		skip = -(x - 7) / 8;
		x += skip * 8;
		if (count <= skip) {
			return;
		} else {
			count -= skip;
			str += skip;
		}
	}

	x &= ~3;
	temp = screens[0].buffer + y * screens[0].pitch;

	while (count && x <= (screens[0].width - 8)) {
		charimg = (long *)&ConChars[(*str) * 128];
		if (screens[0].is8bit) {
#ifdef USEASM
			PrintChar1P (charimg, temp + x, screens[0].pitch);
#else
			int z;
			long *writepos;

			writepos = (long *)(temp + x);
			for (z = 0; z < 8; z++) {
				*writepos = (*writepos & charimg[2]) ^ charimg[0];
				writepos++;
				*writepos = (*writepos & charimg[3]) ^ charimg[1];
				writepos += (screens[0].pitch >> 2) - 1;
				charimg += 4;
			}
#endif
		} else {
			int z;
			int *writepos;

			writepos = (int *)(temp + (x << 2));
			for (z = 0; z < 8; z++) {
#define BYTEIMG ((byte *)charimg)
#define SPOT(a) \
	writepos[a] = (writepos[a] & \
				 ((BYTEIMG[a+8]<<16)|(BYTEIMG[a+8]<<8)|(BYTEIMG[a+8]))) \
				 ^ V_Palette[BYTEIMG[a]]

				SPOT(0);
				SPOT(1);
				SPOT(2);
				SPOT(3);
				SPOT(4);
				SPOT(5);
				SPOT(6);
				SPOT(7);
#undef SPOT
#undef BYTEIMG
				writepos += screens[0].pitch >> 2;
				charimg += 4;
			}
		}
		str++;
		count--;
		x += 8;
	}
}

//
// V_PrintStr2
// Same as V_PrintStr but doubles the size of every character.
//
void V_PrintStr2 (int x, int y, byte *str, int count)
{
	byte *temp;
	long *charimg;
	
	if (y > (screens[0].height - 16))
		return;

	if (x < 0) {
		int skip;

		skip = -(x - 15) / 16;
		x += skip * 16;
		if (count <= skip) {
			return;
		} else {
			count -= skip;
			str += skip;
		}
	}

	x &= ~3;
	temp = screens[0].buffer + y * screens[0].pitch;

	while (count && x <= (screens[0].width - 16)) {
		charimg = (long *)&ConChars[(*str) * 128];
#ifdef USEASM
		if (UseMMX) {
			PrintChar2P_MMX (charimg, temp + x, screens[0].pitch);
		} else
#endif
		{
			int z;
			byte *buildmask, *buildbits, *image;
			unsigned int m1, s1;
			unsigned int *writepos;

			writepos = (unsigned int *)(temp + x);
			buildbits = (byte *)&s1;
			buildmask = (byte *)&m1;
			image = (byte *)charimg;

			for (z = 0; z < 8; z++) {
				buildmask[0] = buildmask[1] = image[8];
				buildmask[2] = buildmask[3] = image[9];
				buildbits[0] = buildbits[1] = image[0];
				buildbits[2] = buildbits[3] = image[1];
				writepos[0] = (writepos[0] & m1) ^ s1;
				writepos[screens[0].pitch/4] = (writepos[screens[0].pitch/4] & m1) ^ s1;

				buildmask[0] = buildmask[1] = image[10];
				buildmask[2] = buildmask[3] = image[11];
				buildbits[0] = buildbits[1] = image[2];
				buildbits[2] = buildbits[3] = image[3];
				writepos[1] = (writepos[1] & m1) ^ s1;
				writepos[1+screens[0].pitch/4] = (writepos[1+screens[0].pitch/4] & m1) ^ s1;

				buildmask[0] = buildmask[1] = image[12];
				buildmask[2] = buildmask[3] = image[13];
				buildbits[0] = buildbits[1] = image[4];
				buildbits[2] = buildbits[3] = image[5];
				writepos[2] = (writepos[2] & m1) ^ s1;
				writepos[2+screens[0].pitch/4] = (writepos[2+screens[0].pitch/4] & m1) ^ s1;

				buildmask[0] = buildmask[1] = image[14];
				buildmask[2] = buildmask[3] = image[15];
				buildbits[0] = buildbits[1] = image[6];
				buildbits[2] = buildbits[3] = image[7];
				writepos[3] = (writepos[3] & m1) ^ s1;
				writepos[3+screens[0].pitch/4] = (writepos[3+screens[0].pitch/4] & m1) ^ s1;

				writepos += screens[0].pitch >> 1;
				image += 16;
			}

		}
		str++;
		count--;
		x += 16;
	}
#ifdef USEASM
	ENDMMX;
#endif
}

//
//		Write a string using the hu_font
//
extern patch_t *hu_font[HU_FONTSIZE];

void V_DrawText (int x, int y, byte *string)
{
	int 		w;
	byte*		ch;
	int 		c;
	int 		cx;
	int 		cy;
	int			drawer;

	V_ColorMap = WhiteMap;

	ch = string;
	cx = x;
	cy = y;
		
	while (1) {
		c = *ch++;
		if (!c)
			break;
		if (c & 0x80) {
			drawer = V_DRAWPATCH;
			c &= 0x7f;
		} else {
			drawer = V_DRAWTRANSLATEDPATCH;
		}

		if (c == '\n') {
			cx = x;
			cy += 12;
			continue;
		}

		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || c>= HU_FONTSIZE) {
			cx += 4;
			continue;
		}
				
		w = SHORT (hu_font[c]->width);
		if (cx+w > screens[0].width)
			break;

		V_DrawWrapper (drawer, cx, cy, &screens[0], hu_font[c]);
		cx+=w;
	}
}

void V_DrawTextClean (int x, int y, byte *string)
{
	int 		w;
	byte*		ch;
	int 		c;
	int 		cx;
	int 		cy;
	int			drawer;

	V_ColorMap = WhiteMap;

	ch = string;
	cx = x;
	cy = y;
		
	while (1) {
		c = *ch++;
		if (!c)
			break;
		if (c & 0x80) {
			drawer = V_DRAWPATCH;
			c &= 0x7f;
		} else {
			drawer = V_DRAWTRANSLATEDPATCH;
		}

		if (c == '\n') {
			cx = x;
			cy += 12 * CleanYfac;
			continue;
		}

		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || c>= HU_FONTSIZE) {
			cx += 4 * CleanXfac;
			continue;
		}
				
		w = SHORT (hu_font[c]->width) * CleanXfac;
		if (cx+w > screens[0].width)
			break;

		V_DrawCNMWrapper (drawer, cx, cy, &screens[0], hu_font[c]);
		cx += w;
	}
}

void V_DrawWhiteText (int x, int y, byte *string)
{
	int 		w;
	byte*		ch;
	int 		c;
	int 		cx;
	int 		cy;

	V_ColorMap = WhiteMap;			

	ch = string;
	cx = x;
	cy = y;
		
	while(1) {
		c = *ch++;
		if (!c)
			break;
		c &= 0x7f;

		if (c == '\n') {
			cx = x;
			cy += 12;
			continue;
		}

		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || c>= HU_FONTSIZE) {
			cx += 4;
			continue;
		}
				
		w = SHORT (hu_font[c]->width);
		if (cx+w > screens[0].width)
			break;
		V_DrawTranslatedPatch (cx, cy, &screens[0], hu_font[c]);
		cx+=w;
	}
}

void V_DrawRedText (int x, int y, byte *string)
{
	int 		w;
	byte*		ch;
	int 		c;
	int 		cx;
	int 		cy;
				

	ch = string;
	cx = x;
	cy = y;
		
	while (1) {
		c = *ch++;
		if (!c)
			break;
		c &= 0x7f;

		if (c == '\n') {
			cx = x;
			cy += 12;
			continue;
		}

		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || c>= HU_FONTSIZE) {
			cx += 4;
			continue;
		}
				
		w = SHORT (hu_font[c]->width);
		if (cx+w > screens[0].width)
			break;
		V_DrawPatch (cx, cy, &screens[0], hu_font[c]);
		cx+=w;
	}
}

void V_DrawWhiteTextClean (int x, int y, byte *string)
{
	int 		w;
	byte*		ch;
	int 		c;
	int 		cx;
	int 		cy;

	V_ColorMap = WhiteMap;

	ch = string;
	cx = x;
	cy = y;
		
	while (1) {
		c = *ch++;
		if (!c)
			break;
		c &= 0x7f;

		if (c == '\n') {
			cx = x;
			cy += 12;
			continue;
		}

		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || c>= HU_FONTSIZE) {
			cx += 4;
			continue;
		}
				
		w = SHORT (hu_font[c]->width);
		if (cx+w > 320)
			break;
		V_DrawTranslatedPatchClean (cx, cy, &screens[0], hu_font[c]);
		cx+=w;
	}
}

void V_DrawRedTextClean (int x, int y, byte *string)
{
	int 		w;
	byte*		ch;
	int 		c;
	int 		cx;
	int 		cy;
				

	ch = string;
	cx = x;
	cy = y;
		
	while (1) {
		c = *ch++;
		if (!c)
			break;
		c &= 0x7f;

		if (c == '\n') {
			cx = x;
			cy += 12;
			continue;
		}

		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || c>= HU_FONTSIZE) {
			cx += 4;
			continue;
		}
				
		w = SHORT (hu_font[c]->width);
		if (cx+w > 320)
			break;
		V_DrawPatchClean (cx, cy, &screens[0], hu_font[c]);
		cx+=w;
	}
}

//
// Find string width from hu_font chars
//
int V_StringWidth (byte *string)
{
	int w = 0, c;
		
	while (*string) {
		c = toupper((*string++) & 0x7f) - HU_FONTSTART;
		if (c < 0 || c >= HU_FONTSIZE)
			w += 4;
		else
			w += SHORT (hu_font[c]->width);
	}
				
	return w;
}

//
// [RH] Break long lines of text into multiple lines no
//		longer than maxwidth pixels.
//
static void breakit (brokenlines_t *line, const byte *start, const byte *string)
{
	// Leave out trailing white space
	while (string > start && isspace (*(string - 1)))
		string--;

	line->string = Z_Malloc (string - start + 1, PU_STATIC, 0);
	strncpy (line->string, start, string - start);
	line->string[string - start] = 0;
	line->width = V_StringWidth (line->string);
}

brokenlines_t *V_BreakLines (int maxwidth, const byte *string)
{
	brokenlines_t lines[128];	// Support up to 128 lines (should be plenty)

	const byte *space = NULL, *start = string;
	int i, c, w, nw;
	BOOL lastWasSpace = false;

	i = w = 0;

	while ( (c = *string++) ) {
		if (isspace(c)) {
			if (!lastWasSpace) {
				space = string - 1;
				lastWasSpace = true;
			}
		} else
			lastWasSpace = false;

		c = toupper (c & 0x7f) - HU_FONTSTART;

		if (c < 0 || c >= HU_FONTSIZE)
			nw = 4;
		else
			nw = SHORT (hu_font[c]->width);

		if (w + nw > maxwidth || c == '\n' - HU_FONTSTART) {	// Time to break the line
			if (!space)
				space = string - 1;

			breakit (&lines[i], start, space);

			i++;
			w = 0;
			lastWasSpace = false;
			start = space;
			space = NULL;

			while (*start && isspace (*start))
				start++;
			string = start;
		} else
			w += nw;
	}

	if (string - start > 1) {
		const byte *s = start;

		while (s < string) {
			if (!isspace (*s++)) {
				breakit (&lines[i++], start, string);
				break;
			}
		}
	}

	{
		// Make a copy of the broken lines and return them
		brokenlines_t *broken =
			Z_Malloc (sizeof(brokenlines_t) * (i + 1), PU_STATIC, 0);

		memcpy (broken, lines, sizeof(brokenlines_t) * i);
		broken[i].string = NULL;
		broken[i].width = -1;

		return broken;
	}
}

void V_FreeBrokenLines (brokenlines_t *lines)
{
	if (lines) {
		int i = 0;

		while (lines[i].width != -1) {
			Z_Free (lines[i].string);
			lines[i].string = NULL;
			i++;
		}
		Z_Free (lines);
	}
}