#include "actor.h"
#include "info.h"
#include "p_enemy.h"
#include "a_doomglobal.h"
#include "a_hereticglobal.h"
#include "a_pickups.h"
#include "a_action.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "hstrings.h"

// --- Pods -----------------------------------------------------------------

void A_PodPain (AActor *);
void A_RemovePod (AActor *);
void A_MakePod (AActor *);

// Pod ----------------------------------------------------------------------

class APod : public AExplosiveBarrel
{
	DECLARE_ACTOR (APod, AExplosiveBarrel);
public:
	void BeginPlay ();
	AActor *Generator;
};

IMPLEMENT_POINTY_SERIAL (APod, AExplosiveBarrel)
	DECLARE_POINTER (Generator)
END_POINTERS;

void APod::Serialize (FArchive &arc)
{
	Super::Serialize (arc);
	arc << Generator;
}

REGISTER_ACTOR (APod, Heretic);

FState APod::States[] =
{
#define S_POD_WAIT 0
	S_NORMAL (PPOD, 'A',   10, NULL 						, &States[S_POD_WAIT+0]),

#define S_POD_PAIN (S_POD_WAIT+1)
	S_NORMAL (PPOD, 'B',   14, A_PodPain					, &States[S_POD_WAIT+0]),

#define S_POD_DIE (S_POD_PAIN+1)
	S_BRIGHT (PPOD, 'C',	5, A_RemovePod					, &States[S_POD_DIE+1]),
	S_BRIGHT (PPOD, 'D',	5, A_Scream 					, &States[S_POD_DIE+2]),
	S_BRIGHT (PPOD, 'E',	5, A_Explode					, &States[S_POD_DIE+3]),
	S_BRIGHT (PPOD, 'F',   10, NULL 						, &AActor::States[S_FREETARGMOBJ]),

#define S_POD_GROW (S_POD_DIE+4)
	S_NORMAL (PPOD, 'I',	3, NULL 						, &States[S_POD_GROW+1]),
	S_NORMAL (PPOD, 'J',	3, NULL 						, &States[S_POD_GROW+2]),
	S_NORMAL (PPOD, 'K',	3, NULL 						, &States[S_POD_GROW+3]),
	S_NORMAL (PPOD, 'L',	3, NULL 						, &States[S_POD_GROW+4]),
	S_NORMAL (PPOD, 'M',	3, NULL 						, &States[S_POD_GROW+5]),
	S_NORMAL (PPOD, 'N',	3, NULL 						, &States[S_POD_GROW+6]),
	S_NORMAL (PPOD, 'O',	3, NULL 						, &States[S_POD_GROW+7]),
	S_NORMAL (PPOD, 'P',	3, NULL 						, &States[S_POD_WAIT+0])
};

void APod::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 2035;
	info->spawnstate = &States[S_POD_WAIT];
	info->spawnhealth = 45;
	info->painstate = &States[S_POD_PAIN];
	info->painchance = 255;
	info->deathstate = &States[S_POD_DIE];
	info->deathsound = "world/podexplode";
	info->radius = 16 * FRACUNIT;
	info->height = 54 * FRACUNIT;
	info->flags = MF_SOLID|MF_NOBLOOD|MF_SHOOTABLE|MF_DROPOFF;
	info->flags2 = MF2_WINDTHRUST|MF2_PUSHABLE|MF2_SLIDE|MF2_PASSMOBJ|MF2_TELESTOMP;
	info->flags3 = MF3_DONTMORPH;
}

void APod::BeginPlay ()
{
	Super::BeginPlay ();
	Generator = NULL;
}

// Pod goo (falls from pod when damaged) ------------------------------------

class APodGoo : public AActor
{
	DECLARE_ACTOR (APodGoo, AActor);
};

IMPLEMENT_DEF_SERIAL (APodGoo, AActor);
REGISTER_ACTOR (APodGoo, Heretic);

