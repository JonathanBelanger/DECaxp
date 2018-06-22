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
 *  ...:
 *	This is one or more arguments to be passed onto the action routine.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  The value of the next state for the state machine.
 */
u8 AXP_Execute_SM(
	    AXP_StateMachine **sm,
	    u8 action,
	    u8 curState,
	    u8 max,
	    ...)
{
    u8		retVal = curState;
    va_list	args;

    /*
     * If there is an action Routine, go ahead and call it.
     */
    if (action <= max)
    {
	va_start(args, max);
	if (sm[action][curState].actionRtn != NULL)
	    (sm[action][curState].actionRtn)(args);
	va_end(args);
	retVal = sm[action][curState].nextState;
    }

    /*
     * Return the next state (or current state) back to the caller.
     */
    return(retVal);
}
