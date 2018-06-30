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
 *  This file contains the test code for the TELNET processing.
 *
 * Revision History:
 *
 *  V01.000	16-Jun-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_Telnet.h"

extern AXP_StateMachine	TN_Option_SM[AXP_OPT_MAX_ACTION][AXP_OPT_MAX_STATE];
extern AXP_StateMachine	TN_Receive_SM[AXP_ACT_MAX][AXP_RCV_MAX_STATE];
void Send_DO(AXP_SM_Args *);
void Send_DONT(AXP_SM_Args *);
void Send_WILL(AXP_SM_Args *);
void Send_WONT(AXP_SM_Args *);
void Echo_Data(AXP_SM_Args *);
void Save_CMD(AXP_SM_Args *);
void Process_CMD(AXP_SM_Args *);
void Cvt_Process_IAC(AXP_SM_Args *);
void SubOpt_Clear(AXP_SM_Args *);
void SubOpt_Accumulate(AXP_SM_Args *);
void SubOpt_TermProcess(AXP_SM_Args *);

typedef struct
{
    u8	currentState;
    u8	action;
    u8	resultantState;
    u16	actionMask;
} AXP_Test_SM;
u16 testActionMask;
#define NO_ACTION		0x0000
#define WILL_SENT		0x0001
#define WONT_SENT		0x0002
#define DO_SENT			0x0004
#define DONT_SENT		0x0008
#define ECHO_DATA		0x0010
#define PROC_CMD		0x0100
#define SUBOPT_CLEAR		0x0200
#define SUBOPT_ACCUM		0x0400
#define PROC_IAC		0x0800
#define SAVE_CMD		0x1000
#define CVT_PROC_IAC		0x2000
#define SUBOPT_TERM		0x4000
#define CLEAR_ACTION_MASK	(testActionMask) = 0;

#define YES_CLI_NOPREF		(YES_CLI-YES_SRV)*2
#define YES_CLI_PREF		(YES_CLI-YES_SRV)*2+1
#define NO_CLI_NOPREF		(NO_CLI-YES_SRV)*2
#define NO_CLI_PREF		(NO_CLI-YES_SRV)*2+1
#define YES_SRV_NOPREF		(YES_SRV-YES_SRV)*2
#define YES_SRV_PREF		(YES_SRV-YES_SRV)*2+1
#define NO_SRV_NOPREF		(NO_SRV-YES_SRV)*2
#define NO_SRV_PREF		(NO_SRV-YES_SRV)*2+1
#define WILL_NOPREF		(WILL-YES_SRV)*2
#define WILL_PREF		(WILL-YES_SRV)*2+1
#define WONT_NOPREF		(WONT-YES_SRV)*2
#define WONT_PREF		(WONT-YES_SRV)*2+1
#define DO_NOPREF		(DO-YES_SRV)*2
#define DO_PREF			(DO-YES_SRV)*2+1
#define DONT_NOPREF		(DONT-YES_SRV)*2
#define DONT_PREF		(DONT-YES_SRV)*2+1

