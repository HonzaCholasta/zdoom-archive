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
// DESCRIPTION:  none
//
//-----------------------------------------------------------------------------


static const char
rcsid[] = "$Id: g_game.c,v 1.8 1997/02/03 22:45:09 b1 Exp $";

#include <string.h>
#include <stdlib.h>

#include "doomdef.h" 
#include "doomstat.h"

#include "d_protocol.h"

#include "z_zone.h"
#include "f_finale.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_random.h"
#include "i_system.h"

#include "p_setup.h"
#include "p_saveg.h"
#include "p_tick.h"

#include "d_main.h"

#include "wi_stuff.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"

#include "c_console.h"
#include "c_cvars.h"
#include "c_bindings.h"
#include "c_dispatch.h"

// Needs access to LFB.
#include "v_video.h"

#include "w_wad.h"

#include "p_local.h" 

#include "s_sound.h"

// Data.
#include "dstrings.h"
#include "sounds.h"

// SKY handling - still the wrong place.
#include "r_data.h"
#include "r_sky.h"



#include "g_game.h"


#define SAVEGAMESIZE	0x2c000
#define SAVESTRINGSIZE	24


boolean G_CheckDemoStatus (void); 
void	G_ReadDemoTiccmd (ticcmd_t* cmd); 
void	G_WriteDemoTiccmd (ticcmd_t* cmd); 
void	G_PlayerReborn (int player); 
void	G_InitNew (skill_t skill, int episode, int map); 
 
void	G_DoReborn (int playernum); 
 
void	G_DoLoadLevel (void); 
void	G_DoNewGame (void); 
void	G_DoLoadGame (void); 
void	G_DoPlayDemo (void); 
void	G_DoCompleted (void); 
void	G_DoVictory (void); 
void	G_DoWorldDone (void); 
void	G_DoSaveGame (void); 
 
 
gameaction_t	gameaction; 
gamestate_t 	gamestate; 
cvar_t		   *gameskill; 
boolean 		respawnmonsters;
int 			gameepisode; 
int 			gamemap; 
 
boolean 		paused; 
boolean 		sendpause;				// send a pause event next tic 
boolean 		sendsave;				// send a save event next tic 
boolean 		usergame;				// ok to save / end game
boolean			sendcenterview;			// send a center view event next tic
 
boolean 		timingdemo; 			// if true, exit with report on completion 
boolean 		nodrawers;				// for comparative timing purposes 
boolean 		noblit; 				// for comparative timing purposes 
int 			starttime;				// for comparative timing purposes
boolean			olddemo;				// true if demo is non-ZDEM type
 
boolean 		viewactive; 
 
boolean 		deathmatch; 			// only if started as net death 
boolean 		netgame;				// only true if packets are broadcast 
boolean 		playeringame[MAXPLAYERS]; 
player_t		players[MAXPLAYERS]; 
 
int 			consoleplayer;			// player taking events and displaying 
int 			displayplayer;			// view being displayed 
int 			gametic; 
int 			levelstarttic;			// gametic at level start 
int 			totalkills, totalitems, totalsecret;	// for intermission 
 
char			demoname[32]; 
boolean 		demorecording; 
boolean 		demoplayback; 
boolean 		netdemo; 
byte*			demobuffer;
byte*			demo_p;
byte*			demoend; 
boolean 		singledemo; 			// quit after playing a demo from cmdline 
 
boolean 		precache = true;		// if true, load all graphics at start 
 
wbstartstruct_t wminfo; 				// parms for world map / intermission 
 
short			consistancy[MAXPLAYERS][BACKUPTICS]; 
 
byte*			savebuffer;
 
 
#define MAXPLMOVE				(forwardmove[1]) 
 
#define TURBOTHRESHOLD	12800

fixed_t 		forwardmove[2] = {0x19, 0x32}; 
fixed_t 		sidemove[2] = {0x18, 0x28}; 
fixed_t 		angleturn[3] = {640, 1280, 320};		// + slow turn
byte			flyspeed[2] = {1, 3};
int				lookspeed[2] = {3072, 5120};

#define SLOWTURNTICS	6 

cvar_t			*cl_run;								// Always run when set
 
int 			turnheld;								// for accelerative turning 
 
// mouse values are used once 
int 			mousex;
int 			mousey; 		

// joystick values are repeated 
int 			joyxmove;
int 			joyymove;
 
int 			savegameslot; 
char			savedescription[32]; 
 
 
#define BODYQUESIZE 	32

mobj_t* 		bodyque[BODYQUESIZE]; 
int 			bodyqueslot; 
 
void*			statcopy;								// for statistics driver
 
 
 
int G_CmdChecksum (ticcmd_t* cmd) 
{ 
	int 		i;
	int 		sum = 0; 
		 
	for (i=0 ; i< sizeof(*cmd)/4 - 1 ; i++) 
		sum += ((int *)cmd)[i]; 
				 
	return sum; 
} 

/* Impulses: Temporary hack to get weapon changing
 * working with keybindings until I can get the
 * inventory system working.
 */
int Impulse;

void Cmd_Impulse (player_t *plyr, int argc, char **argv)
{
	if (argc > 1)
		Impulse = atoi (argv[1]);
}

void Cmd_CenterView (player_t *plyr, int argc, char **argv)
{
	sendcenterview = true;
}

