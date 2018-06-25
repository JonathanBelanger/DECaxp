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
 *  This module contains the TELNET server code.  It sets up a port on which to
 *  listen and accepts just one connection.  Once a connection is accepted,
 *  other attempts to connect will be rejected.  If the active connection is
 *  dropped, it will get cleaned up and the listener will now accept new
 *  connection requests.
 *
 * Revision History:
 *
 *  V01.000	16-Jun-2018	Jonathan D. Belanger
 *  Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Configure.h"
#include "AXP_StateMachine.h"
#define TELCMDS 		1
#define TELOPTS			1
#include "AXP_Telnet.h"

/*
 * State machine definitions.
 *
 * Action Routines for the state machines.
 */
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
void Cvt_Proc_CMD(AXP_SM_Args *);

/*
 * This definition below is used for processing the options sent from the
 * client and ones we want to send to the client.
 */
AXP_StateMachine TN_Option_SM[AXP_OPT_MAX_ACTION][AXP_OPT_MAX_STATE] =
{
    /* YES_SRV	- NOT PREFERRED */
    {
	{AXP_OPT_WANTYES_SRV,	Send_WILL},
	{AXP_OPT_WANTNO_CLI,	NULL},
	{AXP_OPT_WANTNO_CLI,	NULL},
	{AXP_OPT_WANTYES_SRV,	NULL},
	{AXP_OPT_WANTYES_SRV,	NULL},
	{AXP_OPT_YES,		NULL}
    },
    /* YES_SRV	- PREFERRED */
    {
	{AXP_OPT_WANTYES_SRV,	Send_WILL},
	{AXP_OPT_WANTNO_CLI,	NULL},
	{AXP_OPT_WANTNO_CLI,	NULL},
	{AXP_OPT_WANTYES_SRV,	NULL},
	{AXP_OPT_WANTYES_SRV,	NULL},
	{AXP_OPT_YES,		NULL}
    },
    /* NO_SRV	- NOT PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTNO_SRV,	NULL},
	{AXP_OPT_WANTNO_SRV,	NULL},
	{AXP_OPT_WANTYES_CLI,	NULL},
	{AXP_OPT_WANTYES_CLI,	NULL},
	{AXP_OPT_WANTNO_SRV,	Send_WONT}
    },
    /* NO_SRV	- PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTNO_SRV,	NULL},
	{AXP_OPT_WANTNO_SRV,	NULL},
	{AXP_OPT_WANTYES_CLI,	NULL},
	{AXP_OPT_WANTYES_CLI,	NULL},
	{AXP_OPT_WANTNO_SRV,	Send_WONT}
    },
    /* YES_CLI	- NOT PREFERRED */
    {
	{AXP_OPT_WANTYES_SRV,	Send_DO},
	{AXP_OPT_WANTNO_CLI,	NULL},
	{AXP_OPT_WANTNO_CLI,	NULL},
	{AXP_OPT_WANTYES_SRV,	NULL},
	{AXP_OPT_WANTYES_SRV,	NULL},
	{AXP_OPT_YES,		NULL}
    },
    /* YES_CLI	- PREFERRED */
    {
	{AXP_OPT_WANTYES_SRV,	Send_DO},
	{AXP_OPT_WANTNO_CLI,	NULL},
	{AXP_OPT_WANTNO_CLI,	NULL},
	{AXP_OPT_WANTYES_SRV,	NULL},
	{AXP_OPT_WANTYES_SRV,	NULL},
	{AXP_OPT_YES,		NULL}
    },
    /* NO_CLI	- NOT PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTNO_SRV,	NULL},
	{AXP_OPT_WANTNO_SRV,	NULL},
	{AXP_OPT_WANTYES_CLI,	NULL},
	{AXP_OPT_WANTYES_CLI,	NULL},
	{AXP_OPT_WANTNO_SRV,	Send_DONT}
    },
    /* NO_CLI	- PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTNO_SRV,	NULL},
	{AXP_OPT_WANTNO_SRV,	NULL},
	{AXP_OPT_WANTYES_CLI,	NULL},
	{AXP_OPT_WANTYES_CLI,	NULL},
	{AXP_OPT_WANTNO_SRV,	Send_DONT}
    },
    /* WILL	- NOT PREFERRED */
    {
	{AXP_OPT_NO,		Send_DONT},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_WANTNO_SRV,	Send_DONT},
	{AXP_OPT_YES,		NULL}
    },
    /* WILL	- PREFERRED */
    {
	{AXP_OPT_YES,		Send_DO},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_WANTNO_SRV,	Send_DONT},
	{AXP_OPT_YES,		NULL}
    },
    /* WONT	- NOT PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTYES_SRV,	Send_DO},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		Send_DONT}
    },
    /* WONT	- PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTYES_SRV,	Send_DO},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		Send_DONT}
    },
    /* DO	- NOT PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_WANTNO_SRV,	Send_WONT},
	{AXP_OPT_YES,		NULL}
    },
    /* DO	- PREFERRED */
    {
	{AXP_OPT_YES,		Send_WILL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_WANTNO_SRV,	Send_WONT},
	{AXP_OPT_YES,		NULL}
    },
    /* DONT	- NOT PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTYES_SRV,	Send_WILL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		Send_WONT}
    },
    /* DONT	- PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTYES_SRV,	Send_WILL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		Send_WONT}
    }
};

/*
 *
 * This definition below is used for processing data received from the client.
 */
