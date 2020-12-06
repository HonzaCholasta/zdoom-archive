#include "actor.h"
#include "gi.h"
#include "m_random.h"
#include "s_sound.h"
#include "d_player.h"
#include "a_action.h"

void A_Pain (AActor *);
void A_PlayerScream (AActor *);
void A_XScream (AActor *);

class ADoomPlayer : public APlayerPawn
{
	DECLARE_ACTOR (ADoomPlayer, APlayerPawn);
public:
	void GiveDefaultInventory ();
	int GetMOD ();
};

IMPLEMENT_DEF_SERIAL (ADoomPlayer, APlayerPawn);
REGISTER_ACTOR (ADoomPlayer, Doom);

FState ADoomPlayer::States[] =
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
	S_NORMAL (PLAY, 'H',   10, NULL 						, &States[S_PLAY_DIE+1]),
	S_NORMAL (PLAY, 'I',   10, A_PlayerScream				, &States[S_PLAY_DIE+2]),
	S_NORMAL (PLAY, 'J',   10, A_NoBlocking					, &States[S_PLAY_DIE+3]),
	S_NORMAL (PLAY, 'K',   10, NULL 						, &States[S_PLAY_DIE+4]),
	S_NORMAL (PLAY, 'L',   10, NULL 						, &States[S_PLAY_DIE+5]),
	S_NORMAL (PLAY, 'M',   10, NULL 						, &States[S_PLAY_DIE+6]),
	S_NORMAL (PLAY, 'N',   -1, NULL 						, NULL),

#define S_PLAY_XDIE (S_PLAY_DIE+7)
	S_NORMAL (PLAY, 'O',	5, NULL 						, &States[S_PLAY_XDIE+1]),
	S_NORMAL (PLAY, 'P',	5, A_XScream					, &States[S_PLAY_XDIE+2]),
	S_NORMAL (PLAY, 'Q',	5, A_NoBlocking					, &States[S_PLAY_XDIE+3]),
	S_NORMAL (PLAY, 'R',	5, NULL 						, &States[S_PLAY_XDIE+4]),
	S_NORMAL (PLAY, 'S',	5, NULL 						, &States[S_PLAY_XDIE+5]),
	S_NORMAL (PLAY, 'T',	5, NULL 						, &States[S_PLAY_XDIE+6]),
	S_NORMAL (PLAY, 'U',	5, NULL 						, &States[S_PLAY_XDIE+7]),
	S_NORMAL (PLAY, 'V',	5, NULL 						, &States[S_PLAY_XDIE+8]),
	S_NORMAL (PLAY, 'W',   -1, NULL 						, NULL)
};

void ADoomPlayer::SetDefaults (FActorInfo *info)
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
	info->deathsound = "*death1";
	info->radius = 16 * FRACUNIT;
	info->height = 56 * FRACUNIT;
	info->mass = 100;
	info->flags = MF_SOLID|MF_SHOOTABLE|MF_DROPOFF|MF_PICKUP|MF_NOTDMATCH;
	info->flags2 = MF2_SLIDE|MF2_PASSMOBJ|MF2_PUSHWALL;
}

void ADoomPlayer::GiveDefaultInventory ()
{
	player->health = deh.StartHealth;		// [RH] Used to be MAXHEALTH
	player->readyweapon = player->pendingweapon = wp_pistol;
	player->weaponowned[wp_fist] = true;
	player->weaponowned[wp_pistol] = true;
	player->ammo[am_clip] = deh.StartBullets; // [RH] Used to be 50
}

int ADoomPlayer::GetMOD ()
{
	switch (player->readyweapon)
	{
	case wp_fist:			return MOD_FIST;
	case wp_pistol:			return MOD_PISTOL;
	case wp_shotgun:		return MOD_SHOTGUN;
	case wp_chaingun:		return MOD_CHAINGUN;
	case wp_supershotgun:	return MOD_SSHOTGUN;
	case wp_chainsaw:		return MOD_CHAINSAW;
	default:				return MOD_UNKNOWN;
	}
}

void A_PlayerScream (AActor *self)
{
	char nametemp[128];
	char *sound;

	if (gameinfo.gametype == GAME_Doom)
	{
		if (self->health < -50)
		{
			// IF THE PLAYER DIES LESS THAN -50% WITHOUT GIBBING
			sound = "*xdeath1";
		}
		else
		{
			// [RH] More variety in death sounds
			sprintf (nametemp, "*death%d", (P_Random (pr_playerscream)&3) + 1);
			sound = nametemp;
		}
	}
	else
	{	// Heretic
		// Handle the different player death screams
		if (self->special1 < 10)
		{ // Wimpy death sound
			sound = "*wimpydeath";
		}
		else if (self->health > -50)
		{ // Normal death sound
			sound = GetInfo (self)->deathsound;
		}
		else if (self->health > -100)
		{ // Crazy death sound
			sound = "*crazydeath";
		}
		else
		{ // Extreme death sound
			sound = "*gibbed";
		}
	}
	S_Sound (self, CHAN_VOICE, sound, 1, ATTN_NORM);
}

// Dead marine -------------------------------------------------------------

class ADeadMarine : public AActor
{
	DECLARE_STATELESS_ACTOR (ADeadMarine, AActor);
};

IMPLEMENT_DEF_SERIAL (ADeadMarine, AActor);
REGISTER_ACTOR (ADeadMarine, Doom);

void ADeadMarine::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 15;
	info->spawnstate = &ADoomPlayer::States[S_PLAY_DIE+6];
}

// Gibbed marine -----------------------------------------------------------

class AGibbedMarine : public AActor
{
	DECLARE_STATELESS_ACTOR (AGibbedMarine, AActor);
};

IMPLEMENT_DEF_SERIAL (AGibbedMarine, AActor);
REGISTER_ACTOR (AGibbedMarine, Doom);

void AGibbedMarine::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 10;
	info->spawnstate = &ADoomPlayer::States[S_PLAY_XDIE+8];
}

// Gibbed marine (extra copy) ----------------------------------------------

class AGibbedMarineExtra : public AGibbedMarine
{
	DECLARE_STATELESS_ACTOR (AGibbedMarineExtra, AGibbedMarine);
};

IMPLEMENT_DEF_SERIAL (AGibbedMarineExtra, AGibbedMarine);
REGISTER_ACTOR (AGibbedMarineExtra, Doom);

void AGibbedMarineExtra::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 12;
}
