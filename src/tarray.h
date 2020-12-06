#ifndef __TARRAY_H__
#define __TARRAY_H__

#include <stdlib.h>
#include "m_alloc.h"

template <class T>
class TArray
{
public:
	TArray ()
	{
		Most = 0;
		Count = 0;
		Array = NULL;
	}
	TArray (int max)
	{
		Most = max;
		Count = 0;
		Array = (T *)Malloc (sizeof(T)*max);
	}
	TArray (const TArray<T> &other)
	{
		DoCopy (other);
	}
	TArray<T> &operator= (const TArray<T> &other)
	{
		if (&other != this)
		{
			if (Array != NULL)
			{
				free (Array);
			}
			DoCopy (other);
		}
		return *this;
	}
	~TArray ()
	{
		if (Array)
			free (Array);
	}
	T &operator[] (size_t index) const
	{
		return Array[index];
	}
	size_t Push (const T &item)
	{
		if (Count >= Most)
		{
			Most = (Most >= 16) ? Most + Most / 2 : 16;
			Array = (T *)Realloc (Array, sizeof(T)*Most);
		}
		Array[Count] = item;
		return Count++;
	}
	bool Pop (T &item)
	{
		if (Count > 0)
		{
			item = Array[--Count];
			return true;
		}
		return false;
	}
	void Delete (int index)
	{
		if (index < Count-1)
			memmove (Array + index, Array + index + 1, (Count - index) * sizeof(T));
		else if (index < Count)
			Count--;
	}
	void ShrinkToFit ()
	{
		if (Most > Count)
		{
			Most = Count;
			if (Most == 0)
			{
				if (Array != NULL)
				{
					free (Array);
					Array = NULL;
				}
			}
			else
			{
				Array = (T *)Realloc (Array, sizeof(T)*Most);
			}
		}
	}
	// Grow Array to be large enough to hold amount more entries without
	// further growing.
	void Grow (size_t amount)
	{
		if (Count + amount > Most)
		{
			const size_t choicea = Count + amount;
			const size_t choiceb = Most + Most/2;
			Most = (choicea > choiceb ? choicea : choiceb);
			Array = (T *)Realloc (Array, sizeof(T)*Most);
		}
	}
	// Reserves amount entries at the end of the array, but does nothing
	// with them.
	size_t Reserve (size_t amount)
	{
		if (Count + amount > Most)
		{
			Grow (amount);
		}
		size_t place = Count;
		Count += amount;
		return place;
	}
	size_t Size () const
	{
		return Count;
	}
	size_t Max () const
	{
		return Most;
	}
	void Clear ()
	{
		Count = 0;
	}
private:
	T *Array;
	size_t Most;
	size_t Count;

	void DoCopy (const TArray<T> &other)
	{
		Most = Count = other.Count;
		if (Count != 0)
		{
			Array = (T *)Malloc (sizeof(T)*Most);
			for (size_t i = 0; i < Count; ++i)
			{
				Array[i] = other.Array[i];
			}
		}
		else
		{
			Array = NULL;
		}
	}
};

#endif //__TARRAY_H__