AXP_StateMachine TN_Receive_SM[AXP_ACT_MAX][AXP_RCV_MAX_STATE] =
{
    /* '\0' */
    {
	{AXP_RCV_DATA,		Echo_Data},
	{AXP_RCV_DATA,		NULL},
	{AXP_RCV_DATA,		Process_CMD},
	{AXP_RCV_DATA,		NULL},
	{AXP_RCV_SB,		SubOpt_Accumulate},
	{AXP_RCV_IAC,		Cvt_Process_IAC}
    },
    /* IAC */
    {
	{AXP_RCV_IAC,		NULL},
	{AXP_RCV_DATA,		Echo_Data},
	{AXP_RCV_DATA,		Process_CMD},
	{AXP_RCV_DATA,		Echo_Data},
	{AXP_RCV_SE,		NULL},
	{AXP_RCV_SB,		SubOpt_Accumulate}
    },
    /* '\r' */
    {
	{AXP_RCV_CR,		Echo_Data},
	{AXP_RCV_DATA,		NULL},
	{AXP_RCV_DATA,		Process_CMD},
	{AXP_RCV_DATA,		NULL},
	{AXP_RCV_SB,		SubOpt_Accumulate},
	{AXP_RCV_IAC,		Cvt_Process_IAC}
    },
    /* TELNET-CMD */
    {
	{AXP_RCV_DATA,		Echo_Data},
	{AXP_RCV_CMD,		Save_CMD},
	{AXP_RCV_DATA,		Process_CMD},
	{AXP_RCV_DATA,		Echo_Data},
	{AXP_RCV_SB,		SubOpt_Accumulate},
	{AXP_RCV_IAC,		Cvt_Process_IAC}
    },
    /* SE */
    {
	{AXP_RCV_DATA,		Cvt_Process_IAC},
	{AXP_RCV_DATA,		NULL},
	{AXP_RCV_DATA,		Process_CMD},
	{AXP_RCV_DATA,		Echo_Data},
	{AXP_RCV_SB,		SubOpt_Accumulate},
	{AXP_RCV_DATA,		SubOpt_TermProcess}
    },
    /* SB */
    {
	{AXP_RCV_DATA,		Echo_Data},
	{AXP_RCV_SB,		SubOpt_Clear},
	{AXP_RCV_DATA,		Process_CMD},
	{AXP_RCV_DATA,		Echo_Data},
	{AXP_RCV_SB,		SubOpt_TermProcess},
	{AXP_RCV_IAC,		Cvt_Process_IAC}
    },
    /* CATCH-ALL */
    {
	{AXP_RCV_DATA,		Echo_Data},
	{AXP_RCV_DATA,		NULL},
	{AXP_RCV_DATA,		Process_CMD},
	{AXP_RCV_DATA,		Echo_Data},
	{AXP_RCV_SB,		SubOpt_Accumulate},
	{AXP_RCV_IAC,		Cvt_Process_IAC}
    }
};

/*
 * This state value is used to maintain the state of being able to listen and
 * accept connections.  Once a connection has been accepted, then a session
 * block will be created to hold the TELNET connection information.  At this
 * point we probably could accept more connections, but we will not allow it,
 * at least at this time.
 */
AXP_Telnet_Session_State srvState;

#define DIRECTION(dir)	(((dir) == '<') ? "RCVD" : "SENT")

/*
 * Local Prototypes.
 */
static void Process_Suboption(AXP_TELNET_SESSION *);
static void AXP_Telnet_PrintOption(char, u8 *, u32);
static void AXP_Telnet_PrintSub(char, u8 *, int);
static bool AXP_Telnet_Listener(int *);
static AXP_TELNET_SESSION *AXP_Telnet_Accept(int);
static bool AXP_Telnet_Receive(AXP_TELNET_SESSION *, u8 *, u32 *);
static bool AXP_Telnet_Reject(AXP_TELNET_SESSION *);
static bool AXP_Telnet_Ignore(int);
static bool AXP_Telnet_Processor(AXP_TELNET_SESSION *, u8 *, u32);

