/*
** Decorations that do special things
*/

#include "actor.h"
#include "info.h"
#include "a_action.h"
#include "p_enemy.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_lnspec.h"

// SwitchableDecoration: Activate and Deactivate change state ---------------

class ASwitchableDecoration : public AActor
{
	DECLARE_STATELESS_ACTOR (ASwitchableDecoration, AActor);
public:
	void Activate (AActor *activator);
	void Deactivate (AActor *activator);
};

IMPLEMENT_DEF_SERIAL (ASwitchableDecoration, AActor);
REGISTER_ACTOR (ASwitchableDecoration, Any);

void ASwitchableDecoration::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
}

void ASwitchableDecoration::Activate (AActor *activator)
{
	SetState (GetInfo (this)->seestate);
}

void ASwitchableDecoration::Deactivate (AActor *activator)
{
	SetState (GetInfo (this)->meleestate);
}

// SwitchingDecoration: Only Activate changes state -------------------------

class ASwitchingDecoration : public ASwitchableDecoration
{
	DECLARE_STATELESS_ACTOR (ASwitchingDecoration, AActor);
public:
	void Deactivate (AActor *activator) {}
};

IMPLEMENT_DEF_SERIAL (ASwitchingDecoration, ASwitchableDecoration);
REGISTER_ACTOR (ASwitchingDecoration, Any);

void ASwitchingDecoration::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
}

// Winged Statue (no skull) -------------------------------------------------

class AZWingedStatueNoSkull : public ASwitchingDecoration
{
	DECLARE_ACTOR (AZWingedStatueNoSkull, ASwitchingDecoration);
};

IMPLEMENT_DEF_SERIAL (AZWingedStatueNoSkull, ASwitchingDecoration);
REGISTER_ACTOR (AZWingedStatueNoSkull, Hexen);

FState AZWingedStatueNoSkull::States[] =
{
	S_NORMAL (STWN, 'A', -1, NULL, NULL),
	S_NORMAL (STWN, 'B', -1, NULL, NULL)
};

void AZWingedStatueNoSkull::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 9011;
	info->radius = 10*FRACUNIT;
	info->height = 62*FRACUNIT;
	info->flags = MF_SOLID;
	info->spawnstate = &States[0];
	info->seestate = &States[0];
	info->meleestate = &States[1];
}

// Gem pedestal -------------------------------------------------------------

class AZGemPedestal : public ASwitchingDecoration
{
	DECLARE_ACTOR (AZGemPedestal, ASwitchingDecoration);
};

IMPLEMENT_DEF_SERIAL (AZGemPedestal, ASwitchingDecoration);
REGISTER_ACTOR (AZGemPedestal, Hexen);

FState AZGemPedestal::States[] =
{
	S_NORMAL (GMPD, 'A', -1, NULL, NULL),
	S_NORMAL (GMPD, 'B', -1, NULL, NULL)
};

void AZGemPedestal::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 9012;
	info->radius = 10*FRACUNIT;
	info->height = 40*FRACUNIT;
	info->flags = MF_SOLID;
	info->spawnstate = &States[0];
	info->seestate = &States[0];
	info->meleestate = &States[1];
}

// Tree (destructible) ------------------------------------------------------

class ATreeDestructible : public AActor
{
	DECLARE_ACTOR (ATreeDestructible, AActor);
public:
	void GetExplodeParms (int &damage, int &distance, bool &hurtSource);
	void Die (AActor *source, AActor *inflictor);
};

IMPLEMENT_DEF_SERIAL (ATreeDestructible, AActor);
REGISTER_ACTOR (ATreeDestructible, Hexen);

FState ATreeDestructible::States[] =
{
#define S_ZTREEDESTRUCTIBLE 0
	S_NORMAL (TRDT, 'A',   -1, NULL 					, NULL),

#define S_ZTREEDES_D (S_ZTREEDESTRUCTIBLE+1)
	S_NORMAL (TRDT, 'B',	5, NULL 					, &States[S_ZTREEDES_D+1]),
	S_NORMAL (TRDT, 'C',	5, A_Scream 				, &States[S_ZTREEDES_D+2]),
	S_NORMAL (TRDT, 'D',	5, NULL 					, &States[S_ZTREEDES_D+3]),
	S_NORMAL (TRDT, 'E',	5, NULL 					, &States[S_ZTREEDES_D+4]),
	S_NORMAL (TRDT, 'F',	5, NULL 					, &States[S_ZTREEDES_D+5]),
	S_NORMAL (TRDT, 'G',   -1, NULL 					, NULL),

#define S_ZTREEDES_X (S_ZTREEDES_D+6)
	S_BRIGHT (TRDT, 'H',	5, A_Pain 					, &States[S_ZTREEDES_X+1]),
	S_BRIGHT (TRDT, 'I',	5, NULL 					, &States[S_ZTREEDES_X+2]),
	S_BRIGHT (TRDT, 'J',	5, NULL 					, &States[S_ZTREEDES_X+3]),
	S_BRIGHT (TRDT, 'K',	5, NULL 					, &States[S_ZTREEDES_X+4]),
	S_BRIGHT (TRDT, 'L',	5, NULL 					, &States[S_ZTREEDES_X+5]),
	S_BRIGHT (TRDT, 'M',	5, A_Explode				, &States[S_ZTREEDES_X+6]),
	S_BRIGHT (TRDT, 'N',	5, NULL 					, &States[S_ZTREEDES_X+7]),
	S_NORMAL (TRDT, 'O',	5, NULL 					, &States[S_ZTREEDES_X+8]),
	S_NORMAL (TRDT, 'P',	5, NULL 					, &States[S_ZTREEDES_X+9]),
	S_NORMAL (TRDT, 'Q',   -1, NULL 					, NULL)
};

void ATreeDestructible::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 8062;
	info->spawnstate = &States[S_ZTREEDESTRUCTIBLE];
	info->spawnhealth = 70;
	info->deathstate = &States[S_ZTREEDES_D];
	info->bdeathstate = &States[S_ZTREEDES_X];
	info->deathsound = "TreeBreak";
	info->painsound = "TreeExplode";
	info->radius = 15*FRACUNIT;
	info->height = 180*FRACUNIT;
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD;
}

void ATreeDestructible::GetExplodeParms (int &damage, int &distance, bool &hurtSource)
{
	damage = 10;
}

void ATreeDestructible::Die (AActor *source, AActor *inflictor)
{
	Super::Die (source, inflictor);
	height = 24*FRACUNIT;
	flags &= ~MF_CORPSE;
}

// Pottery1 ------------------------------------------------------------------

void A_PotteryExplode (AActor *);
void A_PotteryChooseBit (AActor *);
void A_PotteryCheck (AActor *);

class APottery1 : public AActor
{
	DECLARE_ACTOR (APottery1, AActor);
public:
	void HitFloor ();
};

IMPLEMENT_DEF_SERIAL (APottery1, AActor);
REGISTER_ACTOR (APottery1, Hexen);

FState APottery1::States[] =
{
#define S_ZPOTTERY 0
	S_NORMAL (POT1, 'A',   -1, NULL 					, NULL),

#define S_ZPOTTERY_EXPLODE (S_ZPOTTERY+1)
	S_NORMAL (POT1, 'A',	0, A_PotteryExplode 		, NULL)
};

