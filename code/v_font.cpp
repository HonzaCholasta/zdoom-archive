#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "doomtype.h"
#include "m_swap.h"
#include "v_font.h"
#include "v_video.h"
#include "w_wad.h"
#include "r_data.h"
#include "z_zone.h"
#include "i_system.h"

FFont::FFont (const char *nametemplate, int first, int count, int start)
{
	int neededsize;
	int i, lump;
	char buffer[12];
	byte usedcolors[256], translated[256], identity[256];
	double *luminosity;

	Chars = new CharData[count];
	Bitmaps = NULL;
	Ranges = NULL;
	FirstChar = first;
	LastChar = first + count - 1;
	FontHeight = 0;
	memset (usedcolors, 0, 256);

	for (i = neededsize = 0; i < count; i++)
	{
		sprintf (buffer, nametemplate, i + start);
		lump = W_CheckNumForName (buffer);
		if (lump >= 0)
		{
			patch_t *patch = (patch_t *)W_CacheLumpNum (lump, PU_CACHE);
			Chars[i].Width = SHORT(patch->width);
			Chars[i].Height = SHORT(patch->height);
			Chars[i].XOffs = SHORT(patch->leftoffset);
			Chars[i].YOffs = SHORT(patch->topoffset);
			if (Chars[i].Height > FontHeight)
				FontHeight = Chars[i].Height;
			neededsize += Chars[i].Width * Chars[i].Height;
			RecordPatchColors (patch, usedcolors);
		}
		else
		{
			Chars[i].Width = Chars[i].Height = 0;
			Chars[i].XOffs = Chars[i].YOffs = 0;
		}
	}

	SpaceWidth = (Chars['N' - first].Width + 1) / 2;
	ActiveColors = SimpleTranslation (usedcolors, translated, identity, &luminosity);
	Bitmaps = new byte[neededsize];
	memset (Bitmaps, 255, neededsize);

	for (i = neededsize = 0; i < count; i++)
	{
		sprintf (buffer, nametemplate, i + start);
		lump = W_CheckNumForName (buffer);
		if (lump >= 0)
		{
			Chars[i].Data = Bitmaps + neededsize;
			RawDrawPatch ((patch_t *)W_CacheLumpNum (lump, PU_CACHE),
				Chars[i].Data, translated);
			neededsize += Chars[i].Width * Chars[i].Height;
		}
		else
		{
			Chars[i].Data = NULL;
		}
	}

	BuildTranslations (usedcolors, translated, identity, luminosity);

	delete[] luminosity;
}

FFont::FFont (int firstlump, int first, int count)
{
	int neededsize;
	int i;
	byte usedcolors[256], translated[256], identity[256];
	double *luminosity;

	memset (usedcolors, 0, 256);
	Bitmaps = NULL;
	Ranges = NULL;
	FirstChar = first;
	LastChar = first + count - 1;
	FontHeight = 0;
	Chars = new CharData[count];

	for (i = neededsize = 0; i < count; i++)
	{
		patch_t *patch = (patch_t *)W_CacheLumpNum (firstlump + i, PU_CACHE);
		Chars[i].Width = SHORT(patch->width);
		Chars[i].Height = SHORT(patch->height);
		if (Chars[i].Height > FontHeight)
			FontHeight = Chars[i].Height;
		Chars[i].XOffs = SHORT(patch->leftoffset);
		Chars[i].YOffs = SHORT(patch->topoffset);
		neededsize += Chars[i].Width * Chars[i].Height;
		RecordPatchColors (patch, usedcolors);
	}

	SpaceWidth = (Chars['N' - first].Width + 1) / 2;
	ActiveColors = SimpleTranslation (usedcolors, translated, identity, &luminosity);
	Bitmaps = new byte[neededsize];
	memset (Bitmaps, 255, neededsize);

	for (i = neededsize = 0; i < count; i++)
	{
		Chars[i].Data = Bitmaps + neededsize;
		RawDrawPatch ((patch_t *)W_CacheLumpNum (firstlump + i, PU_CACHE), Chars[i].Data, translated);
		neededsize += Chars[i].Width * Chars[i].Height;
	}

	BuildTranslations (usedcolors, translated, identity, luminosity);

	delete[] luminosity;
}

FFont::~FFont ()
{
	if (Chars)
	{
		delete[] Chars;
		Chars = NULL;
	}
	if (Ranges)
	{
		delete[] Ranges;
		Ranges = NULL;
	}
}

