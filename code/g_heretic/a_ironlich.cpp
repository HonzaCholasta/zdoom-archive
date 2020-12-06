#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "a_action.h"

void A_LichAttack (AActor *);
void A_LichIceImpact (AActor *);
void A_LichFireGrow (AActor *);
void A_WhirlwindSeek (AActor *);

// Ironlich -----------------------------------------------------------------

class AIronlich : public AActor
{
	DECLARE_ACTOR (AIronlich, AActor);
public:
	void NoBlockingSet ();
};

IMPLEMENT_DEF_SERIAL (AIronlich, AActor);
REGISTER_ACTOR (AIronlich, Heretic);

FState AIronlich::States[] =
{
#define S_HEAD_LOOK 0
	S_NORMAL (HEAD, 'A',   10, A_Look					, &States[S_HEAD_LOOK]),

#define S_HEAD_FLOAT (S_HEAD_LOOK+1)
	S_NORMAL (HEAD, 'A',	4, A_Chase					, &States[S_HEAD_FLOAT]),

#define S_HEAD_ATK (S_HEAD_FLOAT+1)
	S_NORMAL (HEAD, 'A',	5, A_FaceTarget 			, &States[S_HEAD_ATK+1]),
	S_NORMAL (HEAD, 'B',   20, A_LichAttack 			, &States[S_HEAD_FLOAT]),

#define S_HEAD_PAIN (S_HEAD_ATK+2)
	S_NORMAL (HEAD, 'A',	4, NULL 					, &States[S_HEAD_PAIN+1]),
	S_NORMAL (HEAD, 'A',	4, A_Pain					, &States[S_HEAD_FLOAT]),

#define S_HEAD_DIE (S_HEAD_PAIN+2)
	S_NORMAL (HEAD, 'C',	7, NULL 					, &States[S_HEAD_DIE+1]),
	S_NORMAL (HEAD, 'D',	7, A_Scream 				, &States[S_HEAD_DIE+2]),
	S_NORMAL (HEAD, 'E',	7, NULL 					, &States[S_HEAD_DIE+3]),
	S_NORMAL (HEAD, 'F',	7, NULL 					, &States[S_HEAD_DIE+4]),
	S_NORMAL (HEAD, 'G',	7, A_NoBlocking 			, &States[S_HEAD_DIE+5]),
	S_NORMAL (HEAD, 'H',	7, NULL 					, &States[S_HEAD_DIE+6]),
	S_NORMAL (HEAD, 'I',   -1, A_BossDeath				, NULL)
};

void AIronlich::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 6;
	info->spawnstate = &States[S_HEAD_LOOK];
	info->spawnhealth = 700;
	info->seestate = &States[S_HEAD_FLOAT];
	info->seesound = "ironlich/sight";
	info->attacksound = "ironlich/attack";
	info->painstate = &States[S_HEAD_PAIN];
	info->painchance = 32;
	info->painsound = "ironlich/pain";
	info->missilestate = &States[S_HEAD_ATK];
	info->deathstate = &States[S_HEAD_DIE];
	info->deathsound = "ironlich/death";
	info->speed = 6;
	info->radius = 40 * FRACUNIT;
	info->height = 72 * FRACUNIT;
	info->mass = 325;
	info->activesound = "ironlich/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_NOBLOOD;
	info->flags2 = MF2_PASSMOBJ;
	info->flags3 = MF3_DONTMORPH|MF3_DONTSQUASH;
}

void AIronlich::NoBlockingSet ()
{
	P_DropItem (this, "BlasterWimpy", 10, 84);
	P_DropItem (this, "ArtiEgg", 0, 51);
}

// Head FX 1 ----------------------------------------------------------------

class AHeadFX1 : public AActor
{
	DECLARE_ACTOR (AHeadFX1, AActor);
};

IMPLEMENT_DEF_SERIAL (AHeadFX1, AActor);
REGISTER_ACTOR (AHeadFX1, Heretic);

