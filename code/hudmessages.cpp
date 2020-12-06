#include "doomdef.h"
#include "sbar.h"
#include "c_cvars.h"
#include "v_video.h"

EXTERN_CVAR (con_scaletext)

//
// Basic HUD message. Appears and disappears without any special FX
//

FHUDMessage::FHUDMessage (char *text, float x, float y, EColorRange textColor,
						  float holdTime)
{
	CenterX = (x < -1.f || x > 1.f);
	Left = x;
	Top = y;
	Next = NULL;
	Lines = NULL;
	HoldTics = (int)(holdTime * TICRATE);
	Tics = 0;
	TextColor = textColor;
	State = 0;

	ResetText (text);
}

FHUDMessage::~FHUDMessage ()
{
	if (Lines)
	{
		V_FreeBrokenLines (Lines);
		Lines = NULL;
		BorderNeedRefresh = true;
	}
}

void FHUDMessage::ResetText (char *text)
{
	Lines = V_BreakLines (con_scaletext.value ?
		screen->width / CleanXfac : screen->width, (byte *)text);

	NumLines = 0;
	Width = 0;
	Height = 0;

	if (Lines)
	{
		for (; Lines[NumLines].width != -1; NumLines++)
		{
			Height += SmallFont->GetHeight ();
			Width = MAX (Width, Lines[NumLines].width);
		}
	}
}

bool FHUDMessage::Tick ()
{
	Tics++;
	if (HoldTics != 0 && HoldTics <= Tics)
	{ // This message has expired
		return true;
	}
	return false;
}

void FHUDMessage::Draw (int bottom)
{
	int xscale, yscale;
	int x, y;
	int ystep;
	int i;
	bool clean;

	DrawSetup ();

	if ( (clean = (con_scaletext.value != 0.f)) )
	{
		xscale = CleanXfac;
		yscale = CleanYfac;
	}
	else
	{
		xscale = yscale = 1;
	}

	if (CenterX)
	{
		x = screen->width / 2;
	}
	else if (Left > 0.f)
	{
		x = (int)((float)(screen->width - Width * xscale) * Left);
	}
	else
	{
		x = (int)((float)screen->width * -Left);
	}

	if (Top > 0.f)
	{
		y = (int)((float)(bottom - Height * yscale) * Top);
	}
	else
	{
		y = (int)((float)bottom * -Top);
	}

	ystep = SmallFont->GetHeight() * yscale;

	for (i = 0; i < NumLines; i++)
	{
		int drawx;

		drawx = CenterX ? x - Lines[i].width*xscale/2 : x;
		DoDraw (i, drawx, y, xscale, yscale, clean);
		y += ystep;
	}
}

void FHUDMessage::DrawSetup ()
{
}

void FHUDMessage::DoDraw (int linenum, int x, int y, int xscale, int yscale,
						  bool clean)
{
	if (clean)
	{
		screen->DrawTextClean (TextColor, x, y, Lines[linenum].string);
	}
	else
	{
		screen->DrawText (TextColor, x, y, Lines[linenum].string);
	}
}

//
// HUD message that fades out
//

FHUDMessageFadeOut::FHUDMessageFadeOut (char *text, float x, float y,
	EColorRange textColor, float holdTime, float fadeOutTime)
	: FHUDMessage (text, x, y, textColor, holdTime)
{
	FadeOutTics = (int)(fadeOutTime * TICRATE);
	State = 1;
}

bool FHUDMessageFadeOut::Tick ()
{
	Tics++;
	if (State == 1 && HoldTics <= Tics)
	{
		State++;
		Tics -= HoldTics;
	}
	if (State == 2 && FadeOutTics <= Tics)
	{
		return true;
	}
	return false;
}

void FHUDMessageFadeOut::DoDraw (int linenum, int x, int y, int xscale,
								 int yscale, bool clean)
{
	if (State == 1)
	{
		FHUDMessage::DoDraw (linenum, x, y, xscale, yscale, clean);
	}
	else
	{
		fixed_t trans = -(Tics - FadeOutTics) * FRACUNIT / FadeOutTics;
		if (clean)
		{
			screen->DrawTextCleanLuc (TextColor, x, y, Lines[linenum].string,
				trans);
		}
		else
		{
			screen->DrawTextLuc (TextColor, x, y, Lines[linenum].string,
				trans);
		}
		BorderNeedRefresh = true;
	}
}
