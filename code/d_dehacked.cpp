#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stddef.h>

#include "doomtype.h"
#include "doomstat.h"
#include "info.h"
#include "d_dehacked.h"
#include "s_sound.h"
#include "d_items.h"
#include "g_level.h"
#include "m_cheat.h"
#include "cmdlib.h"
#include "dstrings.h"
#include "m_alloc.h"
#include "m_misc.h"
#include "w_wad.h"
#include "d_player.h"
#include "r_state.h"
#include "gi.h"

extern int clipammo[NUMAMMO];

// Available action functions
void A_FireRailgun(player_s*, pspdef_t*);
void A_FireRailgunLeft(player_s*, pspdef_t*);
void A_FireRailgunRight(player_s*, pspdef_t*);
void A_RailWait(player_s*, pspdef_t*);
void A_Light0(player_s*, pspdef_t*);
void A_WeaponReady(player_s*, pspdef_t*);
void A_Lower(player_s*, pspdef_t*);
void A_Raise(player_s*, pspdef_t*);
void A_Punch(player_s*, pspdef_t*);
void A_ReFire(player_s*, pspdef_t*);
void A_FirePistol(player_s*, pspdef_t*);
void A_Light1(player_s*, pspdef_t*);
void A_FireShotgun(player_s*, pspdef_t*);
void A_Light2(player_s*, pspdef_t*);
void A_FireShotgun2(player_s*, pspdef_t*);
void A_CheckReload(player_s*, pspdef_t*);
void A_OpenShotgun2(player_s*, pspdef_t*);
void A_LoadShotgun2(player_s*, pspdef_t*);
void A_CloseShotgun2(player_s*, pspdef_t*);
void A_FireCGun(player_s*, pspdef_t*);
void A_GunFlash(player_s*, pspdef_t*);
void A_FireMissile(player_s*, pspdef_t*);
void A_Saw(player_s*, pspdef_t*);
void A_FirePlasma(player_s*, pspdef_t*);
void A_BFGsound(player_s*, pspdef_t*);
void A_FireBFG(player_s*, pspdef_t*);
void A_BFGSpray(AActor*);
void A_Explode(AActor*);
void A_Pain(AActor*);
void A_PlayerScream(AActor*);
void A_NoBlocking(AActor*);
void A_XScream(AActor*);
void A_Look(AActor*);
void A_Chase(AActor*);
void A_FaceTarget(AActor*);
void A_PosAttack(AActor*);
void A_Scream(AActor*);
void A_SPosAttack(AActor*);
void A_VileChase(AActor*);
void A_VileStart(AActor*);
void A_VileTarget(AActor*);
void A_VileAttack(AActor*);
void A_StartFire(AActor*);
void A_Fire(AActor*);
void A_FireCrackle(AActor*);
void A_Tracer(AActor*);
void A_SkelWhoosh(AActor*);
void A_SkelFist(AActor*);
void A_SkelMissile(AActor*);
void A_FatRaise(AActor*);
void A_FatAttack1(AActor*);
void A_FatAttack2(AActor*);
void A_FatAttack3(AActor*);
void A_BossDeath(AActor*);
void A_CPosAttack(AActor*);
void A_CPosRefire(AActor*);
void A_TroopAttack(AActor*);
void A_SargAttack(AActor*);
void A_HeadAttack(AActor*);
void A_BruisAttack(AActor*);
void A_SkullAttack(AActor*);
void A_Metal(AActor*);
void A_SpidRefire(AActor*);
void A_BabyMetal(AActor*);
void A_BspiAttack(AActor*);
void A_Hoof(AActor*);
void A_CyberAttack(AActor*);
void A_PainAttack(AActor*);
void A_PainDie(AActor*);
void A_KeenDie(AActor*);
void A_BrainPain(AActor*);
void A_BrainScream(AActor*);
void A_BrainDie(AActor*);
void A_BrainAwake(AActor*);
void A_BrainSpit(AActor*);
void A_SpawnSound(AActor*);
void A_SpawnFly(AActor*);
void A_BrainExplode(AActor*);
void A_Die(AActor*);
void A_Detonate(AActor*);
void A_Mushroom(AActor*);
void A_MonsterRail(AActor*);

// Miscellaneous info that used to be constant
struct DehInfo deh = {
	100,	// .StartHealth
	 50,	// .StartBullets
	100,	// .MaxHealth
	200,	// .MaxArmor
	  1,	// .GreenAC
	  2,	// .BlueAC
	200,	// .MaxSoulsphere
	100,	// .SoulsphereHealth
	200,	// .MegasphereHealth
	100,	// .GodHealth
	200,	// .FAArmor
	  2,	// .FAAC
	200,	// .KFAArmor
	  2,	// .KFAAC
	  0,	// .Infight
};

// These are the original heights of every Doom 2 thing. They are used if a patch
// specifies that a thing should be hanging from the ceiling but doesn't specify
// a height for the thing, since these are the heights it probably wants.

static const byte OrgHeights[] = {
	56, 56, 56, 56, 16, 56, 8, 16, 64, 8, 56, 56,
	56, 56, 56, 64, 8, 64, 56, 100, 64, 110, 56, 56,
	72, 16, 32, 32, 32, 16, 42, 8, 8, 8,
	8, 8, 8, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 68, 84, 84,
	68, 52, 84, 68, 52, 52, 68, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 88, 88, 64, 64, 64, 64,
	16, 16, 16
};

#define LINESIZE 2048

#define CHECKKEY(a,b)		if (!stricmp (Line1, (a))) (b) = atoi(Line2);

static char *PatchFile, *PatchPt;
static char *Line1, *Line2;
static int	 dversion, pversion;
static BOOL  including, includenotext;

static const char *unknown_str = "Unknown key %s encountered in %s %d.\n";

// This is an offset to be used for computing the text stuff.
// Straight from the DeHackEd source which was
// Written by Greg Lewis, gregl@umich.edu.
static int toff[] = {129044, 129044, 129044, 129284, 129380};

// This is a list of all the action functions used by each of Doom's states.
#define NUMACTIONS 812
static actionf_t Actions[NUMACTIONS] =
{
	{NULL}, {A_Light0}, {A_WeaponReady}, {A_Lower}, {A_Raise}, {NULL},
	{A_Punch}, {NULL}, {NULL}, {A_ReFire}, {A_WeaponReady}, {A_Lower},
	{A_Raise}, {NULL}, {A_FirePistol}, {NULL}, {A_ReFire}, {A_Light1},
	{A_WeaponReady}, {A_Lower}, {A_Raise}, {NULL}, {A_FireShotgun}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {A_ReFire}, {A_Light1},
	{A_Light2}, {A_WeaponReady}, {A_Lower}, {A_Raise}, {NULL},
	{A_FireShotgun2}, {NULL}, {A_CheckReload}, {A_OpenShotgun2}, {NULL},
	{A_LoadShotgun2}, {NULL}, {A_CloseShotgun2}, {A_ReFire}, {NULL}, {NULL},
	{A_Light1}, {A_Light2}, {A_WeaponReady}, {A_Lower}, {A_Raise},
	{A_FireCGun}, {A_FireCGun}, {A_ReFire}, {A_Light1}, {A_Light2},
	{A_WeaponReady}, {A_Lower}, {A_Raise}, {A_GunFlash}, {A_FireMissile},
	{A_ReFire}, {A_Light1}, {NULL}, {A_Light2}, {A_Light2}, {A_WeaponReady},
	{A_WeaponReady}, {A_Lower}, {A_Raise}, {A_Saw}, {A_Saw}, {A_ReFire},
	{A_WeaponReady}, {A_Lower}, {A_Raise}, {A_FirePlasma}, {A_ReFire},
	{A_Light1}, {A_Light1}, {A_WeaponReady}, {A_Lower}, {A_Raise},
	{A_BFGsound}, {A_GunFlash}, {A_FireBFG}, {A_ReFire}, {A_Light1},
	{A_Light2}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {A_BFGSpray}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {A_Explode}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{A_Pain}, {NULL}, {A_PlayerScream}, {A_NoBlocking}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {A_XScream}, {A_NoBlocking}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {A_Look}, {A_Look}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_FaceTarget},
	{A_PosAttack}, {NULL}, {NULL}, {A_Pain}, {NULL}, {A_Scream}, {A_NoBlocking},
	{NULL}, {NULL}, {NULL}, {A_XScream}, {A_NoBlocking}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {A_Look},
	{A_Look}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_FaceTarget}, {A_SPosAttack}, {NULL},
	{NULL}, {A_Pain}, {NULL}, {A_Scream}, {A_NoBlocking}, {NULL}, {NULL}, {NULL},
	{A_XScream}, {A_NoBlocking}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {A_Look}, {A_Look},
	{A_VileChase}, {A_VileChase}, {A_VileChase}, {A_VileChase},
	{A_VileChase}, {A_VileChase}, {A_VileChase}, {A_VileChase},
	{A_VileChase}, {A_VileChase}, {A_VileChase}, {A_VileChase},
	{A_VileStart}, {A_FaceTarget}, {A_VileTarget}, {A_FaceTarget},
	{A_FaceTarget}, {A_FaceTarget}, {A_FaceTarget}, {A_FaceTarget},
	{A_FaceTarget}, {A_VileAttack}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{A_Pain}, {NULL}, {A_Scream}, {A_NoBlocking}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {A_StartFire}, {A_Fire}, {A_Fire}, {A_Fire},
	{A_FireCrackle}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire},
	{A_Fire}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire},
	{A_Fire}, {A_FireCrackle}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire},
	{A_Fire}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {A_Tracer}, {A_Tracer}, {NULL},
	{NULL}, {NULL}, {A_Look}, {A_Look}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_FaceTarget}, {A_SkelWhoosh},
	{A_FaceTarget}, {A_SkelFist}, {A_FaceTarget}, {A_FaceTarget},
	{A_SkelMissile}, {A_FaceTarget}, {NULL}, {A_Pain}, {NULL}, {NULL},
	{A_Scream}, {A_NoBlocking}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {A_Look},
	{A_Look}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_FatRaise}, {A_FatAttack1}, {A_FaceTarget}, {A_FaceTarget},
	{A_FatAttack2}, {A_FaceTarget}, {A_FaceTarget}, {A_FatAttack3},
	{A_FaceTarget}, {A_FaceTarget}, {NULL}, {A_Pain}, {NULL}, {A_Scream},
	{A_NoBlocking}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {A_BossDeath},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{A_Look}, {A_Look}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_FaceTarget},
	{A_CPosAttack}, {A_CPosAttack}, {A_CPosRefire}, {NULL}, {A_Pain},
	{NULL}, {A_Scream}, {A_NoBlocking}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{A_XScream}, {A_NoBlocking}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {A_Look}, {A_Look}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_FaceTarget}, {A_FaceTarget}, {A_TroopAttack}, {NULL},
	{A_Pain}, {NULL}, {A_Scream}, {NULL}, {A_NoBlocking}, {NULL}, {NULL},
	{A_XScream}, {NULL}, {A_NoBlocking}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {A_Look}, {A_Look}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_FaceTarget}, {A_FaceTarget}, {A_SargAttack}, {NULL},
	{A_Pain}, {NULL}, {A_Scream}, {NULL}, {A_NoBlocking}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {A_Look}, {A_Chase},
	{A_FaceTarget}, {A_FaceTarget}, {A_HeadAttack}, {NULL}, {A_Pain},
	{NULL}, {NULL}, {A_Scream}, {NULL}, {NULL}, {A_NoBlocking}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {A_Look}, {A_Look}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_FaceTarget},
	{A_FaceTarget}, {A_BruisAttack}, {NULL}, {A_Pain}, {NULL}, {A_Scream},
	{NULL}, {A_NoBlocking}, {NULL}, {NULL}, {A_BossDeath}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {A_Look}, {A_Look}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_FaceTarget}, {A_FaceTarget}, {A_BruisAttack}, {NULL},
	{A_Pain}, {NULL}, {A_Scream}, {NULL}, {A_NoBlocking}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {A_Look},
	{A_Look}, {A_Chase}, {A_Chase}, {A_FaceTarget}, {A_SkullAttack}, {NULL},
	{NULL}, {NULL}, {A_Pain}, {NULL}, {A_Scream}, {NULL}, {A_NoBlocking}, {NULL},
	{NULL}, {A_Look}, {A_Look}, {A_Metal}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Metal}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Metal}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_FaceTarget}, {A_SPosAttack}, {A_SPosAttack},
	{A_SpidRefire}, {NULL}, {A_Pain}, {A_Scream}, {A_NoBlocking}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {A_BossDeath}, {A_Look},
	{A_Look}, {NULL}, {A_BabyMetal}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_BabyMetal}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_FaceTarget}, {A_BspiAttack}, {NULL},
	{A_SpidRefire}, {NULL}, {A_Pain}, {A_Scream}, {A_NoBlocking}, {NULL}, {NULL},
	{NULL}, {NULL}, {A_BossDeath}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{A_Look}, {A_Look}, {A_Hoof}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Metal}, {A_Chase}, {A_FaceTarget},
	{A_CyberAttack}, {A_FaceTarget}, {A_CyberAttack}, {A_FaceTarget},
	{A_CyberAttack}, {A_Pain}, {NULL}, {A_Scream}, {NULL}, {NULL}, {NULL},
	{A_NoBlocking}, {NULL}, {NULL}, {NULL}, {A_BossDeath}, {A_Look}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_FaceTarget},
	{A_FaceTarget}, {A_FaceTarget}, {A_PainAttack}, {NULL}, {A_Pain},
	{NULL}, {A_Scream}, {NULL}, {NULL}, {A_PainDie}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {A_Look}, {A_Look}, {A_Chase},
	{A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase}, {A_Chase},
	{A_Chase}, {A_FaceTarget}, {A_FaceTarget}, {A_CPosAttack},
	{A_FaceTarget}, {A_CPosAttack}, {A_CPosRefire}, {NULL}, {A_Pain},
	{NULL}, {A_Scream}, {A_NoBlocking}, {NULL}, {NULL}, {NULL}, {A_XScream},
	{A_NoBlocking}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {A_Scream},
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {A_KeenDie},
	{NULL}, {NULL}, {A_Pain}, {NULL}, {A_BrainPain}, {A_BrainScream},
	{NULL}, {NULL}, {A_BrainDie}, {A_Look}, {A_BrainAwake}, {A_BrainSpit},
	{A_SpawnSound}, {A_SpawnFly}, {A_SpawnFly}, {A_SpawnFly}, {A_Fire},
	{A_Fire}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire}, {A_Fire},
	{NULL}, {NULL}, {A_BrainExplode}, {NULL}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {NULL}, {A_Scream}, {NULL}, {A_Explode}
};

