# Makefile -- build file for detab.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Sun Sep  8 12:51:15 EEST 2019
# Copyright: (C) 2019 LUIS COLORADO.  All rights reserved.
# License: BSD

RM             ?= rm -f
INSTALL        ?= /usr/bin/install

targets         = detab entab
toclean         = $(targets)

prefix         ?= $(HOME)
bindir         ?= $(prefix)/bin
datadir		   ?= $(prefix)/share
mandir		   ?= $(datadir)/man
man1dir		   ?= $(mandir)/man1
own            ?= `id -u`
grp            ?= `id -g`
xmod           ?= 755
mmod		   ?= 644


detab_objs      = detab.o
detab_deps      =
detab_libs      =
detab_ldflags   =

tabber_objs      = tabber.o
tabber_deps      =
tabber_libs      =
tabber_ldflags   =

all: $(targets)
clean:
	$(RM) $(toclean)
install: $(targets) detab.1.gz
	$(INSTALL) -o $(own) -g $(grp) -m $(xmod) detab $(bindir)/detab
	ln -f $(bindir)/detab $(bindir)/entab
	$(INSTALL) -o $(own) -g $(grp) -m $(mmod) detab.1.gz $(man1dir)/detab.1.gz
	ln -f $(man1dir)/detab.1.gz $(man1dir)/entab.1.gz
uninstall:
	$(RM) \
		$(bindir)/detab \
		$(bindir)/entab \
		$(man1dir)/detab.1.gz \
		$(man1dir)/entab.1.gz
	
detab.1.gz: detab.1
	gzip <detab.1 >$@

toclean += $(detab_objs)
detab: $(detab_deps) $(detab_objs)
	$(CC) $(LDFLAGS) -o $@ $($@_objs) $($@_ldflags) $($@_libs) $(LIBS)
entab: detab
	ln -f detab entab
