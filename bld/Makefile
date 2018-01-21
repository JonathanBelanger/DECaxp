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
#	V01.004		17-Nov-2017	Jonathan D. Belanger
#	Updated the compiler switches to include -pthread, because we are making
#	this program multithreaded.
#
#	V01.005		19-Nov-2017	Jonathan D. Belanger
#	Moved the test folder (tst) into the src folder.
#
#	V01.006		30-Dec-2017	Jonathan D. Belanger
#	What was here before looked like it was working, but it was doing so in
#	error.  I made a bunch of clean ups and made this code a bit more
#	resilient.
#
#	V01.007		31-Dec-2017	Jonathan D. Belanger
#	Made this Makefile much more dynamic.  You'll notice that there are not too
#	many explicit filenames in here.  They are generated as part of the depend
#	target.  At some point I may make this Makefile more self-aware, where it
#	will rebuild itself if any of the source files changed, including the
#	Makefile itself.
#
PROJDIR = /cygdrive/g/git/DECaxp/

#
# Create variables for the various top-level directories we care about.
#
COMDIR = $(PROJDIR)src/comutl/
CPUDIR = $(PROJDIR)src/cpu/
SYSDIR = $(PROJDIR)src/system/
SRCDIR = $(COMDIR) $(CPUDIR) $(SYSDIR)
TSTDIR = $(PROJDIR)src/tst/
OBJDIR = $(PROJDIR)bin/
EXEDIR = $(PROJDIR)exe/
BLDDIR = $(PROJDIR)bld/

#
# Build file information, so that we can build from any directory and not have
# to worry where files get written.
#
SELF = $(BLDDIR)Makefile
DEP = $(addsuffix .dep, $(SELF))
BAK = $(addsuffix .bak, $(SELF))

#
# Create variables for the .c source code files for the emulator and the object
# files that are created from compiling.  This definition will look in all
# sub-directories under the top-level source directory.
#
CFILES = $(foreach DIR, $(SRCDIR), $(wildcard $(DIR)*.c))
SRCS = $(CFILES)
OBJS = $(addprefix $(OBJDIR), $(notdir $(CFILES:.c=.o)))

#
# Create variables for the .c source code files and the object files that are
# created from compiling.
#
TSTFILES = $(wildcard $(TSTDIR)*.c)
TSTS = $(TSTFILES)
EXE = .exe
EXES = $(addprefix $(EXEDIR), $(notdir $(TSTFILES:.c=$(EXE))))

#
# Create the variable for the emulator source files (the emulator itself and
# any companion programs - ie: configuration programs, if any).
#
PROGS =

#
# Create variables for the include path for all the emulator header files.
#
INCDIRS = $(wildcard $(SRCDIR))
INCS = $(foreach d, $(INCDIRS), -I$d)

#
# Set the compiler and initial compiler flags to be used to build build all the
# object and executable files.
#
CC = gcc
CFLAGS = $(INCS) -m64 -std=gnu99 -Wall -pthread

#
# Debug adds to the compiler flags and then compiles everything that has not
# already been compiled.
#
debug: CFLAGS += -g -O0 -DDEBUG
debug: all

#
# Build everything.  Objects, test executables, and emulator executables.
#
all: tests progs

#
# The Test executables require the objects and the test source code.
#
tests:	$(OBJS) $(EXES)

#
# The Program executeable(s) require the objects and the program source
# file(s).
#
progs: $(OBJS) $(PROGS)

#
# The Object files require the component source files.  The objects are written
# to the obj folder and there may be multiple source folders.
#
$(OBJDIR)%.o: $(SRCS)
	$(eval SRC = $(findstring $(COMDIR)$(*F).c, $(SRCS)))
	$(eval SRC1 = $(findstring $(CPUDIR)$(*F).c, $(SRCS)))
	$(eval SRC2 = $(findstring $(SYSDIR)$(*F).c, $(SRCS)))
	$(eval $(strip SRC += $(SRC1)))
	$(eval $(strip SRC += $(SRC2)))
	$(CC) -c $(CFLAGS) -o $@ $(SRC)

#
# The Tests require the objects and test source files.  The executables are
# written to the exe folder and there may be multiple source folders.
#
$(EXEDIR)%.exe: $(TSTS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $<

.PHONY: clean

#
# Clean up everything (equvalent to what the 'all' target created, plus
# restoring the Makefile that had been updated by a "make depend" command).
#
clean: straighten
ifneq ($(wildcard $(BLDDIR)*.bak),)
	rm -f $(SELF)
	mv -f $(BAK) $(SELF)
endif
ifneq ($(wildcard $(BLDDIR)*.dep),)
	rm -f $(DEP)
endif

#
# Clean up everything (equvalent to what the 'all' target created, only).
#
straighten:
	rm -f $(OBJDIR)* $(EXEDIR)*

#
# Make all the dependencies.
#
depend: $(DEP)
	makedepend -f$(SELF)								\
		-p$(OBJDIR) 									\
		-I/usr/lib/gcc/x86_64-pc-cygwin/6.4.0/include	\
		-- $(CFLAGS) -- $(SRCS) $(TSTFILES) $(PROGS)

$(DEP): $(SRCS) $(TSTS)
	$(file >$(DEP))
	$(foreach _,										\
		$(SRCS), 										\
		$(file >>$(DEP),$(addprefix $(OBJDIR),			\
				$(addsuffix .o,							\
					$(basename 							\
						$(notdir $(_))))): $(_)))
	$(foreach _,										\
		$(TSTS), 										\
		$(file >>$(DEP),$(addprefix $(EXEDIR),			\
				$(addsuffix .exe,						\
					$(basename 							\
						$(notdir $(_))))): $(_)))

include $(DEP)

# DO NOT DELETE THIS LINE -- make depend depends on it.