FState APodGoo::States[] =
{
#define S_PODGOO 0
	S_NORMAL (PPOD, 'G',	8, NULL 						, &States[S_PODGOO+1]),
	S_NORMAL (PPOD, 'H',	8, NULL 						, &States[S_PODGOO+0]),

#define S_PODGOOX (S_PODGOO+2)
	S_NORMAL (PPOD, 'G',   10, NULL 						, NULL)
};

void APodGoo::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_PODGOO];
	info->radius = 2 * FRACUNIT;
	info->height = 4 * FRACUNIT;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF;
	info->flags2 = MF2_NOTELEPORT|MF2_LOGRAV|MF2_CANNOTPUSH;
}

// Pod generator ------------------------------------------------------------

class APodGenerator : public AActor
{
	DECLARE_ACTOR (APodGenerator, AActor);
};

IMPLEMENT_DEF_SERIAL (APodGenerator, AActor);
REGISTER_ACTOR (APodGenerator, Heretic);

FState APodGenerator::States[] =
{
	S_NORMAL (TNT1, 'A',   35, A_MakePod					, &States[0])
};

void APodGenerator::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 43;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOSECTOR;
}

// --- Pod action functions -------------------------------------------------

//----------------------------------------------------------------------------
//
// PROC A_PodPain
//
//----------------------------------------------------------------------------

