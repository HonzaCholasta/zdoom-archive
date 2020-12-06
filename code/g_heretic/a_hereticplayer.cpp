#include "actor.h"
#include "gi.h"
#include "m_random.h"
#include "s_sound.h"
#include "d_player.h"
#include "a_action.h"
#include "p_local.h"

void A_Pain (AActor *);
void A_PlayerScream (AActor *);
void A_SkullPop (AActor *);
void A_CheckSkullFloor (AActor *);
void A_CheckSkullDone (AActor *);
void A_CheckBurnGone (AActor *);
void A_FlameSnd (AActor *);

// The player ---------------------------------------------------------------

class AHereticPlayer : public APlayerPawn
{
	DECLARE_ACTOR (AHereticPlayer, APlayerPawn);
public:
	void GiveDefaultInventory ();
};

IMPLEMENT_DEF_SERIAL (AHereticPlayer, APlayerPawn);
REGISTER_ACTOR (AHereticPlayer, Heretic);

FState AHereticPlayer::States[] =
{
#define S_PLAY 0
	S_NORMAL (PLAY, 'A',   -1, NULL 						, NULL),

#define S_PLAY_RUN (S_PLAY+1)
	S_NORMAL (PLAY, 'A',	4, NULL 						, &States[S_PLAY_RUN+1]),
	S_NORMAL (PLAY, 'B',	4, NULL 						, &States[S_PLAY_RUN+2]),
	S_NORMAL (PLAY, 'C',	4, NULL 						, &States[S_PLAY_RUN+3]),
	S_NORMAL (PLAY, 'D',	4, NULL 						, &States[S_PLAY_RUN+0]),

#define S_PLAY_ATK (S_PLAY_RUN+4)
	S_NORMAL (PLAY, 'E',   12, NULL 						, &States[S_PLAY]),
	S_BRIGHT (PLAY, 'F',	6, NULL 						, &States[S_PLAY_ATK+0]),

#define S_PLAY_PAIN (S_PLAY_ATK+2)
	S_NORMAL (PLAY, 'G',	4, NULL 						, &States[S_PLAY_PAIN+1]),
	S_NORMAL (PLAY, 'G',	4, A_Pain						, &States[S_PLAY]),

#define S_PLAY_DIE (S_PLAY_PAIN+2)
	S_NORMAL (PLAY, 'H',	6, NULL 						, &States[S_PLAY_DIE+1]),
	S_NORMAL (PLAY, 'I',	6, A_PlayerScream				, &States[S_PLAY_DIE+2]),
	S_NORMAL (PLAY, 'J',	6, NULL 						, &States[S_PLAY_DIE+3]),
	S_NORMAL (PLAY, 'K',	6, NULL 						, &States[S_PLAY_DIE+4]),
	S_NORMAL (PLAY, 'L',	6, A_NoBlocking 				, &States[S_PLAY_DIE+5]),
	S_NORMAL (PLAY, 'M',	6, NULL 						, &States[S_PLAY_DIE+6]),
	S_NORMAL (PLAY, 'N',	6, NULL 						, &States[S_PLAY_DIE+7]),
	S_NORMAL (PLAY, 'O',	6, NULL 						, &States[S_PLAY_DIE+8]),
	S_NORMAL (PLAY, 'P',   -1, NULL							, NULL),

#define S_PLAY_XDIE (S_PLAY_DIE+9)
	S_NORMAL (PLAY, 'Q',	5, A_PlayerScream 				, &States[S_PLAY_XDIE+1]),
	S_NORMAL (PLAY, 'R',	0, A_NoBlocking					, &States[S_PLAY_XDIE+2]),
	S_NORMAL (PLAY, 'R',	5, A_SkullPop					, &States[S_PLAY_XDIE+3]),
	S_NORMAL (PLAY, 'S',	5, NULL			 				, &States[S_PLAY_XDIE+4]),
	S_NORMAL (PLAY, 'T',	5, NULL 						, &States[S_PLAY_XDIE+5]),
	S_NORMAL (PLAY, 'U',	5, NULL 						, &States[S_PLAY_XDIE+6]),
	S_NORMAL (PLAY, 'V',	5, NULL 						, &States[S_PLAY_XDIE+7]),
	S_NORMAL (PLAY, 'W',	5, NULL 						, &States[S_PLAY_XDIE+8]),
	S_NORMAL (PLAY, 'X',	5, NULL 						, &States[S_PLAY_XDIE+9]),
	S_NORMAL (PLAY, 'Y',   -1, NULL							, NULL),

#define S_PLAY_FDTH (S_PLAY_XDIE+10)
	S_BRIGHT (FDTH, 'A',	5, A_FlameSnd					, &States[S_PLAY_FDTH+1]),
	S_BRIGHT (FDTH, 'B',	4, NULL 						, &States[S_PLAY_FDTH+2]),
	S_BRIGHT (FDTH, 'C',	5, NULL 						, &States[S_PLAY_FDTH+3]),
	S_BRIGHT (FDTH, 'D',	4, A_PlayerScream				, &States[S_PLAY_FDTH+4]),
	S_BRIGHT (FDTH, 'E',	5, NULL 						, &States[S_PLAY_FDTH+5]),
	S_BRIGHT (FDTH, 'F',	4, NULL 						, &States[S_PLAY_FDTH+6]),
	S_BRIGHT (FDTH, 'G',	5, A_FlameSnd					, &States[S_PLAY_FDTH+7]),
	S_BRIGHT (FDTH, 'H',	4, NULL 						, &States[S_PLAY_FDTH+8]),
	S_BRIGHT (FDTH, 'I',	5, NULL 						, &States[S_PLAY_FDTH+9]),
	S_BRIGHT (FDTH, 'J',	4, NULL 						, &States[S_PLAY_FDTH+10]),
	S_BRIGHT (FDTH, 'K',	5, NULL 						, &States[S_PLAY_FDTH+11]),
	S_BRIGHT (FDTH, 'L',	4, NULL 						, &States[S_PLAY_FDTH+12]),
	S_BRIGHT (FDTH, 'M',	5, NULL 						, &States[S_PLAY_FDTH+13]),
	S_BRIGHT (FDTH, 'N',	4, NULL 						, &States[S_PLAY_FDTH+14]),
	S_BRIGHT (FDTH, 'O',	5, A_NoBlocking 				, &States[S_PLAY_FDTH+15]),
	S_BRIGHT (FDTH, 'P',	4, NULL 						, &States[S_PLAY_FDTH+16]),
	S_BRIGHT (FDTH, 'Q',	5, NULL 						, &States[S_PLAY_FDTH+17]),
	S_BRIGHT (FDTH, 'R',	4, NULL 						, &States[S_PLAY_FDTH+18]),
	S_NORMAL (ACLO, 'E',   35, A_CheckBurnGone				, &States[S_PLAY_FDTH+18]),
	S_NORMAL (ACLO, 'E',	8, NULL 						, NULL)
};

