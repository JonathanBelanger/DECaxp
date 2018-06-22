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
 *  This header contains the definitions to support TELNET server code.
 *
 * Revision History:
 *
 *  V01.000	16-Jun-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef AXP_TELNET_H_
#define AXP_TELNET_H_
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"

/*
 * Definitions used in the source file.
 */
#define AXP_TELNET_MSG_LEN	1024
#define AXP_TELNET_DEFAULT_PORT	108
#define TELCMDS 		1
#define TELOPTS			1

/*
 * Define the states for the TELNET session.
 */
typedef enum
{
    Listen,
    Accept,
    Active,
    Inactive,
    Closing,
    Finished
} AXP_Telnet_Session_State;

/*
 * Telnet option states, based on RFC 1143.
 */
typedef enum
{
    No,
    WantNo,
    WantYes,
    Yes
} AXP_Telnet_UsHim_State;
typedef enum
{
    Empty,
    Opposite
} AXP_Telnet_UsqHimq_State;
typedef struct
{
    AXP_Telnet_UsHim_State	us;
    AXP_Telnet_UsqHimq_State	usQ;
    AXP_Telnet_UsHim_State	him;
    AXP_Telnet_UsqHimq_State	himQ;
} AXP_Telnet_Option_State;

/*
 * This data structure is used to handle a TELNET session.  It contains
 * state information and negotiated options.
 */
typedef struct
{
    int				mySocket;
    AXP_Telnet_Session_State	myState;
    AXP_Telnet_Option_State	myOptions[NTELOPTS];
} AXP_Telnet_Session;

/*
 * Function prototypes.
 */
bool AXP_Telnet_Send(int, u8 *, u32);
void AXP_Telnet_Main(void);

#endif /* AXP_TELNET_H_ */
