
CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -Werror

PREFIX?=/usr/local
DESTDIR?=
BINDIR=$(DESTDIR)$(PREFIX)/bin

all: diredit

diredit: diredit.c
	$(CC) $(CFLAGS) -o $@ $<

install: diredit
	mkdir -p $(BINDIR)
	cp diredit $(BINDIR)

uninstall:
	rm $(BINDIR)/diredit

clean:
	rm -f diredit *.o

.PHONY: clean install uninstall
