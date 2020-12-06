#include "sbar.h"
#include "c_cvars.h"
#include "v_video.h"
#include "m_swap.h"
#include "r_draw.h"
#include "w_wad.h"
#include "z_zone.h"
#include "v_text.h"
#include "s_sound.h"
#include "gi.h"
#include "../version.h"

#define XHAIRSHRINKSIZE		(FRACUNIT/18)
#define XHAIRPICKUPSIZE		(FRACUNIT*2+XHAIRSHRINKSIZE)

EXTERN_CVAR (am_showmonsters)
EXTERN_CVAR (am_showsecrets)
EXTERN_CVAR (am_showtime)

FBaseStatusBar *StatusBar;
CVAR (hud_scale, "0", CVAR_ARCHIVE);

int ST_X, ST_Y;
int SB_state = -1;

static FImageCollection CrosshairImage;
static DWORD CrosshairColor;

// [RH] Base blending values (for e.g. underwater)
int BaseBlendR, BaseBlendG, BaseBlendB;
float BaseBlendA;

BEGIN_CUSTOM_CVAR (st_scale, "1", CVAR_ARCHIVE)		// Stretch status bar to full screen width?
{
	if (StatusBar)
	{
		StatusBar->SetScaled (var.value != 0.0);
		setsizeneeded = true;
	}
}
END_CUSTOM_CVAR (st_scale)

BEGIN_CUSTOM_CVAR (crosshair, "0", CVAR_ARCHIVE)
{
	int num = (int)var.value;
	char name[16], size;
	const char *namelist;
	int lump;

	if (num == 0)
	{
		CrosshairImage.Uninit ();
		return;
	}
	if (num < 0)
	{
		num = -num;
	}
	size = (screen->width < 640) ? 'S' : 'B';
	sprintf (name, "XHAIR%c%d", size, num);
	if ((lump = W_CheckNumForName (name)) == -1)
	{
		sprintf (name, "XHAIR%c1", size);
		if ((lump = W_CheckNumForName (name)) == -1)
		{
			strcpy (name, "XHAIRS1");
		}
	}
	namelist = name;
	CrosshairImage.Init (&namelist, 1);
}
END_CUSTOM_CVAR (crosshair)

BEGIN_CUSTOM_CVAR (crosshaircolor, "ff 00 00", CVAR_ARCHIVE)
{
	char *string;

	if ( (string = V_GetColorStringByName (crosshaircolor.string)) )
	{
		CrosshairColor = V_GetColorFromString (NULL, string);
		delete[] string;
	}
	else
	{
		CrosshairColor = V_GetColorFromString (NULL, crosshaircolor.string);
	}
}
END_CUSTOM_CVAR (crosshaircolor)

CVAR (crosshairhealth, "1", CVAR_ARCHIVE);
CVAR (scalecrosshair, "0", CVAR_ARCHIVE);

// [RH] Amount of red flash for up to 114 damage points. Calculated by hand
//		using a logarithmic scale and my trusty HP48G.
byte FBaseStatusBar::DamageToAlpha[114] =
{
	  0,   8,  16,  23,  30,  36,  42,  47,  53,  58,  62,  67,  71,  75,  79,
	 83,  87,  90,  94,  97, 100, 103, 107, 109, 112, 115, 118, 120, 123, 125,
	128, 130, 133, 135, 137, 139, 141, 143, 145, 147, 149, 151, 153, 155, 157,
	159, 160, 162, 164, 165, 167, 169, 170, 172, 173, 175, 176, 178, 179, 181,
	182, 183, 185, 186, 187, 189, 190, 191, 192, 194, 195, 196, 197, 198, 200,
	201, 202, 203, 204, 205, 206, 207, 209, 210, 211, 212, 213, 214, 215, 216,
	217, 218, 219, 220, 221, 221, 222, 223, 224, 225, 226, 227, 228, 229, 229,
	230, 231, 232, 233, 234, 235, 235, 236, 237
};

//---------------------------------------------------------------------------
//
// Constructor
//
//---------------------------------------------------------------------------

