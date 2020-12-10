#include "stdafx.h"
#include "doomdll.h"

//
// Console
//

FCommandLine::FCommandLine (const char *commandline)
{
	cmd = commandline;
	_argc = -1;
	_argv = NULL;
}

FCommandLine::~FCommandLine ()
{
	if (_argv != NULL)
	{
		gi.Z_Free (_argv);
	}
}

int FCommandLine::argc ()
{
	if (_argc == -1)
	{
		argsize = gi.Con_ParseCommandLine (cmd, &_argc, NULL);
	}
	return _argc;
}

char *FCommandLine::operator[] (int i)
{
	if (_argv == NULL)
	{
		int count = argc();
		_argv = (char **)gi.Z_Malloc (count*sizeof(char *) + argsize, PU_STATIC, 0);
		_argv[0] = (char *)_argv + count*sizeof(char *);
		gi.Con_ParseCommandLine (cmd, NULL, _argv);
	}
	return _argv[i];
}
