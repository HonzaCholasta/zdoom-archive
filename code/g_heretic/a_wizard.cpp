#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "a_hereticglobal.h"
#include "a_action.h"

void A_WizAtk1 (AActor *);
void A_WizAtk2 (AActor *);
void A_WizAtk3 (AActor *);
void A_GhostOff (AActor *);

// Class definitions --------------------------------------------------------

IMPLEMENT_DEF_SERIAL (AWizard, AActor);
REGISTER_ACTOR (AWizard, Heretic);

FState AWizard::States[] =
{
#define S_WIZARD_LOOK 0
	S_NORMAL (WZRD, 'A',   10, A_Look						, &States[S_WIZARD_LOOK+1]),
	S_NORMAL (WZRD, 'B',   10, A_Look						, &States[S_WIZARD_LOOK+0]),

#define S_WIZARD_WALK (S_WIZARD_LOOK+2)
	S_NORMAL (WZRD, 'A',	3, A_Chase						, &States[S_WIZARD_WALK+1]),
	S_NORMAL (WZRD, 'A',	4, A_Chase						, &States[S_WIZARD_WALK+2]),
	S_NORMAL (WZRD, 'A',	3, A_Chase						, &States[S_WIZARD_WALK+3]),
	S_NORMAL (WZRD, 'A',	4, A_Chase						, &States[S_WIZARD_WALK+4]),
	S_NORMAL (WZRD, 'B',	3, A_Chase						, &States[S_WIZARD_WALK+5]),
	S_NORMAL (WZRD, 'B',	4, A_Chase						, &States[S_WIZARD_WALK+6]),
	S_NORMAL (WZRD, 'B',	3, A_Chase						, &States[S_WIZARD_WALK+7]),
	S_NORMAL (WZRD, 'B',	4, A_Chase						, &States[S_WIZARD_WALK+0]),

#define S_WIZARD_ATK (S_WIZARD_WALK+8)
	S_NORMAL (WZRD, 'C',	4, A_WizAtk1					, &States[S_WIZARD_ATK+1]),
	S_NORMAL (WZRD, 'C',	4, A_WizAtk2					, &States[S_WIZARD_ATK+2]),
	S_NORMAL (WZRD, 'C',	4, A_WizAtk1					, &States[S_WIZARD_ATK+3]),
	S_NORMAL (WZRD, 'C',	4, A_WizAtk2					, &States[S_WIZARD_ATK+4]),
	S_NORMAL (WZRD, 'C',	4, A_WizAtk1					, &States[S_WIZARD_ATK+5]),
	S_NORMAL (WZRD, 'C',	4, A_WizAtk2					, &States[S_WIZARD_ATK+6]),
	S_NORMAL (WZRD, 'C',	4, A_WizAtk1					, &States[S_WIZARD_ATK+7]),
	S_NORMAL (WZRD, 'C',	4, A_WizAtk2					, &States[S_WIZARD_ATK+8]),
	S_NORMAL (WZRD, 'D',   12, A_WizAtk3					, &States[S_WIZARD_WALK+0]),

#define S_WIZARD_PAIN (S_WIZARD_ATK+9)
	S_NORMAL (WZRD, 'E',	3, A_GhostOff					, &States[S_WIZARD_PAIN+1]),
	S_NORMAL (WZRD, 'E',	3, A_Pain						, &States[S_WIZARD_WALK+0]),

#define S_WIZARD_DIE (S_WIZARD_PAIN+2)
	S_NORMAL (WZRD, 'F',	6, A_GhostOff					, &States[S_WIZARD_DIE+1]),
	S_NORMAL (WZRD, 'G',	6, A_Scream 					, &States[S_WIZARD_DIE+2]),
	S_NORMAL (WZRD, 'H',	6, NULL 						, &States[S_WIZARD_DIE+3]),
	S_NORMAL (WZRD, 'I',	6, NULL 						, &States[S_WIZARD_DIE+4]),
	S_NORMAL (WZRD, 'J',	6, A_NoBlocking 				, &States[S_WIZARD_DIE+5]),
	S_NORMAL (WZRD, 'K',	6, NULL 						, &States[S_WIZARD_DIE+6]),
	S_NORMAL (WZRD, 'L',	6, NULL 						, &States[S_WIZARD_DIE+7]),
	S_NORMAL (WZRD, 'M',   -1, NULL 						, NULL)
};