void APottery1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 104;
	info->spawnstate = &States[S_ZPOTTERY];
	info->spawnhealth = 15;
	info->deathstate = &States[S_ZPOTTERY_EXPLODE];
	info->radius = 10*FRACUNIT;
	info->height = 32*FRACUNIT;
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD|MF_DROPOFF;
	info->flags2 = MF2_SLIDE|MF2_PUSHABLE|MF2_TELESTOMP|MF2_PASSMOBJ;
}

void APottery1::HitFloor ()
{
	Super::HitFloor ();
	P_DamageMobj (this, NULL, NULL, 25);
}

// Pottery2 -----------------------------------------------------------------

class APottery2 : public APottery1
{
	DECLARE_ACTOR (APottery2, APottery1);
};

IMPLEMENT_DEF_SERIAL (APottery2, APottery1);
REGISTER_ACTOR (APottery2, Hexen);

FState APottery2::States[] =
{
	S_NORMAL (POT2, 'A',   -1, NULL 					, NULL),
};

void APottery2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 105;
	info->spawnstate = &States[0];
	info->height = 25*FRACUNIT;
}

// Pottery3 -----------------------------------------------------------------

class APottery3 : public APottery1
{
	DECLARE_ACTOR (APottery3, APottery1);
};

IMPLEMENT_DEF_SERIAL (APottery3, APottery1);
REGISTER_ACTOR (APottery3, Hexen);

FState APottery3::States[] =
{
	S_NORMAL (POT3, 'A',   -1, NULL 					, NULL),
};

void APottery3::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 106;
	info->spawnstate = &States[0];
	info->height = 25*FRACUNIT;
}

// Pottery Bit --------------------------------------------------------------

class APotteryBit : public AActor
{
	DECLARE_ACTOR (APotteryBit, AActor);
};

IMPLEMENT_DEF_SERIAL (APotteryBit, AActor);
REGISTER_ACTOR (APotteryBit, Hexen);

FState APotteryBit::States[] =
{
#define S_POTTERYBIT 0
	S_NORMAL (PBIT, 'A',   -1, NULL 					, NULL),
	S_NORMAL (PBIT, 'B',   -1, NULL 					, NULL),
	S_NORMAL (PBIT, 'C',   -1, NULL 					, NULL),
	S_NORMAL (PBIT, 'D',   -1, NULL 					, NULL),
	S_NORMAL (PBIT, 'E',   -1, NULL 					, NULL),

#define S_POTTERYBIT_EX0 (S_POTTERYBIT+5)
	S_NORMAL (PBIT, 'F',	0, A_PotteryChooseBit		, NULL),

#define S_POTTERYBIT_EX1 (S_POTTERYBIT_EX0+1)
	S_NORMAL (PBIT, 'F',  140, NULL 					, &States[S_POTTERYBIT_EX0+1]),
	S_NORMAL (PBIT, 'F',	1, A_PotteryCheck			, NULL),

#define S_POTTERYBIT_EX2 (S_POTTERYBIT_EX1+2)
	S_NORMAL (PBIT, 'G',  140, NULL 					, &States[S_POTTERYBIT_EX1+1]),
	S_NORMAL (PBIT, 'G',	1, A_PotteryCheck			, NULL),

#define S_POTTERYBIT_EX3 (S_POTTERYBIT_EX2+2)
	S_NORMAL (PBIT, 'H',  140, NULL 					, &States[S_POTTERYBIT_EX2+1]),
	S_NORMAL (PBIT, 'H',	1, A_PotteryCheck			, NULL),

#define S_POTTERYBIT_EX4 (S_POTTERYBIT_EX3+2)
	S_NORMAL (PBIT, 'I',  140, NULL 					, &States[S_POTTERYBIT_EX3+1]),
	S_NORMAL (PBIT, 'I',	1, A_PotteryCheck			, NULL),

#define S_POTTERYBIT_EX5 (S_POTTERYBIT_EX4+2)
	S_NORMAL (PBIT, 'J',  140, NULL 					, &States[S_POTTERYBIT_EX4+1]),
	S_NORMAL (PBIT, 'J',	1, A_PotteryCheck			, NULL)
};

void APotteryBit::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_POTTERYBIT];
	info->deathstate = &States[S_POTTERYBIT_EX0];
	info->radius = 5*FRACUNIT;
	info->height = 5*FRACUNIT;
	info->flags = MF_MISSILE;
	info->flags2 = MF2_NOTELEPORT;
}

//============================================================================
//
// A_PotteryExplode
//
//============================================================================

void A_PotteryExplode (AActor *actor)
{
	AActor *mo = NULL;
	int i;

	for(i = (P_Random()&3)+3; i; i--)
	{
		mo = Spawn<APotteryBit> (actor->x, actor->y, actor->z);
		mo->SetState (GetInfo (mo)->spawnstate + (P_Random()%5));
		if (mo)
		{
			mo->momz = ((P_Random()&7)+5)*(3*FRACUNIT/4);
			mo->momx = (PS_Random())<<(FRACBITS-6);
			mo->momy = (PS_Random())<<(FRACBITS-6);
		}
	}
	S_Sound (mo, CHAN_BODY, "PotteryExplode", 1, ATTN_NORM);
	if (SpawnableThings[actor->args[0]])
	{ // Spawn an item
		if (!(dmflags & DF_NO_MONSTERS) 
		|| !(SpawnableThings[actor->args[0]]->ActorInfo->flags & MF_COUNTKILL))
		{ // Only spawn monsters if not -nomonsters
			Spawn (SpawnableThings[actor->args[0]],
				actor->x, actor->y, actor->z);
		}
	}
}

//============================================================================
//
// A_PotteryChooseBit
//
//============================================================================

void A_PotteryChooseBit (AActor *actor)
{
	actor->SetState (GetInfo (actor)->deathstate+1 + 2*(P_Random()%5));
	actor->tics = 256+(P_Random()<<1);
}

//============================================================================
//
// A_PotteryCheck
//
//============================================================================

void A_PotteryCheck (AActor *actor)
{
	int i;

	for(i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i])
		{
			AActor *pmo = players[i].mo;
			if (P_CheckSight (actor, pmo) && (abs (R_PointToAngle2 (pmo->x,
				pmo->y, actor->x, actor->y) - pmo->angle) <= ANGLE_45))
			{ // Previous state (pottery bit waiting state)
				actor->SetState (actor->state - 1);
				return;
			}
		}
	}		
}

// Blood pool ---------------------------------------------------------------

class ABloodPool : public AActor
{
	DECLARE_ACTOR (ABloodPool, AActor);
};

IMPLEMENT_DEF_SERIAL (ABloodPool, AActor);
REGISTER_ACTOR (ABloodPool, Hexen);

FState ABloodPool::States[] =
{
	S_NORMAL (BDPL, 'A',   -1, NULL 					, NULL)
};

void ABloodPool::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 111;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP;
}

// Lynched corpse (no heart) ------------------------------------------------

void A_CorpseBloodDrip (AActor *);

