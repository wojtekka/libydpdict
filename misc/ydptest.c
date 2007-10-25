#include <stdio.h>
#include <ydpdict/ydpdict.h>

#define DICT_PATH "/usr/local/share/ydpdict"

int main(int argc, char **argv)
{
	ydpdict_t dict;
	uint32_t i;
	FILE *f;

	if (ydpdict_open(&dict, DICT_PATH "/dict100.dat", DICT_PATH "/dict100.idx", YDPDICT_ENCODING_UTF8) == -1) {
		perror("ydpdict_open");
		return 1;
	}

	ydpdict_xhtml_set_title(&dict, "ydpdict");

	if (argc < 2)
		i = 0;
	else if (!(i = atoi(argv[1])))
		i = ydpdict_find(&dict, argv[1]);

	printf("ydpdict_find = %u\n", i);
	
	if (i != (uint32_t) -1) {
		char *rtf, *xhtml;
		
		rtf = ydpdict_read_rtf(&dict, i);
		xhtml = ydpdict_read_xhtml(&dict, i);
		
		printf("-----\n%s\n-----\n%s\n-----\n", rtf, xhtml);
		
		f = fopen("ydptest.rtf", "w");
		fprintf(f, "%s\n", rtf);
		fclose(f);

		f = fopen("ydptest.html", "w");
		fprintf(f, "%s\n", xhtml);
		fclose(f);
		
		free(rtf);
		free(xhtml);
	}

	ydpdict_close(&dict);

	return 0;
}

