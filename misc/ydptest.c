#include <stdio.h>
#include <ydpdict/ydpdict.h>

#define DICT_PATH "/usr/local/share/ydpdict"

int main(int argc, char **argv)
{
	char dat[128], idx[128];
	ydpdict_t dict;
	uint32_t i;
	FILE *f;
	int dictno;
	const char *word;

	if (argc == 1) {
		dictno = 100;
		word = "0";
	} else if (argc == 2) {
		dictno = atoi(argv[1]);
		word = "0";
	} else if (argc == 3) {
		dictno = atoi(argv[1]);
		word = argv[2];
	} else {
		fprintf(stderr, "usage: %s [dict] [word]\n", argv[0]);
		return 1;
	}
	
	snprintf(dat, sizeof(dat), "%s/dict%03d.dat", DICT_PATH, dictno);
	snprintf(idx, sizeof(idx), "%s/dict%03d.idx", DICT_PATH, dictno);

	if (ydpdict_open(&dict, dat, idx, YDPDICT_ENCODING_UTF8) == -1) {
		perror("ydpdict_open");
		return 1;
	}

	if (!(i = atoi(word)))
		i = ydpdict_find(&dict, word);

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