class AZCorpseLynchedNoHeart : public AActor
{
	DECLARE_ACTOR (AZCorpseLynchedNoHeart, AActor);
public:
	void PostBeginPlay ();
};

IMPLEMENT_DEF_SERIAL (AZCorpseLynchedNoHeart, AActor);
REGISTER_ACTOR (AZCorpseLynchedNoHeart, Hexen);

FState AZCorpseLynchedNoHeart::States[] =
{
	S_NORMAL (CPS5, 'A',  140, A_CorpseBloodDrip		, &States[0])
};

void AZCorpseLynchedNoHeart::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 109;
	info->spawnstate = &States[0];
	info->radius = 10*FRACUNIT;
	info->height = 100*FRACUNIT;
	info->flags = MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY;
}

void AZCorpseLynchedNoHeart::PostBeginPlay ()
{
	Super::PostBeginPlay ();
	Spawn<ABloodPool> (x, y, ONFLOORZ);
}

// CorpseBloodDrip ----------------------------------------------------------

class ACorpseBloodDrip : public AActor
{
	DECLARE_ACTOR (ACorpseBloodDrip, AActor);
};

IMPLEMENT_DEF_SERIAL (ACorpseBloodDrip, AActor);
REGISTER_ACTOR (ACorpseBloodDrip, Hexen);

FState ACorpseBloodDrip::States[] =
{
#define S_CORPSEBLOODDRIP 0
	S_NORMAL (BDRP, 'A',   -1, NULL 					, NULL),

#define S_CORPSEBLOODDRIP_X (S_CORPSEBLOODDRIP+1)
	S_NORMAL (BDSH, 'A',	3, NULL 					, &States[S_CORPSEBLOODDRIP_X+1]),
	S_NORMAL (BDSH, 'B',	3, NULL 					, &States[S_CORPSEBLOODDRIP_X+2]),
	S_NORMAL (BDSH, 'C',	2, NULL 					, &States[S_CORPSEBLOODDRIP_X+3]),
	S_NORMAL (BDSH, 'D',	2, NULL 					, NULL)
};

void ACorpseBloodDrip::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_CORPSEBLOODDRIP];
	info->deathstate = &States[S_CORPSEBLOODDRIP_X];
	info->radius = FRACUNIT;
	info->height = 4*FRACUNIT;
	info->flags = MF_MISSILE;
	info->flags2 = MF2_LOGRAV;
}

//============================================================================
//
// A_CorpseBloodDrip
//
//============================================================================

void A_CorpseBloodDrip (AActor *actor)
{
	if (P_Random() <= 128)
	{
		Spawn<ACorpseBloodDrip> (actor->x, actor->y,
			actor->z + actor->height/2);
	}
}

// Corpse bit ---------------------------------------------------------------

class ACorpseBit : public AActor
{
	DECLARE_ACTOR (ACorpseBit, AActor);
};

IMPLEMENT_DEF_SERIAL (ACorpseBit, AActor);
REGISTER_ACTOR (ACorpseBit, Hexen);

FState ACorpseBit::States[] =
{
	S_NORMAL (CPB1, 'A',   -1, NULL 					, NULL),
	S_NORMAL (CPB2, 'A',   -1, NULL 					, NULL),
	S_NORMAL (CPB3, 'A',   -1, NULL 					, NULL),
	S_NORMAL (CPB4, 'A',   -1, NULL 					, NULL)
};

void ACorpseBit::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->radius = 5*FRACUNIT;
	info->height = 5*FRACUNIT;
	info->flags = MF_NOBLOCKMAP;
	info->flags2 = MF2_TELESTOMP;
}

// Corpse (sitting, splatterable) -------------------------------------------

void A_CorpseExplode (AActor *);

class AZCorpseSitting : public AActor
{
	DECLARE_ACTOR (AZCorpseSitting, AActor);
};

IMPLEMENT_DEF_SERIAL (AZCorpseSitting, AActor);
REGISTER_ACTOR (AZCorpseSitting, Hexen);

FState AZCorpseSitting::States[] =
{
	S_NORMAL (CPS6, 'A',   -1, NULL 					, NULL),
	S_NORMAL (CPS6, 'A',	1, A_CorpseExplode			, NULL)
};

void AZCorpseSitting::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 110;
	info->spawnstate = &States[0];
	info->spawnhealth = 30;
	info->deathstate = &States[1];
	info->deathsound = "FireDemonDeath";
	info->radius = 15*FRACUNIT;
	info->height = 35*FRACUNIT;
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD;
}

//============================================================================
//
// A_CorpseExplode
//
//============================================================================

void A_CorpseExplode (AActor *actor)
{
	AActor *mo;
	int i;

	for (i = (P_Random()&3)+3; i; i--)
	{
		mo = Spawn<ACorpseBit> (actor->x, actor->y, actor->z);
		mo->SetState (GetInfo (mo)->spawnstate + (P_Random()%3));
		if (mo)
		{
			mo->momz = ((P_Random()&7)+5)*(3*FRACUNIT/4);
			mo->momx = PS_Random()<<(FRACBITS-6);
			mo->momy = PS_Random()<<(FRACBITS-6);
		}
	}
	// Spawn a skull
	mo = Spawn<ACorpseBit> (actor->x, actor->y, actor->z);
	mo->SetState (GetInfo (mo)->spawnstate + 3);
	if (mo)
	{
		mo->momz = ((P_Random()&7)+5)*(3*FRACUNIT/4);
		mo->momx = PS_Random()<<(FRACBITS-6);
		mo->momy = PS_Random()<<(FRACBITS-6);
	}
	S_Sound (actor, CHAN_BODY, GetInfo (actor)->deathsound, 1, ATTN_IDLE);
	actor->Destroy ();
}

// Leaf Spawner -------------------------------------------------------------

void A_LeafSpawn (AActor *);
void A_LeafThrust (AActor *);
void A_LeafCheck (AActor *);

class ALeafSpawner : public AActor
{
	DECLARE_ACTOR (ALeafSpawner, AActor);
};

IMPLEMENT_DEF_SERIAL (ALeafSpawner, AActor);
REGISTER_ACTOR (ALeafSpawner, Hexen);

FState ALeafSpawner::States[] =
{
	S_NORMAL (MAN1, 'A',   20, A_LeafSpawn				, &States[0])
};

void ALeafSpawner::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 113;
	info->spawnstate = &States[0];
	info->flags = MF_NOBLOCKMAP|MF_NOSECTOR;
	info->flags2 = MF2_DONTDRAW;
}

// Leaves -------------------------------------------------------------------

class ALeaf1 : public AActor
{
	DECLARE_ACTOR (ALeaf1, AActor);
};

IMPLEMENT_DEF_SERIAL (ALeaf1, AActor);
REGISTER_ACTOR (ALeaf1, Hexen);