AXP_Test_SM SM_Opt_Tests[] =
{

    /*
     * Set Remote Option (Yes) (using their options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		YES_CLI_PREF,	AXP_OPT_WANTYES_SRV,	DO_SENT},	/* 0 */
    {AXP_OPT_NO,		YES_CLI_NOPREF,	AXP_OPT_WANTYES_SRV,	DO_SENT},
    {AXP_OPT_YES,		YES_CLI_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_YES,		YES_CLI_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	YES_CLI_PREF,	AXP_OPT_WANTNO_CLI,	NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	YES_CLI_NOPREF,	AXP_OPT_WANTNO_CLI,	NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	YES_CLI_PREF,	AXP_OPT_WANTNO_CLI,	NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	YES_CLI_NOPREF,	AXP_OPT_WANTNO_CLI,	NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	YES_CLI_PREF,	AXP_OPT_WANTYES_SRV,	NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	YES_CLI_NOPREF,	AXP_OPT_WANTYES_SRV,	NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	YES_CLI_PREF,	AXP_OPT_WANTYES_SRV,	NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	YES_CLI_NOPREF,	AXP_OPT_WANTYES_SRV,	NO_ACTION},

    /*
     * Set Remote Option (No) (using their options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		NO_CLI_PREF,	AXP_OPT_NO,		NO_ACTION},	/* 12 */
    {AXP_OPT_NO,		NO_CLI_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_YES,		NO_CLI_PREF,	AXP_OPT_WANTNO_SRV,	DONT_SENT},
    {AXP_OPT_YES,		NO_CLI_NOPREF,	AXP_OPT_WANTNO_SRV,	DONT_SENT},
    {AXP_OPT_WANTNO_SRV,	NO_CLI_PREF,	AXP_OPT_WANTNO_SRV,	NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	NO_CLI_NOPREF,	AXP_OPT_WANTNO_SRV,	NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	NO_CLI_PREF,	AXP_OPT_WANTNO_SRV,	NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	NO_CLI_NOPREF,	AXP_OPT_WANTNO_SRV,	NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	NO_CLI_PREF,	AXP_OPT_WANTYES_CLI,	NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	NO_CLI_NOPREF,	AXP_OPT_WANTYES_CLI,	NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	NO_CLI_PREF,	AXP_OPT_WANTYES_CLI, NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	NO_CLI_NOPREF,	AXP_OPT_WANTYES_CLI, NO_ACTION},

    /*
     * Set Local Option (Yes) (using my options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		YES_SRV_PREF,	AXP_OPT_WANTYES_SRV,	WILL_SENT},	/* 24 */
    {AXP_OPT_NO,		YES_SRV_NOPREF,	AXP_OPT_WANTYES_SRV,	WILL_SENT},
    {AXP_OPT_YES,		YES_SRV_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_YES,		YES_SRV_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	YES_SRV_PREF,	AXP_OPT_WANTNO_CLI,	NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	YES_SRV_NOPREF,	AXP_OPT_WANTNO_CLI,	NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	YES_SRV_PREF,	AXP_OPT_WANTNO_CLI,	NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	YES_SRV_NOPREF,	AXP_OPT_WANTNO_CLI,	NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	YES_SRV_PREF,	AXP_OPT_WANTYES_SRV,	NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	YES_SRV_NOPREF,	AXP_OPT_WANTYES_SRV,	NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	YES_SRV_PREF,	AXP_OPT_WANTYES_SRV,	NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	YES_SRV_NOPREF,	AXP_OPT_WANTYES_SRV,	NO_ACTION},

    /*
     * Set Local Option (No) (using my options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		NO_SRV_PREF,	AXP_OPT_NO,		NO_ACTION},	/* 36 */
    {AXP_OPT_NO,		NO_SRV_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_YES,		NO_SRV_PREF,	AXP_OPT_WANTNO_SRV,	WONT_SENT},
    {AXP_OPT_YES,		NO_SRV_NOPREF,	AXP_OPT_WANTNO_SRV,	WONT_SENT},
    {AXP_OPT_WANTNO_SRV,	NO_SRV_PREF,	AXP_OPT_WANTNO_SRV,	NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	NO_SRV_NOPREF,	AXP_OPT_WANTNO_SRV,	NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	NO_SRV_PREF,	AXP_OPT_WANTNO_SRV,	NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	NO_SRV_NOPREF,	AXP_OPT_WANTNO_SRV,	NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	NO_SRV_PREF,	AXP_OPT_WANTYES_CLI,	NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	NO_SRV_NOPREF,	AXP_OPT_WANTYES_CLI,	NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	NO_SRV_PREF,	AXP_OPT_WANTYES_CLI,	NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	NO_SRV_NOPREF,	AXP_OPT_WANTYES_CLI,	NO_ACTION},

    /*
     * Receive WILL (using their options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		WILL_PREF,	AXP_OPT_YES,		DO_SENT},	/* 48 */
    {AXP_OPT_NO,		WILL_NOPREF,	AXP_OPT_NO,		DONT_SENT},
    {AXP_OPT_YES,		WILL_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_YES,		WILL_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	WILL_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	WILL_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	WILL_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	WILL_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	WILL_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	WILL_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	WILL_PREF,	AXP_OPT_WANTNO_SRV,	DONT_SENT},
    {AXP_OPT_WANTYES_CLI,	WILL_NOPREF,	AXP_OPT_WANTNO_SRV,	DONT_SENT},

    /*
     * Receive WONT (using their options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		WONT_PREF,	AXP_OPT_NO,		NO_ACTION},	/* 60 */
    {AXP_OPT_NO,		WONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_YES,		WONT_PREF,	AXP_OPT_NO,		DONT_SENT},
    {AXP_OPT_YES,		WONT_NOPREF,	AXP_OPT_NO,		DONT_SENT},
    {AXP_OPT_WANTNO_SRV,	WONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	WONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	WONT_PREF,	AXP_OPT_WANTYES_SRV,	DO_SENT},
    {AXP_OPT_WANTNO_CLI,	WONT_NOPREF,	AXP_OPT_WANTYES_SRV,	DO_SENT},
    {AXP_OPT_WANTYES_SRV,	WONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	WONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	WONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	WONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},

    /*
     * Receive DO (using my options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		DO_PREF,	AXP_OPT_YES,		WILL_SENT},	/* 72 */
    {AXP_OPT_NO,		DO_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_YES,		DO_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_YES,		DO_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	DO_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	DO_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	DO_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	DO_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	DO_PREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	DO_NOPREF,	AXP_OPT_YES,		NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	DO_PREF,	AXP_OPT_WANTNO_SRV,	WONT_SENT},
    {AXP_OPT_WANTYES_CLI,	DO_NOPREF,	AXP_OPT_WANTNO_SRV,	WONT_SENT},

    /*
     * Receive DONT (using my options state machine)
     *
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_OPT_NO,		DONT_PREF,	AXP_OPT_NO,		NO_ACTION},	/* 84 */
    {AXP_OPT_NO,		DONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_YES,		DONT_PREF,	AXP_OPT_NO,		WONT_SENT},
    {AXP_OPT_YES,		DONT_NOPREF,	AXP_OPT_NO,		WONT_SENT},
    {AXP_OPT_WANTNO_SRV,	DONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_SRV,	DONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTNO_CLI,	DONT_PREF,	AXP_OPT_WANTYES_SRV,	WILL_SENT},
    {AXP_OPT_WANTNO_CLI,	DONT_NOPREF,	AXP_OPT_WANTYES_SRV,	WILL_SENT},
    {AXP_OPT_WANTYES_SRV,	DONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_SRV,	DONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	DONT_PREF,	AXP_OPT_NO,		NO_ACTION},
    {AXP_OPT_WANTYES_CLI,	DONT_NOPREF,	AXP_OPT_NO,		NO_ACTION},

    /*
     * This is the last entry.
     */
    {AXP_OPT_MAX_STATE,		0,		0,			0}		/* 96 */
};

