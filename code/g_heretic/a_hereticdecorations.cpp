#include "actor.h"
#include "info.h"

/********** Decorations ***********/

#define _DECCOMMON(cls,ednum,rad,hi,ns) \
	class cls : public AActor { DECLARE_STATELESS_ACTOR (cls, AActor); static FState States[ns]; }; \
	IMPLEMENT_DEF_SERIAL (cls, AActor); \
	REGISTER_ACTOR (cls, Heretic); \
	void cls::SetDefaults (FActorInfo *info) { \
		INHERIT_DEFS; \
		info->doomednum = ednum; \
		info->spawnstate = &States[0]; \
		info->radius = rad * FRACUNIT; \
		info->height = hi * FRACUNIT;

#define _DECSTARTSTATES(cls,ns) \
	FState cls::States[ns] =

#define DEC(cls,ednum,rad,hi,ns) \
	_DECCOMMON(cls,ednum,rad,hi,ns) info->flags = MF_SOLID; } _DECSTARTSTATES(cls,ns)

#define DECNSOLID(cls,ednum,rad,hi,ns) \
	_DECCOMMON(cls,ednum,rad,hi,ns) } _DECSTARTSTATES(cls,ns)

#define DECHANG(cls,ednum,rad,hi,ns) \
	_DECCOMMON(cls,ednum,rad,hi,ns) info->flags = MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY; } _DECSTARTSTATES(cls,ns)

#define DECHANGNS(cls,ednum,rad,hi,ns) \
	_DECCOMMON(cls,ednum,rad,hi,ns) info->flags = MF_SPAWNCEILING|MF_NOGRAVITY; } _DECSTARTSTATES(cls,ns)

#define DECFLOATNS(cls,ednum,rad,hi,ns) \
	_DECCOMMON(cls,ednum,rad,hi,ns) info->flags = MF_NOGRAVITY; } _DECSTARTSTATES(cls,ns)

#define DECNBLOCK(cls,ednum,rad,hi,ns) \
	_DECCOMMON(cls,ednum,rad,hi,ns) info->flags = MF_NOBLOCKMAP; } _DECSTARTSTATES(cls,ns)

#define SUBCLASS_NS(cls,super,ednum,rad,hi) \
	class cls : public super { DECLARE_STATELESS_ACTOR (cls, super); }; \
	IMPLEMENT_DEF_SERIAL (cls, super); \
	REGISTER_ACTOR (cls, Doom); \
	void cls::SetDefaults (FActorInfo *info) { \
		INHERIT_DEFS_STATELESS; \
		info->doomednum= ednum; \
		info->radius = rad * FRACUNIT; \
		info->height = rad * FRACUNIT; \
		info->flags &= ~MF_SOLID; }

DECHANGNS (ASkullHang70, 17, 20, 70, 1)
{
	S_NORMAL (SKH1, 'A', -1, NULL, NULL)
};

DECHANGNS (ASkullHang60, 24, 20, 60, 1)
{
	S_NORMAL (SKH2, 'A', -1, NULL, NULL)
};

DECHANGNS (ASkullHang45, 25, 20, 45, 1)
{
	S_NORMAL (SKH3, 'A', -1, NULL, NULL)
};

DECHANGNS (ASkullHang35, 26, 20, 35, 1)
{
	S_NORMAL (SKH4, 'A', -1, NULL, NULL)
};

DECHANGNS (AChandelier, 28, 20, 60, 3)
{
	S_NORMAL (CHDL, 'A',  4, NULL, &States[1]),
	S_NORMAL (CHDL, 'B',  4, NULL, &States[2]),
	S_NORMAL (CHDL, 'C',  4, NULL, &States[0])
};

DEC (ASerpentTorch, 27, 12, 54, 3)
{
	S_NORMAL (SRTC, 'A',  4, NULL, &States[1]),
	S_NORMAL (SRTC, 'B',  4, NULL, &States[2]),
	S_NORMAL (SRTC, 'C',  4, NULL, &States[0])
};

DEC (ASmallPillar, 29, 16, 34, 1)
{
	S_NORMAL (SMPL, 'A', -1, NULL, NULL)
};

DEC (AStalagmiteSmall, 37, 8, 32, 1)
{
	S_NORMAL (STGS, 'A', -1, NULL, NULL)
};

DEC (AStalagmiteLarge, 38, 12, 64, 1)
{
	S_NORMAL (STGL, 'A', -1, NULL, NULL)
};

DECHANG (AStalactiteSmall, 39, 8, 36, 1)
{
	S_NORMAL (STCS, 'A', -1, NULL, NULL)
};

DECHANG (AStalactiteLarge, 40, 12, 68, 1)
{
	S_NORMAL (STCL, 'A', -1, NULL, NULL)
};


DEC (AFireBrazier, 76, 16, 44, 8)
{
	S_BRIGHT (KFR1, 'A',  3, NULL, &States[1]),
	S_BRIGHT (KFR1, 'B',  3, NULL, &States[2]),
	S_BRIGHT (KFR1, 'C',  3, NULL, &States[3]),
	S_BRIGHT (KFR1, 'D',  3, NULL, &States[4]),
	S_BRIGHT (KFR1, 'E',  3, NULL, &States[5]),
	S_BRIGHT (KFR1, 'F',  3, NULL, &States[6]),
	S_BRIGHT (KFR1, 'G',  3, NULL, &States[7]),
	S_BRIGHT (KFR1, 'H',  3, NULL, &States[0])
};

DEC (ABarrel, 44, 12, 32, 1)
{
	S_NORMAL (BARL, 'A', -1, NULL, NULL)
};

DEC (ABrownPillar, 47, 14, 128, 1)
{
	S_NORMAL (BRPL, 'A', -1, NULL, NULL)
};

DECHANGNS (AMoss1, 48, 20, 23, 1)
{
	S_NORMAL (MOS1, 'A', -1, NULL, NULL)
};

DECHANGNS (AMoss2, 49, 20, 27, 1)
{
	S_NORMAL (MOS2, 'A', -1, NULL, NULL)
};

DECFLOATNS (AWallTorch, 50, 20, 16, 3)
{
	S_BRIGHT (WTRH, 'A',  6, NULL, &States[1]),
	S_BRIGHT (WTRH, 'B',  6, NULL, &States[2]),
	S_BRIGHT (WTRH, 'C',  6, NULL, &States[0])
};

DECHANG (AHangingCorpse, 51, 8, 104, 1)
{
	S_NORMAL (HCOR, 'A', -1, NULL, NULL)
};