FState ALeaf1::States[] =
{
#define S_LEAF1 0
	S_NORMAL (LEF1, 'A',	4, NULL 					, &States[S_LEAF1+1]),
	S_NORMAL (LEF1, 'B',	4, NULL 					, &States[S_LEAF1+2]),
	S_NORMAL (LEF1, 'C',	4, NULL 					, &States[S_LEAF1+3]),
	S_NORMAL (LEF1, 'D',	4, A_LeafThrust 			, &States[S_LEAF1+4]),
	S_NORMAL (LEF1, 'E',	4, NULL 					, &States[S_LEAF1+5]),
	S_NORMAL (LEF1, 'F',	4, NULL 					, &States[S_LEAF1+6]),
	S_NORMAL (LEF1, 'G',	4, NULL 					, &States[S_LEAF1+7]),
	S_NORMAL (LEF1, 'H',	4, A_LeafThrust 			, &States[S_LEAF1+8]),
	S_NORMAL (LEF1, 'I',	4, NULL 					, &States[S_LEAF1+9]),
	S_NORMAL (LEF1, 'A',	4, NULL 					, &States[S_LEAF1+10]),
	S_NORMAL (LEF1, 'B',	4, NULL 					, &States[S_LEAF1+11]),
	S_NORMAL (LEF1, 'C',	4, A_LeafThrust 			, &States[S_LEAF1+12]),
	S_NORMAL (LEF1, 'D',	4, NULL 					, &States[S_LEAF1+13]),
	S_NORMAL (LEF1, 'E',	4, NULL 					, &States[S_LEAF1+14]),
	S_NORMAL (LEF1, 'F',	4, NULL 					, &States[S_LEAF1+15]),
	S_NORMAL (LEF1, 'G',	4, A_LeafThrust 			, &States[S_LEAF1+16]),
	S_NORMAL (LEF1, 'H',	4, NULL 					, &States[S_LEAF1+17]),
	S_NORMAL (LEF1, 'I',	4, NULL 					, NULL),

#define S_LEAF_X (S_LEAF1+18)
	S_NORMAL (LEF3, 'D',   10, A_LeafCheck				, &States[S_LEAF_X+0])
};

void ALeaf1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_LEAF1];
	info->deathstate = &States[S_LEAF_X];
	info->radius = 2*FRACUNIT;
	info->height = 4*FRACUNIT;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE;
	info->flags2 = MF2_NOTELEPORT|MF2_LOGRAV;
	info->flags3 = MF3_DONTSPLASH;
}

class ALeaf2 : public ALeaf1
{
	DECLARE_ACTOR (ALeaf2, ALeaf1);
};

IMPLEMENT_DEF_SERIAL (ALeaf2, ALeaf1);
REGISTER_ACTOR (ALeaf2, Hexen);

FState ALeaf2::States[] =
{
#define S_LEAF2 0
	S_NORMAL (LEF2, 'A',	4, NULL 					, &States[S_LEAF2+1]),
	S_NORMAL (LEF2, 'B',	4, NULL 					, &States[S_LEAF2+2]),
	S_NORMAL (LEF2, 'C',	4, NULL 					, &States[S_LEAF2+3]),
	S_NORMAL (LEF2, 'D',	4, A_LeafThrust 			, &States[S_LEAF2+4]),
	S_NORMAL (LEF2, 'E',	4, NULL 					, &States[S_LEAF2+5]),
	S_NORMAL (LEF2, 'F',	4, NULL 					, &States[S_LEAF2+6]),
	S_NORMAL (LEF2, 'G',	4, NULL 					, &States[S_LEAF2+7]),
	S_NORMAL (LEF2, 'H',	4, A_LeafThrust 			, &States[S_LEAF2+8]),
	S_NORMAL (LEF2, 'I',	4, NULL 					, &States[S_LEAF2+9]),
	S_NORMAL (LEF2, 'A',	4, NULL 					, &States[S_LEAF2+10]),
	S_NORMAL (LEF2, 'B',	4, NULL 					, &States[S_LEAF2+11]),
	S_NORMAL (LEF2, 'C',	4, A_LeafThrust 			, &States[S_LEAF2+12]),
	S_NORMAL (LEF2, 'D',	4, NULL 					, &States[S_LEAF2+13]),
	S_NORMAL (LEF2, 'E',	4, NULL 					, &States[S_LEAF2+14]),
	S_NORMAL (LEF2, 'F',	4, NULL 					, &States[S_LEAF2+15]),
	S_NORMAL (LEF2, 'G',	4, A_LeafThrust 			, &States[S_LEAF2+16]),
	S_NORMAL (LEF2, 'H',	4, NULL 					, &States[S_LEAF2+17]),
	S_NORMAL (LEF2, 'I',	4, NULL 					, NULL)
};

void ALeaf2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_LEAF2];
}

//============================================================================
//
// A_LeafSpawn
//
//============================================================================

void A_LeafSpawn (AActor *actor)
{
	AActor *mo;
	int i;

	for (i = (P_Random()&3)+1; i; i--)
	{
		mo = Spawn (P_Random()&1 ? RUNTIME_CLASS(ALeaf1) : RUNTIME_CLASS(ALeaf2),
			actor->x + (PS_Random()<<14),
			actor->y + (PS_Random()<<14),
			actor->z + (P_Random()<<14));
		if (mo)
		{
			P_ThrustMobj (mo, actor->angle, (P_Random()<<9)+3*FRACUNIT);
			mo->target = actor;
			mo->special1 = 0;
		}
	}
}

//============================================================================
//
// A_LeafThrust
//
//============================================================================

void A_LeafThrust (AActor *actor)
{
	if (P_Random() <= 96)
	{
		actor->momz += (P_Random()<<9)+FRACUNIT;
	}
}

//============================================================================
//
// A_LeafCheck
//
//============================================================================

void A_LeafCheck (AActor *actor)
{
	actor->special1++;
	if (actor->special1 >= 20)
	{
		actor->SetState (NULL);
		return;
	}
	if (P_Random() > 64)
	{
		if (!actor->momx && !actor->momy)
		{
			P_ThrustMobj (actor, actor->target->angle,
				(P_Random()<<9)+FRACUNIT);
		}
		return;
	}
	actor->SetState (GetInfo (actor)->spawnstate + 7);
	actor->momz = (P_Random()<<9)+FRACUNIT;
	P_ThrustMobj (actor, actor->target->angle, (P_Random()<<9)+2*FRACUNIT);
	actor->flags |= MF_MISSILE;
}

// Torch base class ---------------------------------------------------------

// Twined torch -------------------------------------------------------------

class AZTwinedTorch : public ASwitchableDecoration
{
	DECLARE_ACTOR (AZTwinedTorch, ASwitchableDecoration);
public:
	void Activate (AActor *activator);
};

IMPLEMENT_DEF_SERIAL (AZTwinedTorch, ASwitchableDecoration);
REGISTER_ACTOR (AZTwinedTorch, Hexen);

FState AZTwinedTorch::States[] =
{
#define S_ZTWINEDTORCH 0
	S_BRIGHT (TWTR, 'A',	4, NULL 					, &States[S_ZTWINEDTORCH+1]),
	S_BRIGHT (TWTR, 'B',	4, NULL 					, &States[S_ZTWINEDTORCH+2]),
	S_BRIGHT (TWTR, 'C',	4, NULL 					, &States[S_ZTWINEDTORCH+3]),
	S_BRIGHT (TWTR, 'D',	4, NULL 					, &States[S_ZTWINEDTORCH+4]),
	S_BRIGHT (TWTR, 'E',	4, NULL 					, &States[S_ZTWINEDTORCH+5]),
	S_BRIGHT (TWTR, 'F',	4, NULL 					, &States[S_ZTWINEDTORCH+6]),
	S_BRIGHT (TWTR, 'G',	4, NULL 					, &States[S_ZTWINEDTORCH+7]),
	S_BRIGHT (TWTR, 'H',	4, NULL 					, &States[S_ZTWINEDTORCH+0]),

#define S_ZTWINEDTORCH_UNLIT (S_ZTWINEDTORCH+8)
	S_NORMAL (TWTR, 'I',   -1, NULL 					, NULL)
};

