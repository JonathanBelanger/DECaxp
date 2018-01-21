/*
 * Copyright (C) Jonathan D. Belanger 2018.
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied only
 * in accordance with the terms of such license and with the inclusion of the
 * above copyright notice.  This software or any other copies thereof may not
 * be provided or otherwise made available to any other person.  No title to
 * and ownership of the software is hereby transferred.
 *
 * The information in this software is subject to change without notice and
 * should not be construed as a commitment by the author or co-authors.
 *
 * The author and any co-authors assume no responsibility for the use or
 * reliability of this software.
 *
 * Description:
 *
 *	This header file contains definitions normally provided by various system
 *	header files.  When editing on my laptop, there is no compiler installed.
 *	This header file defines a series of items to keep the editor's code
 *	analysis happy.
 *
 *	Revision History:
 *
 *	V01.000		10-May-2017	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_NO_COMPILER_HACK_
#define _AXP_NO_COMPILER_HACK_	1

#if 0

/*
 * The following definitions are here to keep eclipse happy on my laptop, that
 * does not have gcc on it.
 */
#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#ifndef NULL
#define NULL (void *) 0
#endif

#ifndef bool
typedef _Bool bool;
#endif

#ifndef __int128
#define __int128 long long int
#endif

#ifndef FILE
typedef void * FILE;
#define EOF -1
#endif

#ifndef pthread_cond_t
typedef int pthread_cond_t;
typedef int pthread_mutex_t;
typedef int pthread_t;
#endif

#ifndef FP_SUBNORMAL
#define FP_SUBNORMAL	-1
#define FE_OVERFLOW		-2
#define FE_UNDERFLOW	-3
#define FE_INVALID		-4
#define FE_TOWARDZERO	1
#define FE_DOWNWARD		2
#define FE_TONEAREST	3
#define FE_UPWARD		4
#define FE_DIVBYZERO	5
#define FE_INEXACT		6
#define FE_ALL_EXCEPT	0
#endif

#ifndef size_t
typedef int size_t;
#endif

#ifndef stderr
#define stderr	1
#endif
#endif
#endif	/* _AXP_NO_COMPILER_HACK_ */
