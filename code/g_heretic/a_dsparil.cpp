#include "actor.h"
#include "info.h"
#include "a_hereticglobal.h"
#include "p_local.h"
#include "p_enemy.h"
#include "a_action.h"
#include "s_sound.h"
#include "m_random.h"
#include "a_sharedglobal.h"

void A_Sor1Chase (AActor *);
void A_Sor1Pain (AActor *);
void A_Srcr1Attack (AActor *);
void A_SorZap (AActor *);
void A_SorcererRise (AActor *);
void A_SorRise (AActor *);
void A_SorSightSnd (AActor *);
void A_Srcr2Decide (AActor *);
void A_Srcr2Attack (AActor *);
void A_Sor2DthInit (AActor *);
void A_SorDSph (AActor *);
void A_Sor2DthLoop (AActor *);
void A_SorDExp (AActor *);
void A_SorDBon (AActor *);
void A_BlueSpark (AActor *);
void A_GenWizard (AActor *);

// Boss spot ----------------------------------------------------------------

class ABossSpot : public AActor
{
	DECLARE_STATELESS_ACTOR (ABossSpot, AActor);
public:
	ABossSpot *NextSpot;
	void BeginPlay ();
};

IMPLEMENT_SERIAL (ABossSpot, AActor);
REGISTER_ACTOR (ABossSpot, Heretic);

void ABossSpot::Serialize (FArchive &arc)
{
	Super::Serialize (arc);
	arc << NextSpot;
}

void ABossSpot::SetDefaults (FActorInfo *info)
{
	ACTOR_DEFS_STATELESS;
	info->doomednum = 56;
	info->flags2 = MF2_DONTDRAW;
}

void ABossSpot::BeginPlay ()
{
	Super::BeginPlay ();
	NextSpot = NULL;
}

// Sorcerer (D'Sparil on his serpent) ---------------------------------------

class ASorcerer1 : public AActor
{
	DECLARE_ACTOR (ASorcerer1, AActor);
};

IMPLEMENT_DEF_SERIAL (ASorcerer1, AActor);
REGISTER_ACTOR (ASorcerer1, Heretic);

FState ASorcerer1::States[] =
{
#define S_SRCR1_LOOK 0
	S_NORMAL (SRCR, 'A',   10, A_Look					, &States[S_SRCR1_LOOK+1]),
	S_NORMAL (SRCR, 'B',   10, A_Look					, &States[S_SRCR1_LOOK+0]),

#define S_SRCR1_WALK (S_SRCR1_LOOK+2)
	S_NORMAL (SRCR, 'A',	5, A_Sor1Chase				, &States[S_SRCR1_WALK+1]),
	S_NORMAL (SRCR, 'B',	5, A_Sor1Chase				, &States[S_SRCR1_WALK+2]),
	S_NORMAL (SRCR, 'C',	5, A_Sor1Chase				, &States[S_SRCR1_WALK+3]),
	S_NORMAL (SRCR, 'D',	5, A_Sor1Chase				, &States[S_SRCR1_WALK+0]),

#define S_SRCR1_PAIN (S_SRCR1_WALK+4)
	S_NORMAL (SRCR, 'Q',	6, A_Sor1Pain				, &States[S_SRCR1_WALK+0]),

#define S_SRCR1_ATK (S_SRCR1_PAIN+1)
	S_NORMAL (SRCR, 'Q',	7, A_FaceTarget 			, &States[S_SRCR1_ATK+1]),
	S_NORMAL (SRCR, 'R',	6, A_FaceTarget 			, &States[S_SRCR1_ATK+2]),
	S_NORMAL (SRCR, 'S',   10, A_Srcr1Attack			, &States[S_SRCR1_WALK+0]),
	S_NORMAL (SRCR, 'S',   10, A_FaceTarget 			, &States[S_SRCR1_ATK+4]),
	S_NORMAL (SRCR, 'Q',	7, A_FaceTarget 			, &States[S_SRCR1_ATK+5]),
	S_NORMAL (SRCR, 'R',	6, A_FaceTarget 			, &States[S_SRCR1_ATK+6]),
	S_NORMAL (SRCR, 'S',   10, A_Srcr1Attack			, &States[S_SRCR1_WALK+0]),

#define S_SRCR1_DIE (S_SRCR1_ATK+7)
	S_NORMAL (SRCR, 'E',	7, NULL 					, &States[S_SRCR1_DIE+1]),
	S_NORMAL (SRCR, 'F',	7, A_Scream 				, &States[S_SRCR1_DIE+2]),
	S_NORMAL (SRCR, 'G',	7, NULL 					, &States[S_SRCR1_DIE+3]),
	S_NORMAL (SRCR, 'H',	6, NULL 					, &States[S_SRCR1_DIE+4]),
	S_NORMAL (SRCR, 'I',	6, NULL 					, &States[S_SRCR1_DIE+5]),
	S_NORMAL (SRCR, 'J',	6, NULL 					, &States[S_SRCR1_DIE+6]),
	S_NORMAL (SRCR, 'K',	6, NULL 					, &States[S_SRCR1_DIE+7]),
	S_NORMAL (SRCR, 'L',   25, A_SorZap 				, &States[S_SRCR1_DIE+8]),
	S_NORMAL (SRCR, 'M',	5, NULL 					, &States[S_SRCR1_DIE+9]),
	S_NORMAL (SRCR, 'N',	5, NULL 					, &States[S_SRCR1_DIE+10]),
	S_NORMAL (SRCR, 'O',	4, NULL 					, &States[S_SRCR1_DIE+11]),
	S_NORMAL (SRCR, 'L',   20, A_SorZap 				, &States[S_SRCR1_DIE+12]),
	S_NORMAL (SRCR, 'M',	5, NULL 					, &States[S_SRCR1_DIE+13]),
	S_NORMAL (SRCR, 'N',	5, NULL 					, &States[S_SRCR1_DIE+14]),
	S_NORMAL (SRCR, 'O',	4, NULL 					, &States[S_SRCR1_DIE+15]),
	S_NORMAL (SRCR, 'L',   12, NULL 					, &States[S_SRCR1_DIE+16]),
	S_NORMAL (SRCR, 'P',   -1, A_SorcererRise			, NULL)
};