FBaseStatusBar::FBaseStatusBar (int reltop)
{
	Centering = false;
	FixedOrigin = false;
	CrosshairSize = FRACUNIT;
	RelTop = reltop;
	ScaleCopy = NULL;
	Messages = NULL;
	PlayerName = NULL;
	Displacement = 0;

	SetScaled (st_scale.value != 0.f);
	AmmoImages.Init (AmmoPics, NUMAMMO);
	ArtiImages.Init (ArtiPics, NUMARTIFACTS);
	ArmorImages.Init (ArmorPics, NUMARMOR);
}

//---------------------------------------------------------------------------
//
// Destructor
//
//---------------------------------------------------------------------------

FBaseStatusBar::~FBaseStatusBar ()
{
	FHUDMessage *msg;

	if (ScaleCopy)
	{
		delete ScaleCopy;
	}
	msg = Messages;
	while (msg)
	{
		FHUDMessage *next = msg->Next;
		delete msg;
		msg = next;
	}
}

//---------------------------------------------------------------------------
//
// PROC SetScaled
//
//---------------------------------------------------------------------------

void FBaseStatusBar::SetScaled (bool scale)
{
	Scaled = RelTop != 0 && (screen->width != 320 && scale);
	if (!Scaled)
	{
		ST_X = (screen->width - 320) / 2;
		ST_Y = screen->height - RelTop;
		::ST_Y = ST_Y;
		if (RelTop > 0)
		{
			Displacement = ((ST_Y * 200 / screen->height) - (200 - RelTop))*FRACUNIT/RelTop;
		}
		else
		{
			Displacement = 0;
		}
	}
	else
	{
		ST_X = 0;
		ST_Y = 200 - RelTop;
		::ST_Y = (ST_Y * screen->height) / 200;
		if (ScaleCopy == NULL)
		{
			ScaleCopy = new DCanvas (320, RelTop, 8);
		}
		ScaleX = (screen->width << FRACBITS) / 320;
		ScaleY = (screen->height << FRACBITS) / 200;
		ScaleIX = (320 << FRACBITS) / screen->width;
		ScaleIY = (200 << FRACBITS) / screen->height;
		Displacement = 0;
	}
	::ST_X = ST_X;
	SB_state = -1;
}

//---------------------------------------------------------------------------
//
// PROC AttachToPlayer
//
//---------------------------------------------------------------------------

void FBaseStatusBar::AttachToPlayer (player_s *player)
{
	CPlayer = player;
	SB_state = -1;
}

//---------------------------------------------------------------------------
//
// PROC Tick
//
//---------------------------------------------------------------------------

void FBaseStatusBar::Tick ()
{
	FHUDMessage *msg = Messages;
	FHUDMessage **prev = &Messages;

	while (msg)
	{
		FHUDMessage *next = msg->Next;

		if (msg->Tick ())
		{
			*prev = next;
			delete msg;
		}
		msg = next;
	}
}

//---------------------------------------------------------------------------
//
// PROC AttachMessage
//
//---------------------------------------------------------------------------

void FBaseStatusBar::AttachMessage (FHUDMessage *msg)
{
	msg->Next = Messages;
	Messages = msg;
}

//---------------------------------------------------------------------------
//
// PROC DetachMessage
//
//---------------------------------------------------------------------------

FHUDMessage *FBaseStatusBar::DetachMessage (FHUDMessage *msg)
{
	FHUDMessage *probe = Messages;
	FHUDMessage **prev = &Messages;

	while (probe && probe != msg)
	{
		prev = &probe;
		probe = probe->Next;
	}
	if (probe != NULL)
	{
		*prev = probe->Next;
		probe->Next = NULL;
	}
	return probe;
}

//---------------------------------------------------------------------------
//
// PROC CheckMessage
//
//---------------------------------------------------------------------------

bool FBaseStatusBar::CheckMessage (FHUDMessage *msg)
{
	FHUDMessage *probe = Messages;
	while (probe && probe != msg)
	{
		probe = probe->Next;
	}
	return (probe == msg);
}

//---------------------------------------------------------------------------
//
// PROC ShowPlayerName
//
//---------------------------------------------------------------------------

