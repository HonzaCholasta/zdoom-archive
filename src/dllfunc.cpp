#include "dllfunc.h"
#include "doomtype.h"
#include "info.h"
#include "c_cvars.h"
#include "c_dispatch.h"
#include "actor.h"
#include "farchive.h"
#include "tables.h"

//
// Console functions
//

void Generic_CVarCallback (FStringCVar &self)
{
	if (self.m_DllCallback)
		self.m_DllCallback ((char*)((const char *)self));
}

void Con_RunString (char *cmd)
{
	AddCommandString (cmd);
}

void Con_CmdAdd (const char *name, CCmdRun callback)
{
	FConsoleCommand *cmd;

	cmd = new FConsoleCommand (name, callback);
}

void Con_CVarSet (const char *name, char *value, int flags, void(*callback)(char *) = NULL)
{
	FBaseCVar *var;
	FStringCVar *strvar;

	var = FindCVar (name, NULL);
	if (var == NULL)
	{
		strvar = new FStringCVar (name, NULL, flags, Generic_CVarCallback);
		strvar->m_DllCallback = callback;
	}

	if (var == NULL)
		strvar->CmdSet (value);
	else
		var->CmdSet (value);
}

bool Con_CVarUnSet (const char *name)
{
	FBaseCVar *var = FindCVar (name, NULL);
	if (var != NULL)
	{
		if (var->GetFlags() & CVAR_UNSETTABLE)
			delete var;
		else
			return false;
	}
	return true;
}

char *Con_CVarGet (const char *name)
{
	FBaseCVar *var, *prev;

	if ( (var = FindCVar (name, &prev)) )
	{
		UCVarValue val;
		val = var->GetGenericRep (CVAR_String);
		return val.String;
	}
	else
	{
		return NULL;
	}
}
