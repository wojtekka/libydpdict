check_PROGRAMS = ydpstress ydptest ydpxml ydp2stardict gentable
EXTRA_DIST = mkdict.py

ydpstress_CFLAGS = -I$(top_builddir)/include
ydpstress_LDADD = $(top_builddir)/src/libydpdict.la
ydpstress_SOURCES = ydpstress.c

ydptest_CFLAGS = -I$(top_builddir)/include
ydptest_LDADD = $(top_builddir)/src/libydpdict.la
ydptest_SOURCES = ydptest.c

ydpxml_CFLAGS = -I$(top_builddir)/include $(LIBXML2_CFLAGS)
ydpxml_LDADD = $(top_builddir)/src/libydpdict.la $(LIBXML2_LIBS)
ydpxml_SOURCES = ydpxml.c

ydp2stardict_CFLAGS = -I$(top_builddir)/include
ydp2stardict_LDADD = $(top_builddir)/src/libydpdict.la
ydp2stardict_SOURCES = ydp2stardict.c

gentable_SOURCES = gentable.c

clean-local:
	rm -f gmon.out test.html *.dat *.idx
