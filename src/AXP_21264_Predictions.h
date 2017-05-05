/*
 * Copyright (C) Jonathan D. Belanger 2017.
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
 *	This header file contains the structures and definitions required to
 *	implement the branch prediction as part of the emulation for the Alpha
 *	21264 (EV68) processor.
 *
 *	Revision History:
 *
 *	V01.000	 05-May-2017	Jonathan D. Belanger
 *	Initially written.
 *	(Happy Cinco Daimio)
 *
 */
#ifndef _AXP_21264_PRED_DEFS_
#define _AXP_21264_PRED_DEFS_

/*
 * The following definitions are used to decode the Virtual Program Counter
 * (VPC).
 */
typedef struct
{
	u16	ign1 : 2;
	u16	index : 10;
	u16 ign2 : 4;
	u16 ign3;
	u32 ign4;
} LCLindex;
typedef union
{
	u64 vpc;
	LCLindex index;
} LPTIndex;

/*
 *
 */
#define AXP_MASK_10_BITS			0x03ff
#define AXP_MASK_12_BITS			0x0fff

/*
 * States for the 2-bit Saturation Counter
 */
#define AXP_2BIT_STRONGLY_NOT_TAKEN	0
#define AXP_2BIT_WEAKLY_NOT_TAKEN	1
#define AXP_2BIT_WEAKLY_TAKEN		2
#define AXP_2BIT_STRONGLY_TAKEN		3
#define AXP_2BIT_MAX_VALUE			3
#define AXP_2BIT_TAKEN_MIN			2

/*
 * States for the 3-bit Saturation Counter
 */
#define AXP_3BIT_HIGHLY_NOT_TAKEN	0
#define AXP_3BIT_MOSTLY_NOT_TAKEN	1
#define AXP_3BIT_USUALLY_NOT_TAKEN	2
#define AXP_3BIT_FAVORS_NOT_TAKEN	3
#define AXP_3BIT_FAVORS_TAKEN		4
#define AXP_3BIT_USUALLY_TAKEN		5
#define AXP_3BIT_MOSTLY_TAKEN		6
#define AXP_3BIT_HIGHLY_TAKEN		7
#define AXP_3BIT_MAX_VALUE			7
#define AXP_3BIT_NOT_TAKEN_MAX		3
#define AXP_3BIT_TAKEN_MIN			4

/*
 * Define the table definition for the 2-bit saturation counters for the
 * Global Prediction.
 */
#define AXP_GLOBAL_PREDICTOR_TABLE_SIZE 4096
typedef struct
{
	u8	gbl_pred[AXP_GLOBAL_PREDICTOR_TABLE_SIZE];
} GPT;

/*
 * Define the table definition for the 3-bit saturation counters for the
 * Local Prediction.
 */
#define AXP_LOCAL_PREDICTOR_TABLE_SIZE 1024
typedef struct
{
	u16	lcl_history[AXP_LOCAL_PREDICTOR_TABLE_SIZE];
} LHT;
typedef struct
{
	u8	lcl_pred[AXP_LOCAL_PREDICTOR_TABLE_SIZE];
} LPT;

/*
 * Define the table definition for the 2-bit saturation counters for the
 * Choice Prediction.
 *
 * The way this table work is, if the local and global predictions do not
 * match, if the local prediction was correct, then we decrement the 2-bit
 * saturation counter.  Otherwise, we increment the 2-bit saturation counter.
 * If the local and golbal predictions match, we leave the counter alone.
 */
#define AXP_CHOICE_PREDICTOR_TABLE_SIZE 4096
typedef struct
{
	u8	choice_pred[AXP_CHOICE_PREDICTOR_TABLE_SIZE];
} CPT;

/*
 * Macros for incrementing, decrementing, and determining whether to predict
 * to take the branch or not for 2-bit saturation counters.
 *
 * The INCR is called when a branch is actually taken.
 * The DECR is called when a branch is not actually taken.
 */
#define AXP_2BIT_INCR(cntr)	if ((cntr) < AXP_2BIT_MAX_VALUE) (cntr)++
#define AXP_2BIT_DECR(cntr)	if ((cntr) > 0) (cntr)--
#define AXP_2BIT_TAKE(cntr)	(((cntr) < AXP_2BIT_TAKEN_MIN) ? false : true)

/*
 * Macros for incrementing, decrementing, and determining whether to predict
 * to take the branch or not for 3-bit saturation counters.
 *
 * The INCR is called when a branch is actually taken.
 * The DECR is called when a branch is not actually taken.
 *
 * NOTE:	We do the following, when incrementing and decrementing, to be able
 *			to provide a bit of a historesis, to precent alternating between
 *			the FAVORS_NOT_TAKEN and FAVORS_TAKEN states.  Alternating between
 *			taken and not taken will still properly predict the branch 50% of
 *			the time.
 */
#define AXP_3BIT_INCR(cntr)\
	if ((cntr) < AXP_3BIT_MAX_VALUE)\
	{\
		if ((cntr) == AXP_3BIT_NOT_TAKEN_MAX)\
			(cntr) = AXP_3BIT_MAX_VALUE;\
		else\
			(cntr)++;\
	}
#define AXP_3BIT_DECR(cntr)\
	if ((cntr) > 0)\
	{\
		if ((cntr) == AXP_3BIT_TAKEN_MIN)\
			(cntr) = 0;\
		else\
			(cntr)--;\
	}
#define AXP_3BIT_TAKE(cntr)	(((cntr) < AXP_3BIT_TAKE_MIN) ? false : true)

/*
 * The following macros are to maintain the Local History Table and the Global
 * History Path.
 */
#define AXP_LOCAL_PATH_TAKEN(lpte)		(lpte) = (((lpte) * 2) + 1) & AXP_MASK_10_BITS
#define AXP_LOCAL_PATH_NOT_TAKEN(lpte)	(lpte) = ((lpte) * 2) & AXP_MASK_10_BITS
#define AXP_GLOBAL_PATH_TAKEN(gph)		(gph) = (((gph) * 2) + 1) & AXP_MASK_12_BITS
#define AXP_GLOBAL_PATH_NOT_TAKEN(gph)	(gph) = ((gph) * 2) & AXP_MASK_12_BITS

#endif /* _AXP_21264_PRED_DEFS_ */