#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "a_hereticglobal.h"
#include "a_action.h"
#include "gi.h"

void A_MinotaurDecide (AActor *);
void A_MinotaurAtk1 (AActor *);
void A_MinotaurAtk2 (AActor *);
void A_MinotaurAtk3 (AActor *);
void A_MinotaurCharge (AActor *);
void A_MntrFloorFire (AActor *);

void P_MinotaurSlam (AActor *source, AActor *target);

// Class definitions --------------------------------------------------------

IMPLEMENT_DEF_SERIAL (AMinotaur, AActor);
REGISTER_ACTOR (AMinotaur, Raven);

FState AMinotaur::States[] =
{
#define S_MNTR_LOOK 0
	S_NORMAL (MNTR, 'A',   10, A_Look						, &States[S_MNTR_LOOK+1]),
	S_NORMAL (MNTR, 'B',   10, A_Look						, &States[S_MNTR_LOOK+0]),

#define S_MNTR_WALK (S_MNTR_LOOK+2)
	S_NORMAL (MNTR, 'A',	5, A_Chase						, &States[S_MNTR_WALK+1]),
	S_NORMAL (MNTR, 'B',	5, A_Chase						, &States[S_MNTR_WALK+2]),
	S_NORMAL (MNTR, 'C',	5, A_Chase						, &States[S_MNTR_WALK+3]),
	S_NORMAL (MNTR, 'D',	5, A_Chase						, &States[S_MNTR_WALK+0]),

#define S_MNTR_ATK1 (S_MNTR_WALK+4)
	S_NORMAL (MNTR, 'V',   10, A_FaceTarget 				, &States[S_MNTR_ATK1+1]),
	S_NORMAL (MNTR, 'W',	7, A_FaceTarget 				, &States[S_MNTR_ATK1+2]),
	S_NORMAL (MNTR, 'X',   12, A_MinotaurAtk1				, &States[S_MNTR_WALK+0]),

#define S_MNTR_ATK2 (S_MNTR_ATK1+3)
	S_NORMAL (MNTR, 'V',   10, A_MinotaurDecide 			, &States[S_MNTR_ATK2+1]),
	S_NORMAL (MNTR, 'Y',	4, A_FaceTarget 				, &States[S_MNTR_ATK2+2]),
	S_NORMAL (MNTR, 'Z',	9, A_MinotaurAtk2				, &States[S_MNTR_WALK+0]),

#define S_MNTR_ATK3 (S_MNTR_ATK2+3)
	S_NORMAL (MNTR, 'V',   10, A_FaceTarget 				, &States[S_MNTR_ATK3+1]),
	S_NORMAL (MNTR, 'W',	7, A_FaceTarget 				, &States[S_MNTR_ATK3+2]),
	S_NORMAL (MNTR, 'X',   12, A_MinotaurAtk3				, &States[S_MNTR_WALK+0]),
	S_NORMAL (MNTR, 'X',   12, NULL 						, &States[S_MNTR_ATK3+0]),

#define S_MNTR_ATK4 (S_MNTR_ATK3+4)
	S_NORMAL (MNTR, 'U',	2, A_MinotaurCharge 			, &States[S_MNTR_ATK4+0]),

#define S_MNTR_PAIN (S_MNTR_ATK4+1)
	S_NORMAL (MNTR, 'E',	3, NULL 						, &States[S_MNTR_PAIN+1]),
	S_NORMAL (MNTR, 'E',	6, A_Pain						, &States[S_MNTR_WALK+0]),

#define S_MNTR_DIE (S_MNTR_PAIN+2)
	S_NORMAL (MNTR, 'F',	6, NULL 						, &States[S_MNTR_DIE+1]),
	S_NORMAL (MNTR, 'G',	5, NULL 						, &States[S_MNTR_DIE+2]),
	S_NORMAL (MNTR, 'H',	6, A_Scream 					, &States[S_MNTR_DIE+3]),
	S_NORMAL (MNTR, 'I',	5, NULL 						, &States[S_MNTR_DIE+4]),
	S_NORMAL (MNTR, 'J',	6, NULL 						, &States[S_MNTR_DIE+5]),
	S_NORMAL (MNTR, 'K',	5, NULL 						, &States[S_MNTR_DIE+6]),
	S_NORMAL (MNTR, 'L',	6, NULL 						, &States[S_MNTR_DIE+7]),
	S_NORMAL (MNTR, 'M',	5, A_NoBlocking 				, &States[S_MNTR_DIE+8]),
	S_NORMAL (MNTR, 'N',	6, NULL 						, &States[S_MNTR_DIE+9]),
	S_NORMAL (MNTR, 'O',	5, NULL 						, &States[S_MNTR_DIE+10]),
	S_NORMAL (MNTR, 'P',	6, NULL 						, &States[S_MNTR_DIE+11]),
	S_NORMAL (MNTR, 'Q',	5, NULL 						, &States[S_MNTR_DIE+12]),
	S_NORMAL (MNTR, 'R',	6, NULL 						, &States[S_MNTR_DIE+13]),
	S_NORMAL (MNTR, 'S',	5, NULL 						, &States[S_MNTR_DIE+14]),
	S_NORMAL (MNTR, 'T',   -1, A_BossDeath					, NULL)
};

