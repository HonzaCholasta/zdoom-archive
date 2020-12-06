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
//		DOOM selection menu, options, episode etc.
//		Sliders and icons. Kinda widget stuff.
//
//-----------------------------------------------------------------------------

// HEADER FILES ------------------------------------------------------------

#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "doomdef.h"
#include "dstrings.h"
#include "c_console.h"
#include "c_dispatch.h"
#include "d_main.h"
#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"
#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "m_argv.h"
#include "m_swap.h"
#include "m_random.h"
#include "s_sound.h"
#include "doomstat.h"
#include "m_menu.h"
#include "v_text.h"
#include "st_stuff.h"

#include "gi.h"

// MACROS ------------------------------------------------------------------

#define SAVESTRINGSIZE		24
#define SKULLXOFF			-32
#define SELECTOR_XOFFSET	(-28)
#define SELECTOR_YOFFSET	(-1)

#define NUM_MENU_ITEMS(m)	(sizeof(m)/sizeof(m[0]))

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void M_DrawSlider (int x, int y, float min, float max, float cur);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

int  M_StringHeight (const char *string);
void M_ClearMenus ();

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void M_NewGame (int choice);
static void M_Episode (int choice);
static void M_ChooseSkill (int choice);
static void M_LoadGame (int choice);
static void M_SaveGame (int choice);
static void M_Options (int choice);
static void M_EndGame (int choice);
static void M_ReadThis (int choice);
static void M_ReadThisMore (int choice);
static void M_QuitDOOM (int choice);
static void M_GameFiles (int choice);

static void M_FinishReadThis (int choice);
static void M_LoadSelect (int choice);
static void M_SaveSelect (int choice);
static void M_ReadSaveStrings ();
static void M_QuickSave ();
static void M_QuickLoad ();

static void M_DrawMainMenu ();
static void M_DrawReadThis ();
static void M_DrawNewGame ();
static void M_DrawEpisode ();
static void M_DrawLoad ();
static void M_DrawSave ();

static void M_DrawHereticMainMenu ();
static void M_DrawFiles ();

static void M_DrawSaveLoadBorder (int x,int y, int len);
static void M_SetupNextMenu (oldmenu_t *menudef);
static void M_StartMessage (char *string, void(*routine)(int), bool input);

// [RH] For player setup menu.
static void M_PlayerSetup (int choice);
static void M_PlayerSetupTicker ();
static void M_PlayerSetupDrawer ();
static void M_EditPlayerName (int choice);
static void M_EditPlayerTeam (int choice);
static void M_PlayerTeamChanged (int choice);
static void M_PlayerNameChanged (int choice);
static void M_SlidePlayerRed (int choice);
static void M_SlidePlayerGreen (int choice);
static void M_SlidePlayerBlue (int choice);
static void M_ChangeGender (int choice);
static void M_ChangeSkin (int choice);
static void M_ChangeAutoAim (int choice);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

EXTERN_CVAR (name)
EXTERN_CVAR (team)

extern bool		sendpause;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

bool			menuactive;
menustack_t		MenuStack[16];
int				MenuStackDepth;
int				skullAnimCounter;	// skull animation counter
bool			drawSkull;			// [RH] don't always draw skull
bool			M_DemoNoPlay;
bool			OptionsActive;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char		tempstring[80];
static char		underscore[2];

static int 		quickSaveSlot;			// -1 = no quicksave slot picked!
static int 		messageToPrint;			// 1 = message to be printed
static char*	messageString;			// ...and here is the message string!
static bool		messageLastMenuActive;
static bool		messageNeedsInput;		// timed message = no input from user
static void	  (*messageRoutine)(int response);

static int 		genStringEnter;	// we are going to be entering a savegame string
static int		genStringLen;	// [RH] Max # of chars that can be entered
static void	  (*genStringEnd)(int slot);
static int 		saveSlot;		// which slot to save in
static int 		saveCharIndex;	// which char we're editing
static char		saveOldString[SAVESTRINGSIZE];	// old save description before edit

static int		LINEHEIGHT;

static char		savegamestrings[10][SAVESTRINGSIZE];
static char		endstring[160];

static short	itemOn; 			// menu item skull is on
static short	whichSkull; 		// which skull to draw
static int		SkullBaseLump;		// Heretic's spinning skull
static int		MenuTime;
static int		InfoType;

static const char skullName[2][9] = {"M_SKULL1", "M_SKULL2"};	// graphic name of skulls

static oldmenu_t *currentMenu;		// current menudef
static oldmenu_t *TopLevelMenu;		// The main menu everything hangs off of

static DCanvas	*FireScreen;
static byte		FireRemap[256];

static char		*genders[3] = { "male", "female", "cyborg" };
static const TypeInfo *PlayerClass;
static FState	*PlayerState;
static int		PlayerTics;
static int		PlayerRotation;

// PRIVATE MENU DEFINITIONS ------------------------------------------------

//
// DOOM MENU
//
static oldmenuitem_t MainMenu[]=
{
	{1,0,'n',"M_NGAME",M_NewGame},
	{1,0,'l',"M_LOADG",M_LoadGame},
	{1,0,'s',"M_SAVEG",M_SaveGame},
	{1,0,'o',"M_OPTION",M_Options},		// [RH] Moved
	{1,0,'p',"M_PSETUP",M_PlayerSetup},	// [RH] Player setup
	{1,0,'r',"M_RDTHIS",M_ReadThis},	// Another hickup with Special edition.
	{1,0,'q',"M_QUITG",M_QuitDOOM}
};

static oldmenu_t MainDef =
{
	NUM_MENU_ITEMS (MainMenu),
	MainMenu,
	M_DrawMainMenu,
	97,64,
	0
};

//
// HERETIC MENU
//
static oldmenuitem_t HereticMainMenu[] =
{
	{1,1,'n',"NEW GAME",M_NewGame},
	{1,1,'o',"OPTIONS",M_Options},
	{1,1,'f',"GAME FILES",M_GameFiles},
	{1,1,'p',"PLAYER SETUP",M_PlayerSetup},
	{1,1,'i',"INFO",M_ReadThis},
	{1,1,'q',"QUIT GAME",M_QuitDOOM}
};

static oldmenu_t HereticMainDef =
{
	NUM_MENU_ITEMS (HereticMainMenu),
	HereticMainMenu,
	M_DrawHereticMainMenu,
	110, 56,
	0
};

//
// DOOM EPISODE SELECT
//
static oldmenuitem_t EpisodeMenu[]=
{
	{1,0,'k',"M_EPI1", M_Episode},
	{1,0,'t',"M_EPI2", M_Episode},
	{1,0,'i',"M_EPI3", M_Episode},
	{1,0,'t',"M_EPI4", M_Episode}
};

static oldmenu_t EpiDef =
{
	3,
	EpisodeMenu,		// oldmenuitem_t ->
	M_DrawEpisode,		// drawing routine ->
	48,63,				// x,y
	0	 				// lastOn
};

//
// HERETIC EPISODE SELECT
//
static oldmenuitem_t HereticEpisodeMenu[] =
{
	{1,1,'c',"CITY OF THE DAMNED",M_Episode},
	{1,1,'h',"HELL'S MAW",M_Episode},
	{1,1,'d',"THE DOME OF D'SPARIL",M_Episode},
	{1,1,'o',"THE OSSUARY",M_Episode},
	{1,1,'s',"THE STAGNANT DEMESNE",M_Episode}
};