void AHereticPlayer::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_PLAY];
	info->spawnhealth = 100;
	info->seestate = &States[S_PLAY_RUN];
	info->painstate = &States[S_PLAY_PAIN];
	info->painchance = 255;
	info->painsound = "*pain100_1";
	info->missilestate = &States[S_PLAY_ATK];
	info->deathstate = &States[S_PLAY_DIE];
	info->xdeathstate = &States[S_PLAY_XDIE];
	info->bdeathstate = &States[S_PLAY_FDTH];
	info->deathsound = "*death1";
	info->radius = 16 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 100;
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_DROPOFF|MF_PICKUP|MF_NOTDMATCH;
	info->flags2 = MF2_WINDTHRUST|MF2_FLOORCLIP|MF2_SLIDE|MF2_PASSMOBJ|MF2_PUSHWALL|MF2_TELESTOMP;
}

void AHereticPlayer::GiveDefaultInventory ()
{
	player->health = GetInfo (this)->spawnhealth;
	player->readyweapon = player->pendingweapon = wp_goldwand;
	player->weaponowned[wp_staff] = true;
	player->weaponowned[wp_goldwand] = true;
	player->ammo[am_goldwand] = 50;
}

// The player's skull -------------------------------------------------------

class ABloodySkull : public APlayerPawn
{
	DECLARE_ACTOR (ABloodySkull, APlayerPawn);
};

