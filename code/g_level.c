#include "d_main.h"
#include "m_alloc.h"
#include "g_level.h"
#include "g_game.h"
#include "s_sound.h"
#include "d_event.h"
#include "m_random.h"
#include "doomstat.h"
#include "wi_stuff.h"
#include "r_data.h"
#include "w_wad.h"
#include "am_map.h"
#include "c_dispch.h"
#include "z_zone.h"
#include "i_system.h"
#include "p_setup.h"
#include "p_local.h"
#include "r_sky.h"
#include "c_consol.h"
#include "f_finale.h"
#include "dstrings.h"
#include "v_video.h"
#include "st_stuff.h"
#include "hu_stuff.h"

static level_info_t *FindDefLevelInfo (char *mapname);
static cluster_info_t *FindDefClusterInfo (int cluster);

extern int timingdemo;

// Start time for timing demos
int starttime;


extern BOOL netdemo;

extern int mousex, mousey, joyxmove, joyymove, Impulse;
extern BOOL sendpause, sendsave, sendcenterview;

void *statcopy;					// for statistics driver

level_locals_t level;			// info about current level

static level_pwad_info_t *wadlevelinfos;
static cluster_info_t *wadclusterinfos;
static int numwadlevelinfos = 0;
static int numwadclusterinfos = 0;

static int FindWadLevelInfo (char *name)
{
	int i;

	for (i = 0; i < numwadlevelinfos; i++)
		if (!strnicmp (name, wadlevelinfos[i].mapname, 8))
			return i;
		
	return -1;
}

static int FindWadClusterInfo (int cluster)
{
	int i;

	for (i = 0; i < numwadclusterinfos; i++)
		if (wadclusterinfos[i].cluster == cluster)
			return i;
		
	return -1;
}