void Cmd_Pause (player_t *plyr, int argc, char **argv)
{
	sendpause = true;
}

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer. 
// If recording a demo, write it out 
// 
void G_BuildTiccmd (ticcmd_t* cmd) 
{ 
	boolean 	strafe;
	int 		speed;
	int 		tspeed; 
	int 		forward;
	int 		side;
	int			look;
	int			fly;
	
	ticcmd_t*	base;

	base = I_BaseTiccmd (); 			// empty, or external driver
	memcpy (cmd,base,sizeof(*cmd)); 
		
	cmd->consistancy = 
		consistancy[consoleplayer][maketic%BACKUPTICS]; 

 
	strafe = Actions & ACTION_STRAFE;
	speed = ((Actions & ACTION_SPEED) || (cl_run->value)) ? 1 : 0;
 
	forward = side = look = fly = 0;
	
	// use two stage accelerative turning
	// on the keyboard and joystick
	if (joyxmove < 0
		|| joyxmove > 0  
		|| (Actions & ACTION_LEFT)
		|| (Actions & ACTION_RIGHT))
		turnheld += ticdup; 
	else 
		turnheld = 0; 

	if (turnheld < SLOWTURNTICS) 
		tspeed = 2; 			// slow turn 
	else 
		tspeed = speed;
	
	// let movement keys cancel each other out
	if (strafe) 
	{ 
		if (Actions & ACTION_RIGHT) 
		{
			// fprintf(stderr, "strafe right\n");
			side += sidemove[speed]; 
		}
		if (Actions & ACTION_LEFT) 
		{
			//	fprintf(stderr, "strafe left\n");
			side -= sidemove[speed]; 
		}
		if (joyxmove > 0) 
			side += sidemove[speed]; 
		if (joyxmove < 0) 
			side -= sidemove[speed]; 
 
	} 
	else 
	{ 
		if (Actions & ACTION_RIGHT) 
			cmd->ucmd.yaw -= angleturn[tspeed]; 
		if (Actions & ACTION_LEFT) 
			cmd->ucmd.yaw += angleturn[tspeed]; 
		if (joyxmove > 0) 
			cmd->ucmd.yaw -= angleturn[tspeed]; 
		if (joyxmove < 0) 
			cmd->ucmd.yaw += angleturn[tspeed]; 
	} 

	if (Actions & ACTION_LOOKUP)
		look += lookspeed[speed];
	if (Actions & ACTION_LOOKDOWN)
		look -= lookspeed[speed];
	if (Actions & ACTION_MOVEUP)
		fly += flyspeed[speed];
	if (Actions & ACTION_MOVEDOWN)
		fly -= flyspeed[speed];


	if (Actions & ACTION_KLOOK) {
		if (Actions & ACTION_FORWARD)
			look += lookspeed[speed];
		if (Actions & ACTION_BACK)
			look -= lookspeed[speed];
	} else {
		if (Actions & ACTION_FORWARD)
			forward += forwardmove[speed];
		if (Actions & ACTION_BACK)
			forward -= forwardmove[speed];
	}

	if (joyymove < 0) 
		forward += forwardmove[speed]; 
	if (joyymove > 0) 
		forward -= forwardmove[speed]; 
	if (Actions & ACTION_MOVERIGHT) 
		side += sidemove[speed]; 
	if (Actions & ACTION_MOVELEFT) 
		side -= sidemove[speed];

	// buttons
	cmd->chatchar = HU_dequeueChatChar(); 
 
	if (Actions & ACTION_ATTACK) 
		cmd->ucmd.buttons |= BT_ATTACK; 
 
	if (Actions & ACTION_USE) 
		cmd->ucmd.buttons |= BT_USE;

	// chainsaw overrides 
	if (Impulse >= 1 && Impulse <= NUMWEAPONS-1) {
		cmd->ucmd.buttons |= BT_CHANGE;
		cmd->ucmd.buttons |= (Impulse - 1) << BT_WEAPONSHIFT;
		Impulse = 0;
	}

	if (Actions & ACTION_MLOOK) {
		look += mousey * 256;
		if (look > 32767)
			look = 32767;
		else if (look < -32767)
			look = -32767;
	} else {
		forward += mousey;
	}

	if (sendcenterview) {
		sendcenterview = false;
		look = -32768;
	}

	if (strafe) 
		side += mousex*2; 
	else 
		cmd->ucmd.yaw -= mousex*0x8; 

	mousex = mousey = 0; 
		 
	if (forward > MAXPLMOVE) 
		forward = MAXPLMOVE; 
	else if (forward < -MAXPLMOVE) 
		forward = -MAXPLMOVE; 
	if (side > MAXPLMOVE) 
		side = MAXPLMOVE; 
	else if (side < -MAXPLMOVE) 
		side = -MAXPLMOVE; 
 
	cmd->ucmd.forwardmove += forward; 
	cmd->ucmd.sidemove += side;
	cmd->ucmd.pitch = look;
	cmd->ucmd.upmove = fly;
	
	// special buttons
	if (sendpause) 
	{ 
		sendpause = false; 
		cmd->ucmd.buttons = BT_SPECIAL | BTS_PAUSE; 
	} 
 
	if (sendsave) 
	{ 
		sendsave = false; 
		cmd->ucmd.buttons = BT_SPECIAL | BTS_SAVEGAME | (savegameslot<<BTS_SAVESHIFT); 
	}

	cmd->ucmd.forwardmove *= 256;
	cmd->ucmd.sidemove *= 256;
} 
 

//
// G_DoLoadLevel 
//
extern	gamestate_t 	wipegamestate; 
 