// DeHackEd made the erroneous assumption that if a state didn't appear in
// Doom with an action function, then it was incorrect to assign it one.
// This is a list of the states that had action functions, so we can figure
// out where in the original list of states a DeHackEd codepointer is.
// (DeHackEd might also have done this for compatibility between Doom
// versions, because states could move around, but actions would never
// disappear, but that doesn't explain why frame patches specify an exact
// state rather than a code pointer.)
#define NUMCODEP 448
static short CodePConv[NUMCODEP] =
{
	  1,   2,   3,   4,   6,   9,  10,  11,  12,  14,
	 16,  17,  18,  19,  20,  22,  29,  30,  31,  32,
	 33,  34,  36,  38,  39,  41,  43,  44,  47,  48,
	 49,  50,  51,  52,  53,  54,  55,  56,  57,  58,
	 59,  60,  61,  62,  63,  65,  66,  67,  68,  69,
	 70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
	119, 127, 157, 159, 160, 166, 167, 174, 175, 176,
	177, 178, 179, 180, 181, 182, 183, 184, 185, 188,
	190, 191, 195, 196, 207, 208, 209, 210, 211, 212,
	213, 214, 215, 216, 217, 218, 221, 223, 224, 228,
	229, 241, 242, 243, 244, 245, 246, 247, 248, 249,
	250, 251, 252, 253, 254, 255, 256, 257, 258, 259,
	260, 261, 262, 263, 264, 270, 272, 273, 281, 282,
	283, 284, 285, 286, 287, 288, 289, 290, 291, 292,
	293, 294, 295, 296, 297, 298, 299, 300, 301, 302,
	303, 304, 305, 306, 307, 308, 309, 310, 316, 317,
	321, 322, 323, 324, 325, 326, 327, 328, 329, 330,
	331, 332, 333, 334, 335, 336, 337, 338, 339, 340,
	341, 342, 344, 347, 348, 362, 363, 364, 365, 366,
	367, 368, 369, 370, 371, 372, 373, 374, 375, 376,
	377, 378, 379, 380, 381, 382, 383, 384, 385, 387,
	389, 390, 397, 406, 407, 408, 409, 410, 411, 412,
	413, 414, 415, 416, 417, 418, 419, 421, 423, 424,
	430, 431, 442, 443, 444, 445, 446, 447, 448, 449,
	450, 451, 452, 453, 454, 456, 458, 460, 463, 465,
	475, 476, 477, 478, 479, 480, 481, 482, 483, 484,
	485, 486, 487, 489, 491, 493, 502, 503, 504, 505,
	506, 508, 511, 514, 527, 528, 529, 530, 531, 532,
	533, 534, 535, 536, 537, 538, 539, 541, 543, 545,
	548, 556, 557, 558, 559, 560, 561, 562, 563, 564,
	565, 566, 567, 568, 570, 572, 574, 585, 586, 587,
	588, 589, 590, 594, 596, 598, 601, 602, 603, 604,
	605, 606, 607, 608, 609, 610, 611, 612, 613, 614,
	615, 616, 617, 618, 620, 621, 622, 631, 632, 633,
	635, 636, 637, 638, 639, 640, 641, 642, 643, 644,
	645, 646, 647, 648, 650, 652, 653, 654, 659, 674,
	675, 676, 677, 678, 679, 680, 681, 682, 683, 684,
	685, 686, 687, 688, 689, 690, 692, 696, 700, 701,
	702, 703, 704, 705, 706, 707, 708, 709, 710, 711,
	713, 715, 718, 726, 727, 728, 729, 730, 731, 732,
	733, 734, 735, 736, 737, 738, 739, 740, 741, 743,
	745, 746, 750, 751, 766, 774, 777, 779, 780, 783,
	784, 785, 786, 787, 788, 789, 790, 791, 792, 793,
	794, 795, 796, 797, 798, 801, 809, 811
};

// Sprite names in the order Doom originally had them.
#define NUMSPRITES 138
static const char *OrgSprNames[NUMSPRITES] =
{
	"TROO","SHTG","PUNG","PISG","PISF","SHTF","SHT2","CHGG","CHGF","MISG",
	"MISF","SAWG","PLSG","PLSF","BFGG","BFGF","BLUD","PUFF","BAL1","BAL2",
	"PLSS","PLSE","MISL","BFS1","BFE1","BFE2","TFOG","IFOG","PLAY","POSS",
	"SPOS","VILE","FIRE","FATB","FBXP","SKEL","MANF","FATT","CPOS","SARG",
	"HEAD","BAL7","BOSS","BOS2","SKUL","SPID","BSPI","APLS","APBX","CYBR",
	"PAIN","SSWV","KEEN","BBRN","BOSF","ARM1","ARM2","BAR1","BEXP","FCAN",
	"BON1","BON2","BKEY","RKEY","YKEY","BSKU","RSKU","YSKU","STIM","MEDI",
	"SOUL","PINV","PSTR","PINS","MEGA","SUIT","PMAP","PVIS","CLIP","AMMO",
	"ROCK","BROK","CELL","CELP","SHEL","SBOX","BPAK","BFUG","MGUN","CSAW",
	"LAUN","PLAS","SHOT","SGN2","COLU","SMT2","GOR1","POL2","POL5","POL4",
	"POL3","POL1","POL6","GOR2","GOR3","GOR4","GOR5","SMIT","COL1","COL2",
	"COL3","COL4","CAND","CBRA","COL6","TRE1","TRE2","ELEC","CEYE","FSKU",
	"COL5","TBLU","TGRN","TRED","SMBT","SMGT","SMRT","HDB1","HDB2","HDB3",
	"HDB4","HDB5","HDB6","POB1","POB2","BRS1","TLMP","TLP2"
};

// Map to where the orginal Doom states have moved to
#define NUMSTATEMAPS 134

enum EStateBase
{
	FirstState,
	SpawnState,
	DeathState
};

