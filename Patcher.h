// --- Author	: Moreau Cyril - Spl3en
// --- File		: Patcher.h
// --- Date		: 2013-04-12-16.31.07
// --- Version	: 1.0

#ifndef Patcher_H_INCLUDED
#define Patcher_H_INCLUDED


// ---------- Includes ------------
#include <stdlib.h>
#include "../Win32Tools/Win32Tools.h"
#include "../MemProc/MemProc.h"
#include "../BbQueue/BbQueue.h"

// ---------- Defines -------------


// ------ Class declaration -------
typedef
struct _PatchItem
{
	Ztring *z;
	int offset;

} 	PatchItem;


typedef
struct _Patch
{
	DWORD addr;
	char *description;

	unsigned char *signature;
	unsigned char *patch;
	char *mask;
	int size; // = strlen(mask)

	bool activated;
	MemProc *ctxt;

	BbQueue *patch_items;

}	Patch;


// --------- Constructors ---------

Patch *
patch_new (char *description, MemProc *mp, DWORD addr, unsigned char *code, unsigned char *signature, unsigned char *patch, char *mask);

BbQueue *
patch_list_get ();


// ----------- Methods ------------

void
patch_set_activated (Patch *p, bool activated);


void
patch_debug (Patch *p);



// --------- Destructors ----------

void
patch_list_set (BbQueue *patchlist, bool enabled);

void
patch_free (Patch *patch);

inline void
patch_list_free (BbQueue *pl);






#endif // Patcher_INCLUDED
