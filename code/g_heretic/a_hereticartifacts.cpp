#include "info.h"
#include "a_pickups.h"
#include "a_artifacts.h"
#include "hstrings.h"
#include "p_local.h"
#include "p_enemy.h"
#include "s_sound.h"

// Invisibility -------------------------------------------------------------

BASIC_ARTI (Invisibility, arti_invisibility, TXT_ARTIINVISIBILITY)
private:
	static bool ActivateArti (player_t *player, artitype_t arti)
	{
		return P_GivePower (player, pw_invisibility);
	}
};

ARTI_SETUP (Invisibility, Heretic);

FState AArtiInvisibility::States[] =
{
	S_BRIGHT (INVS, 'A',  350, NULL, &States[0])
};

void AArtiInvisibility::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 75;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
	info->translucency = HR_SHADOW;
	ArtiDispatch[arti_invisibility] = ActivateArti;
	ArtiPics[arti_invisibility] = "ARTIINVS";
}

// Tome of power ------------------------------------------------------------

BASIC_ARTI (TomeOfPower, arti_tomeofpower, TXT_ARTITOMEOFPOWER)
private:
	static bool ActivateArti (player_t *player, artitype_t arti)
	{
		if (player->morphTics)
		{ // Attempt to undo chicken
			if (P_UndoPlayerMorph (player) == false)
			{ // Failed
				P_DamageMobj (player->mo, NULL, NULL, 10000);
			}
			else
			{ // Succeeded
				player->morphTics = 0;
				S_Sound (player->mo, CHAN_WEAPON, "*evillaugh", 1, ATTN_IDLE);
			}
		}
		else
		{
			if (!P_GivePower (player, pw_weaponlevel2))
			{
				return false;
			}
			if (wpnlev1info[player->readyweapon]->readystate !=
				wpnlev2info[player->readyweapon]->readystate)
			{
				P_SetPsprite (player, ps_weapon,
					wpnlev2info[player->readyweapon]->readystate);
			}
		}
		return true;
	}
};

ARTI_SETUP (TomeOfPower, Heretic);

FState AArtiTomeOfPower::States[] =
{
	S_NORMAL (PWBK, 'A',  350, NULL, &States[0])
};

void AArtiTomeOfPower::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 86;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
	ArtiDispatch[arti_tomeofpower] = ActivateArti;
	ArtiPics[arti_tomeofpower] = "ARTIPWBK";
}

// Time bomb ----------------------------------------------------------------

class AActivatedTimeBomb : public AActor
{
	DECLARE_ACTOR (AActivatedTimeBomb, AActor);
public:
	void PreExplode ()
	{
		z += 32*FRACUNIT;
		translucency = OPAQUE;
	}
};

IMPLEMENT_DEF_SERIAL (AActivatedTimeBomb, AActor);
REGISTER_ACTOR (AActivatedTimeBomb, Heretic);

FState AActivatedTimeBomb::States[] =
{
	S_NORMAL (FBMB, 'A',   10, NULL 	, &States[1]),
	S_NORMAL (FBMB, 'B',   10, NULL 	, &States[2]),
	S_NORMAL (FBMB, 'C',   10, NULL 	, &States[3]),
	S_NORMAL (FBMB, 'D',   10, NULL 	, &States[4]),
	S_NORMAL (FBMB, 'E',	6, A_Scream , &States[5]),
	S_BRIGHT (XPL1, 'A',	4, A_Explode, &States[6]),
	S_BRIGHT (XPL1, 'B',	4, NULL 	, &States[7]),
	S_BRIGHT (XPL1, 'C',	4, NULL 	, &States[8]),
	S_BRIGHT (XPL1, 'D',	4, NULL 	, &States[9]),
	S_BRIGHT (XPL1, 'E',	4, NULL 	, &States[10]),
	S_BRIGHT (XPL1, 'F',	4, NULL 	, NULL)
};

void AActivatedTimeBomb::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->deathsound = "misc/timebomb";
	info->flags = MF_NOGRAVITY;
	info->translucency = HR_SHADOW;
}

BASIC_ARTI (TimeBomb, arti_firebomb, TXT_ARTIFIREBOMB)
private:
	static bool ActivateArti (player_t *player, artitype_t arti)
	{
		angle_t angle = player->mo->angle >> ANGLETOFINESHIFT;
		AActor *mo = Spawn<AActivatedTimeBomb> (
			player->mo->x + 24*finecosine[angle],
			player->mo->y + 24*finesine[angle],
			player->mo->z - player->mo->floorclip);
		mo->target = player->mo;
		return true;
	}
};

ARTI_SETUP (TimeBomb, Heretic);

FState AArtiTimeBomb::States[] =
{
	S_NORMAL (FBMB, 'E',  350, NULL, &States[0]),
};

void AArtiTimeBomb::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 34;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
	ArtiDispatch[arti_firebomb] = &ActivateArti;
	ArtiPics[arti_firebomb] = "ARTIFBMB";
}
