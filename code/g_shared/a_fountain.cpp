#include "actor.h"
#include "info.h"
#include "p_effect.h"

class AParticleFountain : public AActor
{
	DECLARE_STATELESS_ACTOR (AParticleFountain, AActor);
public:
	void PostBeginPlay ();
	void Activate (AActor *activator);
	void Deactivate (AActor *activator);
};

IMPLEMENT_DEF_SERIAL (AParticleFountain, AActor);

void AParticleFountain::SetDefaults (FActorInfo *info)
{
	ACTOR_DEFS_STATELESS;
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->flags2 = MF2_DONTDRAW;
	info->height = 0;
}

#define FOUNTAIN(color,ednum) \
	class A##color##ParticleFountain : public AParticleFountain { \
		DECLARE_STATELESS_ACTOR (A##color##ParticleFountain, AParticleFountain); }; \
	IMPLEMENT_DEF_SERIAL (A##color##ParticleFountain, AParticleFountain); \
	REGISTER_ACTOR (A##color##ParticleFountain, Any); \
	void A##color##ParticleFountain::SetDefaults (FActorInfo *info) { \
		INHERIT_DEFS_STATELESS; \
		info->doomednum = ednum; \
	}

FOUNTAIN (Red, 9027);
FOUNTAIN (Green, 9028);
FOUNTAIN (Blue, 9029);
FOUNTAIN (Yellow, 9030);
FOUNTAIN (Purple, 9031);
FOUNTAIN (Black, 9032);
FOUNTAIN (White, 9033);

void AParticleFountain::PostBeginPlay ()
{
	Super::PostBeginPlay ();
	if (!(mapflags & MTF_DORMANT))
		Activate (NULL);
}

void AParticleFountain::Activate (AActor *activator)
{
	Super::Activate (activator);
	effects &= ~FX_FOUNTAINMASK;
	effects |= (GetInfo (this)->doomednum - 9026) << FX_FOUNTAINSHIFT;
}

void AParticleFountain::Deactivate (AActor *activator)
{
	Super::Deactivate (activator);
	effects &= ~FX_FOUNTAINMASK;
}

