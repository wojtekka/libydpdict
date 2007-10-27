/*
 *  ydpdict support library
 *  (C) Copyright 1998-2007 Wojtek Kaniewski <wojtekka@toxygen.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License Version
 *  2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
 *  USA.
 *  
 *  $Id$
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <ctype.h>
#include "ydpdict.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define ATTR_B 1
#define ATTR_CF0 2
#define ATTR_CF1 4
#define ATTR_CF2 8
#define ATTR_QC 16
#define ATTR_SUPER 32
#define ATTR_F 64
#define ATTR_F1 128
#define ATTR_I 256
#define ATTR_CF5 1024
#define ATTR_SA 2048

/**
 * \brief Conversion table from phonetic characters to UTF-8
 */
static char *ydpdict_phonetic_to_utf8_table[32] =
{
	"?", "?", "ɔ", "ʒ", "?", "ʃ", "ɛ", "ʌ",
	"ə", "θ", "ɪ", "ɑ", "?", "ː", "ˈ", "?",
	"ŋ", "?", "?", "?", "?", "?", "?", "ð",
	"æ", "?", "?", "?", "?", "?", "?", "?"
};

/**
 * \brief Conversion table from windows-1250 to UTF-8
 */
static char *ydpdict_windows1250_to_utf8_table[128] =
{
	"€", "?", "‚", "?", "„", "…", "†", "‡", 
	"?", "‰", "Š", "‹", "Ś", "Ť", "Ž", "Ź", 
	"?", "‘", "’", "“", "”", "•", "–", "—", 
	"?", "™", "š", "›", "ś", "ť", "ž", "ź", 
	" ", "ˇ", "˘", "Ł", "¤", "Ą", "¦", "§", 
	"¨", "©", "Ş", "«", "¬", "­", "®", "Ż", 
	"°", "±", "˛", "ł", "´", "µ", "¶", "·", 
	"¸", "ą", "ş", "»", "Ľ", "˝", "ľ", "ż", 
	"Ŕ", "Á", "Â", "Ă", "Ä", "Ĺ", "Ć", "Ç", 
	"Č", "É", "Ę", "Ë", "Ě", "Í", "Î", "Ď", 
	"Đ", "Ń", "Ň", "Ó", "Ô", "Ő", "Ö", "×", 
	"Ř", "Ů", "Ú", "Ű", "Ü", "Ý", "Ţ", "ß", 
	"à", "á", "â", "ă", "ä", "ĺ", "ć", "ç", 
	"č", "é", "ę", "ë", "ě", "í", "î", "ï", 
	"đ", "ń", "ň", "ó", "ô", "ő", "ö", "÷", 
	"ř", "ů", "ú", "ű", "ü", "ý", "ţ", "˙", 
};

/**
 * \brief Convert 32-bit value from little-endian to machine-endian
 *
 * \param value little-endian value
 * 
 * \return machine-endian value
 */
static inline uint32_t ydpdict_fix32(uint32_t value)
{
#ifndef WORDS_BIGENDIAN
	return value;
#else
	return (uint32_t) (((value & (uint32_t) 0x000000ffU) << 24) |
		((value & (uint32_t) 0x0000ff00U) << 8) |
		((value & (uint32_t) 0x00ff0000U) >> 8) |
		((value & (uint32_t) 0xff000000U) >> 24));
#endif		
}

/**
 * \brief Convert 16-bit value from little-endian to machine-endian
 *
 * \param value little-endian value
 * 
 * \return machine-endian value
 */
static inline uint16_t ydpdict_fix16(uint16_t value)
{
#ifndef WORDS_BIGENDIAN
	return value;
#else
	return (uint16_t) (((value & (uint16_t) 0x00ffU) << 8) |
		((value & (uint16_t) 0xff00U) >> 8));
#endif
}

/**
 * \brief Open dictionary and read words' indices
 * 
 * The common mistake is to supply lowercase names, while the files have
 * uppercase names.
 * 
 * \param dict allocated dictionary description
 * \param dat data file path
 * \param idx index file path
 * \param encoding output encoding for XHTML
 * 
 * \return 0 on success, -1 on error
 */
