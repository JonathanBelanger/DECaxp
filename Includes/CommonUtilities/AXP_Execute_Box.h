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
 *	This source file contains the functions needed to implement the instruction
 *	execution loop for both the Ebox and Fbox.
 *
 *	Revision History:
 *
 *	V01.000		26-June-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_EXECUTE_INS_BOX_
#define _AXP_EXECUTE_INS_BOX_

/*
 * Function prototype
 */
void AXP_Execution_Box(AXP_21264_CPU *, AXP_PIPELINE, AXP_COUNTED_QUEUE *,
        pthread_cond_t *, pthread_mutex_t *,
        void (*)(AXP_21264_CPU *, AXP_QUEUE_ENTRY *));

#endif	/* _AXP_EXECUTE_INS_BOX_ */
