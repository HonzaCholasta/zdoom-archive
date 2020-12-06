#include "info.h"
#include "a_pickups.h"
#include "d_player.h"
#include "hstrings.h"
#include "p_local.h"

// Green key ------------------------------------------------------------

class AKeyGreen : public AKey
{
	DECLARE_ACTOR (AKeyGreen, AKey);
protected:
	virtual keytype_t GetKeyType ()
	{
		return key_green;
	}
	virtual const char *PickupMessage ()
	{
		return TXT_GOTGREENKEY;
	}
};

IMPLEMENT_DEF_SERIAL (AKeyGreen, AKey);
REGISTER_ACTOR (AKeyGreen, Heretic);

FState AKeyGreen::States[] =
{
	S_BRIGHT (AKYY, 'A',	3, NULL 				, &States[1]),
	S_BRIGHT (AKYY, 'B',	3, NULL 				, &States[2]),
	S_BRIGHT (AKYY, 'C',	3, NULL 				, &States[3]),
	S_BRIGHT (AKYY, 'D',	3, NULL 				, &States[4]),
	S_BRIGHT (AKYY, 'E',	3, NULL 				, &States[5]),
	S_BRIGHT (AKYY, 'F',	3, NULL 				, &States[6]),
	S_BRIGHT (AKYY, 'G',	3, NULL 				, &States[7]),
	S_BRIGHT (AKYY, 'H',	3, NULL 				, &States[8]),
	S_BRIGHT (AKYY, 'I',	3, NULL 				, &States[9]),
	S_BRIGHT (AKYY, 'J',	3, NULL 				, &States[0])
};

void AKeyGreen::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 73;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_NOTDMATCH;
}

// Blue key -----------------------------------------------------------------

class AKeyBlue : public AKey
{
	DECLARE_ACTOR (AKeyBlue, AKey);
protected:
	virtual keytype_t GetKeyType ()
	{
		return key_blue;
	}
	virtual const char *PickupMessage ()
	{
		return TXT_GOTBLUEKEY;
	}
};

IMPLEMENT_DEF_SERIAL (AKeyBlue, AKey);
REGISTER_ACTOR (AKeyBlue, Heretic);

FState AKeyBlue::States[] =
{
	S_BRIGHT (BKYY, 'A',	3, NULL 				, &States[1]),
	S_BRIGHT (BKYY, 'B',	3, NULL 				, &States[2]),
	S_BRIGHT (BKYY, 'C',	3, NULL 				, &States[3]),
	S_BRIGHT (BKYY, 'D',	3, NULL 				, &States[4]),
	S_BRIGHT (BKYY, 'E',	3, NULL 				, &States[5]),
	S_BRIGHT (BKYY, 'F',	3, NULL 				, &States[6]),
	S_BRIGHT (BKYY, 'G',	3, NULL 				, &States[7]),
	S_BRIGHT (BKYY, 'H',	3, NULL 				, &States[8]),
	S_BRIGHT (BKYY, 'I',	3, NULL 				, &States[9]),
	S_BRIGHT (BKYY, 'J',	3, NULL 				, &States[0])
};

void AKeyBlue::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 79;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_NOTDMATCH;
}

// Yellow key ---------------------------------------------------------------

class AKeyYellow : public AKey
{
	DECLARE_ACTOR (AKeyYellow, AKey);
protected:
	virtual keytype_t GetKeyType ()
	{
		return key_yellow;
	}
	virtual const char *PickupMessage ()
	{
		return TXT_GOTYELLOWKEY;
	}
};

IMPLEMENT_DEF_SERIAL (AKeyYellow, AKey);
REGISTER_ACTOR (AKeyYellow, Heretic);

FState AKeyYellow::States[] =
{
	S_BRIGHT (CKYY, 'A',	3, NULL 				, &States[1]),
	S_BRIGHT (CKYY, 'B',	3, NULL 				, &States[2]),
	S_BRIGHT (CKYY, 'C',	3, NULL 				, &States[3]),
	S_BRIGHT (CKYY, 'D',	3, NULL 				, &States[4]),
	S_BRIGHT (CKYY, 'E',	3, NULL 				, &States[5]),
	S_BRIGHT (CKYY, 'F',	3, NULL 				, &States[6]),
	S_BRIGHT (CKYY, 'G',	3, NULL 				, &States[7]),
	S_BRIGHT (CKYY, 'H',	3, NULL 				, &States[8]),
	S_BRIGHT (CKYY, 'I',	3, NULL 				, &States[0])
};

