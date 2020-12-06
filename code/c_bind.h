#ifndef __C_BINDINGS_H__
#define __C_BINDINGS_H__

#include "doomtype.h"
#include "d_event.h"
#include <stdio.h>

class FConfigFile;

BOOL C_DoKey (event_t *ev);
void C_ArchiveBindings (FConfigFile *f, bool dodouble);

// Stuff used by the customize controls menu
int  C_GetKeysForCommand (char *cmd, int *first, int *second);
void C_NameKeys (char *str, int first, int second);
void C_UnbindACommand (char *str);
void C_ChangeBinding (const char *str, int newone);
void C_DoBind (const char *key, const char *bind, bool doublebind);

// Returns string bound to given key (NULL if none)
char *C_GetBinding (int key);

#endif //__C_BINDINGS_H__