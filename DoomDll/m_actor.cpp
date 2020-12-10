#include "doomdll.h"
#include "m_actor.h"
#include <memory.h>

AActor::~AActor ()
{
}

AActor::AActor () throw()
{
	memset (&x, 0, (byte *)&this[1] - (byte *)&x);
}

AActor::AActor (const AActor &other) throw()
{
	memcpy (&x, &other.x, (byte *)&this[1] - (byte *)&x);
}

AActor &AActor::operator= (const AActor &other)
{
	memcpy (&x, &other.x, (byte *)&this[1] - (byte *)&x);
	return *this;
}
