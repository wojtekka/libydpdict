#include <stdio.h>
#include <stdlib.h>
#include <ydpdict/ydpdict.h>

#define DICT_PATH "/usr/local/share/ydpdict"

int main(int argc, char **argv)
{
	ydpdict_t dict;
	uint32_t i, j;

	for (j = 0; j < 4; j++) {
		int dicts[4] = { 100, 101, 200, 201 };
		char dat[4096], idx[4096];

		snprintf(dat, sizeof(dat), DICT_PATH "/dict%d.dat", dicts[j]);
		snprintf(idx, sizeof(idx), DICT_PATH "/dict%d.idx", dicts[j]);

		if (ydpdict_open(&dict, dat, idx, YDPDICT_ENCODING_UTF8) == -1) {
			perror("ydpdict_open");
			return 1;
		}

		for (i = 0; i < dict.word_count; i++) {
			unsigned char *tmp;
	
			tmp = ydpdict_read_rtf(&dict, i);
			free(tmp);
	
			tmp = ydpdict_read_xhtml(&dict, i);
			free(tmp);
		}
	
		ydpdict_close(&dict);
	}
	
	return 0;
}

