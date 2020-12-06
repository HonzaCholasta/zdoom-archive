/* Extract the .c, .cpp, and .nas files out of a .dsp
 * file and print them to stdout them in a format useful
 * for make. The specific output is tailored for use with
 * ZDoom, but it should be easy enough to modify it for
 * more general use.
 *
 * Randy Heit in 2001.
 * This file is public domain.
 */

#include <stdio.h>
#include <string.h>

char Files[1024][256];

int main (int argc, char **argv)
{
	char text[256], *ext;
	FILE *f;
	int numf, i;
	
	if (argc != 2 || !(f = fopen (argv[1], "r")))
		return -1;
	
	for (numf = 0; (fgets (text, 256, f)) != NULL; numf < 1024)
	{
		if (strncmp (text, "SOURCE=.\\", 9) == 0)
		{
			strcpy (Files[numf], text+9);
			strtok (Files[numf], "\r\n");
			
			for (i = strlen(Files[numf])-1; i >= 0; --i)
				if (Files[numf][i] == '\\')
					Files[numf][i] = '/';
				else
					Files[numf][i] = tolower (Files[numf][i]);
			
			ext = strrchr (Files[numf], '.');
			if (ext == NULL)
				continue;
			++ext;
			if (strcmp (ext, "c") != 0 &&
				strcmp (ext, "cpp") != 0 &&
				strcmp (ext, "nas") != 0)
				continue;
			if (strncmp (Files[numf], "win32/", 6) == 0)
				continue;
			
			for (i = 0; i < numf; ++i)
			{
				if (strcmp (Files[i], Files[numf]) == 0)
					break;
			}
			if (i != numf)
				break;
			++numf;
		}
	}

	fclose (f);

	printf ("DSPOBJS = \\\n\t$(INTDIR)/autostart.o");
	for (i = 0; i < numf; ++i)
	{
		strcpy (text, Files[i]);
		ext = strrchr (text, '.');
		ext[1] = 'o';
		ext[2] = 0;
		if (strncmp (text, "auto", 4) == 0)
			continue;
		printf (" \\\n\t$(INTDIR)/%s", text);
	}
	printf (" \\\n\t$(INTDIR)/autozend.o\n");

	printf ("\nDSPSOURCES =");
	for (i = 0; i < numf; ++i)
		printf (" \\\n\t$(src_dir)/%s", Files[i]);

	printf ("\n\nDSPINTDIRS = $(INTDIR)");
	text[0] = 0;
	for (i = 0; i < numf; ++i)
	{
		ext = strchr (Files[i], '/');
		if (ext == NULL) continue;
		if (strncmp (text, Files[i], ext - Files[i]) == 0) continue;
		strncpy (text, Files[i], ext - Files[i]);
		text[ext - Files[i]] = 0;
		printf (" \\\n\t$(INTDIR)/%s", text);
	}

	printf ("\n\nDSPDEPSDIRS = $(DEPSDIR)");
	text[0] = 0;
	for (i = 0; i < numf; ++i)
	{
		ext = strchr (Files[i], '/');
		if (ext == NULL) continue;
		if (strncmp (text, Files[i], ext - Files[i]) == 0) continue;
		strncpy (text, Files[i], ext - Files[i]);
		text[ext - Files[i]] = 0;
		printf (" \\\n\t$(DEPSDIR)/%s", text);
	}

	printf ("\n");
	return 0;
}