//
// G_ParseMapInfo
// Parses the MAPINFO lumps of all loaded WADs and generates
// data for wadlevelinfos and wadclusterinfos.
//
void G_ParseMapInfo (void)
{
	int lastlump, lump;
	char *mapinfo, *data;
	int levelindex = -1, clusterindex = -1;
	level_pwad_info_t *levelinfo;
	cluster_info_t *clusterinfo;
	unsigned levelflags;

	lastlump = 0;
	while ((lump = W_FindLump ("MAPINFO", &lastlump)) != -1) {
		mapinfo = W_CacheLumpNum (lump, PU_CACHE);

		while ( (data = COM_Parse (mapinfo)) ) {
			if (!stricmp (com_token, "map")) {
				// map <MAPNAME> <Nice Name>
				if (levelindex > -1) {
					// Store previous level's flags
					levelinfo->flags = levelflags;
				}
				levelflags = 0;
				clusterindex = -1;
				data = COM_Parse (data);
				if ((levelindex = FindWadLevelInfo (com_token)) == -1)
				{
					levelindex = numwadlevelinfos++;
					wadlevelinfos = Realloc (wadlevelinfos, sizeof(level_pwad_info_t)*numwadlevelinfos);
					memset (wadlevelinfos + levelindex, 0, sizeof(level_pwad_info_t));
				}
				levelinfo = wadlevelinfos + levelindex;
				strncpy (levelinfo->mapname, com_token, 8);
				data = COM_Parse (data);
				ReplaceString (&levelinfo->level_name, com_token);

				mapinfo = data;
			} else if (levelindex > -1) {
				if (!stricmp (com_token, "levelnum")) {
					// levelnum <levelnum>
					mapinfo = COM_Parse (data);
					levelinfo->levelnum = atoi (com_token);

				} else if (!stricmp (com_token, "next")) {
					// next <MAPNAME>
					mapinfo = COM_Parse (data);
					strncpy (levelinfo->nextmap, com_token, 8);

				} else if (!stricmp (com_token, "secretnext")) {
					// secretnext <MAPNAME>
					mapinfo = COM_Parse (data);
					strncpy (levelinfo->secretmap, com_token, 8);

				} else if (!stricmp (com_token, "cluster")) {
					// cluster <clusternum>
					mapinfo = COM_Parse (data);
					levelinfo->cluster = atoi (com_token);

				} else if (!stricmp (com_token, "sky1")) {
					// sky1 <sky1texture> <sky1scrollspeed>
					mapinfo = COM_Parse (data);
					strncpy (levelinfo->skypic1, com_token, 8);
					mapinfo = COM_Parse (mapinfo);
					levelinfo->skyspeed1 = (int)(atof (com_token) * 65536.0);

				} else if (!stricmp (com_token, "sky2")) {
					// sky2 <sky2texture> <sky2scrollspeed>
					mapinfo = COM_Parse (data);
					strncpy (levelinfo->skypic2, com_token, 8);
					mapinfo = COM_Parse (mapinfo);
					levelinfo->skyspeed2 = (int)(atof (com_token) * 65536.0);

				} else if (!stricmp (com_token, "fade")) {
					// fade <colorname> OR fade <colordescriptor>
					char *colorstring;

					mapinfo = COM_Parse (data);
					if ( (colorstring = V_GetColorStringByName (com_token)) ) {
						levelinfo->fadeto = V_GetColorFromString (NULL, colorstring);
						free (colorstring);
					} else {
						levelinfo->fadeto = V_GetColorFromString (NULL, com_token);
					}
				
				} else if (!stricmp (com_token, "titlepatch")) {
					// titlepatch <patchname>
					mapinfo = COM_Parse (data);
					strncpy (levelinfo->pname, com_token, 8);
					
				} else if (!stricmp (com_token, "par")) {
					// par <partime (in seconds)>
					mapinfo = COM_Parse (data);
					levelinfo->partime = atoi (com_token);
				
				} else if (!stricmp (com_token, "music")) {
					// music <musiclump>
					mapinfo = COM_Parse (data);
					strncpy (levelinfo->music, com_token, 8);

				} else if (!stricmp (com_token, "nointermission")) {
					// nointermission
					mapinfo = data;
					levelflags |= LEVEL_NOINTERMISSION;
				
				} else if (!stricmp (com_token, "doublesky")) {
					// doublesky
					mapinfo = data;
					levelflags |= LEVEL_DOUBLESKY;

				} else if (!stricmp (com_token, "nosoundclipping")) {
					// nosoundclipping
					mapinfo = data;
					levelflags |= LEVEL_NOSOUNDCLIPPING;

				} else if (!stricmp (com_token, "allowmonstertelefrags")) {
					// allowmonstertelefrags
					mapinfo = data;
					levelflags |= LEVEL_MONSTERSTELEFRAG;

				} else if (!stricmp (com_token, "map07special")) {
					// map07special
					mapinfo = data;
					levelflags |= LEVEL_MAP07SPECIAL;

				} else if (!stricmp (com_token, "baronspecial")) {
					// baronspecial
					mapinfo = data;
					levelflags |= LEVEL_BRUISERSPECIAL;

				} else if (!stricmp (com_token, "cyberdemonspecial")) {
					// cyberdemonspecial
					mapinfo = data;
					levelflags |= LEVEL_CYBORGSPECIAL;

				} else if (!stricmp (com_token, "spidermastermindspecial")) {
					// spidermastermindspecial
					mapinfo = data;
					levelflags |= LEVEL_SPIDERSPECIAL;

				} else if (!stricmp (com_token, "specialaction_exitlevel")) {
					// specialaction_exitlevel
					mapinfo = data;
					levelflags = (levelflags & (~LEVEL_SPECACTIONSMASK));

				} else if (!stricmp (com_token, "specialaction_opendoor")) {
					// specialaction_opendoor
					mapinfo = data;
					levelflags = (levelflags & (~LEVEL_SPECACTIONSMASK)) | LEVEL_SPECOPENDOOR;

				} else if (!stricmp (com_token, "specialaction_lowerfloor")) {
					// specialaction_lowerfloor
					mapinfo = data;
					levelflags = (levelflags & (~LEVEL_SPECACTIONSMASK)) | LEVEL_SPECLOWERFLOOR;

				}
			}
			
			if (!stricmp (com_token, "clusterdef")) {
				// clusterdef <clusternum>
				if (levelindex > -1) {
					// Store previous level's flags
					levelinfo->flags = levelflags;
					levelindex = -1;
				}
				mapinfo = COM_Parse (data);
				if ((clusterindex = FindWadClusterInfo (atoi (com_token))) == -1)
				{
					clusterindex = numwadclusterinfos++;
					wadclusterinfos = Realloc (wadclusterinfos, sizeof(cluster_info_t)*numwadclusterinfos);
					memset (wadclusterinfos + clusterindex, 0, sizeof(cluster_info_t));
				}
				clusterinfo = wadclusterinfos + clusterindex;
				clusterinfo->cluster = atoi (com_token);
			} else if (clusterindex > -1) {
				if (!stricmp (com_token, "entertext")) {
					// entertext <"text to display when entering cluster">
					mapinfo = COM_Parse (data);
					ReplaceString (&clusterinfo->entertext, com_token);

				} else if (!stricmp (com_token, "exittext")) {
					// exittext <"text to display when leaving cluster">
					mapinfo = COM_Parse (data);
					ReplaceString (&clusterinfo->exittext, com_token);

				} else if (!stricmp (com_token, "music")) {
					// music <musiclump>
					mapinfo = COM_Parse (data);
					strncpy (clusterinfo->messagemusic, com_token, 8);
					clusterinfo->messagemusic[8] = 0;
				
				} else if (!stricmp (com_token, "flat")) {
					// flat <flatname>
					mapinfo = COM_Parse (data);
					strncpy (clusterinfo->finaleflat, com_token, 8);

				}
			}
		}
		if (levelindex > -1) {
			// Store previous level's flags
			levelinfo->flags = levelflags;
		}

	}
}

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set.
//
static char		d_mapname[9];

void G_DeferedInitNew (char *mapname)
{
	strncpy (d_mapname, mapname, 8);
	gameaction = ga_newgame;
}

void Cmd_Map (void *plyr, int argc, char **argv)
{
	if (argc > 1) {
		if (W_CheckNumForName (argv[1]) == -1)
			Printf ("No map %s\n", argv[1]);
		else
			G_DeferedInitNew (argv[1]);
	}
}