void G_DoLoadLevel (void) 
{ 
	int 			i; 

	// Set the sky map.
	// First thing, we have a dummy sky texture name,
	//	a flat. The data is in the WAD only because
	//	we look for an actual index, instead of simply
	//	setting one.
	skyflatnum = R_FlatNumForName ( SKYFLATNAME );

	// DOOM determines the sky texture to be used
	// depending on the current episode, and the game version.
	if ( (gamemode == commercial)
		 || ( gamemode == pack_tnt )
		 || ( gamemode == pack_plut ) )
	{
		if (gamemap < 12)
			skytexture = R_TextureNumForName ("SKY1");
		else if (gamemap < 21)
			skytexture = R_TextureNumForName ("SKY2");
		else
			skytexture = R_TextureNumForName ("SKY3");
	}

	levelstarttic = gametic;		// for time calculation
	
	if (wipegamestate == GS_LEVEL) 
		wipegamestate = -1; 			// force a wipe 

	gamestate = GS_LEVEL; 

	for (i=0 ; i<MAXPLAYERS ; i++) 
	{ 
		if (playeringame[i] && players[i].playerstate == PST_DEAD) 
			players[i].playerstate = PST_REBORN; 
		memset (players[i].frags,0,sizeof(players[i].frags)); 
	} 
				 
	P_SetupLevel (gameepisode, gamemap, 0, gameskill->value);	 
	displayplayer = consoleplayer;				// view the guy you are playing    
	starttime = I_GetTime (); 
	gameaction = ga_nothing; 
	Z_CheckHeap ();
	
	// clear cmd building stuff
	Impulse = 0;
	Actions &= ACTION_MLOOK | ACTION_KLOOK;
	joyxmove = joyymove = 0; 
	mousex = mousey = 0; 
	sendpause = sendsave = paused = sendcenterview = false; 

	{
		char *levelname;

#define HU_TITLE		(mapnames[(gameepisode-1)*9+gamemap-1])
#define HU_TITLE2		(mapnames2[gamemap-1])
		extern char *mapnames[];
		extern char *mapnames2[];

		if (gamemode == registered || gamemode == shareware || gamemode == retail) {
			levelname = HU_TITLE;
		} else {
			levelname = HU_TITLE2;
		}

		Printf ("\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36"
				"\36\36\36\36\36\36\36\36\36\36\36\36\37\n%s\n\n",
				levelname);

		C_FlushDisplay ();
		C_HideConsole ();
	}
}
 
 
//
// G_Responder	
// Get info needed to make ticcmd_ts for the players.
// 
boolean G_Responder (event_t* ev) 
{ 
	// allow spy mode changes even during the demo
	if (gamestate == GS_LEVEL && ev->type == ev_keydown 
		&& ev->data1 == KEY_F12 && (singledemo || !deathmatch) )
	{
		// spy mode 
		do 
		{ 
			displayplayer++; 
			if (displayplayer == MAXPLAYERS) 
				displayplayer = 0; 
		} while (!playeringame[displayplayer] && displayplayer != consoleplayer); 
		return true; 
	}
	
	// any other key pops up menu if in demos
	if (gameaction == ga_nothing && !singledemo && 
		(demoplayback || gamestate == GS_DEMOSCREEN) 
		) 
	{ 
		if (ev->type == ev_keydown ||  
			(ev->type == ev_mouse && ev->data1) || 
			(ev->type == ev_joystick && ev->data1) ) 
		{ 
			M_StartControlPanel (); 
			return true; 
		} 
		return false; 
	} 
 
	if (gamestate == GS_LEVEL) 
	{ 
#if 0
		if (devparm && ev->type == ev_keydown && ev->data2 == ';') 
		{ 
			G_DeathMatchSpawnPlayer (0); 
			return true; 
		} 
#endif 

		if (HU_Responder (ev)) 
			return true;		// chat ate the event 
		if (ST_Responder (ev)) 
			return true;		// status window ate it 
		if (AM_Responder (ev)) 
			return true;		// automap ate it 
	} 
		 
	if (gamestate == GS_FINALE) 
	{ 
		if (F_Responder (ev)) 
			return true;		// finale ate the event 
	} 
		 
	switch (ev->type) 
	{ 
	  case ev_keydown: 
		C_DoKey (ev->data1, false);
/*		if (ev->data1 == KEY_PAUSE) 
		{ 
			sendpause = true; 
			return true; 
		} 
*/
		return true;	// eat key down events 
 
	  case ev_keyup: 
		C_DoKey (ev->data1, true);

		return false;	// always let key up events filter down 

	  // [RH] mouse buttons are now sent with key up/down events
	  case ev_mouse: 
		mousex = ev->data2*mouseSensitivity->value; 
		mousey = ev->data3*mouseSensitivity->value; 
		return true;	// eat events 
 
	  case ev_joystick: 
		joyxmove = ev->data2; 
		joyymove = ev->data3; 
		return true;	// eat events 
 
	  default: 
		break; 
	} 
 
	return false; 
} 
 
 
 
//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker (void) 
{ 
	int 		i;
	int 		buf; 
	ticcmd_t*	cmd;
	
	// do player reborns if needed
	for (i=0 ; i<MAXPLAYERS ; i++) 
		if (playeringame[i] && players[i].playerstate == PST_REBORN) 
			G_DoReborn (i);
	
	// do things to change the game state
	while (gameaction != ga_nothing) 
	{ 
		switch (gameaction) 
		{ 
		  case ga_loadlevel: 
			G_DoLoadLevel (); 
			break; 
		  case ga_newgame: 
			G_DoNewGame (); 
			break; 
		  case ga_loadgame: 
			G_DoLoadGame (); 
			break; 
		  case ga_savegame: 
			G_DoSaveGame (); 
			break; 
		  case ga_playdemo: 
			G_DoPlayDemo (); 
			break; 
		  case ga_completed: 
			G_DoCompleted (); 
			break; 
		  case ga_victory: 
			F_StartFinale (); 
			break; 
		  case ga_worlddone: 
			G_DoWorldDone (); 
			break; 
		  case ga_screenshot: 
			M_ScreenShot (); 
			gameaction = ga_nothing; 
			break; 
		  case ga_nothing: 
			break; 
		} 
	}
	
	// get commands, check consistancy,
	// and build new consistancy check
	buf = (gametic/ticdup)%BACKUPTICS; 
 
	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (playeringame[i]) 
		{ 
			cmd = &players[i].cmd; 
 
			memcpy (cmd, &netcmds[i][buf], sizeof(ticcmd_t)); 
 
			if (demoplayback) 
				G_ReadDemoTiccmd (cmd); 
			if (demorecording) 
				G_WriteDemoTiccmd (cmd);
			
			// check for turbo cheats
			if (cmd->ucmd.forwardmove > TURBOTHRESHOLD 
				&& !(gametic&31) && ((gametic>>5)&3) == i )
			{
				static char turbomessage[80];
				extern char *player_names[MAXPLAYERS];
				sprintf (turbomessage, "%s is turbo!",player_names[i]);
				players[consoleplayer].message = turbomessage;
			}
						
			if (netgame && !netdemo && !(gametic%ticdup) ) 
			{ 
				if (gametic > BACKUPTICS 
					&& consistancy[i][buf] != cmd->consistancy) 
				{ 
					I_Error ("consistency failure (%i should be %i)",
							 cmd->consistancy, consistancy[i][buf]); 
				} 
				if (players[i].mo) 
					consistancy[i][buf] = players[i].mo->x; 
				else 
					consistancy[i][buf] = rndindex; 
			} 
		}
	}
	
	// check for special buttons
	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (playeringame[i]) 
		{ 
			if (players[i].cmd.ucmd.buttons & BT_SPECIAL) 
			{ 
				switch (players[i].cmd.ucmd.buttons & BT_SPECIALMASK) 
				{ 
				  case BTS_PAUSE: 
					paused ^= 1; 
					if (paused) 
						S_PauseSound (); 
					else 
						S_ResumeSound (); 
					break; 
										 
				  case BTS_SAVEGAME: 
					if (!savedescription[0]) 
						strcpy (savedescription, "NET GAME"); 
					savegameslot =	
						(players[i].cmd.ucmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT; 
					gameaction = ga_savegame; 
					break; 
				} 
			} 
		}
	}
	
	// do main actions
	switch (gamestate) 
	{ 
	  case GS_LEVEL: 
		P_Ticker (); 
		ST_Ticker (); 
		AM_Ticker (); 
		HU_Ticker ();			 
		break; 
		 
	  case GS_INTERMISSION: 
		WI_Ticker (); 
		break; 
						 
	  case GS_FINALE: 
		F_Ticker (); 
		break; 
 
	  case GS_DEMOSCREEN: 
		D_PageTicker (); 
		break; 
	}		 
} 
 
 
//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_InitPlayer 
// Called at the start.
// Called by the game initialization functions.
//
void G_InitPlayer (int player) 
{ 
	player_t*	p; 
 
	// set up the saved info		 
	p = &players[player]; 
		 
	// clear everything else to defaults 
	G_PlayerReborn (player); 
		 
} 
 
 

