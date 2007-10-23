EXTRA_DIST = gentable.c ydptest.c ydpstress.c

ydpstress:	ydpstress.c
	$(CC) $(CFLAGS) $< -o $@ ../src/.libs/libydpdict.a

ydptest:	ydptest.c
	$(CC) $(CFLAGS) $< -o $@ ../src/.libs/libydpdict.a

gentable:	gentable.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f gmon.out test.html ydpstress ydptest *.o *~ core
