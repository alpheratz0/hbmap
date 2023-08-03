.POSIX:
.PHONY: all clean install uninstall dist

include config.mk

all: hbmap

hbmap: hbmap.o
	$(CC) $(LDFLAGS) -o hbmap hbmap.o $(LDLIBS)

clean:
	rm -f hbmap hbmap.o hbmap-$(VERSION).tar.gz

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f hbmap $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/hbmap
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f hbmap.1 $(DESTDIR)$(MANPREFIX)/man1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/hbmap.1
	cp -f utils/hbmap-download-random-map $(DESTDIR)$(PREFIX)/bin
	cp -f utils/hbmap-rofi-frontend $(DESTDIR)$(PREFIX)/bin

dist: clean
	mkdir -p hbmap-$(VERSION)
	cp -R COPYING config.mk Makefile README utils \
		hbmap.1 hbmap.c hbmap-$(VERSION)
	tar -cf hbmap-$(VERSION).tar hbmap-$(VERSION)
	gzip hbmap-$(VERSION).tar
	rm -rf hbmap-$(VERSION)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/hbmap
	rm -f $(DESTDIR)$(PREFIX)/bin/hbmap-download-random-map
	rm -f $(DESTDIR)$(PREFIX)/bin/hbmap-rofi-frontend
	rm -f $(DESTDIR)$(MANPREFIX)/man1/hbmap.1
