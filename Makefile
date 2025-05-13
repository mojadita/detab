# Makefile -- build file for detab.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Sun Sep  8 12:51:15 EEST 2019
# Copyright: (C) 2019 LUIS COLORADO.  All rights reserved.
# License: BSD

RM             ?= rm -f
INSTALL        ?= /usr/bin/install

OS             != uname -o

OWN-FreeBSD    ?= root
GRP-FreeBSD    ?= wheel
OWN-GNU/Linux  ?= bin
GRP-GNU/Linux  ?= bin

OWN            ?= $(OWN-$(OS))
GRP            ?= $(GRP-$(OS))

FMOD           ?= 444
XMOD           ?= 555
DMOD           ?= 755

IFLAGS         ?= -o $(OWN) -g $(GRP)

targets         = detab detab.1.gz
toclean        += $(targets)

prefix         ?= /usr/local
bindir         ?= $(prefix)/bin
datadir		   ?= $(prefix)/share
mandir		   ?= $(datadir)/man
man1dir		   ?= $(mandir)/man1

toinstall       = $(bindir)/detab \
                  $(bindir)/entab \
                  $(man1dir)/detab.1.gz \
                  $(man1dir)/entab.1.gz


detab_objs       = detab.o
detab_deps       =
detab_libs       =
detab_ldflags    =

tabber_objs      = tabber.o
tabber_deps      =
tabber_libs      =
tabber_ldflags   =

all: $(targets)
clean:
	$(RM) $(toclean)

install: $(toinstall)

$(bindir)/detab: $(@:T) $(@:H)
	-$(INSTALL) $(IFLAGS) -m $(XMOD) $(@:T) $@

$(bindir)/entab:
	-ln -sf detab $(bindir)/entab

$(man1dir)/detab.1.gz : $(@:T) $(@:H)
	-$(INSTALL) $(IFLAGS) -m $(FMOD) $(@:T) $@

$(man1dir)/entab.1.gz:
	-ln -sf detab.1.gz $(man1dir)/entab.1.gz

$(bindir) $(man1dir):
	-$(INSTALL) $(IFLAGS) -m $(DMOD) -d $@

uninstall:
	-$(RM) $(toinstall)
	-rmdir $(bindir) $(man1dir)
	
detab.1.gz: detab.1
	gzip -v <detab.1 >$@

toclean += $(detab_objs)

detab: $(detab_deps) $(detab_objs)
	$(CC) $(LDFLAGS) -o $@ $($@_objs) $($@_ldflags) $($@_libs) $(LIBS)

entab: detab
	ln -sf detab entab
