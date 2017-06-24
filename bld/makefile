#
# Copyright (C) Jonathan D. Belanger 2017.
# All Rights Reserved.
#
# This software is furnished under a license and may be used and copied only
# in accordance with the terms of such license and with the inclusion of the
# above copyright notice.  This software or any other copies thereof may not
# be provided or otherwise made available to any other person.  No title to
# and ownership of the software is hereby transferred.
#
# The information in this software is subject to change without notice and
# should not be construed as a commitment by the author or co-authors.
#
# The author and any co-authors assume no responsibility for the use or
# reliability of this software.
#
# Description:
#
#		This make file is used to build the executables for the DECemu project.
#
#	Revision History:
#
#	V01.000		03-Jun-2017	Jonathan D. Belanger
#	Initially written.
#
#	V01.001		10-Jun-2017	Jonathan D. Belanger
#	Updates for makedepend considerations.
#
#	V01.002		24-Jun-2017	Jonathan D. Belanger
#	Update to have the make clean also delete the makedepend generated
#	Makefile, but rename Makefile.bak to Makefile, but only if the .bak file
#	exists.
#
PROJDIR = /cygdrive/g/git/DECemu/

SRCDIR = $(PROJDIR)src/
TSTDIR = $(PROJDIR)tst/
OBJDIR = $(PROJDIR)bin/
EXEDIR = $(PROJDIR)bin/
BLDDIR = $(PROJDIR)bld/

CFILES = $(wildcard $(SRCDIR)*/*.c)
SRCS = $(CFILES)
OBJS = $(addprefix $(OBJDIR), $(notdir $(CFILES:.c=.o)))

TSTFILES = $(wildcard $(TSTDIR)*.c)
EXE = .exe
TSTS = $(addprefix $(EXEDIR), $(notdir $(TSTFILES:.c=$(EXE))))
PROGS =

INCDIRS = $(wildcard $(SRCDIR)/*)
INCS = $(foreach d, $(INCDIRS), -I$d)

CC = gcc
CFLAGS = $(INCS) -Wall

debug: CFLAGS += -g -O0 -DDEBUG
debug: all

all: $(OBJS) $(TSTS) $(PROGS)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(EXEDIR)%$(EXE) : $(TSTDIR)%.c $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $<

$(OBJDIR)%.o : $(SRCDIR)*/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(OBJDIR)* $(EXEDIR)*
ifneq ($(wildcard $(BLDDIR)*.bak),)
	rm -f $(BLDDIR)Makefile
	mv -f $(BLDDIR)Makefile.bak $(BLDDIR)Makefile
endif

depend:
	makedepend -- $(CFLAGS) -- $(SRCS) $(TSTFILES) $(PROGS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