static struct
{
	union {
		const char *name;
		FState *state;
	} o;
	enum EStateBase basestate;
	int statespan;
} StateMap[] = {
	// S_NULL is implicit
	{{"Weapon"}, FirstState, 1},			// S_LIGHTDONE
	{{"Fist"}, FirstState, 8},				// S_PUNCH - S_PUNCH5
	{{"Pistol"}, FirstState, 8},			// S_PISTOL - S_PISTOLFLASH
	{{"Shotgun"}, FirstState, 14},			// S_SGUN - S_SGUNFLASH2
	{{"SuperShotgun"}, FirstState, 17},		// S_DSGUN - S_DSGUNFLASH2
	{{"Chaingun"}, FirstState, 8},			// S_CHAIN - S_CHAINFLASH2
	{{"RocketLauncher"}, FirstState, 10},	// S_MISSILE - S_MISSILEFLASH4
	{{"Chainsaw"}, FirstState, 7},			// S_SAW - S_SAW3
	{{"PlasmaRifle"}, FirstState, 7},		// S_PLASMA - S_PLASMAFLASH2
	{{"BFG9000"}, FirstState, 9},			// S_BFG - S_BFGFLASH2
	{{"Blood"}, FirstState, 3},				// S_BLOOD1 - S_BLOOD3
	{{"BulletPuff"}, FirstState, 4},		// S_PUFF1 - S_PUFF4
	{{"DoomImpBall"}, FirstState, 5},		// S_TBALL1 - S_TBALLX3
	{{"CacodemonBall"}, FirstState, 5},		// S_RBALL1 - S_RBALLX3
	{{"PlasmaBall"}, FirstState, 7},		// S_PLASBALL - S_PLASEXP5
	{{"Rocket"}, SpawnState, 1},			// S_ROCKET
	{{"BFGBall"}, SpawnState, 8},			// S_BFGSHOT - S_BFGLAND6
	{{"BFGExtra"}, SpawnState, 4},			// S_BFGEXP - S_BFGEXP4
	{{"Rocket"}, DeathState, 3},			// S_EXPLODE1 - S_EXPLODE3
	{{"TeleportFog"}, SpawnState, 12},		// S_TFOG - S_TFOG10
	{{"ItemFog"}, SpawnState, 7},			// S_IFOG - S_IFOG5
	{{"DoomPlayer"}, FirstState, 25},		// S_PLAY - S_PLAY_XDIE9
	{{"ZombieMan"}, FirstState, 33},		// S_POSS_STND - S_POSS_RAISE4
	{{"ShotgunGuy"}, FirstState, 34},		// S_SPOS_STND - S_SPOS_RAISE5
	{{"Archvile"}, FirstState, 40},			// S_VILE_STND - S_VILE_DIE10
	{{"ArchvileFire"}, FirstState, 30},		// S_FIRE1 - S_FIRE30
	{{"RevenantTracerSmoke"}, FirstState, 5}, // S_SMOKE1 - S_SMOKE5
	{{"RevenantTracer"}, FirstState, 5},	// S_TRACER - S_TRACEEXP3
	{{"Revenant"}, FirstState, 36},			// S_SKEL_STND - S_SKEL_RAISE6
	{{"FatShot"}, FirstState, 5},			// S_FATSHOT1 - S_FATSHOTX3
	{{"Fatso"}, FirstState, 44},			// S_FATT_STND - S_FATT_RAISE8
	{{"ChaingunGuy"}, FirstState, 36},		// S_CPOS_STND - S_CPOS_RAISE7
	{{"DoomImp"}, FirstState, 33},			// S_TROO_STND - S_TROO_RAISE5
	{{"Demon"}, FirstState, 27},			// S_SARG_STND - S_SARG_RAISE6
	{{"Cacodemon"}, FirstState, 20},		// S_HEAD_STND - S_HEAD_RAISE6
	{{"BaronBall"}, FirstState, 5},			// S_BRBALL1 - S_BRBALLX3
	{{"BaronOfHell"}, FirstState, 29},		// S_BOSS_STND - S_BOSS_RAISE7
	{{"HellKnight"}, FirstState, 29},		// S_BOS2_STND - S_BOS2_RAISE7
	{{"LostSoul"}, FirstState, 16},			// S_SKULL_STND - S_SKULL_DIE6
	{{"SpiderMastermind"}, FirstState, 31}, // S_SPID_STND - S_SPID_DIE11
	{{"Arachnotron"}, FirstState, 35},		// S_BSPI_STND - S_BSPI_RAISE7
	{{"ArachnotronPlasma"}, FirstState, 7}, // S_ARACH_PLAZ - S_ARACH_PLEX5
	{{"Cyberdemon"}, FirstState, 27},		// S_CYBER_STND - S_CYBER_DIE10
	{{"PainElemental"}, FirstState, 25},	// S_PAIN_STND - S_PAIN_RAISE6
	{{"WolfensteinSS"}, FirstState, 37},	// S_SSWV_STND - S_SSWV_RAISE5
	{{"CommanderKeen"}, FirstState, 15},	// S_KEENSTND - S_KEENPAIN2
	{{"BossBrain"}, SpawnState, 6},			// S_BRAIN - S_BRAIN_DIE4
	{{"BossEye"}, FirstState, 3},			// S_BRAINEYE - S_BRAINEYE1
	{{"SpawnShot"}, FirstState, 4},			// S_SPAWN1 - S_SPAWN4
	{{"SpawnFire"}, FirstState, 8},			// S_SPAWNFIRE1 - S_SPAWNFIRE8
	{{"BossBrain"}, FirstState, 3},			// S_BRAINEXPLODE1 - S_BRAINEXPLODE3
	{{"GreenArmor"}, FirstState, 2},		// S_ARM1 - S_ARM1A
	{{"BlueArmor"}, FirstState, 2},			// S_ARM2 - S_ARM2A
	{{"ExplosiveBarrel"}, FirstState, 7},	// S_BAR1 - S_BEXP5
	{{"BurningBarrel"}, FirstState, 3},		// S_BBAR1 - S_BBAR3
	{{"HealthBonus"}, FirstState, 6},		// S_BON1 - S_BON1E
	{{"ArmorBonus"}, FirstState, 6},		// S_BON2 - S_BON2E
	{{"BlueCard"}, FirstState, 2},			// S_BKEY - S_BKEY2
	{{"RedCard"}, FirstState, 2},			// S_RKEY - S_RKEY2
	{{"YellowCard"}, FirstState, 2},		// S_YKEY - S_YKEY2
	{{"BlueSkull"}, FirstState, 2},			// S_BSKULL - S_BSKULL2
	{{"RedSkull"}, FirstState, 2},			// S_RSKULL - S_RSKULL2
	{{"YellowSkull"}, FirstState, 2},		// S_YSKULL - S_YSKULL2
	{{"Stimpack"}, FirstState, 1},			// S_STIM
	{{"Medikit"}, FirstState, 1},			// S_MEDI
	{{"Soulsphere"}, FirstState, 6},		// S_SOUL - S_SOUL6
	{{"InvulnerabilitySphere"}, FirstState, 4}, // S_PINV - S_PINV4
	{{"Berserk"}, FirstState, 1},			// S_PSTR
	{{"BlurSphere"}, FirstState, 4},		// S_PINS - P_PINS4
	{{"Megasphere"}, FirstState, 4},		// S_MEGA - S_MEGA4
	{{"RadSuit"}, FirstState, 1},			// S_SUIT
	{{"Allmap"}, FirstState, 6},			// S_PMAP - S_PMAP6
	{{"Infrared"}, FirstState, 2},			// S_PVIS - S_PVIS2
	{{"Clip"}, FirstState, 1},				// S_CLIP
	{{"ClipBox"}, FirstState, 1},			// S_AMMO
	{{"RocketAmmo"}, FirstState, 1},		// S_ROCK
	{{"RocketBox"}, FirstState, 1},			// S_BROK
	{{"Cell"}, FirstState, 1},				// S_CELL
	{{"CellPack"}, FirstState, 1},			// S_CELP
	{{"Shell"}, FirstState, 1},				// S_SHEL
	{{"ShellBox"}, FirstState, 1},			// S_SBOX
	{{"Backpack"}, FirstState, 1},			// S_BPAK
	{{"BFG9000"}, SpawnState, 1},			// S_BFUG
	{{"Chaingun"}, SpawnState, 1},			// S_MGUN
	{{"Chainsaw"}, SpawnState, 1},			// S_CSAW
	{{"RocketLauncher"}, SpawnState, 1},	// S_LAUN
	{{"PlasmaRifle"}, SpawnState, 1},		// S_PLAS
	{{"Shotgun"}, SpawnState, 1},			// S_SHOT
	{{"SuperShotgun"}, SpawnState, 1},		// S_SHOT2
	{{"Column"}, FirstState, 1},			// S_COLU
	{{"DoomUnusedStates"}, FirstState, 1},	// S_STALAG
	{{"BloodyTwitch"}, FirstState, 4},		// S_BLOODYTWITCH - S_BLOODYTWITCH4
	{{"DoomUnusedStates"}, DeathState, 2},	// S_DEADTORSO - S_DEADBOTTOM
	{{"HeadsOnAStick"}, FirstState, 1},		// S_HEADSONSTICK
	{{"Gibs"}, FirstState, 1},				// S_GIBS
	{{"HeadOnAStick"}, FirstState, 1},		// S_HEADONASTICK
	{{"HeadCandles"}, FirstState, 2},		// S_HEADCANDLES - S_HEADCANDLES2
	{{"DeadStick"}, FirstState, 1},			// S_DEADSTICK
	{{"LiveStick"}, FirstState, 2},			// S_LIVESTICK
	{{"Meat2"}, FirstState, 1},				// S_MEAT2
	{{"Meat3"}, FirstState, 1},				// S_MEAT3
	{{"Meat4"}, FirstState, 1},				// S_MEAT4
	{{"Meat5"}, FirstState, 1},				// S_MEAT5
	{{"Stalagtite"}, FirstState, 1},		// S_STALAGTITE
	{{"TallGreenColumn"}, FirstState, 1},	// S_TALLGRNCOL
	{{"ShortGreenColumn"}, FirstState, 1},	// S_SHRTGRNCOL
	{{"TallRedColumn"}, FirstState, 1},		// S_TALLREDCOL
	{{"ShortRedColumn"}, FirstState, 1},	// S_SHRTREDCOL
	{{"Candlestick"}, FirstState, 1},		// S_CANDLESTIK
	{{"Candelabra"}, FirstState, 1},		// S_CANDELABRA
	{{"SkullColumn"}, FirstState, 1},		// S_SKULLCOL
	{{"TorchTree"}, FirstState, 1},			// S_TORCHTREE
	{{"BigTree"}, FirstState, 1},			// S_BIGTREE
	{{"TechPillar"}, FirstState, 1},		// S_TECHPILLAR
	{{"EvilEye"}, FirstState, 4},			// S_EVILEYE - S_EVILEYE4
	{{"FloatingSkull"}, FirstState, 3},		// S_FLOATSKULL - S_FLOATSKULL3
	{{"HeartColumn"}, FirstState, 2},		// S_HEARTCOL - S_HEARTCOL2
	{{"BlueTorch"}, FirstState, 4},			// S_BLUETORCH - S_BLUETORCH4
	{{"GreenTorch"}, FirstState, 4},		// S_GREENTORCH - S_GREENTORCH4
	{{"RedTorch"}, FirstState, 4},			// S_REDTORCH - S_REDTORCH4
	{{"ShortBlueTorch"}, FirstState, 4},	// S_BTORCHSHRT - S_BTORCHSHRT4
	{{"ShortGreenTorch"}, FirstState, 4},	// S_GTORCHSHRT - S_GTORCHSHRT4
	{{"ShortRedTorch"}, FirstState, 4},		// S_RTORCHSHRT - S_RTORCHSHRT4
	{{"HangNoGuts"}, FirstState, 1},		// S_HANGNOGUTS
	{{"HangBNoBrain"}, FirstState, 1},		// S_HANGBNOBRAIN
	{{"HangTLookingDown"}, FirstState, 1},	// S_HANGTLOOKDN
	{{"HangTSkull"}, FirstState, 1},		// S_HANGTSKULL
	{{"HangTLookingUp"}, FirstState, 1},	// S_HANGTLOOKUP
	{{"HangTNoBrain"}, FirstState, 1},		// S_HANGTNOBRAIN
	{{"ColonGibs"}, FirstState, 1},			// S_COLONGIBS
	{{"SmallBloodPool"}, FirstState, 1},	// S_SMALLPOOL
	{{"BrainStem"}, FirstState, 1},			// S_BRAINSTEM
	{{"TechLamp"}, FirstState, 4},			// S_TECHLAMP - S_TECHLAMP4
	{{"TechLamp2"}, FirstState, 4}			// S_TECH2LAMP - S_TECH2LAMP4
};