void AKeyYellow::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->doomednum = 80;
	info->spawnstate = &States[0];
	info->flags = MF_SPECIAL|MF_NOTDMATCH;
}

// --- Key gizmos -----------------------------------------------------------

void A_InitKeyGizmo (AActor *);

class AKeyGizmo : public AActor
{
	DECLARE_ACTOR (AKeyGizmo, AActor);
public:
	virtual int GetFloatState () { return 0; }
};

IMPLEMENT_DEF_SERIAL (AKeyGizmo, AActor);
REGISTER_ACTOR (AKeyGizmo, Heretic);

FState AKeyGizmo::States[] =
{
	S_NORMAL (KGZ1, 'A',	1, NULL 				, &States[1]),
	S_NORMAL (KGZ1, 'A',	1, A_InitKeyGizmo		, &States[2]),
	S_NORMAL (KGZ1, 'A',   -1, NULL 				, NULL)
};

void AKeyGizmo::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->radius = 16 * FRACUNIT;
	info->height= 50 * FRACUNIT;
	info->flags = MF_SOLID;
}

class AKeyGizmoFloat : public AActor
{
	DECLARE_ACTOR (AKeyGizmoFloat, AActor);
};

IMPLEMENT_DEF_SERIAL (AKeyGizmoFloat, AActor);
REGISTER_ACTOR (AKeyGizmoFloat, Heretic);

FState AKeyGizmoFloat::States[] =
{
#define S_KGZ_BLUEFLOAT 0
	S_BRIGHT (KGZB, 'A',   -1, NULL 				, NULL),

#define S_KGZ_GREENFLOAT (S_KGZ_BLUEFLOAT+1)
	S_BRIGHT (KGZG, 'A',   -1, NULL 				, NULL),

#define S_KGZ_YELLOWFLOAT (S_KGZ_GREENFLOAT+1)
	S_BRIGHT (KGZY, 'A',   -1, NULL 				, NULL)
};

void AKeyGizmoFloat::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS;
	info->spawnstate = &States[0];
	info->radius = 16 * FRACUNIT;
	info->height = 16 * FRACUNIT;
	info->flags = MF_SOLID|MF_NOGRAVITY;
}

// Blue gizmo ---------------------------------------------------------------

class AKeyGizmoBlue : public AKeyGizmo
{
	DECLARE_STATELESS_ACTOR (AKeyGizmoBlue, AKeyGizmo);
public:
	int GetFloatState () { return S_KGZ_BLUEFLOAT; }
};

IMPLEMENT_DEF_SERIAL (AKeyGizmoBlue, AKeyGizmo);
REGISTER_ACTOR (AKeyGizmoBlue, Heretic);

void AKeyGizmoBlue::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 94;
}

// Green gizmo --------------------------------------------------------------

class AKeyGizmoGreen : public AKeyGizmo
{
	DECLARE_STATELESS_ACTOR (AKeyGizmoGreen, AKeyGizmo);
public:
	int GetFloatState () { return S_KGZ_GREENFLOAT; }
};

IMPLEMENT_DEF_SERIAL (AKeyGizmoGreen, AKeyGizmo);
REGISTER_ACTOR (AKeyGizmoGreen, Heretic);

void AKeyGizmoGreen::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 95;
}

// Yellow gizmo -------------------------------------------------------------

class AKeyGizmoYellow : public AKeyGizmo
{
	DECLARE_STATELESS_ACTOR (AKeyGizmoYellow, AKeyGizmo);
public:
	int GetFloatState () { return S_KGZ_YELLOWFLOAT; }
};

IMPLEMENT_DEF_SERIAL (AKeyGizmoYellow, AKeyGizmo);
REGISTER_ACTOR (AKeyGizmoYellow, Heretic);

void AKeyGizmoYellow::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 96;
}

//----------------------------------------------------------------------------
//
// PROC A_InitKeyGizmo
//
//----------------------------------------------------------------------------

void A_InitKeyGizmo (AActor *gizmo)
{
	AActor *floater;

	floater = Spawn<AKeyGizmoFloat> (gizmo->x, gizmo->y, gizmo->z+60*FRACUNIT);
	floater->SetState (&AKeyGizmoFloat::
		States[static_cast<AKeyGizmo *>(gizmo)->GetFloatState ()]);
}