void AZTwinedTorch::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 116;
	info->spawnstate = &States[S_ZTWINEDTORCH];
	info->seestate = &States[S_ZTWINEDTORCH];
	info->meleestate = &States[S_ZTWINEDTORCH_UNLIT];
	info->radius = 10*FRACUNIT;
	info->height = 64*FRACUNIT;
	info->flags = MF_SOLID;
}

void AZTwinedTorch::Activate (AActor *activator)
{
	Super::Activate (activator);
	S_Sound (this, CHAN_BODY, "Ignite", 1, ATTN_NORM);
}

class AZTwinedTorchUnlit : public AZTwinedTorch
{
	DECLARE_STATELESS_ACTOR (AZTwinedTorchUnlit, AZTwinedTorch);
};

IMPLEMENT_DEF_SERIAL (AZTwinedTorchUnlit, AZTwinedTorch);
REGISTER_ACTOR (AZTwinedTorchUnlit, Hexen);

void AZTwinedTorchUnlit::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 117;
	info->spawnstate = info->meleestate;
}

// Wall torch ---------------------------------------------------------------

class AZWallTorch : public ASwitchableDecoration
{
	DECLARE_ACTOR (AZWallTorch, ASwitchableDecoration);
public:
	void Activate (AActor *activator);
};

IMPLEMENT_DEF_SERIAL (AZWallTorch, ASwitchableDecoration);
REGISTER_ACTOR (AZWallTorch, Hexen);

FState AZWallTorch::States[] =
{
#define S_ZWALLTORCH 0
	S_BRIGHT (WLTR, 'A',	5, NULL 					, &States[S_ZWALLTORCH+1]),
	S_BRIGHT (WLTR, 'B',	5, NULL 					, &States[S_ZWALLTORCH+2]),
	S_BRIGHT (WLTR, 'C',	5, NULL 					, &States[S_ZWALLTORCH+3]),
	S_BRIGHT (WLTR, 'D',	5, NULL 					, &States[S_ZWALLTORCH+4]),
	S_BRIGHT (WLTR, 'E',	5, NULL 					, &States[S_ZWALLTORCH+5]),
	S_BRIGHT (WLTR, 'F',	5, NULL 					, &States[S_ZWALLTORCH+6]),
	S_BRIGHT (WLTR, 'G',	5, NULL 					, &States[S_ZWALLTORCH+7]),
	S_BRIGHT (WLTR, 'H',	5, NULL 					, &States[S_ZWALLTORCH+0]),

#define S_ZWALLTORCH_U (S_ZWALLTORCH+8)
	S_NORMAL (WLTR, 'I',   -1, NULL 					, NULL)
};

void AZWallTorch::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 54;
	info->spawnstate = &States[S_ZWALLTORCH];
	info->seestate = &States[S_ZWALLTORCH];
	info->meleestate = &States[S_ZWALLTORCH_U];
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
}

void AZWallTorch::Activate (AActor *activator)
{
	Super::Activate (activator);
	S_Sound (this, CHAN_BODY, "Ignite", 1, ATTN_NORM);
}

class AZWallTorchUnlit : public AZWallTorch
{
	DECLARE_STATELESS_ACTOR (AZWallTorchUnlit, AZWallTorch);
};

IMPLEMENT_DEF_SERIAL (AZWallTorchUnlit, AZWallTorch);
REGISTER_ACTOR (AZWallTorchUnlit, Hexen);

void AZWallTorchUnlit::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 55;
	info->spawnstate = info->meleestate;
}

// Shrub1 -------------------------------------------------------------------

void A_TreeDeath (AActor *);

class AZShrub1 : public AActor
{
	DECLARE_ACTOR (AZShrub1, AActor);
};

IMPLEMENT_DEF_SERIAL (AZShrub1, AActor);
REGISTER_ACTOR (AZShrub1, Hexen);

FState AZShrub1::States[] =
{
#define S_ZSHRUB1 0
	S_NORMAL (SHB1, 'A',   -1, NULL 					, NULL),

#define S_ZSHRUB1_DIE (S_ZSHRUB1+1)
	S_NORMAL (SHB1, 'A',	1, A_TreeDeath				, &States[S_ZSHRUB1+0]),

#define S_ZSHRUB1_X (S_ZSHRUB1_DIE+1)
	S_BRIGHT (SHB1, 'B',	7, NULL 					, &States[S_ZSHRUB1_X+1]),
	S_BRIGHT (SHB1, 'C',	6, A_Scream 				, &States[S_ZSHRUB1_X+2]),
	S_BRIGHT (SHB1, 'D',	5, NULL 					, NULL)
};

void AZShrub1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 8101;
	info->spawnstate = &States[S_ZSHRUB1];
	info->meleestate = &States[S_ZSHRUB1_X];
	info->deathstate = &States[S_ZSHRUB1_DIE];
	info->deathsound = "TreeExplode";
	info->radius = 8*FRACUNIT;
	info->height = 24*FRACUNIT;
	info->mass = MAXINT;
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD;
}

// Shrub2 -------------------------------------------------------------------

class AZShrub2 : public AActor
{
	DECLARE_ACTOR (AZShrub2, AActor);
public:
	void GetDamageParms (int &damage, int &distance, bool hurtSrc);
};

IMPLEMENT_DEF_SERIAL (AZShrub2, AActor);
REGISTER_ACTOR (AZShrub2, Hexen);

FState AZShrub2::States[] =
{
#define S_ZSHRUB2 0
	S_NORMAL (SHB2, 'A',   -1, NULL 					, NULL),

#define S_ZSHRUB2_DIE (S_ZSHRUB2+1)
	S_NORMAL (SHB2, 'A',	1, A_TreeDeath				, &States[S_ZSHRUB2+1]),

#define S_ZSHRUB2_X (S_ZSHRUB2_DIE+1)
	S_BRIGHT (SHB2, 'B',	7, NULL 					, &States[S_ZSHRUB2_X+1]),
	S_BRIGHT (SHB2, 'C',	6, A_Scream 				, &States[S_ZSHRUB2_X+2]),
	S_BRIGHT (SHB2, 'D',	5, A_Explode				, &States[S_ZSHRUB2_X+3]),
	S_BRIGHT (SHB2, 'E',	5, NULL 					, NULL)
};

void AZShrub2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 8102;
	info->spawnstate = &States[S_ZSHRUB2];
	info->meleestate = &States[S_ZSHRUB2_X];
	info->deathstate = &States[S_ZSHRUB2_DIE];
	info->deathsound = "TreeExplode";
	info->radius = 16*FRACUNIT;
	info->height = 40*FRACUNIT;
	info->mass = MAXINT;
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD;
}

