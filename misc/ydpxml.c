#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ydpdict/ydpdict.h>

#define DICT_PATH "/usr/local/share/ydpdict"

int main(int argc, char **argv)
{
	const char *cmd;
	ydpdict_t dict;
	uint32_t i, j;
	int valid;

	if (argc > 1 && !strcmp(argv[1], "--valid"))
		cmd = "xmllint --valid - 1> /dev/null 2> /dev/null";
	else
		cmd = "xmllint - 1> /dev/null 2> /dev/null";

	for (j = 0; j < 4; j++) {
		int dicts[4] = { 100, 101, 200, 201 };
		char dat[4096], idx[4096], prefix[128];

		snprintf(dat, sizeof(dat), DICT_PATH "/dict%d.dat", dicts[j]);
		snprintf(idx, sizeof(idx), DICT_PATH "/dict%d.idx", dicts[j]);

		if (ydpdict_open(&dict, dat, idx, YDPDICT_ENCODING_UTF8) == -1) {
			perror("ydpdict_open");
			return 1;
		}

		snprintf(prefix, sizeof(prefix), "%s", (strrchr(dat, '/')) ? (strrchr(dat, '/') + 1) : dat);

		for (i = 0; i < dict.word_count; i++) {
			unsigned char *tmp;
			FILE *f;
	
			printf("\r\033[K%s %d/%d %s", prefix, i, dict.word_count, dict.words[i]);
			fflush(stdout);

			tmp = ydpdict_read_xhtml(&dict, i);
			
			f = popen(cmd, "w");
			fprintf(f, "%s", tmp);
			if (pclose(f) != 0)
				printf(" ERROR\n");
			
			free(tmp);
		}

		printf("\r[033K");
		fflush(stdout);
	
		ydpdict_close(&dict);
	}
	
	return 0;
}