FState AHeadFX1::States[] =
{
#define S_HEADFX1 0
	S_NORMAL (FX05, 'A',	6, NULL 					, &States[S_HEADFX1+1]),
	S_NORMAL (FX05, 'B',	6, NULL 					, &States[S_HEADFX1+2]),
	S_NORMAL (FX05, 'C',	6, NULL 					, &States[S_HEADFX1+0]),

#define S_HEADFXI1 (S_HEADFX1+3)
	S_NORMAL (FX05, 'D',	5, A_LichIceImpact			, &States[S_HEADFXI1+1]),
	S_NORMAL (FX05, 'E',	5, NULL 					, &States[S_HEADFXI1+2]),
	S_NORMAL (FX05, 'F',	5, NULL 					, &States[S_HEADFXI1+3]),
	S_NORMAL (FX05, 'G',	5, NULL 					, NULL)
};

void AHeadFX1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_HEADFX1];
	info->deathstate = &States[S_HEADFXI1];
	info->speed = GameSpeed != SPEED_Fast ? 13 * FRACUNIT : 20 * FRACUNIT;
	info->radius = 12 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 1;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT|MF2_THRUGHOST;
}

// Head FX 2 ----------------------------------------------------------------

class AHeadFX2 : public AActor
{
	DECLARE_ACTOR (AHeadFX2, AActor);
};

IMPLEMENT_DEF_SERIAL (AHeadFX2, AActor);
REGISTER_ACTOR (AHeadFX2, Heretic);

FState AHeadFX2::States[] =
{
#define S_HEADFX2 0
	S_NORMAL (FX05, 'H',	6, NULL 					, &States[S_HEADFX2+1]),
	S_NORMAL (FX05, 'I',	6, NULL 					, &States[S_HEADFX2+2]),
	S_NORMAL (FX05, 'J',	6, NULL 					, &States[S_HEADFX2+0]),

#define S_HEADFXI2 (S_HEADFX2+3)
	S_NORMAL (FX05, 'D',	5, NULL 					, &States[S_HEADFXI2+1]),
	S_NORMAL (FX05, 'E',	5, NULL 					, &States[S_HEADFXI2+2]),
	S_NORMAL (FX05, 'F',	5, NULL 					, &States[S_HEADFXI2+3]),
	S_NORMAL (FX05, 'G',	5, NULL 					, NULL)
};

void AHeadFX2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_HEADFX2];
	info->deathstate = &States[S_HEADFXI2];
	info->speed = 8 * FRACUNIT;
	info->radius = 12 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 3;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT;
}

// Head FX 3 ----------------------------------------------------------------

class AHeadFX3 : public AActor
{
	DECLARE_ACTOR (AHeadFX3, AActor);
};

IMPLEMENT_DEF_SERIAL (AHeadFX3, AActor);
REGISTER_ACTOR (AHeadFX3, Heretic);

FState AHeadFX3::States[] =
{
#define S_HEADFX3 0
	S_NORMAL (FX06, 'A',	4, A_LichFireGrow			, &States[S_HEADFX3+1]),
	S_NORMAL (FX06, 'B',	4, A_LichFireGrow			, &States[S_HEADFX3+2]),
	S_NORMAL (FX06, 'C',	4, A_LichFireGrow			, &States[S_HEADFX3+0]),
	S_NORMAL (FX06, 'A',	5, NULL 					, &States[S_HEADFX3+4]),
	S_NORMAL (FX06, 'B',	5, NULL 					, &States[S_HEADFX3+5]),
	S_NORMAL (FX06, 'C',	5, NULL 					, &States[S_HEADFX3+3]),

#define S_HEADFXI3 (S_HEADFX3+6)
	S_NORMAL (FX06, 'D',	5, NULL 					, &States[S_HEADFXI3+1]),
	S_NORMAL (FX06, 'E',	5, NULL 					, &States[S_HEADFXI3+2]),
	S_NORMAL (FX06, 'F',	5, NULL 					, &States[S_HEADFXI3+3]),
	S_NORMAL (FX06, 'G',	5, NULL 					, NULL)
};