static oldmenu_t HereticEpiDef =
{
	3,
	HereticEpisodeMenu,
	M_DrawEpisode,
	80,50,
	0
};

//
// GAME FILES
//
static oldmenuitem_t FilesItems[] =
{
	{1,1,'l',"LOAD GAME",M_LoadGame},
	{1,1,'s',"SAVE GAME",M_SaveGame}
};

static oldmenu_t FilesMenu =
{
	NUM_MENU_ITEMS (FilesItems),
	FilesItems,
	M_DrawFiles,
	110,60,
	0
};

//
// DOOM SKILL SELECT
//
static oldmenuitem_t NewGameMenu[]=
{
	{1,0,'i',"M_JKILL",M_ChooseSkill},
	{1,0,'h',"M_ROUGH",M_ChooseSkill},
	{1,0,'h',"M_HURT",M_ChooseSkill},
	{1,0,'u',"M_ULTRA",M_ChooseSkill},
	{1,0,'n',"M_NMARE",M_ChooseSkill}
};

static oldmenu_t NewDef =
{
	NUM_MENU_ITEMS (NewGameMenu),
	NewGameMenu,		// oldmenuitem_t ->
	M_DrawNewGame,		// drawing routine ->
	48,63,				// x,y
	2					// lastOn
};

//
// HERETIC SKILL SELECT
//
static oldmenuitem_t HereticSkillItems[] =
{
	{1,1,'t',"THOU NEEDETH A WET-NURSE",M_ChooseSkill},
	{1,1,'y',"YELLOWBELLIES-R-US",M_ChooseSkill},
	{1,1,'b',"BRINGEST THEM ONETH",M_ChooseSkill},
	{1,1,'t',"THOU ART A SMITE-MEISTER",M_ChooseSkill},
	{1,1,'b',"BLACK PLAGUE POSSESSES THEE",M_ChooseSkill}
};

static oldmenu_t HereticSkillMenu =
{
	NUM_MENU_ITEMS (HereticSkillItems),
	HereticSkillItems,
	M_DrawNewGame,
	38, 30,
	2
};

//
// [RH] Player Setup Menu
//
static oldmenuitem_t PlayerSetupMenu[] =
{
	{ 1,0,'n',NULL,M_EditPlayerName},
	{ 1,0,'t',NULL,M_EditPlayerTeam},
	{ 2,0,'r',NULL,M_SlidePlayerRed},
	{ 2,0,'g',NULL,M_SlidePlayerGreen},
	{ 2,0,'b',NULL,M_SlidePlayerBlue},
	{ 2,0,'e',NULL,M_ChangeGender},
	{ 2,0,'s',NULL,M_ChangeSkin},
	{ 2,0,'a',NULL,M_ChangeAutoAim}
};

static oldmenu_t PSetupDef =
{
	NUM_MENU_ITEMS (PlayerSetupMenu),
	PlayerSetupMenu,
	M_PlayerSetupDrawer,
	48,	47,
	0
};

//
// Read This! MENU 1 & 2
//
static oldmenuitem_t ReadMenu[] =
{
	{1,0,0,NULL,M_ReadThisMore}
};

static oldmenu_t ReadDef =
{
	1,
	ReadMenu,
	M_DrawReadThis,
	280,185,
	0
};

//
// LOAD GAME MENU
//
static oldmenuitem_t LoadMenu[]=
{
	{1,0,'1',NULL, M_LoadSelect},
	{1,0,'2',NULL, M_LoadSelect},
	{1,0,'3',NULL, M_LoadSelect},
	{1,0,'4',NULL, M_LoadSelect},
	{1,0,'5',NULL, M_LoadSelect},
	{1,0,'6',NULL, M_LoadSelect},
	{1,0,'7',NULL, M_LoadSelect},
	{1,0,'8',NULL, M_LoadSelect},
};

static oldmenu_t LoadDef =
{
	NUM_MENU_ITEMS(LoadMenu),
	LoadMenu,
	M_DrawLoad,
	80,54,
	0
};

//
// SAVE GAME MENU
//
static oldmenuitem_t SaveMenu[] =
{
	{1,0,'1',NULL, M_SaveSelect},
	{1,0,'2',NULL, M_SaveSelect},
	{1,0,'3',NULL, M_SaveSelect},
	{1,0,'4',NULL, M_SaveSelect},
	{1,0,'5',NULL, M_SaveSelect},
	{1,0,'6',NULL, M_SaveSelect},
	{1,0,'7',NULL, M_SaveSelect},
	{1,0,'8',NULL, M_SaveSelect},
};

static oldmenu_t SaveDef =
{
	NUM_MENU_ITEMS(LoadMenu),
	SaveMenu,
	M_DrawSave,
	80,54,
	0
};

// CODE --------------------------------------------------------------------

// [RH] Most menus can now be accessed directly
// through console commands.
BEGIN_COMMAND (menu_main)
{
	M_StartControlPanel (true);
	M_SetupNextMenu (TopLevelMenu);
}
END_COMMAND (menu_main)

BEGIN_COMMAND (menu_load)
{	// F3
	M_StartControlPanel (true);
	M_LoadGame (0);
}
END_COMMAND (menu_load)

BEGIN_COMMAND (menu_save)
{	// F2
	M_StartControlPanel (true);
	M_SaveGame (0);
}
END_COMMAND (menu_save)

BEGIN_COMMAND (menu_help)
{	// F1
	M_StartControlPanel (true);
	M_ReadThis (0);
}
END_COMMAND (menu_help)

BEGIN_COMMAND (quicksave)
{	// F6
	M_StartControlPanel (true);
	M_QuickSave();
}
END_COMMAND (quicksave)

BEGIN_COMMAND (quickload)
{	// F9
	M_StartControlPanel (true);
	M_QuickLoad();
}
END_COMMAND (quickload)

BEGIN_COMMAND (menu_endgame)
{	// F7
	M_StartControlPanel (true);
	M_EndGame(0);
}
END_COMMAND (menu_endgame)

BEGIN_COMMAND (menu_quit)
{	// F10
	M_StartControlPanel (true);
	M_QuitDOOM(0);
}
END_COMMAND (menu_quit)

BEGIN_COMMAND (menu_game)
{
	M_StartControlPanel (true);
	M_NewGame(0);
}
END_COMMAND (menu_game)
								
BEGIN_COMMAND (menu_options)
{
	M_StartControlPanel (true);
	M_Options(0);
}
END_COMMAND (menu_options)

BEGIN_COMMAND (menu_player)
{
	M_StartControlPanel (true);
	M_PlayerSetup(0);
}
END_COMMAND (menu_player)

BEGIN_COMMAND (bumpgamma)
{
	// [RH] Gamma correction tables are now generated
	// on the fly for *any* gamma level.
	// Q: What are reasonable limits to use here?

	float newgamma = Gamma.value + 0.1;

	if (newgamma > 3.0)
		newgamma = 1.0;

	Gamma.Set (newgamma);
	Printf (PRINT_HIGH, "Gamma correction level %g\n", Gamma.value);
}
END_COMMAND (bumpgamma)

void M_DrawFiles ()
{
}

void M_GameFiles (int choice)
{
	M_SetupNextMenu (&FilesMenu);
}