AXP_Test_SM SM_Rcv_Tests[] =
{

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_DATA,		AXP_ACT_NUL,	AXP_RCV_DATA,		ECHO_DATA},	/* 0 */
    {AXP_RCV_DATA,		AXP_ACT_IAC,	AXP_RCV_IAC,		NO_ACTION},
    {AXP_RCV_DATA,		AXP_ACT_R,	AXP_RCV_CR,		ECHO_DATA},
    {AXP_RCV_DATA,		AXP_ACT_CMD,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_DATA,		AXP_ACT_SE,	AXP_RCV_DATA,		CVT_PROC_IAC},
    {AXP_RCV_DATA,		AXP_ACT_SB,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_DATA,		AXP_ACT_CATCHALL,AXP_RCV_DATA,		ECHO_DATA},

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_IAC,		AXP_ACT_NUL,	AXP_RCV_DATA,		NO_ACTION},	/* 7 */
    {AXP_RCV_IAC,		AXP_ACT_IAC,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_IAC,		AXP_ACT_R,	AXP_RCV_DATA,		NO_ACTION},
    {AXP_RCV_IAC,		AXP_ACT_CMD,	AXP_RCV_CMD,		SAVE_CMD},
    {AXP_RCV_IAC,		AXP_ACT_SE,	AXP_RCV_DATA,		NO_ACTION},
    {AXP_RCV_IAC,		AXP_ACT_SB,	AXP_RCV_SB,		SUBOPT_CLEAR},
    {AXP_RCV_IAC,		AXP_ACT_CATCHALL,AXP_RCV_DATA,		NO_ACTION},

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_CMD,		AXP_ACT_NUL,	AXP_RCV_DATA,		PROC_CMD},	/* 14 */
    {AXP_RCV_CMD,		AXP_ACT_IAC,	AXP_RCV_DATA,		PROC_CMD},
    {AXP_RCV_CMD,		AXP_ACT_R,	AXP_RCV_DATA,		PROC_CMD},
    {AXP_RCV_CMD,		AXP_ACT_CMD,	AXP_RCV_DATA,		PROC_CMD},
    {AXP_RCV_CMD,		AXP_ACT_SE,	AXP_RCV_DATA,		PROC_CMD},
    {AXP_RCV_CMD,		AXP_ACT_SB,	AXP_RCV_DATA,		PROC_CMD},
    {AXP_RCV_CMD,		AXP_ACT_CATCHALL,AXP_RCV_DATA,		PROC_CMD},

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_CR,		AXP_ACT_NUL,	AXP_RCV_DATA,		NO_ACTION},	/* 21 */
    {AXP_RCV_CR,		AXP_ACT_IAC,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_CR,		AXP_ACT_R,	AXP_RCV_DATA,		NO_ACTION},
    {AXP_RCV_CR,		AXP_ACT_CMD,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_CR,		AXP_ACT_SE,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_CR,		AXP_ACT_SB,	AXP_RCV_DATA,		ECHO_DATA},
    {AXP_RCV_CR,		AXP_ACT_CATCHALL,AXP_RCV_DATA,		ECHO_DATA},

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_SB,		AXP_ACT_NUL,	AXP_RCV_SB,		SUBOPT_ACCUM},	/* 28 */
    {AXP_RCV_SB,		AXP_ACT_IAC,	AXP_RCV_SE,		NO_ACTION},
    {AXP_RCV_SB,		AXP_ACT_R,	AXP_RCV_SB,		SUBOPT_ACCUM},
    {AXP_RCV_SB,		AXP_ACT_CMD,	AXP_RCV_SB,		SUBOPT_ACCUM},
    {AXP_RCV_SB,		AXP_ACT_SE,	AXP_RCV_SB,		SUBOPT_ACCUM},
    {AXP_RCV_SB,		AXP_ACT_SB,	AXP_RCV_SB,		SUBOPT_TERM},
    {AXP_RCV_SB,		AXP_ACT_CATCHALL,AXP_RCV_SB,		SUBOPT_ACCUM},

    /*
     * Current State		Action		ResultantState		ActionMask
     */
    {AXP_RCV_SE,		AXP_ACT_NUL,	AXP_RCV_IAC,		CVT_PROC_IAC},	/* 35 */
    {AXP_RCV_SE,		AXP_ACT_IAC,	AXP_RCV_SB,		SUBOPT_ACCUM},
    {AXP_RCV_SE,		AXP_ACT_R,	AXP_RCV_IAC,		CVT_PROC_IAC},
    {AXP_RCV_SE,		AXP_ACT_CMD,	AXP_RCV_IAC,		CVT_PROC_IAC},
    {AXP_RCV_SE,		AXP_ACT_SE,	AXP_RCV_DATA,		SUBOPT_TERM},
    {AXP_RCV_SE,		AXP_ACT_SB,	AXP_RCV_IAC,		CVT_PROC_IAC},
    {AXP_RCV_SE,		AXP_ACT_CATCHALL,AXP_RCV_IAC,		CVT_PROC_IAC},

    /*
     * This is the last entry.
     */
    {AXP_RCV_MAX_STATE,		0,		0,			0}		/* 42 */
};

