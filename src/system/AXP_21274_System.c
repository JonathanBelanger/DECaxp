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
 *	This source file the System interface functions.
 *
 * Revision History:
 *
 *	V01.000		21-JAN-2018	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Configure.h"
#include "AXP_Utility.h"
#include "AXP_Blocks.h"
#include "AXP_21274_System.h"

AXP_21274_SYSTEM *AXP_21274_AllocateSystem(void)
{
	AXP_21274_SYSTEM *sys = NULL;
	int				pthreadRet;
	int				ii;
	bool			qRet = true;

	sys = AXP_Allocate_Block(AXP_21274_SYS_BLK);
	if (sys != NULL)
	{
		pthreadRet = pthread_mutex_init(&sys->cChipMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&sys->dChipMutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&sys->p0Mutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&sys->p1Mutex, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&sys->cChipCond, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&sys->dChipCond, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&sys->p0Cond, NULL);
		if (pthreadRet == 0)
			pthreadRet = pthread_mutex_init(&sys->p1Cond, NULL);
	}

	/*
	 * Return what we allocated back to the caller
	 */
	return(sys);
}