void ASorcerer1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 7;
	info->spawnstate = &States[S_SRCR1_LOOK];
	info->spawnhealth = 2000;
	info->seestate = &States[S_SRCR1_WALK];
	info->seesound = "dsparilserpent/sight";
	info->attacksound = "dsparilserpent/attack";
	info->painstate = &States[S_SRCR1_PAIN];
	info->painchance = 56;
	info->painsound = "dsparilserpent/pain";
	info->missilestate = &States[S_SRCR1_ATK];
	info->deathstate = &States[S_SRCR1_DIE];
	info->deathsound = "dsparilserpent/death";
	info->speed = 16;
	info->radius = 28 * FRACUNIT;
	info->height = 100 * FRACUNIT;
	info->mass = 800;
	info->activesound = "dsparilserpent/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_FLOORCLIP|MF2_PASSMOBJ|MF2_BOSS;
	info->flags3 = MF3_DONTMORPH|MF3_NORADIUSDMG|MF3_NOTARGET;
}

// Sorcerer FX 1 ------------------------------------------------------------

class ASorcererFX1 : public AActor
{
	DECLARE_ACTOR (ASorcererFX1, AActor);
};

IMPLEMENT_DEF_SERIAL (ASorcererFX1, AActor);
REGISTER_ACTOR (ASorcererFX1, Heretic);

FState ASorcererFX1::States[] =
{
#define S_SRCRFX1 0
	S_BRIGHT (FX14, 'A',	6, NULL 					, &States[S_SRCRFX1+1]),
	S_BRIGHT (FX14, 'B',	6, NULL 					, &States[S_SRCRFX1+2]),
	S_BRIGHT (FX14, 'C',	6, NULL 					, &States[S_SRCRFX1+0]),

#define S_SRCRFXI1 (S_SRCRFX1+3)
	S_BRIGHT (FX14, 'D',	5, NULL 					, &States[S_SRCRFXI1+1]),
	S_BRIGHT (FX14, 'E',	5, NULL 					, &States[S_SRCRFXI1+2]),
	S_BRIGHT (FX14, 'F',	5, NULL 					, &States[S_SRCRFXI1+3]),
	S_BRIGHT (FX14, 'G',	5, NULL 					, &States[S_SRCRFXI1+4]),
	S_BRIGHT (FX14, 'H',	5, NULL 					, NULL)
};