//
// G_PlayerFinishLevel
// Call when a player completes a level.
//
void G_PlayerFinishLevel (int player) 
{ 
	player_t*	p; 
		 
	p = &players[player]; 
		 
	memset (p->powers, 0, sizeof (p->powers)); 
	memset (p->cards, 0, sizeof (p->cards)); 
	p->mo->flags &= ~MF_SHADOW; 		// cancel invisibility 
	p->extralight = 0;					// cancel gun flashes 
	p->fixedcolormap = 0;				// cancel ir gogles 
	p->damagecount = 0; 				// no palette changes 
	p->bonuscount = 0; 
} 
 

//
// G_PlayerReborn
// Called after a player dies 
// almost everything is cleared and initialized 
//
void G_PlayerReborn (int player) 
{ 
	player_t*	p; 
	int 		i; 
	int 		frags[MAXPLAYERS]; 
	int 		killcount;
	int 		itemcount;
	int 		secretcount; 
		 
	memcpy (frags,players[player].frags,sizeof(frags)); 
	killcount = players[player].killcount; 
	itemcount = players[player].itemcount; 
	secretcount = players[player].secretcount; 
		 
	p = &players[player]; 
	memset (p, 0, sizeof(*p)); 
 
	memcpy (players[player].frags, frags, sizeof(players[player].frags)); 
	players[player].killcount = killcount; 
	players[player].itemcount = itemcount; 
	players[player].secretcount = secretcount; 

	p->ysheer = 0;
	p->usedown = p->attackdown = true;	// don't do anything immediately 
	p->playerstate = PST_LIVE;		 
	p->health = MAXHEALTH; 
	p->readyweapon = p->pendingweapon = wp_pistol; 
	p->weaponowned[wp_fist] = true; 
	p->weaponowned[wp_pistol] = true; 
	p->ammo[am_clip] = 50; 
		 
	for (i=0 ; i<NUMAMMO ; i++) 
		p->maxammo[i] = maxammo[i]; 
				 
}

//
// G_CheckSpot	
// Returns false if the player cannot be respawned
// at the given mapthing_t spot  
// because something is occupying it 
//
void P_SpawnPlayer (mapthing_t* mthing); 
 
boolean
G_CheckSpot
( int			playernum,
  mapthing_t*	mthing ) 
{ 
	fixed_t 			x;
	fixed_t 			y; 
	subsector_t*		ss; 
	unsigned			an; 
	mobj_t* 			mo; 
	int 				i;
		
	if (!players[playernum].mo)
	{
		// first spawn of level, before corpses
		for (i=0 ; i<playernum ; i++)
			if (players[i].mo->x == mthing->x << FRACBITS
				&& players[i].mo->y == mthing->y << FRACBITS)
				return false;	
		return true;
	}
				
	x = mthing->x << FRACBITS; 
	y = mthing->y << FRACBITS; 
		 
	if (!P_CheckPosition (players[playernum].mo, x, y) ) 
		return false; 
 
	// flush an old corpse if needed 
	if (bodyqueslot >= BODYQUESIZE) 
		P_RemoveMobj (bodyque[bodyqueslot%BODYQUESIZE]); 
	bodyque[bodyqueslot%BODYQUESIZE] = players[playernum].mo; 
	bodyqueslot++; 
		
	// spawn a teleport fog 
	ss = R_PointInSubsector (x,y); 
	an = ( ANG45 * (mthing->angle/45) ) >> ANGLETOFINESHIFT; 
 
	mo = P_SpawnMobj (x+20*finecosine[an], y+20*finesine[an] 
					  , ss->sector->floorheight 
					  , MT_TFOG); 
		 
	// consoleplayer changed to displayplayer
	if (players[displayplayer].viewz != 1) 
		S_StartSound (mo, sfx_telept);	// don't start sound on first frame 
 
	return true; 
} 