void AHeadFX3::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_HEADFX3];
	info->deathstate = &States[S_HEADFXI3];
	info->speed = GameSpeed != SPEED_Fast ? 10 * FRACUNIT : 18 * FRACUNIT;
	info->radius = 14 * FRACUNIT;
	info->height = 12 * FRACUNIT;
	info->damage = 5;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_WINDTHRUST|MF2_NOTELEPORT;
}

// Whirlwind ----------------------------------------------------------------

class AWhirlwind : public AActor
{
	DECLARE_ACTOR (AWhirlwind, AActor);
public:
	int DoSpecialDamage (AActor *target, int damage);
};

IMPLEMENT_DEF_SERIAL (AWhirlwind, AActor);
REGISTER_ACTOR (AWhirlwind, Heretic);

FState AWhirlwind::States[] =
{
#define S_HEADFX4 0
	S_NORMAL (FX07, 'D',	3, NULL 					, &States[S_HEADFX4+1]),
	S_NORMAL (FX07, 'E',	3, NULL 					, &States[S_HEADFX4+2]),
	S_NORMAL (FX07, 'F',	3, NULL 					, &States[S_HEADFX4+3]),
	S_NORMAL (FX07, 'G',	3, NULL 					, &States[S_HEADFX4+4]),
	S_NORMAL (FX07, 'A',	3, A_WhirlwindSeek			, &States[S_HEADFX4+5]),
	S_NORMAL (FX07, 'B',	3, A_WhirlwindSeek			, &States[S_HEADFX4+6]),
	S_NORMAL (FX07, 'C',	3, A_WhirlwindSeek			, &States[S_HEADFX4+4]),

#define S_HEADFXI4 (S_HEADFX4+7)
	S_NORMAL (FX07, 'G',	4, NULL 					, &States[S_HEADFXI4+1]),
	S_NORMAL (FX07, 'F',	4, NULL 					, &States[S_HEADFXI4+2]),
	S_NORMAL (FX07, 'E',	4, NULL 					, &States[S_HEADFXI4+3]),
	S_NORMAL (FX07, 'D',	4, NULL 					, NULL)
};

void AWhirlwind::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_HEADFX4];
	info->deathstate = &States[S_HEADFXI4];
	info->speed = 10 * FRACUNIT;
	info->radius = 16 * FRACUNIT;
	info->height = 74 * FRACUNIT;
	info->damage = 1;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT;
	info->flags3 = MF3_EXPLOCOUNT;
	info->translucency = HR_SHADOW;
}

int AWhirlwind::DoSpecialDamage (AActor *target, int damage)
{
	int randVal;

	randVal = P_Random ();
	target->angle += (randVal - P_Random()) << 20;
	randVal = P_Random ();
	target->momx += (randVal - P_Random()) << 10;
	randVal = P_Random ();
	target->momy += (randVal - P_Random()) << 10;
	if ((level.time & 16) && !(target->flags2 & MF2_BOSS))
	{
		randVal = P_Random();
		if (randVal > 160)
		{
			randVal = 160;
		}
		target->momz += randVal << 11;
		if (target->momz > 12*FRACUNIT)
		{
			target->momz = 12*FRACUNIT;
		}
	}
	if (!(level.time & 7))
	{
		P_DamageMobj (target, NULL, this->target, 3);
	}
	return -1;
}

//----------------------------------------------------------------------------
//
// PROC A_LichAttack
//
//----------------------------------------------------------------------------

