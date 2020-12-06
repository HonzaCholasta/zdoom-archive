#ifndef __DARCHIVE_H__
#define __DARCHIVE_H__

#include <stdio.h>
#include "doomtype.h"
#include "dobject.h"

class DObject;

class FFile
{
public:
		enum EOpenMode
		{
			EReading,
			EWriting
		};

		enum ESeekPos
		{
			ESeekSet,
			ESeekRelative,
			ESeekEnd
		};

virtual	~FFile () {}

virtual	bool Open (const char *name, EOpenMode mode) = 0;
virtual	void Close () = 0;
virtual	void Flush () = 0;
virtual EOpenMode Mode () const = 0;
virtual bool IsPersistent () const = 0;
virtual bool IsOpen () const = 0;

virtual	FFile& Write (const void *, unsigned int) = 0;
virtual	FFile& Read (void *, unsigned int) = 0;

virtual	unsigned int Tell () const = 0;
virtual	FFile& Seek (int, ESeekPos) = 0;
inline	FFile& Seek (unsigned int i, ESeekPos p) { return Seek ((int)i, p); }
};

class FLZOFile : public FFile
{
public:
	FLZOFile ();
	FLZOFile (const char *name, EOpenMode mode, bool dontcompress = false);
	FLZOFile (FILE *file, EOpenMode mode, bool dontcompress = false);
	~FLZOFile ();

	bool Open (const char *name, EOpenMode mode);
	void Close ();
	void Flush ();
	EOpenMode Mode () const;
	bool IsPersistent () const { return true; }
	bool IsOpen () const;

	FFile &Write (const void *, unsigned int);
	FFile &Read (void *, unsigned int);
	unsigned int Tell () const;
	FFile &Seek (int, ESeekPos);

protected:
	unsigned int m_Pos;
	unsigned int m_BufferSize;
	unsigned int m_MaxBufferSize;
	unsigned char *m_Buffer;
	bool m_NoCompress;
	EOpenMode m_Mode;
	FILE *m_File;

	void Implode ();
	void Explode ();
	virtual bool FreeOnExplode () { return true; }

private:
	void BeEmpty ();
	void PostOpen ();
};

class FLZOMemFile : public FLZOFile
{
public:
	FLZOMemFile ();
	FLZOMemFile (FILE *file);	// Create for reading

	bool Open (const char *name, EOpenMode mode);	// Works for reading only
	bool Open (void *memblock);	// Open for reading only
	bool Open ();	// Open for writing only
	bool Reopen ();	// Re-opens imploded file for reading only
	void Close ();
	bool IsOpen () const;

	void Serialize (FArchive &arc);

protected:
	bool FreeOnExplode () { return !m_SourceFromMem; }

private:
	bool m_SourceFromMem;
	unsigned char *m_ImplodedBuffer;
};

class FArchive
{
public:
		FArchive (FFile &file);
		virtual ~FArchive ();

		inline bool IsLoading () const { return m_Loading; }
		inline bool IsStoring () const { return m_Storing; }
		inline bool IsPeristent () const { return m_Persistent; }
		
		void SetHubTravel () { m_HubTravel = true; }

		void Close ();

virtual	void Write (const void *mem, unsigned int len);
virtual void Read (void *mem, unsigned int len);

		void WriteCount (DWORD count);
		DWORD ReadCount ();

		FArchive& operator<< (BYTE c);
		FArchive& operator<< (WORD s);
		FArchive& operator<< (DWORD i);
		FArchive& operator<< (QWORD i);
		FArchive& operator<< (float f);
		FArchive& operator<< (double d);
		FArchive& operator<< (const char *str);
		FArchive& operator<< (DObject *obj);

inline	FArchive& operator<< (char c) { return operator<< ((BYTE)c); }
inline	FArchive& operator<< (SBYTE c) { return operator<< ((BYTE)c); }
inline	FArchive& operator<< (SWORD s) { return operator<< ((WORD)s); }
inline	FArchive& operator<< (SDWORD i) { return operator<< ((DWORD)i); }
inline	FArchive& operator<< (SQWORD i) { return operator<< ((QWORD)i); }
inline	FArchive& operator<< (const unsigned char *str) { return operator<< ((const char *)str); }
inline	FArchive& operator<< (const signed char *str) { return operator<< ((const char *)str); }
inline	FArchive& operator<< (bool b) { return operator<< ((BYTE)b); }
#ifndef ALPHA
inline	FArchive& operator<< (int i) { return operator<< ((SDWORD)i); }
inline	FArchive& operator<< (unsigned int i) { return operator<< ((DWORD)i); }
#endif

		FArchive& operator>> (BYTE &c);
		FArchive& operator>> (WORD &s);
		FArchive& operator>> (DWORD &i);
		FArchive& operator>> (QWORD &i);
		FArchive& operator>> (float &f);
		FArchive& operator>> (double &d);
		FArchive& operator>> (char *&str);
		FArchive& ReadObject (DObject *&obj, TypeInfo *wanttype);

inline	FArchive& operator>> (char &c) { return operator>> ((BYTE &)c); }
inline	FArchive& operator>> (SBYTE &c) { return operator>> ((BYTE &)c); }
inline	FArchive& operator>> (SWORD &s) { return operator>> ((WORD &)s); }
inline	FArchive& operator>> (SDWORD &i) { return operator>> ((DWORD &)i); }
inline	FArchive& operator>> (SQWORD &i) { return operator>> ((QWORD &)i); }
inline	FArchive& operator>> (unsigned char *&str) { return operator>> ((char *&)str); }
inline	FArchive& operator>> (signed char *&str) { return operator>> ((char *&)str); }
inline	FArchive& operator>> (bool &b) { return operator>> ((BYTE &)b); }
#ifndef ALPHA
inline	FArchive& operator>> (int &i) { return operator>> ((DWORD &)i); }
inline	FArchive& operator>> (unsigned int &i) { return operator>> ((DWORD &)i); }
#endif
inline  FArchive& operator>> (DObject* &object) { return ReadObject (object, RUNTIME_CLASS(DObject)); }

protected:
		enum { EObjectHashSize = 137 };

		DWORD FindObjectIndex (const DObject *obj) const;
		DWORD MapObject (const DObject *obj);
		DWORD WriteClass (const TypeInfo *info);
		const TypeInfo *ReadClass ();
		const TypeInfo *ReadClass (const TypeInfo *wanttype);
		const TypeInfo *ReadStoredClass (const TypeInfo *wanttype);
		DWORD HashObject (const DObject *obj) const;

		bool m_Persistent;		// meant for persistent storage (disk)?
		bool m_Loading;			// extracting objects?
		bool m_Storing;			// inserting objects?
		bool m_HubTravel;		// travelling inside a hub?
		FFile *m_File;			// unerlying file object
		DWORD m_ObjectCount;	// # of objects currently serialized
		DWORD m_MaxObjectCount;
		DWORD m_ClassCount;		// # of unique classes currently serialized
		struct TypeMap
		{
			const TypeInfo *toCurrent;	// maps archive type index to execution type index
			DWORD toArchive;		// maps execution type index to archive type index
		} *m_TypeMap;
		struct ObjectMap
		{
			const DObject *object;
			size_t hashNext;
		} *m_ObjectMap;
		size_t m_ObjectHash[EObjectHashSize];

private:
		FArchive (const FArchive &src) {}
		void operator= (const FArchive &src) {}
};

#endif //__DARCHIVE_H__
