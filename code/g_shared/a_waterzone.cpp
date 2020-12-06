#include "info.h"
#include "r_defs.h"

class AWaterZone : public AActor
{
	DECLARE_STATELESS_ACTOR (AWaterZone, AActor);
public:
	void PostBeginPlay ();
};

IMPLEMENT_DEF_SERIAL (AWaterZone, AActor);
REGISTER_ACTOR (AWaterZone, Any);

void AWaterZone::SetDefaults (FActorInfo *info)
{
	ACTOR_DEFS_STATELESS;
	info->doomednum = 9045;
	info->flags = MF_NOBLOCKMAP|MF_NOSECTOR|MF_NOGRAVITY;
}

void AWaterZone::PostBeginPlay ()
{
	Super::PostBeginPlay ();
	subsector->sector->waterzone = 1;
	Destroy ();
}

