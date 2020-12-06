// Emacs style mode select	 -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	Sky rendering. The DOOM sky is a texture map like any
//	wall, wrapping around. 1024 columns equal 360 degrees.
//	The default sky map is 256 columns and repeats 4 times
//	on a 320 screen.
//	
//
//-----------------------------------------------------------------------------



// Needed for FRACUNIT.
#include "m_fixed.h"
#include "r_data.h"
#include "c_cvars.h"
#include "g_level.h"
#include "r_sky.h"
#include "gi.h"

extern int *texturewidthmask;

//
// sky mapping
//
int 		skyflatnum;
int 		sky1texture,	sky2texture;
fixed_t		skytexturemid;
fixed_t		skyscale;
int			skystretch;
fixed_t		skyheight;					
fixed_t		skyiscale;

int			sky1shift,		sky2shift;
fixed_t		sky1pos=0,		sky1speed=0;
fixed_t		sky2pos=0,		sky2speed=0;

// [RH] Stretch sky texture if not taller than 128 pixels?
CUSTOM_CVAR (Bool, r_stretchsky, true, CVAR_ARCHIVE)
{
	R_InitSkyMap ();
}

char SKYFLATNAME[8] = "F_SKY1";

extern "C" int detailxshift, detailyshift;
extern fixed_t freelookviewheight;

//==========================================================================
//
// R_InitSkyMap
//
// Called whenever the view size changes.
//
//==========================================================================

void R_InitSkyMap ()
{
	fixed_t fskyheight;

	if (textureheight == NULL)
		return;

	if (textureheight[sky1texture] != textureheight[sky2texture])
	{
		Printf ("\x1f+Both sky textures must be the same height.\x1f-\n");
		sky2texture = sky1texture;
	}

	// Heretic's sky textures are recorded with the wrong height. :-(
	// Assume 200 pixels tall, since anything else would look bad in Heretic
	// anyway.
	fskyheight = textureheight[sky1texture];
	if (gameinfo.gametype == GAME_Heretic && fskyheight == 128 * FRACUNIT)
		fskyheight = 200 * FRACUNIT;

	if (fskyheight <= (128 << FRACBITS))
	{
		skytexturemid = 200/2*FRACUNIT;
		skystretch = (r_stretchsky
					  && !(dmflags & DF_NO_FREELOOK)
					  && !(level.flags & LEVEL_FORCENOSKYSTRETCH)) ? 1 : 0;
	}
	else
	{
		skytexturemid = 200*FRACUNIT;
		skystretch = 0;
	}
	skyheight = fskyheight << skystretch;

	if (viewwidth && viewheight)
	{
		skyiscale = (200*FRACUNIT) / (((freelookviewheight<<detailxshift) * viewwidth) / (viewwidth<<detailxshift));
		skyscale = ((((freelookviewheight<<detailxshift) * viewwidth) / (viewwidth<<detailxshift)) << FRACBITS) /
					(200);

		skyiscale = Scale (skyiscale, FieldOfView, 2048);
		skyscale = Scale (skyscale, 2048, FieldOfView);
	}

	// The sky map is 256*128*4 maps.
	sky1shift = 22+skystretch-16;
	sky2shift = 22+skystretch-16;
	if (texturewidthmask[sky1texture] >= 256*2-1)
		sky1shift -= skystretch;
	if (texturewidthmask[sky2texture] >= 256*2-1)
		sky2shift -= skystretch;
}

