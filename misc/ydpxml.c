#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ydpdict/ydpdict.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#define DICT_PATH "/usr/local/share/ydpdict"

int parse_xml(const char *xml, int validate)
{
	xmlParserCtxtPtr ctxt;
	xmlDocPtr doc;

	ctxt = xmlNewParserCtxt();

	if (ctxt == NULL) {
		perror("xmlNewParserCtxt");
		exit(1);
	}

	doc = xmlCtxtReadMemory(ctxt, xml, strlen(xml), "", NULL, (validate) ? XML_PARSE_DTDVALID : 0);

	if (doc == NULL) {
		return 0;
	} else {
		if (validate && ctxt->valid == 0)
			return 0;

		xmlFreeDoc(doc);
	}

	xmlFreeParserCtxt(ctxt);

	return 1;
}

int main(int argc, char **argv)
{
	const char *cmd;
	ydpdict_t *dict;
	uint32_t i, j;
	int validate;

	if (argc > 1 && !strcmp(argv[1], "--valid"))
		validate = 1;
	else
		validate = 0;

	for (j = 0; j < 4; j++) {
		int count, dicts[4] = { 100, 101, 200, 201 };
		char dat[4096], idx[4096], prefix[128];

		snprintf(dat, sizeof(dat), DICT_PATH "/dict%d.dat", dicts[j]);
		snprintf(idx, sizeof(idx), DICT_PATH "/dict%d.idx", dicts[j]);

		if (!(dict = ydpdict_open(dat, idx, YDPDICT_ENCODING_UTF8))) {
			perror("ydpdict_open");
			return 1;
		}

		snprintf(prefix, sizeof(prefix), "%s", (strrchr(dat, '/')) ? (strrchr(dat, '/') + 1) : dat);

		count = ydpdict_get_count(dict);

		for (i = 0; i < count; i++) {
			unsigned char *tmp;
			FILE *f;
	
			printf("\r\033[K%s %d/%d %s", prefix, i, count, ydpdict_get_word(dict, i));
			fflush(stdout);

			tmp = ydpdict_read_xhtml(dict, i);

			if (!parse_xml(tmp, validate))
				printf(" ERROR\n");
			
			free(tmp);
		}

		printf("\r\033[K");
		fflush(stdout);
	
		ydpdict_close(dict);
	}
	
	return 0;
}