void A_PodPain (AActor *actor)
{
	int count;
	int chance;
	AActor *goo;

	chance = P_Random ();
	if (chance < 128)
	{
		return;
	}
	for (count = chance > 240 ? 2 : 1; count; count--)
	{
		goo = Spawn<APodGoo> (actor->x, actor->y, actor->z + 48*FRACUNIT);
		goo->target = actor;
		goo->momx = PS_Random() << 9;
		goo->momy = PS_Random() << 9;
		goo->momz = FRACUNIT/2 + (P_Random() << 9);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_RemovePod
//
//----------------------------------------------------------------------------

void A_RemovePod (AActor *actor)
{
	AActor *mo;

	if ( (mo = static_cast<APod *>(actor)->Generator) )
	{
		if (mo->special1 > 0)
		{
			mo->special1--;
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MakePod
//
//----------------------------------------------------------------------------

#define MAX_GEN_PODS 16

void A_MakePod (AActor *actor)
{
	APod *mo;
	fixed_t x;
	fixed_t y;
	fixed_t z;

	if (actor->special1 == MAX_GEN_PODS)
	{ // Too many generated pods
		return;
	}
	x = actor->x;
	y = actor->y;
	z = actor->z;
	mo = Spawn<APod> (x, y, ONFLOORZ);
	if (P_CheckPosition (mo, x, y) == false)
	{ // Didn't fit
		mo->Destroy ();
		return;
	}
	mo->SetState (&APod::States[S_POD_GROW]);
	P_ThrustMobj (mo, P_Random()<<24, (fixed_t)(4.5*FRACUNIT));
	S_Sound (mo, CHAN_BODY, "world/podgrow", 1, ATTN_IDLE);
	actor->special1++; // Increment generated pod count
	mo->Generator = actor; // Link the generator to the pod
	return;
}

// --- Teleglitter ----------------------------------------------------------

void A_SpawnTeleGlitter (AActor *);
void A_SpawnTeleGlitter2 (AActor *);
void A_AccTeleGlitter (AActor *);

// Teleglitter generator 1 --------------------------------------------------

class ATeleGlitterGenerator1 : public AActor
{
	DECLARE_ACTOR (ATeleGlitterGenerator1, AActor);
};

IMPLEMENT_DEF_SERIAL (ATeleGlitterGenerator1, AActor);
REGISTER_ACTOR (ATeleGlitterGenerator1, Heretic);

FState ATeleGlitterGenerator1::States[] =
{
	S_NORMAL (TGLT, 'A',	8, A_SpawnTeleGlitter			, &States[0])
};

void ATeleGlitterGenerator1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 74;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOSECTOR;
}

// Teleglitter generator 2 --------------------------------------------------

class ATeleGlitterGenerator2 : public AActor
{
	DECLARE_ACTOR (ATeleGlitterGenerator2, AActor);
};

IMPLEMENT_DEF_SERIAL (ATeleGlitterGenerator2, AActor);
REGISTER_ACTOR (ATeleGlitterGenerator2, Heretic);

FState ATeleGlitterGenerator2::States[] =
{
	S_NORMAL (TGLT, 'F',	8, A_SpawnTeleGlitter2			, &States[0])
};

void ATeleGlitterGenerator2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 52;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOSECTOR;
}

// Teleglitter 1 ------------------------------------------------------------

class ATeleGlitter1 : public AActor
{
	DECLARE_ACTOR (ATeleGlitter1, AActor);
};

IMPLEMENT_DEF_SERIAL (ATeleGlitter1, AActor);
REGISTER_ACTOR (ATeleGlitter1, Heretic);

FState ATeleGlitter1::States[] =
{
	S_BRIGHT (TGLT, 'A',	2, NULL 						, &States[1]),
	S_BRIGHT (TGLT, 'B',	2, A_AccTeleGlitter 			, &States[2]),
	S_BRIGHT (TGLT, 'C',	2, NULL 						, &States[3]),
	S_BRIGHT (TGLT, 'D',	2, A_AccTeleGlitter 			, &States[4]),
	S_BRIGHT (TGLT, 'E',	2, NULL 						, &States[0])
};

void ATeleGlitter1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY|MF_MISSILE;
}

// Teleglitter 2 ------------------------------------------------------------

class ATeleGlitter2 : public AActor
{
	DECLARE_ACTOR (ATeleGlitter2, AActor);
};

IMPLEMENT_DEF_SERIAL (ATeleGlitter2, AActor);
REGISTER_ACTOR (ATeleGlitter2, Heretic);

FState ATeleGlitter2::States[] =
{
	S_BRIGHT (TGLT, 'F',	2, NULL 						, &States[1]),
	S_BRIGHT (TGLT, 'G',	2, A_AccTeleGlitter 			, &States[2]),
	S_BRIGHT (TGLT, 'H',	2, NULL 						, &States[3]),
	S_BRIGHT (TGLT, 'I',	2, A_AccTeleGlitter 			, &States[4]),
	S_BRIGHT (TGLT, 'J',	2, NULL 						, &States[0])
};

void ATeleGlitter2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY|MF_MISSILE;
}

// --- Teleglitter action functions -----------------------------------------

//----------------------------------------------------------------------------
//
// PROC A_SpawnTeleGlitter
//
//----------------------------------------------------------------------------

void A_SpawnTeleGlitter (AActor *actor)
{
	AActor *mo;

	mo = Spawn<ATeleGlitter1> (
		actor->x+((P_Random()&31)-16)*FRACUNIT,
		actor->y+((P_Random()&31)-16)*FRACUNIT,
		actor->subsector->sector->floorheight);
	mo->momz = FRACUNIT/4;
}

//----------------------------------------------------------------------------
//
// PROC A_SpawnTeleGlitter2
//
//----------------------------------------------------------------------------

void A_SpawnTeleGlitter2 (AActor *actor)
{
	AActor *mo;

	mo = Spawn<ATeleGlitter2> (
		actor->x+((P_Random()&31)-16)*FRACUNIT,
		actor->y+((P_Random()&31)-16)*FRACUNIT,
		actor->subsector->sector->floorheight);
	mo->momz = FRACUNIT/4;
}

//----------------------------------------------------------------------------
//
// PROC A_AccTeleGlitter
//
//----------------------------------------------------------------------------

void A_AccTeleGlitter (AActor *actor)
{
	if (++actor->health > 35)
	{
		actor->momz += actor->momz/2;
	}
}

// Super map ----------------------------------------------------------------

class ASuperMap : public APickup
{
	DECLARE_ACTOR (ASuperMap, APickup);
protected:
	bool TryPickup (AActor *toucher)
	{
		return P_GivePower (toucher->player, pw_allmap);
	}
	const char *PickupMessage ()
	{
		return TXT_ITEMSUPERMAP;
	}
};

IMPLEMENT_DEF_SERIAL (ASuperMap, APickup);
REGISTER_ACTOR (ASuperMap, Heretic);

FState ASuperMap::States[] =
{
#define S_ITEM_SPMP 0
	S_NORMAL (SPMP, 'A',   -1, NULL 						, NULL)
};

void ASuperMap::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 35;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
}

