#include "actor.h"
#include "info.h"
#include "a_sharedglobal.h"

// args[0] = pitch
// args[1] = amount camera turns to either side of its initial position
//			 (in degrees)
// args[2] = octics to complete one cycle

class ASecurityCamera : public AActor
{
	DECLARE_STATELESS_ACTOR (ASecurityCamera, AActor);
public:
	void PostBeginPlay ();
	void RunThink ();
	angle_t AngleIncrements ();
protected:
	angle_t Center;
	angle_t Acc;
	angle_t Delta;
	angle_t Range;
};

IMPLEMENT_SERIAL (ASecurityCamera, AActor);
REGISTER_ACTOR (ASecurityCamera, Any);

void ASecurityCamera::SetDefaults (FActorInfo *info)
{
	ACTOR_DEFS_STATELESS;
	info->doomednum = 9025;
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->flags2 = MF2_DONTDRAW;
}

void ASecurityCamera::Serialize (FArchive &arc)
{
	Super::Serialize (arc);
	arc << Center << Acc << Delta << Range;
}

angle_t ASecurityCamera::AngleIncrements ()
{
	return ANGLE_1;
}

void ASecurityCamera::PostBeginPlay ()
{
	Super::PostBeginPlay ();
	Center = angle;
	if (args[2])
		Delta = ANGLE_MAX / (args[2] * TICRATE / 8);
	else
		Delta = 0;
	if (args[1])
		Delta /= 2;
	Acc = 0;
	pitch = (signed int)((char)args[0]) * ANGLE_1;
	if (pitch <= -ANGLE_90)
		pitch = -ANGLE_90 + ANGLE_1;
	else if (pitch >= ANGLE_90)
		pitch = ANGLE_90 - ANGLE_1;
	Range = args[1] * ANGLE_1;
}

void ASecurityCamera::RunThink ()
{
	Acc += Delta;
	if (Range)
		angle = Center + FixedMul (Range, finesine[Acc >> ANGLETOFINESHIFT]);
	else if (Delta)
		angle = Acc;
}