void RecordPatchColors (patch_t *patch, byte *usedcolors)
{
	int x;

	for (x = SHORT(patch->width) - 1; x >= 0; x--)
	{
		column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[x]));

		while (column->topdelta != 0xff)
		{
			byte *source = (byte *)column + 3;
			int count = column->length;

			do
			{
				usedcolors[*source++] = 1;
			} while (--count);

			column = (column_t *)(source + 1);
		}
	}
}

void RawDrawPatch (patch_t *patch, byte *out, byte *tlate)
{
	int width = SHORT(patch->width);
	int height = SHORT(patch->height);
	byte *desttop = out;
	int x;

	for (x = 0; x < width; x++, desttop++)
	{
		column_t *column = (column_t *)((byte *)patch + LONG(patch->columnofs[x]));

		while (column->topdelta != 0xff)
		{
			byte *source = (byte *)column + 3;
			byte *dest = desttop + column->topdelta * width;
			int count = column->length;

			do
			{
				*dest = tlate[*source++];
				dest += width;
			} while (--count);

			column = (column_t *)(source + 1);
		}
	}
}

static int STACK_ARGS compare (const void *arg1, const void *arg2)
{
	if (RPART(DefaultPalette->basecolors[*((byte *)arg1)]) * 0.299 +
		GPART(DefaultPalette->basecolors[*((byte *)arg1)]) * 0.587 +
		BPART(DefaultPalette->basecolors[*((byte *)arg1)]) * 0.114  <
		RPART(DefaultPalette->basecolors[*((byte *)arg2)]) * 0.299 +
		GPART(DefaultPalette->basecolors[*((byte *)arg2)]) * 0.587 +
		BPART(DefaultPalette->basecolors[*((byte *)arg2)]) * 0.114)
		return -1;
	else
		return 1;
}

int FFont::SimpleTranslation (byte *colorsused, byte *translation, byte *reverse, double **luminosity)
{
	double min, max, diver;
	int i, j;

	memset (translation, 255, 256);

	for (i = j = 0; i < 256; i++)
	{
		if (colorsused[i])
		{
			reverse[j++] = i;
		}
	}

	qsort (reverse, j, 1, compare);

	*luminosity = new double[j];
	max = 0.0;
	min = 100000000.0;
	for (i = 0; i < j; i++)
	{
		translation[reverse[i]] = i;

		(*luminosity)[i] = RPART(DefaultPalette->basecolors[reverse[i]]) * 0.299 +
							GPART(DefaultPalette->basecolors[reverse[i]]) * 0.587 +
							BPART(DefaultPalette->basecolors[reverse[i]]) * 0.114;
		if ((*luminosity)[i] > max)
			max = (*luminosity)[i];
		if ((*luminosity)[i] < min)
			min = (*luminosity)[i];
	}
	diver = 1.0 / (max - min);
	for (i = 0; i < j; i++)
	{
		(*luminosity)[i] = ((*luminosity)[i] - min) * diver;
	}

	return j;
}

