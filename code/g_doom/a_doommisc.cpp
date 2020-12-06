#include "actor.h"
#include "info.h"
#include "p_enemy.h"
#include "a_doomglobal.h"

// The barrel of green goop ------------------------------------------------

IMPLEMENT_DEF_SERIAL (AExplosiveBarrel, AActor);
REGISTER_ACTOR (AExplosiveBarrel, Doom);

FState AExplosiveBarrel::States[] =
{
#define S_BAR 0
	S_NORMAL (BAR1, 'A',	6, NULL 						, &States[S_BAR+1]),
	S_NORMAL (BAR1, 'B',	6, NULL 						, &States[S_BAR+0]),

#define S_BEXP (S_BAR+2)
	S_BRIGHT (BEXP, 'A',	5, NULL 						, &States[S_BEXP+1]),
	S_BRIGHT (BEXP, 'B',	5, A_Scream 					, &States[S_BEXP+2]),
	S_BRIGHT (BEXP, 'C',	5, NULL 						, &States[S_BEXP+3]),
	S_BRIGHT (BEXP, 'D',   10, A_Explode					, &States[S_BEXP+4]),
	S_BRIGHT (BEXP, 'E',   10, NULL 						, NULL)
};

void AExplosiveBarrel::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2035;
	info->spawnid = 125;
	info->spawnstate = &States[S_BAR];
	info->spawnhealth = 20;
	info->deathstate = &States[S_BEXP];
	info->deathsound = "world/barrelx";
	info->radius = 10 * FRACUNIT;
	info->height = 34 * FRACUNIT;
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD;
}

// Bullet puff -------------------------------------------------------------

IMPLEMENT_DEF_SERIAL (ABulletPuff, AActor);
REGISTER_ACTOR (ABulletPuff, Doom);

FState ABulletPuff::States[] =
{
	S_BRIGHT (PUFF, 'A',	4, NULL 						, &States[1]),
	S_NORMAL (PUFF, 'B',	4, NULL 						, &States[2]),
	S_NORMAL (PUFF, 'C',	4, NULL 						, &States[3]),
	S_NORMAL (PUFF, 'D',	4, NULL 						, NULL)
};

void ABulletPuff::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnid = 131;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->translucency = TRANSLUC50;
}

// Container for an unused state -------------------------------------------

/* Doom defined the states S_STALAG, S_DEADTORSO, and S_DEADBOTTOM but never
 * actually used them. For compatibility with DeHackEd patches, they still
 * need to be kept around. This actor serves that purpose.
 */

class ADoomUnusedStates : public AActor
{
	DECLARE_ACTOR (ADoomUnusedStates, AActor);
};

IMPLEMENT_DEF_SERIAL (ADoomUnusedStates, AActor);
REGISTER_ACTOR (ADoomUnusedStates, Doom);

FState ADoomUnusedStates::States[] =
{
#define S_STALAG 0
	S_NORMAL (SMT2, 'A',   -1, NULL 			, NULL),

#define S_DEADTORSO (S_STALAG+1)
	S_NORMAL (PLAY, 'N',   -1, NULL 			, NULL),

#define S_DEADBOTTOM (S_DEADTORSO+1)
	S_NORMAL (PLAY, 'S',   -1, NULL 			, NULL)
};

void ADoomUnusedStates::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->deathstate = &States[S_DEADTORSO];
}