void A_LichAttack (AActor *actor)
{
	int i;
	AActor *fire;
	AActor *baseFire;
	AActor *mo;
	AActor *target;
	int randAttack;
	static const int atkResolve1[] = { 50, 150 };
	static const int atkResolve2[] = { 150, 200 };
	int dist;

	// Ice ball		(close 20% : far 60%)
	// Fire column	(close 40% : far 20%)
	// Whirlwind	(close 40% : far 20%)
	// Distance threshold = 8 cells

	target = actor->target;
	if (target == NULL)
	{
		return;
	}
	A_FaceTarget (actor);
	if (P_CheckMeleeRange (actor))
	{
		P_DamageMobj (target, actor, actor, HITDICE(6));
		return;
	}
	dist = P_AproxDistance (actor->x-target->x, actor->y-target->y)
		> 8*64*FRACUNIT;
	randAttack = P_Random ();
	if (randAttack < atkResolve1[dist])
	{ // Ice ball
		P_SpawnMissile (actor, target, RUNTIME_CLASS(AHeadFX1));
		S_Sound (actor, CHAN_BODY, "ironlich/attack2", 1, ATTN_NORM);
	}
	else if (randAttack < atkResolve2[dist])
	{ // Fire column
		baseFire = P_SpawnMissile (actor, target, RUNTIME_CLASS(AHeadFX3));
		if (baseFire != NULL)
		{
			baseFire->SetState (&AHeadFX3::States[S_HEADFX3+3]); // Don't grow
			for (i = 0; i < 5; i++)
			{
				fire = Spawn<AHeadFX3> (baseFire->x, baseFire->y,
					baseFire->z);
				if (i == 0)
				{
					S_Sound (actor, CHAN_BODY, "ironlich/attack1", 1, ATTN_NORM);
				}
				fire->target = baseFire->target;
				fire->angle = baseFire->angle;
				fire->momx = baseFire->momx;
				fire->momy = baseFire->momy;
				fire->momz = baseFire->momz;
				fire->damage = 0;
				fire->health = (i+1) * 2;
				P_CheckMissileSpawn (fire);
			}
		}
	}
	else
	{ // Whirlwind
		mo = P_SpawnMissile (actor, target, RUNTIME_CLASS(AWhirlwind));
		if (mo != NULL)
		{
			mo->z -= 32*FRACUNIT;
			mo->tracer = target;
			mo->special1 = 60;
			mo->special2 = 50; // Timer for active sound
			mo->health = 20*TICRATE; // Duration
			S_Sound (actor, CHAN_BODY, "ironlich/attack3", 1, ATTN_NORM);
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_WhirlwindSeek
//
//----------------------------------------------------------------------------

void A_WhirlwindSeek (AActor *actor)
{
	actor->health -= 3;
	if (actor->health < 0)
	{
		actor->momx = actor->momy = actor->momz = 0;
		actor->SetState (GetInfo (actor)->deathstate);
		actor->flags &= ~MF_MISSILE;
		return;
	}
	if ((actor->special2 -= 3) < 0)
	{
		actor->special2 = 58 + (P_Random() & 31);
		S_Sound (actor, CHAN_BODY, "ironlich/attack3", 1, ATTN_NORM);
	}
	if (actor->tracer && actor->tracer->flags&MF_SHADOW)
	{
		return;
	}
	P_SeekerMissile (actor, ANGLE_1*10, ANGLE_1*30);
}

//----------------------------------------------------------------------------
//
// PROC A_LichIceImpact
//
//----------------------------------------------------------------------------

void A_LichIceImpact (AActor *ice)
{
	int i;
	angle_t angle;
	AActor *shard;

	for (i = 0; i < 8; i++)
	{
		shard = Spawn<AHeadFX2> (ice->x, ice->y, ice->z);
		angle = i*ANG45;
		shard->target = ice->target;
		shard->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		shard->momx = FixedMul (GetInfo (shard)->speed, finecosine[angle]);
		shard->momy = FixedMul (GetInfo (shard)->speed, finesine[angle]);
		shard->momz = -FRACUNIT*6/10;
		P_CheckMissileSpawn (shard);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_LichFireGrow
//
//----------------------------------------------------------------------------

void A_LichFireGrow (AActor *fire)
{
	fire->health--;
	fire->z += 9*FRACUNIT;
	if (fire->health == 0)
	{
		fire->damage = GetInfo (fire)->damage;
		fire->SetState (&AHeadFX3::States[S_HEADFX3+3]);
	}
}

