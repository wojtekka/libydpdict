/*
 *  ydpdict support library
 *  (C) Copyright 1998-2007 Wojtek Kaniewski <wojtekka@toxygen.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  $Id$
 */

#ifndef _YDPDICT_YDPDICT_H
#define _YDPDICT_YDPDICT_H

#include <stdio.h>
#include <inttypes.h>

/**
 * Output encoding type.
 */
typedef enum {
	YDPDICT_ENCODING_WINDOWS1250,
	YDPDICT_ENCODING_UTF8
} ydpdict_encoding_t;
	
/**
 * Dictionary description.
 */
typedef struct {
	FILE *dat;
	FILE *idx;
	char **words;
	uint16_t word_count;
	uint32_t *indices;
	
	ydpdict_encoding_t encoding;

	int xhtml_header;
	char *xhtml_title;
	char *xhtml_style;
	int xhtml_use_style;
} ydpdict_t;

int ydpdict_open(ydpdict_t *dict, const char *dat, const char *idx, ydpdict_encoding_t encoding);
uint32_t ydpdict_find(const ydpdict_t *dict, const char *word);
char *ydpdict_read_rtf(const ydpdict_t *dict, uint32_t def);
char *ydpdict_read_xhtml(const ydpdict_t *dict, uint32_t def);
int ydpdict_xhtml_set_header(ydpdict_t *dict, int header);
int ydpdict_xhtml_set_style(ydpdict_t *dict, const char *style);
int ydpdict_xhtml_set_use_style(ydpdict_t *dict, int use_style);
int ydpdict_xhtml_set_title(ydpdict_t *dict, const char *title);
int ydpdict_close(ydpdict_t *dict);
char *ydpdict_phonetic_to_utf8(const char *input);
char *ydpdict_windows1250_to_utf8(const char *input);

#endif /* _YDPDICT_YDPDICT_H */

