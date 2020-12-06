#include <stdio.h>
#include <string.h>

int main (int argc, char **argv)
{
	const char *endstr;
	char text[4096];
	FILE *fi, *fo;
	int i;
	char *pos;

	if (argc != 4)
	{
		return 1;
	}

	endstr = (argv[1][0] == 'a') ? "\r\n" : "\n";

	fi = fopen (argv[2], "rb");
	if (!fi) return 1;
	
	fo = fopen (argv[3], "wb");
	if (!fo) { fclose (fi); return 1; }

	while (fgets (text, 4096, fi))
	{
		pos = strchr (text, '\r');
		if (pos) *pos = 0;
		pos = strchr (text, '\n');
		if (pos) *pos = 0;

		fprintf (fo, "%s%s", text, endstr);
	}
	fclose (fo);
	fclose (fi);
}