void AZShrub2::GetDamageParms (int &damage, int &distance, bool hurtSrc)
{
	damage = 30;
	distance = 64;
}

//===========================================================================
//
// A_TreeDeath
//
//===========================================================================

void A_TreeDeath (AActor *actor)
{
	if (!(actor->flags2 & MF2_FIREDAMAGE))
	{
		actor->height <<= 2;
		actor->flags |= MF_SHOOTABLE;
		actor->flags &= ~(MF_CORPSE+MF_DROPOFF);
		actor->health = 35;
		return;
	}
	else
	{
		actor->SetState (GetInfo (actor)->meleestate);
	}
}

// Poison Shroom ------------------------------------------------------------

void A_PoisonShroom (AActor *);
void A_PoisonBagInit () {}	// FIXME

class AZPoisonShroom : public AActor
{
	DECLARE_ACTOR (AZPoisonShroom, AActor);
};

IMPLEMENT_DEF_SERIAL (AZPoisonShroom, AActor);
REGISTER_ACTOR (AZPoisonShroom, Hexen);

FState AZPoisonShroom::States[] =
{
#define S_ZPOISONSHROOM_P 0
	S_NORMAL (SHRM, 'A',	6, NULL 			, &States[S_ZPOISONSHROOM_P+1]),
	S_NORMAL (SHRM, 'B',	8, A_Pain			, &States[S_ZPOISONSHROOM_P+2]),//<-- Intentional state

#define S_ZPOISONSHROOM (S_ZPOISONSHROOM_P+2)
	S_NORMAL (SHRM, 'A',	5, A_PoisonShroom	, &States[S_ZPOISONSHROOM_P+1]),

#define S_ZPOISONSHROOM_X (S_ZPOISONSHROOM+1)
	S_NORMAL (SHRM, 'C',	5, NULL 			, &States[S_ZPOISONSHROOM_X+1]),
	S_NORMAL (SHRM, 'D',	5, NULL 			, &States[S_ZPOISONSHROOM_X+2]),
	S_NORMAL (SHRM, 'E',	5, A_PoisonBagInit	, &States[S_ZPOISONSHROOM_X+3]),
	S_NORMAL (SHRM, 'F',   -1, NULL 			, NULL)
};

void AZPoisonShroom::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 8104;
	info->spawnstate = &States[S_ZPOISONSHROOM];
	info->painstate = &States[S_ZPOISONSHROOM_P];
	info->painchance = 255;
	info->painsound = "PoisonShroomPain";
	info->deathstate = &States[S_ZPOISONSHROOM_X];
	info->deathsound = "PoisonShroomDeath";
	info->radius = 6*FRACUNIT;
	info->height = 20*FRACUNIT;
	info->flags = MF_SHOOTABLE|MF_SOLID|MF_NOBLOOD;
}

//===========================================================================
//
// A_PoisonShroom
//
//===========================================================================

void A_PoisonShroom (AActor *actor)
{
	actor->tics = 128+(P_Random()<<1);
}

// Fire Bull ----------------------------------------------------------------

class AZFireBull : public ASwitchableDecoration
{
	DECLARE_ACTOR (AZFireBull, ASwitchableDecoration);
public:
	void Activate (AActor *activator);
};

IMPLEMENT_DEF_SERIAL (AZFireBull, ASwitchableDecoration);
REGISTER_ACTOR (AZFireBull, Hexen);

FState AZFireBull::States[] =
{
#define S_ZFIREBULL 0
	S_BRIGHT (FBUL, 'A',	4, NULL 		, &States[S_ZFIREBULL+1]),
	S_BRIGHT (FBUL, 'B',	4, NULL 		, &States[S_ZFIREBULL+2]),
	S_BRIGHT (FBUL, 'C',	4, NULL 		, &States[S_ZFIREBULL+3]),
	S_BRIGHT (FBUL, 'D',	4, NULL 		, &States[S_ZFIREBULL+4]),
	S_BRIGHT (FBUL, 'E',	4, NULL 		, &States[S_ZFIREBULL+5]),
	S_BRIGHT (FBUL, 'F',	4, NULL 		, &States[S_ZFIREBULL+6]),
	S_BRIGHT (FBUL, 'G',	4, NULL 		, &States[S_ZFIREBULL+0]),

#define S_ZFIREBULL_U (S_ZFIREBULL+7)
	S_NORMAL (FBUL, 'H',   -1, NULL 		, NULL),

#define S_ZFIREBULL_DEATH (S_ZFIREBULL_U+1)
	S_BRIGHT (FBUL, 'J',	4, NULL 		, &States[S_ZFIREBULL_DEATH+1]),
	S_BRIGHT (FBUL, 'I',	4, NULL 		, &States[S_ZFIREBULL_U]),

#define S_ZFIREBULL_BIRTH (S_ZFIREBULL_DEATH+2)
	S_BRIGHT (FBUL, 'I',	4, NULL 		, &States[S_ZFIREBULL_BIRTH+1]),
	S_BRIGHT (FBUL, 'J',	4, NULL 		, &States[S_ZFIREBULL+0])
};

void AZFireBull::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 8042;
	info->spawnstate = &States[S_ZFIREBULL];
	info->seestate = &States[S_ZFIREBULL_BIRTH];
	info->meleestate = &States[S_ZFIREBULL_U];
	info->radius = 20*FRACUNIT;
	info->height = 80*FRACUNIT;
	info->flags = MF_SOLID;
}

void AZFireBull::Activate (AActor *activator)
{
	Super::Activate (activator);
	S_Sound (this, CHAN_BODY, "Ignite", 1, ATTN_NORM);
}

class AZFireBullUnlit : public AZFireBull
{
	DECLARE_STATELESS_ACTOR (AZFireBullUnlit, AZFireBull);
};

IMPLEMENT_DEF_SERIAL (AZFireBullUnlit, AZFireBull);
REGISTER_ACTOR (AZFireBullUnlit, Hexen);

void AZFireBullUnlit::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 8043;
	info->spawnstate = info->meleestate;
}

// Suit of armor ------------------------------------------------------------

void A_SoAExplode (AActor *);

class AZSuitOfArmor : public AActor
{
	DECLARE_ACTOR (AZSuitOfArmor, AActor);
};

IMPLEMENT_DEF_SERIAL (AZSuitOfArmor, AActor);
REGISTER_ACTOR (AZSuitOfArmor, Hexen);

FState AZSuitOfArmor::States[] =
{
	S_NORMAL (SUIT, 'A',   -1, NULL 					, NULL),
	S_NORMAL (SUIT, 'A',	1, A_SoAExplode 			, NULL)
};

void AZSuitOfArmor::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 8064;
	info->spawnstate = &States[0];
	info->spawnhealth = 60;
	info->deathstate = &States[1];
	info->deathsound = "SuitofArmorBreak";
	info->radius = 16*FRACUNIT;
	info->height = 72*FRACUNIT;
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD;
}

// Armor chunk --------------------------------------------------------------

class AZArmorChunk : public AActor
{
	DECLARE_ACTOR (AZArmorChunk, AActor);
};

IMPLEMENT_DEF_SERIAL (AZArmorChunk, AActor);
REGISTER_ACTOR (AZArmorChunk, Hexen);