void AMinotaur::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	if (gameinfo.gametype == GAME_Heretic)
		info->doomednum = 9;
	info->spawnstate = &States[S_MNTR_LOOK];
	info->spawnhealth = 3000;
	info->seestate = &States[S_MNTR_WALK];
	info->seesound = "minotaur/sight";
	info->attacksound = "minotaur/attack1";
	info->painstate = &States[S_MNTR_PAIN];
	info->painchance = 25;
	info->painsound = "minotaur/pain";
	info->meleestate = &States[S_MNTR_ATK1];
	info->missilestate = &States[S_MNTR_ATK2];
	info->deathstate = &States[S_MNTR_DIE];
	info->deathsound = "minotaur/death";
	info->speed = 16;
	info->radius = 28 * FRACUNIT;
	info->height = 100 * FRACUNIT;
	info->mass = 800;
	info->damage = 7;
	info->activesound = "minotaur/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_DROPOFF;
	info->flags2 = MF2_FLOORCLIP|MF2_PASSMOBJ|MF2_BOSS;
	info->flags3 = MF3_NORADIUSDMG|MF3_DONTMORPH|MF3_NOTARGET;
}

void AMinotaur::NoBlockingSet ()
{
	if (gameinfo.gametype == GAME_Heretic)
	{
		P_DropItem (this, "ArtiSuperHealth", 0, 51);
		P_DropItem (this, "PhoenixRodWimpy", 10, 84);
	}
}

int AMinotaur::DoSpecialDamage (AActor *target, int damage)
{
	damage = Super::DoSpecialDamage (target, damage);
	if ((damage != -1) && (flags & MF_SKULLFLY))
	{ // Slam only when in charge mode
		P_MinotaurSlam (this, target);
		return -1;
	}
	return damage;
}

class AMinotaurFX1 : public AActor
{
	DECLARE_ACTOR (AMinotaurFX1, AActor);
};

IMPLEMENT_DEF_SERIAL (AMinotaurFX1, AActor);
REGISTER_ACTOR (AMinotaurFX1, Raven);

FState AMinotaurFX1::States[] =
{
#define S_MNTRFX1 0
	S_BRIGHT (FX12, 'A',	6, NULL 						, &States[S_MNTRFX1+1]),
	S_BRIGHT (FX12, 'B',	6, NULL 						, &States[S_MNTRFX1+0]),

#define S_MNTRFXI1 (S_MNTRFX1+2)
	S_BRIGHT (FX12, 'C',	5, NULL 						, &States[S_MNTRFXI1+1]),
	S_BRIGHT (FX12, 'D',	5, NULL 						, &States[S_MNTRFXI1+2]),
	S_BRIGHT (FX12, 'E',	5, NULL 						, &States[S_MNTRFXI1+3]),
	S_BRIGHT (FX12, 'F',	5, NULL 						, &States[S_MNTRFXI1+4]),
	S_BRIGHT (FX12, 'G',	5, NULL 						, &States[S_MNTRFXI1+5]),
	S_BRIGHT (FX12, 'H',	5, NULL 						, NULL)
};

void AMinotaurFX1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_MNTRFX1];
	info->deathstate = &States[S_MNTRFXI1];
	info->speed = GameSpeed != SPEED_Fast ? 20 * FRACUNIT : 26 * FRACUNIT;
	info->radius = 10 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 3;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT|MF2_FIREDAMAGE;
}

class AMinotaurFX2 : public AMinotaurFX1
{
	DECLARE_ACTOR (AMinotaurFX2, AMinotaurFX1);
public:
	void GetExplodeParms (int &damage, fixed_t &distance, bool &hurtSource)
	{
		damage = 24;
	}
};

IMPLEMENT_DEF_SERIAL (AMinotaurFX2, AMinotaurFX1);
REGISTER_ACTOR (AMinotaurFX2, Raven);