IMPLEMENT_DEF_SERIAL (ABloodySkull, APlayerPawn);
REGISTER_ACTOR (ABloodySkull, Heretic);

FState ABloodySkull::States[] =
{
#define S_BLOODYSKULL 0
	S_NORMAL (BSKL, 'A',	5, A_CheckSkullFloor			, &States[S_BLOODYSKULL+1]),
	S_NORMAL (BSKL, 'B',	5, A_CheckSkullFloor			, &States[S_BLOODYSKULL+2]),
	S_NORMAL (BSKL, 'C',	5, A_CheckSkullFloor			, &States[S_BLOODYSKULL+3]),
	S_NORMAL (BSKL, 'D',	5, A_CheckSkullFloor			, &States[S_BLOODYSKULL+4]),
	S_NORMAL (BSKL, 'E',	5, A_CheckSkullFloor			, &States[S_BLOODYSKULL+0]),

#define S_BLOODYSKULLX1 (S_BLOODYSKULL+5)
	S_NORMAL (BSKL, 'F',   16, A_CheckSkullDone 			, &States[S_BLOODYSKULLX1]),

#define S_BLOODYSKULLX2 (S_BLOODYSKULLX1+1)
	S_NORMAL (BSKL, 'F', 1050, NULL 						, NULL)
};

void ABloodySkull::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[S_BLOODYSKULL];
	info->radius = 4*FRACUNIT;
	info->height = 4*FRACUNIT;
	info->flags = MF_NOBLOCKMAP|MF_DROPOFF;
	info->flags2 = MF2_LOGRAV|MF2_CANNOTPUSH;
	info->flags3 = MF3_SKYEXPLODE;
}

//----------------------------------------------------------------------------
//
// PROC A_SkullPop
//
//----------------------------------------------------------------------------

void A_SkullPop (AActor *actor)
{
	APlayerPawn *mo;
	player_t *player;

	actor->flags &= ~MF_SOLID;
	mo = Spawn<ABloodySkull> (actor->x, actor->y, actor->z + 48*FRACUNIT);
	//mo->target = actor;
	mo->momx = PS_Random() << 9;
	mo->momy = PS_Random() << 9;
	mo->momz = 2*FRACUNIT + (P_Random() << 6);
	// Attach player mobj to bloody skull
	player = actor->player;
	actor->player = NULL;
	mo->player = player;
	mo->health = actor->health;
	mo->angle = actor->angle;
	player->mo = mo;
	if (player->camera == actor)
	{
		player->camera = mo;
	}
	player->damagecount = 32;
}

//----------------------------------------------------------------------------
//
// PROC A_CheckSkullFloor
//
//----------------------------------------------------------------------------

void A_CheckSkullFloor (AActor *actor)
{
	if (actor->z <= actor->floorz)
	{
		actor->SetState (&AHereticPlayer::States[S_BLOODYSKULLX1]);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_CheckSkullDone
//
//----------------------------------------------------------------------------

void A_CheckSkullDone (AActor *actor)
{
	if (actor->special2 == 666)
	{
		actor->SetState (&AHereticPlayer::States[S_BLOODYSKULLX2]);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_CheckBurnGone
//
//----------------------------------------------------------------------------

void A_CheckBurnGone (AActor *actor)
{
	if (actor->special2 == 666)
	{
		actor->SetState (&AHereticPlayer::States[S_PLAY_FDTH+19]);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_FlameSnd
//
//----------------------------------------------------------------------------

void A_FlameSnd (AActor *actor)
{
	if (actor->player != NULL)
	{
		actor->player->cheats |= CF_NOSKIN;
	}
	S_Sound (actor, CHAN_BODY, "misc/burn", 1, ATTN_NORM);	// Burn sound
}

