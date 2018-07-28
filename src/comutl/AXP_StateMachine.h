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
 *  This header file contains the definitions required to support state machine
 *  processing.  A state machine is comprised of an input and a current state,
 *  which points to a next state and potentially an action routine.
 *
 * Revision History:
 *
 *  V01.000		22-Jun-2017	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _AXP_STATE_MACHINE_
#define _AXP_STATE_MACHINE_

#include "AXP_Utility.h"

/*
 * The following definition represents an entry in a state machine.  Each entry
 * in the state machine is indexed by an input value and the current state.
 * When an entry is selected, if the action routine is not NULL, it is called
 * before the current state is set to the next state.  Otherwise, the current
 * state is set to the next state.
 */
#define AXP_SM_MAX_ARGS		16
typedef struct
{
    int		argc;
    void	*argp[AXP_SM_MAX_ARGS];
} AXP_SM_Args;
typedef struct
{
    u8		nextState;
    void	(*actionRtn)(AXP_SM_Args *);
} AXP_SM_Entry;
typedef struct
{
    char	*smName;
    u8		maxActions;
    u8		maxStates;
    void 	*stateMachine;
} AXP_StateMachine;

/*
 * OK, This calculates the address of the entry we are looking to process in
 * the state machine.  The address is calculated as follows (where action is
 * the number of rows and current state is the column within that row):
 *
 * 	entry = base address + (row * (max columns * size of entry)) +
 * 		(column * size of entry)
 */
#define AXP_SM_ENTRY(smp, row, col)				\
    (AXP_SM_Entry *) ((u8 *) (smp)->stateMachine +		\
	((row) * ((smp->maxStates)) * sizeof(AXP_SM_Entry)) +	\
	((col) * sizeof(AXP_SM_Entry)))

u8 AXP_Execute_SM(AXP_StateMachine *, u8, u8, AXP_SM_Args *);

#endif /* _AXP_STATE_MACHINE_ */