void G_DoNewGame (void)
{
	olddemo = false;
	if (demoplayback) {
		demoplayback = false;
		C_RestoreCVars ();
		D_SetupUserInfo ();
	}
	netdemo = false;
	netgame = false;
//	deathmatch = 0;
	{
		int i;

		for (i = 1; i < MAXPLAYERS; i++)
			playeringame[i] = 0;
	}
//	respawnparm = false;
//	fastparm = 0;
//	nomonsters = false;
	consoleplayer = 0;
	G_InitNew (d_mapname);
	gameaction = ga_nothing;
}

// Set if the last call to G_InitNew() was in nightmare mode
BOOL lastwasadream = false;

void G_InitNew (char *mapname)
{
	int i;

	// [RH] Mark all levels as not visited
	for (i = 0; i < numwadlevelinfos; i++)
		wadlevelinfos[i].flags &= ~LEVEL_VISITED;

	for (i = 0; LevelInfos[i].mapname[0]; i++)
		LevelInfos[i].flags &= ~LEVEL_VISITED;

	UnlatchCVars ();

	if (gameskill->value > sk_nightmare)
		SetCVarFloat (gameskill, sk_nightmare);
	else if (gameskill->value < sk_baby)
		SetCVarFloat (gameskill, sk_baby);

	UnlatchCVars ();

	if (paused)
	{
		paused = false;
		S_ResumeSound ();
	}

	// [RH] If this map doesn't exist, bomb out
	if (W_CheckNumForName (mapname) == -1) {
		I_Error ("Could not start map %s\n", mapname);
	}

	if (gameskill->value == sk_nightmare || dmflags & DF_MONSTERS_RESPAWN )
		respawnmonsters = true;
	else
		respawnmonsters = false;

	if (dmflags & DF_FAST_MONSTERS || (gameskill->value == sk_nightmare && !lastwasadream) )
	{
		lastwasadream = true;
		for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
			states[i].tics >>= 1;
		mobjinfo[MT_BRUISERSHOT].speed = 20*FRACUNIT;
		mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT;
		mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT;
	}
	else if (gameskill->value != sk_nightmare && lastwasadream)
	{
		lastwasadream = false;
		for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
			states[i].tics <<= 1;
		mobjinfo[MT_BRUISERSHOT].speed = 15*FRACUNIT;
		mobjinfo[MT_HEADSHOT].speed = 10*FRACUNIT;
		mobjinfo[MT_TROOPSHOT].speed = 10*FRACUNIT;
	}

	M_ClearRandom ();

	// force players to be initialized upon first level load
	for (i=0 ; i<MAXPLAYERS ; i++)
		players[i].playerstate = PST_REBORN;

	usergame = true;				// will be set false if a demo
	paused = false;
	demoplayback = false;
	automapactive = false;
	viewactive = true;
 
	viewactive = true;

	strncpy (level.mapname, mapname, 8);
	G_DoLoadLevel ();
}

//
// G_DoCompleted
//
BOOL 			secretexit;
extern char*	pagename;
 
void G_ExitLevel (void)
{
	secretexit = false;
	gameaction = ga_completed;
}

// Here's for the german edition.
void G_SecretExitLevel (void) 
{
	// [RH] Check for secret levels is done in 
	//		G_DoCompleted()

	secretexit = true;
	gameaction = ga_completed;
}

void G_DoCompleted (void) 
{
	int i; 

	gameaction = ga_nothing;

	// [RH] Mark this level as having been visited
	if (!(level.flags & LEVEL_CHANGEMAPCHEAT))
		FindLevelInfo (level.mapname)->flags |= LEVEL_VISITED;

	for (i=0 ; i<MAXPLAYERS ; i++)
		if (playeringame[i])
			G_PlayerFinishLevel (i);		// take away cards and stuff 

	if (automapactive)
		AM_Stop ();

	if (level.flags & LEVEL_NOINTERMISSION) {
		G_WorldDone ();
		return;
	}

	wminfo.epsd = level.cluster - 1;		// Only used for DOOM I.
	strncpy (wminfo.lname0, level.info->pname, 8);
	strncpy (wminfo.current, level.mapname, 8);

	if (deathmatch->value && dmflags & DF_SAME_LEVEL) {
		strncpy (wminfo.next, level.mapname, 8);
		strncpy (wminfo.lname1, level.info->pname, 8);
	} else {
		wminfo.next[0] = 0;
		if (secretexit) {
			if (W_CheckNumForName (level.secretmap) != -1) {
				strncpy (wminfo.next, level.secretmap, 8);
				strncpy (wminfo.lname1, FindLevelInfo (level.secretmap)->pname, 8);
			} else {
				secretexit = false;
			}
		}
		if (!wminfo.next[0]) {
			strncpy (wminfo.next, level.nextmap, 8);
			strncpy (wminfo.lname1, FindLevelInfo (level.nextmap)->pname, 8);
		}
	}

	wminfo.maxkills = level.total_monsters;
	wminfo.maxitems = level.total_items;
	wminfo.maxsecret = level.total_secrets;
	wminfo.maxfrags = 0;
	wminfo.partime = TICRATE * level.partime;
	wminfo.pnum = consoleplayer;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		wminfo.plyr[i].in = playeringame[i];
		wminfo.plyr[i].skills = players[i].killcount;
		wminfo.plyr[i].sitems = players[i].itemcount;
		wminfo.plyr[i].ssecret = players[i].secretcount;
		wminfo.plyr[i].stime = level.time;
		memcpy (wminfo.plyr[i].frags, players[i].frags
				, sizeof(wminfo.plyr[i].frags));
		wminfo.plyr[i].fragcount = players[i].fragcount;
	}

	gamestate = GS_INTERMISSION;
	viewactive = false;
	automapactive = false;