int ydpdict_open(ydpdict_t *dict, const char *dat, const char *idx, ydpdict_encoding_t encoding)
{
	uint32_t index;
	int i, j;

	/* Clear ydpdict_t and set defaults */

	dict->idx = NULL;
	dict->dat = NULL;
	dict->word_count = 0;
	dict->words = NULL;
	dict->indices = NULL;
	dict->xhtml_header = 1;
	dict->xhtml_title = NULL;
	dict->xhtml_style = NULL;
	dict->xhtml_use_style = 0;
	dict->encoding = encoding;
		
	/* Open files */

	if (!(dict->idx = fopen(idx, "r")))
		goto failure;
	
	if (!(dict->dat = fopen(dat, "r")))
		goto failure;

	/* Read word count */
	
	if (fseek(dict->idx, 8, SEEK_SET) == (off_t) -1)
		goto failure;

	dict->word_count = 0;
	
	if (fread(&dict->word_count, sizeof(dict->word_count), 1, dict->idx) != 1)
		goto failure;
	
	dict->word_count = ydpdict_fix16(dict->word_count);

	/* Allocate memory */

	if (!(dict->indices = calloc(dict->word_count, sizeof(uint32_t))))
		goto failure;
	
	if (!(dict->words = calloc(dict->word_count + 1, sizeof(char*))))
		goto failure;

	dict->words[dict->word_count] = NULL;
    
	/* Read index table offset */

	if (fseek(dict->idx, 16, SEEK_SET) == (off_t) -1)
		goto failure;
	
	index = 0;

	if (fread(&index, sizeof(index), 1, dict->idx) != 1)
		goto failure;

	index = ydpdict_fix32(index);
	
	if (fseek(dict->idx, index, SEEK_SET) == (off_t) -1)
		goto failure;

	/* Read index table */

	i = 0;
	
	do {
		char buf[256];

		if (fseek(dict->idx, 4, SEEK_CUR) == (off_t) -1)
			goto failure;

		dict->indices[i] = 0;
		
		if (fread(&dict->indices[i], 4, 1, dict->idx) != 1)
			goto failure;
		
		dict->indices[i] = ydpdict_fix32(dict->indices[i]);

		j = 0;

		do {
			unsigned char ch;

			if (fread(&ch, 1, 1, dict->idx) != 1)
				goto failure;

			if (dict->encoding == YDPDICT_ENCODING_WINDOWS1250)
				buf[j] = ch;
			else {
				if (ch > 127) {
					const char *str = ydpdict_windows1250_to_utf8_table[ch - 128];
					int k;

					for (k = 0; str[k] && j < sizeof(buf); k++)
						buf[j++] = str[k];
					
					j--;
				} else
					buf[j] = ch;
			}
		} while (j < sizeof(buf) && buf[j++]);

		if (!(dict->words[i] = strdup(buf)))
			goto failure;

	} while (++i < dict->word_count);

	return 0;
	
failure:
	ydpdict_close(dict);

	return -1;
}

/**
 * \brief Read word definition in original RTF format, without charset 
 * convertion
 *
 * \param dict dictionary description
 * \param def definition index
 *
 * \return allocated buffer with definition on success, NULL on error
 */
char *ydpdict_read_rtf(const ydpdict_t *dict, uint32_t def)
{
	char *text = NULL;
	uint32_t len = 0;

	if (!dict || def >= dict->word_count) {
		errno = EINVAL;
		return NULL;
	}

	if (fseek(dict->dat, dict->indices[def], SEEK_SET) == (off_t) -1)
		goto failure;

	if (fread(&len, 4, 1, dict->dat) != 1)
		goto failure;
	
	len = ydpdict_fix32(len);

	if (!(text = malloc(len + 1)))
		goto failure;

	if (fread(text, 1, len, dict->dat) != len)
		goto failure;

	text[len] = 0;

	return text;

failure:
	if (text)
		free(text);

	return NULL;
}

/**
 * \brief Find specified word's index in dictionary
 *
 * \param dict dictionary description
 * \param word complete or partial word
 * 
 * \return definition index on success, (uint32_t) -1 on error
 */
uint32_t ydpdict_find(const ydpdict_t *dict, const char *word)
{
	int i = 0;

	if (!dict)
		return (uint32_t) -1;
	
	for (; i < dict->word_count; i++)
		if (!strncasecmp(dict->words[i], word, strlen(word)))
			return i;
	
	return (uint32_t) -1;
}

/**
 * \brief Close dictionary
 *
 * \param dict dictionary definition
 *
 * \return 0 on success, -1 on error
 */