// Sound equivalences. When a patch tries to change a sound,
// use these sound names.
char *SoundMap[] = {
	NULL,
	"weapons/pistol",
	"weapons/shotgf",
	"weapons/shotgr",
	"weapons/sshotf",
	"weapons/sshoto",
	"weapons/sshotc",
	"weapons/sshotl",
	"weapons/plasmaf",
	"weapons/bfgf",
	"weapons/sawup",
	"weapons/sawidle",
	"weapons/sawfull",
	"weapons/sawhit",
	"weapons/rocklf",
	"weapons/bfgx",
	"imp/attack",
	"imp/shotx",
	"plats/pt1_strt",
	"plats/pt1_stop",
	"doors/dr1_open",
	"doors/dr1_clos",
	"plats/pt1_mid",
	"switches/normbutn",
	"switches/exitbutn",
	"*pain100_1",
	"demon/pain",
	"grunt/pain",
	"vile/pain",
	"fatso/pain",
	"pain/pain",
	"misc/gibbed",
	"misc/i_pkup",
	"misc/w_pkup",
	"*land1",
	"misc/teleport",
	"grunt/sight1",
	"grunt/sight2",
	"grunt/sight3",
	"imp/sight1",
	"imp/sight2",
	"demon/sight",
	"caco/sight",
	"baron/sight",
	"cyber/sight",
	"spider/sight",
	"baby/sight",
	"knight/sight",
	"vile/sight",
	"fatso/sight",
	"pain/sight",
	"skull/melee",
	"demon/melee",
	"skeleton/melee",
	"vile/start",
	"imp/melee",
	"skeleton/swing",
	"*death1",
	"*xdeath1",
	"grunt/death1",
	"grunt/death2",
	"grunt/death3",
	"imp/death1",
	"imp/death2",
	"demon/death",
	"caco/death",
	"misc/unused",
	"baron/death",
	"cyber/death",
	"spider/death",
	"baby/death",
	"vile/death",
	"knight/death",
	"pain/death",
	"skeleton/death",
	"grunt/active",
	"imp/active",
	"demon/active",
	"baby/active",
	"baby/walk",
	"vile/active",
	"*grunt1",
	"world/barrelx",
	"*fist",
	"cyber/hoof",
	"spider/walk",
	"weapons/chngun",
	"misc/chat2",
	"doors/dr2_open",
	"doors/dr2_clos",
	"misc/spawn",
	"vile/firecrkl",
	"vile/firestrt",
	"misc/p_pkup",
	"brain/spit",
	"brain/cube",
	"brain/sight",
	"brain/pain",
	"brain/death",
	"fatso/attack",
	"gatso/death",
	"wolfss/sight",
	"wolfss/death",
	"keen/pain",
	"keen/death",
	"skeleton/active",
	"skeleton/sight",
	"skeleton/attack",
	"misc/chat"
};

// Names of different actor types, in original Doom 2 order
const char *InfoNames[] =
{
	"DoomPlayer",
	"ZombieMan",
	"ShotgunGuy",
	"Archvile",
	"ArchvileFire",
	"Revenant",
	"RevenantTracer",
	"RevenantTracerSmoke",
	"Fatso",
	"FatShot",
	"ChaingunGuy",
	"DoomImp",
	"Demon",
	"Spectre",
	"Cacodemon",
	"BaronOfHell",
	"BaronBall",
	"HellKnight",
	"LostSoul",
	"SpiderMastermind",
	"Arachnotron",
	"Cyberdemon",
	"PainElemental",
	"WolfensteinSS",
	"CommanderKeen",
	"BossBrain",
	"BossEye",
	"BossTarget",
	"SpawnShot",
	"SpawnFire",
	"ExplosiveBarrel",
	"DoomImpBall",
	"CacodemonBall",
	"Rocket",
	"PlasmaBall",
	"BFGBall",
	"ArachnotronPlasma",
	"BulletPuff",
	"Blood",
	"TeleportFog",
	"ItemFog",
	"TeleportDest",
	"BFGExtra",
	"GreenArmor",
	"BlueArmor",
	"HealthBonus",
	"ArmorBonus",
	"BlueCard",
	"RedCard",
	"YellowCard",
	"YellowSkull",
	"RedSkull",
	"BlueSkull",
	"Stimpack",
	"Medikit",
	"Soulsphere",
	"InvulnerabilitySphere",
	"Berserk",
	"BlurSphere",
	"RadSuit",
	"Allmap",
	"Infrared",
	"Megasphere",
	"Clip",
	"ClipBox",
	"RocketAmmo",
	"RocketBox",
	"Cell",
	"CellPack",
	"Shell",
	"ShellBox",
	"Backpack",
	"BFG9000",
	"Chaingun",
	"Chainsaw",
	"RocketLauncher",
	"PlasmaRifle",
	"Shotgun",
	"SuperShotgun",
	"TechLamp",
	"TechLamp2",
	"Column",
	"TallGreenColumn",
	"ShortGreenColumn",
	"TallRedColumn",
	"ShortRedColumn",
	"SkullColumn",
	"HeartColumn",
	"EvilEye",
	"FloatingSkull",
	"TorchTree",
	"BlueTorch",
	"GreenTorch",
	"RedTorch",
	"ShortBlueTorch",
	"ShortGreenTorch",
	"ShortRedTorch",
	"Stalagtite",
	"TechPillar",
	"Candlestick",
	"Candelabra",
	"BloodyTwitch",
	"Meat2",
	"Meat3",
	"Meat4",
	"Meat5",
	"NonsolidMeat2",
	"NonsolidMeat4",
	"NonsolidMeat3",
	"NonsolidMeat5",
	"NonsolidTwitch",
	"DeadCacodemon",
	"DeadMarine",
	"DeadZombieMan",
	"DeadDemon",
	"DeadLostSoul",
	"DeadDoomImp",
	"DeadShotgunGuy",
	"GibbedMarine",
	"GibbedMarineExtra",
	"HeadsOnAStick",
	"Gibs",
	"HeadOnAStick",
	"HeadCandles",
	"DeadStick",
	"LiveStick",
	"BigTree",
	"BurningBarrel",
	"HangNoGuts",
	"HangBNoBrain",
	"HangTLookingDown",
	"HangTSkull",
	"HangTLookingUp",
	"HangTNoBrain",
	"ColonGibs",
	"SmallBloodPool",
	"BrainStem"
};

#define NUMINFOS (sizeof(InfoNames)/sizeof(const char *))

struct CodePtr {
	char *name;
	actionf_t func;
};

static const struct CodePtr CodePtrs[] = {
	{ "NULL",			{(actionf_p1)NULL} },
	{ "MonsterRail",	{(actionf_p1)A_MonsterRail} },
	{ "FireRailgun",	{(actionf_p1)A_FireRailgun} },
	{ "FireRailgunLeft",{(actionf_p1)A_FireRailgunLeft} },
	{ "FireRailgunRight",{(actionf_p1)A_FireRailgunRight} },
	{ "RailWait",		{(actionf_p1)A_RailWait} },
	{ "Light0",			{(actionf_p1)A_Light0} },
	{ "WeaponReady",	{(actionf_p1)A_WeaponReady} },
	{ "Lower",			{(actionf_p1)A_Lower} },
	{ "Raise",			{(actionf_p1)A_Raise} },
	{ "Punch",			{(actionf_p1)A_Punch} },
	{ "ReFire",			{(actionf_p1)A_ReFire} },
	{ "FirePistol",		{(actionf_p1)A_FirePistol} },
	{ "Light1",			{(actionf_p1)A_Light1} },
	{ "FireShotgun",	{(actionf_p1)A_FireShotgun} },
	{ "Light2",			{(actionf_p1)A_Light2} },
	{ "FireShotgun2",	{(actionf_p1)A_FireShotgun2} },
	{ "CheckReload",	{(actionf_p1)A_CheckReload} },
	{ "OpenShotgun2",	{(actionf_p1)A_OpenShotgun2} },
	{ "LoadShotgun2",	{(actionf_p1)A_LoadShotgun2} },
	{ "CloseShotgun2",	{(actionf_p1)A_CloseShotgun2} },
	{ "FireCGun",		{(actionf_p1)A_FireCGun} },
	{ "A_GunFlash",		{(actionf_p1)A_GunFlash} },
	{ "FireMissile",	{(actionf_p1)A_FireMissile} },
	{ "Saw",			{(actionf_p1)A_Saw} },
	{ "FirePlasma",		{(actionf_p1)A_FirePlasma} },
	{ "BFGsound",		{(actionf_p1)A_BFGsound} },
	{ "FireBFG",		{(actionf_p1)A_FireBFG} },
	{ "BFGSpray",		{(actionf_p1)A_BFGSpray} },
	{ "Explode",		{(actionf_p1)A_Explode} },
	{ "Pain",			{(actionf_p1)A_Pain} },
	{ "PlayerScream",	{(actionf_p1)A_PlayerScream} },
	{ "Fall",			{(actionf_p1)A_NoBlocking} },
	{ "XScream",		{(actionf_p1)A_XScream} },
	{ "Look",			{(actionf_p1)A_Look} },
	{ "Chase",			{(actionf_p1)A_Chase} },
	{ "FaceTarget",		{(actionf_p1)A_FaceTarget} },
	{ "PosAttack",		{(actionf_p1)A_PosAttack} },
	{ "Scream",			{(actionf_p1)A_Scream} },
	{ "SPosAttack",		{(actionf_p1)A_SPosAttack} },
	{ "VileChase",		{(actionf_p1)A_VileChase} },
	{ "VileStart",		{(actionf_p1)A_VileStart} },
	{ "VileTarget",		{(actionf_p1)A_VileTarget} },
	{ "VileAttack",		{(actionf_p1)A_VileAttack} },
	{ "StartFire",		{(actionf_p1)A_StartFire} },
	{ "Fire",			{(actionf_p1)A_Fire} },
	{ "FireCrackle",	{(actionf_p1)A_FireCrackle} },
	{ "Tracer",			{(actionf_p1)A_Tracer} },
	{ "SkelWhoosh",		{(actionf_p1)A_SkelWhoosh} },
	{ "SkelFist",		{(actionf_p1)A_SkelFist} },
	{ "SkelMissile",	{(actionf_p1)A_SkelMissile} },
	{ "FatRaise",		{(actionf_p1)A_FatRaise} },
	{ "FatAttack1",		{(actionf_p1)A_FatAttack1} },
	{ "FatAttack2",		{(actionf_p1)A_FatAttack2} },
	{ "FatAttack3",		{(actionf_p1)A_FatAttack3} },
	{ "BossDeath",		{(actionf_p1)A_BossDeath} },
	{ "CPosAttack",		{(actionf_p1)A_CPosAttack} },
	{ "CPosRefire",		{(actionf_p1)A_CPosRefire} },
	{ "TroopAttack",	{(actionf_p1)A_TroopAttack} },
	{ "SargAttack",		{(actionf_p1)A_SargAttack} },
	{ "HeadAttack",		{(actionf_p1)A_HeadAttack} },
	{ "BruisAttack",	{(actionf_p1)A_BruisAttack} },
	{ "SkullAttack",	{(actionf_p1)A_SkullAttack} },
	{ "Metal",			{(actionf_p1)A_Metal} },
	{ "SpidRefire",		{(actionf_p1)A_SpidRefire} },
	{ "BabyMetal",		{(actionf_p1)A_BabyMetal} },
	{ "BspiAttack",		{(actionf_p1)A_BspiAttack} },
	{ "Hoof",			{(actionf_p1)A_Hoof} },
	{ "CyberAttack",	{(actionf_p1)A_CyberAttack} },
	{ "PainAttack",		{(actionf_p1)A_PainAttack} },
	{ "PainDie",		{(actionf_p1)A_PainDie} },
	{ "KeenDie",		{(actionf_p1)A_KeenDie} },
	{ "BrainPain",		{(actionf_p1)A_BrainPain} },
	{ "BrainScream",	{(actionf_p1)A_BrainScream} },
	{ "BrainDie",		{(actionf_p1)A_BrainDie} },
	{ "BrainAwake",		{(actionf_p1)A_BrainAwake} },
	{ "BrainSpit",		{(actionf_p1)A_BrainSpit} },
	{ "SpawnSound",		{(actionf_p1)A_SpawnSound} },
	{ "SpawnFly",		{(actionf_p1)A_SpawnFly} },
	{ "BrainExplode",	{(actionf_p1)A_BrainExplode} },
	{ "Die",			{(actionf_p1)A_Die} },
	{ "Detonate",		{(actionf_p1)A_Detonate} },
	{ "Mushroom",		{(actionf_p1)A_Mushroom} },
	{ NULL, }
};

