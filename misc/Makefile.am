EXTRA_DIST = gentable.c ydptest.c ydpstress.c mkdict.py
libydpdict = ../src/.libs/libydpdict.a

ydpstress:	ydpstress.c $(libydpdict)
	$(CC) $(CFLAGS) $< -o $@ $(libydpdict)

ydptest:	ydptest.c $(libydpdict)
	$(CC) $(CFLAGS) $< -o $@ $(libydpdict)

ydpxml:	ydpxml.c $(libydpdict)
	$(CC) $(shell pkg-config libxml-2.0 --cflags --libs) $(CFLAGS) $< -o $@ $(libydpdict)

gentable:	gentable.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f gmon.out test.html ydpstress ydptest *.o *~ core *.dat *.idx