/*
 * These are test action routines that will be called instead of the real ones.
 */
void Test_Send_DO(AXP_SM_Args *ign)
{
    testActionMask |= DO_SENT;
    return;
}
void Test_Send_DONT(AXP_SM_Args *ign)
{
    testActionMask |= DONT_SENT;
    return;
}
void Test_Send_WILL(AXP_SM_Args *ign)
{
    testActionMask |= WILL_SENT;
    return;
}
void Test_Send_WONT(AXP_SM_Args *ign)
{
    testActionMask |= WONT_SENT;
    return;
}
void Test_Echo_Data(AXP_SM_Args *ign)
{
    testActionMask |= ECHO_DATA;
    return;
}
void Test_Save_CMD(AXP_SM_Args *ign)
{
    testActionMask |= SAVE_CMD;
    return;
}
void Test_Process_CMD(AXP_SM_Args *ign)
{
    testActionMask |= PROC_CMD;
    return;
}
void Test_Cvt_Process_IAC(AXP_SM_Args *ign)
{
    testActionMask |= CVT_PROC_IAC;
    return;
}
void Test_SubOpt_Clear(AXP_SM_Args *ign)
{
    testActionMask |= SUBOPT_CLEAR;
    return;
}
void Test_SubOpt_Accumulate(AXP_SM_Args *ign)
{
    testActionMask |= SUBOPT_ACCUM;
    return;
}
void Test_SubOpt_TermProcess(AXP_SM_Args *ign)
{
    testActionMask |= SUBOPT_TERM;
    return;
}
void Test_Process_IAC(AXP_SM_Args *ign)
{
    testActionMask |= PROC_IAC;
    return;
}

