#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "doomdef.h"
#include "doomstat.h"
#include "d_netinf.h"
#include "d_net.h"
#include "d_proto.h"
#include "v_palett.h"
#include "v_video.h"
#include "i_system.h"
#include "r_draw.h"
#include "r_state.h"
#include "st_stuff.h"

extern BOOL st_firsttime;

cvar_t *autoaim;
cvar_t *name;
cvar_t *color;
cvar_t *skin;
cvar_t *team;
cvar_t *gender;

int D_GenderToInt (const char *gender)
{
	if (!stricmp (gender, "female"))
		return GENDER_FEMALE;
	else if (!stricmp (gender, "neuter"))
		return GENDER_NEUTER;
	else
		return GENDER_MALE;
}

void D_SetupUserInfo (void)
{
	int i;
	userinfo_t *coninfo = &players[consoleplayer].userinfo;

	for (i = 0; i < MAXPLAYERS; i++)
		memset (&players[i].userinfo, 0, sizeof(userinfo_t));

	strncpy (coninfo->netname, name->string, MAXPLAYERNAME);
	strncpy (coninfo->team, team->string, MAXPLAYERNAME);
	coninfo->aimdist = (fixed_t)(autoaim->value * 16384.0);
	coninfo->color = V_GetColorFromString (NULL, color->string);
	coninfo->skin = R_FindSkin (skin->string);
	coninfo->gender = D_GenderToInt (gender->string);
	R_BuildPlayerTranslation (consoleplayer, coninfo->color);
}

void D_UserInfoChanged (cvar_t *cvar)
{
	byte foo[256];

	if (cvar == autoaim) {
		if (cvar->value < 0.0f) {
			SetCVarFloat (cvar, 0.0f);
			return;
		} else if (cvar->value > 5000.0f) {
			SetCVarFloat (cvar, 5000.0f);
			return;
		}
	}

	if (4 + strlen (cvar->name) + strlen (cvar->string) > 256)
		I_Error ("User info descriptor too big");

	sprintf (foo, "\\%s\\%s", cvar->name, cvar->string);

	Net_WriteByte (DEM_UINFCHANGED);
	Net_WriteString (foo);
}

static BOOL SetServerVar (char *name, char *value)
{
	cvar_t *dummy;
	cvar_t *var = FindCVar (name, &dummy);

	if (var) {
		unsigned oldflags = var->flags;

		var->flags &= ~(CVAR_SERVERINFO|CVAR_LATCH);
		SetCVar (var, value);
		var->flags = oldflags;
		return true;
	}
	return false;
}

void D_SendServerInfoChange (const cvar_t *cvar, const char *value)
{
	byte foo[256];

	if (4 + strlen (cvar->name) + strlen (value) > 256)
		I_Error ("Server info descriptor too big");

	sprintf (foo, "\\%s\\%s", cvar->name, value);

	Net_WriteByte (DEM_SINFCHANGED);
	Net_WriteString (foo);
}

void D_DoServerInfoChange (byte **stream)
{
	byte *ptr = *stream;
	byte *breakpt;
	byte *value;

	if (*ptr++ != '\\')
		return;

	while ( (breakpt = strchr (ptr, '\\')) ) {
		*breakpt = 0;
		value = breakpt + 1;
		if ( (breakpt = strchr (value, '\\')) )
			*breakpt = 0;

		if (SetServerVar (ptr, value) && netgame)
			Printf ("%s changed to %s\n", ptr, value);

		*(value - 1) = '\\';
		if (breakpt) {
			*breakpt = '\\';
			ptr = breakpt + 1;
		} else {
			break;
		}
	}

	*stream += strlen (*stream) + 1;
}

void D_WriteUserInfoStrings (int i, byte **stream)
{
	if (i >= MAXPLAYERS) {
		WriteByte (0, stream);
	} else {
		userinfo_t *info = &players[i].userinfo;

		sprintf (*stream, "\\name\\%s\\autoaim\\%g\\color\\%x %x %x\\skin\\%s\\team\\%s\\gender\\%s",
						  info->netname,
						  (double)info->aimdist / 16384.0,
						  RPART(info->color), GPART(info->color), BPART(info->color),
						  skins[info->skin].name, info->team,
						  info->gender == GENDER_FEMALE ? "female" :
								info->gender == GENDER_NEUTER ? "neuter" : "male"
						  );
	}

	*stream += strlen (*stream) + 1;
}

void D_ReadUserInfoStrings (int i, byte **stream, BOOL update)
{
	userinfo_t *info = &players[i].userinfo;
	byte *ptr = *stream;
	byte *breakpt;
	byte *value;

	if (*ptr++ != '\\')
		return;

	if (i < MAXPLAYERS) {
		while ( (breakpt = strchr (ptr, '\\')) ) {
			*breakpt = 0;
			value = breakpt + 1;
			if ( (breakpt = strchr (value, '\\')) )
				*breakpt = 0;

			if (!stricmp (ptr, "autoaim")) {
				info->aimdist = (fixed_t)(atof (value) * 16384.0);
			} else if (!stricmp (ptr, "name")) {
				char oldname[MAXPLAYERNAME+1];

				strncpy (oldname, info->netname, MAXPLAYERNAME);
				oldname[MAXPLAYERNAME] = 0;
				strncpy (info->netname, value, MAXPLAYERNAME);
				info->netname[MAXPLAYERNAME] = 0;

				if (update)
					Printf ("%s is now known as %s\n", oldname, info->netname);
			} else if (!stricmp (ptr, "team")) {
				strncpy (info->team, value, MAXPLAYERNAME);
				info->team[MAXPLAYERNAME] = 0;
				if (update) {
					if (info->team[0])
						Printf ("%s joined the %s team\n", info->netname, info->team);
					else
						Printf ("%s is not on a team\n", info->netname);
				}
			} else if (!stricmp (ptr, "color")) {
				info->color = V_GetColorFromString (NULL, value);
				R_BuildPlayerTranslation (i, info->color);
				st_firsttime = true;
			} else if (!stricmp (ptr, "skin")) {
				info->skin = R_FindSkin (value);
				if (players[i].mo)
					players[i].mo->sprite = skins[info->skin].sprite;
				ST_loadGraphics ();
				st_firsttime = true;
			} else if (!stricmp (ptr, "gender")) {
				info->gender = D_GenderToInt (value);
			}

			*(value - 1) = '\\';
			if (breakpt) {
				*breakpt = '\\';
				ptr = breakpt + 1;
			} else {
				break;
			}
		}
	}

	*stream += strlen (*stream) + 1;
}
