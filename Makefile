# Makefile -- build file for detab.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Sun Sep  8 12:51:15 EEST 2019
# Copyright: (C) 2019 LUIS COLORADO.  All rights reserved.
# License: BSD

RM              = -rm -f
INSTALL         = -/usr/bin/install

targets         = detab detab.1.gz
toclean         = $(targets)

OS	           != uname -o

prefix         ?= /usr/local
bindir         ?= $(prefix)/bin
datadir		   ?= $(prefix)/share
mandir		   ?= $(datadir)/man
man1dir		   ?= $(mandir)/man1

toinstall       = $(bindir)/detab \
                  $(bindir)/entab \
                  $(man1dir)/detab.1.gz \
                  $(man1dir)/entab.1.gz

own-GNU/Linux  ?= bin
grp-GNU/Linux  ?= bin
own-FreeBSD    ?= root
grp-FreeBSD    ?= wheel

own            ?= $(own-$(OS))
grp            ?= $(grp-$(OS))

xmod           ?= 755
fmod		   ?= 644

IFLAGS         ?= -o "$(own)" -g "$(grp)"

detab_objs      = detab.o
detab_deps      =
detab_libs      =
detab_ldflags   =
toclean += $(detab_objs)

toinstall      ?= $(bindir)/detab \
                  $(bindir)/entab \
				  $(man1dir)/detab.1.gz \
				  $(man1dir)/entab.1.gz

all: $(targets)
clean:
	$(RM) $(toclean)
install: $(toinstall)

$(bindir)/detab: detab $(bindir)
	$(INSTALL) $(IFLAGS) -m $(xmod) detab $@
$(bindir)/entab: detab $(bindir)
	ln -sf detab $@
$(man1dir)/detab.1.gz: detab.1.gz $(man1dir)
	$(INSTALL) $(IFLAGS) -m $(fmod) detab.1.gz $@
$(man1dir)/entab.1.gz: detab.1.gz
	ln -sf detab.1.gz $@
$(bindir) $(man1dir):
	$(INSTALL) $(IFLAGS) -m $(dmod) -d $@

uninstall deinstall:
	$(RM) $(toinstall)
	
.SUFFIXES: .1.gz .1

.1.1.gz: 
	gzip -v < $< >$@

detab: $(detab_deps) $(detab_objs)
	$(CC) $(LDFLAGS) -o $@ $($@_objs) $($@_ldflags) $($@_libs) $(LIBS)