bool test_options_StateMachine(void)
{
    AXP_StateMachine	*sm;
    AXP_SM_Entry	*entry;
    bool		retVal = true;
    int			ii, jj, kk = 1;
    u8			nextState;

    /*
     * First things first, we need to substitute the real action routines with
     * the test versions.
     */
    printf("...Initializing Option State Machine for Testing...\n");
    sm = (AXP_StateMachine *) &TN_Option_SM;
    for (ii = 0; ii < AXP_OPT_MAX_ACTION; ii++)
	for (jj = 0; jj < AXP_OPT_MAX_STATE; jj++)
	{
	    entry = AXP_SM_ENTRY(sm, ii, jj);
	    if (entry->actionRtn != NULL)
	    {
		if (entry->actionRtn == Send_DO)
		    entry->actionRtn = Test_Send_DO;
		else if (entry->actionRtn == Send_DONT)
		    entry->actionRtn = Test_Send_DONT;
		else if (entry->actionRtn == Send_WILL)
		    entry->actionRtn = Test_Send_WILL;
		else if (entry->actionRtn == Send_WONT)
		    entry->actionRtn = Test_Send_WONT;
	    }
	}

    /*
     * Now we can run our tests.  Loop through the test cases, execute the
     * state machine and determine if what occurred is what was expected (as
     * far as state transitions and action routines called).
     */
    ii = 0;
    while ((SM_Opt_Tests[ii].currentState < AXP_OPT_MAX_STATE) && (retVal == true))
    {
	printf(
	    "...Executing Option State Machine Test %3d for [%d]="
	    "{curState: %d, action: %d, nextState: %d, action: 0x%04x}...",
	    kk++,
	    ii,
	    SM_Opt_Tests[ii].currentState,
	    SM_Opt_Tests[ii].action,
	    SM_Opt_Tests[ii].resultantState,
	    SM_Opt_Tests[ii].actionMask);
	testActionMask = 0;
	nextState = AXP_Execute_SM(
			sm,
			SM_Opt_Tests[ii].action,
			SM_Opt_Tests[ii].currentState,
			NULL);
	printf(
	    " got {x,x, nextState: %d, action: 0x%04x}...\n",
	    nextState,
	    testActionMask);
	if ((nextState != SM_Opt_Tests[ii].resultantState) ||
	    (testActionMask != SM_Opt_Tests[ii].actionMask))
	    retVal = false;
	ii++;
    }

    /*
     * Last things last, we need to replace the real action routines.
     */
    printf("...Resetting Option State Machine for Use...\n");
    for (ii = 0; ii < AXP_OPT_MAX_ACTION; ii++)
	for (jj = 0; jj < AXP_OPT_MAX_STATE; jj++)
	{
	    entry = AXP_SM_ENTRY(sm, ii, jj);
	    if (entry->actionRtn != NULL)
	    {
		if (entry->actionRtn == Test_Send_DO)
		    entry->actionRtn = Send_DO;
		else if (entry->actionRtn == Test_Send_DONT)
		    entry->actionRtn = Send_DONT;
		else if (entry->actionRtn == Test_Send_WILL)
		    entry->actionRtn = Send_WILL;
		else if (entry->actionRtn == Test_Send_WONT)
		    entry->actionRtn = Send_WONT;
	    }
	}

    /*
     * Now, let's do the session state machine.
     */
    if (retVal == true)
    {

	    /*
	     * First things first, we need to substitute the real action
	     * routines with the test versions.
	     */
	    printf("...Initializing Receive State Machine for Testing...\n");
	    sm = (AXP_StateMachine *) &TN_Receive_SM;
	    for (ii = 0; ii < AXP_ACT_MAX; ii++)
		for (jj = 0; jj < AXP_RCV_MAX_STATE; jj++)
		{
		    entry = AXP_SM_ENTRY(sm, ii, jj);
		    if (entry->actionRtn != NULL)
		    {
			if (entry->actionRtn == Echo_Data)
			    entry->actionRtn = Test_Echo_Data;
			else if (entry->actionRtn == Save_CMD)
			    entry->actionRtn = Test_Save_CMD;
			else if (entry->actionRtn == Process_CMD)
			    entry->actionRtn = Test_Process_CMD;
			else if (entry->actionRtn == Cvt_Process_IAC)
			    entry->actionRtn = Test_Cvt_Process_IAC;
			else if (entry->actionRtn == SubOpt_Clear)
			    entry->actionRtn = Test_SubOpt_Clear;
			else if (entry->actionRtn == SubOpt_Accumulate)
			    entry->actionRtn = Test_SubOpt_Accumulate;
			else if (entry->actionRtn == SubOpt_TermProcess)
			    entry->actionRtn = Test_SubOpt_TermProcess;
		    }
		}

	    /*
	     * Now we can run our tests.  Loop through the test cases, execute the
	     * state machine and determine if what occurred is what was expected (as
	     * far as state transitions and action routines called).
	     */
	    ii = 0;
	    while ((SM_Rcv_Tests[ii].currentState < AXP_RCV_MAX_STATE) && (retVal == true))
	    {
		printf(
		    "...Executing Receive State Machine Test %3d for [%d]="
		    "{curState: %d, action: %d, nextState: %d, action: 0x%04x}...",
		    kk++,
		    ii,
		    SM_Rcv_Tests[ii].currentState,
		    SM_Rcv_Tests[ii].action,
		    SM_Rcv_Tests[ii].resultantState,
		    SM_Rcv_Tests[ii].actionMask);
		testActionMask = 0;
		nextState = AXP_Execute_SM(
				sm,
				SM_Rcv_Tests[ii].action,
				SM_Rcv_Tests[ii].currentState,
				NULL);
		printf(
		    " got {x,x, nextState: %d, action: 0x%04x}...\n",
		    nextState,
		    testActionMask);
		if ((nextState != SM_Rcv_Tests[ii].resultantState) ||
		    (testActionMask != SM_Rcv_Tests[ii].actionMask))
		    retVal = false;
		ii++;
	    }

	    /*
	     * Last things last, we need to substitute the real action routines
	     * with the test versions.
	     */
	    printf("...Resetting Receive State Machine for Use...\n");
	    for (ii = 0; ii < AXP_ACT_MAX; ii++)
		for (jj = 0; jj < AXP_RCV_MAX_STATE; jj++)
		{
		    entry = AXP_SM_ENTRY(sm, ii, jj);
		    if (entry->actionRtn != NULL)
		    {
			if (entry->actionRtn == Test_Echo_Data)
			    entry->actionRtn = Echo_Data;
			else if (entry->actionRtn == Test_Save_CMD)
			    entry->actionRtn = Save_CMD;
			else if (entry->actionRtn == Test_Process_CMD)
			    entry->actionRtn = Process_CMD;
			else if (entry->actionRtn == Test_Cvt_Process_IAC)
			    entry->actionRtn = Cvt_Process_IAC;
			else if (entry->actionRtn == Test_SubOpt_Clear)
			    entry->actionRtn = SubOpt_Clear;
			else if (entry->actionRtn == Test_SubOpt_Accumulate)
			    entry->actionRtn = SubOpt_Accumulate;
			else if (entry->actionRtn == Test_SubOpt_TermProcess)
			    entry->actionRtn = SubOpt_TermProcess;
		    }
		}
    }

    /*
     * Return the results of this test back to the caller.
     */
    return(retVal);
}

int main(void)
{
    bool retVal = true;

    printf("\nDECaxp Telnet Testing...\n");
    printf("\nTesting Options and Receive State Machines...\n");
    if (AXP_TraceInit() == true)
    {
	retVal = test_options_StateMachine();
	if (retVal == true)
	{
	    printf("\nTesting Telnet Server...\n");
	    AXP_Telnet_Main();
	}
    }
    else
	retVal = false;
    if (retVal == true)
	printf("All Tests Successful!\n");
    else
	printf("At Least One Test Failed.\n");
    return(0);
}
