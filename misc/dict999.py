#!/usr/bin/env python
# -*- coding: utf-8 -*-

dict = [
	(ur"słowo", ur"{\b słowo} {[\f1\cf5 wymowa]}\par\pard{\line\cf2\i część mowy\line} ({\i dziedzina}){\f2\cf1 definicja},{\f2\cf1 definicja {\cf2\i rodzaj}}\par\sa100\li300{\cf0 Przykład użycia.}"),
]

import struct

idx = open("dict999.idx", "w")
dat = open("dict999.dat", "w")

idx.write(struct.pack("<LLHHLL236s", 0x8D4E11D5, 0, len(dict), 0, 0, 256, ""))

idx_offset = 1
dat_offset = 0

for i in dict:
	(word, definition) = i

	word = word.encode('windows-1250')
	definition = definition.encode('windows-1250')

	idx.write(struct.pack("<BBHL", len(word) + 1, 2, idx_offset, dat_offset) + word + "\0")

	dat.write(struct.pack("<L", len(definition)) + definition)

	idx_offset = idx_offset + 1
	dat_offset = 4 + len(definition)

idx.close()
dat.close()