//
// G_DeathMatchSpawnPlayer 
// Spawns a player at one of the random death match spots 
// called at level load and each death 
//
void G_DeathMatchSpawnPlayer (int playernum) 
{ 
	int 			i,j; 
	int 						selections; 
		 
	selections = deathmatch_p - deathmatchstarts;
	// 4 changed to doomcom->numplayers
	if (selections < doomcom->numplayers)
		I_Error ("Only %i deathmatch spots, %d required",
				 selections, doomcom->numplayers); 
 
	for (j=0 ; j<20 ; j++) 
	{ 
		i = P_Random() % selections; 
		if (G_CheckSpot (playernum, &deathmatchstarts[i]) ) 
		{ 
			deathmatchstarts[i].type = playernum+1; 
			P_SpawnPlayer (&deathmatchstarts[i]); 
			return; 
		} 
	} 
 
	// no good spot, so the player will probably get stuck 
	P_SpawnPlayer (&playerstarts[playernum]); 
} 

//
// G_DoReborn 
// 
void G_DoReborn (int playernum) 
{ 
	int i; 
		 
	if (!netgame)
	{
		// reload the level from scratch
		gameaction = ga_loadlevel;	
	}
	else 
	{
		// respawn at the start

		// first dissasociate the corpse 
		players[playernum].mo->player = NULL;	
				 
		// spawn at random spot if in death match 
		if (deathmatch) 
		{ 
			G_DeathMatchSpawnPlayer (playernum); 
			return; 
		} 
				 
		if (G_CheckSpot (playernum, &playerstarts[playernum]) ) 
		{ 
			P_SpawnPlayer (&playerstarts[playernum]); 
			return; 
		}
		
		// try to spawn at one of the other players spots 
		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (G_CheckSpot (playernum, &playerstarts[i]) ) 
			{ 
				playerstarts[i].type = playernum+1; 	// fake as other player 
				P_SpawnPlayer (&playerstarts[i]); 
				playerstarts[i].type = i+1; 			// restore 
				return; 
			}		
			// he's going to be inside something.  Too bad.
		}
		P_SpawnPlayer (&playerstarts[playernum]); 
	} 
} 
 
 
void G_ScreenShot (void) 
{ 
	gameaction = ga_screenshot; 
} 
 


// DOOM Par Times
int pars[4][10] = 
{ 
	{0}, 
	{0,30,75,120,90,165,180,180,30,165}, 
	{0,90,90,90,120,90,360,240,30,170}, 
	{0,90,45,90,150,90,90,165,30,135} 
}; 

// DOOM II Par Times
int cpars[32] =
{
	30,90,120,120,90,150,120,120,270,90,		//	1-10
	210,150,150,150,210,150,420,150,210,150,	// 11-20
	240,150,180,150,150,300,330,420,300,180,	// 21-30
	120,30										// 31-32
};
 

//
// G_DoCompleted 
//
boolean 		secretexit; 
extern char*	pagename; 
 
void G_ExitLevel (void) 
{ 
	secretexit = false; 
	gameaction = ga_completed; 
} 

// Here's for the german edition.
void G_SecretExitLevel (void) 
{ 
	// IF NO WOLF3D LEVELS, NO SECRET EXIT!
	if ( (gamemode == commercial)
	  && (W_CheckNumForName("map31")<0))
		secretexit = false;
	else
		secretexit = true; 
	gameaction = ga_completed; 
} 
 
void G_DoCompleted (void) 
{ 
	int 			i; 
		 
	gameaction = ga_nothing; 
 
	for (i=0 ; i<MAXPLAYERS ; i++) 
		if (playeringame[i]) 
			G_PlayerFinishLevel (i);		// take away cards and stuff 
		 
	if (automapactive) 
		AM_Stop (); 
		
	if ( gamemode != commercial)
		switch(gamemap)
		{
		  case 8:
			gameaction = ga_victory;
			return;
		  case 9: 
			for (i=0 ; i<MAXPLAYERS ; i++) 
				players[i].didsecret = true; 
			break;
		}
				
//#if 0  Hmmm - why?
	if ( (gamemap == 8)
		 && (gamemode != commercial) ) 
	{
		// victory 
		gameaction = ga_victory; 
		return; 
	} 
		 
	if ( (gamemap == 9)
		 && (gamemode != commercial) ) 
	{
		// exit secret level 
		for (i=0 ; i<MAXPLAYERS ; i++) 
			players[i].didsecret = true; 
	} 
//#endif
	
		 
	wminfo.didsecret = players[consoleplayer].didsecret; 
	wminfo.epsd = gameepisode -1; 
	wminfo.last = gamemap -1;
	
	// wminfo.next is 0 biased, unlike gamemap
	if ( gamemode == commercial)
	{
		if (secretexit)
			switch(gamemap)
			{
			  case 15: wminfo.next = 30; break;
			  case 31: wminfo.next = 31; break;
			}
		else
			switch(gamemap)
			{
			  case 31:
			  case 32: wminfo.next = 15; break;
			  default: wminfo.next = gamemap;
			}
	}
	else
	{
		if (secretexit) 
			wminfo.next = 8;	// go to secret level 
		else if (gamemap == 9) 
		{
			// returning from secret level 
			switch (gameepisode) 
			{ 
			  case 1: 
				wminfo.next = 3; 
				break; 
			  case 2: 
				wminfo.next = 5; 
				break; 
			  case 3: 
				wminfo.next = 6; 
				break; 
			  case 4:
				wminfo.next = 2;
				break;
			}				 
		} 
		else 
			wminfo.next = gamemap;			// go to next level 
	}
				 
	wminfo.maxkills = totalkills; 
	wminfo.maxitems = totalitems; 
	wminfo.maxsecret = totalsecret; 
	wminfo.maxfrags = 0; 
	if ( gamemode == commercial )
		wminfo.partime = 35*cpars[gamemap-1]; 
	else
		wminfo.partime = 35*pars[gameepisode][gamemap]; 
	wminfo.pnum = consoleplayer; 
 
	for (i=0 ; i<MAXPLAYERS ; i++) 
	{ 
		wminfo.plyr[i].in = playeringame[i]; 
		wminfo.plyr[i].skills = players[i].killcount; 
		wminfo.plyr[i].sitems = players[i].itemcount; 
		wminfo.plyr[i].ssecret = players[i].secretcount; 
		wminfo.plyr[i].stime = leveltime; 
		memcpy (wminfo.plyr[i].frags, players[i].frags 
				, sizeof(wminfo.plyr[i].frags)); 
	} 
 
	gamestate = GS_INTERMISSION; 
	viewactive = false; 
	automapactive = false; 
 
	if (statcopy)
		memcpy (statcopy, &wminfo, sizeof(wminfo));
		
	WI_Start (&wminfo); 
} 


