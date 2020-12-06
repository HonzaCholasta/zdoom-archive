/* m_options.c
**
** New options menu code.
**
** Sorry this got so convoluted. It was originally much cleaner until
** I started adding all sorts of gadgets to the menus. I might someday
** make a project of rewriting the entire menu system using Amiga-style
** taglists to describe each menu item. We'll see...
*/

#include "m_alloc.h"

#include "doomdef.h"
#include "dstrings.h"

#include "c_console.h"
#include "c_dispatch.h"
#include "c_bind.h"

#include "d_main.h"

#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "v_video.h"
#include "v_text.h"
#include "w_wad.h"
#include "gi.h"

#include "r_local.h"


#include "hu_stuff.h"

#include "g_game.h"

#include "m_argv.h"
#include "m_swap.h"

#include "s_sound.h"

#include "doomstat.h"

#include "m_misc.h"

// Data.
#include "m_menu.h"

//
// defaulted values
//
CVAR (mouse_sensitivity, "1.0", CVAR_ARCHIVE)

// Show messages has default, 0 = off, 1 = on
CVAR (show_messages, "1", CVAR_ARCHIVE)

extern bool	OptionsActive;

extern int	skullAnimCounter;

EXTERN_CVAR (cl_run)
EXTERN_CVAR (invertmouse)
EXTERN_CVAR (lookspring)
EXTERN_CVAR (lookstrafe)
EXTERN_CVAR (crosshair)
EXTERN_CVAR (freelook)

void M_ChangeMessages ();
void M_SizeDisplay (float diff);

int  M_StringHeight (char *string);
void M_ClearMenus ();

EColorRange LabelColor;
EColorRange ValueColor;
EColorRange MoreColor;

static bool CanScrollUp;
static bool CanScrollDown;
static int VisBottom;

value_t YesNo[2] = {
	{ 0.0, "No" },
	{ 1.0, "Yes" }
};

value_t NoYes[2] = {
	{ 0.0, "Yes" },
	{ 1.0, "No" }
};

value_t OnOff[2] = {
	{ 0.0, "Off" },
	{ 1.0, "On" }
};

menu_t  *CurrentMenu;
int		CurrentItem;
static bool		WaitingForKey;
static char	   *OldMessage;
static itemtype OldType;

/*=======================================
 *
 * Options Menu
 *
 *=======================================*/

static void CustomizeControls (void);
static void GameplayOptions (void);
static void VideoOptions (void);
static void GoToConsole (void);
void Reset2Defaults (void);
void Reset2Saved (void);

EXTERN_CVAR (snd_midivolume)

static void SetVidMode (void);

static menuitem_t OptionItems[] =
{
	{ more,		"Customize Controls",	{NULL},					{0.0}, {0.0},	{0.0}, {(value_t *)CustomizeControls} },
	{ more,		"Go to console",		{NULL},					{0.0}, {0.0},	{0.0}, {(value_t *)GoToConsole} },
	{ more,		"Gameplay Options",		{NULL},					{0.0}, {0.0},	{0.0}, {(value_t *)GameplayOptions} },
	{ more,		"Display Options",		{NULL},					{0.0}, {0.0},	{0.0}, {(value_t *)VideoOptions} },
	{ more,		"Set video mode",		{NULL},					{0.0}, {0.0},	{0.0}, {(value_t *)SetVidMode} },
	{ redtext,	" ",					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ slider,	"Mouse speed",			{&mouse_sensitivity},	{0.5}, {2.5},	{0.1}, {NULL} },
	{ slider,	"MIDI music volume",	{&snd_midivolume},		{0.0}, {1.0},	{0.05}, {NULL} },
	{ slider,	"MOD music volume",		{&snd_musicvolume},		{0.0}, {64.0},	{1.0}, {NULL} },
	{ slider,	"Sound volume",			{&snd_sfxvolume},		{0.0}, {15.0},	{1.0}, {NULL} },
	{ discrete,	"Always Run",			{&cl_run},				{2.0}, {0.0},	{0.0}, {OnOff} },
	{ discrete, "Always Mouselook",		{&freelook},			{2.0}, {0.0},	{0.0}, {OnOff} },
	{ discrete, "Invert Mouse",			{&invertmouse},			{2.0}, {0.0},	{0.0}, {OnOff} },
	{ discrete, "Lookspring",			{&lookspring},			{2.0}, {0.0},	{0.0}, {OnOff} },
	{ discrete, "Lookstrafe",			{&lookstrafe},			{2.0}, {0.0},	{0.0}, {OnOff} },
	{ redtext,	" ",					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ more,		"Reset to defaults",	{NULL},					{0.0}, {0.0},	{0.0}, {(value_t *)Reset2Defaults} },
	{ more,		"Reset to last saved",	{NULL},					{0.0}, {0.0},	{0.0}, {(value_t *)Reset2Saved} }
};

menu_t OptionMenu = {
	{ 'M','_','O','P','T','T','T','L' },
	"OPTIONS",
	0,
	16,
	177,
	OptionItems,
};


/*=======================================
 *
 * Controls Menu
 *
 *=======================================*/