/*
 * Send_DO
 *  This function sends a DO <option> command to the client.  This function is
 *  called as part of the receive state machine processing.
 *
 * Input Parameters:
 *  args:
 *	A pointer to the argument structure containing the number of arguments,
 *	and an array of pointers to each of the arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void Send_DO(AXP_SM_Args *args)
{
    AXP_TELNET_SESSION	*ses = (AXP_TELNET_SESSION *) args->argp[0];
    bool		retVal = true;
    u8			opt = *((u8 *) args->argp[1]);
    u8			buf[3];

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("\tSend_DO Called (%s).", TELOPT(opt));
	AXP_TRACE_END();
    }

    /*
     * Send the IAC DO <opt> to the client.
     */
    buf[0] = IAC;
    buf[1] = DO;
    buf[2] = opt;
    retVal = AXP_Telnet_Send(ses, buf, 3);

    /*
     * OK, something happened and the session is no longer active.  Set the
     * server state, so that we can start cleaning up the connection.
     */
    if (retVal == false)
	srvState = Inactive;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * Send_DONT
 *  This function sends a DONT <option> command to the client.  This function
 *  is called as part of the receive state machine processing.
 *
 * Input Parameters:
 *  args:
 *	A pointer to the argument structure containing the number of arguments,
 *	and an array of pointers to each of the arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void Send_DONT(AXP_SM_Args *args)
{
    AXP_TELNET_SESSION	*ses = (AXP_TELNET_SESSION *) args->argp[0];
    bool		retVal = true;
    u8			opt = *((u8 *) args->argp[1]);
    u8			buf[3];

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("\tSend_DONT Called (%s).", TELOPT(opt));
	AXP_TRACE_END();
    }

    /*
     * Send the IAC DONT <opt> to the client.
     */
    buf[0] = IAC;
    buf[1] = DONT;
    buf[2] = opt;
    retVal = AXP_Telnet_Send(ses, buf, 3);

    /*
     * OK, something happened and the session is no longer active.  Set the
     * server state, so that we can start cleaning up the connection.
     */
    if (retVal == false)
	srvState = Inactive;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * Send_WILL
 *  This function sends a WILL <option> command to the client.  This function
 *  is called as part of the receive state machine processing.
 *
 * Input Parameters:
 *  args:
 *	A pointer to the argument structure containing the number of arguments,
 *	and an array of pointers to each of the arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void Send_WILL(AXP_SM_Args *args)
{
    AXP_TELNET_SESSION	*ses = (AXP_TELNET_SESSION *) args->argp[0];
    bool		retVal = true;
    u8			opt = *((u8 *) args->argp[1]);
    u8			buf[3];

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("\tSend_WILL Called (%s).", TELOPT(opt));
	AXP_TRACE_END();
    }

    /*
     * Send the IAC WILL <opt> to the client.
     */
    buf[0] = IAC;
    buf[1] = WILL;
    buf[2] = opt;
    retVal = AXP_Telnet_Send(ses, buf, 3);

    /*
     * OK, something happened and the session is no longer active.  Set the
     * server state, so that we can start cleaning up the connection.
     */
    if (retVal == false)
	srvState = Inactive;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * Send_WONT
 *  This function sends a WONT <option> command to the client.  This function
 *  is called as part of the receive state machine processing.
 *
 * Input Parameters:
 *  args:
 *	A pointer to the argument structure containing the number of arguments,
 *	and an array of pointers to each of the arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void Send_WONT(AXP_SM_Args *args)
{
    AXP_TELNET_SESSION	*ses = (AXP_TELNET_SESSION *) args->argp[0];
    bool		retVal = true;
    u8			opt = *((u8 *) args->argp[1]);
    u8			buf[3];

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("\tSend_WONT Called (%s).", TELOPT(opt));
	AXP_TRACE_END();
    }

    /*
     * Send the IAC WONT <opt> to the client.
     */
    buf[0] = IAC;
    buf[1] = WONT;
    buf[2] = opt;
    retVal = AXP_Telnet_Send(ses, buf, 3);

    /*
     * OK, something happened and the session is no longer active.  Set the
     * server state, so that we can start cleaning up the connection.
     */
    if (retVal == false)
	srvState = Inactive;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * Echo_Data
 *  This function sends a character back to the client, but only if we are
 *  supposed to be echoing data back to the client.  This function is called as
 *  part of the receive state machine processing.
 *
 * Input Parameters:
 *  args:
 *	A pointer to the argument structure containing the number of arguments,
 *	and an array of pointers to each of the arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void Echo_Data(AXP_SM_Args *args)
{
    AXP_TELNET_SESSION	*ses = (AXP_TELNET_SESSION *) args->argp[0];
    bool		retVal = true;
    u8			c = *((u8 *) args->argp[1]);

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("\tEcho_Data Called (%c - %02x).", c, c);
	AXP_TRACE_END();
    }

    /*
     * Get the character that needs to be sent back to the client, but only if
     * echoing is turned on.
     */
    if (ses->myOptions[TELOPT_ECHO].state == AXP_OPT_YES)
    {
	retVal = AXP_Telnet_Send(ses, &c, 1);

	/*
	 * OK, something happened and the session is no longer active.  Set the
	 * server state, so that we can start cleaning up the connection.
	 */
	if (retVal == false)
	    srvState = Inactive;
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * Save_CMD
 *  This function is called to save the command that is being processed.  Once
 *  the option is received, then we'll process the command.  This function is
 *  called as part of the receive state machine processing.
 *
 * Input Parameters:
 *  args:
 *	A pointer to the argument structure containing the number of arguments,
 *	and an array of pointers to each of the arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void Save_CMD(AXP_SM_Args *args)
{
    AXP_TELNET_SESSION	*ses = (AXP_TELNET_SESSION *) args->argp[0];

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("\tSave_CMD Called (%s).", TELCMD(*((u8 *) args->argp[1])));
	AXP_TRACE_END();
    }

    /*
     * Get the command we are processing from the variable list portion of the
     * call arguments.
     */
    ses->cmd = *((u8 *) args->argp[1]);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * Process_CMD
 *  This function is called because we have a DO|DONT|WILL|WONT <command> to be
 *  processed.
 *
 * Input Parameters:
 *  args:
 *	A pointer to the argument structure containing the number of arguments,
 *	and an array of pointers to each of the arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void Process_CMD(AXP_SM_Args *args)
{
    AXP_TELNET_SESSION	*ses = (AXP_TELNET_SESSION *) args->argp[0];
    AXP_Telnet_OptState *opts;
    u8			opt = *((u8 *) args->argp[1]);
    AXP_SM_Args		newArg;

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
		"\tProcess_CMD Called (%s %s).",
		TELCMD(ses->cmd),
		TELOPT(opt));
	AXP_TRACE_END();
    }

    /*
     * We now have the command and option.  Run it through the options state
     * machine.  First we need to determine which set of options we are
     * processing, mine or theirs.
     */
    newArg.argc = 2;
    newArg.argp[0] = (void *) ses;
    newArg.argp[1] = (void *) &opt;
    opts = ((ses->cmd == DO) || (ses->cmd == DONT)) ?
	ses->myOptions :
	ses->theirOptions;
    opts[opt].state = AXP_Execute_SM(
			AXP_OPT_MAX_ACTION,
			AXP_OPT_MAX_STATE,
			TN_Option_SM,
			AXP_OPT_ACTION(ses->cmd, opts[opt]),
			opts[opt].state,
			&newArg);
    ses->cmd = 0;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * Cvt_Process_IAC
 *  We actually have an error condition at this point.  We only expected to get
 *  "IAC IAC" or "IAC SE".  A few things may have happened:
 *
 *	1) An IAC was not doubled.
 *	2) The IAC SE was left off.
 *	3) Another option got inserted into the suboption.
 *
 *  We assume that the IAC was not doubled and, in reality the IAC SE was left
 *  off.  If we are not careful, we could get into an infinite loop here.
 *  Therefore, we terminate the suboption and attempt to process what we have
 *  received thus far.  This function is called as part of the receive state
 *  machine processing.
 *
 * Input Parameters:
 *  args:
 *	A pointer to the argument structure containing the number of arguments,
 *	and an array of pointers to each of the arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void Cvt_Process_IAC(AXP_SM_Args *args)
{
    AXP_TELNET_SESSION	*ses = (AXP_TELNET_SESSION *) args->argp[0];
    u8			c = *((u8 *) args->argp[1]);
    u8			iac = IAC;
    AXP_SM_Args		newArg;

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("\tCvt_Process_IAC Called (IAC %s).", TELCMD(c));
	AXP_TRACE_END();
    }

    newArg.argc = 2;
    newArg.argp[0] = (void *) ses;
    newArg.argp[1] = &iac;
    SubOpt_Accumulate(&newArg);
    newArg.argp[1] = &c;
    SubOpt_Accumulate(&newArg);
    newArg.argc = 1;
    SubOpt_TermProcess(&newArg);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * SubOpt_Clear
 *  This function clears the suboption processing because we either completing
 *  processing, or something else happened.  This function is called either as
 *  part of the receive state machine processing, or directly.
 *
 * Input Parameters:
 *  args:
 *	A pointer to the argument structure containing the number of arguments,
 *	and an array of pointers to each of the arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void SubOpt_Clear(AXP_SM_Args *args)
{
    AXP_TELNET_SESSION	*ses = (AXP_TELNET_SESSION *) args->argp[0];

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("\tSubOpt_Clear Called.");
	AXP_TRACE_END();
    }

    /*
     * Reset the suboption processing to it's initial state.
     */
    ses->subOptBufIdx = ses->subOptBufLen = 0;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * SubOpt_Accumulate
 *  This function adds a character to the suboption buffer.  This function is
 *  called either as part of the receive state machine processing or directly.
 *
 * Input Parameters:
 *  args:
 *	A pointer to the argument structure containing the number of arguments,
 *	and an array of pointers to each of the arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void SubOpt_Accumulate(AXP_SM_Args *args)
{
    AXP_TELNET_SESSION	*ses = (AXP_TELNET_SESSION *) args->argp[0];
    u8			c = *((u8 *) args->argp[1]);

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("\tSubOpt_Accumulate Called.");
	AXP_TRACE_END();
    }

    /*
     * If there is more buffer to store the next character, then do so now.
     * Otherwise, ignore it.
     */
    if (ses->subOptBufIdx < AXP_TELNET_SB_LEN)
	ses->subOptBuf[ses->subOptBufIdx++] = c;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * SubOpt_TermProcess
 *  This function terminates the suboption processing and then calls the
 *  function to process the suboption string.  This function is called either
 *  as part of the receive state machine processing, or directly.
 *
 * Input Parameters:
 *  args:
 *	A pointer to the argument structure containing the number of arguments,
 *	and an array of pointers to each of the arguments.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void SubOpt_TermProcess(AXP_SM_Args *args)
{
    AXP_TELNET_SESSION	*ses = (AXP_TELNET_SESSION *) args->argp[0];

    if (AXP_UTL_OPT1)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("\tSubOpt_TermProcess Called.");
	AXP_TRACE_END();
    }

    /*
     * Set the length of the suboption buffer and reset the index back to the
     * beginning.
     */
    ses->subOptBufLen = ses->subOptBufIdx;
    ses->subOptBufIdx = 0;

    Process_Suboption(ses);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * Process_Suboption
 *  This function is called to process a completed suboption buffer.
 *
 * Input Parameters:
 *  ses:
 *	A pointer to the TELNET session structure.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void Process_Suboption(AXP_TELNET_SESSION *ses)
{
    AXP_Telnet_PrintSub('<', ses->subOptBuf, ses->subOptBufLen);

    switch (ses->subOptBuf[ses->subOptBufIdx++])
    {
	case TELOPT_TTYPE:
	    break;
    }
    SubOpt_Clear((void *) ses);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_Telnet_PrintOption
 *  This function is called to trace the option being processed.
 *
 * Input Parameters:
 *  dir:
 *	A value indicating the direction of the option (sent or received).
 *  cmd:
 *	A value indicating the command being traced.
 *  option:
 *	A value indicating the option associated with the command.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
static void AXP_Telnet_PrintOption(char dir, u8 *buf, u32 bufLen)
{
    char	trcBuf[512];
    int		trcIdx = 0;
    u32		idx = 0;
    u8		cmd, option, subOpt;
    char	*fmt;
    char	*opt;

    if (AXP_UTL_BUFF)
    {
	AXP_TRACE_BEGIN();

	/*
	 * If the command is the Interpret As Command, then the option contains
	 * the command.  Otherwise, we have a command with an option.
	 */
	while (idx < bufLen)
	{
	    trcIdx += sprintf(&trcBuf[trcIdx], "\t%s ", DIRECTION(dir));
	    cmd = buf[idx++];
	    option = buf[idx++];
	    if (cmd == IAC)
	    {
		subOpt = buf[idx++];
		trcIdx += sprintf(&trcBuf[trcIdx], "IAC ");
		if (TELCMD_OK(option))
		    trcIdx += sprintf(&trcBuf[trcIdx], "%s ", TELCMD(option));
		else
		    trcIdx += sprintf(&trcBuf[trcIdx], "%d", option);
		if (TELOPT_OK(subOpt))
		    trcIdx += sprintf(&trcBuf[trcIdx], "%s", TELOPT(subOpt));
		else
		    trcIdx += sprintf(&trcBuf[trcIdx], "%d", subOpt);
	    }
	    else
	    {
		fmt = TELCMD(cmd);
		if (fmt)
		{
		    if (TELOPT_OK(option))
			opt = TELOPT(option);
		    else if (option == TELOPT_EXOPL)
			opt = "EXOPL";
		    else
			opt = NULL;

		    if(opt)
			trcIdx += sprintf(&trcBuf[trcIdx], "%s %s", fmt, opt);
		    else
			trcIdx += sprintf(&trcBuf[trcIdx], "%s %d", fmt, option);
		}
		else
		    trcIdx += sprintf(&trcBuf[trcIdx], "%d %d", cmd, option);
	    }
	    AXP_TraceWrite(trcBuf);
	    trcIdx = 0;
	}
	AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_Telnet_PrintSub
 *  This function is called to trace the sub option being processed.
 *
 * Input Parameters:
 *  dir:
 *	A value indicating the direction of the suboption (sent or received).
 *  pointer:
 *	A pointer to a string with the suboption data.
 *  length:
 *	A value indicating the length of the suboption data.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
static void AXP_Telnet_PrintSub(char dir, u8 *pointer, int length)
{
    char	trcBuf[512];
    int		trcIdx = 0;
    int		ii = 0;
    int		jj;

    if (AXP_UTL_BUFF)
    {
	AXP_TRACE_BEGIN();

	/*
	 * If we have a direction, then go ahead and write it out.
	 */
	if (dir != '\0')
	{
	    trcIdx += sprintf(&trcBuf[trcIdx], "\t%s IAC SB ", DIRECTION(dir));
	    if (length >= 3)
	    {
		ii = pointer[length-2];
		jj = pointer[length-1];

		if (ii != IAC || jj != SE)
		{
		    trcIdx += sprintf(&trcBuf[trcIdx], "(terminated by ");
		    if (TELOPT_OK(ii))
			trcIdx += sprintf(&trcBuf[trcIdx], "%s ", TELOPT(ii));
		    else if (TELCMD_OK(ii))
			trcIdx += sprintf(&trcBuf[trcIdx], "%s ", TELCMD(ii));
		    else
			trcIdx += sprintf(&trcBuf[trcIdx], "%d ", ii);
		    if (TELOPT_OK(jj))
			trcIdx += sprintf(&trcBuf[trcIdx], "%s", TELOPT(jj));
		    else if (TELCMD_OK(jj))
			trcIdx += sprintf(&trcBuf[trcIdx], "%s", TELCMD(jj));
		    else
			trcIdx += sprintf(&trcBuf[trcIdx], "%d", jj);
		    trcIdx += sprintf(&trcBuf[trcIdx], ", not IAC SE!) ");
		}
	    }
	    length -= 2;
	}

	/*
	 * We do the following whether we have a direction or not.
	 */
	if (length < 1)
	    trcIdx += sprintf(&trcBuf[trcIdx], "(Empty suboption?)");
	else
	{

	    /*
	     * If the option is a valid one, then we go and print it out.
	     */
	    if (TELOPT_OK(pointer[0]))
	    {
		switch(pointer[0])
		{
		    case TELOPT_TTYPE:
		    case TELOPT_XDISPLOC:
		    case TELOPT_NEW_ENVIRON:
			trcIdx += sprintf(
					&trcBuf[trcIdx],
					"%s",
					TELOPT(pointer[0]));
			break;

		    default:
			trcIdx += sprintf(
					&trcBuf[trcIdx],
					"%s (unsupported)",
					TELOPT(pointer[0]));
			break;
		}
	    }
	    else
		trcIdx += sprintf(&trcBuf[trcIdx], "%d (unknown)", pointer[ii]);

	    switch(pointer[1])
	    {
		case TELQUAL_IS:
		    trcIdx += sprintf(&trcBuf[trcIdx], " IS");
		    break;

		case TELQUAL_SEND:
		    trcIdx += sprintf(&trcBuf[trcIdx], " SEND");
		    break;

		case TELQUAL_INFO:
		    trcIdx += sprintf(&trcBuf[trcIdx], " INFO/REPLY");
		    break;

		case TELQUAL_NAME:
		    trcIdx += sprintf(&trcBuf[trcIdx], " NAME");
		    break;
	    }

	    switch(pointer[0])
	    {
		case TELOPT_TTYPE:
		case TELOPT_XDISPLOC:
		    pointer[length] = 0;
		    trcIdx += sprintf(&trcBuf[trcIdx], " \"%s\"", &pointer[2]);
		    break;

		case TELOPT_NEW_ENVIRON:
		    if(pointer[1] == TELQUAL_IS)
		    {
			trcIdx += sprintf(&trcBuf[trcIdx], " ");
			for(ii = 3; ii < length; ii++)
			{
			    switch(pointer[ii])
			    {
				case NEW_ENV_VAR:
				    trcIdx += sprintf(&trcBuf[trcIdx], ", ");
				    break;

				case NEW_ENV_VALUE:
				    trcIdx += sprintf(&trcBuf[trcIdx], " = ");
				    break;

				default:
				    trcIdx += sprintf(
						&trcBuf[trcIdx],
						"%c",
						pointer[ii]);
				    break;
			    }
			}
		    }
		    break;

		default:
		    for (ii = 2; ii < length; ii++)
			trcIdx += sprintf(
					&trcBuf[trcIdx],
					" %.2x",
					pointer[ii]);
		    break;
	    }
	}
	AXP_TraceWrite(trcBuf);
	trcIdx = 0;
	AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_Telnet_Listener
 *  This function is called to create the port listener.  It gets the port on
 *  which it is supposed to be listening from the configuration.  It will
 *  default to 108 (snagas - Digital SNA Gateway Access Protocol).
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  sock:
 *	A location to receive the socket on which to accept connections.
 *
 * Return Values:
 *  true:	Socket returned to be used used to accept connection requests.
 *  false:	Failed to create, bind, or define a listener for the socket.
 */
static bool AXP_Telnet_Listener(int *sock)
{
    struct sockaddr_in	myName;
    bool		retVal = true;

    /*
     * First things first, we need a socket onto which we will listen for
     * connections.
     */
    *sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*sock >= 0)
    {
	myName.sin_family = AF_INET;
	myName.sin_addr.s_addr = INADDR_ANY;
	myName.sin_port = htons(AXP_TELNET_DEFAULT_PORT);
    }
    else
	retVal = false;

    /*
     * Now bind the name to the socket
     */
    if (retVal == true)
	retVal = bind(*sock, (struct sockaddr *) &myName, sizeof(myName)) >= 0;

    /*
     * Now set up a listener on the socket (only allow one connection request
     * into the listener queue).
     */
    if (retVal == true)
	retVal = listen(*sock, 1) >= 0;

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * AXP_Telnet_Accept
 *  This function is called to wait for the next connection to be requested and
 *  accept it.
 *
 * Input Parameters:
 *  sock:
 *	The value of the socket on which to accept connections.
 *
 * Output Parameters:
 *  ses:
 *	A location to receive the session information on which data is sent and
 *	received to and from the client.
 *
 * Return Values:
 *  NULL:	An error occurred and the connection was not accepted.
 *  <address>:	The address of a TELNET session block used to maintain the
 *		TELNET connection with the client (we are the server).
 */
static AXP_TELNET_SESSION *AXP_Telnet_Accept(int sock)
{
    AXP_SM_Args		args;
    AXP_TELNET_SESSION	*ses = NULL;
    struct sockaddr	theirName;
    int			theirNameSize = sizeof(theirName);

    /*
     * Go allocate a block into which TELNET session information can be
     * maintained throughout the life of the connection with the client.
     */
    ses = (AXP_TELNET_SESSION *) AXP_Allocate_Block(AXP_TELNET_SES_BLK);

    /*
     * Try to accept a connection.  If it fails, return the block, otherwise
     * go ahead initialize and return it back to the caller.
     */
    printf("Ready to accept a TELNET connection...\n");
    ses->mySocket = accept(sock, &theirName, &theirNameSize);
    if (ses->mySocket < 0)
    {
	AXP_Deallocate_Block((AXP_BLOCK_DSC *) ses);
	ses = NULL;
	printf("Accepting a TELNET connection has failed...\n");
    }
    else
    {
	AXP_OPT_SET_PREF(ses->myOptions, TELOPT_ECHO);
	AXP_OPT_SET_PREF(ses->myOptions, TELOPT_SGA);
	AXP_OPT_SET_SUPP(ses->myOptions, TELOPT_TTYPE);
	AXP_OPT_SET_SUPP(ses->myOptions, TELOPT_NEW_ENVIRON);
	AXP_OPT_SET_PREF(ses->theirOptions, TELOPT_ECHO);
	AXP_OPT_SET_PREF(ses->theirOptions, TELOPT_SGA);
	AXP_OPT_SET_PREF(ses->theirOptions, TELOPT_NAWS);
	AXP_OPT_SET_PREF(ses->theirOptions, TELOPT_LFLOW);
	ses->rcvState = AXP_RCV_DATA;
	args.argc = 1;
	args.argp[0] = (void *) ses;
	SubOpt_Clear(&args);
	printf("A TELNET connection has been accepted...\n");
    }

    /*
     * Return back to the caller.
     */
    return(ses);
}

/*
 * AXP_Telnet_Receive
 *  This function is called to wait for the next message to be sent from the
 *  TELNET client and return it back to the caller.
 *
 * Input Parameters:
 *  ses:
 *	A pointer to the session variable used to maintain the TELNET session.
 *  bufLen:
 *	A pointer to a location indicating the number of bytes in the buf
 *	parameter.
 *
 * Output Parameters:
 *  buf:
 *	A location to receive the data received from the TELNET client.
 *  bufLen:
 *	A pointer to a location to receive the number of bytes being returned
 *	in the buf parameter.
 *
 * Return Values:
 *  true:	The buf and bufLen parameters contain valid information.
 *  false:	Failure.
 */
static bool AXP_Telnet_Receive(AXP_TELNET_SESSION *ses, u8 *buf, u32 *bufLen)
{
    bool	retVal = true;

    /*
     * Receive up to a buffers worth of data.  Since we are using a
     * steam protocol, we only may receive part of a complete buffer.
     * A buffer's last character should be a null character.
     */
    *bufLen = recv(ses->mySocket, buf, *bufLen, 0);

    /*
     * If the receive length is less than or equal to zero, we assume
     * the connection has been terminated for one reason or other (them
     * or us).
     */
    if (*bufLen <= 0)
	retVal = false;

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * AXP_Telnet_Send
 *  This function is called to send data to the TELNET client.
 *
 * Input Parameters:
 *  ses:
 *	A pointer to the session variable used to maintain the TELNET session.
 *  buf:
 *	A location containing the data to be sent to the TELNET client.
 *  bufLen:
 *	A value indicating the number of bytes in the buf parameter.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	The data in the buf parameter was sent.
 *  false:	Failure.
 */
bool AXP_Telnet_Send(AXP_TELNET_SESSION *ses, u8 *buf, int bufLen)
{
    bool	retVal = true;

    if (AXP_UTL_CALL)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("AXP_Telnet_Send called.");
	AXP_TRACE_END();
    }

    /*
     * Receive up to a buffers worth of data.  Since we are using a
     * steam protocol, we only may receive part of a complete buffer.
     * A buffer's last character should be a null character.
     */
    if (TELCMD_OK(buf[0]))
	AXP_Telnet_PrintOption('>', buf, bufLen);
    else
    {
	buf[bufLen] = '\0';
    }
    bufLen = send(ses->mySocket, buf, bufLen, 0);

    /*
     * If the sent length is less than or equal to zero, we assume the
     * connection has been terminated for one reason or other (them or us).
     */
    if (bufLen <= 0)
	retVal = false;

    if (AXP_UTL_CALL)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite(
		"AXP_Telnet_Send returning, %s.",
		(retVal ? "Success" : "Failure"));
	AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * AXP_Telnet_Reject
 *  This function is called to close the connection with a TELNET client.  This
 *  does not close the socket used to receive connection requests.
 *
 * Input Parameters:
 *  sock:
 *	The value of the socket to be closed.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	The TELNET socket has been closed.
 *  false:	Failure.
 */
static bool AXP_Telnet_Reject(AXP_TELNET_SESSION *ses)
{
    bool	retVal = true;

    /*
     * Close the socket.
     */
    close(ses->mySocket);

    /*
     * Return the block of memory back to the system.
     */
    AXP_Deallocate_Block((AXP_BLOCK_DSC *) ses);
    ses = NULL;
    printf("TELNET session has been closed...\n");

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * AXP_Telnet_Ignore
 *  This function is called to close the socket used to receive connection
 *  requests.
 *
 * Input Parameters:
 *  sock:
 *	The value of the socket to be closed.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	The Listener socket has been closed.
 *  false:	Failure.
 */
static bool AXP_Telnet_Ignore(int sock)
{
    bool	retVal = true;

    /*
     * Close the socket.
     */
    close(sock);

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * AXP_Telent_Processor
 *  This function is called with a socket, buffer, and buffer length.  The
 *  buffer contains one or more bytes of data that may contain one or more
 *  TELNET commands.  This function process through this data, and when
 *  necessary sends a response in kind.
 *
 * Input Parameters:
 *  sock:
 *	The value of the socket on which to send data, if necessary.
 *  buf:
 *	A location containing the data to be processed.
 *  bufLen:
 *	A value indicating the number of bytes in the buf parameter.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  true:	The data has been processed.
 *  false:	Failure.
 */
static bool AXP_Telnet_Processor(AXP_TELNET_SESSION *ses, u8 *buf, u32 bufLen)
{
    AXP_SM_Args	args;
    bool	retVal = true;
    int		ii;

    /*
     * At this point we want to perform some action.
     */
    if (TELCMD_OK(buf[0]))
	AXP_Telnet_PrintOption('<', buf, bufLen);
    else
    {
	buf[bufLen] = '\0';
    }

    ii = 0;
    args.argc = 2;
    args.argp[0] = (void *) ses;
    while ((ii < bufLen) && (retVal == true))
    {
	args.argp[1] = (void *) &buf[ii];
	ses->rcvState = AXP_Execute_SM(
			AXP_ACT_MAX,
			AXP_RCV_MAX_STATE,
			TN_Receive_SM,
			AXP_RCV_ACTION(buf[ii]),
			ses->rcvState,
			&args);
	if ((srvState != Negotiating) && (srvState != Active))
	    retVal = false;
	ii++;
    }

    /*
     * Return back to the caller.
     */
    return(retVal);
}

/*
 * AXP_Telnet_Main
 *  This function is called to establish the listener socket, accept connection
 *  requests, one at a time, for a TELNET connection, receive data from the
 *  TELNET client, process it as necessary, occasionally send a response back,
 *  all until either the TELNET client goes away or we are shutting down.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void AXP_Telnet_Main(void)
{
    AXP_SM_Args			args;
    u8				buffer[AXP_TELNET_MSG_LEN];
    u32				bufferLen;
    int				connSock;
    int				ii;
    AXP_TELNET_SESSION		*ses = NULL;
    bool			retVal = true;

    if (AXP_UTL_CALL)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("TELNET Server is starting");
	AXP_TRACE_END();
    }

    while(srvState != Finished)
    {
	switch(srvState)
	{
	    case Listen:
		retVal = AXP_Telnet_Listener(&connSock);
		srvState = retVal ? Accept : Closing;
		break;

	    case Accept:
		ses = AXP_Telnet_Accept(connSock);
		if (ses != NULL)
		{

		    /*
		     * If the client does not send us any options to be
		     * negotiated, then it probably is not a TELNET client.
		     */
		    bufferLen = AXP_TELNET_MSG_LEN;
		    retVal = AXP_Telnet_Receive(ses, buffer, &bufferLen);
		    srvState = retVal ? Negotiating : Listen;
		    if (retVal == true)
		    {
			retVal = AXP_Telnet_Processor(ses, buffer, bufferLen);
			if (retVal == false)
			    srvState = Listen;
		    }
		}
		else
		    srvState = Listen;
		break;

	    case Negotiating:
		for (ii = 0; ii < NTELOPTS; ii++)
		{
		    args.argc = 2;
		    args.argp[0] = (void *) ses;
		    args.argp[1] = (void *) &ii;
		    if (ses->myOptions[ii].preferred == true)
		    {
			ses->myOptions[ii].state = AXP_Execute_SM(
				AXP_OPT_MAX_ACTION,
				AXP_OPT_MAX_STATE,
				TN_Option_SM,
				AXP_OPT_ACTION(YES_SRV, ses->myOptions[ii]),
				ses->myOptions[ii].state,
				&args);
		    }
		    if (ses->theirOptions[ii].preferred == true)
		    {
			ses->theirOptions[ii].state = AXP_Execute_SM(
				AXP_OPT_MAX_ACTION,
				AXP_OPT_MAX_STATE,
				TN_Option_SM,
				AXP_OPT_ACTION(YES_CLI, ses->theirOptions[ii]),
				ses->theirOptions[ii].state,
				&args);
		    }
		}

		/*
		 * One of the things that could have happened is that while
		 * possibly sending to the client, the connection was reset or
		 * terminated.  If this is the case, then the server state has
		 * already been changed.  Otherwise, the next state is Active.
		 */
		if (srvState == Negotiating)
		    srvState = Active;
		break;

	    case Active:
		while (srvState == Active)
		{
		    bufferLen = AXP_TELNET_MSG_LEN;
		    retVal = AXP_Telnet_Receive(ses, buffer, &bufferLen);
		    if (retVal == true)
			retVal = AXP_Telnet_Processor(ses, buffer, bufferLen);
		    if (retVal == false)
			srvState = Inactive;
		}
		break;

	    case Inactive:
		retVal = AXP_Telnet_Reject(ses);
		ses = NULL;
		srvState = Listen;
		break;

	    case Closing:
		retVal = AXP_Telnet_Ignore(connSock);
		srvState = Finished;
		break;

	    case Finished:
		break;
	}
    }

    if (AXP_UTL_CALL)
    {
	AXP_TRACE_BEGIN();
	AXP_TraceWrite("TELNET Server is exiting.");
	AXP_TRACE_END();
    }

    /*
     * Return back to the caller.
     */
    return;
}