struct Key {
	char *name;
	ptrdiff_t offset;
};

extern byte cheat_mus_seq[9];
extern byte cheat_choppers_seq[11];
extern byte cheat_god_seq[6];
extern byte cheat_ammo_seq[6];
extern byte cheat_ammonokey_seq[5];
extern byte cheat_noclip_seq[11];
extern byte cheat_noclip_seq2[7];
extern byte cheat_powerup_seq[7][10];
extern byte cheat_clev_seq[10];
extern byte cheat_mypos_seq[8];
extern byte cheat_amap_seq[5];

static int PatchThing (int);
static int PatchSound (int);
static int PatchFrame (int);
static int PatchSprite (int);
static int PatchAmmo (int);
static int PatchWeapon (int);
static int PatchPointer (int);
static int PatchCheats (int);
static int PatchMisc (int);
static int PatchText (int);
static int PatchStrings (int);
static int PatchPars (int);
static int PatchCodePtrs (int);
static int DoInclude (int);

static const struct {
	char *name;
	int (*func)(int);
} Modes[] = {
	// These appear in .deh and .bex files
	{ "Thing",		PatchThing },
	{ "Sound",		PatchSound },
	{ "Frame",		PatchFrame },
	{ "Sprite",		PatchSprite },
	{ "Ammo",		PatchAmmo },
	{ "Weapon",		PatchWeapon },
	{ "Pointer",	PatchPointer },
	{ "Cheat",		PatchCheats },
	{ "Misc",		PatchMisc },
	{ "Text",		PatchText },
	// These appear in .bex files
	{ "include",	DoInclude },
	{ "[STRINGS]",	PatchStrings },
	{ "[PARS]",		PatchPars },
	{ "[CODEPTR]",	PatchCodePtrs },
	{ NULL, },
};

static int HandleMode (const char *mode, int num);
static BOOL HandleKey (const struct Key *keys, void *structure, const char *key, int value);
static void ChangeCheat (char *newcheat, byte *cheatseq, BOOL needsval);
static BOOL ReadChars (char **stuff, int size);
static char *igets (void);
static int GetLine (void);


static int HandleMode (const char *mode, int num)
{
	int i = 0;

	while (Modes[i].name && stricmp (Modes[i].name, mode))
		i++;

	if (Modes[i].name)
		return Modes[i].func (num);

	// Handle unknown or unimplemented data
	Printf (PRINT_HIGH, "Unknown chunk %s encountered. Skipping.\n", mode);
	do
		i = GetLine ();
	while (i == 1);

	return i;
}

static BOOL HandleKey (const struct Key *keys, void *structure, const char *key, int value)
{
	while (keys->name && stricmp (keys->name, key))
		keys++;

	if (keys->name) {
		*((int *)(((byte *)structure) + keys->offset)) = value;
		return false;
	}

	return true;
}

static int FindSprite (const char *sprname)
{
	size_t i;

	for (i = 0; i < sprites.Size (); i++)
	{
		if (strcmp (sprites[i].name, sprname) == 0)
		{
			return i;
		}
	}
	return -1;
}

static FState *FindState (int statenum)
{
	int i;
	int stateacc;

	if (statenum == 0)
		return NULL;

	for (i = 0, stateacc = 1; i < NUMSTATEMAPS; i++)
	{
		if (stateacc <= statenum && stateacc + StateMap[i].statespan > statenum)
		{
			return StateMap[i].o.state + statenum - stateacc;
		}
		stateacc += StateMap[i].statespan;
	}
	return NULL;
}

static bool PrepStateMap ()
{
	static bool alreadyprepped = false;
	static int errs = 0;
	int i;

	if (alreadyprepped)
		return errs > 0;

	alreadyprepped = true;
	for (i = 0; i < NUMSTATEMAPS; i++)
	{
		const TypeInfo *type = TypeInfo::FindType (StateMap[i].o.name);
		if (type == NULL)
		{
			Printf (PRINT_HIGH, "Can't find type %s\n", StateMap[i].o.name);
			errs++;
		}
		else if (type->ActorInfo == NULL)
		{
			Printf (PRINT_HIGH, "%s has no ActorInfo\n", StateMap[i].o.name);
			errs++;
		}
		else
		{
			switch (StateMap[i].basestate)
			{
			case FirstState:
				StateMap[i].o.state = type->ActorInfo->OwnedStates;
				break;
			case SpawnState:
				StateMap[i].o.state = type->ActorInfo->spawnstate;
				break;
			case DeathState:
				StateMap[i].o.state = type->ActorInfo->deathstate;
				break;
			}
		}
	}

	if (errs > 0)
	{
		Printf (PRINT_HIGH, "%d errors trying to prep statemap\n");
		return true;
	}
	return false;
}

static void ChangeCheat (char *newcheat, byte *cheatseq, BOOL needsval)
{
	while (*cheatseq != 0xff && *cheatseq != 1 && *newcheat)
	{
		*cheatseq++ = *newcheat++;
	}

	if (needsval)
	{
		*cheatseq++ = 0;
		*cheatseq++ = 0;
	}

	*cheatseq = 0xff;
}

static BOOL ReadChars (char **stuff, int size)
{
	char *str = *stuff;

	if (!size) {
		*str = 0;
		return true;
	}

	do {
		// Ignore carriage returns
		if (*PatchPt != '\r')
			*str++ = *PatchPt;
		else
			size++;

		PatchPt++;
	} while (--size);

	*str = 0;
	return true;
}

static void ReplaceSpecialChars (char *str)
{
	char *p = str, c;
	int i;

	while ( (c = *p++) ) {
		if (c != '\\') {
			*str++ = c;
		} else {
			switch (*p) {
				case 'n':
				case 'N':
					*str++ = '\n';
					break;
				case 't':
				case 'T':
					*str++ = '\t';
					break;
				case 'r':
				case 'R':
					*str++ = '\r';
					break;
				case 'x':
				case 'X':
					c = 0;
					p++;
					for (i = 0; i < 2; i++) {
						c <<= 4;
						if (*p >= '0' && *p <= '9')
							c += *p-'0';
						else if (*p >= 'a' && *p <= 'f')
							c += 10 + *p-'a';
						else if (*p >= 'A' && *p <= 'F')
							c += 10 + *p-'A';
						else
							break;
						p++;
					}
					*str++ = c;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					c = 0;
					for (i = 0; i < 3; i++) {
						c <<= 3;
						if (*p >= '0' && *p <= '7')
							c += *p-'0';
						else
							break;
						p++;
					}
					*str++ = c;
					break;
				default:
					*str++ = *p;
					break;
			}
			p++;
		}
	}
	*str = 0;
}

static char *skipwhite (char *str)
{
	if (str)
		while (*str && isspace(*str))
			str++;
	return str;
}

static void stripwhite (char *str)
{
	char *end = str + strlen(str) - 1;

	while (end >= str && isspace(*end))
		end--;

	end[1] = '\0';
}

static char *igets (void)
{
	char *line;

	if (*PatchPt == '\0')
		return NULL;

	line = PatchPt;

	while (*PatchPt != '\n' && *PatchPt != '\0')
		PatchPt++;

	if (*PatchPt == '\n')
		*PatchPt++ = 0;

	return line;
}

static int GetLine (void)
{
	char *line, *line2;

	do {
		while ( (line = igets ()) )
			if (line[0] != '#')		// Skip comment lines
				break;

		if (!line)
			return 0;

		Line1 = skipwhite (line);
	} while (Line1 && *Line1 == 0);	// Loop until we get a line with
									// more than just whitespace.
	line = strchr (Line1, '=');

	if (line) {					// We have an '=' in the input line
		line2 = line;
		while (--line2 >= Line1)
			if (*line2 > ' ')
				break;

		if (line2 < Line1)
			return 0;			// Nothing before '='

		*(line2 + 1) = 0;

		line++;
		while (*line && *line <= ' ')
			line++;

		if (*line == 0)
			return 0;			// Nothing after '='

		Line2 = line;

		return 1;
	} else {					// No '=' in input line
		line = Line1 + 1;
		while (*line > ' ')
			line++;				// Get beyond first word

		*line++ = 0;
		while (*line && *line <= ' ')
			line++;				// Skip white space

		//.bex files don't have this restriction
		//if (*line == 0)
		//	return 0;			// No second word

		Line2 = line;

		return 2;
	}
}

