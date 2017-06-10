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
PROJDIR = /cygdrive/g/git/DECemu/

SRCDIR = $(PROJDIR)src/
TSTDIR = $(PROJDIR)tst/
OBJDIR = $(PROJDIR)bin/
EXEDIR = $(PROJDIR)bin/

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
CFLAGS = $(INCS) -Wall -Werror -Wextra

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

depend:
	makedepend -- $(CFLAGS) -- $(SRCS) $(TSTFILES) $(PROGS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Utility.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/stdio.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/_ansi.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/newlib.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/_newlib_version.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/config.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/machine/ieeefp.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/features.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/cdefs.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/machine/_default_types.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/reent.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/_types.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/machine/_types.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/lock.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/types.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/_stdint.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/machine/endian.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/machine/_endian.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/bits/endian.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/select.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/_sigset.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/_timeval.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/timespec.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/_timespec.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/_pthreadtypes.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/machine/types.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/endian.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/bits/byteswap.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/bits/wordsize.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/sysmacros.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/stdio.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/string.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/xlocale.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/sys/string.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/stdlib.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/machine/stdlib.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Utility.o: /usr/include/alloca.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Utility.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/stdio.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/_ansi.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/newlib.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/_newlib_version.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/config.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/machine/ieeefp.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/features.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/cdefs.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/machine/_default_types.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/reent.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/_types.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/machine/_types.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/lock.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/types.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/_stdint.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/machine/endian.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/machine/_endian.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/bits/endian.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/select.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/_sigset.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/_timeval.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/timespec.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/_timespec.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/_pthreadtypes.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/machine/types.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/endian.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/bits/byteswap.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/bits/wordsize.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/sysmacros.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/stdio.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/string.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/xlocale.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/sys/string.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/stdlib.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/machine/stdlib.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /usr/include/alloca.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_CPU.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /cygdrive/g/git/DECemu/src/cpu/AXP_Base_CPU.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_IPRs.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Predictions.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Instructions.h
/cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_ICache.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_CPU.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Utility.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/stdio.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/_ansi.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/newlib.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/_newlib_version.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/config.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/machine/ieeefp.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/features.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/cdefs.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/machine/_default_types.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/reent.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/_types.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/machine/_types.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/lock.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/types.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/_stdint.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/machine/endian.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/machine/_endian.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/bits/endian.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/select.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/_sigset.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/_timeval.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/timespec.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/_timespec.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/_pthreadtypes.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/machine/types.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/endian.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/bits/byteswap.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/bits/wordsize.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/sysmacros.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/stdio.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/string.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/xlocale.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/sys/string.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/stdlib.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/machine/stdlib.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /usr/include/alloca.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_Base_CPU.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_IPRs.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Predictions.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Instructions.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_ICache.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Utility.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/stdio.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/_ansi.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/newlib.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/_newlib_version.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/config.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/machine/ieeefp.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/features.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/cdefs.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/machine/_default_types.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/reent.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/_types.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/machine/_types.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/lock.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/types.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/_stdint.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/machine/endian.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/machine/_endian.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/bits/endian.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/select.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/_sigset.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/_timeval.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/timespec.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/_timespec.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/_pthreadtypes.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/machine/types.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/endian.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/bits/byteswap.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/bits/wordsize.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/sysmacros.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/stdio.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/string.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/xlocale.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/sys/string.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/stdlib.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/machine/stdlib.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /usr/include/alloca.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_CPU.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_Base_CPU.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_IPRs.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Predictions.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Instructions.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_ICache.h
/cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Utility.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/stdio.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/_ansi.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/newlib.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/_newlib_version.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/config.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/machine/ieeefp.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/features.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/cdefs.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/machine/_default_types.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/reent.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/_types.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/machine/_types.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/lock.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/types.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/_stdint.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/machine/endian.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/machine/_endian.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/bits/endian.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/select.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/_sigset.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/_timeval.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/timespec.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/_timespec.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/_pthreadtypes.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/machine/types.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/endian.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/bits/byteswap.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/bits/wordsize.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/sysmacros.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/stdio.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/string.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/xlocale.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/sys/string.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/stdlib.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/machine/stdlib.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /usr/include/alloca.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_CPU.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_Base_CPU.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_IPRs.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Predictions.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Instructions.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_ICache.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Icache_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Utility.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/stdio.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/_ansi.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/newlib.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/_newlib_version.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/config.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/machine/ieeefp.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/features.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/cdefs.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/machine/_default_types.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/reent.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/_types.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/machine/_types.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/lock.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/types.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/_stdint.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/machine/endian.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/machine/_endian.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/bits/endian.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/select.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/_sigset.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/_timeval.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/timespec.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/_timespec.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/_pthreadtypes.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/machine/types.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/endian.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/bits/byteswap.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/bits/wordsize.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/sysmacros.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/stdio.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/string.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/xlocale.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/sys/string.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/stdlib.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/machine/stdlib.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /usr/include/alloca.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_CPU.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_Base_CPU.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_IPRs.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Predictions.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Instructions.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_ICache.h
/cygdrive/g/git/DECemu/tst/AXP_21264_Prediction_Test.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Configure.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Utility.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/stdio.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/_ansi.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/newlib.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/_newlib_version.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/config.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/machine/ieeefp.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/features.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/cdefs.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/machine/_default_types.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/reent.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/_types.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/machine/_types.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/lock.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/types.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/_stdint.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/machine/endian.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/machine/_endian.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/bits/endian.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/select.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/_sigset.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/_timeval.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/timespec.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/_timespec.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/_pthreadtypes.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/machine/types.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/endian.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/bits/byteswap.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/bits/wordsize.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/sysmacros.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/stdio.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/string.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/xlocale.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/sys/string.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/stdlib.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/machine/stdlib.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /usr/include/alloca.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /cygdrive/g/git/DECemu/src/comutl/AXP_Blocks.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Instructions.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /cygdrive/g/git/DECemu/src/cpu/AXP_Base_CPU.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_IPRs.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Predictions.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_ICache.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Ibox.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_CPU.h
/cygdrive/g/git/DECemu/tst/AXP_Test_Structure_Sizes.o: /cygdrive/g/git/DECemu/src/cpu/AXP_21264_Cbox.h