void ASorcererFX1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_SRCRFX1];
	info->deathstate = &States[S_SRCRFXI1];
	info->speed = GameSpeed != SPEED_Fast ? 20 * FRACUNIT : 28 * FRACUNIT;
	info->radius = 10 * FRACUNIT;
	info->height = 10 * FRACUNIT;
	info->damage = 10;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT|MF2_FIREDAMAGE;
}

// Sorcerer 2 (D'Sparil without his serpent) --------------------------------

IMPLEMENT_SERIAL (ASorcerer2, AActor);
REGISTER_ACTOR (ASorcerer2, Heretic);

FState ASorcerer2::States[] =
{
#define S_SOR2_LOOK 0
	S_NORMAL (SOR2, 'M',   10, A_Look					, &States[S_SOR2_LOOK+1]),
	S_NORMAL (SOR2, 'N',   10, A_Look					, &States[S_SOR2_LOOK+0]),

#define S_SOR2_WALK (S_SOR2_LOOK+2)
	S_NORMAL (SOR2, 'M',	4, A_Chase					, &States[S_SOR2_WALK+1]),
	S_NORMAL (SOR2, 'N',	4, A_Chase					, &States[S_SOR2_WALK+2]),
	S_NORMAL (SOR2, 'O',	4, A_Chase					, &States[S_SOR2_WALK+3]),
	S_NORMAL (SOR2, 'P',	4, A_Chase					, &States[S_SOR2_WALK+0]),

#define S_SOR2_RISE (S_SOR2_WALK+4)
	S_NORMAL (SOR2, 'A',	4, NULL 					, &States[S_SOR2_RISE+1]),
	S_NORMAL (SOR2, 'B',	4, NULL 					, &States[S_SOR2_RISE+2]),
	S_NORMAL (SOR2, 'C',	4, A_SorRise				, &States[S_SOR2_RISE+3]),
	S_NORMAL (SOR2, 'D',	4, NULL 					, &States[S_SOR2_RISE+4]),
	S_NORMAL (SOR2, 'E',	4, NULL 					, &States[S_SOR2_RISE+5]),
	S_NORMAL (SOR2, 'F',	4, NULL 					, &States[S_SOR2_RISE+6]),
	S_NORMAL (SOR2, 'G',   12, A_SorSightSnd			, &States[S_SOR2_WALK+0]),

#define S_SOR2_PAIN (S_SOR2_RISE+7)
	S_NORMAL (SOR2, 'Q',	3, NULL 					, &States[S_SOR2_PAIN+1]),
	S_NORMAL (SOR2, 'Q',	6, A_Pain					, &States[S_SOR2_WALK+0]),

#define S_SOR2_ATK (S_SOR2_PAIN+2)
	S_NORMAL (SOR2, 'R',	9, A_Srcr2Decide			, &States[S_SOR2_ATK+1]),
	S_NORMAL (SOR2, 'S',	9, A_FaceTarget 			, &States[S_SOR2_ATK+2]),
	S_NORMAL (SOR2, 'T',   20, A_Srcr2Attack			, &States[S_SOR2_WALK+0]),

#define S_SOR2_TELE (S_SOR2_ATK+3)
	S_NORMAL (SOR2, 'L',	6, NULL 					, &States[S_SOR2_TELE+1]),
	S_NORMAL (SOR2, 'K',	6, NULL 					, &States[S_SOR2_TELE+2]),
	S_NORMAL (SOR2, 'J',	6, NULL 					, &States[S_SOR2_TELE+3]),
	S_NORMAL (SOR2, 'I',	6, NULL 					, &States[S_SOR2_TELE+4]),
	S_NORMAL (SOR2, 'H',	6, NULL 					, &States[S_SOR2_TELE+5]),
	S_NORMAL (SOR2, 'G',	6, NULL 					, &States[S_SOR2_WALK+0]),

#define S_SOR2_DIE (S_SOR2_TELE+6)
	S_NORMAL (SDTH, 'A',	8, A_Sor2DthInit			, &States[S_SOR2_DIE+1]),
	S_NORMAL (SDTH, 'B',	8, NULL 					, &States[S_SOR2_DIE+2]),
	S_NORMAL (SDTH, 'C',	8, A_SorDSph				, &States[S_SOR2_DIE+3]),
	S_NORMAL (SDTH, 'D',	7, NULL 					, &States[S_SOR2_DIE+4]),
	S_NORMAL (SDTH, 'E',	7, NULL 					, &States[S_SOR2_DIE+5]),
	S_NORMAL (SDTH, 'F',	7, A_Sor2DthLoop			, &States[S_SOR2_DIE+6]),
	S_NORMAL (SDTH, 'G',	6, A_SorDExp				, &States[S_SOR2_DIE+7]),
	S_NORMAL (SDTH, 'H',	6, NULL 					, &States[S_SOR2_DIE+8]),
	S_NORMAL (SDTH, 'I',   18, NULL 					, &States[S_SOR2_DIE+9]),
	S_NORMAL (SDTH, 'J',	6, A_NoBlocking 			, &States[S_SOR2_DIE+10]),
	S_NORMAL (SDTH, 'K',	6, A_SorDBon				, &States[S_SOR2_DIE+11]),
	S_NORMAL (SDTH, 'L',	6, NULL 					, &States[S_SOR2_DIE+12]),
	S_NORMAL (SDTH, 'M',	6, NULL 					, &States[S_SOR2_DIE+13]),
	S_NORMAL (SDTH, 'N',	6, NULL 					, &States[S_SOR2_DIE+14]),
	S_NORMAL (SDTH, 'O',   -1, A_BossDeath				, NULL)
};