//
// M_ReadSaveStrings
//	read the strings from the savegame files
//
void M_ReadSaveStrings ()
{
	FILE *handle;
	int count;
	int i;
	char name[256];
		
	for (i = 0; i < NUM_MENU_ITEMS(LoadMenu); i++)
	{
		G_BuildSaveName (name, i);

		handle = fopen (name, "rb");
		if (handle == NULL)
		{
			strcpy (&savegamestrings[i][0], EMPTYSTRING);
			LoadMenu[i].status = 0;
		}
		else
		{
			count = fread (&savegamestrings[i], SAVESTRINGSIZE, 1, handle);
			fclose (handle);
			LoadMenu[i].status = 1;
		}
	}
}


//
// M_LoadGame & Cie.
//
void M_DrawLoad (void)
{
	int i;
	int x, y;

	if (gameinfo.gametype == GAME_Doom)
	{
		screen->DrawPatchClean ((patch_t *)W_CacheLumpName ("M_LOADG",PU_CACHE), 72, 28);
	}
	else
	{
		screen->DrawTextCleanMove (CR_UNTRANSLATED,
			160 - screen->StringWidth ("LOAD GAME")/2, 10,
			"LOAD GAME");
	}
	screen->SetFont (SmallFont);
	x = currentMenu->x;
	y = currentMenu->y;
	if (gameinfo.gametype != GAME_Doom)
	{
		x += 5;
		y += 5;
	}
	for (i = 0; i < NUM_MENU_ITEMS(LoadMenu); i++)
	{
		M_DrawSaveLoadBorder (x, currentMenu->y+LINEHEIGHT*i, 24);
		screen->DrawTextCleanMove (CR_UNTRANSLATED, x,
			y + LINEHEIGHT*i, savegamestrings[i]);
	}
}



//
// Draw border for the savegame description
// [RH] Width of the border is variable
//
void M_DrawSaveLoadBorder (int x, int y, int len)
{
	if (gameinfo.gametype == GAME_Doom)
	{
		int i;

		screen->DrawPatchClean ((patch_t *)W_CacheLumpName ("M_LSLEFT",PU_CACHE), x-8, y+7);
			
		for (i = 0; i < len; i++)
		{
			screen->DrawPatchClean ((patch_t *)W_CacheLumpName ("M_LSCNTR",PU_CACHE), x, y+7);
			x += 8;
		}

		screen->DrawPatchClean ((patch_t *)W_CacheLumpName ("M_LSRGHT",PU_CACHE), x, y+7);
	}
	else
	{
		screen->DrawPatchClean ((patch_t *)W_CacheLumpName ("M_FSLOT",PU_CACHE), x, y+1);
	}
}



//
// User wants to load this game
//
void M_LoadSelect (int choice)
{
	char name[256];

	G_BuildSaveName (name, choice);
	G_LoadGame (name);
	gamestate = gamestate == GS_FULLCONSOLE ? GS_HIDECONSOLE : gamestate;
	M_ClearMenus ();
	BorderNeedRefresh = true;
	if (quickSaveSlot == -2)
	{
		quickSaveSlot = choice;
	}
}

//
// Selected from DOOM menu
//
void M_LoadGame (int choice)
{
	if (netgame)
	{
		M_StartMessage (LOADNET,NULL,false);
		return;
	}
		
	M_SetupNextMenu (&LoadDef);
	M_ReadSaveStrings ();
}


//
//	M_SaveGame & Cie.
//
void M_DrawSave(void)
{
	int i;
	int x, y;

	if (gameinfo.gametype == GAME_Doom)
	{
		screen->DrawPatchClean ((patch_t *)W_CacheLumpName("M_SAVEG",PU_CACHE), 72, 28);
	}
	else
	{
		screen->DrawTextCleanMove (CR_UNTRANSLATED,
			160 - screen->StringWidth ("SAVE GAME")/2, 10,
			"SAVE GAME");
	}
	screen->SetFont (SmallFont);
	x = currentMenu->x;
	y = currentMenu->y;
	if (gameinfo.gametype != GAME_Doom)
	{
		x += 5;
		y += 5;
	}
	for (i = 0; i < NUM_MENU_ITEMS(LoadMenu); i++)
	{
		M_DrawSaveLoadBorder (x, currentMenu->y+LINEHEIGHT*i,24);
		screen->DrawTextCleanMove (CR_UNTRANSLATED, x,
			y+LINEHEIGHT*i, savegamestrings[i]);
	}
		
	if (genStringEnter)
	{
		i = screen->StringWidth (savegamestrings[saveSlot]);
		screen->DrawTextCleanMove (CR_UNTRANSLATED,
			x + i, y+LINEHEIGHT*saveSlot, underscore);
	}
}

