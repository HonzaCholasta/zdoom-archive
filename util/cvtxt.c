#include <stdio.h>
#include <string.h>

int main (int argc, char **argv)
{
	char text[4096];
	FILE *fi, *fo;
	int i;
	char *pos;
	
	for (i = 1; i < argc; ++i)
	{
		pos = strrchr (argv[i], '.');
		if (argc > 2 && pos &&
			strcmp (pos, ".h") &&
			strcmp (pos, ".cpp") &&
			strcmp (pos, ".nas") &&
			strcmp (pos, ".c") &&
			strcmp (pos, ".txt"))
		{
			printf ("skipping %s\n", argv[i]);
			continue;
		}
		
		fi = fopen (argv[i], "r");
		if (!fi) continue;
		
		fo = fopen ("iamadummy", "w");
		if (!fo) { fclose (fi); continue; }
		
		printf ("%s\n", argv[i]);

		while (fgets (text, 4096, fi))
		{
			pos = strchr (text, '\r');
			if (pos) *pos = 0;
			pos = strchr (text, '\n');
			if (pos) *pos = 0;
			
			fprintf (fo, "%s\n", text);
		}
		fclose (fo);
		fclose (fi);
		remove (argv[i]);
		rename ("iamadummy", argv[i]);
	}
}