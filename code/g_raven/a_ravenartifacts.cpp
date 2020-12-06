#include "info.h"
#include "a_pickups.h"
#include "a_artifacts.h"
#include "hstrings.h"
#include "p_local.h"
//#include "p_enemy.h"
#include "s_sound.h"

// Health -------------------------------------------------------------------

BASIC_ARTI (Health, arti_health, TXT_ARTIHEALTH)
private:
	static bool ActivateArti (player_t *player, artitype_t arti)
	{
		return P_GiveBody (player, 25);
	}
};

ARTI_SETUP (Health, Raven);

FState AArtiHealth::States[] =
{
	S_NORMAL (PTN2, 'A',	4, NULL, &States[1]),
	S_NORMAL (PTN2, 'B',	4, NULL, &States[2]),
	S_NORMAL (PTN2, 'C',	4, NULL, &States[0])
};

void AArtiHealth::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 82;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
	ArtiDispatch[arti_health] = ActivateArti;
	ArtiPics[arti_health] = "ARTIPTN2";
}

// Super health -------------------------------------------------------------

BASIC_ARTI (SuperHealth, arti_superhealth, TXT_ARTISUPERHEALTH)
private:
	static bool ActivateArti (player_t *player, artitype_t arti)
	{
		return P_GiveBody (player, GetInfo (player->mo)->spawnhealth);
	}
};

ARTI_SETUP (SuperHealth, Raven);

FState AArtiSuperHealth::States[] =
{
	S_NORMAL (SPHL, 'A',  350, NULL, &States[0])
};

void AArtiSuperHealth::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 32;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
	ArtiDispatch[arti_superhealth] = ActivateArti;
	ArtiPics[arti_superhealth] = "ARTISPHL";
}

// Flight -------------------------------------------------------------------

BASIC_ARTI (Fly, arti_fly, TXT_ARTIFLY)
private:
	static bool ActivateArti (player_t *player, artitype_t arti)
	{
		if (P_GivePower (player, pw_flight))
		{
			if (player->mo->momz <= -35*FRACUNIT)
			{ // stop falling scream
				S_StopSound (player->mo, CHAN_VOICE);
			}
			return true;
		}
		return false;
	}
};

ARTI_SETUP (Fly, Raven);

FState AArtiFly::States[] =
{
	S_NORMAL (SOAR, 'A',	5, NULL, &States[1]),
	S_NORMAL (SOAR, 'B',	5, NULL, &States[2]),
	S_NORMAL (SOAR, 'C',	5, NULL, &States[3]),
	S_NORMAL (SOAR, 'B',	5, NULL, &States[0])
};

void AArtiFly::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 83;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
	ArtiDispatch[arti_fly] = ActivateArti;
	ArtiPics[arti_fly] = "ARTISOAR";
}

// Invulnerability ----------------------------------------------------------

BASIC_ARTI (Invulnerability, arti_invulnerability, TXT_ARTIINVULNERABILITY)
private:
	static bool ActivateArti (player_t *player, artitype_t arti)
	{
		return P_GivePower (player, pw_invulnerability);
	}
};

ARTI_SETUP (Invulnerability, Raven);

FState AArtiInvulnerability::States[] =
{
	S_NORMAL (INVU, 'A',	3, NULL, &States[1]),
	S_NORMAL (INVU, 'B',	3, NULL, &States[2]),
	S_NORMAL (INVU, 'C',	3, NULL, &States[3]),
	S_NORMAL (INVU, 'D',	3, NULL, &States[0])
};

void AArtiInvulnerability::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 84;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
	ArtiDispatch[arti_invulnerability] = ActivateArti;
	ArtiPics[arti_invulnerability] = "ARTIINVU";
}

// Torch --------------------------------------------------------------------

BASIC_ARTI (Torch, arti_torch, TXT_ARTITORCH)
private:
	static bool ActivateArti (player_t *player, artitype_t arti)
	{
		return P_GivePower (player, pw_infrared);
	}
};

ARTI_SETUP (Torch, Raven);

FState AArtiTorch::States[] =
{
	S_BRIGHT (TRCH, 'A',	3, NULL, &States[1]),
	S_BRIGHT (TRCH, 'B',	3, NULL, &States[2]),
	S_BRIGHT (TRCH, 'C',	3, NULL, &States[0])
};

void AArtiTorch::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 33;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
	ArtiDispatch[arti_torch] = ActivateArti;
	ArtiPics[arti_torch] = "ARTITRCH";
}
