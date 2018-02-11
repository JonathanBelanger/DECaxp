#
# Copyright (C) Jonathan D. Belanger 2017-2018.
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
#		This make file is used to build the executables for the DECaxp project.
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
#	V01.008		21-Jan-2018	Jonathan D. Belanger
#	Updated the include list for the maedepend statement.
#
#	V01.009		25-Jan-2018	Jonathan D. Belanger
#	This makefile requires a bit more restructuring.  It worked in the simpler
#	case, where we only had one set of executeables.  It now needs to be
#	broken down into steps.  The steps will be as follows:
#		1) .c --> .o	(in comutl, cpu, and system folders)
#		2) .o --> .a	(libcomutl.a, libcpu.a, libsystem.a)
#		3) .c --> .exe	(in tst and app folders)
#

#
# Useful variables
#
OP := (
CP := )
CM := \#
define NL

endef

#
# THe below variable is used as the base folder for all other project folders.
#
PROJDIR = /cygdrive/g/git/DECaxp/

#
# Build file information, so that we can build from any folder and not have
# to worry where files get written.
#
BLDDIR = $(PROJDIR)bld/
SELF = $(BLDDIR)Makefile
FNC = $(addsuffix .fnc, $(SELF))
DEP = $(addsuffix .dep, $(SELF))
BAK = $(addsuffix .bak, $(SELF))

#
# File extensions for various input and output files
#
CFILE = .c
OFILE = .o
AFILE = .a
EFILE = .exe

#
# Component names.
#
COMUTL = comutl
CPU = cpu
SYS = system
COMPS = $(COMUTL) $(CPU) $(SYS)

#
# Executeable source folders
#
EXEC1 = tst
EXEC2 = app

#
# Create variables for the build and target folders.  The build folder is
# where this file is located.  The target folders are one for object files,
# one for library files, and one for executeable files.
#
OBJDIR = $(PROJDIR)bin/
EXEDIR = $(PROJDIR)exe/
LIBDIR = $(PROJDIR)lib/

#
# Create variables for the folders where component source files are located.
#
SRCDIR = $(PROJDIR)src/
COMDIR = $(SRCDIR)$(COMUTL)/
CPUDIR = $(SRCDIR)$(CPU)/
SYSDIR = $(SRCDIR)$(SYS)/

#
# Create variables for the folders where the source files that will be
# compiled to executeable files.
#
TSTDIR = $(SRCDIR)$(EXEC1)/
APPDIR = $(SRCDIR)$(EXEC2)/

#
# Create variables for the .c source code files for the emulator and the object
# files that are created from compiling.  This definition will look in all
# sub-directories under the top-level source directory.
#
COMSRCS = $(wildcard $(COMDIR)*$(CFILE))
COMOBJS = $(addprefix $(OBJDIR), $(notdir $(COMSRCS:$(CFILE)=$(OFILE))))
CPUSRCS = $(wildcard $(CPUDIR)*$(CFILE))
CPUOBJS = $(addprefix $(OBJDIR), $(notdir $(CPUSRCS:$(CFILE)=$(OFILE))))
SYSSRCS = $(wildcard $(SYSDIR)*$(CFILE))
SYSOBJS = $(addprefix $(SYSDIR), $(notdir $(SYSSRCS:$(CFILE)=$(OFILE))))
SRCS = $(COMSRCS) $(CPUSRCS) $(SYSSRCS)
OBJS = $(COMOBJS) $(CPUOBJS) $(SYSOBJS)

#
# Create variables fo the .a archive files that need to be created.
#
COMARC = $(addprefix $(LIBDIR)lib, $(addsuffix $(AFILE), $(COMUTL)))
CPUARC = $(addprefix $(LIBDIR)lib, $(addsuffix $(AFILE), $(CPU)))
SYSARC = $(addprefix $(LIBDIR)lib, $(addsuffix $(AFILE), $(SYS)))
ARCS = $(COMARC) $(CPUARC) $(SYSARC)

#
# Create variables for the .c source code files and the object files that are
# created from compiling.
#
TSTSRCS = $(wildcard $(TSTDIR)*$(CFILE))
TSTEXES = $(addprefix $(EXEDIR), $(notdir $(TSTSRCS:$(CFILE)=$(EFILE))))

#
# Create the variables for the emulator source files (the emulator itself and
# any companion programs - ie: configuration programs, if any).
#
APPSRCS = $(foreach DIR, $(APPDIR), $(wildcard $(DIR)*$(CFILE)))
APPEXES = $(addprefix $(EXEDIR), $(notdir $(APPSRCS:$(CFILE)=$(EFILE))))

