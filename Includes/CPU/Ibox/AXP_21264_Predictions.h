/*
 * Copyright (C) Jonathan D. Belanger 2017-2019.
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
 *  This header file contains the structures and definitions required to
 *  implement the branch prediction as part of the emulation for the Alpha
 *  21264 (EV68) processor.
 *
 * Revision History:
 *
 *  V01.000        05-May-2017    Jonathan D. Belanger
 *  Initially written.
 *  (Happy Cinco Daimio)
 *
 *  V01.001        10-May-2017    Jonathan D. Belanger
 *  Included the AXP Utility header file for definitions like u64, i64, etc..
 *
 *  V01.002        14-May-2017    Jonathan D. Belanger
 *  Included the AXP_Base_CPU header file, which contains definitions common to
 *  all Alpha AXP CPUs.
 *
 *  V01.003     01-Jul-2019 Jonathan D. Belanger
 *  Change the 2- and 3-bit saturating counters to be determined and tested by
 *  using just bit math.  This is to avoid branch mispredict in the system on
 *  which the branch prediction emulation code is running.
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
    u64 res_1 :2;
    u64 index :10;
    u64 res_2 :52;
} LCLindex;

typedef union
{
    AXP_PC vpc;
    LCLindex index;
} LPTIndex;

/*
 *
 */
#define AXP_MASK_10_BITS            0x03ff
#define AXP_MASK_12_BITS            0x0fff

/*
 * States for the 2-bit Saturation Counter
 */
#define AXP_2BIT_STRONGLY_NOT_TAKEN    0
#define AXP_2BIT_WEAKLY_NOT_TAKEN    1
#define AXP_2BIT_WEAKLY_TAKEN        2
#define AXP_2BIT_STRONGLY_TAKEN        3
#define AXP_2BIT_MAX_VALUE            3
#define AXP_2BIT_TAKEN_MIN            2

/*
 * States for the 3-bit Saturation Counter
 */
#define AXP_3BIT_HIGHLY_NOT_TAKEN    0
#define AXP_3BIT_MOSTLY_NOT_TAKEN    1
#define AXP_3BIT_USUALLY_NOT_TAKEN    2
#define AXP_3BIT_FAVORS_NOT_TAKEN    3
#define AXP_3BIT_FAVORS_TAKEN        4
#define AXP_3BIT_USUALLY_TAKEN        5
#define AXP_3BIT_MOSTLY_TAKEN        6
#define AXP_3BIT_HIGHLY_TAKEN        7
#define AXP_3BIT_MAX_VALUE            7
#define AXP_3BIT_NOT_TAKEN_MAX        3
#define AXP_3BIT_TAKEN_MIN            4

/*
 * Define the 2- and 3-bit saturation counter structures.
 */
typedef union
{
    struct
    {
        u8 b : 1;
        u8 a : 1;
        u8 res : 6;
    };
    u8 cnt;
} AXP_2BIT_SAT_CNT;
typedef union
{
    struct
    {
        u8 c : 1;
        u8 b : 1;
        u8 a : 1;
        u8 res : 5;
    };
    u8 cnt;
} AXP_3BIT_SAT_CNT;

/*
 * Define the table definition for the 2-bit saturation counters for the
 * Global Prediction.
 */
typedef struct
{
    AXP_2BIT_SAT_CNT gbl_pred[FOUR_K];
} GPT;

/*
 * Define the table definition for the 3-bit saturation counters for the
 * Local Prediction.
 */
typedef struct
{
    AXP_3BIT_SAT_CNT lcl_history[ONE_K];
} LHT;
typedef struct
{
    AXP_3BIT_SAT_CNT lcl_pred[ONE_K];
} LPT;

/*
 * Define the table definition for the 2-bit saturation counters for the
 * Choice Prediction.
 *
 * The way this table work is, if the local and global predictions do not
 * match, if the local prediction was correct, then we decrement the 2-bit
 * saturation counter.  Otherwise, we increment the 2-bit saturation counter.
 * If the local and global predictions match, we leave the counter alone.
 */
typedef struct
{
    AXP_2BIT_SAT_CNT choice_pred[FOUR_K];
} CPT;

/*
 * Macros for incrementing, decrementing, and determining whether to predict
 * to take the branch or not for 2-bit saturation counters.
 *
 * The INCR is called when a branch is actually taken.
 * The DECR is called when a branch is not actually taken.
 */
#define AXP_2BIT_INCR(cntr)                                                 \
    {                                                                       \
        AXP_2BIT_SAT_CNT tmp = cntr;                                        \
        cntr.a = tmp.a | tmp.b;                                             \
        cntr.b = tmp.a | (!tmp.b);                                          \
    }
#define AXP_2BIT_DECR(cntr)                                                 \
    {                                                                       \
        AXP_2BIT_SAT_CNT tmp = cntr;                                        \
        cntr.a = tmp.a & tmp.b;                                             \
        cntr.b = tmp.a & (!tmp.b);                                          \
    }
#define AXP_2BIT_TAKE(cntr) cntr.b

/*
 * Macros for incrementing, decrementing, and determining whether to predict
 * to take the branch or not for 3-bit saturation counters.
 *
 * The INCR is called when a branch is actually taken.
 * The DECR is called when a branch is not actually taken.
 *
 * NOTE:    We do the following, when incrementing and decrementing, to be able
 *          to provide a bit of a hysteresis, to percent alternating between
 *          the FAVORS_NOT_TAKEN and FAVORS_TAKEN states.  Alternating between
 *          taken and not taken will still properly predict the branch 50% of
 *          the time.
 */
#define AXP_3BIT_INCR(cntr)                                                 \
    {                                                                       \
        AXP_3BIT_SAT_CNT tmp = cntr;                                        \
        cntr.a = tmp.a | (tmp.b & tmp.c);                                   \
        cntr.b = tmp.b | tmp.c;                                             \
        cntr.c = tmp.b | (!tmp.c);                                          \
    }
#define AXP_3BIT_DECR(cntr)                                                 \
    {                                                                       \
        AXP_3BIT_SAT_CNT tmp = cntr;                                        \
        cntr.a = (tmp.a & tmp.b) | (tmp.a & tmp.c);                         \
        cntr.b = tmp.b & tmp.c;                                             \
        cntr.c = tmp.b & (!tmp.c);                                          \
    }
#define AXP_3BIT_TAKE(cntr) cntr.c

/*
 * The following macros are to maintain the Local History Table and the Global
 * History Path.
 */
#define AXP_LOCAL_PATH_TAKEN(lpte)      (lpte) = (((lpte) * 2) + 1) & AXP_MASK_10_BITS
#define AXP_LOCAL_PATH_NOT_TAKEN(lpte)  (lpte) = ((lpte) * 2) & AXP_MASK_10_BITS
#define AXP_GLOBAL_PATH_TAKEN(gph)      (gph) = (((gph) * 2) + 1) & AXP_MASK_12_BITS
#define AXP_GLOBAL_PATH_NOT_TAKEN(gph)  (gph) = ((gph) * 2) & AXP_MASK_12_BITS

#endif /* _AXP_21264_PRED_DEFS_ */
