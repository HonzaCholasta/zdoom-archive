#include "actor.h"
#include "a_sharedglobal.h"
#include "m_random.h"
#include "gi.h"

// Water --------------------------------------------------------------------

IMPLEMENT_DEF_SERIAL (AWaterSplash, AActor);
REGISTER_ACTOR (AWaterSplash, Any);

FState AWaterSplash::States[] =
{
#define S_SPLASH 0
	S_NORMAL (SPSH, 'A',	8, NULL, &States[S_SPLASH+1]),
	S_NORMAL (SPSH, 'B',	8, NULL, &States[S_SPLASH+2]),
	S_NORMAL (SPSH, 'C',	8, NULL, &States[S_SPLASH+3]),
	S_NORMAL (SPSH, 'D',   16, NULL, NULL),

#define S_SPLASHX (S_SPLASH+4)
	S_NORMAL (SPSH, 'D',   10, NULL, NULL)
};

void AWaterSplash::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_SPLASH];
	info->deathstate = &States[S_SPLASHX];
	info->radius = 2 * FRACUNIT;
	info->height = 4 * FRACUNIT;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF;
	info->flags2 = MF2_NOTELEPORT|MF2_LOGRAV|MF2_CANNOTPUSH;
	info->flags3 = MF3_DONTSPLASH;
}

IMPLEMENT_DEF_SERIAL (AWaterSplashBase, AActor)
REGISTER_ACTOR (AWaterSplashBase, Any);

FState AWaterSplashBase::States[] =
{
	S_NORMAL (SPSH, 'E',	5, NULL, &States[1]),
	S_NORMAL (SPSH, 'F',	5, NULL, &States[2]),
	S_NORMAL (SPSH, 'G',	5, NULL, &States[3]),
	S_NORMAL (SPSH, 'H',	5, NULL, &States[4]),
	S_NORMAL (SPSH, 'I',	5, NULL, &States[5]),
	S_NORMAL (SPSH, 'J',	5, NULL, &States[6]),
	S_NORMAL (SPSH, 'K',	5, NULL, NULL)
};

void AWaterSplashBase::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOCLIP;
	info->flags3 = MF3_DONTSPLASH;
}

// Lava ---------------------------------------------------------------------

IMPLEMENT_DEF_SERIAL (ALavaSplash, AActor);
REGISTER_ACTOR (ALavaSplash, Any);

FState ALavaSplash::States[] =
{
	S_BRIGHT (LVAS, 'A',	5, NULL, &States[1]),
	S_BRIGHT (LVAS, 'B',	5, NULL, &States[2]),
	S_BRIGHT (LVAS, 'C',	5, NULL, &States[3]),
	S_BRIGHT (LVAS, 'D',	5, NULL, &States[4]),
	S_BRIGHT (LVAS, 'E',	5, NULL, &States[5]),
	S_BRIGHT (LVAS, 'F',	5, NULL, NULL)
};

void ALavaSplash::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOCLIP;
	info->flags3 = MF3_DONTSPLASH;
}

IMPLEMENT_DEF_SERIAL (ALavaSmoke, AActor);
REGISTER_ACTOR (ALavaSmoke, Any);

FState ALavaSmoke::States[] =
{
	S_BRIGHT (LVAS, 'G',	5, NULL, &States[1]),
	S_BRIGHT (LVAS, 'H',	5, NULL, &States[2]),
	S_BRIGHT (LVAS, 'I',	5, NULL, &States[3]),
	S_BRIGHT (LVAS, 'J',	5, NULL, &States[4]),
	S_BRIGHT (LVAS, 'K',	5, NULL, NULL)
};

void ALavaSmoke::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOCLIP;
	info->translucency = gameinfo.gametype == GAME_Heretic ? HR_SHADOW : HX_SHADOW;
}

// Sludge -------------------------------------------------------------------

IMPLEMENT_DEF_SERIAL (ASludgeChunk, AActor);
REGISTER_ACTOR (ASludgeChunk, Any);

FState ASludgeChunk::States[] =
{
#define S_SLUDGECHUNK 0
	S_NORMAL (SLDG, 'A',	8, NULL, &States[S_SLUDGECHUNK+1]),
	S_NORMAL (SLDG, 'B',	8, NULL, &States[S_SLUDGECHUNK+2]),
	S_NORMAL (SLDG, 'C',	8, NULL, &States[S_SLUDGECHUNK+3]),
	S_NORMAL (SLDG, 'D',	8, NULL, NULL),

#define S_SLUDGECHUNKX (S_SLUDGECHUNK+4)
	S_NORMAL (SLDG, 'D',	6, NULL, NULL)
};

void ASludgeChunk::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_SLUDGECHUNK];
	info->deathstate = &States[S_SLUDGECHUNKX];
	info->radius = 2 * FRACUNIT;
	info->height = 4 * FRACUNIT;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF;
	info->flags2 = MF2_NOTELEPORT|MF2_LOGRAV|MF2_CANNOTPUSH;
	info->flags3 = MF3_DONTSPLASH;
}

IMPLEMENT_DEF_SERIAL (ASludgeSplash, AActor);
REGISTER_ACTOR (ASludgeSplash, Any);

FState ASludgeSplash::States[] =
{
	S_NORMAL (SLDG, 'E',	6, NULL, &States[1]),
	S_NORMAL (SLDG, 'F',	6, NULL, &States[2]),
	S_NORMAL (SLDG, 'G',	6, NULL, &States[3]),
	S_NORMAL (SLDG, 'H',	6, NULL, NULL)
};

void ASludgeSplash::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOCLIP;
	info->flags3 = MF3_DONTSPLASH;
}