void ASorcerer2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_SOR2_LOOK];
	info->spawnhealth = 3500;
	info->seestate = &States[S_SOR2_WALK];
	info->seesound = "dsparil/sight";
	info->attacksound = "dsparil/attack";
	info->painstate = &States[S_SOR2_PAIN];
	info->painchance = 32;
	info->painsound = "dsparil/pain";
	info->missilestate = &States[S_SOR2_ATK];
	info->deathstate = &States[S_SOR2_DIE];
	info->speed = 14;
	info->radius = 16 * FRACUNIT;
	info->height = 70 * FRACUNIT;
	info->mass = 300;
	info->activesound = "dsparil/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_DROPOFF;
	info->flags2 = MF2_FLOORCLIP|MF2_PASSMOBJ|MF2_BOSS;
	info->flags3 = MF3_DONTMORPH|MF3_FULLVOLACTIVE|MF3_NORADIUSDMG|MF3_NOTARGET;
}

void ASorcerer2::Serialize (FArchive &arc)
{
	Super::Serialize (arc);
	arc << NumBossSpots << FirstBossSpot;
}

void ASorcerer2::BeginPlay ()
{
	TThinkerIterator<ABossSpot> iterator;
	ABossSpot *spot;

	Super::BeginPlay ();
	NumBossSpots = 0;
	spot = iterator.Next ();
	FirstBossSpot = static_cast<ABossSpot *> (spot);
	while (spot)
	{
		NumBossSpots++;
		spot->NextSpot = iterator.Next ();
		spot = spot->NextSpot;
	}
}

bool ASorcerer2::NewTarget (AActor *other)
{
	return !other->IsKindOf (RUNTIME_CLASS(AWizard));
}

// Sorcerer 2 FX 1 ----------------------------------------------------------

class ASorcerer2FX1 : public AActor
{
	DECLARE_ACTOR (ASorcerer2FX1, AActor);
public:
	void GetExplodeParms (int &damage, int &distance, bool &hurtSource);
};

IMPLEMENT_DEF_SERIAL (ASorcerer2FX1, AActor);
REGISTER_ACTOR (ASorcerer2FX1, Heretic);

FState ASorcerer2FX1::States[] =
{
#define S_SOR2FX1 0
	S_BRIGHT (FX16, 'A',	3, A_BlueSpark				, &States[S_SOR2FX1+1]),
	S_BRIGHT (FX16, 'B',	3, A_BlueSpark				, &States[S_SOR2FX1+2]),
	S_BRIGHT (FX16, 'C',	3, A_BlueSpark				, &States[S_SOR2FX1+0]),

#define S_SOR2FXI1 (S_SOR2FX1+3)
	S_BRIGHT (FX16, 'G',	5, A_Explode				, &States[S_SOR2FXI1+1]),
	S_BRIGHT (FX16, 'H',	5, NULL 					, &States[S_SOR2FXI1+2]),
	S_BRIGHT (FX16, 'I',	5, NULL 					, &States[S_SOR2FXI1+3]),
	S_BRIGHT (FX16, 'J',	5, NULL 					, &States[S_SOR2FXI1+4]),
	S_BRIGHT (FX16, 'K',	5, NULL 					, &States[S_SOR2FXI1+5]),
	S_BRIGHT (FX16, 'L',	5, NULL 					, NULL)
};

