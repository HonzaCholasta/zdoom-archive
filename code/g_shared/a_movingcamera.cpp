#include "actor.h"
#include "info.h"
#include "p_local.h"
#include "vectors.h"

/*
== CameraNode: node along a camera's path
==
== args[0] = pitch
== args[1] = time (in octics) to get here from previous node
== args[2] = time (in octics) to stay here before moving to next node
== args[3] = tid of next node
== args[4] = high byte of next node's tid
*/

class ACameraNode : public AActor
{
	DECLARE_STATELESS_ACTOR (ACameraNode, AActor);
public:
	void PostBeginPlay ();
	void RunThink () {}		// Nodes do no thinking
	ACameraNode *ScanForLoop ();

	ACameraNode *Next;
};

IMPLEMENT_POINTY_SERIAL (ACameraNode, AActor)
 DECLARE_POINTER (Next)
END_POINTERS

REGISTER_ACTOR (ACameraNode, Any);

void ACameraNode::SetDefaults (FActorInfo *info)
{
	ACTOR_DEFS_STATELESS;
	info->doomednum = 9070;
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->flags2 = MF2_DONTDRAW;
}

void ACameraNode::Serialize (FArchive &arc)
{
	Super::Serialize (arc);
	arc << Next;
}

void ACameraNode::PostBeginPlay ()
{
	TActorIterator<ACameraNode> iterator (args[3] + 256 * args[4]);
	Next = iterator.Next ();

	if (Next == this)	// Paths must be at least two nodes
		Next = iterator.Next ();

	if (Next == NULL)
		Printf (PRINT_HIGH, "Can't find target for camera node %d\n", tid);

	pitch = (signed int)((char)args[0]) * ANGLE_1;
	if (pitch <= -ANGLE_90)
		pitch = -ANGLE_90 + ANGLE_1;
	else if (pitch >= ANGLE_90)
		pitch = ANGLE_90 - ANGLE_1;
}

// Return the node (if any) where a path loops, relative to this one.
ACameraNode *ACameraNode::ScanForLoop ()
{
	ACameraNode *node = this;
	while (node->Next && !(node->Next->flags & MF_JUSTHIT))
	{
		node = node->Next;
		node->flags |= MF_JUSTHIT;
	}
	ACameraNode *looppt = node->Next ? node : NULL;
	node = this;
	while (node && (node->flags & MF_JUSTHIT))
	{
		node->flags &= ~MF_JUSTHIT;
		node = node->Next;
	}
	return looppt;
}

/*
== CameraPathMover: something that follows a camera path
==		Base class for some moving cameras
==
== args[0] = tid of first node in path
== args[1] = high byte of first node's tid
== args[2] = follow a linear path (rather than curved)
*/

class ACameraPathMover : public AActor
{
	DECLARE_STATELESS_ACTOR (ACameraPathMover, AActor);
public:
	void PostBeginPlay ();
	void RunThink ();
	void Activate (AActor *activator);
	void Deactivate (AActor *activator);
protected:
	float Splerp (float p1, float p2, float p3, float p4);
	float Lerp (float p1, float p2);

	bool bActive, bJustStepped;
	ACameraNode *PrevNode, *CurrNode;
	float Time;		// Runs from 0.0 to 1.0 between CurrNode and CurrNode->Next
	int HoldTime;
};

IMPLEMENT_POINTY_SERIAL (ACameraPathMover, AActor)
 DECLARE_POINTER (PrevNode)
 DECLARE_POINTER (CurrNode)
END_POINTERS

REGISTER_ACTOR (ACameraPathMover, Any);

void ACameraPathMover::SetDefaults (FActorInfo *info)
{
	ACTOR_DEFS_STATELESS;
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->flags2 = MF2_DONTDRAW;
}

void ACameraPathMover::Serialize (FArchive &arc)
{
	Super::Serialize (arc);
	arc << bActive << bJustStepped << PrevNode << CurrNode << Time << HoldTime;
}

// Interpolate between p2 and p3 along a Catmull-Rom spline
// http://research.microsoft.com/~hollasch/cgindex/curves/catmull-rom.html
float ACameraPathMover::Splerp (float p1, float p2, float p3, float p4)
{
	float t = Time;
	float res = 2*p2;
	res += (p3 - p1) * Time;
	t *= Time;
	res += (2*p1 - 5*p2 + 4*p3 - p4) * t;
	t *= Time;
	res += (3*p2 - 3*p3 + p4 - p1) * t;
	return 0.5f * res;
}

// Linearly interpolate between p1 and p2
float ACameraPathMover::Lerp (float p1, float p2)
{
	return p2 * Time + p1 * (1.f - Time);
}

void ACameraPathMover::PostBeginPlay ()
{
	bActive = false;
}

