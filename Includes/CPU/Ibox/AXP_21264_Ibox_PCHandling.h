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
 *	This header file contains the function prototypes for the PC handling
 *	functionality of the Ibox.
 *
 * Revision History:
 *
 *	V01.000		15-Jan-2018	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		20-Jan-2018	Jonathan D. Belanger
 *	The AXP_21264_GetPALBaseVPC function was not being called.  The
 *	functionality in this function was somewhat redundant with the
 *	AXP_21264_GetPALFuncVPC function.  So, the former was removed.
 */
#ifndef _AXP_IBOX_PC_HANDLING_DEFS_
#define _AXP_IBOX_PC_HANDLING_DEFS_		1

void AXP_21264_AddVPC(AXP_21264_CPU *, AXP_PC);
AXP_PC AXP_21264_GetPALFuncVPC(AXP_21264_CPU *, u32);
AXP_PC AXP_21264_MakeVPC(AXP_21264_CPU *, u64, u8);
AXP_PC AXP_21264_GetNextVPC(AXP_21264_CPU *);
AXP_PC AXP_21264_IncrementVPC(AXP_21264_CPU *);
AXP_PC AXP_21264_DisplaceVPC(AXP_21264_CPU *, AXP_PC, i64);

#endif	/* _AXP_IBOX_PC_HANDLING_DEFS_ */