int ydpdict_close(ydpdict_t *dict)
{
	if (dict->indices) {
		free(dict->indices);
		dict->indices = NULL;
	}
	
	if (dict->words) {
		int i = 0;

		while (dict->words[i]) {
			free(dict->words[i]);
			i++;
		}
		
		free(dict->words);
		dict->words = NULL;
	}
	
	if (dict->dat) {
		fclose(dict->dat);
		dict->dat = NULL;
	}

	if (dict->idx) {
		fclose(dict->idx);
		dict->idx = NULL;
	}

	if (dict->xhtml_title) {
		free(dict->xhtml_title);
		dict->xhtml_title = NULL;
	}

	if (dict->xhtml_style) {
		free(dict->xhtml_style);
		dict->xhtml_style = NULL;
	}

	return 0;
}

/**
 * \brief Append text to a string
 *
 * \param buf pointer to char*, freed on error
 * \param len pointer to buffer length
 * \param str string to be appended
 *
 * \return 0 on success, -1 on error
 */
static int ydpdict_append(char **buf, int *len, const char *str)
{
	int len1, len2;
	char *tmp;

	len1 = strlen(*buf);
	len2 = strlen(str);

	if (len1 + len2 > *len - 1) {
		while (len1 + len2 > *len - 1)
			*len <<= 1;
	
		if (!(tmp = realloc(*buf, *len))) {
			free(*buf);
			return -1;
		}
		
		*buf = tmp;
	}

	strcpy(*buf + len1, str);

	return 0;
}

/**
 * \brief Read word definition in XHTML format
 *
 * \param dict dictionary description
 * \param def definition index
 *
 * \return allocated buffer with definition on success, NULL on error
 */