void ASorcerer2FX1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_SOR2FX1];
	info->deathstate = &States[S_SOR2FXI1];
	info->speed = GameSpeed != SPEED_Fast ? 20 * FRACUNIT : 28 * FRACUNIT;
	info->radius = 10 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 1;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT;
}

void ASorcerer2FX1::GetExplodeParms (int &damage, fixed_t &distance, bool &hurtSource)
{
	damage = 80 + (P_Random() & 31);
}

// Sorcerer 2 FX Spark ------------------------------------------------------

class ASorcerer2FXSpark : public AActor
{
	DECLARE_ACTOR (ASorcerer2FXSpark, AActor);
};

IMPLEMENT_DEF_SERIAL (ASorcerer2FXSpark, AActor);
REGISTER_ACTOR (ASorcerer2FXSpark, Heretic);

FState ASorcerer2FXSpark::States[] =
{
#define S_SOR2FXSPARK 0
	S_BRIGHT (FX16, 'D',   12, NULL 					, &States[S_SOR2FXSPARK+1]),
	S_BRIGHT (FX16, 'E',   12, NULL 					, &States[S_SOR2FXSPARK+2]),
	S_BRIGHT (FX16, 'F',   12, NULL 					, NULL)
};

void ASorcerer2FXSpark::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_SOR2FXSPARK];
	info->radius = 20 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT|MF2_CANNOTPUSH;
}

// Sorcerer 2 FX 2 ----------------------------------------------------------

class ASorcerer2FX2 : public AActor
{
	DECLARE_ACTOR (ASorcerer2FX2, AActor);
};

IMPLEMENT_DEF_SERIAL (ASorcerer2FX2, AActor);
REGISTER_ACTOR (ASorcerer2FX2, Heretic);

FState ASorcerer2FX2::States[] =
{
#define S_SOR2FX2 0
	S_BRIGHT (FX11, 'A',   35, NULL 					, &States[S_SOR2FX2+1]),
	S_BRIGHT (FX11, 'A',	5, A_GenWizard				, &States[S_SOR2FX2+2]),
	S_BRIGHT (FX11, 'B',	5, NULL 					, &States[S_SOR2FX2+1]),

#define S_SOR2FXI2 (S_SOR2FX2+3)
	S_BRIGHT (FX11, 'C',	5, NULL 					, &States[S_SOR2FXI2+1]),
	S_BRIGHT (FX11, 'D',	5, NULL 					, &States[S_SOR2FXI2+2]),
	S_BRIGHT (FX11, 'E',	5, NULL 					, &States[S_SOR2FXI2+3]),
	S_BRIGHT (FX11, 'F',	5, NULL 					, &States[S_SOR2FXI2+4]),
	S_BRIGHT (FX11, 'G',	5, NULL 					, NULL)
};

void ASorcerer2FX2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_SOR2FX2];
	info->deathstate = &States[S_SOR2FXI2];
	info->speed = 6 * FRACUNIT;
	info->radius = 10 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 10;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT;
}

// Sorcerer 2 Telefade ------------------------------------------------------

class ASorcerer2Telefade : public AActor
{
	DECLARE_ACTOR (ASorcerer2Telefade, AActor);
};

IMPLEMENT_DEF_SERIAL (ASorcerer2Telefade, AActor);
REGISTER_ACTOR (ASorcerer2Telefade, Heretic);

FState ASorcerer2Telefade::States[] =
{
#define S_SOR2TELEFADE 0
	S_NORMAL (SOR2, 'G',	8, NULL 					, &States[S_SOR2TELEFADE+1]),
	S_NORMAL (SOR2, 'H',	6, NULL 					, &States[S_SOR2TELEFADE+2]),
	S_NORMAL (SOR2, 'I',	6, NULL 					, &States[S_SOR2TELEFADE+3]),
	S_NORMAL (SOR2, 'J',	6, NULL 					, &States[S_SOR2TELEFADE+4]),
	S_NORMAL (SOR2, 'K',	6, NULL 					, &States[S_SOR2TELEFADE+5]),
	S_NORMAL (SOR2, 'L',	6, NULL 					, NULL)
};

void ASorcerer2Telefade::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_SOR2TELEFADE];
	info->flags = MF_NOBLOCKMAP;
}

//----------------------------------------------------------------------------
//
// PROC A_Sor1Pain
//
//----------------------------------------------------------------------------

