#ifndef __A_SHAREDGLOBAL_H__
#define __A_SHAREDGLOBAL_H__

#include "dobject.h"
#include "info.h"

class AUnknown : public AActor
{
	DECLARE_ACTOR (AUnknown, AActor);
};

class AWayPoint : public AActor
{
	DECLARE_STATELESS_ACTOR (AWayPoint, AActor);
};

class ABlood : public AActor
{
	DECLARE_ACTOR (ABlood, AActor);
};

class AMapSpot : public AActor
{
	DECLARE_STATELESS_ACTOR (AMapSpot, AActor);
};

class AMapSpotGravity : public AMapSpot
{
	DECLARE_STATELESS_ACTOR (AMapSpotGravity, AMapSpot);
};

class AGibs : public AActor
{
	DECLARE_ACTOR (AGibs, AActor);
};

class AWaterSplashBase : public AActor
{
	DECLARE_ACTOR (AWaterSplashBase, AActor);
};

class AWaterSplash : public AActor
{
	DECLARE_ACTOR (AWaterSplash, AActor);
};

class ALavaSplash : public AActor
{
	DECLARE_ACTOR (ALavaSplash, AActor);
};

class ALavaSmoke : public AActor
{
	DECLARE_ACTOR (ALavaSmoke, AActor);
};

class ASludgeSplash : public AActor
{
	DECLARE_ACTOR (ASludgeSplash, AActor);
};

class ASludgeChunk : public AActor
{
	DECLARE_ACTOR (ASludgeChunk, AActor);
};

class AAmbientSound : public AActor
{
	DECLARE_STATELESS_ACTOR (AAmbientSound, AActor);
public:
	void PostBeginPlay ();
	void RunThink ();
	void Activate (AActor *activator);
	void Deactivate (AActor *activator);

protected:
	bool bActive;
private:
	void SetTicker (struct AmbientSound *ambient);
	int NextCheck;
};

class ATeleportFog : public AActor
{
	DECLARE_ACTOR (ATeleportFog, AActor);
public:
	void PostBeginPlay ();
};

class ATeleportDest : public AActor
{
	DECLARE_STATELESS_ACTOR (ATeleportDest, AActor);
};



#endif //__A_SHAREDGLOBAL_H__