// Emacs style mode select	 -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//		Endianess handling, swapping 16bit and 32bit.
//
//-----------------------------------------------------------------------------


#ifndef __M_SWAP_H__
#define __M_SWAP_H__

// Endianess handling.
// WAD files are stored little endian.
#ifdef WORDS_BIGENDIAN

// Swap 16bit, that is, MSB and LSB byte.
// No masking with 0xFF should be necessary. 
inline short SHORT (short x)
{
	return (short)((((unsigned short)x)>>8) | (((unsigned short)x)<<8));
}

inline unsigned short SHORT (unsigned short x)
{
	return (unsigned short)((x>>8) | (x<<8));
}

// Swapping 32bit.
inline unsigned int LONG (unsigned int x)
{
	return (unsigned int)(
		(x>>24)
		| ((x>>8) & 0xff00)
		| ((x<<8) & 0xff0000)
		| (x<<24));
}

inline int LONG (int x)
{
	return (int)(
		(((unsigned int)x)>>24)
		| ((((unsigned int)x)>>8) & 0xff00)
		| ((((unsigned int)x)<<8) & 0xff0000)
		| (((unsigned int)x)<<24));
}

#define BESHORT(x)		(x)
#define BELONG(x)		(x)

#else

#define SHORT(x)		(x)
#define LONG(x) 		(x)

#if defined(_MSC_VER) && defined(USEASM)
#pragma warning (disable: 4035)

inline short BESHORT (short x)
{
	__asm mov ax, x
	__asm xchg al, ah
}

inline unsigned short BESHORT (unsigned short x)
{
	__asm mov ax, x
	__asm xchg al, ah
}

inline int BELONG (int x)
{
	__asm mov eax, x
	__asm bswap eax
}

inline unsigned int BELONG (unsigned int x)
{
	__asm mov eax, x
	__asm bswap eax
}

#pragma warning (default: 4035)

#else

inline short BESHORT (short x)
{
	return (short)((((unsigned short)x)>>8) | (((unsigned short)x)<<8));
}

inline unsigned short BESHORT (unsigned short x)
{
	return (unsigned short)((x>>8) | (x<<8));
}

inline unsigned int BELONG (unsigned int x)
{
	return (unsigned int)(
		(x>>24)
		| ((x>>8) & 0xff00)
		| ((x<<8) & 0xff0000)
		| (x<<24));
}

inline int BELONG (int x)
{
	return (int)(
		(((unsigned int)x)>>24)
		| ((((unsigned int)x)>>8) & 0xff00)
		| ((((unsigned int)x)<<8) & 0xff0000)
		| (((unsigned int)x)<<24));
}
#endif // USEASM

#endif // WORDS_BIGENDIAN

#endif // __M_SWAP_H__