static int PatchThing (int thingy)
{
	static const struct Key keys[] = {
		{ "ID #",				myoffsetof(FActorInfo,doomednum) },
		{ "Hit points",			myoffsetof(FActorInfo,spawnhealth) },
		{ "Reaction time",		myoffsetof(FActorInfo,reactiontime) },
		{ "Pain chance",		myoffsetof(FActorInfo,painchance) },
		{ "Speed",				myoffsetof(FActorInfo,speed) },
		{ "Width",				myoffsetof(FActorInfo,radius) },
		{ "Height",				myoffsetof(FActorInfo,height) },
		{ "Mass",				myoffsetof(FActorInfo,mass) },
		{ "Missile damage",		myoffsetof(FActorInfo,damage) },
		{ "Translucency",		myoffsetof(FActorInfo,translucency) },
		{ NULL, }
	};

	// flags can be specified by name (a .bex extension):
	static const struct {
		short bit;
		short whichflags;
		const char *name;
	} bitnames[] = {
		{ 0, 0, "SPECIAL"},
		{ 1, 0, "SOLID"},
		{ 2, 0, "SHOOTABLE"},
		{ 3, 0, "NOSECTOR"},
		{ 4, 0, "NOBLOCKMAP"},
		{ 5, 0, "AMBUSH"},
		{ 6, 0, "JUSTHIT"},
		{ 7, 0, "JUSTATTACKED"},
		{ 8, 0, "SPAWNCEILING"},
		{ 9, 0, "NOGRAVITY"},
		{10, 0, "DROPOFF"},
		{11, 0, "PICKUP"},
		{12, 0, "NOCLIP"},
		{14, 0, "FLOAT"},
		{15, 0, "TELEPORT"},
		{16, 0, "MISSILE"},
		{17, 0, "DROPPED"},
		{18, 0, "SHADOW"},
		{19, 0, "NOBLOOD"},
		{20, 0, "CORPSE"},
		{21, 0, "INFLOAT"},
		{22, 0, "COUNTKILL"},
		{23, 0, "COUNTITEM"},
		{24, 0, "SKULLFLY"},
		{25, 0, "NOTDMATCH"},
		{26, 0, "TRANSLATION1"},
		{26, 0, "TRANSLATION"},		// BOOM compatibility
		{27, 0, "TRANSLATION2"},
		{27, 0, "UNUSED1"},			// BOOM compatibility
		{28, 0, "UNUSED2"},			// BOOM compatibility
		{29, 0, "UNUSED3"},			// BOOM compatibility
		{30, 0, "UNUSED4"},			// BOOM compatibility
		{30, 0, "TRANSLUCENT"},		// BOOM compatibility?
		{30, 0, "STEALTH"},
		{ 1, 2, "TRANSLUC25"},
		{ 2, 2, "TRANSLUC50"},
		{ 3, 2, "TRANSLUC75"},

		// Names for flags2
		{ 0, 1, "LOGRAV"},
		{ 1, 1, "WINDTHRUST"},
		{ 2, 1, "FLOORBOUNCE"},
		{ 3, 1, "BLASTED"},
		{ 4, 1, "FLY"},
		{ 5, 1, "FLOORCLIP"},
		{ 6, 1, "SPAWNFLOAT"},
		{ 7, 1, "NOTELEPORT"},
		{ 8, 1, "RIP"},
		{ 9, 1, "PUSHABLE"},
		{10, 1, "CANSLIDE"},			// Avoid conflict with SLIDE from BOOM
		{11, 1, "ONMOBJ"},
		{12, 1, "PASSMOBJ"},
		{13, 1, "CANNOTPUSH"},
		{14, 1, "DROPPED"},
		{15, 1, "BOSS"},
		{16, 1, "FIREDAMAGE"},
		{17, 1, "NODMGTHRUST"},
		{18, 1, "TELESTOMP"},
		{19, 1, "FLOATBOB"},
		{20, 1, "DONTDRAW"},
		{21, 1, "IMPACT"},
		{22, 1, "PUSHWALL"},
		{23, 1, "MCROSS"},
		{24, 1, "PCROSS"},
		{25, 1, "CANTLEAVEFLOORPIC"},
		{26, 1, "NONSHOOTABLE"},
		{27, 1, "INVULNERABLE"},
		{28, 1, "DORMANT"},
		{29, 1, "ICEDAMAGE"},
		{30, 1, "SEEKERMISSILE"},
		{31, 1, "REFLECTIVE"}
	};
	int result;
	FActorInfo *info, dummy;
	BOOL hadHeight = false;

	if (thingy > NUMINFOS || thingy <= 0)
	{
		info = &dummy;
		Printf (PRINT_HIGH, "Thing %d out of range.\n", thingy);
	}
	else
	{
		DPrintf ("Thing %d\n", thingy);
		if (thingy > 0)
		{
			const TypeInfo *type = TypeInfo::FindType (InfoNames[thingy - 1]);
			if (type == NULL)
			{
				info = &dummy;
				Printf (PRINT_HIGH, "Could not find thing %s (index %d)\n", InfoNames[thingy - 1], thingy);
			}
			else
			{
				info = type->ActorInfo;
			}
		}
	}

	while ((result = GetLine ()) == 1)
	{
		size_t val = atoi (Line2);

		if (HandleKey (keys, info, Line1, val))
		{
			if (strlen (Line1) > 6)
			{
				if (!stricmp (Line1 + strlen (Line1) - 6, " frame"))
				{
					FState *state = FindState (val);

					if (!strnicmp (Line1, "Initial", 7))
						info->spawnstate = state ? state : RUNTIME_CLASS(AActor)->ActorInfo->spawnstate;
					else if (!strnicmp (Line1, "First moving", 12))
						info->seestate = state;
					else if (!strnicmp (Line1, "Injury", 6))
						info->painstate = state;
					else if (!strnicmp (Line1, "Close attack", 12))
						info->meleestate = state;
					else if (!strnicmp (Line1, "Far attack", 10))
						info->missilestate = state;
					else if (!strnicmp (Line1, "Death", 5))
						info->deathstate = state;
					else if (!strnicmp (Line1, "Exploding", 9))
						info->xdeathstate = state;
					else if (!strnicmp (Line1, "Respawn", 7))
						info->raisestate = state;
				}
				else if (!stricmp (Line1 + strlen (Line1) - 6, " sound"))
				{
					char *snd = val >= sizeof(SoundMap) ?
						SoundMap[0] : SoundMap[val];

					if (!strnicmp (Line1, "Alert", 5))
						info->seesound = snd;
					else if (!strnicmp (Line1, "Attack", 6))
						info->attacksound = snd;
					else if (!strnicmp (Line1, "Pain", 4))
						info->painsound = snd;
					else if (!strnicmp (Line1, "Death", 5))
						info->deathsound = snd;
					else if (!strnicmp (Line1, "Action", 6))
						info->activesound = snd;
				}
			}
			else if (!stricmp (Line1, "Bits"))
			{
				int value = 0, value2 = 0;
				BOOL vchanged = false, v2changed = false;
				char *strval;

				for (strval = Line2; (strval = strtok (strval, ",+| \t\f\r")); strval = NULL)
				{
					size_t iy;

					if (IsNum (strval))
					{
						// Force the top 4 bits to 0 so that the user is forced
						// to use the mnemonics to change them.
						value |= (atoi(strval) & 0x0fffffff);
						vchanged = true;
					}
					else
					{
						for (iy = 0; iy < sizeof(bitnames)/sizeof(bitnames[0]); iy++)
						{
							if (!stricmp (strval, bitnames[iy].name))
							{
								if (bitnames[iy].whichflags)
								{
									v2changed = true;
									if (bitnames[iy].bit & 0xff00)
										value2 |= 1 << (bitnames[iy].bit >> 8);
									value2 |= 1 << (bitnames[iy].bit & 0xff);
								}
								else
								{
									vchanged = true;
									if (bitnames[iy].bit & 0xff00)
										value |= 1 << (bitnames[iy].bit >> 8);
									value |= 1 << (bitnames[iy].bit & 0xff);
								}
								break;
							}
						}
						if (iy >= sizeof(bitnames)/sizeof(bitnames[0]))
							DPrintf("Unknown bit mnemonic %s\n", strval);
					}
				}
				if (vchanged)
				{
					info->flags = value;
					// Bit flags are no longer used to specify translucency.
					// This is just a temporary hack.
					if (info->flags & 0x60000000)
						info->translucency = (info->flags & 0x60000000) >> 15;
				}
				if (v2changed)
					info->flags2 = value2;
				DPrintf ("Bits: %d,%d (0x%08x,0x%08x)\n", info->flags, info->flags2,
														  info->flags, info->flags2);
			}
			else Printf (PRINT_HIGH, unknown_str, Line1, "Thing", thingy);
		} else if (!stricmp (Line1, "Height")) {
			hadHeight = true;
		}
	}

	if (info->flags & MF_SPAWNCEILING && !hadHeight && thingy <= sizeof(OrgHeights) && thingy > 0)
		info->height = OrgHeights[thingy - 1] * FRACUNIT;

	return result;
}

static int PatchSound (int soundNum)
{
	int result;

	DPrintf ("Sound %d (no longer supported)\n", soundNum);
/*
	sfxinfo_t *info, dummy;
	int offset = 0;
	if (soundNum >= 1 && soundNum <= NUMSFX) {
		info = &S_sfx[soundNum];
	} else {
		info = &dummy;
		Printf ("Sound %d out of range.\n");
	}
*/
	while ((result = GetLine ()) == 1) {
		/*
		if (!stricmp  ("Offset", Line1))
			offset = atoi (Line2);
		else CHECKKEY ("Zero/One",			info->singularity)
		else CHECKKEY ("Value",				info->priority)
		else CHECKKEY ("Zero 1",			info->link)
		else CHECKKEY ("Neg. One 1",		info->pitch)
		else CHECKKEY ("Neg. One 2",		info->volume)
		else CHECKKEY ("Zero 2",			info->data)
		else CHECKKEY ("Zero 3",			info->usefulness)
		else CHECKKEY ("Zero 4",			info->lumpnum)
		else Printf (unknown_str, Line1, "Sound", soundNum);
		*/
	}
/*
	if (offset) {
		// Calculate offset from start of sound names
		offset -= toff[dversion] + 21076;

		if (offset <= 64)			// pistol .. bfg
			offset >>= 3;
		else if (offset <= 260)		// sawup .. oof
			offset = (offset + 4) >> 3;
		else						// telept .. skeatk
			offset = (offset + 8) >> 3;

		if (offset >= 0 && offset < NUMSFX) {
			S_sfx[soundNum].name = OrgSfxNames[offset + 1];
		} else {
			Printf ("Sound name %d out of range.\n", offset + 1);
		}
	}
*/
	return result;
}

