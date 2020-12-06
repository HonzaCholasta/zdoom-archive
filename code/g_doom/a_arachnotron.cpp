#include "actor.h"
#include "info.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "a_doomglobal.h"
#include "dstrings.h"
#include "a_action.h"

void A_BspiAttack (AActor *self);
void A_BabyMetal (AActor *self);
void A_SpidRefire (AActor *self);

class AArachnotron : public AActor
{
	DECLARE_ACTOR (AArachnotron, AActor);
public:
	const char *GetObituary () { return OB_BABY; }
};

IMPLEMENT_DEF_SERIAL (AArachnotron, AActor);
REGISTER_ACTOR (AArachnotron, Doom);

FState AArachnotron::States[] =
{
#define S_BSPI_STND 0
	S_NORMAL (BSPI, 'A',   10, A_Look						, &States[S_BSPI_STND+1]),
	S_NORMAL (BSPI, 'B',   10, A_Look						, &States[S_BSPI_STND]),

#define S_BSPI_SIGHT (S_BSPI_STND+2)
#define S_BSPI_RUN (S_BSPI_SIGHT+1)
	S_NORMAL (BSPI, 'A',   20, NULL 						, &States[S_BSPI_RUN+0]),

	S_NORMAL (BSPI, 'A',	3, A_BabyMetal					, &States[S_BSPI_RUN+1]),
	S_NORMAL (BSPI, 'A',	3, A_Chase						, &States[S_BSPI_RUN+2]),
	S_NORMAL (BSPI, 'B',	3, A_Chase						, &States[S_BSPI_RUN+3]),
	S_NORMAL (BSPI, 'B',	3, A_Chase						, &States[S_BSPI_RUN+4]),
	S_NORMAL (BSPI, 'C',	3, A_Chase						, &States[S_BSPI_RUN+5]),
	S_NORMAL (BSPI, 'C',	3, A_Chase						, &States[S_BSPI_RUN+6]),
	S_NORMAL (BSPI, 'D',	3, A_BabyMetal					, &States[S_BSPI_RUN+7]),
	S_NORMAL (BSPI, 'D',	3, A_Chase						, &States[S_BSPI_RUN+8]),
	S_NORMAL (BSPI, 'E',	3, A_Chase						, &States[S_BSPI_RUN+9]),
	S_NORMAL (BSPI, 'E',	3, A_Chase						, &States[S_BSPI_RUN+10]),
	S_NORMAL (BSPI, 'F',	3, A_Chase						, &States[S_BSPI_RUN+11]),
	S_NORMAL (BSPI, 'F',	3, A_Chase						, &States[S_BSPI_RUN+0]),

#define S_BSPI_ATK (S_BSPI_RUN+12)
	S_BRIGHT (BSPI, 'A',   20, A_FaceTarget 				, &States[S_BSPI_ATK+1]),
	S_BRIGHT (BSPI, 'G',	4, A_BspiAttack 				, &States[S_BSPI_ATK+2]),
	S_BRIGHT (BSPI, 'H',	4, NULL 						, &States[S_BSPI_ATK+3]),
	S_BRIGHT (BSPI, 'H',	1, A_SpidRefire 				, &States[S_BSPI_ATK+1]),

#define S_BSPI_PAIN (S_BSPI_ATK+4)
	S_NORMAL (BSPI, 'I',	3, NULL 						, &States[S_BSPI_PAIN+1]),
	S_NORMAL (BSPI, 'I',	3, A_Pain						, &States[S_BSPI_RUN+0]),

#define S_BSPI_DIE (S_BSPI_PAIN+2)
	S_NORMAL (BSPI, 'J',   20, A_Scream 					, &States[S_BSPI_DIE+1]),
	S_NORMAL (BSPI, 'K',	7, A_NoBlocking					, &States[S_BSPI_DIE+2]),
	S_NORMAL (BSPI, 'L',	7, NULL 						, &States[S_BSPI_DIE+3]),
	S_NORMAL (BSPI, 'M',	7, NULL 						, &States[S_BSPI_DIE+4]),
	S_NORMAL (BSPI, 'N',	7, NULL 						, &States[S_BSPI_DIE+5]),
	S_NORMAL (BSPI, 'O',	7, NULL 						, &States[S_BSPI_DIE+6]),
	S_NORMAL (BSPI, 'P',   -1, A_BossDeath					, NULL),

#define S_BSPI_RAISE (S_BSPI_DIE+7)
	S_NORMAL (BSPI, 'P',	5, NULL 						, &States[S_BSPI_RAISE+1]),
	S_NORMAL (BSPI, 'O',	5, NULL 						, &States[S_BSPI_RAISE+2]),
	S_NORMAL (BSPI, 'N',	5, NULL 						, &States[S_BSPI_RAISE+3]),
	S_NORMAL (BSPI, 'M',	5, NULL 						, &States[S_BSPI_RAISE+4]),
	S_NORMAL (BSPI, 'L',	5, NULL 						, &States[S_BSPI_RAISE+5]),
	S_NORMAL (BSPI, 'K',	5, NULL 						, &States[S_BSPI_RAISE+6]),
	S_NORMAL (BSPI, 'J',	5, NULL 						, &States[S_BSPI_RUN+0])
};