//
// M_Responder calls this when user is finished
//
void M_DoSave (int slot)
{
	G_SaveGame (slot,savegamestrings[slot]);
	M_ClearMenus ();
	BorderNeedRefresh = true;
	// PICK QUICKSAVE SLOT YET?
	if (quickSaveSlot == -2)
		quickSaveSlot = slot;
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect (int choice)
{
	// we are going to be intercepting all chars
	genStringEnter = 1;
	genStringEnd = M_DoSave;
	genStringLen = SAVESTRINGSIZE-1;
	
	saveSlot = choice;
	strcpy(saveOldString,savegamestrings[choice]);
	if (!strcmp(savegamestrings[choice],EMPTYSTRING))
		savegamestrings[choice][0] = 0;
	saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
	if (!usergame || (players[consoleplayer].health <= 0 && !multiplayer))
	{
		M_StartMessage(SAVEDEAD,NULL,false);
		return;
	}
		
	if (gamestate != GS_LEVEL)
		return;
		
	M_SetupNextMenu(&SaveDef);
	M_ReadSaveStrings();
}



//
//		M_QuickSave
//
void M_QuickSaveResponse (int ch)
{
	if (ch == 'y')
	{
		M_DoSave (quickSaveSlot);
		S_Sound (CHAN_VOICE, "menu/dismiss", 1, ATTN_NONE);
	}
}

void M_QuickSave ()
{
	if (!usergame || (players[consoleplayer].health <= 0 && !multiplayer))
	{
		S_Sound (CHAN_VOICE, "menu/invalid", 1, ATTN_NONE);
		return;
	}

	if (gamestate != GS_LEVEL)
		return;
		
	if (quickSaveSlot < 0)
	{
		M_StartControlPanel(false);
		M_ReadSaveStrings();
		M_SetupNextMenu(&SaveDef);
		quickSaveSlot = -2; 	// means to pick a slot now
		return;
	}
	sprintf (tempstring, QSPROMPT, savegamestrings[quickSaveSlot]);
	M_StartMessage (tempstring, M_QuickSaveResponse, true);
}



//
// M_QuickLoad
//
void M_QuickLoadResponse (int ch)
{
	if (ch == 'y')
	{
		M_LoadSelect(quickSaveSlot);
		S_Sound (CHAN_VOICE, "menu/dismiss", 1, ATTN_NONE);
	}
}


void M_QuickLoad ()
{
	if (netgame)
	{
		M_StartMessage(QLOADNET,NULL,false);
		return;
	}
		
	if (quickSaveSlot < 0)
	{
		M_StartControlPanel(false);
		M_LoadGame (0);
		return;
	}
	sprintf(tempstring,QLPROMPT,savegamestrings[quickSaveSlot]);
	M_StartMessage(tempstring,M_QuickLoadResponse,true);
}

//
// Read This Menus
//
void M_DrawReadThis ()
{
	if (gameinfo.flags & GI_PAGESARERAW)
	{
		screen->DrawPageBlock ((byte *)W_CacheLumpNum (
			W_GetNumForName (gameinfo.info.indexed.basePage) +
			InfoType, PU_CACHE));
	}
	else
	{
		screen->DrawPatchIndirect ((patch_t *)W_CacheLumpName (
			gameinfo.info.infoPage[InfoType-1], PU_CACHE), 0, 0);
	}
}

//
// M_DrawMainMenu
//
void M_DrawMainMenu (void)
{
	screen->DrawPatchClean ((patch_t *)W_CacheLumpName("M_DOOM",PU_CACHE), 94, 2);
}

void M_DrawHereticMainMenu ()
{
	int frame = (MenuTime / 3) % 18;
	screen->DrawPatchClean ((patch_t *)W_CacheLumpName("M_HTIC",PU_CACHE), 88, 0);
	screen->DrawPatchClean ((patch_t *)W_CacheLumpNum(SkullBaseLump+(17-frame),
		PU_CACHE), 40, 10);
	screen->DrawPatchClean ((patch_t *)W_CacheLumpNum(SkullBaseLump+frame,
		PU_CACHE), 232, 10);
}

//
// M_NewGame
//
void M_DrawNewGame(void)
{
	if (gameinfo.gametype == GAME_Doom)
	{
		screen->DrawPatchClean ((patch_t *)W_CacheLumpName("M_NEWG",PU_CACHE), 96, 14);
		screen->DrawPatchClean ((patch_t *)W_CacheLumpName("M_SKILL",PU_CACHE), 54, 38);
	}
}

void M_NewGame(int choice)
{
	if (netgame && !demoplayback)
	{
		M_StartMessage(NEWGAME,NULL,false);
		return;
	}
		
	if (gameinfo.flags & GI_MAPxx)
		M_SetupNextMenu (&NewDef);
	else if (gameinfo.gametype == GAME_Doom)
		M_SetupNextMenu (&EpiDef);
	else
		M_SetupNextMenu (&HereticEpiDef);
}


//
//		M_Episode
//
int 	epi;

void M_DrawEpisode(void)
{
	if (gameinfo.gametype == GAME_Doom)
	{
		screen->DrawPatchClean ((patch_t *)W_CacheLumpName (
			"M_EPISOD", PU_CACHE), 54, 38);
	}
}

void M_VerifyNightmare(int ch)
{
	if (ch != 'y')
		return;
		
	gameskill.Set (4.f);
	G_DeferedInitNew (CalcMapName (epi+1, 1));
	gamestate = gamestate == GS_FULLCONSOLE ? GS_HIDECONSOLE : gamestate;
	M_ClearMenus ();
}

void M_ChooseSkill(int choice)
{
	if (gameinfo.gametype == GAME_Doom && choice == NewDef.numitems - 1)
	{
		M_StartMessage (NIGHTMARE, M_VerifyNightmare, true);
		return;
	}

	gameskill.Set ((float)choice);
	gamestate = gamestate == GS_FULLCONSOLE ? GS_HIDECONSOLE : gamestate;
	G_DeferedInitNew (CalcMapName (epi+1, 1));
	gamestate = gamestate == GS_FULLCONSOLE ? GS_HIDECONSOLE : gamestate;
	M_ClearMenus ();
}

void M_Episode (int choice)
{
	if ((gameinfo.flags & GI_SHAREWARE) && choice)
	{
		M_StartMessage(SWSTRING,NULL,false);
		M_SetupNextMenu(&ReadDef);
		return;
	}

	epi = choice;
	if (gameinfo.gametype == GAME_Doom)
		M_SetupNextMenu (&NewDef);
	else
		M_SetupNextMenu (&HereticSkillMenu);
}



void M_Options(int choice)
{
	OptionsActive = M_StartOptionsMenu();
}




//
// M_EndGame
//
void M_EndGameResponse(int ch)
{
	if (ch != 'y')
		return;
				
	currentMenu->lastOn = itemOn;
	M_ClearMenus ();
	D_StartTitle ();
}

void M_EndGame(int choice)
{
	choice = 0;
	if (!usergame)
	{
		S_Sound (CHAN_VOICE, "menu/invalid", 1, ATTN_NONE);
		return;
	}
		
	if (netgame)
	{
		M_StartMessage(NETEND,NULL,false);
		return;
	}
		
	M_StartMessage(ENDGAME,M_EndGameResponse,true);
}




//
// M_ReadThis
//
void M_ReadThis (int choice)
{
	drawSkull = false;
	InfoType = 1;
	M_SetupNextMenu (&ReadDef);
}

void M_ReadThisMore (int choice)
{
	InfoType++;
	if (gameinfo.flags & GI_PAGESARERAW)
	{
		if (InfoType >= gameinfo.info.indexed.numPages)
		{
			M_FinishReadThis (0);
		}
	}
	else if (InfoType > 2)
	{
		M_FinishReadThis (0);
	}
}

void M_FinishReadThis (int choice)
{
	drawSkull = true;
	M_PopMenuStack ();
}

//
// M_QuitDOOM
//

void M_QuitResponse(int ch)
{
	if (ch != 'y')
		return;
	if (!netgame)
	{
		if (gameinfo.quitSounds)
		{
			S_Sound (CHAN_VOICE, gameinfo.quitSounds[(gametic>>2)&7],
				1, ATTN_SURROUND);
			I_WaitVBL (105);
		}
	}
	exit (0);
}

void M_QuitDOOM (int choice)
{
	// We pick index 0 which is language sensitive,
	//  or one at random, between 1 and maximum number.
	if (gameinfo.gametype == GAME_Doom)
	{
		if (language != english )
			sprintf (endstring,"%s\n\n%s", endmsg[0], DOSY);
		else
			sprintf (endstring,"%s\n\n%s", endmsg[(gametic%(NUM_QUITMESSAGES-2))+1], DOSY);
	}
	else
	{
		strcpy (endstring, "ARE YOU SURE YOU WANT TO QUIT?");
	}

	M_StartMessage (endstring, M_QuitResponse, true);
}


//
// [RH] Player Setup Menu code
//
static void M_PlayerSetup (int choice)
{
	choice = 0;
	strcpy (savegamestrings[0], name.string);
	strcpy (savegamestrings[1], team.string);
	M_DemoNoPlay = true;
	if (demoplayback)
		G_CheckDemoStatus ();
	M_SetupNextMenu (&PSetupDef);
	PlayerState = PlayerClass->ActorInfo->seestate;
	PlayerTics = PlayerState->tics;
	if (FireScreen == NULL)
		FireScreen = new DCanvas (72, 72+5, 8);
}

static void M_PlayerSetupTicker (void)
{
	// Based on code in f_finale.c
	if (--PlayerTics > 0)
		return;

	if (PlayerState->tics == -1 || PlayerState->nextstate == NULL)
	{
		PlayerState = PlayerClass->ActorInfo->seestate;
	}
	else
	{
		PlayerState = PlayerState->nextstate;
	}
	PlayerTics = PlayerState->tics;
}

static void M_PlayerSetupDrawer (void)
{
	int xo, yo;
	EColorRange label, value;

	if (gameinfo.gametype != GAME_Doom)
	{
		xo = 5;
		yo = 5;
		label = CR_GREEN;
		value = CR_UNTRANSLATED;
	}
	else
	{
		xo = yo = 0;
		label = CR_UNTRANSLATED;
		value = CR_GREY;
	}

	// Draw title
	if (gameinfo.gametype == GAME_Doom)
	{
		patch_t *patch = (patch_t *)W_CacheLumpName ("M_PSTTL", PU_CACHE);

		screen->DrawPatchClean (patch,
			160 - (SHORT(patch->width) >> 1),
			PSetupDef.y - (SHORT(patch->height) * 3));
	}
	else
	{
		screen->DrawTextCleanMove (CR_UNTRANSLATED,
			160 - screen->StringWidth ("PLAYER SETUP")/2,
			15,
			"PLAYER SETUP");
	}

	screen->SetFont (SmallFont);

	// Draw player name box
	screen->DrawTextCleanMove (label, PSetupDef.x, PSetupDef.y+yo, "Name");
	M_DrawSaveLoadBorder (PSetupDef.x + 56, PSetupDef.y, MAXPLAYERNAME+1);
	screen->DrawTextCleanMove (CR_UNTRANSLATED, PSetupDef.x + 56 + xo, PSetupDef.y+yo, savegamestrings[0]);

	// Draw player team box
	screen->DrawTextCleanMove (label, PSetupDef.x, PSetupDef.y + LINEHEIGHT+yo, "Team");
	M_DrawSaveLoadBorder (PSetupDef.x + 56, PSetupDef.y + LINEHEIGHT, MAXPLAYERNAME+1);
	screen->DrawTextCleanMove (CR_UNTRANSLATED, PSetupDef.x + 56+xo, PSetupDef.y + LINEHEIGHT+yo, savegamestrings[1]);

	// Draw cursor for either of the above
	if (genStringEnter)
		screen->DrawTextCleanMove (CR_UNTRANSLATED,
			PSetupDef.x + screen->StringWidth(savegamestrings[saveSlot]) + 56+xo,
			PSetupDef.y + ((saveSlot == 0) ? 0 : LINEHEIGHT) + yo, underscore);

	// Draw player character
	{
		int x = 320 - 88 - 32 + xo, y = PSetupDef.y + LINEHEIGHT*3 - 14 + yo;

		x = (x-160)*CleanXfac+(screen->width>>1);
		y = (y-100)*CleanYfac+(screen->height>>1);
		if (!FireScreen)
		{
			screen->Clear (x, y, x + 72 * CleanXfac, y + 72 * CleanYfac+yo, 0);
		}
		else
		{
			// [RH] The following fire code is based on the PTC fire demo
			int a, b;
			byte *from;
			int width, height, pitch;

			FireScreen->Lock ();

			width = FireScreen->width;
			height = FireScreen->height;
			pitch = FireScreen->pitch;

			from = FireScreen->buffer + (height - 3) * pitch;
			for (a = 0; a < width; a++, from++)
			{
				*from = *(from + (pitch << 1)) = M_Random();
			}

			from = FireScreen->buffer;
			for (b = 0; b < FireScreen->height - 4; b += 2)
			{
				byte *pixel = from;

				// special case: first pixel on line
				byte *p = pixel + (pitch << 1);
				unsigned int top = *p + *(p + width - 1) + *(p + 1);
				unsigned int bottom = *(pixel + (pitch << 2));
				unsigned int c1 = (top + bottom) >> 2;
				if (c1 > 1) c1--;
				*pixel = c1;
				*(pixel + pitch) = (c1 + bottom) >> 1;
				pixel++;

				// main line loop
				for (a = 1; a < width-1; a++)
				{
					// sum top pixels
					p = pixel + (pitch << 1);
					top = *p + *(p - 1) + *(p + 1);

					// bottom pixel
					bottom = *(pixel + (pitch << 2));

					// combine pixels
					c1 = (top + bottom) >> 2;
					if (c1 > 1) c1--;

					// store pixels
					*pixel = c1;
					*(pixel + pitch) = (c1 + bottom) >> 1;		// interpolate

					// next pixel
					pixel++;
				}

				// special case: last pixel on line
				p = pixel + (pitch << 1);
				top = *p + *(p - 1) + *(p - width + 1);
				bottom = *(pixel + (pitch << 2));
				c1 = (top + bottom) >> 2;
				if (c1 > 1) c1--;
				*pixel = c1;
				*(pixel + pitch) = (c1 + bottom) >> 1;

				// next line
				from += pitch << 1;
			}

			y--;
			pitch = screen->pitch;
			switch (CleanXfac)
			{
			case 1:
				for (b = 0; b < FireScreen->height; b++)
				{
					byte *to = screen->buffer + y * screen->pitch + x;
					from = FireScreen->buffer + b * FireScreen->pitch;
					y += CleanYfac;

					for (a = 0; a < FireScreen->width; a++, to++, from++)
					{
						int c;
						for (c = CleanYfac; c; c--)
							*(to + pitch*c) = FireRemap[*from];
					}
				}
				break;

			case 2:
				for (b = 0; b < FireScreen->height; b++)
				{
					byte *to = screen->buffer + y * screen->pitch + x;
					from = FireScreen->buffer + b * FireScreen->pitch;
					y += CleanYfac;

					for (a = 0; a < FireScreen->width; a++, to += 2, from++)
					{
						int c;
						for (c = CleanYfac; c; c--)
						{
							*(to + pitch*c) = FireRemap[*from];
							*(to + pitch*c + 1) = FireRemap[*from];
						}
					}
				}
				break;

			case 3:
				for (b = 0; b < FireScreen->height; b++)
				{
					byte *to = screen->buffer + y * screen->pitch + x;
					from = FireScreen->buffer + b * FireScreen->pitch;
					y += CleanYfac;

					for (a = 0; a < FireScreen->width; a++, to += 3, from++)
					{
						int c;
						for (c = CleanYfac; c; c--)
						{
							*(to + pitch*c) = FireRemap[*from];
							*(to + pitch*c + 1) = FireRemap[*from];
							*(to + pitch*c + 2) = FireRemap[*from];
						}
					}
				}
				break;

			case 4:
			default:
				for (b = 0; b < FireScreen->height; b++)
				{
					byte *to = screen->buffer + y * screen->pitch + x;
					from = FireScreen->buffer + b * FireScreen->pitch;
					y += CleanYfac;

					for (a = 0; a < FireScreen->width; a++, to += 4, from++)
					{
						int c;
						for (c = CleanYfac; c; c--)
						{
							*(to + pitch*c) = FireRemap[*from];
							*(to + pitch*c + 1) = FireRemap[*from];
							*(to + pitch*c + 2) = FireRemap[*from];
							*(to + pitch*c + 3) = FireRemap[*from];
						}
					}
				}
				break;
			}
			FireScreen->Unlock ();
		}
	}
	{
		spriteframe_t *sprframe =
			&sprites[skins[players[consoleplayer].userinfo.skin].sprite].spriteframes[PlayerState->frame & FF_FRAMEMASK];

		V_ColorMap = translationtables + consoleplayer*256*2;
		screen->DrawTranslatedPatchClean ((patch_t *)W_CacheLumpNum (
			sprframe->lump[PlayerRotation], PU_CACHE),
			320 - 52 - 32 + xo, PSetupDef.y + LINEHEIGHT*3 + 52);

		screen->DrawPatchClean ((patch_t *)W_CacheLumpName ("M_PBOX", PU_CACHE),
			320 - 88 - 32 + 36 + xo, PSetupDef.y + LINEHEIGHT*3 + 22 + yo);

		char *str = "PRESS " TEXTCOLOR_WHITE "SPACE";
		screen->DrawTextCleanMove (CR_GOLD, 320 - 52 - 32 -
			screen->StringWidth (str)/2,
			PSetupDef.y + LINEHEIGHT*3 + 76, str);
		str = PlayerRotation ? "TO SEE FRONT" : "TO SEE BACK";
		screen->DrawTextCleanMove (CR_GOLD, 320 - 52 - 32 -
			screen->StringWidth (str)/2,
			PSetupDef.y + LINEHEIGHT*3 + 76 + SmallFont->GetHeight (), str);
	}

	// Draw player color sliders
	//V_DrawTextCleanMove (CR_GREY, PSetupDef.x, PSetupDef.y + LINEHEIGHT, "Color");

	screen->DrawTextCleanMove (label, PSetupDef.x, PSetupDef.y + LINEHEIGHT*2+yo, "Red");
	screen->DrawTextCleanMove (label, PSetupDef.x, PSetupDef.y + LINEHEIGHT*3+yo, "Green");
	screen->DrawTextCleanMove (label, PSetupDef.x, PSetupDef.y + LINEHEIGHT*4+yo, "Blue");

	{
		int x = screen->StringWidth ("Green") + 8 + PSetupDef.x;
		int color = players[consoleplayer].userinfo.color;

		M_DrawSlider (x, PSetupDef.y + LINEHEIGHT*2+yo, 0.0f, 255.0f, RPART(color));
		M_DrawSlider (x, PSetupDef.y + LINEHEIGHT*3+yo, 0.0f, 255.0f, GPART(color));
		M_DrawSlider (x, PSetupDef.y + LINEHEIGHT*4+yo, 0.0f, 255.0f, BPART(color));
	}

	// Draw gender setting
	{
		int x = screen->StringWidth ("Gender") + 8 + PSetupDef.x;
		screen->DrawTextCleanMove (label, PSetupDef.x, PSetupDef.y + LINEHEIGHT*5+yo, "Gender");
		screen->DrawTextCleanMove (value, x, PSetupDef.y + LINEHEIGHT*5+yo,
							  genders[players[consoleplayer].userinfo.gender]);
	}

	// Draw skin setting
	{
		int x = screen->StringWidth ("Skin") + 8 + PSetupDef.x;
		screen->DrawTextCleanMove (label, PSetupDef.x, PSetupDef.y + LINEHEIGHT*6+yo, "Skin");
		screen->DrawTextCleanMove (value, x, PSetupDef.y + LINEHEIGHT*6+yo,
							  skins[players[consoleplayer].userinfo.skin].name);
	}

	// Draw autoaim setting
	{
		int x = screen->StringWidth ("Autoaim") + 8 + PSetupDef.x;
		float aim = autoaim.value;

		screen->DrawTextCleanMove (label, PSetupDef.x, PSetupDef.y + LINEHEIGHT*7+yo, "Autoaim");
		screen->DrawTextCleanMove (value, x, PSetupDef.y + LINEHEIGHT*7+yo,
			aim == 0 ? "Never" :
			aim <= 0.25 ? "Very Low" :
			aim <= 0.5 ? "Low" :
			aim <= 1 ? "Medium" :
			aim <= 2 ? "High" :
			aim <= 3 ? "Very High" : "Always");
	}
	{
		char foo[4];
		sprintf (foo, "%d", players[consoleplayer].userinfo.skin);
		screen->DrawText (CR_YELLOW, 0, 0, foo);
	}
}

static void M_ChangeGender (int choice)
{
	int gender = players[consoleplayer].userinfo.gender;

	if (!choice)
		gender = (gender == 0) ? 2 : gender - 1;
	else
		gender = (gender == 2) ? 0 : gender + 1;

	cvar_set ("gender", genders[gender]);
}

static void M_ChangeSkin (int choice)
{
	int skin = players[consoleplayer].userinfo.skin;

	if (!choice)
		skin = (skin == 0) ? numskins - 1 : skin - 1;
	else
		skin = (skin < (int)numskins - 1) ? skin + 1 : 0;

	cvar_set ("skin", skins[skin].name);
}

static void M_ChangeAutoAim (int choice)
{
	static const float ranges[] = { 0, 0.25, 0.5, 1, 2, 3, 5000 };
	float aim = autoaim.value;
	int i;

	if (!choice) {
		// Select a lower autoaim

		for (i = 6; i >= 1; i--) {
			if (aim >= ranges[i]) {
				aim = ranges[i - 1];
				break;
			}
		}
	} else {
		// Select a higher autoaim

		for (i = 5; i >= 0; i--) {
			if (aim >= ranges[i]) {
				aim = ranges[i + 1];
				break;
			}
		}
	}

	autoaim.Set (aim);
}

static void M_EditPlayerName (int choice)
{
	// we are going to be intercepting all chars
	genStringEnter = 1;
	genStringEnd = M_PlayerNameChanged;
	genStringLen = MAXPLAYERNAME;
	
	saveSlot = 0;
	strcpy(saveOldString,savegamestrings[0]);
	if (!strcmp(savegamestrings[0],EMPTYSTRING))
		savegamestrings[0][0] = 0;
	saveCharIndex = strlen(savegamestrings[0]);
}

static void M_PlayerNameChanged (int choice)
{
	char command[SAVESTRINGSIZE+8];

	sprintf (command, "name \"%s\"", savegamestrings[0]);
	AddCommandString (command);
}

static void M_EditPlayerTeam (int choice)
{
	// we are going to be intercepting all chars
	genStringEnter = 1;
	genStringEnd = M_PlayerTeamChanged;
	genStringLen = MAXPLAYERNAME;
	
	saveSlot = 1;
	strcpy(saveOldString,savegamestrings[1]);
	if (!strcmp(savegamestrings[1],EMPTYSTRING))
		savegamestrings[1][0] = 0;
	saveCharIndex = strlen(savegamestrings[1]);
}

static void M_PlayerTeamChanged (int choice)
{
	char command[SAVESTRINGSIZE+8];

	sprintf (command, "team \"%s\"", savegamestrings[1]);
	AddCommandString (command);
}


static void SendNewColor (int red, int green, int blue)
{
	char command[24];

	sprintf (command, "color \"%02x %02x %02x\"", red, green, blue);
	AddCommandString (command);
}

static void M_SlidePlayerRed (int choice)
{
	int color = players[consoleplayer].userinfo.color;
	int red = RPART(color);

	if (choice == 0) {
		red -= 16;
		if (red < 0)
			red = 0;
	} else {
		red += 16;
		if (red > 255)
			red = 255;
	}

	SendNewColor (red, GPART(color), BPART(color));
}

static void M_SlidePlayerGreen (int choice)
{
	int color = players[consoleplayer].userinfo.color;
	int green = GPART(color);

	if (choice == 0) {
		green -= 16;
		if (green < 0)
			green = 0;
	} else {
		green += 16;
		if (green > 255)
			green = 255;
	}

	SendNewColor (RPART(color), green, BPART(color));
}

static void M_SlidePlayerBlue (int choice)
{
	int color = players[consoleplayer].userinfo.color;
	int blue = BPART(color);

	if (choice == 0) {
		blue -= 16;
		if (blue < 0)
			blue = 0;
	} else {
		blue += 16;
		if (blue > 255)
			blue = 255;
	}

	SendNewColor (RPART(color), GPART(color), blue);
}


//
//		Menu Functions
//
void M_StartMessage (char *string, void (*routine)(int), bool input)
{
	C_HideConsole ();
	messageLastMenuActive = menuactive;
	messageToPrint = 1;
	messageString = string;
	messageRoutine = routine;
	messageNeedsInput = input;
	menuactive = true;
	if (input)
	{
		S_StopSound ((AActor *)NULL, CHAN_VOICE);
		S_Sound (CHAN_VOICE, "menu/prompt", 1, ATTN_NONE);
	}
	return;
}



//
//		Find string height from hu_font chars
//
int M_StringHeight (const char *string)
{
	int h;
	int height = screen->Font->GetHeight ();
		
	h = height;
	while (*string)
	{
		if ((*string++) == '\n')
			h += height;
	}
				
	return h;
}



//
// CONTROL PANEL
//

//
// M_Responder
//
BOOL M_Responder (event_t* ev)
{
	int ch, ch2;
	int i;

	ch = ch2 = -1;

	if (ev->type == ev_keydown)
	{
		ch = ev->data1; 		// scancode
		ch2 = ev->data2;		// ASCII
	}
	
	if (ch == -1 || chatmodeon)
		return false;

	if (menuactive && OptionsActive)
	{
		M_OptResponder (ev);
		return true;
	}
	
	// Save Game string input
	// [RH] and Player Name string input
	if (genStringEnter)
	{
		switch(ch)
		{
		case KEY_BACKSPACE:
			if (saveCharIndex > 0)
			{
				saveCharIndex--;
				savegamestrings[saveSlot][saveCharIndex] = 0;
			}
			break;

		case KEY_ESCAPE:
			genStringEnter = 0;
			M_ClearMenus ();
			strcpy(&savegamestrings[saveSlot][0],saveOldString);
			break;
								
		case KEY_ENTER:
			genStringEnter = 0;
			if (messageToPrint)
				M_ClearMenus ();
			if (savegamestrings[saveSlot][0])
				genStringEnd(saveSlot);	// [RH] Function to call when enter is pressed
			break;

		default:
			ch = toupper(ev->data3);	// [RH] Use user keymap
			if (ch != 32)
				if (ch-HU_FONTSTART < 0 || ch-HU_FONTSTART >= HU_FONTSIZE)
					break;
			if (ch >= 32 && ch <= 127 &&
				saveCharIndex < genStringLen &&
				screen->StringWidth(savegamestrings[saveSlot]) <
				(genStringLen-1)*8)
			{
				savegamestrings[saveSlot][saveCharIndex++] = ch;
				savegamestrings[saveSlot][saveCharIndex] = 0;
			}
			break;
		}
		return true;
	}
	
	// Take care of any messages that need input
	if (messageToPrint)
	{
		if (messageNeedsInput &&
			!(ch2 == ' ' || ch2 == 'n' || ch2 == 'y' || ch == KEY_ESCAPE))
			return false;
				
		menuactive = messageLastMenuActive;
		messageToPrint = 0;
		if (messageRoutine)
			messageRoutine(ch2);
						
		menuactive = false;
		SB_state = -1;	// refresh the statbar
		BorderNeedRefresh = true;
		S_Sound (CHAN_VOICE, "menu/dismiss", 1, ATTN_NONE);
		return true;
	}
		
	// [RH] F-Keys are now just normal keys that can be bound,
	//		so they aren't checked here anymore.
	
	// If devparm is set, pressing F1 always takes a screenshot no matter
	// what it's bound to. (for those who don't bother to read the docs)
	if (devparm && ch == KEY_F1)
	{
		G_ScreenShot (NULL);
		return true;
	}

	// Pop-up menu?
	if (!menuactive)
	{
		if (ch == KEY_ESCAPE)
		{
			M_StartControlPanel (true);
			M_SetupNextMenu (TopLevelMenu);
			return true;
		}
		return false;
	}

	
	// Keys usable within menu
	switch (ch)
	{
	case KEY_DOWNARROW:
		do
		{
			if (itemOn+1 > currentMenu->numitems-1)
				itemOn = 0;
			else itemOn++;
			S_Sound (CHAN_VOICE, "menu/cursor", 1, ATTN_NONE);
		} while(currentMenu->menuitems[itemOn].status==-1);
		return true;
				
	case KEY_UPARROW:
		do
		{
			if (!itemOn)
				itemOn = currentMenu->numitems-1;
			else itemOn--;
			S_Sound (CHAN_VOICE, "menu/cursor", 1, ATTN_NONE);
		} while(currentMenu->menuitems[itemOn].status==-1);
		return true;

	case KEY_LEFTARROW:
		if (currentMenu->menuitems[itemOn].routine &&
			currentMenu->menuitems[itemOn].status == 2)
		{
			S_Sound (CHAN_VOICE, "menu/change", 1, ATTN_NONE);
			currentMenu->menuitems[itemOn].routine(0);
		}
		return true;
				
	case KEY_RIGHTARROW:
		if (currentMenu->menuitems[itemOn].routine &&
			currentMenu->menuitems[itemOn].status == 2)
		{
			S_Sound (CHAN_VOICE, "menu/change", 1, ATTN_NONE);
			currentMenu->menuitems[itemOn].routine(1);
		}
		return true;

	case KEY_ENTER:
		if (currentMenu->menuitems[itemOn].routine &&
			currentMenu->menuitems[itemOn].status)
		{
			currentMenu->lastOn = itemOn;
			if (currentMenu->menuitems[itemOn].status == 2)
			{
				currentMenu->menuitems[itemOn].routine(1);		// right arrow
				S_Sound (CHAN_VOICE, "menu/change", 1, ATTN_NONE);
			}
			else
			{
				currentMenu->menuitems[itemOn].routine(itemOn);
				S_Sound (CHAN_VOICE, "menu/choose", 1, ATTN_NONE);
			}
		}
		return true;
				
	  // [RH] Escape now moves back one menu instead of
	  //	  quitting the menu system. Thus, backspace
	  //	  is now ignored.
	case KEY_ESCAPE:
		currentMenu->lastOn = itemOn;
		M_PopMenuStack ();
		return true;
		
	case KEY_SPACE:
		if (currentMenu == &PSetupDef)
		{
			PlayerRotation ^= 4;
			break;
		}
		// intentional fall-through
								
	default:
		if (ch2)
		{
			for (i = itemOn+1; i < currentMenu->numitems; i++)
			{
				if (currentMenu->menuitems[i].alphaKey == ch2)
				{
					itemOn = i;
					S_Sound (CHAN_VOICE, "menu/change", 1, ATTN_NONE);
					return true;
				}
			}
			for (i = 0; i <= itemOn; i++)
			{
				if (currentMenu->menuitems[i].alphaKey == ch2)
				{
					itemOn = i;
					S_Sound (CHAN_VOICE, "menu/cursor", 1, ATTN_NONE);
					return true;
				}
			}
		}
		break;
		
	}

	// [RH] Menu now eats all keydown events while active
	return (ev->type == ev_keydown);
}



//
// M_StartControlPanel
//
void M_StartControlPanel (bool makeSound)
{
	// intro might call this repeatedly
	if (menuactive)
		return;
	
	drawSkull = true;
	MenuStackDepth = 0;
	menuactive = 1;
	currentMenu = TopLevelMenu;
	itemOn = currentMenu->lastOn;
	C_HideConsole ();				// [RH] Make sure console goes bye bye.
	OptionsActive = false;			// [RH] Make sure none of the options menus appear.
	I_PauseMouse ();				// [RH] Give the mouse back in windowed modes.

	if (makeSound)
	{
		S_Sound (CHAN_VOICE, "menu/activate", 1, ATTN_NONE);
	}
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer ()
{
	int i, x, y, max;

	// Horiz. & Vertically center string and print it.
	if (messageToPrint)
	{
		screen->Dim ();
		BorderNeedRefresh = true;
		SB_state = -1;

		brokenlines_t *lines = V_BreakLines (320, messageString);
		y = 100;

		for (i = 0; lines[i].width != -1; i++)
			y -= screen->Font->GetHeight () / 2;

		for (i = 0; lines[i].width != -1; i++)
		{
			screen->DrawTextCleanMove (CR_UNTRANSLATED, 160 - lines[i].width/2, y, lines[i].string);
			y += screen->Font->GetHeight ();
		}

		V_FreeBrokenLines (lines);
	}
	else if (menuactive)
	{
		screen->Dim ();
		BorderNeedRefresh = true;
		SB_state = -1;

		if (OptionsActive)
		{
			M_OptDrawer ();
		}
		else
		{
			screen->SetFont (BigFont);
			if (currentMenu->routine)
				currentMenu->routine(); 		// call Draw routine
		
			// DRAW MENU
			x = currentMenu->x;
			y = currentMenu->y;
			max = currentMenu->numitems;

			for (i = 0; i < max; i++)
			{
				if (currentMenu->menuitems[i].name)
				{
					if (currentMenu->menuitems[i].fulltext)
					{
						screen->DrawTextCleanMove (CR_UNTRANSLATED, x, y,
							currentMenu->menuitems[i].name);
					}
					else
					{
						screen->DrawPatchClean ((patch_t *)W_CacheLumpName (
							currentMenu->menuitems[i].name ,PU_CACHE), x, y);
					}
				}
				y += LINEHEIGHT;
			}
			screen->SetFont (SmallFont);
			
			// DRAW CURSOR
			if (drawSkull)
			{
				if (gameinfo.gametype == GAME_Doom)
				{
					screen->DrawPatchClean ((patch_t *)W_CacheLumpName (
						skullName[whichSkull], PU_CACHE),
						x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT);
				}
				else
				{
					screen->DrawPatchClean ((patch_t *)W_CacheLumpName (
						MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2", PU_CACHE),
						x + SELECTOR_XOFFSET,
						currentMenu->y + itemOn*LINEHEIGHT + SELECTOR_YOFFSET);
				}
			}
		}
	}
}


//
// M_ClearMenus
//
void M_ClearMenus ()
{
	if (FireScreen)
	{
		delete FireScreen;
		FireScreen = NULL;
	}
	menuactive = false;
	MenuStackDepth = 0;
	InfoType = 0;
	drawSkull = true;
	M_DemoNoPlay = false;
	BorderNeedRefresh = true;
	C_HideConsole ();		// [RH] Hide the console if we can.
	if (gamestate != GS_FULLCONSOLE)
		I_ResumeMouse ();	// [RH] Recapture the mouse in windowed modes.
}




//
// M_SetupNextMenu
//
void M_SetupNextMenu (oldmenu_t *menudef)
{
	MenuStack[MenuStackDepth].menu.old = menudef;
	MenuStack[MenuStackDepth].isNewStyle = false;
	MenuStack[MenuStackDepth].drawSkull = drawSkull;
	MenuStackDepth++;

	currentMenu = menudef;
	itemOn = currentMenu->lastOn;
}


void M_PopMenuStack (void)
{
	M_DemoNoPlay = false;
	InfoType = 0;
	if (MenuStackDepth > 1)
	{
		I_PauseMouse ();
		MenuStackDepth -= 2;
		if (MenuStack[MenuStackDepth].isNewStyle)
		{
			OptionsActive = true;
			CurrentMenu = MenuStack[MenuStackDepth].menu.newmenu;
			CurrentItem = CurrentMenu->lastOn;
		}
		else
		{
			OptionsActive = false;
			currentMenu = MenuStack[MenuStackDepth].menu.old;
			itemOn = currentMenu->lastOn;
		}
		drawSkull = MenuStack[MenuStackDepth].drawSkull;
		MenuStackDepth++;
		S_Sound (CHAN_VOICE, "menu/backup", 1, ATTN_NONE);
	}
	else
	{
		M_ClearMenus ();
		S_Sound (CHAN_VOICE, "menu/clear", 1, ATTN_NONE);
	}
}


//
// M_Ticker
//
void M_Ticker (void)
{
	if (!menuactive)
	{
		return;
	}
	MenuTime++;
	if (--skullAnimCounter <= 0)
	{
		whichSkull ^= 1;
		skullAnimCounter = 8;
	}
	if (currentMenu == &PSetupDef)
		M_PlayerSetupTicker ();
}


//
// M_Init
//
EXTERN_CVAR (screenblocks)

void M_Init (void)
{
	int i;

	if (gameinfo.gametype == GAME_Doom)
	{
		TopLevelMenu = currentMenu = &MainDef;
		PlayerClass = TypeInfo::FindType ("DoomPlayer");
	}
	else
	{
		TopLevelMenu = currentMenu = &HereticMainDef;
		PlayerClass = TypeInfo::FindType ("HereticPlayer");
		PSetupDef.y -= 7;
		LoadDef.y -= 20;
		SaveDef.y -= 20;
	}
	OptionsActive = false;
	menuactive = 0;
	InfoType = 0;
	itemOn = currentMenu->lastOn;
	whichSkull = 0;
	skullAnimCounter = 10;
	drawSkull = true;
	messageToPrint = 0;
	messageString = NULL;
	messageLastMenuActive = menuactive;
	quickSaveSlot = -1;
	SkullBaseLump = W_CheckNumForName ("M_SKL00");

	underscore[0] = (gameinfo.gametype == GAME_Doom) ? '_' : '[';
	underscore[1] = '\0';

	if (gameinfo.gametype == GAME_Doom)
	{
		LINEHEIGHT = 16;
	}
	else
	{
		LINEHEIGHT = 20;
	}

	if (gameinfo.flags & GI_SHAREWARE)
	{
		EpiDef.numitems = 1;
		HereticEpiDef.numitems = 1;
	}

	switch (gameinfo.flags & GI_MENUHACK)
	{
	case GI_MENUHACK_COMMERCIAL:
		MainMenu[MainDef.numitems-2] = MainMenu[MainDef.numitems-1];
		MainDef.numitems--;
		MainDef.y += 8;
		ReadDef.routine = M_DrawReadThis;
		ReadDef.x = 330;
		ReadDef.y = 165;
		ReadMenu[0].routine = M_FinishReadThis;
		break;
	case GI_MENUHACK_RETAIL:
		// add the fourth episode.
		EpiDef.numitems++;
		break;
	case GI_MENUHACK_EXTENDED:
		HereticEpiDef.numitems = 5;
		HereticEpiDef.y -= LINEHEIGHT;
		break;
	default:
		break;
	}
	M_OptInit ();

	// [RH] Build a palette translation table for the fire
	for (i = 0; i < 255; i++)
		FireRemap[i] = BestColor (DefaultPalette->basecolors, i, 0, 0, DefaultPalette->numcolors);
}