void ACameraPathMover::Deactivate (AActor *activator)
{
	bActive = false;
}

void ACameraPathMover::Activate (AActor *activator)
{
	// Find first node of path
	TActorIterator<ACameraNode> iterator (args[0] + 256 * args[1]);
	CurrNode = iterator.Next ();

	if (CurrNode == NULL)
	{
		Printf (PRINT_HIGH, "CameraPathMover %d: Can't find target %d\n",
			tid, args[0] + 256 * args[1]);
		return;
	}

	// Verify the path has enough nodes
	if (args[2])
	{	// linear path; need 2 nodes
		if (CurrNode->Next == NULL)
		{
			Printf (PRINT_HIGH, "CameraPathMover %d: Path needs at least 2 nodes\n", tid);
			return;
		}
		PrevNode = NULL;
	}
	else
	{	// spline path; need 4 nodes
		if (CurrNode->Next == NULL ||
			CurrNode->Next->Next == NULL ||
			CurrNode->Next->Next->Next == NULL)
		{
			Printf (PRINT_HIGH, "CameraPathMover %d: Path needs at least 4 nodes\n", tid);
			return;
		}
		// If the first node is in a loop, we can start there.
		// Otherwise, we need to start at the second node in the path.
		PrevNode = CurrNode->ScanForLoop ();
		if (PrevNode == NULL || PrevNode->Next != CurrNode)
		{
			PrevNode = CurrNode;
			CurrNode = CurrNode->Next;
		}
	}

	Time = 0.f;
	HoldTime = 0;
	bJustStepped = true;
	bActive = true;
}

void ACameraPathMover::RunThink ()
{
	if (!bActive)
		return;

	if (bJustStepped)
	{
		bJustStepped = false;
		if (CurrNode->args[2])
		{
			HoldTime = CurrNode->args[2] * TICRATE / 8;
			x = CurrNode->x;
			y = CurrNode->y;
			z = CurrNode->z;
		}
	}

	if (HoldTime > gametic)
		return;

	if (CurrNode->Next == NULL)
	{
		bActive = false;
		return;
	}

	if (args[2])
	{	// linear
		x = FLOAT2FIXED(Lerp (FIXED2FLOAT(CurrNode->x), FIXED2FLOAT(CurrNode->Next->x)));
		y = FLOAT2FIXED(Lerp (FIXED2FLOAT(CurrNode->y), FIXED2FLOAT(CurrNode->Next->y)));
		z = FLOAT2FIXED(Lerp (FIXED2FLOAT(CurrNode->z), FIXED2FLOAT(CurrNode->Next->z)));
	}
	else
	{	// spline
		x = FLOAT2FIXED(Splerp (FIXED2FLOAT(PrevNode->x), FIXED2FLOAT(CurrNode->x),
								FIXED2FLOAT(CurrNode->Next->x), FIXED2FLOAT(CurrNode->Next->Next->x)));
		x = FLOAT2FIXED(Splerp (FIXED2FLOAT(PrevNode->y), FIXED2FLOAT(CurrNode->y),
								FIXED2FLOAT(CurrNode->Next->y), FIXED2FLOAT(CurrNode->Next->Next->y)));
		x = FLOAT2FIXED(Splerp (FIXED2FLOAT(PrevNode->z), FIXED2FLOAT(CurrNode->z),
								FIXED2FLOAT(CurrNode->Next->z), FIXED2FLOAT(CurrNode->Next->Next->z)));
	}

	Time += 8.f / ((float)CurrNode->args[1] * (float)TICRATE);
	if (Time > 1.f)
	{
		Time -= 1.f;
		bJustStepped = true;
		PrevNode = CurrNode;
		CurrNode = CurrNode->Next;
	}
}

/*
== CameraTarget: Something for a camera to aim at
==
== Shares args with CameraPathMover
*/

class ACameraTarget : public ACameraPathMover
{
	DECLARE_STATELESS_ACTOR (ACameraTarget, ACameraPathMover);
public:
	void PostBeginPlay ();
};

IMPLEMENT_DEF_SERIAL (ACameraTarget, ACameraPathMover);
REGISTER_ACTOR (ACameraTarget, Any);

void ACameraTarget::SetDefaults (FActorInfo *info)
{
	INHERIT_DEFS_STATELESS;
	info->doomednum = 9071;
	info->flags = MF_NOBLOCKMAP|MF_NOGRAVITY;
	info->flags2 = MF2_DONTDRAW;
}

void ACameraTarget::PostBeginPlay ()
{
	Super::PostBeginPlay ();
	UnlinkFromWorld ();
	flags2 = 0;
	flags &= ~MF_NOSECTOR;
	LinkToWorld ();
	//this->SetState (S_TROO_STND);
}
