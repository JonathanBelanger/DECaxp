/*
 * Copyright (C) TDEVJ1B 2018.
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
 *  This module contains the code to execute a state machine.
 *
 * Revision History:
 *
 *  V01.000		22-Jun-2018	TDEVJ1B
 *  Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_StateMachine.h"

/*
 * AXP_Execute_SM
 *  This function is called to execute a state machine based upon the current
 *  state.  This includes calling the action routine, if it is not NULL.
 *
 * Input Parameters:
 *  sm:
 *	This is the address of the state machine matrix.  The action and
 *	curState parameters are the input into this state machine matrix.
 *  action:
 *	This is the action that is being performed on the state machine.
 *  curState:
 *	This is the current state of the state machine and is the other input
 *	into the state machine.
 *  max:
 *	This is the maximum value allowed for the state machine.
 *  args:
 *	This is one or more arguments to be passed onto the action routine.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  The value of the next state for the state machine.
 */
u8 AXP_Execute_SM(
	    u16 maxAction,
	    u16 maxState,
	    AXP_StateMachine sm[maxAction][maxState],
	    u8 action,
	    u8 curState,
	    AXP_SM_Args *args)
{
    char	trcBuf[512];
    int		trcIdx = 0;
    u8		retVal = curState;
    bool	act;

    if (AXP_UTL_CALL)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("AXP_Execute_SM Called.");
	AXP_TRACE_END();
    }

    /*
     * If there is an action Routine, go ahead and call it.
     */
    if (action <= maxAction)
    {
	trcIdx += sprintf(
			&trcBuf[trcIdx],
			"\tCurrent State = %d, Action = 0x%02x (%d) --> ",
			curState,
			action,
			action);
	if (sm[action][curState].actionRtn != NULL)
	{
	    (*sm[action][curState].actionRtn)(args);
	    act = true;
	}
	else
	    act = false;
	retVal = sm[action][curState].nextState;
	trcIdx += sprintf(
			&trcBuf[trcIdx],
			"Next State = %d (Action Routine %s called)",
			retVal,
			(act ? "" : "not"));
	if (AXP_UTL_OPT2)
	{
	    AXP_TRACE_BEGIN();
	    AXP_TraceWrite(trcBuf);
	    AXP_TRACE_END();
	}
    }
    else if (AXP_UTL_OPT2)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
		"\tState Machine not executed because action was outside limits "
		"(action = %d, max = %d).",
		action,
		maxAction);
	AXP_TRACE_END();
    }

    if (AXP_UTL_CALL)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("AXP_Execute_SM Returning (%d).", retVal);
	AXP_TRACE_END();
    }

    /*
     * Return the next state (or current state) back to the caller.
     */
    return(retVal);
}