// [RH] If you ever get a statistics driver operational, uncomment this.
//	if (statcopy)
//		memcpy (statcopy, &wminfo, sizeof(wminfo));
		
	WI_Start (&wminfo);
}

//
// G_DoLoadLevel 
//
extern	gamestate_t 	wipegamestate; 
 
void G_DoLoadLevel (void) 
{ 
	int i; 

	G_InitLevelLocals ();

	Printf ("\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36"
			"\36\36\36\36\36\36\36\36\36\36\36\36\37\n%s\n\n",
			level.level_name);
	
	C_HideConsole ();
	C_FlushDisplay ();

	// Set the sky map.
	// First thing, we have a dummy sky texture name,
	//	a flat. The data is in the WAD only because
	//	we look for an actual index, instead of simply
	//	setting one.
	skyflatnum = R_FlatNumForName ( SKYFLATNAME );

	// DOOM determines the sky texture to be used
	// depending on the current episode, and the game version.
	// [RH] Fetch sky parameters from level_locals_t.
	sky1texture = R_TextureNumForName (level.skypic1);
	sky2texture = R_TextureNumForName (level.skypic2);

	// [RH] Set up details about sky rendering
	R_InitSkyMap (r_stretchsky);

	if (wipegamestate == GS_LEVEL) 
		wipegamestate = -1; 			// force a wipe 

	gamestate = GS_LEVEL; 

	for (i=0 ; i<MAXPLAYERS ; i++) 
	{ 
		if (playeringame[i] && players[i].playerstate == PST_DEAD) 
			players[i].playerstate = PST_REBORN; 
		memset (players[i].frags,0,sizeof(players[i].frags)); 
		players[i].fragcount = 0;
	} 

	S_ClearAmbients ();							// [RH] Clear all ambient sounds
	P_SetupLevel (level.mapname);	 
	displayplayer = consoleplayer;				// view the guy you are playing
	ST_Start();		// [RH] Make sure status bar knows who we are
	HU_Start();
	gameaction = ga_nothing; 
	Z_CheckHeap ();
	
	// clear cmd building stuff
	Impulse = 0;
	Actions &= ACTION_MLOOK | ACTION_KLOOK;
	joyxmove = joyymove = 0; 
	mousex = mousey = 0; 
	sendpause = sendsave = paused = sendcenterview = false; 

	level.starttime = I_GetTime ();

	if (timingdemo) {
		static BOOL firstTime = true;

		if (firstTime) {
			starttime = I_GetTimeReally ();
			firstTime = false;
		}
	}
}


//
// G_WorldDone 
//
void G_WorldDone (void) 
{ 
	cluster_info_t *nextcluster;
	cluster_info_t *thiscluster;

	gameaction = ga_worlddone; 

	if (level.flags & LEVEL_CHANGEMAPCHEAT)
		return;

	thiscluster = FindClusterInfo (level.cluster);
	if (!strncmp (level.nextmap, "EndGame", 7)) {
		F_StartFinale (thiscluster->messagemusic, thiscluster->finaleflat, thiscluster->exittext);
	} else {
		if (!secretexit)
			nextcluster = FindClusterInfo (FindLevelInfo (level.nextmap)->cluster);
		else
			nextcluster = FindClusterInfo (FindLevelInfo (level.secretmap)->cluster);

		if (nextcluster->cluster != level.cluster) {
			// Only start the finale if the next level's cluster is different than the current one.
			if (nextcluster->entertext) {
				F_StartFinale (nextcluster->messagemusic, nextcluster->finaleflat, nextcluster->entertext);
			} else if (thiscluster->exittext) {
				F_StartFinale (thiscluster->messagemusic, thiscluster->finaleflat, thiscluster->exittext);
			}
		}
	}
} 
 
void G_DoWorldDone (void) 
{		 
	gamestate = GS_LEVEL;
	if (wminfo.next[0] == 0) {
		// Don't die if no next map is given,
		// just repeat the current one.
		Printf ("No next map specified.\n");
	} else {
		strncpy (level.mapname, wminfo.next, 8);
	}
	G_DoLoadLevel (); 
	gameaction = ga_nothing; 
	viewactive = true; 
} 
 