void FBaseStatusBar::ShowPlayerName ()
{
	EColorRange color;

	color = (CPlayer == &players[consoleplayer]) ? CR_GOLD : CR_GREEN;
	if (PlayerName && DetachMessage (PlayerName))
	{
		delete PlayerName;
	}
	PlayerName = new FHUDMessageFadeOut (CPlayer->userinfo.netname,
		-2.f, 1.f, color, 2.f, 0.35f);
	AttachMessage (PlayerName);
}

//---------------------------------------------------------------------------
//
// PROC CopyToScreen
//
// Copies a region from ScaleCopy to the visible screen. We have to be
// very careful with our texture coordinates so that the end result looks
// like one continuous status bar instead of several individual pieces
// slapped on top of a status bar.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::CopyToScreen (int x, int y, int w, int h) const
{
	fixed_t ix = ScaleIX;
	fixed_t iy = ScaleIY;
	int nx = (x * ScaleX) >> FRACBITS;
	int ny = (y * ScaleY) >> FRACBITS;
	fixed_t left = ix * nx;
	fixed_t err = iy * ny;
	w += x - (left >> FRACBITS);
	h += y - (err >> FRACBITS);
	int ow = (((x + w) * ScaleX) >> FRACBITS) - nx;
	int oh = (((y + h) * ScaleY) >> FRACBITS) - ny;

	if (ny + ::ST_Y + oh > screen->height)
		oh = screen->height - ny - ::ST_Y;
	if (nx + ow > screen->width)
		ow = screen->width - nx;

	if ((oh | ow) <= 0)
		return;

	byte *from = ScaleCopy->buffer + (left >> FRACBITS);
	left &= FRACUNIT-1;
	byte *to = screen->buffer + nx + screen->pitch * (ny + ::ST_Y);
	int frompitch = ScaleCopy->pitch;
	int toskip = screen->pitch - ow;

	do
	{
		if (err >= FRACUNIT)
		{
			from += frompitch * (err >> FRACBITS);
			err &= FRACUNIT-1;
		}
		int l = ow;
		fixed_t cx = left;
		do
		{
			*to++ = from[cx>>FRACBITS];
			cx += ix;
		} while (--l);
		to += toskip;
		err += iy;
	} while (--oh);
}

