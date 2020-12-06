#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "a_doomglobal.h"
#include "gi.h"
#include "dstrings.h"
#include "a_action.h"

void A_SkullAttack (AActor *);

IMPLEMENT_DEF_SERIAL (ALostSoul, AActor);
REGISTER_ACTOR (ALostSoul, Doom);

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

void ALostSoul::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 3006;
	info->spawnid = 110;
	info->spawnstate = &States[S_SKULL_STND];
	info->spawnhealth = 100;
	info->seestate = &States[S_SKULL_RUN];
	info->attacksound = "skull/melee";
	info->painstate = &States[S_SKULL_PAIN];
	info->painchance = 256;
	info->painsound = "skull/pain";
	info->missilestate = &States[S_SKULL_ATK];
	info->deathstate = &States[S_SKULL_DIE];
	info->deathsound = "skull/death";
	info->speed = 8;
	info->radius = 16 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 50;
	info->damage = 3;
	info->activesound = "skull/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_FLOAT|MF_NOGRAVITY;
	info->flags2 = MF2_MCROSS|MF2_PUSHWALL|MF2_PASSMOBJ;
}

const char *ALostSoul::GetObituary ()
{
	return OB_SKULL;
}

bool ALostSoul::SuggestMissileAttack (fixed_t dist)
{
	return P_Random (pr_checkmissilerange) >= MIN (dist >> (FRACBITS + 1), 200);
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
	angle_t an;
	int dist;

	if (!self->target)
		return;
				
	dest = self->target;		
	self->flags |= MF_SKULLFLY;

	S_Sound (self, CHAN_VOICE, GetInfo (self)->attacksound, 1, ATTN_NORM);
	A_FaceTarget (self);
	an = self->angle >> ANGLETOFINESHIFT;
	self->momx = FixedMul (SKULLSPEED, finecosine[an]);
	self->momy = FixedMul (SKULLSPEED, finesine[an]);
	dist = P_AproxDistance (dest->x - self->x, dest->y - self->y);
	dist = dist / SKULLSPEED;
	
	if (dist < 1)
		dist = 1;
	self->momz = (dest->z+(dest->height>>1) - self->z) / dist;
}

// Dead lost soul ----------------------------------------------------------

/* [RH] Considering that the lost soul removes itself when it dies, there
 * really wasn't much point in id including this thing, but they did anyway.
 * (There was probably a time when it stayed around after death, and this is
 * a holdover from that.)
 */

class ADeadLostSoul : public ALostSoul
{
	DECLARE_STATELESS_ACTOR (ADeadLostSoul, ALostSoul);
};

IMPLEMENT_DEF_SERIAL (ADeadLostSoul, ALostSoul);
REGISTER_ACTOR (ADeadLostSoul, Doom);

void ADeadLostSoul::SetDefaults (FActorInfo *info)
{
	AActor::SetDefaults (info);
	info->OwnedStates = NULL;
	info->NumOwnedStates = 0;
	info->doomednum = 23;
	info->spawnstate = &States[S_SKULL_DIE+5];
}

//==========================================================================
//
// TransSoulsCallabck
//
// [RH] This function is called whenever the lost soul translucency level
// changes. It searches through the entire world for any lost souls and sets
// their translucency levels as appropriate. New skulls are also set to
// spawn with the desired translucency.
//
//==========================================================================

BEGIN_CUSTOM_CVAR (transsouls, "0.75", CVAR_ARCHIVE)
{
	if (var.value < 0.25)
	{
		var.Set (0.25);
	}
	else if (var.value > 1)
	{
		var.Set (1);
	}
	else
	{
		if (gameinfo.gametype != GAME_Doom)
			return;

		FActorInfo *info = RUNTIME_CLASS(ALostSoul)->ActorInfo;
		if (info)
		{
			fixed_t newlucent = (fixed_t)(FRACUNIT * var.value);

			info->translucency = newlucent;

			// Find all the lost souls in the world and change them, too.
			AActor *actor;
			TThinkerIterator<ALostSoul> iterator;

			while ( (actor = iterator.Next ()) )
			{
				actor->translucency = newlucent;
			}
		}
	}
}
END_CUSTOM_CVAR (transsouls)