// --- Volcano --------------------------------------------------------------

void A_VolcanoSet (AActor *);
void A_VolcanoBlast (AActor *);
void A_VolcBallImpact (AActor *);
extern void A_BeastPuff (AActor *);

// Volcano ------------------------------------------------------------------

class AVolcano : public AActor
{
	DECLARE_ACTOR (AVolcano, AActor);
};

IMPLEMENT_DEF_SERIAL (AVolcano, AActor);
REGISTER_ACTOR (AVolcano, Heretic);

FState AVolcano::States[] =
{
	S_NORMAL (VLCO, 'A',  350, NULL 					, &States[1]),
	S_NORMAL (VLCO, 'A',   35, A_VolcanoSet 			, &States[2]),
	S_NORMAL (VLCO, 'B',	3, NULL 					, &States[3]),
	S_NORMAL (VLCO, 'C',	3, NULL 					, &States[4]),
	S_NORMAL (VLCO, 'D',	3, NULL 					, &States[5]),
	S_NORMAL (VLCO, 'B',	3, NULL 					, &States[6]),
	S_NORMAL (VLCO, 'C',	3, NULL 					, &States[7]),
	S_NORMAL (VLCO, 'D',	3, NULL 					, &States[8]),
	S_NORMAL (VLCO, 'E',   10, A_VolcanoBlast			, &States[1])
};

void AVolcano::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 87;
	info->spawnstate = &States[0];
	info->radius = 12 * FRACUNIT;
	info->height = 20 * FRACUNIT;
	info->flags = MF_SOLID;
}

// Volcano blast ------------------------------------------------------------

class AVolcanoBlast : public AActor
{
	DECLARE_ACTOR (AVolcanoBlast, AActor);
};

IMPLEMENT_DEF_SERIAL (AVolcanoBlast, AActor);
REGISTER_ACTOR (AVolcanoBlast, Heretic);

FState AVolcanoBlast::States[] =
{
#define S_VOLCANOBALL 0
	S_NORMAL (VFBL, 'A',	4, A_BeastPuff				, &States[S_VOLCANOBALL+1]),
	S_NORMAL (VFBL, 'B',	4, A_BeastPuff				, &States[S_VOLCANOBALL+0]),

#define S_VOLCANOBALLX (S_VOLCANOBALL+2)
	S_NORMAL (XPL1, 'A',	4, A_VolcBallImpact 		, &States[S_VOLCANOBALLX+1]),
	S_NORMAL (XPL1, 'B',	4, NULL 					, &States[S_VOLCANOBALLX+2]),
	S_NORMAL (XPL1, 'C',	4, NULL 					, &States[S_VOLCANOBALLX+3]),
	S_NORMAL (XPL1, 'D',	4, NULL 					, &States[S_VOLCANOBALLX+4]),
	S_NORMAL (XPL1, 'E',	4, NULL 					, &States[S_VOLCANOBALLX+5]),
	S_NORMAL (XPL1, 'F',	4, NULL 					, NULL)
};

void AVolcanoBlast::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_VOLCANOBALL];
	info->deathstate = &States[S_VOLCANOBALLX];
	info->deathsound = "world/volcanoblast";
	info->speed = 2 * FRACUNIT;
	info->radius = 8 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 2;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF;
	info->flags2 = MF2_LOGRAV|MF2_NOTELEPORT|MF2_FIREDAMAGE;
}

// Volcano T Blast ----------------------------------------------------------

class AVolcanoTBlast : public AActor
{
	DECLARE_ACTOR (AVolcanoTBlast, AActor);
};

