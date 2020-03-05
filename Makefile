# Makefile -- build file for detab.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Sun Sep  8 12:51:15 EEST 2019
# Copyright: (C) 2019 LUIS COLORADO.  All rights reserved.
# License: BSD

RM				?= rm -f

targets			= detab
toclean			= $(targets)

detab_objs		= detab.o
detab_deps 		=
detab_libs		=
detab_ldflags	= 

all: $(targets)
clean:
	$(RM) $(toclean)

toclean += $(detab_objs)
detab: $(detab_deps) $(detab_objs)
	$(CC) $(LDFLAGS) -o $@ $($@_objs) $($@_ldflags) $($@_libs) $(LIBS)
