# Makefile -- build file for detab.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Sun Sep  8 12:51:15 EEST 2019
# Copyright: (C) 2019 LUIS COLORADO.  All rights reserved.
# License: BSD

RM             ?= rm -f
INSTALL        ?= /usr/bin/install

targets         = detab
toclean         = $(targets)

prefix         ?= $(HOME)
bindir         ?= $(HOME)/bin
own            ?= `id -u`
grp            ?= `id -g`
xmod           ?= 755


detab_objs      = detab.o
detab_deps      =
detab_libs      =
detab_ldflags   =

all: $(targets)
clean:
	$(RM) $(toclean)
install: $(targets)
	$(INSTALL) -o $(own) -g $(grp) -m $(xmod) detab $(bindir)/detab

toclean += $(detab_objs)
detab: $(detab_deps) $(detab_objs)
	$(CC) $(LDFLAGS) -o $@ $($@_objs) $($@_ldflags) $($@_libs) $(LIBS)