void A_Sor1Pain (AActor *actor)
{
	actor->special1 = 20; // Number of steps to walk fast
	A_Pain (actor);
}

//----------------------------------------------------------------------------
//
// PROC A_Sor1Chase
//
//----------------------------------------------------------------------------

void A_Sor1Chase (AActor *actor)
{
	if (actor->special1)
	{
		actor->special1--;
		actor->tics -= 3;
	}
	A_Chase(actor);
}

//----------------------------------------------------------------------------
//
// PROC A_Srcr1Attack
//
// Sorcerer demon attack.
//
//----------------------------------------------------------------------------

void A_Srcr1Attack (AActor *actor)
{
	AActor *mo;
	fixed_t momz;
	angle_t angle;

	if (!actor->target)
	{
		return;
	}
	S_Sound (actor, CHAN_BODY, GetInfo (actor)->attacksound, 1, ATTN_NORM);
	if (P_CheckMeleeRange (actor))
	{
		P_DamageMobj (actor->target, actor, actor, HITDICE(8));
		return;
	}
	if (actor->health > (GetInfo (actor)->spawnhealth/3)*2)
	{ // Spit one fireball
		P_SpawnMissileZ (actor, actor->z + 48*FRACUNIT, actor->target, RUNTIME_CLASS(ASorcererFX1));
	}
	else
	{ // Spit three fireballs
		mo = P_SpawnMissileZ (actor, actor->z + 48*FRACUNIT, actor->target, RUNTIME_CLASS(ASorcererFX1));
		if (mo)
		{
			momz = mo->momz;
			angle = mo->angle;
			P_SpawnMissileAngleZ (actor, actor->z + 48*FRACUNIT, RUNTIME_CLASS(ASorcererFX1), angle-ANGLE_1*3, momz);
			P_SpawnMissileAngleZ (actor, actor->z + 48*FRACUNIT, RUNTIME_CLASS(ASorcererFX1), angle+ANGLE_1*3, momz);
		}
		if (actor->health < GetInfo (actor)->spawnhealth/3)
		{ // Maybe attack again
			if (actor->special1)
			{ // Just attacked, so don't attack again
				actor->special1 = 0;
			}
			else
			{ // Set state to attack again
				actor->special1 = 1;
				actor->SetState (&ASorcerer1::States[S_SRCR1_ATK+3]);
			}
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_SorcererRise
//
//----------------------------------------------------------------------------

void A_SorcererRise (AActor *actor)
{
	AActor *mo;

	actor->flags &= ~MF_SOLID;
	mo = Spawn<ASorcerer2> (actor->x, actor->y, actor->z);
	mo->SetState (&ASorcerer2::States[S_SOR2_RISE]);
	mo->angle = actor->angle;
	mo->target = actor->target;
}

//----------------------------------------------------------------------------
//
// PROC P_DSparilTeleport
//
//----------------------------------------------------------------------------

void P_DSparilTeleport (AActor *actor)
{
	int i;
	fixed_t prevX;
	fixed_t prevY;
	fixed_t prevZ;
	AActor *mo;
	ASorcerer2 *self = static_cast<ASorcerer2 *> (actor);
	ABossSpot *spot;
	ABossSpot *initial;

	if (!self->NumBossSpots)
	{ // No spots
		return;
	}
	i = P_Random () % self->NumBossSpots;
	spot = static_cast<ABossSpot *> (self->FirstBossSpot);
	while (i-- > 0)
	{
		spot = spot->NextSpot;
	}
	initial = spot;
	while (P_AproxDistance (actor->x - spot->x, actor->y - spot->y) < 128*FRACUNIT)
	{
		spot = spot->NextSpot;
		if (spot == NULL)
		{
			spot = static_cast<ABossSpot *> (self->FirstBossSpot);
		}
		if (spot == initial)
		{
			// [RH] Don't inifinite loop if no spots further than 128*FRACUNIT
			return;
		}
	}
	prevX = actor->x;
	prevY = actor->y;
	prevZ = actor->z;
	if (P_TeleportMove (actor, spot->x, spot->y, spot->z, false))
	{
		mo = Spawn<ASorcerer2Telefade> (prevX, prevY, prevZ);
		S_Sound (mo, CHAN_BODY, "misc/teleport", 1, ATTN_NORM);
		actor->SetState (&ASorcerer2::States[S_SOR2_TELE]);
		S_Sound (actor, CHAN_BODY, "misc/teleport", 1, ATTN_NORM);
		actor->z = actor->floorz;
		actor->angle = spot->angle;
		actor->momx = actor->momy = actor->momz = 0;
	}
}

//----------------------------------------------------------------------------
//
// PROC A_Srcr2Decide
//
//----------------------------------------------------------------------------

void A_Srcr2Decide (AActor *actor)
{
	static const int chance[] =
	{
		192, 120, 120, 120, 64, 64, 32, 16, 0
	};

	if (P_Random() < chance[actor->health / (GetInfo (actor)->spawnhealth/8)])
	{
		P_DSparilTeleport (actor);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_Srcr2Attack
//
//----------------------------------------------------------------------------

void A_Srcr2Attack (AActor *actor)
{
	int chance;

	if (!actor->target)
	{
		return;
	}
	S_Sound (actor, CHAN_BODY, GetInfo (actor)->attacksound, 1, ATTN_NONE);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj (actor->target, actor, actor, HITDICE(20));
		return;
	}
	chance = actor->health < GetInfo (actor)->spawnhealth/2 ? 96 : 48;
	if (P_Random() < chance)
	{ // Wizard spawners
		P_SpawnMissileAngle (actor, RUNTIME_CLASS(ASorcerer2FX2),
			actor->angle-ANG45, FRACUNIT/2);
		P_SpawnMissileAngle (actor, RUNTIME_CLASS(ASorcerer2FX2),
			actor->angle+ANG45, FRACUNIT/2);
	}
	else
	{ // Blue bolt
		P_SpawnMissile (actor, actor->target, RUNTIME_CLASS(ASorcerer2FX1));
	}
}

//----------------------------------------------------------------------------
//
// PROC A_BlueSpark
//
//----------------------------------------------------------------------------

void A_BlueSpark (AActor *actor)
{
	int i;
	AActor *mo;

	for (i = 0; i < 2; i++)
	{
		mo = Spawn<ASorcerer2FXSpark> (actor->x, actor->y, actor->z);
		mo->momx = PS_Random() << 9;
		mo->momy = PS_Random() << 9;
		mo->momz = FRACUNIT + (P_Random()<<8);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_GenWizard
//
//----------------------------------------------------------------------------

void A_GenWizard (AActor *actor)
{
	AActor *mo;

	mo = Spawn<AWizard> (actor->x, actor->y,
		actor->z - RUNTIME_CLASS(AWizard)->ActorInfo->height/2);
	if (P_TestMobjLocation(mo) == false)
	{ // Didn't fit
		mo->Destroy ();
		return;
	}
	actor->momx = actor->momy = actor->momz = 0;
	actor->SetState (GetInfo (actor)->deathstate);
	actor->flags &= ~MF_MISSILE;
	Spawn<ATeleportFog> (actor->x, actor->y, actor->z);
}

//----------------------------------------------------------------------------
//
// PROC A_Sor2DthInit
//
//----------------------------------------------------------------------------

void A_Sor2DthInit (AActor *actor)
{
	actor->special1 = 7; // Animation loop counter
	P_Massacre (); // Kill monsters early
}

//----------------------------------------------------------------------------
//
// PROC A_Sor2DthLoop
//
//----------------------------------------------------------------------------

void A_Sor2DthLoop (AActor *actor)
{
	if (--actor->special1)
	{ // Need to loop
		actor->SetState (&ASorcerer2::States[S_SOR2_DIE+3]);
	}
}

//----------------------------------------------------------------------------
//
// D'Sparil Sound Routines
//
//----------------------------------------------------------------------------

void A_SorZap (AActor *actor) {S_Sound (actor, CHAN_BODY, "dsparil/zap", 1, ATTN_NONE);}
void A_SorRise (AActor *actor) {S_Sound (actor, CHAN_BODY, "dsparil/rise", 1, ATTN_NONE);}
void A_SorDSph (AActor *actor) {S_Sound (actor, CHAN_BODY, "dsparil/scream", 1, ATTN_NONE);}
void A_SorDExp (AActor *actor) {S_Sound (actor, CHAN_BODY, "dsparil/explode", 1, ATTN_NONE);}
void A_SorDBon (AActor *actor) {S_Sound (actor, CHAN_BODY, "dsparil/bones", 1, ATTN_NONE);}
void A_SorSightSnd (AActor *actor) {S_Sound (actor, CHAN_BODY, "dsparil/sight", 1, ATTN_NONE);}