//
// G_WorldDone 
//
void G_WorldDone (void) 
{ 
	gameaction = ga_worlddone; 

	if (secretexit) 
		players[consoleplayer].didsecret = true; 

	if ( gamemode == commercial )
	{
		switch (gamemap)
		{
		  case 15:
		  case 31:
			if (!secretexit)
				break;
		  case 6:
		  case 11:
		  case 20:
		  case 30:
			F_StartFinale ();
			break;
		}
	}
} 
 
void G_DoWorldDone (void) 
{		 
	gamestate = GS_LEVEL; 
	gamemap = wminfo.next+1; 
	G_DoLoadLevel (); 
	gameaction = ga_nothing; 
	viewactive = true; 
} 
 


//
// G_InitFromSavegame
// Can be called by the startup code or the menu task. 
//
extern boolean setsizeneeded;
void R_ExecuteSetViewSize (void);

char	savename[256];

void G_LoadGame (char* name) 
{ 
	strcpy (savename, name); 
	gameaction = ga_loadgame; 
} 
 
#define VERSIONSIZE 			16 


void G_DoLoadGame (void) 
{ 
	int 		length; 
	int 		i; 
	int 		a,b,c; 
	char		vcheck[VERSIONSIZE]; 
		 
	gameaction = ga_nothing; 
		 
	length = M_ReadFile (savename, &savebuffer); 
	save_p = savebuffer + SAVESTRINGSIZE;
	
	// skip the description field 
	memset (vcheck,0,sizeof(vcheck)); 
	sprintf (vcheck,"version %i",VERSION); 
	if (strcmp (save_p, vcheck)) 
		return; 						// bad version 
	save_p += VERSIONSIZE; 
						 
		SetCVarFloat (gameskill, *save_p++);
	gameepisode = *save_p++; 
	gamemap = *save_p++; 
	for (i=0 ; i<MAXPLAYERS ; i++) 
		playeringame[i] = *save_p++; 

	// load a base level 
	G_InitNew (gameskill->value, gameepisode, gamemap); 
 
	// get the times 
	a = *save_p++; 
	b = *save_p++; 
	c = *save_p++; 
	leveltime = (a<<16) + (b<<8) + c; 
		 
	// dearchive all the modifications
	P_UnArchivePlayers (); 
	P_UnArchiveWorld (); 
	P_UnArchiveThinkers (); 
	P_UnArchiveSpecials (); 
 
	if (*save_p != 0x1d) 
		I_Error ("Bad savegame");
	
	// done 
	Z_Free (savebuffer); 
 
	if (setsizeneeded)
		R_ExecuteSetViewSize ();
	
	// draw the pattern into the back screen
	R_FillBackScreen ();   
} 
 

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string 
//
void
G_SaveGame
( int	slot,
  char* description ) 
{ 
	savegameslot = slot; 
	strcpy (savedescription, description); 
	sendsave = true; 
} 
 
void G_DoSaveGame (void) 
{ 
	char		name[100]; 
	char		name2[VERSIONSIZE]; 
	char*		description; 
	int 		length; 
	int 		i; 
		
	if (M_CheckParm("-cdrom"))
		sprintf(name,"c:\\doomdata\\"SAVEGAMENAME"%d.dsg",savegameslot);
	else
		sprintf (name,SAVEGAMENAME"%d.dsg",savegameslot); 
	description = savedescription; 
		 
	save_p = savebuffer = screens[1]+0x4000; 
		 
	memcpy (save_p, description, SAVESTRINGSIZE); 
	save_p += SAVESTRINGSIZE; 
	memset (name2,0,sizeof(name2)); 
	sprintf (name2,"version %i",VERSION); 
	memcpy (save_p, name2, VERSIONSIZE); 
	save_p += VERSIONSIZE; 
		 
	*save_p++ = (byte)gameskill->value; 
	*save_p++ = gameepisode; 
	*save_p++ = gamemap; 
	for (i=0 ; i<MAXPLAYERS ; i++) 
		*save_p++ = playeringame[i]; 
	*save_p++ = leveltime>>16; 
	*save_p++ = leveltime>>8; 
	*save_p++ = leveltime; 
 
	P_ArchivePlayers (); 
	P_ArchiveWorld (); 
	P_ArchiveThinkers (); 
	P_ArchiveSpecials (); 
		 
	*save_p++ = 0x1d;			// consistancy marker 
		 
	length = save_p - savebuffer; 
	if (length > SAVEGAMESIZE) 
		I_Error ("Savegame buffer overrun"); 
	M_WriteFile (name, savebuffer, length); 
	gameaction = ga_nothing; 
	savedescription[0] = 0; 			 
		 
	players[consoleplayer].message = GGSAVED; 

	// draw the pattern into the back screen
	R_FillBackScreen ();		
} 
 

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set. 
//
skill_t d_skill; 
int 	d_episode; 
int 	d_map; 
 
void
G_DeferedInitNew
( skill_t		skill,
  int			episode,
  int			map) 
{ 
	d_skill = skill; 
	d_episode = episode; 
	d_map = map; 
	gameaction = ga_newgame; 
} 


void G_DoNewGame (void) 
{
	demoplayback = false; 
	netdemo = false;
	netgame = false;
	deathmatch = 0;
	// playeringame[1] = playeringame[2] = playeringame[3] = 0;
	{
		int i;

		for (i = 1; i < MAXPLAYERS; i++)
			playeringame[i] = 0;
	}
	respawnparm = false;
	fastparm = 0;
	nomonsters = false;
	consoleplayer = 0;
	G_InitNew (d_skill, d_episode, d_map); 
	gameaction = ga_nothing; 
} 

// The sky texture to be used instead of the F_SKY1 dummy.
extern	int 	skytexture; 