FState AMinotaurFX2::States[] =
{
#define S_MNTRFX2 0
	S_NORMAL (FX13, 'A',	2, A_MntrFloorFire				, &States[S_MNTRFX2+0]),

#define S_MNTRFXI2 (S_MNTRFX2+1)
	S_BRIGHT (FX13, 'I',	4, A_Explode					, &States[S_MNTRFXI2+1]),
	S_BRIGHT (FX13, 'J',	4, NULL 						, &States[S_MNTRFXI2+2]),
	S_BRIGHT (FX13, 'K',	4, NULL 						, &States[S_MNTRFXI2+3]),
	S_BRIGHT (FX13, 'L',	4, NULL 						, &States[S_MNTRFXI2+4]),
	S_BRIGHT (FX13, 'M',	4, NULL 						, NULL)
};

void AMinotaurFX2::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_MNTRFX2];
	info->deathstate = &States[S_MNTRFXI2];
	info->deathsound = "minotaur/fx2hit";
	info->speed = GameSpeed != SPEED_Fast ? 14 * FRACUNIT : 20 * FRACUNIT;
	info->radius = 5 * FRACUNIT;
	info->height = 12 * FRACUNIT;
	info->damage = 4;
	info->flags3 = MF3_FLOORHUGGER;
}

class AMinotaurFX3 : public AMinotaurFX2
{
	DECLARE_ACTOR (AMinotaurFX3, AMinotaurFX2);
public:
	void GetExplodeParms (int &damage, fixed_t &distance, bool &hurtSource)
	{
		damage = 128;
	}
};

IMPLEMENT_DEF_SERIAL (AMinotaurFX3, AMinotaurFX2);
REGISTER_ACTOR (AMinotaurFX3, Raven);

FState AMinotaurFX3::States[] =
{
#define S_MNTRFX3 0
	S_BRIGHT (FX13, 'D',	4, NULL 						, &States[S_MNTRFX3+1]),
	S_BRIGHT (FX13, 'C',	4, NULL 						, &States[S_MNTRFX3+2]),
	S_BRIGHT (FX13, 'B',	5, NULL 						, &States[S_MNTRFX3+3]),
	S_BRIGHT (FX13, 'C',	5, NULL 						, &States[S_MNTRFX3+4]),
	S_BRIGHT (FX13, 'D',	5, NULL 						, &States[S_MNTRFX3+5]),
	S_BRIGHT (FX13, 'E',	5, NULL 						, &States[S_MNTRFX3+6]),
	S_BRIGHT (FX13, 'F',	4, NULL 						, &States[S_MNTRFX3+7]),
	S_BRIGHT (FX13, 'G',	4, NULL 						, &States[S_MNTRFX3+8]),
	S_BRIGHT (FX13, 'H',	4, NULL 						, NULL)
};

void AMinotaurFX3::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_MNTRFX3];
	info->deathsound = "minotaur/fx3hit";
	info->speed = 0;
	info->radius = 8 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags3 = 0;
}

// Action functions for the minotaur ----------------------------------------

//----------------------------------------------------------------------------
//
// PROC A_MinotaurAtk1
//
// Melee attack.
//
//----------------------------------------------------------------------------