FState AZArmorChunk::States[] =
{
	S_NORMAL (SUIT, 'B',   -1, NULL 					, NULL),
	S_NORMAL (SUIT, 'C',   -1, NULL 					, NULL),
	S_NORMAL (SUIT, 'D',   -1, NULL 					, NULL),
	S_NORMAL (SUIT, 'E',   -1, NULL 					, NULL),
	S_NORMAL (SUIT, 'F',   -1, NULL 					, NULL),
	S_NORMAL (SUIT, 'G',   -1, NULL 					, NULL),
	S_NORMAL (SUIT, 'H',   -1, NULL 					, NULL),
	S_NORMAL (SUIT, 'I',   -1, NULL 					, NULL),
	S_NORMAL (SUIT, 'J',   -1, NULL 					, NULL),
	S_NORMAL (SUIT, 'K',   -1, NULL 					, NULL)
};

void AZArmorChunk::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->radius = 4*FRACUNIT;
	info->height = 8*FRACUNIT;
}

//===========================================================================
//
// A_SoAExplode - Suit of Armor Explode
//
//===========================================================================

void A_SoAExplode (AActor *actor)
{
	AActor *mo;
	int i;

	for (i = 0; i < 10; i++)
	{
		mo = Spawn<AZArmorChunk> (actor->x+((P_Random()-128)<<12),
			actor->y+((P_Random()-128)<<12), 
			actor->z+(P_Random()*actor->height/256));
		mo->SetState (GetInfo (mo)->spawnstate + i);
		if (mo)
		{
			mo->momz = ((P_Random()&7)+5)*FRACUNIT;
			mo->momx = PS_Random()<<(FRACBITS-6);
			mo->momy = PS_Random()<<(FRACBITS-6);
		}
	}
	if (SpawnableThings[actor->args[0]])
	{ // Spawn an item
		if (!(dmflags & DF_NO_MONSTERS) 
		|| !(SpawnableThings[actor->args[0]]->ActorInfo->flags & MF_COUNTKILL))
		{ // Only spawn monsters if not -nomonsters
			Spawn (SpawnableThings[actor->args[0]],
				actor->x, actor->y, actor->z);
		}
	}
	S_Sound (actor, CHAN_BODY, GetInfo (actor)->deathsound, 1, ATTN_NORM);
	actor->Destroy ();
}

// Bell ---------------------------------------------------------------------

void A_BellReset1 (AActor *);
void A_BellReset2 (AActor *);

class AZBell : public AActor
{
	DECLARE_ACTOR (AZBell, AActor);
public:
	void Activate (AActor *activator);
};

IMPLEMENT_DEF_SERIAL (AZBell, AActor);
REGISTER_ACTOR (AZBell, Hexen);

FState AZBell::States[] =
{
#define S_ZBELL 0
	S_NORMAL (BBLL, 'F',   -1, NULL 					, NULL),

#define S_ZBELL_X (S_ZBELL+1)
	S_NORMAL (BBLL, 'A',	4, A_BellReset1 			, &States[S_ZBELL_X+1]),
	S_NORMAL (BBLL, 'B',	4, NULL 					, &States[S_ZBELL_X+2]),
	S_NORMAL (BBLL, 'C',	4, NULL 					, &States[S_ZBELL_X+3]),
	S_NORMAL (BBLL, 'D',	5, A_Scream 				, &States[S_ZBELL_X+4]),
	S_NORMAL (BBLL, 'C',	4, NULL 					, &States[S_ZBELL_X+5]),
	S_NORMAL (BBLL, 'B',	4, NULL 					, &States[S_ZBELL_X+6]),
	S_NORMAL (BBLL, 'A',	3, NULL 					, &States[S_ZBELL_X+7]),
	S_NORMAL (BBLL, 'E',	4, NULL 					, &States[S_ZBELL_X+8]),
	S_NORMAL (BBLL, 'F',	5, NULL 					, &States[S_ZBELL_X+9]),
	S_NORMAL (BBLL, 'G',	6, A_Scream 				, &States[S_ZBELL_X+10]),
	S_NORMAL (BBLL, 'F',	5, NULL 					, &States[S_ZBELL_X+11]),
	S_NORMAL (BBLL, 'E',	4, NULL 					, &States[S_ZBELL_X+12]),
	S_NORMAL (BBLL, 'A',	4, NULL 					, &States[S_ZBELL_X+13]),
	S_NORMAL (BBLL, 'B',	5, NULL 					, &States[S_ZBELL_X+14]),
	S_NORMAL (BBLL, 'C',	5, NULL 					, &States[S_ZBELL_X+15]),
	S_NORMAL (BBLL, 'D',	6, A_Scream 				, &States[S_ZBELL_X+16]),
	S_NORMAL (BBLL, 'C',	5, NULL 					, &States[S_ZBELL_X+17]),
	S_NORMAL (BBLL, 'B',	5, NULL 					, &States[S_ZBELL_X+18]),
	S_NORMAL (BBLL, 'A',	4, NULL 					, &States[S_ZBELL_X+19]),
	S_NORMAL (BBLL, 'E',	5, NULL 					, &States[S_ZBELL_X+20]),
	S_NORMAL (BBLL, 'F',	5, NULL 					, &States[S_ZBELL_X+21]),
	S_NORMAL (BBLL, 'G',	7, A_Scream 				, &States[S_ZBELL_X+22]),
	S_NORMAL (BBLL, 'F',	5, NULL 					, &States[S_ZBELL_X+23]),
	S_NORMAL (BBLL, 'E',	5, NULL 					, &States[S_ZBELL_X+24]),
	S_NORMAL (BBLL, 'A',	5, NULL 					, &States[S_ZBELL_X+25]),
	S_NORMAL (BBLL, 'B',	6, NULL 					, &States[S_ZBELL_X+26]),
	S_NORMAL (BBLL, 'C',	6, NULL 					, &States[S_ZBELL_X+27]),
	S_NORMAL (BBLL, 'D',	7, A_Scream 				, &States[S_ZBELL_X+28]),
	S_NORMAL (BBLL, 'C',	6, NULL 					, &States[S_ZBELL_X+29]),
	S_NORMAL (BBLL, 'B',	6, NULL 					, &States[S_ZBELL_X+30]),
	S_NORMAL (BBLL, 'A',	5, NULL 					, &States[S_ZBELL_X+31]),
	S_NORMAL (BBLL, 'E',	6, NULL 					, &States[S_ZBELL_X+32]),
	S_NORMAL (BBLL, 'F',	6, NULL 					, &States[S_ZBELL_X+33]),
	S_NORMAL (BBLL, 'G',	7, A_Scream 				, &States[S_ZBELL_X+34]),
	S_NORMAL (BBLL, 'F',	6, NULL 					, &States[S_ZBELL_X+35]),
	S_NORMAL (BBLL, 'E',	6, NULL 					, &States[S_ZBELL_X+36]),
	S_NORMAL (BBLL, 'A',	6, NULL 					, &States[S_ZBELL_X+37]),
	S_NORMAL (BBLL, 'B',	6, NULL 					, &States[S_ZBELL_X+38]),
	S_NORMAL (BBLL, 'C',	6, NULL 					, &States[S_ZBELL_X+39]),
	S_NORMAL (BBLL, 'B',	7, NULL 					, &States[S_ZBELL_X+40]),
	S_NORMAL (BBLL, 'A',	8, NULL 					, &States[S_ZBELL_X+41]),
	S_NORMAL (BBLL, 'E',   12, NULL 					, &States[S_ZBELL_X+42]),
	S_NORMAL (BBLL, 'A',   10, NULL 					, &States[S_ZBELL_X+43]),
	S_NORMAL (BBLL, 'B',   12, NULL 					, &States[S_ZBELL_X+44]),
	S_NORMAL (BBLL, 'A',   12, NULL 					, &States[S_ZBELL_X+45]),
	S_NORMAL (BBLL, 'E',   14, NULL 					, &States[S_ZBELL_X+46]),
	S_NORMAL (BBLL, 'A',	1, A_BellReset2 			, &States[S_ZBELL])
};