void
G_InitNew
( skill_t		skill,
  int			episode,
  int			map ) 
{ 
	int 			i; 

	if (paused) 
	{ 
		paused = false; 
		S_ResumeSound (); 
	} 
		

	if (skill > sk_nightmare) 
		skill = sk_nightmare;


	// This was quite messy with SPECIAL and commented parts.
	// Supposedly hacks to make the latest edition work.
	// It might not work properly.
	if (episode < 1)
	  episode = 1; 

	if ( gamemode == retail )
	{
	  if (episode > 4)
		episode = 4;
	}
	else if ( gamemode == shareware )
	{
	  if (episode > 1) 
		   episode = 1; // only start episode 1 on shareware
	}  
	else
	{
	  if (episode > 3)
		episode = 3;
	}
	

  
	if (map < 1) 
		map = 1;
	
	if ( (map > 9)
		 && ( gamemode != commercial) )
	  map = 9; 
				 
	M_ClearRandom (); 
		 
	if (skill == sk_nightmare || respawnparm )
		respawnmonsters = true;
	else
		respawnmonsters = false;
				
	if (fastparm || (skill == sk_nightmare && gameskill->value != sk_nightmare) )
	{ 
		for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++) 
			states[i].tics >>= 1; 
		mobjinfo[MT_BRUISERSHOT].speed = 20*FRACUNIT; 
		mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT; 
		mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT; 
	} 
	else if (skill != sk_nightmare && gameskill->value == sk_nightmare) 
	{ 
		for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++) 
			states[i].tics <<= 1; 
		mobjinfo[MT_BRUISERSHOT].speed = 15*FRACUNIT; 
		mobjinfo[MT_HEADSHOT].speed = 10*FRACUNIT; 
		mobjinfo[MT_TROOPSHOT].speed = 10*FRACUNIT; 
	} 
		 
						 
	// force players to be initialized upon first level load		 
	for (i=0 ; i<MAXPLAYERS ; i++) 
		players[i].playerstate = PST_REBORN; 
 
	usergame = true;				// will be set false if a demo 
	paused = false; 
	demoplayback = false; 
	automapactive = false; 
	viewactive = true; 
	gameepisode = episode; 
	gamemap = map;
	SetCVarFloat (gameskill, skill);
 
	viewactive = true;
	
	// set the sky map for the episode
	if ( gamemode == commercial)
	{
		if (gamemap < 12) {
			skytexture = R_TextureNumForName ("SKY1");
		} else if (gamemap < 21) {
			skytexture = R_TextureNumForName ("SKY2");
		} else {
			skytexture = R_TextureNumForName ("SKY3");
		}
	}
	else
		switch (episode) 
		{ 
		  case 1: 
			skytexture = R_TextureNumForName ("SKY1"); 
			break; 
		  case 2: 
			skytexture = R_TextureNumForName ("SKY2"); 
			break; 
		  case 3: 
			skytexture = R_TextureNumForName ("SKY3"); 
			break; 
		  case 4:		// Special Edition sky
			skytexture = R_TextureNumForName ("SKY4");
			break;
		} 
 
	G_DoLoadLevel (); 
} 
 

//
// DEMO RECORDING 
// 
#define DEMOMARKER				0x80


void G_ReadDemoTiccmd (ticcmd_t* cmd)
{
	if (olddemo) {
		if (*demo_p == DEMOMARKER) {
			// end of demo data stream
			G_CheckDemoStatus ();
		} else {
			cmd->ucmd.forwardmove = ((signed char)*demo_p++) << 8;
			cmd->ucmd.sidemove = ((signed char)*demo_p++) << 8;
			cmd->ucmd.yaw = ((unsigned char)*demo_p++)<<8;
			cmd->ucmd.buttons = (unsigned char)*demo_p++;
		}
	} else {
		short id = SVC_NOP, len;

		while (id != SVC_USERCMD) {
			len = ReadWord (&demo_p);
			id = ReadByte (&demo_p);

			if (len == ZDEMO_STOP) {
				// end of demo stream
				G_CheckDemoStatus ();
				break;
			} else if (len > MAX_MSGLEN) {
				I_Error ("Message too long\n(%d bytes, max is %d)", len, MAX_MSGLEN);
				break;
			}

			switch (id) {
				char *s;

				case SVC_BAD:
					I_Error ("Received bad message");
					break;
				case SVC_USERCMD:
					demo_p += len - UnpackUserCmd (&cmd->ucmd, &demo_p);
					break;
				case SVC_STUFFTEXT:
					s = ReadString (&demo_p);
					demo_p += len - strlen (s) - 1;
					AddCommandString (s);
					free (s);
					break;
				case SVC_MUSICCHANGE:
					s = ReadString (&demo_p);
					demo_p += len - strlen (s) - 1;
					Printf ("Music change to %s not implemented yet.\n", s);
					free (s);
					break;
				case SVC_PRINT:
					s = ReadString (&demo_p);
					demo_p += len - strlen (s) - 1;
					Printf (s);
					break;
				case SVC_CENTERPRINT:
					s = ReadString (&demo_p);
					demo_p += len - strlen (s) - 1;
					C_MidPrint (s);
					break;
				case SVC_NOP:
					break;
				default:
					I_Error ("Received unknown message %d", id);
					break;
			}
		}
	}
} 

boolean stoprecording;

void Cmd_Stop (player_t *plyr, int argc, char **argv)
{
	stoprecording = true;
}

void G_WriteDemoTiccmd (ticcmd_t* cmd) 
{
	int byteswritten;

	// Always write new demos
	if (stoprecording)		   // use stop command to end demo recording 
		G_CheckDemoStatus ();
	byteswritten = WriteUserCmdMessage (&cmd->ucmd, &demo_p);
	demo_p -= byteswritten;

	if (demo_p > demoend - 16) {
		// no more space 
		G_CheckDemoStatus (); 
		return; 
	} 
		
	G_ReadDemoTiccmd (cmd); 		// make SURE it is exactly the same 
} 
 
 
 