void FFont::BuildTranslations (const byte *usedcolors, const byte *tlate, const byte *identity, const double *luminosity)
{
	byte colors[3*17];
	byte *playpal = (byte *)W_CacheLumpName ("PLAYPAL", PU_STATIC);
	int i, j, k;
	double v;
	byte *base;
	byte *range;

	range = Ranges = new byte[NUM_TEXT_COLORS * ActiveColors];

	// Create different translations for different color ranges
	for (i = 0; i < CR_UNTRANSLATED; i++)
	{
		base = colors;

		switch (i)
		{
		case CR_BRICK:
			for (v = 0.0, k = 17; k > 0; k--, v += 0.0625f)
			{
				*base++ = (int)((v * 184.0) + 71.0);	// R
				*base++ = (int)(v * 184.0);				// G
				*base++ = (int)(v * 184.0);				// B
			}
			break;

		case CR_TAN:
			for (v = 0.0, k = 17; k > 0; k--, v += 0.0625f)
			{
				*base++ = (int)((v * 204.0) + 51.0);	// R
				*base++ = (int)((v * 192.0) + 43.0);	// G
				*base++ = (int)((v * 204.0) + 19.0);	// B
			}
			break;

		case CR_GRAY:
			for (v = 0.0, k = 17; k > 0; k--, v += 0.0625f)
			{
				*base++ = (int)((v * 200.0) + 39.0);	// R
				*base++ = (int)((v * 200.0) + 39.0);	// G
				*base++ = (int)((v * 200.0) + 39.0);	// B
			}
			break;

		case CR_GREEN:
			for (v = 0.0, k = 17; k > 0; k--, v += 0.0625f)
			{
				*base++ = (int)((v * 108.0) + 11.0);	// R
				*base++ = (int)((v * 232.0) + 23.0);	// G
				*base++ = (int)((v * 104.0) + 7.0);		// B
			}
			break;

		case CR_BROWN:
			for (v = 0.0, k = 17; k > 0; k--, v += 0.0625f)
			{
				*base++ = (int)((v * 108.0) + 83.0);	// R
				*base++ = (int)((v * 104.0) + 63.0);	// G
				*base++ = (int)((v * 96.0) + 47.0);		// B
			}
			break;

		case CR_GOLD:
			for (v = 0.0, k = 17; k > 0; k--, v += 0.0625f)
			{
				*base++ = (int)((v * 140.0) + 115.0);	// R
				*base++ = (int)((v * 212.0) + 43.0);	// G
				*base++ = (int)((v * 115.0) + 0.0);		// B
			}
			break;

		case CR_RED:
			for (v = 0.0, k = 17; k > 0; k--, v += 0.0625f)
			{
				*base++ = (int)((v * 192.0) + 63.0);	// R
				*base++ = 0;							// G
				*base++ = 0;							// B
			}
			break;

		case CR_BLUE:
			for (v = 0.0, k = 17; k > 0; k--, v += 0.0625f)
			{
				*base++ = 0;							// R
				*base++ = 0;							// G
				*base++ = (int)((v * 216.0) + 39.0);	// B
			}
			break;

		case CR_ORANGE:
			for (v = 0.0, k = 17; k > 0; k--, v += 0.0625f)
			{
				*base++ = (int)(v * 223.0 + 32.f);
				*base++ = (int)(v * 128.0);
				*base++ = 0;
			}
			break;

		case CR_WHITE:		// Heretic/Hexen white
			for (v = 0.0, k = 17; k > 0; k--, v += 0.0625f)
			{
				*base++ = (int)((v * 219.0) + 36.0);
				*base++ = (int)((v * 219.0) + 36.0);
				*base++ = (int)((v * 219.0) + 36.0);
			}
			break;

		case CR_YELLOW:		// Heretic/Hexen yellow
			for (v = 0.0, k = 17; k > 0; k--, v += 0.0625f)
			{
				if (v <= 0.25)
				{
					*base++ = (int)((v * 168.0) + 39.0);
					*base++ = (int)((v * 168.0) + 39.0);
					*base++ = (int)((v * 168.0) + 39.0);
				}
				else if (v < 0.8125)
				{
					*base++ = (int)((v * 230.0) + 61.9);
					*base++ = (int)((v * 172.5) + 29.4);
					*base++ = 24;
				}
				else
				{
					*base++ = (int)((v * 46.8) + 205.2);
					*base++ = (int)((v * 210.6) - 2.6);
					*base++ = (int)((v * 292.5) - 195.5);
				}
			}
			break;
		}

		for (j = 0; j < ActiveColors; j++)
		{
			double p1 = luminosity[j];
			double i1 = p1 * 16.0;
			double index, frac;
			int r, g, b;

			frac = modf (i1, &index);

			if (p1 < 1.0)
			{
				double ifrac = 1.0 - frac;
				k = (int)index * 3;
				r = (int)(colors[k+0] * ifrac + colors[k+3] * frac);
				g = (int)(colors[k+1] * ifrac + colors[k+4] * frac);
				b = (int)(colors[k+2] * ifrac + colors[k+5] * frac);
			}
			else
			{
				r = colors[3*16+0];
				g = colors[3*16+1];
				b = colors[3*16+2];
			}
			*range++ = BestColor (DefaultPalette->basecolors, r, g, b, DefaultPalette->numcolors);
		}
	}

	// Store the identity translation
	memcpy (range, identity, ActiveColors);
}

byte *FFont::GetColorTranslation (EColorRange range) const
{
	if (range < NUM_TEXT_COLORS)
		return Ranges + ActiveColors * range;
	else
		return Ranges + ActiveColors * CR_UNTRANSLATED;
}

byte *FFont::GetChar (int code, int *const width, int *const height, int *const xoffs, int *const yoffs) const
{
	if (code > LastChar)
		code = toupper (code);
	if (code < FirstChar ||
		code > LastChar ||
		Chars[code - FirstChar].Data == NULL)
	{
		*width = SpaceWidth;
		return NULL;
	}
	else
	{
		code -= FirstChar;
		*width = Chars[code].Width;
		*height = Chars[code].Height;
		*xoffs = Chars[code].XOffs;
		*yoffs = Chars[code].YOffs;
		return Chars[code].Data;
	}
}

