.POSIX:
.PHONY: all clean install uninstall dist

include config.mk

all: hbmapquery

hbmapquery: hbmapquery.o
	$(CC) $(LDFLAGS) -o hbmapquery hbmapquery.o $(LDLIBS)

clean:
	rm -f hbmapquery hbmapquery.o hbmapquery-$(VERSION).tar.gz

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f hbmapquery $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/hbmapquery
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f hbmapquery.1 $(DESTDIR)$(MANPREFIX)/man1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/hbmapquery.1

dist: clean
	mkdir -p hbmapquery-$(VERSION)
	cp -R COPYING config.mk Makefile README hbmapquery.1 hbmapquery.c hbmapquery-$(VERSION)
	tar -cf hbmapquery-$(VERSION).tar hbmapquery-$(VERSION)
	gzip hbmapquery-$(VERSION).tar
	rm -rf hbmapquery-$(VERSION)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/hbmapquery
	rm -f $(DESTDIR)$(MANPREFIX)/man1/hbmapquery.1
