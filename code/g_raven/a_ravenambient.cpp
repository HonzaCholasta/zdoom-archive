#include "actor.h"
#include "info.h"
#include "s_sound.h"
#include "gi.h"

void A_WindSound (AActor *);
void A_WaterfallSound (AActor *);

// Wind ---------------------------------------------------------------------

class ASoundWind : public AActor
{
	DECLARE_ACTOR (ASoundWind, AActor);
};

IMPLEMENT_DEF_SERIAL (ASoundWind, AActor);
REGISTER_ACTOR (ASoundWind, Raven);

FState ASoundWind::States[] =
{
	S_NORMAL (TNT1, 'A',  100, A_WindSound, &States[0])
};

void ASoundWind::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = (gameinfo.gametype == GAME_Heretic) ? 42 : 1410;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOSECTOR;
}

// Waterfall ----------------------------------------------------------------

class ASoundWaterfall : public AActor
{
	DECLARE_ACTOR (ASoundWaterfall, AActor);
};

IMPLEMENT_DEF_SERIAL (ASoundWaterfall, AActor);
REGISTER_ACTOR (ASoundWaterfall, Raven);

FState ASoundWaterfall::States[] =
{
	S_NORMAL (TNT1, 'A',   85, A_WaterfallSound, &States[0])
};

void ASoundWaterfall::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 41;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOSECTOR;
}

//----------------------------------------------------------------------------
//
// PROC A_WindSound
//
//----------------------------------------------------------------------------

void A_WindSound (AActor *self)
{
	S_Sound (self, CHAN_BODY, "world/wind", 1, ATTN_NORM);
}

//----------------------------------------------------------------------------
//
// PROC A_WaterfallSound
//
//----------------------------------------------------------------------------

void A_WaterfallSound (AActor *self)
{
// Oddly, Hexen does not play any sound for the waterfall, even though it
// includes it as a thing. Since Hexen also doesn't define a "world/waterfall"
// sound, we won't play anything for it either, but people who want to hear
// it can do so by defining the sound.
//
// For Heretic, we *do* define "world/waterfall", so it will be audible in
// Heretic.

	S_Sound (self, CHAN_BODY, "world/waterfall", 1, ATTN_NORM);
}
