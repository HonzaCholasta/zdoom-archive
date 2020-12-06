#ifndef __P_ENEMY_H__
#define __P_ENEMY_H__

#include "r_defs.h"

enum dirtype_t
{
	DI_EAST,
	DI_NORTHEAST,
	DI_NORTH,
	DI_NORTHWEST,
	DI_WEST,
	DI_SOUTHWEST,
	DI_SOUTH,
	DI_SOUTHEAST,
	DI_NODIR,
	NUMDIRS
};

extern fixed_t xspeed[8], yspeed[8];

void P_RecursiveSound (sector_t *sec, int soundblocks);
void P_NoiseAlert (AActor *target, AActor *emmiter);
BOOL P_CheckMeleeRange (AActor *actor);
bool P_CheckMeleeRange2 (AActor *actor);
BOOL P_Move (AActor *actor);
BOOL P_TryWalk (AActor *actor);
void P_NewChaseDir (AActor *actor);
BOOL P_LookForPlayers (AActor *actor, BOOL allaround);
void P_DropItem (AActor *source, const TypeInfo *type, int special, int chance);
inline void P_DropItem (AActor *source, const char *type, int special, int chance)
{
	P_DropItem (source, TypeInfo::FindType (type), special, chance);
}

void A_Look (AActor *actor);
void A_Chase (AActor *actor);
void A_FaceTarget (AActor *actor);
void A_MonsterRail (AActor *actor);
void A_Scream (AActor *actor);
void A_XScream (AActor *actor);
void A_Pain (AActor *actor);
void A_Die (AActor *actor);
void A_Detonate (AActor *mo);
void A_Explode (AActor *thing);
void A_Mushroom (AActor *actor);
void A_BossDeath (AActor *actor);
void A_PlayerScream (AActor *mo);

bool A_RaiseMobj (AActor *);
bool A_SinkMobj (AActor *);

int P_Massacre ();

#endif //__P_ENEMY_H__