void G_InitLevelLocals ()
{
	int oldfade = level.fadeto;
	level_info_t *info;
	int i;

	if ((i = FindWadLevelInfo (level.mapname)) > -1) {
		level_pwad_info_t *pinfo = wadlevelinfos + i;

		level.info = (level_info_t *)pinfo;
		level.skyspeed1 = pinfo->skyspeed1;
		level.skyspeed2 = pinfo->skyspeed2;
		level.fadeto = pinfo->fadeto;
		info = (level_info_t *)pinfo;
		strncpy (level.skypic2, pinfo->skypic2, 8);
	} else {
		info = FindDefLevelInfo (level.mapname);
		level.info = info;
		level.skyspeed1 =  level.skyspeed2 = 0;
		level.fadeto = 0;
		level.skypic2[0] = 0;
	}

	if (info->level_name) {
		level.partime = info->partime;
		level.cluster = info->cluster;
		level.flags = info->flags;
		level.levelnum = info->levelnum;

		strncpy (level.level_name, info->level_name, 63);
		strncpy (level.nextmap, info->nextmap, 8);
		strncpy (level.secretmap, info->secretmap, 8);
		strncpy (level.music, info->music, 8);
		strncpy (level.skypic1, info->skypic1, 8);
		if (!level.skypic2[0])
			strncpy (level.skypic2, level.skypic1, 8);
	} else {
		level.partime = level.cluster = 0;
		strcpy (level.level_name, "Unnamed");
		level.nextmap[0] =
			level.secretmap[0] =
			level.music[0] = 0;
			strncpy (level.skypic1, "SKY1", 8);
			strncpy (level.skypic2, "SKY1", 8);
		level.flags = 0;
		level.levelnum = 1;
	}

	if (oldfade != level.fadeto)
		RefreshPalettes ();
}

char *CalcMapName (int episode, int level)
{
	static char lumpname[9];

	if (gamemode == commercial) {
		sprintf (lumpname, "MAP%02d", level);
	} else {
		lumpname[0] = 'E';
		lumpname[1] = '0' + episode;
		lumpname[2] = 'M';
		lumpname[3] = '0' + level;
		lumpname[4] = 0;
	}
	return lumpname;
}

static level_info_t *FindDefLevelInfo (char *mapname)
{
	level_info_t *i;

	i = LevelInfos;
	while (i->level_name) {
		if (!strnicmp (i->mapname, mapname, 8))
			break;
		i++;
	}
	return i;
}

level_info_t *FindLevelInfo (char *mapname)
{
	int i;

	if ((i = FindWadLevelInfo (mapname)) > -1)
		return (level_info_t *)(wadlevelinfos + i);
	else
		return FindDefLevelInfo (mapname);
}

static cluster_info_t *FindDefClusterInfo (int cluster)
{
	cluster_info_t *i;

	i = ClusterInfos;
	while (i->cluster && i->cluster != cluster)
		i++;

	return i;
}

cluster_info_t *FindClusterInfo (int cluster)
{
	int i;

	if ((i = FindWadClusterInfo (cluster)) > -1)
		return wadclusterinfos + i;
	else
		return FindDefClusterInfo (cluster);
}

void G_SetLevelStrings (void)
{
	char temp[8];
	char *namepart;
	int i, start;

	temp[0] = '0';
	temp[1] = ':';
	temp[2] = 0;
	for (i = 65; i < 101; i++) {		// HUSTR_E1M1 .. HUSTR_E4M9
		if (temp[0] < '9')
			temp[0]++;
		else
			temp[0] = '1';

		if ( (namepart = strstr (Strings[i].string, temp)) ) {
			namepart += 2;
			while (*namepart && *namepart <= ' ')
				namepart++;
		} else {
			namepart = Strings[i].string;
		}

		ReplaceString (&LevelInfos[i-65].level_name, namepart);
	}

	for (i = 0; i < 4; i++)
		ReplaceString (&ClusterInfos[i].exittext, Strings[221+i].string);

	if (gamemission == pack_plut)
		start = 133;		// PHUSTR_1
	else if (gamemission == pack_tnt)
		start = 165;		// THUSTR_1
	else
		start = 101;		// HUSTR_1

	for (i = 0; i < 32; i++) {
		sprintf (temp, "%d:", i + 1);
		if ( (namepart = strstr (Strings[i+start].string, temp)) ) {
			namepart += strlen (temp);
			while (*namepart && *namepart <= ' ')
				namepart++;
		} else {
			namepart = Strings[i+start].string;
		}
		ReplaceString (&LevelInfos[36+i].level_name, namepart);
	}

	if (gamemission == pack_plut)
		start = 231;		// P1TEXT
	else if (gamemission == pack_tnt)
		start = 237;		// T1TEXT
	else
		start = 225;		// C1TEXT

	for (i = 0; i < 4; i++)
		ReplaceString (&ClusterInfos[4 + i].exittext, Strings[start+i].string);
	for (; i < 6; i++)
		ReplaceString (&ClusterInfos[4 + i].entertext, Strings[start+i].string);

	if (level.info)
		strncpy (level.level_name, level.info->level_name, 63);
}

// Static level info from original game
// The level names and cluster messages now get filled in
// by G_SetLevelStrings().

