#include "info.h"
#include "a_pickups.h"
#include "a_artifacts.h"
#include "hstrings.h"
#include "p_local.h"
#include "gi.h"
#include "s_sound.h"
#include "m_random.h"

// Teleport (self) ----------------------------------------------------------

BASIC_ARTI (Teleport, arti_teleport, TXT_ARTITELEPORT)
private:
	static bool ActivateArti (player_t *player, artitype_t arti)
	{
		fixed_t destX;
		fixed_t destY;
		angle_t destAngle;

		if (deathmatch.value)
		{
			int selections = deathmatchstarts.Size ();
			int i = P_Random() % selections;
			destX = deathmatchstarts[i].x << FRACBITS;
			destY = deathmatchstarts[i].y << FRACBITS;
			destAngle = ANG45 * (deathmatchstarts[i].angle/45);
		}
		else
		{
			destX = playerstarts[player - players].x << FRACBITS;
			destY = playerstarts[player - players].y << FRACBITS;
			destAngle = ANG45 * (playerstarts[player - players].angle/45);
		}
		P_Teleport (player->mo, destX, destY, ONFLOORZ, destAngle, true);
		if (gameinfo.gametype == GAME_Hexen && player->morphTics)
		{ // Teleporting away will undo any morph effects (pig)
			P_UndoPlayerMorph (player);
		}
		if (gameinfo.gametype == GAME_Heretic)
		{ // Full volume laugh
			S_Sound (player->mo, CHAN_VOICE, "*evillaugh", 1, ATTN_NONE);
		}
		return true;
	}
};

ARTI_SETUP (Teleport, Raven);

FState AArtiTeleport::States[] =
{
	S_NORMAL (ATLP, 'A',	4, NULL, &States[1]),
	S_NORMAL (ATLP, 'B',	4, NULL, &States[2]),
	S_NORMAL (ATLP, 'C',	4, NULL, &States[3]),
	S_NORMAL (ATLP, 'B',	4, NULL, &States[0])
};

void AArtiTeleport::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 36;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_COUNTITEM;
	info->flags2 = MF2_FLOATBOB;
	ArtiDispatch[arti_teleport] = ActivateArti;
	ArtiPics[arti_teleport] = "ARTIATLP";
}
