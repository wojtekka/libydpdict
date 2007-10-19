#include <stdio.h>
#include <ydpdict/ydpdict.h>

int main(int argc, char **argv)
{
	ydpdict_t dict;
	uint32_t i;
	char *foo;
	FILE *f;

	if (ydpdict_open(&dict, "/usr/local/share/ydpdict/dict100.dat", "/usr/local/share/ydpdict/dict100.idx", YDPDICT_ENCODING_UTF8) == -1) {
		perror("ydpdict_open");
		return 1;
	}

	if (argc < 2)
		i = 0;
	else if (!(i = atoi(argv[1])))
		i = ydpdict_find(&dict, argv[1]);

	printf("ydpdict_find = %u\n", i);
	
	if (i != (uint32_t) -1) {
		foo = ydpdict_read_rtf(&dict, i);

		printf("-----\n%s\n-----\n%s\n-----\n", foo, ydpdict_read_xhtml(&dict, i));

		f = fopen("test.html", "w");

		fprintf(f, "%s\n", ydpdict_read_xhtml(&dict, i));
		fclose(f);
	}

	ydpdict_close(&dict);

	return 0;
}