//---------------------------------------------------------------------------
//
// PROC DrawImage
//
// Draws an image with the status bar's upper-left corner as the origin.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrawImage (const FImageCollection &coll, int img,
	int x, int y, byte *translation) const
{
	int w, h, xo, yo;
	byte *data = coll.GetImage (img, &w, &h, &xo, &yo);
	if (data)
	{
		x -= xo;
		y -= yo;
		if (Scaled)
		{
			if (y < 0)
			{
				screen->ScaleMaskedBlock (x * screen->width / 320,
					(y + ST_Y) * screen->height / 200, w, h,
					(w * screen->width) / 320, (h * screen->height) / 200,
					data, NULL);
			}
			else
			{
				ScaleCopy->Lock ();
				ScaleCopy->DrawMaskedBlock (x, y, w, h, data, translation);
				CopyToScreen (x, y, w, h);
				ScaleCopy->Unlock ();
			}
		}
		else
		{
			screen->DrawMaskedBlock (x + ST_X, y + ST_Y, w, h, data, translation);
		}
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawPartialImage
//
// Draws a portion of an image with the status bar's upper-left corner as
// the origin. The image must not have any mask bytes. Used for Doom's sbar.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrawPartialImage (const FImageCollection &coll, int img,
	int x, int y, int wx, int wy, int ww, int wh) const
{
	int w, h, xo, yo;
	byte *data = coll.GetImage (img, &w, &h, &xo, &yo);
	if (data)
	{
		byte *dest;
		int pitch;

		data += wx + wy*w;

		if (Scaled)
		{
			ScaleCopy->Lock ();
			dest = ScaleCopy->buffer + x + y*ScaleCopy->pitch;
			pitch = ScaleCopy->pitch;
		}
		else
		{
			dest = screen->buffer + x+ST_X + (y+ST_Y)*screen->pitch;
			pitch = screen->pitch;
		}

		do
		{
			memcpy (dest, data, ww);
			dest += pitch;
			data += w;
		} while (--wh);

		if (Scaled)
		{
			ScaleCopy->Unlock ();
		}
	}
}

//---------------------------------------------------------------------------
//
// PROC RepositionCoords
//
// Repositions coordinates for the outside status bar drawers. Returns
// true if the image should be stretched.
//
//---------------------------------------------------------------------------

bool FBaseStatusBar::RepositionCoords (int &x, int &y, int xo, int yo,
	const int w, const int h) const
{
	if (FixedOrigin)
	{
		xo = w / 2;
		yo = h;
	}
	if (hud_scale.value)
	{
		x *= CleanXfac;
		if (Centering)
			x += screen->width / 2;
		else if (x < 0)
			x = screen->width + x;
		x -= xo * CleanXfac;
		y *= CleanYfac;
		if (y < 0)
			y = screen->height + y;
		y -= yo * CleanYfac;
		return true;
	}
	else
	{
		if (Centering)
			x += screen->width / 2;
		else if (x < 0)
			x = screen->width + x;
		if (y < 0)
			y = screen->height + y;
		x -= xo;
		y -= yo;
		return false;
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawFadedImage
//
// Draws a translucenct image outside the status bar, possibly scaled.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrawFadedImage (const FImageCollection &coll, int img,
	int x, int y, fixed_t shade) const
{
	int w, h, xo, yo;
	byte *data = coll.GetImage (img, &w, &h, &xo, &yo);
	if (data)
	{
		if (RepositionCoords (x, y, xo, yo, w, h))
		{
			screen->ScaleTranslucentMaskedBlock (x, y, w, h, w * CleanXfac,
				h * CleanYfac, data, NULL, shade);
		}
		else
		{
			screen->DrawTranslucentMaskedBlock (x, y, w, h, data, NULL, shade);
		}
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawShadowedImage
//
// Draws a shadowed image outside the status bar, possibly scaled.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrawShadowedImage (const FImageCollection &coll, int img,
	int x, int y, fixed_t shade) const
{
	int w, h, xo, yo;
	byte *data = coll.GetImage (img, &w, &h, &xo, &yo);
	if (data)
	{
		if (RepositionCoords (x, y, xo, yo, w, h))
		{
			screen->ScaleShadowedMaskedBlock (x, y, w, h, w * CleanXfac,
				h * CleanYfac, data, NULL, shade);
		}
		else
		{
			screen->DrawShadowedMaskedBlock (x, y, w, h, data, NULL, shade);
		}
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawOuterImage
//
// Draws an image outside the status bar, possibly scaled.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrawOuterImage (const FImageCollection &coll, int img,
	int x, int y) const
{
	int w, h, xo, yo;
	byte *data = coll.GetImage (img, &w, &h, &xo, &yo);
	if (data)
	{
		if (RepositionCoords (x, y, xo, yo, w, h))
		{
			screen->ScaleMaskedBlock (x, y, w, h, w * CleanXfac,
				h * CleanYfac, data, NULL);
		}
		else
		{
			screen->DrawMaskedBlock (x, y, w, h, data, NULL);
		}
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawOuterPatch
//
// Draws a patch outside the status bar, possibly scaled.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrawOuterPatch (const patch_t *patch, int x, int y) const
{
	if (patch)
	{
		if (RepositionCoords (x, y, 0, 0, 0, 0))
		{
			screen->DrawPatchStretched (patch, x, y,
				SHORT(patch->width) * CleanXfac, SHORT(patch->height) * CleanYfac);
		}
		else
		{
			screen->DrawPatch (patch, x, y);
		}
	}
}

//---------------------------------------------------------------------------
//
// PROC DrINumber
//
// Draws a three digit number.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrINumber (signed int val, int x, int y) const
{
	int oldval;

	if (val > 999)
		val = 999;
	oldval = val;
	if (val < 0)
	{
		if (val < -9)
		{
			DrawImage (Images, imgLAME, x+1, y+1);
			return;
		}
		val = -val;
		DrawImage (Images, imgINumbers+val, x+10, y);
		DrawImage (Images, imgNEGATIVE, x+9, y);
		return;
	}
	if (val > 99)
	{
		DrawImage (Images, imgINumbers+val/100, x, y);
	}
	val = val % 100;
	if (val > 9 || oldval > 99)
	{
		DrawImage (Images, imgINumbers+val/10, x+9, y);
	}
	val = val % 10;
	DrawImage (Images, imgINumbers+val, x+18, y);
}

//---------------------------------------------------------------------------
//
// PROC DrBNumber
//
// Draws an x digit number using the big font.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrBNumber (signed int val, int x, int y, int size) const
{
	int xpos;
	int index;
	int w, h;
	bool neg;
	int i;
	int power;

	w = Images.GetImageWidth (imgBNumbers+3);
	h = Images.GetImageHeight (imgBNumbers+3);

	if (Scaled)
	{
		ScaleCopy->Lock ();
		CopyToScreen (x, y, size*w, h);
		ScaleCopy->Unlock ();
	}

	xpos = x + w/2 + w*size;

	for (i = size-1, power = 10; i > 0; i--)
	{
		power *= 10;
	}

	if (val >= power)
	{
		val = power - 1;
	}
	if ( (neg = val < 0) )
	{
		if (size == 2 && val < -9)
		{
			val = -9;
		}
		else if (size == 3 && val < -99)
		{
			val = -99;
		}
		val = -val;
		size--;
	}
	if (val == 0)
	{
		DrawImage (Images, imgBNumbers,
			xpos - Images.GetImageWidth (imgBNumbers)/2 - w, y);
		return;
	}
	while (val != 0 && size--)
	{
		xpos -= w;
		int oldval = val;
		val /= 10;
		index = imgBNumbers + (oldval - val*10);
		DrawImage (Images, index, xpos - Images.GetImageWidth (index)/2, y);
	}
	if (neg)
	{
		xpos -= w;
		DrawImage (Images, imgBNEGATIVE,
			xpos - Images.GetImageWidth (imgBNEGATIVE)/2, y);
	}
}

//---------------------------------------------------------------------------
//
// PROC DrSmallNumber
//
// Draws a small three digit number.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrSmallNumber (int val, int x, int y) const
{
	int digit = 0;

	if (Scaled)
	{
		ScaleCopy->Lock ();
		CopyToScreen (x, y, 3*4, 6);
		ScaleCopy->Unlock ();
	}
	if (val > 999)
	{
		val = 999;
	}
	if (val > 99)
	{
		digit = val / 100;
		DrawImage (Images, imgSmNumbers + digit, x, y);
		val -= digit * 100;
	}
	if (val > 9 || digit)
	{
		digit = val / 10;
		DrawImage (Images, imgSmNumbers + digit, x+4, y);
		val -= digit * 10;
	}
	DrawImage (Images, imgSmNumbers + val, x+8, y);
}

//---------------------------------------------------------------------------
//
// PROC DrINumberOuter
//
// Draws a three digit number outside the status bar, possibly scaled.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrINumberOuter (signed int val, int x, int y) const
{
	int oldval;

	if (val > 999)
	{
		val = 999;
	}
	oldval = val;
	if (val < 0)
	{
		if (val < -9)
		{
			DrawOuterImage (Images, imgLAME, x+1, y+1);
			return;
		}
		val = -val;
		DrawOuterImage (Images, imgINumbers+val, x+10, y);
		DrawOuterImage (Images, imgNEGATIVE, x+9, y);
		return;
	}
	if (val > 99)
	{
		DrawOuterImage (Images, imgINumbers+val/100, x, y);
	}
	val = val % 100;
	if (val > 9 || oldval > 99)
	{
		DrawOuterImage (Images, imgINumbers+val/10, x+9, y);
	}
	val = val % 10;
	DrawOuterImage (Images, imgINumbers+val, x+18, y);
}

//---------------------------------------------------------------------------
//
// PROC DrBNumberOuter
//
// Draws a three digit number using the big font outside the status bar.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrBNumberOuter (signed int val, int x, int y, int size) const
{
	int xpos;
	int i;
	int w;
	int div;
	bool neg;
	int first;

	Images.GetImage (imgBNumbers+3, &w, &xpos, &xpos, &xpos);
	w = Images.GetImageWidth (imgBNumbers+3);

	div = 1;
	for (i = size-1; i>0; i--)
		div *= 10;

	xpos = x + w/2;

	if (val >= div*10)
	{
		val = div*10 - 1;
	}
	if ( (neg = (val < 0)) )
	{
		if (val <= -div)
		{
			val = -div + 1;
		}
		val = -val;
		size--;
		div /= 10;
	}
	if (val == 0)
	{
		DrawShadowedImage (Images, imgBNumbers,
			xpos - Images.GetImageWidth (imgBNumbers)/2 + w*(size-1), y, HR_SHADOW);
		return;
	}
	first = -99999;
	while (size--)
	{
		i = val / div;

		if (i != 0 || first != -99999)
		{
			if (first == -99999)
			{
				first = xpos;
			}
			DrawShadowedImage (Images, i + imgBNumbers,
				xpos - Images.GetImageWidth (i + imgBNumbers)/2, y, HR_SHADOW);
			val -= i * div;
		}
		div /= 10;
		xpos += w;
	}
	if (neg)
	{
		DrawShadowedImage (Images, imgBNEGATIVE,
			first - Images.GetImageWidth (imgBNEGATIVE)/2 - w, y, HR_SHADOW);
	}
}

//---------------------------------------------------------------------------
//
// PROC DrSmallNumberOuter
//
// Draws a small three digit number outside the status bar.
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrSmallNumberOuter (int val, int x, int y) const
{
	int digit = 0;

	if (val > 999)
	{
		val = 999;
	}
	if (val > 99)
	{
		digit = val / 100;
		DrawOuterImage (Images, imgSmNumbers + digit, x, y);
		val -= digit * 100;
	}
	if (val > 9 || digit)
	{
		digit = val / 10;
		DrawOuterImage (Images, imgSmNumbers + digit, x+4, y);
		val -= digit * 10;
	}
	DrawOuterImage (Images, imgSmNumbers + val, x+8, y);
}

//---------------------------------------------------------------------------
//
// PROC ShadeChain
//
//---------------------------------------------------------------------------

void FBaseStatusBar::ShadeChain (int left, int right, int top, int height) const
{
	int i;
	int pitch;
	int diff;
	byte *top_p;

	if (Scaled)
	{
		ScaleCopy->Lock ();
		pitch = ScaleCopy->pitch;
		top_p = ScaleCopy->buffer;
	}
	else
	{
		top += ST_Y;
		left += ST_X;
		right += ST_X;
		pitch = screen->pitch;
		top_p = screen->buffer;
	}
	top_p += top*pitch + left;
	diff = right+15-left;

	for (i = 0; i < 16; i++)
	{
		unsigned int *darkener = Col2RGB8[18 + i*2];
		int h = height;
		byte *dest = top_p;
		do
		{
			unsigned int lbg = darkener[*dest] | 0x1f07c1f;
			unsigned int rbg = darkener[*(dest+diff)] | 0x1f07c1f;
			*dest = RGB32k[0][0][lbg & (lbg>>15)];
			*(dest+diff) = RGB32k[0][0][rbg & (rbg>>15)];
			dest += pitch;
		} while (--h);
		top_p++;
		diff -= 2;
	}

	if (Scaled)
	{
		CopyToScreen (left, top, 16, height);
		CopyToScreen (right, top, 16, height);
		ScaleCopy->Unlock ();
	}
}

//---------------------------------------------------------------------------
//
// RefreshBackground
//
//---------------------------------------------------------------------------

void FBaseStatusBar::RefreshBackground () const
{
	if (screen->width > 320)
	{
		R_DrawBorder (0, ST_Y, ST_X, screen->height);
		R_DrawBorder (screen->width - ST_X, ST_Y, screen->width, screen->height);
	}
}

//---------------------------------------------------------------------------
//
// DrawCrosshair
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrawCrosshair ()
{
	static DWORD prevcolor = 0xffffffff;
	static int palettecolor = 0;

	DWORD color;
	fixed_t size;
	int w, h;
	int iw, ih, iox, ioy;
	byte *image;

	if (CrosshairSize > FRACUNIT)
	{
		CrosshairSize -= XHAIRSHRINKSIZE;
		if (CrosshairSize < FRACUNIT)
		{
			CrosshairSize = FRACUNIT;
		}
	}

	// Don't draw the crosshair in chasecam mode
	if (players[consoleplayer].cheats & CF_CHASECAM)
		return;

	image = CrosshairImage.GetImage (0, &iw, &ih, &iox, &ioy);
	// Don't draw the crosshair if there is none
	if (image == NULL)
	{
		return;
	}

	if (scalecrosshair.value)
	{
		size = screen->width * FRACUNIT / 320;
	}
	else
	{
		size = FRACUNIT;
	}

	size = FixedMul (size, CrosshairSize);
	w = (iw * size) >> FRACBITS;
	h = (ih * size) >> FRACBITS;

	if (crosshairhealth.value)
	{
		int health = CPlayer->health;

		if (health >= 85)
		{
			color = 0x00ff00;
		}
		else 
		{
			int red, green;
			health -= 25;
			if (health < 0)
			{
				health = 0;
			}
			if (health < 30)
			{
				red = 255;
				green = health * 255 / 30;
			}
			else
			{
				red = (60 - health) * 255 / 30;
				green = 255;
			}
			color = (red<<16) | (green<<8);
		}
	}
	else
	{
		color = CrosshairColor;
	}

	if (color != prevcolor)
	{
		prevcolor = color;
		palettecolor = BestColor (DefaultPalette->basecolors,
			RPART(color), GPART(color), BPART(color), DefaultPalette->numcolors);
	}

	screen->ScaleAlphaMaskedBlock (
		realviewwidth / 2 + viewwindowx - ((iox * size) >> FRACBITS),
		realviewheight / 2  + viewwindowy- ((ioy * size) >> FRACBITS),
		iw, ih, w, h, image, palettecolor);
}

//---------------------------------------------------------------------------
//
// FlashCrosshair
//
//---------------------------------------------------------------------------

void FBaseStatusBar::FlashCrosshair ()
{
	CrosshairSize = XHAIRPICKUPSIZE;
}

//---------------------------------------------------------------------------
//
// DrawMessages
//
//---------------------------------------------------------------------------

void FBaseStatusBar::DrawMessages (int bottom) const
{
	FHUDMessage *msg = Messages;
	while (msg)
	{
		msg->Draw (bottom);
		msg = msg->Next;
	}
}

//---------------------------------------------------------------------------
//
// Draw
//
//---------------------------------------------------------------------------

void FBaseStatusBar::Draw (EHudState state)
{
	float blend[4];

	blend[0] = blend[1] = blend[2] = blend[3] = 0;
	BlendView (blend);

	if (SB_state == -1 && state == HUD_StatusBar && !Scaled)
	{
		RefreshBackground ();
	}

	if (viewactive)
	{
		DrawCrosshair ();
	}
	else if (automapactive)
	{
		char line[64+10];
		int y, i, time = level.time / TICRATE, height;
		EColorRange highlight = (gameinfo.gametype == GAME_Doom) ?
			CR_UNTRANSLATED : CR_YELLOW;

		height = screen->Font->GetHeight () * CleanYfac;

		// Draw timer
		if (am_showtime.value)
		{
			sprintf (line, "%02d:%02d:%02d", time/3600, (time%3600)/60, time%60);	// Time
			screen->DrawTextClean (CR_GREY, screen->width - 80*CleanXfac, 8, line);
		}

		// Draw map name
		y = ::ST_Y - height;
		if ((gameinfo.gametype == GAME_Heretic && screen->width > 320 && !Scaled)
			|| (gameinfo.gametype == GAME_Hexen))
		{
			y -= 8;
		}
		cluster_info_t *cluster = FindClusterInfo (level.cluster);
		i = 0;
		if (cluster == NULL || !(cluster->flags & CLUSTER_HUB))
		{
			while (i < 8 && level.mapname[i])
			{
				line[i++] = level.mapname[i];
			}
			line[i++] = ':';
			line[i++] = ' ';
		}
		line[i++] = '\x81';
		line[i++] = CR_GREY + 'A';
		strcpy (&line[i], level.level_name);
		screen->DrawTextClean (highlight,
			(screen->width - screen->StringWidth (line)*CleanXfac)/2, y, line);

		if (!deathmatch.value)
		{
			// Draw monster count
			if (am_showmonsters.value)
			{
				sprintf (line, "MONSTERS:"
							   TEXTCOLOR_GREY " %d/%d",
							   level.killed_monsters, level.total_monsters);
				screen->DrawTextClean (highlight, 8, 8, line);
			}

			// Draw secret count
			if (am_showsecrets.value)
			{
				sprintf (line, "SECRETS:"
							   TEXTCOLOR_GREY " %d/%d",
							   level.found_secrets, level.total_secrets);
				screen->DrawTextClean (highlight, 8, 8+height, line);
			}
		}
	}

	if (noisedebug.value)
	{
		S_NoiseDebug ();
	}

	if (demoplayback && demover != GAMEVER)
	{
		screen->DrawTextClean (CR_TAN, 0, ST_Y - 40 * CleanYfac,
			"Demo was recorded with a different version\n"
			"of ZDoom. Expect it to go out of sync.");
	}

	if (state == HUD_StatusBar)
	{
		DrawMessages (::ST_Y);
	}
	else
	{
		DrawMessages (screen->height);
	}

	DrawConsistancy ();
}

/*
=============
SV_AddBlend
[RH] This is from Q2.
=============
*/
void FBaseStatusBar::AddBlend (float r, float g, float b, float a, float v_blend[4])
{
	float a2, a3;

	if (a <= 0)
		return;
	a2 = v_blend[3] + (1-v_blend[3])*a;	// new total alpha
	a3 = v_blend[3]/a2;		// fraction of color from old

	v_blend[0] = v_blend[0]*a3 + r*(1-a3);
	v_blend[1] = v_blend[1]*a3 + g*(1-a3);
	v_blend[2] = v_blend[2]*a3 + b*(1-a3);
	v_blend[3] = a2;
}

//---------------------------------------------------------------------------
//
// BlendView
//
//---------------------------------------------------------------------------

void FBaseStatusBar::BlendView (float blend[4])
{
	int cnt;

	AddBlend (BaseBlendR / 255.0f, BaseBlendG / 255.0f, BaseBlendB / 255.0f, BaseBlendA, blend);
	if (CPlayer->powers[pw_ironfeet] > 4*32 || CPlayer->powers[pw_ironfeet]&8)
	{
		AddBlend (0.0f, 1.0f, 0.0f, 0.125f, blend);
	}
	if (CPlayer->bonuscount)
	{
		cnt = CPlayer->bonuscount << 3;
		AddBlend (0.8431f, 0.7294f, 0.2706f, cnt > 128 ? 0.5f : cnt / 255.0f, blend);
	}

	cnt = DamageToAlpha[MIN (113, CPlayer->damagecount)];

	if (CPlayer->powers[pw_strength])
	{
		// slowly fade the berzerk out
		int bzc = 128 - ((CPlayer->powers[pw_strength]>>3) & (~0x1f));

		if (bzc > cnt)
			cnt = bzc;
	}
		
	if (cnt)
	{
		if (cnt > 228)
			cnt = 228;

		AddBlend (1.0f, 0.0f, 0.0f, cnt / 255.0f, blend);
	}

	V_SetBlend ((int)(blend[0] * 255.0f), (int)(blend[1] * 255.0f),
				(int)(blend[2] * 255.0f), (int)(blend[3] * 256.0f));
}

void FBaseStatusBar::DrawConsistancy () const
{
	int i;
	char conbuff[64], *buff_p;

	if (!netgame)
		return;

	buff_p = NULL;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && players[i].inconsistant)
		{
			if (buff_p == NULL)
			{
				strcpy (conbuff, "Consistency failure: ");
				buff_p = conbuff + 21;
			}
			*buff_p++ = '0' + i;
			*buff_p = 0;
		}
	}

	if (buff_p != NULL)
	{
		screen->DrawTextClean (CR_GREEN, 
			(screen->width - screen->StringWidth (conbuff)*CleanXfac) / 2,
			0, conbuff);
		BorderTopRefresh = true;
	}
}
