#include "stdafx.h"
#include "DoomDll.h"

dllexport_t globals;
dllimport_t gi;

//
// Test callbacks
//

void CVar_dllcvar (char *self)
{
	gi.Printf (PRINT_HIGH, "dllcvar changed to %s\n", self);
}

void CCmd_dllcmd (FCommandLine &argv, AActor *m_Instigator, int key)
{
	if (argv.argc() != 3)
		gi.Printf (PRINT_HIGH, "using: dllcmd <arg1> <arg2>\n");
	else
		gi.Printf (PRINT_HIGH, "dllcmd is ok (args %s and %s)\n", argv[1], argv[2]);
}

//
// Main & export
//

void Init (void)
{
	gi.Con_CVarSet ("dllcvar", "", CVAR_AUTO | CVAR_UNSETTABLE | CVAR_ARCHIVE, CVar_dllcvar);
	gi.Con_CmdAdd ("dllcmd", CCmd_dllcmd);
}

dllexport_t *GetDllAPI (dllimport_t *import)
{
	gi = *import;

	globals.apiversion = DLL_API_VERSION;

	globals.Init = Init;

	return &globals;
}