char *ydpdict_read_xhtml(const ydpdict_t *dict, uint32_t def)
{
	char *buf = NULL;
	int attr_stack[16], block_stack[16], level = 0, attr = 0, block_begin = 0;
	int paragraph = 1, margin = 0, buf_len;
	unsigned char *rtf, *rtf_orig;

#undef APPEND
#define APPEND(x) \
	do { \
		if (ydpdict_append(&buf, &buf_len, x) == -1) \
			goto failure; \
	} while (0)

	if (!(rtf = (unsigned char*) ydpdict_read_rtf(dict, def)))
		return NULL;

	rtf_orig = rtf;

	buf_len = 256;

	if (!(buf = malloc(buf_len)))
		goto failure;
	
	buf[0] = 0;

	if (dict->xhtml_header) {
		const char *charset;
		
		switch (dict->encoding) {
			case YDPDICT_ENCODING_UTF8:
				charset = "utf-8";
				break;
			case YDPDICT_ENCODING_WINDOWS1250:
				charset = "windows-1250";
				break;
			default:
				charset = NULL;
		}
		
		APPEND("<?xml version=\"1.0\"?>\n<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");

		if (dict->xhtml_title) {
			APPEND("<title>");
			APPEND(dict->xhtml_title);
			APPEND("</title>");
		}
		if (charset) {
			APPEND("<meta http-equiv=\"Content-type\" content=\"text/html; charset=");
			APPEND(charset);
			APPEND("\" />");
		}
		if (dict->xhtml_style) {
			APPEND("<style>");
			APPEND(dict->xhtml_style);
			APPEND("</style>");
		}
		APPEND("</head><body>");
	}

	APPEND("<p>");

	while (*rtf) {
		switch (*rtf) {
			case '{':
				if (level < 16) {
					attr_stack[level] = attr;
					block_stack[level] = block_begin;
					level++;
				}

				block_begin = 1;

				attr = 0;
				
				if (margin && !(attr_stack[level - 1] & ATTR_SA))
					attr |= ATTR_SA;
				
				rtf++;
				break;

			case '}':
				if (!level)
					break;

				// don't end tags if they haven't started.
				//
				if (!block_begin) {
					if ((attr & ATTR_SUPER))
						APPEND("</sup>");

					if ((attr & (ATTR_CF0 | ATTR_CF1 | ATTR_CF2 | ATTR_CF5)))
						APPEND("</span>");
				
					if ((attr & ATTR_I))
						APPEND("</i>");

					if ((attr & ATTR_B))
						APPEND("</b>");
				}

				if (margin && (attr & ATTR_SA)) {
					APPEND("</div><p>");
					margin = 0;
				}
				
				paragraph = 0;

				level--;
				attr = attr_stack[level];
				block_begin = block_stack[level];

				rtf++;
				break;

			case '\\':
			{
				int len = 0;
				char token[16];
				
				rtf++;
				
				while (isalnum(*rtf)) {
					if (len < 14)
						token[len++] = *rtf;
					
					rtf++;
				}

				if (*rtf == ' ')
					rtf++;
				
				token[len] = 0;

				if (!strcmp(token, "par") && margin) {
					APPEND("</div><p>");
					attr &= ~ATTR_SA;
					margin = 0;
				}

				if (!strcmp(token, "pard")) {
					APPEND("</p><p>");
					paragraph = 1;
				}

				if (!strcmp(token, "line") && !paragraph)
					APPEND("<br />");

				if (block_begin) {
					if (!strcmp(token, "b"))
						attr |= ATTR_B;

					if (!strcmp(token, "i"))
						attr |= ATTR_I;

					if (!strcmp(token, "cf0"))
						attr |= ATTR_CF0;

					if (!strcmp(token, "cf1"))
						attr |= ATTR_CF1;

					if (!strcmp(token, "cf2"))
						attr |= ATTR_CF2;
	
					if (!strcmp(token, "cf5"))
						attr |= ATTR_CF5;
				}

				if (!strcmp(token, "qc"))
					attr |= ATTR_QC;

				if (!strcmp(token, "super")) {
					attr |= ATTR_SUPER;
					block_begin = 1;
				}

				if (!strncmp(token, "f", 1) && strcmp(token, "f1"))
					attr |= ATTR_F;

				if (!strcmp(token, "f1"))
					attr |= ATTR_F1;

				if (!strncmp(token, "sa", 2)) {
					if (!margin) {
						if (dict->xhtml_use_style)
							APPEND("</p><div class=\"example\">");
						else
							APPEND("</p><div style=\"margin: -1em 2em auto 2em;\">");
						margin = 1;
					} else
						APPEND("<br />");
				}
					
				break;
			}

			default:
				if (block_begin && *rtf != ' ') {
					block_begin = 0;

					if ((attr & ATTR_B))
						APPEND("<b>");

					if ((attr & ATTR_I))
						APPEND("<i>");

					if ((attr & (ATTR_CF0 | ATTR_CF1 | ATTR_CF2 | ATTR_CF5))) {
						const char *color = NULL, *class = NULL;

						if ((attr & ATTR_CF0)) {
							color = "blue";
							class = "cf0";
						}

						if ((attr & ATTR_CF1)) {
							color = "green";
							class = "cf1";
						}

						if ((attr & ATTR_CF2)) {
							color = "red";
							class = "cf2";
						}

						if ((attr & ATTR_CF5)) {
							color = "magenta";
							class = "cf5";
						}
						
						if (dict->xhtml_use_style) {
							APPEND("<span class=\"");
							APPEND(class);
							APPEND("\">");
						} else {
							APPEND("<span style=\"color: ");
							APPEND(color);
							APPEND(";\">");
						}
					}

					if ((attr & ATTR_SUPER))
						APPEND("<sup>");
				}

				// workaround for square bracket like in
				// {[\f1\cf5 pronunciation]}
				//
				if (*rtf == ']' && (attr & ATTR_F1) && (attr & ATTR_CF5)) {
					attr &= ~ATTR_CF5;
					APPEND("</span>");
				}

				if (!(attr & ATTR_QC) && (dict->encoding == YDPDICT_ENCODING_UTF8)) {
					if ((attr & ATTR_F1) && *rtf > 127 && *rtf < 160)
						APPEND(ydpdict_phonetic_to_utf8_table[*rtf - 128]);
					else if (((attr & ATTR_F1) && *rtf > 160) || *rtf > 127)
						APPEND(ydpdict_windows1250_to_utf8_table[*rtf - 128]);
					else if (*rtf == 127)
						APPEND("~");
					else if (*rtf == '&')
						APPEND("&amp;");
					else if (*rtf == '<')
						APPEND("&lt;");
					else if (*rtf == '>')
						APPEND("&gt;");
					else {
						char tmp[2] = { *rtf, 0 };
						APPEND(tmp);
					}
				}

				if (!(attr & ATTR_QC) && (dict->encoding == YDPDICT_ENCODING_WINDOWS1250)) {
					if (*rtf == 127)
						APPEND("~");
					else {
						char tmp[2] = { *rtf, 0 };
						APPEND(tmp);
					}
				}

				paragraph = 0;

				rtf++;
		}
	}

	free(rtf_orig);

	APPEND("</p>");

	if (dict->xhtml_header)
		APPEND("</body></html>");

#undef APPEND

	return buf;

failure:
	free(rtf_orig);

	return NULL;
}

