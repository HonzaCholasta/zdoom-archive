#include "actor.h"
#include "info.h"
#include "p_local.h"
#include "p_spec.h"
#include "p_enemy.h"
#include "a_action.h"

void A_KeenDie (AActor *);

class ACommanderKeen : public AActor
{
	DECLARE_ACTOR (ACommanderKeen, AActor);
};

IMPLEMENT_DEF_SERIAL (ACommanderKeen, AActor);
REGISTER_ACTOR (ACommanderKeen, Doom);

FState ACommanderKeen::States[] =
{
#define S_KEENSTND 0
	S_NORMAL (KEEN, 'A',   -1, NULL 						, &States[S_KEENSTND]),

#define S_COMMKEEN (S_KEENSTND+1)
	S_NORMAL (KEEN, 'A',	6, NULL 						, &States[S_COMMKEEN+1]),
	S_NORMAL (KEEN, 'B',	6, NULL 						, &States[S_COMMKEEN+2]),
	S_NORMAL (KEEN, 'C',	6, A_Scream 					, &States[S_COMMKEEN+3]),
	S_NORMAL (KEEN, 'D',	6, NULL 						, &States[S_COMMKEEN+4]),
	S_NORMAL (KEEN, 'E',	6, NULL 						, &States[S_COMMKEEN+5]),
	S_NORMAL (KEEN, 'F',	6, NULL 						, &States[S_COMMKEEN+6]),
	S_NORMAL (KEEN, 'G',	6, NULL 						, &States[S_COMMKEEN+7]),
	S_NORMAL (KEEN, 'H',	6, NULL 						, &States[S_COMMKEEN+8]),
	S_NORMAL (KEEN, 'I',	6, NULL 						, &States[S_COMMKEEN+9]),
	S_NORMAL (KEEN, 'J',	6, NULL 						, &States[S_COMMKEEN+10]),
	S_NORMAL (KEEN, 'K',	6, A_KeenDie					, &States[S_COMMKEEN+11]),
	S_NORMAL (KEEN, 'L',   -1, NULL 						, NULL),

#define S_KEENPAIN (S_COMMKEEN+12)
	S_NORMAL (KEEN, 'M',	4, NULL 						, &States[S_KEENPAIN+1]),
	S_NORMAL (KEEN, 'M',	8, A_Pain						, &States[S_KEENSTND])
};

void ACommanderKeen::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 72;
	info->spawnstate = &States[S_KEENSTND];
	info->spawnhealth = 100;
	info->painstate = &States[S_KEENPAIN];
	info->painchance = 256;
	info->painsound = "keen/pain";
	info->deathstate = &States[S_COMMKEEN];
	info->deathsound = "keen/death";
	info->radius = 16 * FRACUNIT;
	info->height = 72 * FRACUNIT;
	info->mass = 10000000;
	info->flags = MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY|MF_SHOOTABLE|MF_COUNTKILL;
}

//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie (AActor *self)
{
	A_NoBlocking (self);
	
	// scan the remaining thinkers to see if all Keens are dead
	ACommanderKeen *other;
	TThinkerIterator<ACommanderKeen> iterator;

	while ( (other = iterator.Next ()) )
	{
		if (other != self && other->health > 0)
		{
			// other Keen not dead
			return;
		}
	}

	EV_DoDoor (DDoor::doorOpen, NULL, NULL, 666, 2*FRACUNIT, 0, NoKey);
}