level_info_t LevelInfos[] = {
	// Registered/Retail Episode 1
	{
		"E1M1",
		1,
		NULL,
		"WILV00",
		"E1M2",
		"E1M9",
		30,
		"SKY1",
		"D_E1M1",
		0,
		1
	},
	{
		"E1M2",
		2,
		NULL,
		"WILV01",
		"E1M3",
		"E1M9",
		75,
		"SKY1",
		"D_E1M2",
		0,
		1
	},
	{
		"E1M3",
		3,
		NULL,
		"WILV02",
		"E1M4",
		"E1M9",
		120,
		"SKY1",
		"D_E1M3",
		0,
		1
	},
	{
		"E1M4",
		4,
		NULL,	
		"WILV03",
		"E1M5",
		"E1M9",
		90,
		"SKY1",
		"D_E1M4",
		0,
		1
	},
	{
		"E1M5",
		5,
		NULL,
		"WILV04",
		"E1M6",
		"E1M9",
		165,
		"SKY1",
		"D_E1M5",
		0,
		1
	},
	{
		"E1M6",
		6,
		NULL,
		"WILV05",
		"E1M7",
		"E1M9",
		180,
		"SKY1",
		"D_E1M6",
		0,
		1
	},
	{
		"E1M7",
		7,
		NULL,
		"WILV06",
		"E1M8",
		"E1M9",
		180,
		"SKY1",
		"D_E1M7",
		0,
		1
	},
	{
		"E1M8",
		8,
		NULL,
		"WILV07",
		"EndGame1",
		"E1M9",
		30,
		"SKY1",
		"D_E1M8",
		LEVEL_NOINTERMISSION|LEVEL_NOSOUNDCLIPPING|LEVEL_BRUISERSPECIAL|LEVEL_SPECLOWERFLOOR,
		1
	},
	{
		"E1M9",
		9,
		NULL,
		"WILV08",
		"E1M4",
		"E1M4",
		165,
		"SKY1",
		"D_E1M9",
		0,
		1
	},

	// Registered/Retail Episode 2
	{
		"E2M1",
		11,
		NULL,
		"WILV10",
		"E2M2",
		"E2M9",
		90,
		"SKY2",
		"D_E2M1",
		0,
		2
	},

	{
		"E2M2",
		12,
		NULL,
		"WILV11",
		"E2M3",
		"E2M9",
		90,
		"SKY2",
		"D_E2M2",
		0,
		2
	},
	{
		"E2M3",
		13,
		NULL,
		"WILV12",
		"E2M4",
		"E2M9",
		90,
		"SKY2",
		"D_E2M3",
		0,
		2
	},
	{
		"E2M4",
		14,
		NULL,
		"WILV13",
		"E2M5",
		"E2M9",
		120,
		"SKY2",
		"D_E2M4",
		0,
		2
	},
	{
		"E2M5",
		15,
		NULL,
		"WILV14",
		"E2M6",
		"E2M9",
		90,
		"SKY2",
		"D_E2M5",
		0,
		2
	},
	{
		"E2M6",
		16,
		NULL,
		"WILV15",
		"E2M7",
		"E2M9",
		360,
		"SKY2",
		"D_E2M6",
		0,
		2
	},
	{
		"E2M7",
		17,
		NULL,
		"WILV16",
		"E2M8",
		"E2M9",
		240,
		"SKY2",
		"D_E2M7",
		0,
		2
	},
	{
		"E2M8",
		18,
		NULL,
		"WILV17",
		"EndGame2",
		"E2M9",
		30,
		"SKY2",
		"D_E2M8",
		LEVEL_NOINTERMISSION|LEVEL_NOSOUNDCLIPPING|LEVEL_CYBORGSPECIAL,
		2
	},
	{
		"E2M9",
		19,
		NULL,
		"WILV18",
		"E2M6",
		"E2M6",
		170,
		"SKY2",
		"D_E2M9",
		0,
		2
	},

	// Registered/Retail Episode 3

	{
		"E3M1",
		21,
		NULL,
		"WILV20",
		"E3M2",
		"E3M9",
		90,
		"SKY3",
		"D_E3M1",
		0,
		3
	},
	{
		"E3M2",
		22,
		NULL,
		"WILV21",
		"E3M3",
		"E3M9",
		45,
		"SKY3",
		"D_E3M2",
		0,
		3
	},
	{
		"E3M3",
		23,
		NULL,
		"WILV22",
		"E3M4",
		"E3M9",
		90,
		"SKY3",
		"D_E3M3",
		0,
		3
	},
	{
		"E3M4",
		24,
		NULL,
		"WILV23",
		"E3M5",
		"E3M9",
		150,
		"SKY3",
		"D_E3M4",
		0,
		3
	},
	{
		"E3M5",
		25,
		NULL,
		"WILV24",
		"E3M6",
		"E3M9",
		90,
		"SKY3",
		"D_E3M5",
		0,
		3
	},
	{
		"E3M6",
		26,
		NULL,
		"WILV25",
		"E3M7",
		"E3M9",
		90,
		"SKY3",
		"D_E3M6",
		0,
		3
	},
	{
		"E3M7",
		27,
		NULL,
		"WILV26",
		"E3M8",
		"E3M9",
		165,
		"SKY3",
		"D_E3M7",
		0,
		3
	},
	{
		"E3M8",
		28,
		NULL,
		"WILV27",
		"EndGame3",
		"E3M9",
		30,
		"SKY3",
		"D_E3M8",
		LEVEL_NOINTERMISSION|LEVEL_NOSOUNDCLIPPING|LEVEL_SPIDERSPECIAL,
		3
	},
	{
		"E3M9",
		29,
		NULL,
		"WILV28",
		"E3M7",
		"E3M7",
		135,
		"SKY3",
		"D_E3M9",
		0,
		3
	},

	// Retail Episode 4
	{
		"E4M1",
		31,
		NULL,
		"WILV30",
		"E4M2",
		"E4M9",
		0,
		"SKY4",
		"D_E3M4",
		0,
		4
	},
	{
		"E4M2",
		32,
		NULL,
		"WILV31",
		"E4M3",
		"E4M9",
		0,
		"SKY4",
		"D_E3M2",
		0,
		4
	},
	{
		"E4M3",
		33,
		NULL,
		"WILV32",
		"E4M4",
		"E4M9",
		0,
		"SKY4",
		"D_E3M3",
		0,
		4
	},
	{
		"E4M4",
		34,
		NULL,
		"WILV33",
		"E4M5",
		"E4M9",
		0,
		"SKY4",
		"D_E1M5",
		0,
		4
	},
	{
		"E4M5",
		35,
		NULL,
		"WILV34",
		"E4M6",
		"E4M9",
		0,
		"SKY4",
		"D_E2M7",
		0,
		4
	},
	{
		"E4M6",
		36,
		NULL,
		"WILV35",
		"E4M7",
		"E4M9",
		0,
		"SKY4",
		"D_E2M4",
		LEVEL_CYBORGSPECIAL|LEVEL_SPECOPENDOOR,
		4
	},
	{
		"E4M7",
		37,
		NULL,
		"WILV36",
		"E4M8",
		"E4M9",
		0,
		"SKY4",
		"D_E2M6",
		0,
		4
	},
	{
		"E4M8",
		38,
		NULL,
		"WILV37",
		"EndGame4",
		"E4M9",
		0,
		"SKY4",
		"D_E2M5",
		LEVEL_NOINTERMISSION|LEVEL_NOSOUNDCLIPPING|LEVEL_SPIDERSPECIAL|LEVEL_SPECLOWERFLOOR,
		4
	},
	{
		"E4M9",
		39,
		NULL,
		"WILV38",
		"E4M3",
		"E4M3",
		0,
		"SKY4",
		"D_E1M9",
		0,
		4
	},

	// DOOM 2 Levels

	{
		"MAP01",
		1,
		NULL,
		"CWILV00",
		"MAP02",
		"MAP02",
		30,
		"SKY1",
		"D_RUNNIN",
		0,
		5
	},
	{
		"MAP02",
		2,
		NULL,
		"CWILV01",
		"MAP03",
		"MAP03",
		90,
		"SKY1",
		"D_STALKS",
		0,
		5
	},
	{
		"MAP03",
		3,
		NULL,
		"CWILV02",
		"MAP04",
		"MAP04",
		120,
		"SKY1",
		"D_COUNTD",
		0,
		5
	},
	{
		"MAP04",
		4,
		NULL,
		"CWILV03",
		"MAP05",
		"MAP05",
		120,
		"SKY1",
		"D_BETWEE",
		0,
		5
	},
	{
		"MAP05",
		5,
		NULL,
		"CWILV04",
		"MAP06",
		"MAP06",
		90,
		"SKY1",
		"D_DOOM",
		0,
		5
	},
	{
		"MAP06",
		6,
		NULL,
		"CWILV05",
		"MAP07",
		"MAP07",
		150,
		"SKY1",
		"D_THE_DA",
		0,
		5
	},
	{
		"MAP07",
		7,
		NULL,
		"CWILV06",
		"MAP08",
		"MAP08",
		120,
		"SKY1",
		"D_SHAWN",
		LEVEL_MAP07SPECIAL,
		6
	},
	{
		"MAP08",
		8,
		NULL,
		"CWILV07",
		"MAP09",
		"MAP09",
		120,
		"SKY1",
		"D_DDTBLU",
		0,
		6
	},
	{
		"MAP09",
		9,
		NULL,
		"CWILV08",
		"MAP10",
		"MAP10",
		270,
		"SKY1",
		"D_IN_CIT",
		0,
		6
	},
	{
		"MAP10",
		10,
		NULL,
		"CWILV09",
		"MAP11",
		"MAP11",
		90,
		"SKY1",
		"D_DEAD",
		0,
		6
	},
	{
		"MAP11",
		11,
		NULL,
		"CWILV10",
		"MAP12",
		"MAP12",
		210,
		"SKY1",
		"D_STLKS2",
		0,
		6
	},
	{
		"MAP12",
		12,
		NULL,
		"CWILV11",
		"MAP13",
		"MAP13",
		150,
		"SKY2",
		"D_THEDA2",
		0,
		7
	},
	{
		"MAP13",
		13,
		NULL,
		"CWILV12",
		"MAP14",
		"MAP14",
		150,
		"SKY2",
		"D_DOOM2",
		0,
		7
	},
	{
		"MAP14",
		14,
		NULL,
		"CWILV13",
		"MAP15",
		"MAP15",
		150,
		"SKY2",
		"D_DDTBL2",
		0,
		7
	},
	{
		"MAP15",
		15,
		NULL,
		"CWILV14",
		"MAP16",
		"MAP31",
		210,
		"SKY2",
		"D_RUNNI2",
		0,
		7
	},
	{
		"MAP16",
		16,
		NULL,
		"CWILV15",
		"MAP17",
		"MAP17",
		150,
		"SKY2",
		"D_DEAD2",
		0,
		7
	},
	{
		"MAP17",
		17,
		NULL,
		"CWILV16",
		"MAP18",
		"MAP18",
		420,
		"SKY2",
		"D_STLKS3",
		0,
		7
	},
	{
		"MAP18",
		18,
		NULL,
		"CWILV17",
		"MAP19",
		"MAP19",
		150,
		"SKY2",
		"D_ROMERO",
		0,
		7
	},
	{
		"MAP19",
		19,
		NULL,
		"CWILV18",
		"MAP20",
		"MAP20",
		210,
		"SKY2",
		"D_SHAWN2",
		0,
		7
	},
	{
		"MAP20",
		20,
		NULL,
		"CWILV19",
		"MAP21",
		"MAP21",
		150,
		"SKY2",
		"D_MESSAG",
		0,
		7
	},
	{
		"MAP21",
		21,
		NULL,
		"CWILV20",
		"MAP22",
		"MAP22",
		240,
		"SKY3",
		"D_COUNT2",
		0,
		8
	},
	{
		"MAP22",
		22,
		NULL,
		"CWILV21",
		"MAP23",
		"MAP23",
		150,
		"SKY3",
		"D_DDTBL3",
		0,
		8
	},
	{
		"MAP23",
		23,
		NULL,
		"CWILV22",
		"MAP24",
		"MAP24",
		180,
		"SKY3",
		"D_AMPIE",
		0,
		8
	},
	{
		"MAP24",
		24,
		NULL,
		"CWILV23",
		"MAP25",
		"MAP25",
		150,
		"SKY3",
		"D_THEDA3",
		0,
		8
	},
	{
		"MAP25",
		25,
		NULL,
		"CWILV24",
		"MAP26",
		"MAP26",
		150,
		"SKY3",
		"D_ADRIAN",
		0,
		8
	},
	{
		"MAP26",
		26,
		NULL,
		"CWILV25",
		"MAP27",
		"MAP27",
		300,
		"SKY3",
		"D_MESSG2",
		0,
		8
	},
	{
		"MAP27",
		27,
		NULL,
		"CWILV26",
		"MAP28",
		"MAP28",
		330,
		"SKY3",
		"D_ROMER2",
		0,
		8
	},
	{
		"MAP28",
		28,
		NULL,
		"CWILV27",
		"MAP29",
		"MAP29",
		420,
		"SKY3",
		"D_TENSE",
		0,
		8
	},
	{
		"MAP29",
		29,
		NULL,
		"CWILV28",
		"MAP30",
		"MAP30",
		300,
		"SKY3",
		"D_SHAWN3",
		0,
		8
	},
	{
		"MAP30",
		30,
		NULL,
		"CWILV29",
		"EndGameC",
		"EndGameC",
		180,
		"SKY3",
		"D_OPENIN",
		LEVEL_MONSTERSTELEFRAG,
		8
	},
	{
		"MAP31",
		31,
		NULL,
		"CWILV30",
		"MAP16",
		"MAP32",
		120,
		"SKY3",
		"D_EVIL",
		0,
		9
	},
	{
		"MAP32",
		32,
		NULL,
		"CWILV31",
		"MAP16",
		"MAP16",
		30,
		"SKY3",
		"D_ULTIMA",
		0,
		10
	},
	{
		"",
		0,
		NULL,
	}
};