void AArachnotron::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 68;
	info->spawnid = 6;
	info->spawnstate = &States[S_BSPI_STND];
	info->spawnhealth = 500;
	info->seestate = &States[S_BSPI_SIGHT];
	info->seesound = "baby/sight";
	info->painstate = &States[S_BSPI_PAIN];
	info->painchance = 128;
	info->painsound = "baby/pain";
	info->missilestate = &States[S_BSPI_ATK];
	info->deathstate = &States[S_BSPI_DIE];
	info->deathsound = "baby/death";
	info->speed = 12;
	info->radius = 64 * FRACUNIT;
	info->height = 64 * FRACUNIT;
	info->mass = 600;
	info->activesound = "baby/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL;
	info->raisestate = &States[S_BSPI_RAISE];
}

class AStealthArachnotron : public AArachnotron
{
	DECLARE_STATELESS_ACTOR (AStealthArachnotron, AArachnotron);
public:
	const char *GetObituary () { return OB_STEALTHBABY; }
};

IMPLEMENT_DEF_SERIAL (AStealthArachnotron, AArachnotron);
REGISTER_ACTOR (AStealthArachnotron, Doom);

void AStealthArachnotron::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9050;
	info->spawnid = 117;
	info->flags |= MF_STEALTH;
	info->translucency = 0;
}

class AArachnotronPlasma : public APlasmaBall
{
	DECLARE_ACTOR (AArachnotronPlasma, APlasmaBall);
};

IMPLEMENT_DEF_SERIAL (AArachnotronPlasma, APlasmaBall);
REGISTER_ACTOR (AArachnotronPlasma, Doom);

FState AArachnotronPlasma::States[] =
{
#define S_ARACH_PLAZ 0
	S_BRIGHT (APLS, 'A',	5, NULL 						, &States[S_ARACH_PLAZ+1]),
	S_BRIGHT (APLS, 'B',	5, NULL 						, &States[S_ARACH_PLAZ]),

#define S_ARACH_PLEX (S_ARACH_PLAZ+2)
	S_BRIGHT (APBX, 'A',	5, NULL 						, &States[S_ARACH_PLEX+1]),
	S_BRIGHT (APBX, 'B',	5, NULL 						, &States[S_ARACH_PLEX+2]),
	S_BRIGHT (APBX, 'C',	5, NULL 						, &States[S_ARACH_PLEX+3]),
	S_BRIGHT (APBX, 'D',	5, NULL 						, &States[S_ARACH_PLEX+4]),
	S_BRIGHT (APBX, 'E',	5, NULL 						, NULL)
};

void AArachnotronPlasma::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnid = 129;
	info->spawnstate = &States[S_ARACH_PLAZ];
	info->seesound = "baby/attack";
	info->deathstate = &States[S_ARACH_PLEX];
	info->deathsound = "baby/shotx";
	info->speed = 25 * FRACUNIT;
	info->radius = 13 * FRACUNIT;
	info->height = 8 * FRACUNIT;
	info->damage = 5;
	info->flags = MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY;
	info->flags2 = MF2_PCROSS|MF2_IMPACT;
	info->translucency = TRANSLUC66;
}

void A_BspiAttack (AActor *self)
{		
	if (!self->target)
		return;

	// [RH] Andy Baker's stealth monsters
	if (self->flags & MF_STEALTH)
	{
		self->visdir = 1;
	}

	A_FaceTarget (self);

	// launch a missile
	P_SpawnMissile (self, self->target, RUNTIME_CLASS(AArachnotronPlasma));
}

void A_BabyMetal (AActor *self)
{
	S_Sound (self, CHAN_BODY, "baby/walk", 1, ATTN_IDLE);
	A_Chase (self);
}