IMPLEMENT_DEF_SERIAL (AVolcanoTBlast, AActor);
REGISTER_ACTOR (AVolcanoTBlast, Heretic);

FState AVolcanoTBlast::States[] =
{
#define S_VOLCANOTBALL 0
	S_NORMAL (VTFB, 'A',	4, NULL 					, &States[S_VOLCANOTBALL+1]),
	S_NORMAL (VTFB, 'B',	4, NULL 					, &States[S_VOLCANOTBALL+0]),

#define S_VOLCANOTBALLX (S_VOLCANOTBALL+2)
	S_NORMAL (SFFI, 'C',	4, NULL 					, &States[S_VOLCANOTBALLX+1]),
	S_NORMAL (SFFI, 'B',	4, NULL 					, &States[S_VOLCANOTBALLX+2]),
	S_NORMAL (SFFI, 'A',	4, NULL 					, &States[S_VOLCANOTBALLX+3]),
	S_NORMAL (SFFI, 'B',	4, NULL 					, &States[S_VOLCANOTBALLX+4]),
	S_NORMAL (SFFI, 'C',	4, NULL 					, &States[S_VOLCANOTBALLX+5]),
	S_NORMAL (SFFI, 'D',	4, NULL 					, &States[S_VOLCANOTBALLX+6]),
	S_NORMAL (SFFI, 'E',	4, NULL 					, NULL)
};

void AVolcanoTBlast::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_VOLCANOTBALL];
	info->deathstate = &States[S_VOLCANOTBALLX];
	info->speed = 2 * FRACUNIT;
	info->radius = 8 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 1;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF;
	info->flags2 = MF2_LOGRAV|MF2_NOTELEPORT|MF2_FIREDAMAGE;
}

//----------------------------------------------------------------------------
//
// PROC A_VolcanoSet
//
//----------------------------------------------------------------------------

void A_VolcanoSet (AActor *volcano)
{
	volcano->tics = 105 + (P_Random() & 127);
}

//----------------------------------------------------------------------------
//
// PROC A_VolcanoBlast
//
//----------------------------------------------------------------------------

void A_VolcanoBlast (AActor *volcano)
{
	int i;
	int count;
	AActor *blast;
	angle_t angle;

	count = 1 + (P_Random() % 3);
	for (i = 0; i < count; i++)
	{
		blast = Spawn<AVolcanoBlast> (volcano->x, volcano->y,
			volcano->z + 44*FRACUNIT);
		blast->target = volcano;
		angle = P_Random () << 24;
		blast->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		blast->momx = FixedMul (1*FRACUNIT, finecosine[angle]);
		blast->momy = FixedMul (1*FRACUNIT, finesine[angle]);
		blast->momz = (FRACUNIT*5/2) + (P_Random() << 10);
		S_Sound (blast, CHAN_BODY, "volcano/shoot", 1, ATTN_NORM);
		P_CheckMissileSpawn (blast);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_VolcBallImpact
//
//----------------------------------------------------------------------------

void A_VolcBallImpact (AActor *ball)
{
	int i;
	AActor *tiny;
	angle_t angle;

	if (ball->z <= ball->floorz)
	{
		ball->flags |= MF_NOGRAVITY;
		ball->flags2 &= ~MF2_LOGRAV;
		ball->z += 28*FRACUNIT;
		//ball->momz = 3*FRACUNIT;
	}
	P_RadiusAttack (ball, ball->target, 25, 25, true, MOD_LAVA);
	for (i = 0; i < 4; i++)
	{
		tiny = Spawn<AVolcanoTBlast> (ball->x, ball->y, ball->z);
		tiny->target = ball;
		angle = i*ANG90;
		tiny->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		tiny->momx = FixedMul (FRACUNIT*7/10, finecosine[angle]);
		tiny->momy = FixedMul (FRACUNIT*7/10, finesine[angle]);
		tiny->momz = FRACUNIT + (P_Random() << 9);
		P_CheckMissileSpawn (tiny);
	}
}

