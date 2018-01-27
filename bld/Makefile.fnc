#
# Copyright (C) Jonathan D. Belanger 2018.
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
#		This make file contains the user defined functions used to assist in
#       building the DECaxp emulator.
#
#	Revision History:
#
#	V01.000		27-Jun-2017	Jonathan D. Belanger
#	Initially written.
#

#
# UPPER
#   This function is called to upcase a string.
#
# Input Parameters:
#   1:  String to be upcased
#
# Return Values:
#   The upcased string
#
define UPPER =
    $(subst a, A, $(subst b, B, $(subst c, C, $(subst d, D, $(subst e, E,       \
        $(subst f, F, $(subst g, G, $(subst h, H, $(subst i, I, $(subst j, J,   \
        $(subst k, K, $(subst l, L, $(subst m, M, $(subst n, N, $(subst o, O,   \
        $(subst p, P, $(subst q, Q, $(subst r, R, $(subst s, S, $(subst t, T,   \
        $(subst u, U, $(subst v, V, $(subst w, W, $(subst x, X, $(subst y, Y,   \
        $(subst z, Z, $(1)))))))))))))))))))))))))))
endef

#
# LOWER
#   This function is called to downcase a stringX
#
# Input Parameters:
#   1:  String to be downcased
#
# Return Values:
#   The downcased string
#
define LOWER =
    $(subst A, a, $(subst B, b, $(subst C, c, $(subst D, d, $(subst E, e,       \
        $(subst F, f, $(subst G, g, $(subst H, h, $(subst I, i, $(subst J, j,   \
        $(subst K, k, $(subst L, l, $(subst M, m, $(subst N, n, $(subst O, o,   \
        $(subst P, p, $(subst Q, q, $(subst R, r, $(subst S, s, $(subst T, t,   \
        $(subst U, u, $(subst V, v, $(subst W, w, $(subst X, x, $(subst Y, y,   \
        $(subst Z, z, $(1)))))))))))))))))))))))))))
endef

#
# PARSE_FILE
#   This function is called to return a requested part of a file specification.
#
# Input Parameters:
#   1:  The file specification to be parsed.
#   2:  The part of the specification being requested.  This parameter can be
#       one of the following:
#           DIR     - Return the directory part of the file specification
#           NODIR   - Return the filename with type of the file specification
#           NAME    - Return the filename part of the file specification
#           TYPE    - Return the file type part of the file specification
#
# Return Values:
#   The requested part of the file specification
#
define PARSE_FILE =
    ifeq ($(2), DIR))
	    $(dir $(1)))
    else ($(2), NODIR)
        $(notdir $(1))
    else ($(2), NAME)
        $(basename $(notdir $(1)))
    else ($(2), TYPE)
        $(suffix $(1))
endef

#
# CVT_FILESPEC
#   This function is called with a file specification and returns an updated
#   which has had the file prefix and suffix replaced with the specified
#   values
#
# Input Parameters:
#   1:  The file specification to be converted.
#   2:  The prefix to replace the one in the first parameter.
#   3:  The suffix of replace the one in the first parameter.
#
# Return Values:
#   The converted file specification.
#
define CVT_FILESPEC =
    $(addprefix $(2), $(addsuffix $(3), $(eval $(call PARSE_FILE, $(1)))))
endef

#
# MAKE_ARCFN
#   This function is called to take a directory, name, and type to generate an
#   archive filename.
#
# Input Parameters:
#   1:  The name to be used to generate the archive filename.
#
# Return values:
#   A filename in the form of <lib-dir>lib<name><arctype>
#
define MAKE_ARCNAME =
    $(LIBDIR)lib$(1)$(AFILE)
endef

#
# MAKE_OBJFN
#   This function is called to take a source file and make an object file
#   specification.
#
# Input Parameters:
#   1:  The source file specification.
#
# Return Values:
#   A file specification in the form of <obj-dir><src-filename><obj-file-type>
#
define MAKE_OBJFN =
    $(eval $(call CVT_FILESPEC, $(1), $(OJDIR), $(OFILE)))
endef

#
# SRC_TEMPLATE
#   This function is called to generate a template statement that indicates
#   the object file that is generated from the source file.
#
# Input Parameters:
#   1:  The source file specification
#
# Return Values:
#   A template in the form of: <object-file>: <source-file>
#
define SRC_TEMPLATE =
    $(eval $(call MAKE_OBJFN, $(1))): $(1)
endef

#
# ARC_TEMPLATE
#   This function is called to generate a template statement that indicates
#   the archive and member that is generated from the source file.
#
# Input Parameters:
#   1:  The source file specification
#
# Return Values:
#   A template in the form of: <archive>(<object-file>): <object-spec>
#
define ARC_TEMPLATE =
    $(eval $(call MAKE_ARCFILE, $(1)))($(notdir $(eval $(call MAKE_OBJFN, $(2))))):  \
        $(eval $(call MAKE_OBJFN, $(2)))
endef

#
# EXE_TEMPLATE
#   This function is called to generate a template statement that indicates
#   the executable that is generated from the source file.
#
# Input Parameters:
#   1:  The source file specification that have the executables main function.
#
# Return Values:
#   A template in the form of: <exe-dir><exe-file>: <source-spec> <archive-list>
#
define EXE_TEMPLATE =
    $(eval $(call CVT_FILESPEC, $(1), $(EXEDIR), $(EFILE))): $(1) $(ARCS)
endef
