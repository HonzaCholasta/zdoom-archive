#include "templates.h"
#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "a_doomglobal.h"
#include "gi.h"
#include "gstrings.h"
#include "a_action.h"

#include "p_grubber.h"	// [GRB]

void A_SkullAttack (AActor *);

FState ALostSoul::States[] =
{
#define S_SKULL_STND 0
	S_BRIGHT (SKUL, 'A',   10, A_Look						, &States[S_SKULL_STND+1]),
	S_BRIGHT (SKUL, 'B',   10, A_Look						, &States[S_SKULL_STND]),

#define S_SKULL_RUN (S_SKULL_STND+2)
	S_BRIGHT (SKUL, 'A',	6, A_Chase						, &States[S_SKULL_RUN+1]),
	S_BRIGHT (SKUL, 'B',	6, A_Chase						, &States[S_SKULL_RUN+0]),

#define S_SKULL_ATK (S_SKULL_RUN+2)
	S_BRIGHT (SKUL, 'C',   10, A_FaceTarget 				, &States[S_SKULL_ATK+1]),
	S_BRIGHT (SKUL, 'D',	4, A_SkullAttack				, &States[S_SKULL_ATK+2]),
	S_BRIGHT (SKUL, 'C',	4, NULL 						, &States[S_SKULL_ATK+3]),
	S_BRIGHT (SKUL, 'D',	4, NULL 						, &States[S_SKULL_ATK+2]),

#define S_SKULL_PAIN (S_SKULL_ATK+4)
	S_BRIGHT (SKUL, 'E',	3, NULL 						, &States[S_SKULL_PAIN+1]),
	S_BRIGHT (SKUL, 'E',	3, A_Pain						, &States[S_SKULL_RUN+0]),

#define S_SKULL_DIE (S_SKULL_PAIN+2)
	S_BRIGHT (SKUL, 'F',	6, NULL 						, &States[S_SKULL_DIE+1]),
	S_BRIGHT (SKUL, 'G',	6, A_Scream 					, &States[S_SKULL_DIE+2]),
	S_BRIGHT (SKUL, 'H',	6, NULL 						, &States[S_SKULL_DIE+3]),
	S_BRIGHT (SKUL, 'I',	6, A_NoBlocking					, &States[S_SKULL_DIE+4]),
	S_NORMAL (SKUL, 'J',	6, NULL 						, &States[S_SKULL_DIE+5]),
	S_NORMAL (SKUL, 'K',	6, NULL 						, NULL)
};

IMPLEMENT_ACTOR (ALostSoul, Doom, 3006, 110)
	PROP_SpawnHealth (100)
	PROP_RadiusFixed (16)
	PROP_HeightFixed (56)
	PROP_Mass (50)
	PROP_SpeedFixed (8)
	PROP_Damage (3)
	PROP_MaxPainChance
	PROP_Flags (MF_SOLID|MF_SHOOTABLE|MF_FLOAT|MF_NOGRAVITY)
	PROP_Flags2 (MF2_MCROSS|MF2_PUSHWALL|MF2_PASSMOBJ)
	PROP_RenderStyle (STYLE_SoulTrans)

	PROP_SpawnState (S_SKULL_STND)
	PROP_SeeState (S_SKULL_RUN)
	PROP_PainState (S_SKULL_PAIN)
	PROP_MissileState (S_SKULL_ATK)
	PROP_DeathState (S_SKULL_DIE)

	PROP_AttackSound ("skull/melee")
	PROP_PainSound ("skull/pain")
	PROP_DeathSound ("skull/death")
	PROP_ActiveSound ("skull/active")
END_DEFAULTS

const char *ALostSoul::GetObituary ()
{
	return GStrings(OB_SKULL);
}

bool ALostSoul::SuggestMissileAttack (fixed_t dist)
{
	return P_Random (pr_checkmissilerange) >= MIN<int> (dist >> (FRACBITS + 1), 200);
}

void ALostSoul::Die (AActor *source, AActor *inflictor)
{
	Super::Die (source, inflictor);
	flags |= MF_NOGRAVITY;
}

//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED (20*FRACUNIT)

void A_SkullAttack (AActor *self)
{
	AActor *dest;
//	angle_t an;
//	int dist;

	if (!self->target)
		return;
				
	dest = self->target;		
	self->flags |= MF_SKULLFLY;

	S_SoundID (self, CHAN_VOICE, self->AttackSound, 1, ATTN_NORM);
	A_FaceTarget (self);
	P_MonsterFire (cl_mon_lostsoul_fire, self, self->angle, P_AimLineAttack (self, self->angle, MISSILERANGE));
}

AActor *Grb_SkullAttack (AActor *self, int angle)	// [GRB]
{
	angle_t an = self->angle >> ANGLETOFINESHIFT;

	if (self->damage == 0)
		self->damage = 3;

	self->momx = FixedMul (SKULLSPEED, finecosine[an]);
	self->momy = FixedMul (SKULLSPEED, finesine[an]);
	int dist = P_AproxDistance (self->target->x - self->x, self->target->y - self->y);
	dist = dist / SKULLSPEED;
	
	if (dist < 1)
		dist = 1;
	self->momz = (self->target->z+(self->target->height>>1) - self->z) / dist;

	return self;
}

// Dead lost soul ----------------------------------------------------------

/* [RH] Considering that the lost soul removes itself when it dies, there
 * really wasn't much point in id including this thing, but they did anyway.
 * (There was probably a time when it stayed around after death, and this is
 * a holdover from that.)
 */

class ADeadLostSoul : public ALostSoul
{
	DECLARE_STATELESS_ACTOR (ADeadLostSoul, ALostSoul)
};

IMPLEMENT_STATELESS_ACTOR (ADeadLostSoul, Doom, 23, 0)
	PROP_SKIP_SUPER
	PROP_SpawnState (S_SKULL_DIE+5)
END_DEFAULTS

//==========================================================================
//
// CVAR transsouls
//
// How translucent things drawn with STYLE_SoulTrans are. Normally, only
// Lost Souls have this render style, but a dehacked patch could give other
// things this style. Values less than 0.25 will automatically be set to
// 0.25 to ensure some degree of visibility. Likewise, values above 1.0 will
// be set to 1.0, because anything higher doesn't make sense.
//
//==========================================================================

CUSTOM_CVAR (Float, transsouls, 0.75f, CVAR_ARCHIVE)
{
	if (self < 0.25f)
	{
		self = 0.25f;
	}
	else if (self > 1.f)
	{
		self = 1.f;
	}
}