void AWizard::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 15;
	info->spawnstate = &States[S_WIZARD_LOOK];
	info->spawnhealth = 100;
	info->seestate = &States[S_WIZARD_WALK];
	info->seesound = "wizard/sight";
	info->attacksound = "wizard/attack";
	info->painstate = &States[S_WIZARD_PAIN];
	info->painchance = 64;
	info->painsound = "wizard/pain";
	info->missilestate = &States[S_WIZARD_ATK];
	info->deathstate = &States[S_WIZARD_DIE];
	info->deathsound = "wizard/death";
	info->speed = 12;
	info->radius = 16 * FRACUNIT;
	info->height = 68 * FRACUNIT;
	info->mass = 100;
	info->activesound = "wizard/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_FLOAT|MF_NOGRAVITY;
	info->flags2 = MF2_PASSMOBJ;
	info->flags3 = MF3_SEEISALSOACTIVE|MF3_DONTOVERLAP;
}

void AWizard::NoBlockingSet ()
{
	P_DropItem (this, "BlasterWimpy", 10, 84);
	P_DropItem (this, "ArtiTomeOfPower", 0, 4);
}

bool AWizard::NewTarget (AActor *other)
{
	return !other->IsKindOf (RUNTIME_CLASS(ASorcerer2));
}

class AWizardFX1 : public AActor
{
	DECLARE_ACTOR (AWizardFX1, AActor);
};

IMPLEMENT_DEF_SERIAL (AWizardFX1, AActor);
REGISTER_ACTOR (AWizardFX1, Heretic);

FState AWizardFX1::States[] =
{
#define S_WIZFX1 0
	S_BRIGHT (FX11, 'A',	6, NULL 						, &States[S_WIZFX1+1]),
	S_BRIGHT (FX11, 'B',	6, NULL 						, &States[S_WIZFX1+0]),

#define S_WIZFXI1 (S_WIZFX1+2)
	S_BRIGHT (FX11, 'C',	5, NULL 						, &States[S_WIZFXI1+1]),
	S_BRIGHT (FX11, 'D',	5, NULL 						, &States[S_WIZFXI1+2]),
	S_BRIGHT (FX11, 'E',	5, NULL 						, &States[S_WIZFXI1+3]),
	S_BRIGHT (FX11, 'F',	5, NULL 						, &States[S_WIZFXI1+4]),
	S_BRIGHT (FX11, 'G',	5, NULL 						, NULL)
};

void AWizardFX1::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_WIZFX1];
	info->deathstate = &States[S_WIZFXI1];
	info->speed = GameSpeed != SPEED_Fast ? 18 * FRACUNIT : 24 * FRACUNIT;
	info->radius = 10 * FRACUNIT;
	info->height = 6 * FRACUNIT;
	info->damage = 3;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_NOTELEPORT;
}

// --- Action functions -----------------------------------------------------

//----------------------------------------------------------------------------
//
// PROC A_GhostOff
//
//----------------------------------------------------------------------------

void A_GhostOff (AActor *actor)
{
	actor->translucency = OPAQUE;
	actor->flags3 &= ~MF3_GHOST;
}

//----------------------------------------------------------------------------
//
// PROC A_WizAtk1
//
//----------------------------------------------------------------------------

void A_WizAtk1 (AActor *actor)
{
	A_FaceTarget (actor);
	A_GhostOff (actor);
}

//----------------------------------------------------------------------------
//
// PROC A_WizAtk2
//
//----------------------------------------------------------------------------

void A_WizAtk2 (AActor *actor)
{
	A_FaceTarget (actor);
	actor->translucency = HR_SHADOW;
	actor->flags3 |= MF3_GHOST;
}

//----------------------------------------------------------------------------
//
// PROC A_WizAtk3
//
//----------------------------------------------------------------------------

void A_WizAtk3 (AActor *actor)
{
	AActor *mo;

	A_GhostOff (actor);
	if (!actor->target)
	{
		return;
	}
	S_Sound (actor, CHAN_WEAPON, GetInfo (actor)->attacksound, 1, ATTN_NORM);
	if (P_CheckMeleeRange(actor))
	{
		P_DamageMobj (actor->target, actor, actor, HITDICE(4));
		return;
	}
	mo = P_SpawnMissile (actor, actor->target, RUNTIME_CLASS(AWizardFX1));
	if (mo)
	{
		P_SpawnMissileAngle(actor, RUNTIME_CLASS(AWizardFX1), mo->angle-(ANG45/8), mo->momz);
		P_SpawnMissileAngle(actor, RUNTIME_CLASS(AWizardFX1), mo->angle+(ANG45/8), mo->momz);
	}
}
