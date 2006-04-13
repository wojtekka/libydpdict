/*
 * gentable.c
 *
 * Generates windows-1250 to utf-8 conversion table with iconv.
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>

int main(void)
{
	iconv_t cd;
	int i;

	cd = iconv_open("utf-8", "windows-1250");

	if (cd == (iconv_t)(-1)) {
		perror("iconv_open");
		exit(1);
	}

	for (i = 128; i < 256; i++) {
		char inbuf[2], outbuf[16], *in, *out;
		size_t inbytes, outbytes, res;

		inbuf[0] = i;
		inbuf[1] = 0;
		inbytes = 1;
		in = inbuf;

		outbuf[0] = 0;
		outbytes = sizeof(outbuf) - 1;
		out = outbuf;

		res = iconv(cd, &in, &inbytes, &out, &outbytes);
		*out = 0;

		if ((i % 8) == 0)
			printf("\t");
		
		if (res == (size_t)(-1))
			printf("\"?\", ");
		else
			printf("\"%s\", ", outbuf);

		if ((i % 8) == 7)
			printf("\n");
	}

	iconv_close(cd);

	return 0;
}
