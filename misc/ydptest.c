/* This example and/or test program is released under the terms of CC0 license. */

#include <stdio.h>
#include <stdlib.h>

#include "ydpdict.h"

#define DICT_PATH "/usr/local/share/ydpdict"

int main(int argc, char **argv)
{
	char dat[128], idx[128];
	ydpdict_t *dict;
	FILE *f;
	int i, dictno;
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

	if (!(dict = ydpdict_open(dat, idx, YDPDICT_ENCODING_UTF8))) {
		perror("ydpdict_open");
		return 1;
	}

	if (!(i = atoi(word)))
		i = ydpdict_find_word(dict, word);

	printf("ydpdict_find_word() = %d\n", i);
	
	if (i != -1) {
		char *rtf, *xhtml;
		
		rtf = ydpdict_read_rtf(dict, i);
		xhtml = ydpdict_read_xhtml(dict, i);
		
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

	ydpdict_close(dict);

	return 0;
}

