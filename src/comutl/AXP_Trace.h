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
 *	This header file defines the prototypes and macros in support of the
 *	tracing functions.
 *
 * Revision History:
 *
 *	V01.000		27-Jam-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef AXP_TRACE_H_
#define AXP_TRACE_H_

#include "AXP_Utility.h"

extern bool	_axp_trc_active_;

/*
 * A macros to wrap around trace statements.
 */
#define AXP_TRACE_BEGIN()	if (_axp_trc_active_) { AXP_TraceLock();
#define AXP_TRACE_END()		AXP_TraceUnlock(); }

/*
 * Let's defined a DEBUG environment variable that will turn on certain
 * tracing options.
 *
 * BTW: The environment variable name and the typedef defined below is a
 *		homage to my Digital SNA Development days (June 16, 1986 to December
 *		31, 1994).
 */
typedef u32				AXP_TRCLOG;
extern AXP_TRCLOG		_axp_trc_log_;

/*
 * These bits are used in each of the nibbles within the AXP_LOGMASK
 * environment variable to trace different items.
 */
#define AXP_TRC_CALL	0x001	/* b0000 0000 0001 */
#define AXP_TRC_BUFF	0x002	/* b0000 0000 0010 */
#define AXP_TRC_OPT1	0x004	/* b0000 0000 0100 */
#define AXP_TRC_OPT2	0x008	/* b0000 0000 1000 */

/*
 * These are bits used by the CPU in subsequent nibbles to trace various
 * aspects of the CPU.
 */
#define AXP_TRC_INST	0x010	/* b0000 0001 0000 */
#define AXP_TRC_CACHE	0x040	/* b0000 0100 0000 */
#define AXP_TRC_IBOX	0x080	/* b0000 1000 0000 */
#define AXP_TRC_EBOX	0x100	/* b0001 0000 0000 */
#define AXP_TRC_FBOX	0x200	/* b0010 0000 0000 */
#define AXP_TRC_MBOX	0x400	/* b0100 0000 0000 */
#define AXP_TRC_CBOX	0x800	/* b1000 0000 0000 */

/*
 * These bytes within the AXP_LOGMASK environment variable are used in the
 * emulation code to trace different parts.
 */
#define AXP_COMP_UTL	0x0000000f	/* COMUTL */
#define AXP_SHIFT_UTL	0			/* bits to right shift */
#define AXP_COMP_CPU	0x0000fff0	/* CPU */
#define AXP_SHIFT_CPU	4			/* bits to right shift */
#define AXP_COMP_SYS	0x0fff0000	/* System */
#define AXP_SHIFT_SYS	16			/* bits to right shift */

#define AXP_TRCLOG_INIT	((_axp_trc_active_ == false) ? AXP_TraceInit() : true)

/*
 * These macros return true when a type of tracing is to be performed.
 */
#define AXP_UTL_CALL	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_UTL) >> AXP_SHIFT_UTL) &	\
						    AXP_TRC_CALL) == AXP_TRC_CALL))
#define AXP_UTL_BUFF	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_UTL) >> AXP_SHIFT_UTL) &	\
						   AXP_TRC_BUFF) == AXP_TRC_BUFF))
#define AXP_UTL_OPT1	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_UTL) >> AXP_SHIFT_UTL) &	\
						   AXP_TRC_OPT1) == AXP_TRC_OPT1))
#define AXP_UTL_OPT2	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_UTL) >> AXP_SHIFT_UTL) &	\
						   AXP_TRC_OPT2) == AXP_TRC_OPT2))
#define AXP_IBOX_CALL	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_IBOX | AXP_TRC_CALL)) != 0))
#define AXP_IBOX_BUFF	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_IBOX | AXP_TRC_BUFF)) != 0))
#define AXP_IBOX_OPT1	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_IBOX | AXP_TRC_OPT1)) != 0))
#define AXP_IBOX_OPT2	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_IBOX | AXP_TRC_OPT2)) != 0))
#define AXP_IBOX_INST	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_IBOX | AXP_TRC_INST)) != 0))
#define AXP_EBOX_CALL	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_EBOX | AXP_TRC_CALL)) != 0))
#define AXP_EBOX_BUFF	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_EBOX | AXP_TRC_BUFF)) != 0))
#define AXP_EBOX_OPT1	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_EBOX | AXP_TRC_OPT1)) != 0))
#define AXP_EBOX_OPT2	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_EBOX | AXP_TRC_OPT2)) != 0))
#define AXP_FBOX_CALL	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_FBOX | AXP_TRC_CALL)) != 0))
#define AXP_FBOX_BUFF	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_FBOX | AXP_TRC_BUFF)) != 0))
#define AXP_FBOX_OPT1	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_FBOX | AXP_TRC_OPT1)) != 0))
#define AXP_FBOX_OPT2	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_FBOX | AXP_TRC_OPT2)) != 0))
#define AXP_MBOX_CALL	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_MBOX | AXP_TRC_CALL)) != 0))
#define AXP_MBOX_BUFF	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_MBOX | AXP_TRC_BUFF)) != 0))
#define AXP_MBOX_OPT1	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_MBOX | AXP_TRC_OPT1)) != 0))
#define AXP_MBOX_OPT2	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_MBOX | AXP_TRC_OPT2)) != 0))
#define AXP_CBOX_CALL	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_CBOX | AXP_TRC_CALL)) != 0))
#define AXP_CBOX_BUFF	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_CBOX | AXP_TRC_BUFF)) != 0))
#define AXP_CBOX_OPT1	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_CBOX | AXP_TRC_OPT1)) != 0))
#define AXP_CBOX_OPT2	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_CBOX | AXP_TRC_OPT2)) != 0))
#define AXP_CBOX_INST	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_CBOX | AXP_TRC_INST)) != 0))
#define AXP_CACHE_CALL	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_CACHE | AXP_TRC_CALL)) != 0))
#define AXP_CACHE_BUFF	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_CACHE | AXP_TRC_BUFF)) != 0))
#define AXP_CACHE_OPT1	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_CACHE | AXP_TRC_OPT1)) != 0))
#define AXP_CACHE_OPT2	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_CPU) >> AXP_SHIFT_CPU) &	\
						   (AXP_TRC_CACHE | AXP_TRC_OPT2)) != 0))
#define AXP_SYS_CALL	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_SYS) >> AXP_SHIFT_SYS) & 	\
						   AXP_TRC_CALL) == AXP_TRC_CALL))
#define AXP_SYS_BUFF	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_SYS) >> AXP_SHIFT_SYS) & 	\
						   AXP_TRC_BUFF) == AXP_TRC_BUFF))
#define AXP_SYS_OPT1	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_SYS) >> AXP_SHIFT_SYS) & 	\
						   AXP_TRC_OPT1) == AXP_TRC_OPT1))
#define AXP_SYS_OPT2	(AXP_TRCLOG_INIT &										\
						 ((((_axp_trc_log_ & AXP_COMP_SYS) >> AXP_SHIFT_SYS) & 	\
						   AXP_TRC_OPT2) == AXP_TRC_OPT2))

/*
 * Function Prototypes
 */
bool AXP_TraceInit(void);
void AXP_TraceEnd(void);
void AXP_TraceWrite(char *, ...);
void AXP_TraceLock(void);
void AXP_TraceUnlock(void);

#endif /* AXP_TRACE_H_ */