void A_MinotaurAtk1 (AActor *actor)
{
	player_t *player;

	if (!actor->target)
	{
		return;
	}
	S_Sound (actor, CHAN_WEAPON, "minotaur/melee", 1, ATTN_NORM);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj (actor->target, actor, actor, HITDICE(4));
		if ((player = actor->target->player) != NULL)
		{ // Squish the player
			player->deltaviewheight = -16*FRACUNIT;
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurDecide
//
// Choose a missile attack.
//
//----------------------------------------------------------------------------

#define MNTR_CHARGE_SPEED (13*FRACUNIT)

void A_MinotaurDecide (AActor *actor)
{
	angle_t angle;
	AActor *target;
	int dist;

	target = actor->target;
	if (!target)
	{
		return;
	}
	S_Sound (actor, CHAN_WEAPON, "minotaur/sight", 1, ATTN_NORM);
	dist = P_AproxDistance (actor->x-target->x, actor->y-target->y);
	if (target->z+target->height > actor->z
		&& target->z+target->height < actor->z+actor->height
		&& dist < 8*64*FRACUNIT
		&& dist > 1*64*FRACUNIT
		&& P_Random() < 150)
	{ // Charge attack
		// Don't call the state function right away
		actor->SetStateNF (&AMinotaur::States[S_MNTR_ATK4]);
		actor->flags |= MF_SKULLFLY;
		A_FaceTarget (actor);
		angle = actor->angle>>ANGLETOFINESHIFT;
		actor->momx = FixedMul (MNTR_CHARGE_SPEED, finecosine[angle]);
		actor->momy = FixedMul (MNTR_CHARGE_SPEED, finesine[angle]);
		actor->special1 = 35/2; // Charge duration
	}
	else if (target->z == target->floorz
		&& dist < 9*64*FRACUNIT
		&& P_Random() < 220)
	{ // Floor fire attack
		actor->SetState (&AMinotaur::States[S_MNTR_ATK3]);
		actor->special2 = 0;
	}
	else
	{ // Swing attack
		A_FaceTarget (actor);
		// Don't need to call P_SetMobjState because the current state
		// falls through to the swing attack
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurCharge
//
//----------------------------------------------------------------------------

void A_MinotaurCharge (AActor *actor)
{
	AActor *puff;

	if (actor->special1)
	{
		puff = Spawn<APhoenixPuff> (actor->x, actor->y, actor->z);
		puff->momz = 2*FRACUNIT;
		actor->special1--;
	}
	else
	{
		actor->flags &= ~MF_SKULLFLY;
		actor->SetState (GetInfo (actor)->seestate);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurAtk2
//
// Swing attack.
//
//----------------------------------------------------------------------------

void A_MinotaurAtk2 (AActor *actor)
{
	AActor *mo;
	angle_t angle;
	fixed_t momz;
	fixed_t z;

	if (!actor->target)
	{
		return;
	}
	S_Sound (actor, CHAN_WEAPON, "minotaur/attack2", 1, ATTN_NORM);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj (actor->target, actor, actor, HITDICE(5));
		return;
	}
	z = actor->z + 40*FRACUNIT;
	mo = P_SpawnMissileZ (actor, z, actor->target, RUNTIME_CLASS(AMinotaurFX1));
	if (mo)
	{
		S_Sound (mo, CHAN_WEAPON, "minotaur/attack2", 1, ATTN_NORM);
		momz = mo->momz;
		angle = mo->angle;
		P_SpawnMissileAngleZ (actor, z, RUNTIME_CLASS(AMinotaurFX1), angle-(ANG45/8), momz);
		P_SpawnMissileAngleZ (actor, z, RUNTIME_CLASS(AMinotaurFX1), angle+(ANG45/8), momz);
		P_SpawnMissileAngleZ (actor, z, RUNTIME_CLASS(AMinotaurFX1), angle-(ANG45/16), momz);
		P_SpawnMissileAngleZ (actor, z, RUNTIME_CLASS(AMinotaurFX1), angle+(ANG45/16), momz);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MinotaurAtk3
//
// Floor fire attack.
//
//----------------------------------------------------------------------------

void A_MinotaurAtk3 (AActor *actor)
{
	AActor *mo;
	player_t *player;

	if (!actor->target)
	{
		return;
	}
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj (actor->target, actor, actor, HITDICE(5));
		if ((player = actor->target->player) != NULL)
		{ // Squish the player
			player->deltaviewheight = -16*FRACUNIT;
		}
	}
	else
	{
		mo = P_SpawnMissileZ (actor, ONFLOORZ, actor->target, RUNTIME_CLASS(AMinotaurFX2));
		if (mo != NULL)
		{
			S_Sound (mo, CHAN_WEAPON, "minotaur/attack1", 1, ATTN_NORM);
		}
	}
	if (P_Random() < 192 && actor->special2 == 0)
	{
		actor->SetState (&AMinotaur::States[S_MNTR_ATK3+3]);
		actor->special2 = 1;
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MntrFloorFire
//
//----------------------------------------------------------------------------

void A_MntrFloorFire (AActor *actor)
{
	AActor *mo;
	fixed_t x, y;

	actor->z = actor->floorz;
	x = actor->x + (PS_Random () << 10);
	y = actor->y + (PS_Random () << 10);
	mo = Spawn<AMinotaurFX3> (x, y, ONFLOORZ);
	mo->target = actor->target;
	mo->momx = 1; // Force block checking
	P_CheckMissileSpawn (mo);
}

//---------------------------------------------------------------------------
//
// FUNC P_MinotaurSlam
//
//---------------------------------------------------------------------------

void P_MinotaurSlam (AActor *source, AActor *target)
{
	angle_t angle;
	fixed_t thrust;

	angle = R_PointToAngle2 (source->x, source->y, target->x, target->y);
	angle >>= ANGLETOFINESHIFT;
	thrust = 16*FRACUNIT+(P_Random()<<10);
	target->momx += FixedMul (thrust, finecosine[angle]);
	target->momy += FixedMul (thrust, finesine[angle]);
	P_DamageMobj (target, NULL, NULL, HITDICE(6));
	if (target->player)
	{
		target->reactiontime = 14+(P_Random()&7);
	}
}
