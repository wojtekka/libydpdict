#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ydpdict/ydpdict.h>

#define DICT_PATH "/usr/local/share/ydpdict"

int main(int argc, char **argv)
{
	ydpdict_t *dict;
	uint32_t i, j;

	for (j = 0; j < 4; j++) {
		int count, dicts[4] = { 100, 101, 200, 201 };
		char dat[4096], idx[4096];

		snprintf(dat, sizeof(dat), DICT_PATH "/dict%d.dat", dicts[j]);
		snprintf(idx, sizeof(idx), DICT_PATH "/dict%d.idx", dicts[j]);

		if (!(dict = ydpdict_open(dat, idx, YDPDICT_ENCODING_UTF8))) {
			perror("ydpdict_open");
			return 1;
		}

		count = ydpdict_get_count(dict);

		printf("%s %d ", (strrchr(dat, '/')) ? (strrchr(dat, '/') + 1) : dat, count);
		fflush(stdout);

		for (i = 0; i < count; i++) {
			unsigned char *tmp;
	
			if (i % 1000 == 999) {
				printf("#");
				fflush(stdout);
			}

			tmp = ydpdict_read_rtf(dict, i);
			free(tmp);
	
			tmp = ydpdict_read_xhtml(dict, i);
			free(tmp);
		}

		printf("\n");
	
		ydpdict_close(dict);
	}
	
	return 0;
}