/**
 * \brief Set XHTML style
 *
 * \param dict dictionary description
 * \param style style
 *
 * \result 0 on success, -1 on error
 */
int ydpdict_xhtml_set_style(ydpdict_t *dict, const char *style)
{
	if (!dict) {
		errno = EINVAL;
		return -1;
	}

	if (dict->xhtml_style) {
		free(dict->xhtml_style);
		dict->xhtml_style = NULL;
	}

	if (style && (!(dict->xhtml_style = strdup(style))))
		return -1;
	
	return 0;
}

/**
 * \brief Set XHTML title
 *
 * \param dict dictionary description
 * \param title title
 *
 * \result 0 on success, -1 on error
 */
int ydpdict_xhtml_set_title(ydpdict_t *dict, const char *title)
{
	if (!dict) {
		errno = EINVAL;
		return -1;
	}

	if (dict->xhtml_title) {
		free(dict->xhtml_title);
		dict->xhtml_title = NULL;
	}

	if (title && (!(dict->xhtml_title = strdup(title))))
		return -1;
	
	return 0;
}

/**
 * \brief Toggle XHTML header output
 *
 * \param dict dictionary description
 * \param header header output flag
 *
 * \result 0 on success, -1 on error
 */
int ydpdict_xhtml_set_header(ydpdict_t *dict, int header)
{
	if (!dict) {
		errno = EINVAL;
		return -1;
	}

	dict->xhtml_header = header;

	return 0;
}

/**
 * \brief Toggle XHTML style usage
 *
 * \param dict dictionary description
 * \param use_style style usage flag
 *
 * \result 0 on success, -1 on error
 */
int ydpdict_xhtml_set_use_style(ydpdict_t *dict, int use_style)
{
	if (!dict) {
		errno = EINVAL;
		return -1;
	}

	dict->xhtml_use_style = use_style;

	return 0;
}

/**
 * \brief Converts phonetic string to UTF-8
 *
 * \param input input string
 *
 * \return allocated buffer with converted string on success, NULL on error
 */
char *ydpdict_phonetic_to_utf8(const char *input)
{
	int i, len = 0;
	char *result;
	
	for (i = 0; input[i]; i++) {
		if (((unsigned char*) input)[i] >= 128 && ((unsigned char*) input)[i] < 160)
			len += strlen(ydpdict_phonetic_to_utf8_table[((unsigned char*) input)[i] - 128]);
		else
			len++;
	}

	if (!(result = malloc(len + 1)))
		return NULL;

	strcpy(result, "");

	for (i = 0; input[i]; i++) {
		if (((unsigned char*) input)[i] >= 128 && ((unsigned char*) input)[i] < 160)
			strcat(result, ydpdict_phonetic_to_utf8_table[((unsigned char*) input)[i] - 128]);
		else {
			char tmp[2] = { input[i], 0 };

			strcat(result, tmp);
		}
	}
	
	return result;
}

/**
 * \brief Converts windows1250 string to UTF-8
 *
 * \param input input string
 *
 * \return allocated buffer with converted string on success, NULL on error
 */
char *ydpdict_windows1250_to_utf8(const char *input)
{
	int i, len = 0;
	char *result;
	
	for (i = 0; input[i]; i++) {
		if (((unsigned char*) input)[i] >= 128)
			len += strlen(ydpdict_windows1250_to_utf8_table[((unsigned char*) input)[i] - 128]);
		else
			len++;
	}

	if (!(result = malloc(len + 1)))
		return NULL;

	strcpy(result, "");

	for (i = 0; input[i]; i++) {
		if (((unsigned char*) input)[i] >= 128)
			strcat(result, ydpdict_windows1250_to_utf8_table[((unsigned char*) input)[i] - 128]);
		else {
			char tmp[2] = { input[i], 0 };

			strcat(result, tmp);
		}
	}
	
	return result;
}