static int PatchFrame (int frameNum)
{
	static const struct Key keys[] =
	{
		{ "Sprite subnumber",	myoffsetof(FState,frame) },
		{ "Duration",			myoffsetof(FState,tics) },
		{ "Unknown 1",			myoffsetof(FState,misc1) },
		{ "Unknown 2",			myoffsetof(FState,misc2) },
		{ NULL, }
	};
	int result;
	FState *info, dummy;

	info = FindState (frameNum);
	if (info)
	{
		DPrintf ("Frame %d\n", frameNum);
	} else
	{
		info = &dummy;
		Printf (PRINT_HIGH, "Frame %d out of range\n", frameNum);
	}

	while ((result = GetLine ()) == 1)
	{
		int val = atoi (Line2);

		if (HandleKey (keys, info, Line1, val))
		{
			if (stricmp (Line1, "Sprite number") == 0)
			{
				int i;

				if (val < NUMSPRITES)
				{
					for (i = 0; i < sprites.Size(); i++)
					{
						if (memcmp (OrgSprNames[val], sprites[i].name, 4) == 0)
						{
							info->sprite.index = i;
							break;
						}
					}
					if (i == sprites.Size ())
					{
						Printf (PRINT_HIGH, "Frame %d: Sprite %d (%s) is undefined\n",
							frameNum, val, OrgSprNames[val]);
					}
				}
				else
				{
					Printf (PRINT_HIGH, "Frame %d: Sprite %d out of range\n", frameNum, val);
				}
			}
			else if (stricmp (Line1, "Next frame") == 0)
			{
				info->nextstate = FindState (val);
			}
			else
			{
				Printf (PRINT_HIGH, unknown_str, Line1, "Frame", frameNum);
			}
		}
	}

	return result;
}

static int PatchSprite (int sprNum)
{
	int result;
	int offset = 0;

	if (sprNum >= 0 && sprNum < NUMSPRITES)
	{
		DPrintf ("Sprite %d\n", sprNum);
	}
	else
	{
		Printf (PRINT_HIGH, "Sprite %d out of range.\n", sprNum);
		sprNum = -1;
	}

	while ((result = GetLine ()) == 1)
	{
		if (!stricmp ("Offset", Line1))
			offset = atoi (Line2);
		else Printf (PRINT_HIGH, unknown_str, Line1, "Sprite", sprNum);
	}

	if (offset > 0 && sprNum != -1)
	{
		// Calculate offset from beginning of sprite names.
		offset = (offset - toff[dversion] - 22044) / 8;

		if (offset >= 0 && offset < NUMSPRITES)
		{
			sprNum = FindSprite (OrgSprNames[sprNum]);
			if (sprNum != -1)
				strncpy (sprites[sprNum].name, OrgSprNames[offset], 4);
		}
		else
		{
			Printf (PRINT_HIGH, "Sprite name %d out of range.\n", offset);
		}
	}

	return result;
}

static int PatchAmmo (int ammoNum)
{
	int result;
	int *max;
	int *per;
	int oldclip;
	int dummy;

	if (ammoNum >= 0 && ammoNum < 4)
	{
		DPrintf ("Ammo %d.\n", ammoNum);
		max = &maxammo[ammoNum];
		per = &clipammo[ammoNum];
	}
	else
	{
		Printf (PRINT_HIGH, "Ammo %d out of range.\n", ammoNum);
		max = per = &dummy;
	}

	oldclip = *per;

	while ((result = GetLine ()) == 1)
	{
			 CHECKKEY ("Max ammo", *max)
		else CHECKKEY ("Per ammo", *per)
		else Printf (PRINT_HIGH, unknown_str, Line1, "Ammo", ammoNum);
	}

	if (oldclip != *per)
	{
		int i;

		oldclip = *per;
		for (i = 0; i < NUMWEAPONS; i++)
		{
			if (wpnlev1info[i] && wpnlev1info[i]->ammo == ammoNum)
				wpnlev1info[i]->ammogive = oldclip*2;
		}
	}

	return result;
}

static int PatchWeapon (int weapNum)
{
	int result;
	FWeaponInfo *info, dummy;

	if (weapNum >= 0 && weapNum < NUMWEAPONS)
	{
		info = wpnlev1info[weapNum];
		DPrintf ("Weapon %d\n", weapNum);
	}
	else
	{
		info = &dummy;
		Printf (PRINT_HIGH, "Weapon %d out of range.\n", weapNum);
	}

	while ((result = GetLine ()) == 1)
	{
		int val = atoi (Line2);

		if (strlen (Line1) >= 9)
		{
			if (stricmp (Line1 + strlen (Line1) - 6, " frame") == 0)
			{
				FState *state = FindState (val);

				if (strnicmp (Line1, "Deselect", 8) == 0)
					info->upstate = state;
				else if (strnicmp (Line1, "Select", 6) == 0)
					info->downstate = state;
				else if (strnicmp (Line1, "Bobbing", 7) == 0)
					info->readystate = state;
				else if (strnicmp (Line1, "Shooting", 8) == 0)
					info->atkstate = info->holdatkstate = state;
				else if (strnicmp (Line1, "Firing", 6) == 0)
					info->flashstate = state;
			}
			else if (stricmp (Line1, "Ammo type") == 0)
			{
				info->ammo = (ammotype_t)val;
				info->ammogive = clipammo[val];
			}
			else
			{
				Printf (PRINT_HIGH, unknown_str, Line1, "Weapon", weapNum);
			}
		}
	}

	return result;
}

static int PatchPointer (int ptrNum)
{
	int result;

	if (ptrNum >= 0 && ptrNum < 448) {
		DPrintf ("Pointer %d\n", ptrNum);
	} else {
		Printf (PRINT_HIGH, "Pointer %d out of range.\n", ptrNum);
		ptrNum = -1;
	}

	while ((result = GetLine ()) == 1) {
		if ((unsigned)ptrNum < NUMCODEP && (!stricmp (Line1, "Codep Frame")))
		{
			FState *state = FindState (CodePConv[ptrNum]);
			if (state)
			{
				if ((unsigned)(atoi (Line2)) >= NUMACTIONS)
					state->action.acvoid = NULL;
				else
					state->action = Actions[atoi (Line2)];
			}
			else
			{
				Printf (PRINT_HIGH, "Bad code pointer %d\n", ptrNum);
			}
		}
		else Printf (PRINT_HIGH, unknown_str, Line1, "Pointer", ptrNum);
	}
	return result;
}

static int PatchCheats (int dummy)
{
	static const struct {
		char *name;
		byte *cheatseq;
		BOOL needsval;
	} keys[] = {
		{ "Change music",		cheat_mus_seq,				 true },
		{ "Chainsaw",			cheat_choppers_seq,			 false },
		{ "God mode",			cheat_god_seq,				 false },
		{ "Ammo & Keys",		cheat_ammo_seq,				 false },
		{ "Ammo",				cheat_ammonokey_seq,		 false },
		{ "No Clipping 1",		cheat_noclip_seq,			 false },
		{ "No Clipping 2",		cheat_noclip_seq2,			 false },
		{ "Invincibility",		cheat_powerup_seq[0],		 false },
		{ "Berserk",			cheat_powerup_seq[1],		 false },
		{ "Invisibility",		cheat_powerup_seq[2],		 false },
		{ "Radiation Suit",		cheat_powerup_seq[3],		 false },
		{ "Auto-map",			cheat_powerup_seq[4],		 false },
		{ "Lite-Amp Goggles",	cheat_powerup_seq[5],		 false },
		{ "BEHOLD menu",		cheat_powerup_seq[6],		 false },
		{ "Level Warp",			cheat_clev_seq,				 true },
		{ "Player Position",	cheat_mypos_seq,			 false },
		{ "Map cheat",			cheat_amap_seq,				 false },
		{ NULL, }
	};
	int result;

	DPrintf ("Cheats\n");

	while ((result = GetLine ()) == 1)
	{
		int i = 0;
		while (keys[i].name && stricmp (keys[i].name, Line1))
			i++;

		if (!keys[i].name)
			Printf (PRINT_HIGH, "Unknown cheat %s.\n", Line2);
		else
			ChangeCheat (Line2, keys[i].cheatseq, keys[i].needsval);
	}
	return result;
}

static int PatchMisc (int dummy)
{
	static const struct Key keys[] = {
		{ "Initial Health",			myoffsetof(struct DehInfo,StartHealth) },
		{ "Initial Bullets",		myoffsetof(struct DehInfo,StartBullets) },
		{ "Max Health",				myoffsetof(struct DehInfo,MaxHealth) },
		{ "Max Armor",				myoffsetof(struct DehInfo,MaxArmor) },
		{ "Green Armor Class",		myoffsetof(struct DehInfo,GreenAC) },
		{ "Blue Armor Class",		myoffsetof(struct DehInfo,BlueAC) },
		{ "Max Soulsphere",			myoffsetof(struct DehInfo,MaxSoulsphere) },
		{ "Soulsphere Health",		myoffsetof(struct DehInfo,SoulsphereHealth) },
		{ "Megasphere Health",		myoffsetof(struct DehInfo,MegasphereHealth) },
		{ "God Mode Health",		myoffsetof(struct DehInfo,GodHealth) },
		{ "IDFA Armor",				myoffsetof(struct DehInfo,FAArmor) },
		{ "IDFA Armor Class",		myoffsetof(struct DehInfo,FAAC) },
		{ "IDKFA Armor",			myoffsetof(struct DehInfo,KFAArmor) },
		{ "IDKFA Armor Class",		myoffsetof(struct DehInfo,KFAAC) },
		{ "Monsters Infight",		myoffsetof(struct DehInfo,Infight) },
		{ NULL, }
	};
	int result;
	gitem_t *item;

	DPrintf ("Misc\n");

	while ((result = GetLine()) == 1)
	{
		if (HandleKey (keys, &deh, Line1, atoi (Line2)))
		{
			if (stricmp (Line1, "BFG Cells/Shot") == 0)
			{
				if (wpnlev1info[wp_bfg])
					wpnlev1info[wp_bfg]->ammouse = atoi (Line2);
			}
			else
			{
				Printf (PRINT_HIGH, "Unknown miscellaneous info %s.\n", Line2);
			}
		}
	}

	if ( (item = FindItem ("Basic Armor")) )
		item->offset = deh.GreenAC;

	if ( (item = FindItem ("Mega Armor")) )
		item->offset = deh.BlueAC;

	// 0xDD == enable infighting
	deh.Infight = deh.Infight == 0xDD ? 1 : 0;

	return result;
}

static int PatchPars (int dummy)
{
	char *space, mapname[8], *moredata;
	level_info_t *info;
	int result, par;

	DPrintf ("[Pars]\n");

	while ( (result = GetLine()) ) {
		// Argh! .bex doesn't follow the same rules as .deh
		if (result == 1) {
			Printf (PRINT_HIGH, "Unknown key in [PARS] section: %s\n", Line1);
			continue;
		}
		if (stricmp ("par", Line1))
			return result;

		space = strchr (Line2, ' ');

		if (!space) {
			Printf (PRINT_HIGH, "Need data after par.\n");
			continue;
		}

		*space++ = '\0';

		while (*space && isspace(*space))
			space++;

		moredata = strchr (space, ' ');

		if (moredata) {
			// At least 3 items on this line, must be E?M? format
			sprintf (mapname, "E%cM%c", *Line2, *space);
			par = atoi (moredata + 1);
		} else {
			// Only 2 items, must be MAP?? format
			sprintf (mapname, "MAP%02d", atoi(Line2) % 100);
			par = atoi (space);
		}

		if (!(info = FindLevelInfo (mapname)) ) {
			Printf (PRINT_HIGH, "No map %s\n", mapname);
			continue;
		}

		info->partime = par;
		DPrintf ("Par for %s changed to %d\n", mapname, par);
	}
	return result;
}

