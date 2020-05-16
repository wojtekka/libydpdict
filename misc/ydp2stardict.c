#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "ydpdict.h"

#define DICT_PATH "/usr/local/share/ydpdict"

int dummy_g_ascii_strcasecmp(const char *a, const char *b)
{
	while ((*a != 0) && (*b != 0)) {
		int res = (int)(unsigned char)tolower(*a) - (int)(unsigned char)tolower(*b);
		if (res != 0)
			return res;
		a++;
		b++;
	}

	return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int stardict_cmp(const char *a, const char *b)
{
	int res = dummy_g_ascii_strcasecmp(a, b);
	if (res == 0)
		res = strcmp(a, b);
	return res;
}

struct entry {
	const char *word;
	unsigned int offset;
	unsigned int size;
};

int compare_entries(const void *a, const void *b)
{
	struct entry *entry_a = (void*) a;
	struct entry *entry_b = (void*) b;

	return stardict_cmp(entry_a->word, entry_b->word);
}

int main(int argc, char **argv)
{
	ydpdict_t *dict;
	int i, count;
	struct entry *entries;
	char dat[4096], idx[4096];

	if (argc != 2) {
		fprintf(stderr, "usage: %s <100|101|200|201>\n", argv[0]);
		exit(1);
	}

	snprintf(dat, sizeof(dat), DICT_PATH "/dict%s.dat", argv[1]);
	snprintf(idx, sizeof(idx), DICT_PATH "/dict%s.idx", argv[1]);

	if (!(dict = ydpdict_open(dat, idx, YDPDICT_ENCODING_UTF8))) {
		perror("ydpdict_open");
		return 1;
	}

	count = ydpdict_get_count(dict);

	entries = calloc(sizeof(struct entry), count);

	if (entries == NULL) {
		perror("calloc");
		exit(1);
	}

	snprintf(dat, sizeof(dat), "dict%s.dict", argv[1]);

	FILE *filp = fopen(dat, "wb");

	if (filp == NULL) {
		perror(dat);
		exit(1);
	}

	unsigned int offset = 0;

	for (i = 0; i < count; i++) {
		unsigned char *tmp;
	
		tmp = ydpdict_read_xhtml(dict, i);

		if (fwrite(tmp, 1, strlen(tmp), filp) != strlen(tmp)) {
			perror(dat);
			exit(1);
		}

		entries[i].word = ydpdict_get_word(dict, i);
		entries[i].offset = offset;
		entries[i].size = strlen(tmp);

		offset += strlen(tmp);

		free(tmp);
	}

	if (fclose(filp) == EOF) {
		perror(dat);
		exit(1);
	}

	qsort(entries, count, sizeof(struct entry), compare_entries);

	snprintf(dat, sizeof(dat), "dict%s.idx", argv[1]);

	filp = fopen(dat, "wb");

	if (filp == NULL) {
		perror(dat);
		exit(1);
	}

	for (i = 0; i < count; i++) {
		if (fwrite(entries[i].word, 1, strlen(entries[i].word) + 1, filp) != strlen(entries[i].word) + 1) {
			perror(dat);
			exit(1);
		}

		unsigned char buf[8];

		buf[0] = (entries[i].offset >> 24) & 255;
		buf[1] = (entries[i].offset >> 16) & 255;
		buf[2] = (entries[i].offset >> 8) & 255;
		buf[3] = entries[i].offset & 255;
		buf[4] = (entries[i].size >> 24) & 255;
		buf[5] = (entries[i].size >> 16) & 255;
		buf[6] = (entries[i].size >> 8) & 255;
		buf[7] = entries[i].size & 255;

		if (fwrite(buf, 1, sizeof(buf), filp) != sizeof(buf)) {
			perror(dat);
			exit(1);
		}
	}

	long idx_file_size = ftell(filp);

	if (fclose(filp) == EOF) {
		perror(dat);
		exit(1);
	}

	snprintf(dat, sizeof(dat), "dict%s.ifo", argv[1]);

	filp = fopen(dat, "wb");

	if (filp == NULL) {
		perror(dat);
		exit(1);
	}

	fprintf(filp, "StarDict's dict ifo file\n"
			"version=2.4.2\n"
			"bookname=dict%s\n"
			"wordcount=%d\n"
			"idxfilesize=%ld\n"
			"sametypesequence=h\n", argv[1], count, idx_file_size);
   
	if (fclose(filp) == EOF) {
		perror(dat);
		exit(1);
	}

	free(entries);

	ydpdict_close(dict);



	return 0;
}