//
// G_RecordDemo 
// 
void G_RecordDemo (char* name) 
{ 
	int 			i; 
	int 			maxsize;
		
	usergame = false;
	strcpy (demoname, name);
	DefaultExtension (demoname, ".lmp");
	maxsize = 0x20000;
	i = M_CheckParm ("-maxdemo");
	if (i && i<myargc-1)
		maxsize = atoi(myargv[i+1])*1024;
	demobuffer = Z_Malloc (maxsize,PU_STATIC,NULL); 
	demoend = demobuffer + maxsize;
		
	demorecording = true; 
} 
 
 
void G_BeginRecording (void) 
{ 
	int i; 

	olddemo = false;
	demo_p = demobuffer;

	WriteLong (ZDEMO_ID, &demo_p);
	*demo_p++ = VERSION / 100;
	*demo_p++ = VERSION % 100;
	*demo_p++ = 1;
	*demo_p++ = 11;

	*demo_p++ = MAXPLAYERS;
	*demo_p++ = (byte)gameskill->value; 
	*demo_p++ = gameepisode; 
	*demo_p++ = gamemap; 
	*demo_p++ = deathmatch; 
	*demo_p++ = respawnparm;
	*demo_p++ = fastparm;
	*demo_p++ = nomonsters;
	*demo_p++ = consoleplayer;
		 
	for (i=0 ; i<MAXPLAYERS ; i++)
		*demo_p++ = playeringame[i];

	// Quick test
	{
		char s[256];

		sprintf (s, "Skill: %g\nEpisode: %d\nMap: %d",
				 gameskill->value, gameepisode, gamemap);
		WriteWord ((short)(strlen(s) + 1), &demo_p);
		WriteByte (SVC_CENTERPRINT, &demo_p);
		WriteString (s, &demo_p);
		C_MidPrint (s);
	}
} 
 

//
// G_PlayDemo 
//

char*	defdemoname; 
 
void G_DeferedPlayDemo (char* name) 
{ 
	defdemoname = name; 
	gameaction = ga_playdemo; 
} 

void G_DoPlayDemo (void) 
{ 
	skill_t skill; 
	int		i, episode, map, players = 4;
	int		demoversion = 5;

	gameaction = ga_nothing;
	demobuffer = demo_p = W_CacheLumpName (defdemoname, PU_STATIC);

	if (ReadLong (&demo_p) == ZDEMO_ID) {
		short gamever = ((VERSION % 100)) | ((VERSION / 100) << 8);

		olddemo = false;
		if (ReadWord (&demo_p) <= gamever) {
			if (ReadWord (&demo_p) >= gamever) {
				players = *demo_p++;
				demoversion = VERSION;
			}
		}
	} else {
		olddemo = true;
		demo_p -= 4;
		demoversion = *demo_p++;
		players = 4;
	}

	if ((demoversion > 4 && demoversion < 104) || (demoversion > VERSION)) {
		Printf ("Demo is from unknown game version!\n");
		gameaction = ga_nothing;
		Z_Free (demobuffer);
		return;
	}
	if (players > MAXPLAYERS) {
		Printf ("Demo has too many players (%d)\n", players);
		gameaction = ga_nothing;
		Z_Free (demobuffer);
		return;
	}

	if (demoversion <= 4) {
		skill = (float)demoversion;
		episode = *demo_p++;
		map = *demo_p++;
		deathmatch = 0;
		respawnparm = 0;
		fastparm = 0;
		nomonsters = 0;
		consoleplayer = 0;
	} else {
		skill = (float)(*demo_p++);
		episode = *demo_p++;
		map = *demo_p++;
		deathmatch = *demo_p++;
		respawnparm = *demo_p++;
		fastparm = *demo_p++;
		nomonsters = *demo_p++;
		consoleplayer = *demo_p++;
	}

	for (i=0 ; i < MAXPLAYERS ; i++) {
		if (i < players)
			playeringame[i] = *demo_p++;
		else
			playeringame[i] = 0;
	}
	if (players > MAXPLAYERS)
		demo_p += players - i;

	if (playeringame[1]) {
		netgame = true;
		netdemo = true;
	}

	// don't spend a lot of time in loadlevel 
	precache = false;
	G_InitNew (skill, episode, map); 
	precache = true; 

	usergame = false; 
	demoplayback = true; 
} 

//
// G_TimeDemo 
//
void G_TimeDemo (char* name) 
{		 
	nodrawers = M_CheckParm ("-nodraw"); 
	noblit = M_CheckParm ("-noblit"); 
	timingdemo = true; 
	singletics = true; 

	defdemoname = name; 
	gameaction = ga_playdemo; 
} 
 
 
/* 
=================== 
= 
= G_CheckDemoStatus 
= 
= Called after a death or level completion to allow demos to be cleaned up 
= Returns true if a new demo loop action will take place 
=================== 
*/ 
 
boolean G_CheckDemoStatus (void) 
{ 
	int 			endtime; 
		 
	if (timingdemo) 
	{ 
		endtime = I_GetTime ();
		endtime -= starttime;
		I_Error ("timed %i gametics in %i realtics (%.1f fps)",gametic 
				 , endtime, (float)gametic/(float)endtime*(float)TICRATE);
	} 
		 
	if (demoplayback) 
	{ 
		if (singledemo) 
			I_Quit (); 
						 
		Z_ChangeTag (demobuffer, PU_CACHE); 
		demoplayback = false; 
		netdemo = false;
		netgame = false;
		deathmatch = 0;
		//playeringame[1] = playeringame[2] = playeringame[3] = 0;
		{
			int i;

			for (i = 1; i < MAXPLAYERS; i++)
				playeringame[i] = 0;
		}
		respawnparm = false;
		fastparm = 0;
		nomonsters = false;
		consoleplayer = 0;
		D_AdvanceDemo (); 
		return true; 
	} 
 
	if (demorecording) 
	{
		WriteWord (ZDEMO_STOP, &demo_p);
		WriteByte (SVC_NOP, &demo_p);
		M_WriteFile (demoname, demobuffer, demo_p - demobuffer); 
		Z_Free (demobuffer); 
		demorecording = false;
		stoprecording = false;
		I_Error ("Demo %s recorded",demoname); 
	} 
		 
	return false; 
} 
 
 
 
