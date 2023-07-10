.PHONY: clean install uninstall

CFLAGS ?= -O2 -fPIC -fstack-protector-strong -D_GNU_SOURCE -s -z norelro
PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
DESTDIR ?=

supernice: main.c
	$(CC) $(CFLAGS) $< -o $@

install: supernice
	install -D -m 755 supernice "$(DESTDIR)$(BINDIR)/supernice"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/supernice"

clean:
	rm -f supernice
