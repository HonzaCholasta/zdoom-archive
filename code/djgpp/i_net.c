// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_net.c,v 1.4 1998/05/16 09:41:03 jim Exp $
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
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "z_zone.h"  /* memory allocation wrappers -- killough */

#include "m_alloc.h"
#include "doomstat.h"
#include "i_system.h"
#include "d_event.h"
#include "d_net.h"
#include "m_argv.h"

#include <stdlib.h>
#undef PI
#include <math.h>
#include <sys/nearptr.h>
#include <dpmi.h>

#include "i_net.h"

//
// NETWORKING
//

//
// I_InitNetwork
//
void I_InitNetwork (void)
{
	int i, j;
	char *p;
      
	// set up for network
                      
	// parse network game options,
	//  -net <consoleplayer> <host> <host> ...
	i = Args.CheckParm ("-net");
	if (!i)
	{
		// single player game
		doomcom = Malloc (sizeof (*doomcom) );
		memset (doomcom, 0, sizeof(*doomcom) );

		netgame = false;
		multiplayer = false;
		doomcom->id = DOOMCOM_ID;
		doomcom->numplayers = doomcom->numnodes = 1;
		doomcom->consoleplayer = 0;
		doomcom->extratics = 0;
		doomcom->ticdup = 1;
		return;
	}

	if (!__djgpp_nearptr_enable())  //handle nearptr now
		I_FatalError ("Failed trying to allocate DOS near pointers.");

	doomcom=(doomcom_t *)(__djgpp_conventional_base+atoi(myargv[i+1]));

	doomcom->ticdup = 1;
	if (Args.CheckParm ("-extratic"))
		doomcom-> extratics = 1;
	else
		doomcom-> extratics = 0;

	p = Args.CheckValue ("-dup");
	if (p)
	{
		doomcom->ticdup = myargv[j+1][0]-'0';
		if (doomcom->ticdup < 1)
			doomcom->ticdup = 1;
		if (doomcom->ticdup > 9)
			doomcom->ticdup = 9;
	}
	else
		doomcom->ticdup = 1;

	netgame = true;
	multiplayer = true;
}


void I_NetCmd (void)
{
	__dpmi_regs r;
                              
	__dpmi_int(doomcom->intnum,&r);
}
