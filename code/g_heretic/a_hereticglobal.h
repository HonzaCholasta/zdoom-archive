#ifndef __A_HERETICGLOBAL_H__
#define __A_HERETICGLOBAL_H__

#include "info.h"
#include "a_pickups.h"

class AHereticWeapon : public AWeapon
{
	DECLARE_STATELESS_ACTOR (AHereticWeapon, AWeapon);
protected:
	virtual void PlayPickupSound (AActor *toucher);
};

class APhoenixFX1 : public AActor
{
	DECLARE_ACTOR (APhoenixFX1, AActor);
public:
	int DoSpecialDamage (AActor *target, int damage);
};

class APhoenixPuff : public AActor
{
	DECLARE_ACTOR (APhoenixPuff, AActor);
};

class AMinotaur : public AActor
{
	DECLARE_ACTOR (AMinotaur, AActor);
public:
	void NoBlockingSet ();
	int DoSpecialDamage (AActor *target, int damage);
};

class ASorcerer2 : public AActor
{
	DECLARE_ACTOR (ASorcerer2, AActor);
public:
	void BeginPlay ();
	bool NewTarget (AActor *other);

	int NumBossSpots;
	AActor *FirstBossSpot;
};

class AWizard : public AActor
{
	DECLARE_ACTOR (AWizard, AActor);
public:
	void NoBlockingSet ();
	bool NewTarget (AActor *other);
};

void P_DSparilTeleport (AActor *actor);

class AStaffPuff : public AActor
{
	DECLARE_ACTOR (AStaffPuff, AActor);
public:
	void BeginPlay ();
};

#endif //__A_HERETICGLOBAL_H__