#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "p_enemy.h"
#include "a_doomglobal.h"
#include "dstrings.h"
#include "a_action.h"

void A_CyberAttack (AActor *);
void A_Hoof (AActor *);
void A_Metal (AActor *);

class ACyberdemon : public AActor
{
	DECLARE_ACTOR (ACyberdemon, AActor);
public:
	bool SuggestMissileAttack (fixed_t dist);
	const char *GetObituary () { return OB_CYBORG; }
};

IMPLEMENT_DEF_SERIAL (ACyberdemon, AActor);
REGISTER_ACTOR (ACyberdemon, Doom);

FState ACyberdemon::States[] =
{
#define S_CYBER_STND 0
	S_NORMAL (CYBR, 'A',   10, A_Look						, &States[S_CYBER_STND+1]),
	S_NORMAL (CYBR, 'B',   10, A_Look						, &States[S_CYBER_STND]),

#define S_CYBER_RUN (S_CYBER_STND+2)
	S_NORMAL (CYBR, 'A',	3, A_Hoof						, &States[S_CYBER_RUN+1]),
	S_NORMAL (CYBR, 'A',	3, A_Chase						, &States[S_CYBER_RUN+2]),
	S_NORMAL (CYBR, 'B',	3, A_Chase						, &States[S_CYBER_RUN+3]),
	S_NORMAL (CYBR, 'B',	3, A_Chase						, &States[S_CYBER_RUN+4]),
	S_NORMAL (CYBR, 'C',	3, A_Chase						, &States[S_CYBER_RUN+5]),
	S_NORMAL (CYBR, 'C',	3, A_Chase						, &States[S_CYBER_RUN+6]),
	S_NORMAL (CYBR, 'D',	3, A_Metal						, &States[S_CYBER_RUN+7]),
	S_NORMAL (CYBR, 'D',	3, A_Chase						, &States[S_CYBER_RUN+0]),

#define S_CYBER_ATK (S_CYBER_RUN+8)
	S_NORMAL (CYBR, 'E',	6, A_FaceTarget 				, &States[S_CYBER_ATK+1]),
	S_NORMAL (CYBR, 'F',   12, A_CyberAttack				, &States[S_CYBER_ATK+2]),
	S_NORMAL (CYBR, 'E',   12, A_FaceTarget 				, &States[S_CYBER_ATK+3]),
	S_NORMAL (CYBR, 'F',   12, A_CyberAttack				, &States[S_CYBER_ATK+4]),
	S_NORMAL (CYBR, 'E',   12, A_FaceTarget 				, &States[S_CYBER_ATK+5]),
	S_NORMAL (CYBR, 'F',   12, A_CyberAttack				, &States[S_CYBER_RUN+0]),

#define S_CYBER_PAIN (S_CYBER_ATK+6)
	S_NORMAL (CYBR, 'G',   10, A_Pain						, &States[S_CYBER_RUN+0]),

#define S_CYBER_DIE (S_CYBER_PAIN+1)
	S_NORMAL (CYBR, 'H',   10, NULL 						, &States[S_CYBER_DIE+1]),
	S_NORMAL (CYBR, 'I',   10, A_Scream 					, &States[S_CYBER_DIE+2]),
	S_NORMAL (CYBR, 'J',   10, NULL 						, &States[S_CYBER_DIE+3]),
	S_NORMAL (CYBR, 'K',   10, NULL 						, &States[S_CYBER_DIE+4]),
	S_NORMAL (CYBR, 'L',   10, NULL 						, &States[S_CYBER_DIE+5]),
	S_NORMAL (CYBR, 'M',   10, A_NoBlocking					, &States[S_CYBER_DIE+6]),
	S_NORMAL (CYBR, 'N',   10, NULL 						, &States[S_CYBER_DIE+7]),
	S_NORMAL (CYBR, 'O',   10, NULL 						, &States[S_CYBER_DIE+8]),
	S_NORMAL (CYBR, 'P',   30, NULL 						, &States[S_CYBER_DIE+9]),
	S_NORMAL (CYBR, 'P',   -1, A_BossDeath					, NULL)
};

void ACyberdemon::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 16;
	info->spawnid = 114;
	info->spawnstate = &States[S_CYBER_STND];
	info->spawnhealth = 4000;
	info->seestate = &States[S_CYBER_RUN];
	info->seesound = "cyber/sight";
	info->painstate = &States[S_CYBER_PAIN];
	info->painchance = 20;
	info->painsound = "cyber/pain";
	info->missilestate = &States[S_CYBER_ATK];
	info->deathstate = &States[S_CYBER_DIE];
	info->deathsound = "cyber/death";
	info->speed = 16;
	info->radius = 40 * FRACUNIT;
	info->height = 110 * FRACUNIT;
	info->mass = 1000;
	info->activesound = "cyber/active";
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL;
	info->flags2 = MF2_MCROSS|MF2_PASSMOBJ|MF2_PUSHWALL|MF2_BOSS;
	info->flags3 = MF3_NORADIUSDMG;
}

bool ACyberdemon::SuggestMissileAttack (fixed_t dist)
{
	return P_Random (pr_checkmissilerange) >= MIN (dist >> (FRACBITS + 1), 160);
}

void A_CyberAttack (AActor *self)
{
	AActor *rocket;

	if (!self->target)
		return;
				
	A_FaceTarget (self);
	rocket = P_SpawnMissile (self, self->target, RUNTIME_CLASS(ARocket));
}

void A_Hoof (AActor *self)
{
	S_Sound (self, CHAN_BODY, "cyber/hoof", 1, ATTN_IDLE);
	A_Chase (self);
}