static menuitem_t ControlsItems[] = {
	{ redtext,"ENTER to change, BACKSPACE to clear", {NULL}, {0.0}, {0.0}, {0.0}, {NULL} },
	{ redtext,	" ",					{NULL},	{0.0}, {0.0}, {0.0}, {NULL} },
	{ whitetext,"Controls",				{NULL},	{0.0}, {0.0}, {0.0}, {NULL} },
	{ control,	"Fire",					{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+attack"} },
	{ control,	"Use / Open",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+use"} },
	{ control,	"Move forward",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+forward"} },
	{ control,	"Move backward",		{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+back"} },
	{ control,	"Strafe left",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+moveleft"} },
	{ control,	"Strafe right",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+moveright"} },
	{ control,	"Turn left",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+left"} },
	{ control,	"Turn right",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+right"} },
	{ control,	"Jump",					{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+jump"} },
	{ control,	"Fly / Swim up",		{NULL},	{0.0}, {0.0}, {0.0}, {(value_t *)"+moveup"} },
	{ control,	"Fly / Swim down",		{NULL},	{0.0}, {0.0}, {0.0}, {(value_t *)"+movedown"} },
	{ control,	"Stop flying",			{NULL},	{0.0}, {0.0}, {0.0}, {(value_t *)"land"} },
	{ control,	"Mouse look",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+mlook"} },
	{ control,	"Keyboard look",		{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+klook"} },
	{ control,	"Look up",				{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+lookup"} },
	{ control,	"Look down",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+lookdown"} },
	{ control,	"Center view",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"centerview"} },
	{ control,	"Run",					{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+speed"} },
	{ control,	"Strafe",				{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"+strafe"} },
	{ redtext,	" ",					{NULL},	{0.0}, {0.0}, {0.0}, {NULL} },
	{ whitetext,"Chat",					{NULL},	{0.0}, {0.0}, {0.0}, {NULL} },
	{ control,	"Say",					{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"messagemode"} },
	{ control,	"Team say",				{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"messagemode2"} },
	{ redtext,	" ",					{NULL},	{0.0}, {0.0}, {0.0}, {NULL} },
	{ whitetext,"Weapons",				{NULL},	{0.0}, {0.0}, {0.0}, {NULL} },
	{ control,	"Next weapon",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"weapnext"} },
	{ control,	"Previous weapon",		{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"weapprev"} },
	{ redtext,	" ",					{NULL},	{0.0}, {0.0}, {0.0}, {NULL} },
	{ whitetext,"Inventory",			{NULL},	{0.0}, {0.0}, {0.0}, {NULL} },
	{ control,	"Activate item",		{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"invuse"} },
	{ control,	"Activate all items",	{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"invuseall"} },
	{ control,	"Next item",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"invnext"} },
	{ control,	"Previous item",		{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"invprev"} },
	{ redtext,	" ",					{NULL},	{0.0}, {0.0}, {0.0}, {NULL} },
	{ whitetext,"Other",				{NULL},	{0.0}, {0.0}, {0.0}, {NULL} },
	{ control,	"Toggle automap",		{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"togglemap"} },
	{ control,	"Chasecam",				{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"chase"} },
	{ control,	"Add a bot",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"addbot"} },
	{ control,	"Coop spy",				{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"spynext"} },
	{ control,	"Screeshot",			{NULL}, {0.0}, {0.0}, {0.0}, {(value_t *)"screenshot"} }
};

menu_t ControlsMenu = {
	{ 'M','_','C','O','N','T','R','O' },
	"CUSTOMIZE CONTROLS",
	3,
	sizeof(ControlsItems)/sizeof(ControlsItems[0]),
	0,
	ControlsItems,
	2,
};

/*=======================================
 *
 * Display Options Menu
 *
 *=======================================*/
static void StartMessagesMenu (void);

EXTERN_CVAR (am_rotate)
EXTERN_CVAR (am_overlay)
EXTERN_CVAR (st_scale)
EXTERN_CVAR (am_usecustomcolors)
EXTERN_CVAR (r_detail)
EXTERN_CVAR (r_stretchsky)
EXTERN_CVAR (r_columnmethod)
EXTERN_CVAR (r_drawfuzz)
EXTERN_CVAR (cl_rockettrails)
EXTERN_CVAR (cl_pufftype)
EXTERN_CVAR (cl_bloodtype)
EXTERN_CVAR (wipetype)
EXTERN_CVAR (vid_palettehack)
EXTERN_CVAR (screenblocks)

static value_t Crosshairs[] =
{
	{ 0.0, "None" },
	{ 1.0, "Cross 1" },
	{ 2.0, "Cross 2" },
	{ 3.0, "X" },
	{ 4.0, "Circle" },
	{ 5.0, "Angle" },
	{ 6.0, "Triangle" },
	{ 7.0, "Dot" }
};

static value_t DetailModes[] =
{
	{ 0.0, "Normal" },
	{ 1.0, "Double Horizontally" },
	{ 2.0, "Double Vertically" },
	{ 3.0, "Double Horiz and Vert" }
};

static value_t ColumnMethods[] = {
	{ 0.0, "Original" },
	{ 1.0, "Optimized" }
};

static value_t BloodTypes[] = {
	{ 0.0, "Sprites" },
	{ 1.0, "Sprites and Particles" },
	{ 2.0, "Particles" }
};

static value_t PuffTypes[] = {
	{ 0.0, "Sprites" },
	{ 1.0, "Particles" }
};

static value_t Wipes[] = {
	{ 0.0, "None" },
	{ 1.0, "Melt" },
	{ 2.0, "Burn" },
	{ 3.0, "Crossfade" }
};

static menuitem_t VideoItems[] = {
	{ more,		"Messages",				{NULL},					{0.0}, {0.0},	{0.0}, {(value_t *)StartMessagesMenu} },
	{ redtext,	" ",					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ slider,	"Screen size",			{&screenblocks},	   	{3.0}, {12.0},	{1.0}, {NULL} },
	{ slider,	"Brightness",			{&Gamma},			   	{1.0}, {3.0},	{0.1}, {NULL} },
	{ discrete,	"Crosshair",			{&crosshair},		   	{8.0}, {0.0},	{0.0}, {Crosshairs} },
	{ discrete, "Column render mode",	{&r_columnmethod},		{2.0}, {0.0},	{0.0}, {ColumnMethods} },
	{ discrete, "Detail mode",			{&r_detail},		   	{4.0}, {0.0},	{0.0}, {DetailModes} },
	{ discrete, "Stretch short skies",	{&r_stretchsky},	   	{2.0}, {0.0},	{0.0}, {OnOff} },
	{ discrete, "Stretch status bar",	{&st_scale},			{2.0}, {0.0},	{0.0}, {OnOff} },
	{ discrete, "Screen wipe style",	{&wipetype},			{4.0}, {0.0},	{0.0}, {Wipes} },
#ifdef _WIN32
	{ discrete, "DirectDraw palette hack", {&vid_palettehack},	{2.0}, {0.0},	{0.0}, {OnOff} },
#endif
	{ redtext,	" ",					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ discrete, "Use fuzz effect",		{&r_drawfuzz},			{2.0}, {0.0},	{0.0}, {YesNo} },
	{ discrete, "Rocket Trails",		{&cl_rockettrails},		{2.0}, {0.0},	{0.0}, {OnOff} },
	{ discrete, "Blood Type",			{&cl_bloodtype},	   	{3.0}, {0.0},	{0.0}, {BloodTypes} },
	{ discrete, "Bullet Puff Type",		{&cl_pufftype},			{2.0}, {0.0},	{0.0}, {PuffTypes} },
	{ redtext,	" ",					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ discrete, "Rotate automap",		{&am_rotate},		   	{2.0}, {0.0},	{0.0}, {OnOff} },
	{ discrete, "Overlay automap",		{&am_overlay},			{2.0}, {0.0},	{0.0}, {OnOff} },
	{ discrete, "Standard map colors",	{&am_usecustomcolors},	{2.0}, {0.0},	{0.0}, {NoYes} },
};

menu_t VideoMenu = {
	"M_VIDEO",
	"DISPLAY OPTIONS",
	0,
#ifdef _WIN32
	20,
#else
	19,
#endif
	0,
	VideoItems,
};

/*=======================================
 *
 * Messages Menu
 *
 *=======================================*/
EXTERN_CVAR (con_scaletext)
EXTERN_CVAR (con_centernotify)
EXTERN_CVAR (msg0color)
EXTERN_CVAR (msg1color)
EXTERN_CVAR (msg2color)
EXTERN_CVAR (msg3color)
EXTERN_CVAR (msg4color)
EXTERN_CVAR (msgmidcolor)
EXTERN_CVAR (msglevel)

static value_t TextColors[] =
{
	{ 0.0, "brick" },
	{ 1.0, "tan" },
	{ 2.0, "gray" },
	{ 3.0, "green" },
	{ 4.0, "brown" },
	{ 5.0, "gold" },
	{ 6.0, "red" },
	{ 7.0, "blue" },
	{ 8.0, "orange" },
	{ 9.0, "white" },
	{ 10.0, "yellow" }
};

static value_t MessageLevels[] = {
	{ 0.0, "Item Pickup" },
	{ 1.0, "Obituaries" },
	{ 2.0, "Critical Messages" }
};

static menuitem_t MessagesItems[] = {
	{ discrete,	"Scale text in high res", {&con_scaletext},		{2.0}, {0.0}, 	{0.0}, {OnOff} },
	{ discrete, "Minimum message level", {&msglevel},		   	{3.0}, {0.0},   {0.0}, {MessageLevels} },
	{ discrete, "Center messages",		{&con_centernotify},	{2.0}, {0.0},	{0.0}, {OnOff} },
	{ redtext,	" ",					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ whitetext, "Message Colors",		{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ redtext,	" ",					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ cdiscrete, "Item Pickup",			{&msg0color},		   	{11.0}, {0.0},	{0.0}, {TextColors} },
	{ cdiscrete, "Obituaries",			{&msg1color},		   	{11.0}, {0.0},	{0.0}, {TextColors} },
	{ cdiscrete, "Critical Messages",	{&msg2color},		   	{11.0}, {0.0},	{0.0}, {TextColors} },
	{ cdiscrete, "Chat Messages",		{&msg3color},		   	{11.0}, {0.0},	{0.0}, {TextColors} },
	{ cdiscrete, "Team Messages",		{&msg4color},		   	{11.0}, {0.0},	{0.0}, {TextColors} },
	{ cdiscrete, "Centered Messages",	{&msgmidcolor},			{11.0}, {0.0},	{0.0}, {TextColors} }
};

menu_t MessagesMenu = {
	"M_MESS",
	"MESSAGES",
	0,
	12,
	0,
	MessagesItems,
};


/*=======================================
 *
 * Video Modes Menu
 *
 *=======================================*/

extern BOOL setmodeneeded;
extern int NewWidth, NewHeight, NewBits;
extern int DisplayBits;

int testingmode;		// Holds time to revert to old mode
int OldWidth, OldHeight, OldBits;

static void BuildModesList (int hiwidth, int hiheight, int hi_id);
static BOOL GetSelectedSize (int line, int *width, int *height);
static void SetModesMenu (int w, int h, int bits);

EXTERN_CVAR (vid_defwidth)
EXTERN_CVAR (vid_defheight)
EXTERN_CVAR (vid_defbits)

static cvar_t DummyDepthCvar (NULL, NULL, 0);

EXTERN_CVAR (fullscreen)

static value_t Depths[22];

static char VMEnterText[] = "Press ENTER to set mode";
static char VMTestText[] = "T to test mode for 5 seconds";

static menuitem_t ModesItems[] = {
	{ discrete, "Screen mode",			{&DummyDepthCvar},		{0.0}, {0.0},	{0.0}, {Depths} },
	{ redtext,	" ",					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ discrete, "Fullscreen",			{&fullscreen},			{2.0}, {0.0},	{0.0}, {YesNo} },
	{ redtext,	" ",					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ screenres,{NULL},					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ screenres,{NULL},					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ screenres,{NULL},					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ screenres,{NULL},					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ screenres,{NULL},					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ screenres,{NULL},					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ screenres,{NULL},					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ screenres,{NULL},					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ screenres,{NULL},					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ whitetext,"Note: Only 8 bpp modes are supported",{NULL},	{0.0}, {0.0},	{0.0}, {NULL} },
	{ redtext,  VMEnterText,			{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ redtext,	" ",					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ redtext,  VMTestText,				{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ redtext,	" ",					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ redtext,  {NULL},					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} },
	{ redtext,  {NULL},					{NULL},					{0.0}, {0.0},	{0.0}, {NULL} }
};

#define VM_DEPTHITEM	0
#define VM_RESSTART		4
#define VM_ENTERLINE	14
#define VM_TESTLINE		16
#define VM_MAKEDEFLINE	18
#define VM_CURDEFLINE	19

menu_t ModesMenu = {
	{ 'M','_','V','I','D','M','O','D' },
	"VIDEO MODE",
	2,
	20,
	130,
	ModesItems,
};

/*=======================================
 *
 * Gameplay Options (dmflags) Menu
 *
 *=======================================*/

static cvar_t *flagsvar;

static menuitem_t DMFlagsItems[] = {
	{ bitflag,	"Falling damage",		{(cvar_t *)DF_YES_FALLING},			{0}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Weapons stay (DM)",	{(cvar_t *)DF_WEAPONS_STAY},		{0}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Allow powerups (DM)",	{(cvar_t *)DF_NO_ITEMS},			{1}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Allow health (DM)",	{(cvar_t *)DF_NO_HEALTH},			{1}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Allow armor (DM)",		{(cvar_t *)DF_NO_ARMOR},			{1}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Spawn farthest (DM)",	{(cvar_t *)DF_SPAWN_FARTHEST},		{0}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Same map (DM)",		{(cvar_t *)DF_SAME_LEVEL},			{0}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Force respawn (DM)",	{(cvar_t *)DF_FORCE_RESPAWN},		{0}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Allow exit (DM)",		{(cvar_t *)DF_NO_EXIT},				{1}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Infinite ammo",		{(cvar_t *)DF_INFINITE_AMMO},		{0}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"No monsters",			{(cvar_t *)DF_NO_MONSTERS},			{0}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Monsters respawn",		{(cvar_t *)DF_MONSTERS_RESPAWN},	{0}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Items respawn",		{(cvar_t *)DF_ITEMS_RESPAWN},		{0}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Fast monsters",		{(cvar_t *)DF_FAST_MONSTERS},		{0}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Allow jump",			{(cvar_t *)DF_NO_JUMP},				{1}, {0}, {0}, {(value_t *)&dmflags} },
	{ bitflag,	"Allow freelook",		{(cvar_t *)DF_NO_FREELOOK},			{1}, {0}, {0}, {(value_t *)&dmflags} },
	{ redtext,	" ",					{NULL},								{0.0}, {0.0},	{0.0}, {NULL} },
	{ slider,	"Friendly fire scale",	{&friendlyfire},					{0.0}, {1.0},	{0.05},{NULL} },
	{ discrete, "Teamplay",				{&teamplay},						{2.0}, {0.0},	{0.0}, {OnOff} }
};

static menu_t DMFlagsMenu = {
	{ 'M','_','G','M','P','L','A','Y' },
	"GAMEPLAY OPTIONS",
	0,
	19,
	0,
	DMFlagsItems,
};

void M_FreeValues (value_t **values, int num)
{
	int i;

	if (*values)
	{
		for (i = 0; i < num; i++)
		{
			if ((*values)[i].name)
				free ((*values)[i].name);
		}

		free (*values);
		*values = NULL;
	}
}

//
//		Set some stuff up for the video modes menu
//
static byte BitTranslate[16];

void M_OptInit (void)
{
	int currval = 0, dummy1, dummy2, i;
	char name[24];

	for (i = 1; i < 32; i++)
	{
		I_StartModeIterator (i);
		if (I_NextMode (&dummy1, &dummy2))
		{
			Depths[currval].value = currval;
			sprintf (name, "%d bit", i);
			Depths[currval].name = copystring (name);
			BitTranslate[currval] = i;
			currval++;
		}
	}

	ModesItems[VM_DEPTHITEM].b.min = (float)currval;

	switch (I_DisplayType ())
	{
	case DISPLAY_FullscreenOnly:
		ModesItems[2].type = nochoice;
		ModesItems[2].b.min = 1.f;
		break;
	case DISPLAY_WindowOnly:
		ModesItems[2].type = nochoice;
		ModesItems[2].b.min = 0.f;
		break;
	default:
		break;
	}

	if (gameinfo.gametype == GAME_Doom)
	{
		LabelColor = CR_UNTRANSLATED;
		ValueColor = CR_GRAY;
		MoreColor = CR_GRAY;
	}
	else if (gameinfo.gametype == GAME_Heretic)
	{
		LabelColor = CR_GREEN;
		ValueColor = CR_UNTRANSLATED;
		MoreColor = CR_UNTRANSLATED;
	}
	else // Hexen
	{
		LabelColor = CR_RED;
		ValueColor = CR_UNTRANSLATED;
		MoreColor = CR_UNTRANSLATED;
	}
}


//
//		Toggle messages on/off
//
void M_ChangeMessages (void)
{
	if (show_messages.value)
	{
		Printf (128, "%s\n", MSGOFF);
		show_messages.Set (0.0f);
	}
	else
	{
		Printf (128, "%s\n", MSGON);
		show_messages.Set (1.0f);
	}
}

BEGIN_COMMAND (togglemessages)
{
	M_ChangeMessages ();
}
END_COMMAND (togglemessages)

	void M_SizeDisplay (float diff)
{
	// changing screenblocks automatically resizes the display
	screenblocks.Set (screenblocks.value + diff);
}

BEGIN_COMMAND (sizedown)
{
	M_SizeDisplay (-1.0);
	S_Sound (CHAN_VOICE, "menu/change", 1, ATTN_NONE);
}
END_COMMAND (sizedown)

	BEGIN_COMMAND (sizeup)
{
	M_SizeDisplay(1.0);
	S_Sound (CHAN_VOICE, "menu/change", 1, ATTN_NONE);
}
END_COMMAND (sizeup)

void M_BuildKeyList (menuitem_t *item, int numitems)
{
	int i;

	for (i = 0; i < numitems; i++, item++)
	{
		if (item->type == control)
			C_GetKeysForCommand (item->e.command, &item->b.key1, &item->c.key2);
	}
}

void M_SwitchMenu (menu_t *menu)
{
	int i, widest = 0, thiswidth;
	menuitem_t *item;

	MenuStack[MenuStackDepth].menu.newmenu = menu;
	MenuStack[MenuStackDepth].isNewStyle = true;
	MenuStack[MenuStackDepth].drawSkull = false;
	MenuStackDepth++;

	CanScrollUp = false;
	CanScrollDown = false;
	CurrentMenu = menu;
	CurrentItem = menu->lastOn;

	if (menu == &ControlsMenu)
		I_ResumeMouse ();

	if (!menu->indent)
	{
		for (i = 0; i < menu->numitems; i++)
		{
			item = menu->items + i;
			if (item->type != whitetext && item->type != redtext)
			{
				thiswidth = screen->StringWidth (item->label);
				if (thiswidth > widest)
					widest = thiswidth;
			}
		}
		menu->indent = widest + 6;
	}

	flagsvar = NULL;
}

bool M_StartOptionsMenu (void)
{
	M_SwitchMenu (&OptionMenu);
	return true;
}

void M_DrawSlider (int x, int y, float min, float max, float cur)
{
	float range;

	range = max - min;

	if (cur > max)
		cur = max;
	else if (cur < min)
		cur = min;

	cur -= min;

	screen->SetFont (ConFont);
	screen->DrawTextCleanMove (CR_WHITE, x, y, "\x10\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x12");
	screen->DrawTextCleanMove (CR_ORANGE, x + 5 + (int)((cur * 78.f) / range), y, "\x13");
	screen->SetFont (SmallFont);
}

int M_FindCurVal (float cur, value_t *values, int numvals)
{
	int v;

	for (v = 0; v < numvals; v++)
		if (values[v].value == cur)
			break;

	return v;
}

void M_OptDrawer ()
{
	EColorRange color;
	int y, width, i, x, ytop;
	menuitem_t *item;

	i = W_CheckNumForName (CurrentMenu->title);
	if (gameinfo.gametype == GAME_Doom && i >= 0)
	{
		patch_t *title = (patch_t *)W_CacheLumpNum (i, PU_CACHE);
		screen->DrawPatchClean (title, 160-SHORT(title->width)/2, 10);
		y = 15 + title->height;
	}
	else if (BigFont && CurrentMenu->texttitle)
	{
		screen->SetFont (BigFont);
		screen->DrawTextCleanMove (CR_UNTRANSLATED,
			160-screen->StringWidth (CurrentMenu->texttitle)/2, 10,
			CurrentMenu->texttitle);
		screen->SetFont (SmallFont);
		y = 15 + BigFont->GetHeight ();
	}
	else
	{
		y = 15;
	}

	ytop = y + CurrentMenu->scrolltop * 8;
	for (i = 0; i < CurrentMenu->numitems &&
		y <= 200 - SmallFont->GetHeight (); i++, y += 8)
	{
		if (i == CurrentMenu->scrolltop)
		{
			i += CurrentMenu->scrollpos;
		}

		item = CurrentMenu->items + i;

		if (item->type != screenres)
		{
			width = screen->StringWidth (item->label);
			switch (item->type)
			{
			case more:
				x = CurrentMenu->indent - width;
				color = MoreColor;
				break;

			case redtext:
				x = 160 - width / 2;
				color = LabelColor;
				break;

			case whitetext:
				x = 160 - width / 2;
				color = ValueColor;
				break;

			case listelement:
				x = CurrentMenu->indent + 14;
				color = LabelColor;
				break;

			default:
				x = CurrentMenu->indent - width;
				color = (item->type == control && WaitingForKey && i == CurrentItem)
					? CR_YELLOW : LabelColor;
				break;
			}
			screen->DrawTextCleanMove (color, x, y, item->label);

			switch (item->type)
			{
			case discrete:
			case cdiscrete:
			{
				int v, vals;

				vals = (int)item->b.min;
				v = M_FindCurVal (item->a.cvar->value, item->e.values, vals);

				if (v == vals)
				{
					screen->DrawTextCleanMove (ValueColor, CurrentMenu->indent + 14, y, "Unknown");
				}
				else
				{
					if (item->type == cdiscrete)
						screen->DrawTextCleanMove (v, CurrentMenu->indent + 14, y, item->e.values[v].name);
					else
						screen->DrawTextCleanMove (ValueColor, CurrentMenu->indent + 14, y, item->e.values[v].name);
				}

			}
			break;

			case nochoice:
				screen->DrawTextCleanMove (CR_GOLD, CurrentMenu->indent + 14, y,
										   (item->e.values[(int)item->b.min]).name);
				break;

			case slider:
				M_DrawSlider (CurrentMenu->indent + 14, y, item->b.min, item->c.max, item->a.cvar->value);
				break;

			case control:
			{
				char description[64];

				C_NameKeys (description, item->b.key1, item->c.key2);
				screen->SetFont (ConFont);
				screen->DrawTextCleanMove (CR_WHITE,
					CurrentMenu->indent + 14, y, description);
				screen->SetFont (SmallFont);
			}
			break;

			case bitflag:
			{
				value_t *value;
				char *str;

				if (item->b.min)
					value = NoYes;
				else
					value = YesNo;

				if (*item->e.flagint & item->a.flagmask)
					str = value[1].name;
				else
					str = value[0].name;

				screen->DrawTextCleanMove (ValueColor,
					CurrentMenu->indent + 14, y, str);
			}
			break;

			default:
				break;
			}

			if (i == CurrentItem && (skullAnimCounter < 6 || WaitingForKey))
			{
				screen->SetFont (ConFont);
				screen->DrawTextCleanMove (CR_RED, CurrentMenu->indent + 3, y, "\xd");
				screen->SetFont (SmallFont);
			}
		}
		else
		{
			char *str = NULL;

			for (x = 0; x < 3; x++)
			{
				switch (x)
				{
				case 0:
					str = item->b.res1;
					break;
				case 1:
					str = item->c.res2;
					break;
				case 2:
					str = item->d.res3;
					break;
				}
				if (str)
				{
					if (x == item->e.highlight)
						color = ValueColor;
					else
						color = LabelColor;

					screen->DrawTextCleanMove (color, 104 * x + 20, y, str);
				}
			}

			if (i == CurrentItem && ((item->a.selmode != -1 && (skullAnimCounter < 6 || WaitingForKey)) || testingmode))
			{
				screen->SetFont (ConFont);
				screen->DrawTextCleanMove (CR_RED, item->a.selmode * 104 + 8, y, "\xd");
				screen->SetFont (SmallFont);
			}
		}
	}

	CanScrollUp = (CurrentMenu->scrollpos != 0);
	CanScrollDown = (i < CurrentMenu->numitems);
	VisBottom = i - 1;

	screen->SetFont (ConFont);
	if (CanScrollUp)
	{
		screen->DrawTextCleanMove (CR_ORANGE, 3, ytop, "\x1a");
	}
	if (CanScrollDown)
	{
		screen->DrawTextCleanMove (CR_ORANGE, 3, y - 8, "\x1b");
	}
	screen->SetFont (SmallFont);

	if (flagsvar)
	{
		char flagsblah[64];

		sprintf (flagsblah, "%s = %s", flagsvar->name, flagsvar->string);
		screen->DrawTextCleanMove (ValueColor,
			160 - (screen->StringWidth (flagsblah) >> 1), 190, flagsblah);
	}
}

void M_OptResponder (event_t *ev)
{
	menuitem_t *item;
	int ch = ev->data1;
	
	item = CurrentMenu->items + CurrentItem;

	if (WaitingForKey)
	{
		if (ch != KEY_ESCAPE)
		{
			C_ChangeBinding (item->e.command, ch);
			M_BuildKeyList (CurrentMenu->items, CurrentMenu->numitems);
		}
		WaitingForKey = false;
		CurrentMenu->items[0].label = OldMessage;
		CurrentMenu->items[0].type = OldType;
		return;
	}

	if (item->type == bitflag && flagsvar &&
		(ch == KEY_LEFTARROW || ch == KEY_RIGHTARROW || ch == KEY_ENTER)
		&& !demoplayback)
	{
		int newflags = *item->e.flagint ^ item->a.flagmask;
		char val[16];

		sprintf (val, "%d", newflags);
		flagsvar->Set (val);
		return;
	}

	switch (ch)
	{
	case KEY_DOWNARROW:
		{
			int modecol;

			if (item->type == screenres)
			{
				modecol = item->a.selmode;
				item->a.selmode = -1;
			}
			else
			{
				modecol = 0;
			}

			do
			{
				CurrentItem++;
				if (CanScrollDown && CurrentItem == VisBottom)
				{
					CurrentMenu->scrollpos++;
					VisBottom++;
				}
				if (CurrentItem == CurrentMenu->numitems)
				{
					CurrentMenu->scrollpos = 0;
					CurrentItem = 0;
				}
			} while (CurrentMenu->items[CurrentItem].type == redtext ||
					 CurrentMenu->items[CurrentItem].type == whitetext ||
					 (CurrentMenu->items[CurrentItem].type == screenres &&
					  !CurrentMenu->items[CurrentItem].b.res1));

			if (CurrentMenu->items[CurrentItem].type == screenres)
				CurrentMenu->items[CurrentItem].a.selmode = modecol;

			S_Sound (CHAN_VOICE, "menu/cursor", 1, ATTN_NONE);
		}
		break;

	case KEY_UPARROW:
		{
			int modecol;

			if (item->type == screenres)
			{
				modecol = item->a.selmode;
				item->a.selmode = -1;
			}
			else
			{
				modecol = 0;
			}

			do
			{
				CurrentItem--;
				if (CanScrollUp &&
					CurrentItem == CurrentMenu->scrolltop + CurrentMenu->scrollpos)
				{
					CurrentMenu->scrollpos--;
				}
				if (CurrentItem < 0)
				{
					CurrentMenu->scrollpos = MAX (0,CurrentMenu->numitems - 22 + CurrentMenu->scrolltop);
					CurrentItem = CurrentMenu->numitems - 1;
				}
			} while (CurrentMenu->items[CurrentItem].type == redtext ||
					 CurrentMenu->items[CurrentItem].type == whitetext ||
					 (CurrentMenu->items[CurrentItem].type == screenres &&
					  !CurrentMenu->items[CurrentItem].b.res1));

			if (CurrentMenu->items[CurrentItem].type == screenres)
				CurrentMenu->items[CurrentItem].a.selmode = modecol;

			S_Sound (CHAN_VOICE, "menu/cursor", 1, ATTN_NONE);
		}
		break;

	case KEY_LEFTARROW:
		switch (item->type)
		{
			case slider:
				{
					float newval = item->a.cvar->value - item->d.step;

					if (newval < item->b.min)
						newval = item->b.min;

					if (item->e.cfunc)
						item->e.cfunc (item->a.cvar, newval);
					else
						item->a.cvar->Set (newval);
				}
				S_Sound (CHAN_VOICE, "menu/change", 1, ATTN_NONE);
				break;

			case discrete:
			case cdiscrete:
				{
					int cur;
					int numvals;

					numvals = (int)item->b.min;
					cur = M_FindCurVal (item->a.cvar->value, item->e.values, numvals);
					if (--cur < 0)
						cur = numvals - 1;

					item->a.cvar->Set (item->e.values[cur].value);

					// Hack hack. Rebuild list of resolutions
					if (item->e.values == Depths)
						BuildModesList (screen->width, screen->height, DisplayBits);
				}
				S_Sound (CHAN_VOICE, "menu/change", 1, ATTN_NONE);
				break;

			case screenres:
				{
					int col;

					col = item->a.selmode - 1;
					if (col < 0)
					{
						if (CurrentItem > 0)
						{
							if (CurrentMenu->items[CurrentItem - 1].type == screenres)
							{
								item->a.selmode = -1;
								CurrentMenu->items[--CurrentItem].a.selmode = 2;
							}
						}
					}
					else
					{
						item->a.selmode = col;
					}
				}
				S_Sound (CHAN_VOICE, "menu/cursor", 1, ATTN_NONE);
				break;

			default:
				break;
		}
		break;

	case KEY_RIGHTARROW:
		switch (item->type)
		{
			case slider:
				{
					float newval = item->a.cvar->value + item->d.step;

					if (newval > item->c.max)
						newval = item->c.max;

					if (item->e.cfunc)
						item->e.cfunc (item->a.cvar, newval);
					else
						item->a.cvar->Set (newval);
				}
				S_Sound (CHAN_VOICE, "menu/change", 1, ATTN_NONE);
				break;

			case discrete:
			case cdiscrete:
				{
					int cur;
					int numvals;

					numvals = (int)item->b.min;
					cur = M_FindCurVal (item->a.cvar->value, item->e.values, numvals);
					if (++cur >= numvals)
						cur = 0;

					item->a.cvar->Set (item->e.values[cur].value);

					// Hack hack. Rebuild list of resolutions
					if (item->e.values == Depths)
						BuildModesList (screen->width, screen->height, DisplayBits);
				}
				S_Sound (CHAN_VOICE, "menu/change", 1, ATTN_NONE);
				break;

			case screenres:
				{
					int col;

					col = item->a.selmode + 1;
					if ((col > 2) || (col == 2 && !item->d.res3) || (col == 1 && !item->c.res2))
					{
						if (CurrentMenu->numitems - 1 > CurrentItem)
						{
							if (CurrentMenu->items[CurrentItem + 1].type == screenres)
							{
								if (CurrentMenu->items[CurrentItem + 1].b.res1)
								{
									item->a.selmode = -1;
									CurrentMenu->items[++CurrentItem].a.selmode = 0;
								}
							}
						}
					}
					else
					{
						item->a.selmode = col;
					}
				}
				S_Sound (CHAN_VOICE, "menu/cursor", 1, ATTN_NONE);
				break;

			default:
				break;
		}
		break;

	case KEY_BACKSPACE:
		if (item->type == control)
		{
			C_UnbindACommand (item->e.command);
			item->b.key1 = item->c.key2 = 0;
		}
		break;

	case KEY_ENTER:
		if (CurrentMenu == &ModesMenu)
		{
			if (!(item->type == screenres && GetSelectedSize (CurrentItem, &NewWidth, &NewHeight)))
			{
				NewWidth = screen->width;
				NewHeight = screen->height;
			}
			NewBits = BitTranslate[(int)DummyDepthCvar.value];
			setmodeneeded = true;
			S_Sound (CHAN_VOICE, "menu/choose", 1, ATTN_NONE);
			SetModesMenu (NewWidth, NewHeight, NewBits);
		}
		else if (item->type == more && item->e.mfunc)
		{
			CurrentMenu->lastOn = CurrentItem;
			S_Sound (CHAN_VOICE, "menu/choose", 1, ATTN_NONE);
			item->e.mfunc();
		}
		else if (item->type == discrete || item->type == cdiscrete)
		{
			int cur;
			int numvals;

			numvals = (int)item->b.min;
			cur = M_FindCurVal (item->a.cvar->value, item->e.values, numvals);
			if (++cur >= numvals)
				cur = 0;

			item->a.cvar->Set (item->e.values[cur].value);

			// Hack hack. Rebuild list of resolutions
			if (item->e.values == Depths)
				BuildModesList (screen->width, screen->height, DisplayBits);

			S_Sound (CHAN_VOICE, "menu/change", 1, ATTN_NONE);
		}
		else if (item->type == control)
		{
			WaitingForKey = true;
			OldMessage = CurrentMenu->items[0].label;
			OldType = CurrentMenu->items[0].type;
			CurrentMenu->items[0].label = "Press new key for control, ESC to cancel";
			CurrentMenu->items[0].type = redtext;
		}
		else if (item->type == listelement)
		{
			CurrentMenu->lastOn = CurrentItem;
			S_Sound (CHAN_VOICE, "menu/choose", 1, ATTN_NONE);
			item->e.lfunc (CurrentItem);
		}
		else if (item->type == screenres)
		{
		}
		break;

	case KEY_ESCAPE:
		CurrentMenu->lastOn = CurrentItem;
		M_PopMenuStack ();
		break;

	default:
		if (ev->data2 == 't')
		{
			// Test selected resolution
			if (CurrentMenu == &ModesMenu)
			{
				if (!(item->type == screenres &&
					GetSelectedSize (CurrentItem, &NewWidth, &NewHeight)))
				{
					NewWidth = screen->width;
					NewHeight = screen->height;
				}
				OldWidth = screen->width;
				OldHeight = screen->height;
				OldBits = DisplayBits;
				NewBits = BitTranslate[(int)DummyDepthCvar.value];
				setmodeneeded = true;
				testingmode = I_GetTime() + 5 * TICRATE;
				S_Sound (CHAN_VOICE, "menu/choose", 1, ATTN_NONE);
				SetModesMenu (NewWidth, NewHeight, NewBits);
			}
		}
		else if (ev->data2 == 'd' && CurrentMenu == &ModesMenu)
		{
			// Make current resolution the default
			vid_defwidth.Set ((float)screen->width);
			vid_defheight.Set ((float)screen->height);
			vid_defbits.Set ((float)DisplayBits);
			SetModesMenu (screen->width, screen->height, DisplayBits);
		}
		break;
	}
}

static void GoToConsole (void)
{
	M_ClearMenus ();
	C_ToggleConsole ();
}

static void UpdateStuff (void)
{
	M_SizeDisplay (0.0);
}

void Reset2Defaults (void)
{
	AddCommandString ("unbindall; binddefaults");
	C_SetCVarsToDefaults ();
	UpdateStuff();
}

void Reset2Saved (void)
{
	char *execcommand;
	char *configfile = GetConfigPath ();

	execcommand = new char[strlen (configfile) + 8];
	sprintf (execcommand, "exec \"%s\"", configfile);
	delete[] configfile;
	AddCommandString (execcommand);
	delete[] execcommand;
	UpdateStuff();
}

static void StartMessagesMenu (void)
{
	M_SwitchMenu (&MessagesMenu);
}

static void CustomizeControls (void)
{
	M_BuildKeyList (ControlsMenu.items, ControlsMenu.numitems);
	M_SwitchMenu (&ControlsMenu);
}

BEGIN_COMMAND (menu_keys)
{
	M_StartControlPanel (true);
	OptionsActive = true;
	CustomizeControls ();
}
END_COMMAND (menu_keys)

EXTERN_CVAR (dmflagsvar)

static void GameplayOptions (void)
{
	M_SwitchMenu (&DMFlagsMenu);
	flagsvar = &dmflagsvar;
}

BEGIN_COMMAND (menu_gameplay)
{
	M_StartControlPanel (true);
	OptionsActive = true;
	GameplayOptions ();
}
END_COMMAND (menu_gameplay)

static void VideoOptions (void)
{
	M_SwitchMenu (&VideoMenu);
}

BEGIN_COMMAND (menu_display)
{
	M_StartControlPanel (true);
	OptionsActive = true;
	M_SwitchMenu (&VideoMenu);
}
END_COMMAND (menu_display)

static void BuildModesList (int hiwidth, int hiheight, int hi_bits)
{
	char strtemp[32], **str;
	int	 i, c;
	int	 width, height, showbits;

	showbits = BitTranslate[(int)DummyDepthCvar.value];

	I_StartModeIterator (showbits);

	for (i = VM_RESSTART; ModesItems[i].type == screenres; i++)
	{
		ModesItems[i].e.highlight = -1;
		for (c = 0; c < 3; c++)
		{
			switch (c)
			{
				case 0:
					str = &ModesItems[i].b.res1;
					break;
				case 1:
					str = &ModesItems[i].c.res2;
					break;
				case 2:
					str = &ModesItems[i].d.res3;
					break;
			}
			if (I_NextMode (&width, &height))
			{
				if (/* hi_bits == showbits && */ width == hiwidth && height == hiheight)
					ModesItems[i].e.highlight = ModesItems[i].a.selmode = c;
				
				sprintf (strtemp, "%dx%d", width, height);
				ReplaceString (str, strtemp);
			}
			else
			{
				if (*str)
				{
					free (*str);
					*str = NULL;
				}
			}
		}
	}
}

void M_RefreshModesList ()
{
	BuildModesList (screen->width, screen->height, DisplayBits);
}

static BOOL GetSelectedSize (int line, int *width, int *height)
{
	int i, stopat;

	if (ModesItems[line].type != screenres)
	{
		return false;
	}
	else
	{
		I_StartModeIterator (BitTranslate[(int)DummyDepthCvar.value]);

		stopat = (line - VM_RESSTART) * 3 + ModesItems[line].a.selmode + 1;

		for (i = 0; i < stopat; i++)
			if (!I_NextMode (width, height))
				return false;
	}

	return true;
}

static int FindBits (int bits)
{
	int i;

	for (i = 0; i < 22; i++)
	{
		if (BitTranslate[i] == bits)
			return i;
	}

	return 0;
}

static void SetModesMenu (int w, int h, int bits)
{
	char strtemp[64];

	DummyDepthCvar.Set ((float)FindBits (bits));

	if (!testingmode)
	{
		if (ModesItems[VM_ENTERLINE].label != VMEnterText)
			free (ModesItems[VM_ENTERLINE].label);
		ModesItems[VM_ENTERLINE].label = VMEnterText;
		ModesItems[VM_TESTLINE].label = VMTestText;

		sprintf (strtemp, "D to make %dx%dx%d the default", w, h, bits);
		ReplaceString (&ModesItems[VM_MAKEDEFLINE].label, strtemp);

		sprintf (strtemp, "Current default is %dx%dx%d",
				 (int)vid_defwidth.value,
				 (int)vid_defheight.value,
				 (int)vid_defbits.value);
		ReplaceString (&ModesItems[VM_CURDEFLINE].label, strtemp);
	}
	else
	{
		sprintf (strtemp, "TESTING %dx%dx%d", w, h, bits);
		ModesItems[VM_ENTERLINE].label = copystring (strtemp);
		ModesItems[VM_TESTLINE].label = "Please wait 5 seconds...";
		ModesItems[VM_MAKEDEFLINE].label = copystring (" ");
		ModesItems[VM_CURDEFLINE].label = copystring (" ");
	}

	BuildModesList (w, h, bits);
}

void M_RestoreMode (void)
{
	NewWidth = OldWidth;
	NewHeight = OldHeight;
	NewBits = OldBits;
	setmodeneeded = true;
	testingmode = 0;
	SetModesMenu (OldWidth, OldHeight, OldBits);
}

static void SetVidMode (void)
{
	SetModesMenu (screen->width, screen->height, DisplayBits);
	if (ModesMenu.items[ModesMenu.lastOn].type == screenres)
	{
		if (ModesMenu.items[ModesMenu.lastOn].a.selmode == -1)
		{
			ModesMenu.items[ModesMenu.lastOn].a.selmode++;
		}
	}
	M_SwitchMenu (&ModesMenu);
}

BEGIN_COMMAND (menu_video)
{
	M_StartControlPanel (true);
	OptionsActive = true;
	SetVidMode ();
}
END_COMMAND (menu_video)
