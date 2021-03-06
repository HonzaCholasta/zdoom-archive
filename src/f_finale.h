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
// DESCRIPTION:
//
//	  
//-----------------------------------------------------------------------------


#ifndef __F_FINALE__
#define __F_FINALE__

#include "doomtype.h"
#include "d_event.h"
//
// FINALE
//

// Called by main loop.
BOOL F_Responder (event_t* ev);

// Called by main loop.
void F_Ticker ();

// Called by main loop.
void F_Drawer ();


void F_StartFinale (char *music, int musicorder, int cdtrack, unsigned int cdid, char *flat, char *text,
					BOOL textInLump, BOOL finalePic, BOOL lookupText);

void F_StartSlideshow ();

void F_EndFinale ();

#endif