static int PatchCodePtrs (int dummy)
{
	int result;

	DPrintf ("[CodePtr]\n");

	while ((result = GetLine()) == 1)
	{
		if (!strnicmp ("Frame", Line1, 5) && isspace(Line1[5]))
		{
			int frame = atoi (Line1 + 5);
			FState *state = FindState (frame);

			if (state == NULL)
			{
				Printf (PRINT_HIGH, "Frame %d out of range\n", frame);
			}
			else
			{
				int i = 0;
				char *data;

				COM_Parse (Line2);

				if ((com_token[0] == 'A' || com_token[0] == 'a') && com_token[1] == '_')
					data = com_token + 2;
				else
					data = com_token;

				while (CodePtrs[i].name && stricmp (CodePtrs[i].name, data))
					i++;

				if (CodePtrs[i].name)
				{
					state->action.acp1 = CodePtrs[i].func.acp1;
					DPrintf ("Frame %d set to %s\n", frame, CodePtrs[i].name);
				}
				else
				{
					state->action.acp1 = NULL;
					DPrintf ("Unknown code pointer: %s\n", com_token);
				}
			}
		}
	}
	return result;
}

static int PatchText (int oldSize)
{
	int newSize;
	char *oldStr;
	char *newStr;
	char *temp;
	BOOL good;
	int result;
	int i;

	temp = COM_Parse (Line2);		// Skip old size, since we already have it
	if (!COM_Parse (temp))
	{
		Printf (PRINT_HIGH, "Text chunk is missing size of new string.\n");
		return 2;
	}
	newSize = atoi (com_token);

	oldStr = new char[oldSize + 1];
	newStr = new char[newSize + 1];

	if (!oldStr || !newStr)
	{
		Printf (PRINT_HIGH, "Out of memory.\n");
		goto donewithtext;
	}

	good = ReadChars (&oldStr, oldSize);
	good += ReadChars (&newStr, newSize);

	if (!good)
	{
		delete[] newStr;
		delete[] oldStr;
		Printf (PRINT_HIGH, "Unexpected end-of-file.\n");
		return 0;
	}

	if (includenotext)
	{
		Printf (PRINT_HIGH, "Skipping text chunk in included patch.\n");
		goto donewithtext;
	}

	DPrintf ("Searching for text:\n%s\n", oldStr);
	good = false;

	// Search through sprite names
	for (i = 0; i < NUMSPRITES; i++)
	{
		int j = FindSprite (oldStr);
		if (j != -1)
		{
			strncpy (sprites[j].name, newStr, 4);
			good = true;
			// See above.
		}
	}

	if (good)
		goto donewithtext;


	// Search through music names.
	// This is something of an even bigger hack
	// since I changed the way music is handled.
	if (oldSize < 7)
	{		// Music names are never >6 chars
		if ( (temp = new char[oldSize + 3]) )
		{
			level_info_t *info = LevelInfos;
			sprintf (temp, "d_%s", oldStr);

			while (info->level_name)
			{
				if (!stricmp (info->music, temp))
				{
					good = true;
					strcpy (info->music, temp);
				}
				info++;
			}

			delete[] temp;
		}
	}

	if (good)
		goto donewithtext;

	
	// Search through most other texts
	for (i = 0; i < NUMSTRINGS; i++)
	{
		if (!stricmp (Strings[i].builtin, oldStr))
		{
			ReplaceString (&Strings[i].string, newStr);
			Strings[i].type = str_patched;
			good = true;
			break;
		}
	}

	if (!good)
		DPrintf ("   (Unmatched)\n");

donewithtext:
	if (newStr)
		delete[] newStr;
	if (oldStr)
		delete[] oldStr;

	// Fetch next identifier for main loop
	while ((result = GetLine ()) == 1)
		;

	return result;
}

static int PatchStrings (int dummy)
{
	static size_t maxstrlen = 128;
	static char *holdstring;
	int result;

	DPrintf ("[Strings]\n");

	if (!holdstring)
		holdstring = (char *)Malloc (maxstrlen);

	while ((result = GetLine()) == 1)
	{
		int i;

		*holdstring = '\0';
		do
		{
			while (maxstrlen < strlen (holdstring) + strlen (Line2))
			{
				maxstrlen += 128;
				holdstring = (char *)Realloc (holdstring, maxstrlen);
			}
			strcat (holdstring, skipwhite (Line2));
			stripwhite (holdstring);
			if (holdstring[strlen(holdstring)-1] == '\\')
			{
				holdstring[strlen(holdstring)-1] = '\0';
				Line2 = igets ();
			} else
				Line2 = NULL;
		} while (Line2 && *Line2);

		for (i = 0; i < NUMSTRINGS; i++)
			if (!stricmp (Strings[i].name, Line1))
				break;

		if (i == NUMSTRINGS)
		{
			Printf (PRINT_HIGH, "Unknown string: %s\n", Line1);
		}
		else
		{
			ReplaceSpecialChars (holdstring);
			ReplaceString (&Strings[i].string, copystring (holdstring));
			Strings[i].type = str_patched;
			Printf (PRINT_HIGH, "%s set to:\n%s\n", Line1, holdstring);
		}
	}

	return result;
}

static int DoInclude (int dummy)
{
	char *data;
	int savedversion, savepversion;
	char *savepatchfile, *savepatchpt;

	if (including)
	{
		Printf (PRINT_HIGH, "Sorry, can't nest includes\n");
		goto endinclude;
	}

	data = COM_Parse (Line2);
	if (!stricmp (com_token, "notext"))
	{
		includenotext = true;
		data = COM_Parse (data);
	}

	if (!com_token[0])
	{
		includenotext = false;
		Printf (PRINT_HIGH, "Include directive is missing filename\n");
		goto endinclude;
	}

	DPrintf ("Including %s\n", com_token);
	savepatchfile = PatchFile;
	savepatchpt = PatchPt;
	savedversion = dversion;
	savepversion = pversion;
	including = true;

	DoDehPatch (com_token, false);

	DPrintf ("Done with include\n");
	PatchFile = savepatchfile;
	PatchPt = savepatchpt;
	dversion = savedversion;
	pversion = savepversion;

endinclude:
	including = false;
	includenotext = false;
	return GetLine();
}

void DoDehPatch (const char *patchfile, BOOL autoloading)
{
	char file[256];
	int cont;
	int filelen = 0;	// Be quiet, gcc
	int lump;

	PatchFile = NULL;

	lump = W_CheckNumForName ("DEHACKED");

	if (lump >= 0 && autoloading)
	{
		// Execute the DEHACKED lump as a patch.
		filelen = W_LumpLength (lump);
		if ( (PatchFile = new char[filelen + 1]) )
		{
			W_ReadLump (lump, PatchFile);
		}
		else
		{
			Printf (PRINT_HIGH, "Not enough memory to apply patch\n");
			return;
		}
	}
	else if (patchfile)
	{
		// Try to use patchfile as a patch.
		FILE *deh;

		strcpy (file, patchfile);
		FixPathSeperator (file);
		DefaultExtension (file, ".deh");

		if ( !(deh = fopen (file, "rb")) )
		{
			strcpy (file, patchfile);
			FixPathSeperator (file);
			DefaultExtension (file, ".bex");
			deh = fopen (file, "rb");
		}

		if (deh)
		{
			filelen = Q_filelength (deh);
			if ( (PatchFile = new char[filelen + 1]) )
			{
				fread (PatchFile, 1, filelen, deh);
				fclose (deh);
			}
		}

		if (!PatchFile)
		{
			// Couldn't find it on disk, try reading it from a lump
			strcpy (file, patchfile);
			FixPathSeperator (file);
			ExtractFileBase (file, file);
			file[8] = 0;
			lump = W_CheckNumForName (file);
			if (lump >= 0)
			{
				filelen = W_LumpLength (lump);
				if ( (PatchFile = new char[filelen + 1]) )
				{
					W_ReadLump (lump, PatchFile);
				}
				else
				{
					Printf (PRINT_HIGH, "Not enough memory to apply patch\n");
					return;
				}
			}
		}

		if (!PatchFile)
		{
			Printf (PRINT_HIGH, "Could not open DeHackEd patch \"%s\"\n", file);
			return;
		}
	}
	else
	{
		// Nothing to do.
		return;
	}

	// End file with a NULL for our parser
	PatchFile[filelen] = 0;

	dversion = pversion = -1;

	if (gameinfo.gametype != GAME_Doom)
	{
		Printf (PRINT_HIGH, "DeHackEd/BEX patches are only supported for DOOM mode\n");
		delete[] PatchFile;
		return;
	}

	if (PrepStateMap ())
	{
		delete[] PatchFile;
		return;
	}

	cont = 0;
	if (!strncmp (PatchFile, "Patch File for DeHackEd v", 25))
	{
		PatchPt = strchr (PatchFile, '\n');
		while ((cont = GetLine()) == 1)
		{
				 CHECKKEY ("Doom version", dversion)
			else CHECKKEY ("Patch format", pversion)
		}
		if (!cont || dversion == -1 || pversion == -1)
		{
			delete[] PatchFile;
			Printf (PRINT_HIGH, "\"%s\" is not a DeHackEd patch file\n");
			return;
		}
	}
	else
	{
		DPrintf ("Patch does not have DeHackEd signature. Assuming .bex\n");
		dversion = 19;
		pversion = 6;
		PatchPt = PatchFile;
		while ((cont = GetLine()) == 1)
			;
	}

	if (pversion != 6)
	{
		Printf (PRINT_HIGH, "DeHackEd patch version is %d.\nUnexpected results may occur.\n", pversion);
	}

	if (dversion == 16)
		dversion = 0;
	else if (dversion == 17)
		dversion = 2;
	else if (dversion == 19)
		dversion = 3;
	else if (dversion == 20)
		dversion = 1;
	else if (dversion == 21)
		dversion = 4;
	else
	{
		Printf (PRINT_HIGH, "Patch created with unknown DOOM version.\nAssuming version 1.9.\n");
		dversion = 3;
	}

	do
	{
		if (cont == 1)
		{
			Printf (PRINT_HIGH, "Key %s encountered out of context\n", Line1);
			cont = 0;
		}
		else if (cont == 2)
		{
			cont = HandleMode (Line1, atoi (Line2));
		}
	} while (cont);

	delete[] PatchFile;
	Printf (PRINT_HIGH, "Patch installed\n");

}