int FFont::GetCharWidth (int code) const
{
	if (code > LastChar)
	{
		code = toupper (code);
	}
	if (code < FirstChar ||
		code > LastChar ||
		Chars[code - FirstChar].Data == NULL)
	{
		return SpaceWidth;
	}
	else
	{
		return Chars[code - FirstChar].Width;
	}
}


FFont::FFont ()
{
	Bitmaps = NULL;
	Chars = NULL;
	Ranges = NULL;
}

FConsoleFont::FConsoleFont (int lump)
{
	byte *data = (byte *)W_CacheLumpNum (lump, PU_CACHE);
	byte *data_p;
	int w, h;
	int i;

	if (data[0] != 'F' || data[1] != 'O' || data[2] != 'N' || data[3] != '1')
	{
		I_FatalError ("Console font is not of type FON1");
	}

	Chars = new CharData[256];

	w = data[4] + data[5]*256;
	h = data[6] + data[7]*256;

	FontHeight = h;
	SpaceWidth = w;
	ActiveColors = 255;
	Bitmaps = new byte[w*h*256];
	FirstChar = 0;
	LastChar = 255;

	BuildTranslations ();

	data_p = data + 8;

	for (i = 0; i < 256; i++)
	{
		Chars[i].Width = w;
		Chars[i].Height = h;
		Chars[i].XOffs = 0;
		Chars[i].YOffs = 0;
		Chars[i].Data = Bitmaps + w*h*i;

		byte *dest = Chars[i].Data;
		int destSize = w*h;

		do
		{
			SBYTE code = *data_p++;
			if (code >= 0)
			{
				memcpy (dest, data_p, code+1);
				dest += code+1;
				data_p += code+1;
				destSize -= code+1;
			}
			else if (code != 0x80)
			{
				memset (dest, *data_p, (-code)+1);
				dest += (-code)+1;
				data_p++;
				destSize -= (-code)+1;
			}
		} while (destSize > 0);
	}
}

void FConsoleFont::BuildTranslations ()
{
	byte *range;
	range = Ranges = new byte[NUM_TEXT_COLORS * ActiveColors];
	int i, j, r, g, b;

	// Create different translations for different color ranges
	// These are not the same as FFont::BuildTranslations()
	for (i = 0; i < CR_UNTRANSLATED; i++)
	{
		for (j = 0; j < 127; j++)
		{
			switch (i)
			{
			case CR_BRICK:
				r = j*184/255 + 71;
				g = j*184/255;
				b = j*184/255;
				break;

			case CR_TAN:
				r = j*204/255 + 51;
				g = j*192/255 + 43;
				b = j*204/255 + 19;
				break;

			case CR_GRAY:
				r = g = b = j*200/255 + 39;
				break;

			case CR_GREEN:
				r = b = 0;
				g = j;
				break;

			case CR_BROWN:
				r = j;
				g = j/2;
				b = 0;
				break;

			case CR_GOLD:
				r = j;
				g = j*3/2;
				b = j/2;
				break;

			case CR_RED:
				r = j;
				g = b = 0;
				break;

			case CR_BLUE:
				b = j;
				r = g = 0;
				break;

			case CR_ORANGE:
				r = j*223/255 + 32;
				g = j/2;
				b = 0;
				break;

			case CR_WHITE:
				r = g = b = j;
				break;

			case CR_YELLOW:
				r = g = j;
				b = 0;
				break;
			}
			*range++ = BestColor (DefaultPalette->basecolors, r, g, b, DefaultPalette->numcolors);
		}
		for (j = 0; j < 128; j++)
		{
			switch (i)
			{
			case CR_BRICK:
				r = j+128;
				b = g = j*2;
				break;

			case CR_TAN:
				r = 153+j*102/127;
				g = 139+j*116/127;
				b = 121+j*134/127;
				break;

			case CR_GRAY:
				r = g = b = 63+j*192/127;
				break;

			case CR_GREEN:
				r = b = j*2;
				g = 255;
				break;

			case CR_BROWN:
				r = 108+j*147/127;
				g = 104+j*151/127;
				b = 96+j*159/127;
				break;

			case CR_GOLD:
				r = 255;
				g = MIN(191+j*64/127,255);
				b = j*2;
				break;

			case CR_RED:
				r = 255;
				g = b = j*2;
				break;

			case CR_BLUE:
				b = 255;
				r = g = 64+j*3/2;
				break;

			case CR_ORANGE:
				r = 255;
				g = 127+j;
				b = j*2;
				break;

			case CR_WHITE:
				r = g = b = j+128;
				break;

			case CR_YELLOW:
				r = g = 255;
				b = j*2;
				break;
			}
			*range++ = BestColor (DefaultPalette->basecolors, r, g, b, DefaultPalette->numcolors);
		}
	}
}