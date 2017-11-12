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
#	V01.003		07-Jul-2017	Jonathan D. Belanger
#	Created an 'exe' directory into which the executables are to be stored.
#	Created a 'straighten' target, which just deletes the object and executable
#	files, and does not delete the Makefile and rename Makefile.bak to
#	Makefile.
#	Got makedepend to stop spitting out warnings about not finding standard
#	library header files.
#
#	V01.004		11-Nov-2017	Jonathan D. Belanger
#	Change the project name from DECemu to DECaxp.
#
PROJDIR = /cygdrive/g/git/DECaxp/

#
# Create variables for the various top-level directories we care about.
#
SRCDIR = $(PROJDIR)src/
TSTDIR = $(PROJDIR)tst/
OBJDIR = $(PROJDIR)bin/
EXEDIR = $(PROJDIR)exe/
BLDDIR = $(PROJDIR)bld/

#
# Create variables for the .c source code files for the emulator and the object
# files that are created from compiling.  This definition will look in all
# sub-directories under the top-level source directory.
#
CFILES = $(wildcard $(SRCDIR)*/*.c)
SRCS = $(CFILES)
OBJS = $(addprefix $(OBJDIR), $(notdir $(CFILES:.c=.o)))

#
# Create variables for the .c source code files and the object files that are
# created from compiling.
#
TSTFILES = $(wildcard $(TSTDIR)*.c)
EXE = .exe
TSTS = $(addprefix $(EXEDIR), $(notdir $(TSTFILES:.c=$(EXE))))

#
# Create the variable for the emulator source files (the emulator itself and
# any companion programs - ie: configuration programs, if any).
#
PROGS =

#
# Create variables for the include path for all the emulator header files.
#
INCDIRS = $(wildcard $(SRCDIR)/*)
INCS = $(foreach d, $(INCDIRS), -I$d)

#
# Set the compiler and initial compiler flags to be used to build build all the
# object and executable files.
#
CC = gcc
CFLAGS = $(INCS) -std=c99 -Wall

#
# Debug adds to the compiler flags and then compiles everything that has not
# already been compiled.
#
debug: CFLAGS += -g -O0 -DDEBUG
debug: all

#
# Build everything.  Objects, test executables, and emulator executables.
#
all: $(OBJS) $(TSTS) $(PROGS)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(EXEDIR)%$(EXE) : $(TSTDIR)%.c $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $<

$(OBJDIR)%.o : $(SRCDIR)*/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

#
# Clean up everything (equvalent to what the 'all' target created, plus
# restoring the Makefile that had been updated by a "make depend" command).
#
clean: straighten
ifneq ($(wildcard $(BLDDIR)*.bak),)
	rm -f $(BLDDIR)Makefile
	mv -f $(BLDDIR)Makefile.bak $(BLDDIR)Makefile
endif

#
# Clean up everything (equvalent to what the 'all' target created, only).
#
straighten:
	rm -f $(OBJDIR)* $(EXEDIR)*

#
# Make all the dependencies.
#
depend:
	makedepend -p$(OBJDIR) -I/usr/lib/gcc/x86_64-pc-cygwin/6.4.0/include -- $(CFLAGS) -- $(SRCS) $(TSTFILES) $(PROGS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