// Episode/Cluster information
cluster_info_t ClusterInfos[] = {
	{
		1,		// DOOM Episode 1
		"D_VICTOR",
		"FLOOR4_8",
		NULL,
		NULL
	},
	{
		2,		// DOOM Episode 2
		"D_VICTOR",
		"SFLR6_1",
		NULL,
		NULL
	},
	{
		3,		// DOOM Episode 3
		"D_VICTOR",
		"MFLR8_4",
		NULL,
		NULL
	},
	{
		4,		// DOOM Episode 4
		"D_VICTOR",
		"MFLR8_3",
		NULL,
		NULL
	},
	{
		5,		// DOOM II first cluster (up thru level 6)
		"D_READ_M",
		"SLIME16",
		NULL,
		NULL
	},
	{
		6,		// DOOM II second cluster (up thru level 11)
		"D_READ_M",
		"RROCK14",
		NULL,
		NULL
	},
	{
		7,		// DOOM II third cluster (up thru level 20)
		"D_READ_M",
		"RROCK07",
		NULL,
		NULL
	},
	{
		8,		// DOOM II fourth cluster (up thru level 30)
		"D_READ_M",
		"RROCK17",
		NULL,
		NULL
	},
	{
		9,		// DOOM II fifth cluster (level 31)
		"D_READ_M",
		"RROCK13",
		NULL,
		NULL
	},
	{
		10,		// DOOM II sixth cluster (level 32)
		"D_READ_M",
		"RROCK19",
		NULL,
		NULL
	},
	{
		0		// End-of-clusters marker
	}
};