#
# Create variables for the include path for all the emulator header files.
#
CDIRS = $(COMDIR) $(CPUDIR) $(SYSDIR)
INCDIRS = $(wildcard $(CDIRS))
INCS = $(foreach d, $(INCDIRS), -I$d) -I/usr/include/libxml2/

#
# Set the compiler and initial compiler flags to be used to build build all the
# object and executable files.
#
CC = gcc
CFLAGS = $(INCS) -m64 -std=gnu99 -Wall -pthread -D_AXP_COMPILER_PRESENT_

#
# Set the libraries the linker need to resolve references.
#
LFLAGS = -lxml2 -lm

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
# The Test executables require the test executables to be built.
#
tests:	$(TSTEXES)

#
# The Program executeable(s) require the objects and the program source
# file(s).
#
progs: $(APPEXES)

#
# Define the suffix rule to go from .c file to .o file
#
$(OBJDIR)%$(OFILE): $(COMDIR)%$(CFILE)
	$(CC) -c $(CFLAGS) -o $@ $<
$(OBJDIR)%$(OFILE): $(CPUDIR)%$(CFILE)
	$(CC) -c $(CFLAGS) -o $@ $<
$(OBJDIR)%$(OFILE): $(SYSDIR)%$(CFILE)
	$(CC) -c $(CFLAGS) -o $@ $<

#
# Define the suffix rule to go from .o file to .a file
#
$(LIBDIR)%$(AFILE): $(OBJDIR)%$(OFILE)
	$(AR) r $@ $<

#
# Define the suffix rule to go from .c and .a to .exe file
#
$(EXEDIR)%.exe: $(TSTDIR)%$(CFILE) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $< $(LFLAGS) 
$(EXEDIR)%.exe: $(APPDIR)%$(CFILE) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $< $(LFLAGS) 

.PHONY: clean purge

#
# Clean up everything (equvalent to what the 'all' target created, plus
# restoring the Makefile that had been updated by a "make depend" command).
#
clean: purge
ifneq ($(wildcard $(BLDDIR)*.bak),)
	rm -f $(SELF)
	mv -f $(BAK) $(SELF)
endif
ifneq ($(wildcard $(BLDDIR)*.dep),)
	rm -f $(DEP)
endif
ifneq ($(wildcard $(BLDDIR)*.stackdump),)
	rm -f $(BLDDIR)*.stackdump
endif

#
# Clean up everything (equvalent to what the 'all' target created, only).
#
purge:
	rm -f $(OBJDIR)* $(LIBDIR)* $(EXEDIR)*

#
# Make all the dependencies.  The dependencies include all the following:
#	1) #include files within each of the source files (makde with makedepend)
#	2) Each of the target component object files from component source files
#	3) Each of the target component archives from component object files
#	4) Each of the target executable file from the component archive files
#	5) Each of the target executable files from the equivalent source files
#
depend: $(DEP)
	makedepend -f$(SELF)	\
		-p$(OBJDIR)	\
		-I /usr/include/libxml2/	\
		-I /usr/lib/gcc/x86_64-pc-cygwin/6.4.0/include/	\
		-- $(CFLAGS) -- $(SRCS) $(TSTSRCS) $(APPSRCS)

#
# The following code makes all the dependencies for the following:
#	1) obj\*.o: src\*\*.c		- for each component
#	2) lib\lib*.a: obj\*.o		- for each component and component object file
#	3) exe\*.exe: *\*.c lib\*.a	- for each executable source and all archives
#
$(DEP):
	$(file >$(DEP),$(CM))
	$(file >>$(DEP),$(CM) All Source Files to Object Files.)
	$(file >>$(DEP),$(CM))
	$(foreach _, $(SRCS),$(file >>$(DEP),$(OBJDIR)$(basename $(notdir $(_)))$(OFILE): $(_)))
	$(file >>$(DEP),$(NL))
	$(file >>$(DEP),$(CM))
	$(file >>$(DEP),$(CM) Test Source Files to executabled.)
	$(file >>$(DEP),$(CM))
	$(foreach _, $(TSTSRCS), 	\
		$(file >>$(DEP),$(EXEDIR)$(basename $(notdir $(_)))$(EFILE): $(_) $(OBJS)))
	$(file >>$(DEP),$(NL))
	$(file >>$(DEP),$(CM))
	$(file >>$(DEP),$(CM) Program Source Files to executabled.)
	$(file >>$(DEP),$(CM))
	$(foreach _, $(APPSRCS), 	\
		$(file >>$(DEP),$(EXEDIR)$(basename $(notdir $(_)))$(EFILE): $(_) $(OBJS)))

include $(DEP)

# DO NOT DELETE THIS LINE -- make depend depends on it.