void AZBell::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 8065;
	info->spawnstate = &States[S_ZBELL];
	info->spawnhealth = 5;
	info->deathstate = &States[S_ZBELL_X];
	info->deathsound = "BellRing";
	info->radius = 56*FRACUNIT;
	info->height = 120*FRACUNIT;
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD|MF_NOGRAVITY|MF_SPAWNCEILING;
}

void AZBell::Activate (AActor *activator)
{
	if (health > 0)
	{
		P_DamageMobj (this, activator, activator, 10); // 'ring' the bell
	}
}

//===========================================================================
//
// A_BellReset1
//
//===========================================================================

void A_BellReset1 (AActor *actor)
{
	actor->flags |= MF_NOGRAVITY;
	actor->height <<= 2;
	if (actor->special)
	{ // Initiate death action
		LineSpecials[actor->special] (NULL, NULL, actor->args[0],
			actor->args[1], actor->args[2], actor->args[3], actor->args[4]);
		actor->special = 0;
	}
}

//===========================================================================
//
// A_BellReset2
//
//===========================================================================

void A_BellReset2 (AActor *actor)
{
	actor->flags |= MF_SHOOTABLE;
	actor->flags &= ~MF_CORPSE;
	actor->health = 5;
}

// "Christmas" Tree ---------------------------------------------------------

class AZXmasTree : public AActor
{
	DECLARE_ACTOR (AZXmasTree, AActor);
public:
	void SetExplodeParms (int &damage, int &dist, bool dmgSource);
};

IMPLEMENT_DEF_SERIAL (AZXmasTree, AActor);
REGISTER_ACTOR (AZXmasTree, Hexen);

FState AZXmasTree::States[] =
{
#define S_ZXMAS_TREE 0
	S_NORMAL (XMAS, 'A',   -1, NULL 					, NULL),

#define S_ZXMAS_TREE_DIE (S_ZXMAS_TREE+1)
	S_NORMAL (XMAS, 'A',	4, A_TreeDeath				, &States[S_ZXMAS_TREE]),

#define S_ZXMAS_TREE_X (S_ZXMAS_TREE_DIE+1)
	S_BRIGHT (XMAS, 'B',	6, NULL 					, &States[S_ZXMAS_TREE_X+1]),
	S_BRIGHT (XMAS, 'C',	6, A_Scream 				, &States[S_ZXMAS_TREE_X+2]),
	S_BRIGHT (XMAS, 'D',	5, NULL 					, &States[S_ZXMAS_TREE_X+3]),
	S_BRIGHT (XMAS, 'E',	5, A_Explode				, &States[S_ZXMAS_TREE_X+4]),
	S_BRIGHT (XMAS, 'F',	5, NULL 					, &States[S_ZXMAS_TREE_X+5]),
	S_BRIGHT (XMAS, 'G',	4, NULL 					, &States[S_ZXMAS_TREE_X+6]),
	S_NORMAL (XMAS, 'H',	5, NULL 					, &States[S_ZXMAS_TREE_X+7]),
	S_NORMAL (XMAS, 'I',	4, A_NoBlocking 			, &States[S_ZXMAS_TREE_X+8]),
	S_NORMAL (XMAS, 'J',	4, NULL 					, &States[S_ZXMAS_TREE_X+9]),
	S_NORMAL (XMAS, 'K',   -1, NULL 					, NULL)
};

void AZXmasTree::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 8068;
	info->spawnstate = &States[S_ZXMAS_TREE];
	info->spawnhealth = 20;
	info->meleestate = &States[S_ZXMAS_TREE_X];
	info->deathstate = &States[S_ZXMAS_TREE_DIE];
	info->deathsound = "TreeExplode";
	info->radius = 11*FRACUNIT;
	info->height = 130*FRACUNIT;
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD;
}

void AZXmasTree::SetExplodeParms (int &damage, int &dist, bool dmgSource)
{
	damage = 30;
	dist = 64;
}

// Cauldron -----------------------------------------------------------------

class AZCauldron : public ASwitchableDecoration
{
	DECLARE_ACTOR (AZCauldron, ASwitchableDecoration);
public:
	void Activate (AActor *activator);
};

IMPLEMENT_DEF_SERIAL (AZCauldron, ASwitchableDecoration);
REGISTER_ACTOR (AZCauldron, Hexen);

FState AZCauldron::States[] =
{
#define S_ZCAULDRON 0
	S_BRIGHT (CDRN, 'B',	4, NULL 					, &States[S_ZCAULDRON+1]),
	S_BRIGHT (CDRN, 'C',	4, NULL 					, &States[S_ZCAULDRON+2]),
	S_BRIGHT (CDRN, 'D',	4, NULL 					, &States[S_ZCAULDRON+3]),
	S_BRIGHT (CDRN, 'E',	4, NULL 					, &States[S_ZCAULDRON+4]),
	S_BRIGHT (CDRN, 'F',	4, NULL 					, &States[S_ZCAULDRON+5]),
	S_BRIGHT (CDRN, 'G',	4, NULL 					, &States[S_ZCAULDRON+6]),
	S_BRIGHT (CDRN, 'H',	4, NULL 					, &States[S_ZCAULDRON+0]),

#define S_ZCAULDRON_U (S_ZCAULDRON+7)
	S_NORMAL (CDRN, 'A',   -1, NULL 					, NULL)
};

void AZCauldron::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 8069;
	info->spawnstate = &States[S_ZCAULDRON];
	info->seestate = &States[S_ZCAULDRON];
	info->meleestate = &States[S_ZCAULDRON_U];
	info->radius = 12*FRACUNIT;
	info->height = 26*FRACUNIT;
	info->flags = MF_SOLID;
}

void AZCauldron::Activate (AActor *activator)
{
	Super::Activate (activator);
	S_Sound (this, CHAN_BODY, "Ignite", 1, ATTN_NORM);
}

class AZCauldronUnlit : public AZCauldron
{
	DECLARE_STATELESS_ACTOR (AZCauldronUnlit, AZCauldron);
};

IMPLEMENT_DEF_SERIAL (AZCauldronUnlit, AZCauldron);
REGISTER_ACTOR (AZCauldronUnlit, Hexen);

void AZCauldronUnlit::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 8070;
	info->spawnstate = info->meleestate;
}
