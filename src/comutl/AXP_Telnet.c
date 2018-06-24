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
 * TODO: Make these static after testing.
 *
 * Action Routines for the state machines.
 */
void Send_DO(void *, ...);
void Send_DONT(void *, ...);
void Send_WILL(void *, ...);
void Send_WONT(void *, ...);
void Echo_Data(void *, ...);
void Process_CMD(void *, ...);
void Cvt_Process_IAC(void *, ...);
void SubOpt_Clear(void *, ...);
void SubOpt_Accumulate(void *, ...);
void SubOpt_TermProcess(void *, ...);
void Cvt_Proc_CMD(void *, ...);
void Process_Suboption(void *, ...);

/*
 * This definition below is used for processing the options sent from the
 * client and ones we want to send to the client.
 */
AXP_StateMachine TN_Option_SM[AXP_OPT_MAX_ACTION][AXP_OPT_MAX_STATE] =
{
    /* YES_SRV	- NOT PREFERRED */
    {
	{AXP_OPT_WANTYES_LOCAL,	Send_WILL},
	{AXP_OPT_WANTNO_REMOTE,	NULL},
	{AXP_OPT_WANTNO_REMOTE,	NULL},
	{AXP_OPT_WANTYES_LOCAL,	NULL},
	{AXP_OPT_WANTYES_LOCAL,	NULL},
	{AXP_OPT_YES,		NULL}
    },
    /* YES_SRV	- PREFERRED */
    {
	{AXP_OPT_WANTYES_LOCAL,	Send_WILL},
	{AXP_OPT_WANTNO_REMOTE,	NULL},
	{AXP_OPT_WANTNO_REMOTE,	NULL},
	{AXP_OPT_WANTYES_LOCAL,	NULL},
	{AXP_OPT_WANTYES_LOCAL,	NULL},
	{AXP_OPT_YES,		NULL}
    },
    /* NO_SRV	- NOT PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTNO_LOCAL,	NULL},
	{AXP_OPT_WANTNO_LOCAL,	NULL},
	{AXP_OPT_WANTYES_REMOTE,NULL},
	{AXP_OPT_WANTYES_REMOTE,NULL},
	{AXP_OPT_WANTNO_LOCAL,	Send_WONT}
    },
    /* NO_SRV	- PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTNO_LOCAL,	NULL},
	{AXP_OPT_WANTNO_LOCAL,	NULL},
	{AXP_OPT_WANTYES_REMOTE,NULL},
	{AXP_OPT_WANTYES_REMOTE,NULL},
	{AXP_OPT_WANTNO_LOCAL,	Send_WONT}
    },
    /* YES_CLI	- NOT PREFERRED */
    {
	{AXP_OPT_WANTYES_LOCAL,	Send_DO},
	{AXP_OPT_WANTNO_REMOTE,	NULL},
	{AXP_OPT_WANTNO_REMOTE,	NULL},
	{AXP_OPT_WANTYES_LOCAL,	NULL},
	{AXP_OPT_WANTYES_LOCAL,	NULL},
	{AXP_OPT_YES,		NULL}
    },
    /* YES_CLI	- PREFERRED */
    {
	{AXP_OPT_WANTYES_LOCAL,	Send_DO},
	{AXP_OPT_WANTNO_REMOTE,	NULL},
	{AXP_OPT_WANTNO_REMOTE,	NULL},
	{AXP_OPT_WANTYES_LOCAL,	NULL},
	{AXP_OPT_WANTYES_LOCAL,	NULL},
	{AXP_OPT_YES,		NULL}
    },
    /* NO_CLI	- NOT PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTNO_LOCAL,	NULL},
	{AXP_OPT_WANTNO_LOCAL,	NULL},
	{AXP_OPT_WANTYES_REMOTE,NULL},
	{AXP_OPT_WANTYES_REMOTE,NULL},
	{AXP_OPT_WANTNO_LOCAL,	Send_DONT}
    },
    /* NO_CLI	- PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTNO_LOCAL,	NULL},
	{AXP_OPT_WANTNO_LOCAL,	NULL},
	{AXP_OPT_WANTYES_REMOTE,NULL},
	{AXP_OPT_WANTYES_REMOTE,NULL},
	{AXP_OPT_WANTNO_LOCAL,	Send_DONT}
    },
    /* WILL	- NOT PREFERRED */
    {
	{AXP_OPT_NO,		Send_DONT},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_WANTNO_LOCAL,	Send_DONT},
	{AXP_OPT_YES,		NULL}
    },
    /* WILL	- PREFERRED */
    {
	{AXP_OPT_YES,		Send_DO},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_WANTNO_LOCAL,	Send_DONT},
	{AXP_OPT_YES,		NULL}
    },
    /* WONT	- NOT PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTYES_LOCAL,	Send_DO},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		Send_DONT}
    },
    /* WONT	- PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTYES_LOCAL,	Send_DO},
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
	{AXP_OPT_WANTNO_LOCAL,	Send_WONT},
	{AXP_OPT_YES,		NULL}
    },
    /* DO	- PREFERRED */
    {
	{AXP_OPT_YES,		Send_WILL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_YES,		NULL},
	{AXP_OPT_WANTNO_LOCAL,	Send_WONT},
	{AXP_OPT_YES,		NULL}
    },
    /* DONT	- NOT PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTYES_LOCAL,	Send_WILL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		Send_WONT}
    },
    /* DONT	- PREFERRED */
    {
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_NO,		NULL},
	{AXP_OPT_WANTYES_LOCAL,	Send_WILL},
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
	{AXP_RCV_CMD,		NULL},
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

#define DIRECTION(dir)	(((dir) == '<') ? "RCVD" : "SENT")

/*
 * Local Prototypes.
 */
static void AXP_Telnet_PrintOption(char, u8, u8);
static void AXP_Telnet_PrintSub(char, u8 *, int);
static bool AXP_Telnet_Listener(int *);
static bool AXP_Telnet_Accept(int, AXP_Telnet_Session *);
static bool AXP_Telnet_Receive(AXP_Telnet_Session *, u8 *, u32 *);
static bool AXP_Telnet_Reject(AXP_Telnet_Session *);
static bool AXP_Telnet_Ignore(int);
static bool AXP_Telnet_Processor(AXP_Telnet_Session *, u8 *, u32);

/*
 * <State Machine Action Routines>
 *  The following set of functions are all action routines called by the
 *  execution of the state machine.  These functions perform the actions that
 *  are required when an action is applied to a state machine.
 *
 * Input Parameters:
 *  sesPtr:
 *	A pointer to the TELNET session structure.
 * ...:
 *	A variable number of arguments to be used for the action processing.
 *
 * Output Parameters:
 *  None.
 *
 * Return Value:
 *  None.
 */
void Send_DO(void *sesPtr, ...)
{
    AXP_Telnet_Session *ses = (AXP_Telnet_Session *) sesPtr;
    u8		opt;
    u8		buf[3];
    va_list	ap;

    va_start(ap, sesPtr);
    opt = va_arg(ap, int);
    va_end(ap);

    /*
     * Send the IAC DO <opt> to the client.
     */
    buf[0] = IAC;
    buf[1] = DO;
    buf[2] = opt;
    AXP_Telnet_Send(ses, buf, 3);

    /*
     * Return back to the caller.
     */
    return;
}
void Send_DONT(void *sesPtr, ...)
{
    AXP_Telnet_Session *ses = (AXP_Telnet_Session *) sesPtr;
    u8		opt;
    u8		buf[3];
    va_list	ap;

    va_start(ap, sesPtr);
    opt = va_arg(ap, int);
    va_end(ap);

    /*
     * Send the IAC DONT <opt> to the client.
     */
    buf[0] = IAC;
    buf[1] = DONT;
    buf[2] = opt;
    AXP_Telnet_Send(ses, buf, 3);

    /*
     * Return back to the caller.
     */
    return;
}
void Send_WILL(void *sesPtr, ...)
{
    AXP_Telnet_Session *ses = (AXP_Telnet_Session *) sesPtr;
    u8		opt;
    u8		buf[3];
    va_list	ap;

    va_start(ap, sesPtr);
    opt = va_arg(ap, int);
    va_end(ap);

    /*
     * Send the IAC WILL <opt> to the client.
     */
    buf[0] = IAC;
    buf[1] = WILL;
    buf[2] = opt;
    AXP_Telnet_Send(ses, buf, 3);

    /*
     * Return back to the caller.
     */
    return;
}
void Send_WONT(void *sesPtr, ...)
{
    AXP_Telnet_Session *ses = (AXP_Telnet_Session *) sesPtr;
    u8		opt;
    u8		buf[3];
    va_list	ap;

    va_start(ap, sesPtr);
    opt = va_arg(ap, int);
    va_end(ap);

    /*
     * Send the IAC WONT <opt> to the client.
     */
    buf[0] = IAC;
    buf[1] = WONT;
    buf[2] = opt;
    AXP_Telnet_Send(ses, buf, 3);

    /*
     * Return back to the caller.
     */
    return;
}
void Echo_Data(void *sesPtr, ...)
{
    AXP_Telnet_Session *ses = (AXP_Telnet_Session *) sesPtr;

    /*
     * Return back to the caller.
     */
    return;
}
void Process_CMD(void *sesPtr, ...)
{
    AXP_Telnet_Session *ses = (AXP_Telnet_Session *) sesPtr;

    /*
     * Return back to the caller.
     */
    return;
}
void Cvt_Process_IAC(void *sesPtr, ...)
{
    AXP_Telnet_Session *ses = (AXP_Telnet_Session *) sesPtr;

    /*
     * Return back to the caller.
     */
    return;
}
void SubOpt_Clear(void *sesPtr, ...)
{
    AXP_Telnet_Session *ses = (AXP_Telnet_Session *) sesPtr;

    /*
     * Return back to the caller.
     */
    return;
}
void SubOpt_Accumulate(void *sesPtr, ...)
{
    AXP_Telnet_Session *ses = (AXP_Telnet_Session *) sesPtr;

    /*
     * Return back to the caller.
     */
    return;
}
void SubOpt_TermProcess(void *sesPtr, ...)
{
    AXP_Telnet_Session *ses = (AXP_Telnet_Session *) sesPtr;

    /*
     * Return back to the caller.
     */
    return;
}
void Process_Suboption(void *sesPtr, ...)
{
    AXP_Telnet_Session *ses = (AXP_Telnet_Session *) sesPtr;

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
static void AXP_Telnet_PrintOption(char dir, u8 cmd, u8 option)
{
    char *fmt;
    char *opt;

    /*
     * If the command is the Interpret As Command, then the option contains the
     * command.  Otherwise, we have a command with an option.
     */
    printf("%s ", DIRECTION(dir));
    if (cmd == IAC)
    {
	if (TELCMD_OK(option))
	    printf("IAC %s", TELCMD(option));
	else
	    printf("IAC %d", option);
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
		printf("%s %s", fmt, opt);
	    else
		printf("%s %d", fmt, option);
	}
	else
	    printf("%d %d", cmd, option);
    }
    printf("\n");

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
    int ii = 0;
    int jj;

    /*
     * If we have a direction, then go ahead and write it out.
     */
    if (dir != '\0')
    {
	printf("%s IAC SB ", DIRECTION(dir));
	if (length >= 3)
	{

	    ii = pointer[length-2];
	    jj = pointer[length-1];

	    if (ii != IAC || jj != SE)
	    {
		printf("(terminated by ");
		if (TELOPT_OK(ii))
		    printf("%s ", TELOPT(ii));
		else if (TELCMD_OK(ii))
		    printf("%s ", TELCMD(ii));
		else
		    printf("%d ", ii);
		if (TELOPT_OK(jj))
		    printf("%s", TELOPT(jj));
		else if (TELCMD_OK(jj))
		    printf("%s", TELCMD(jj));
		else
		    printf("%d", jj);
		printf(", not IAC SE!) ");
	    }
	}
	length -= 2;
    }

    /*
     * We do the following whether we have a direction or not.
     */
    if (length < 1)
    {
	printf("(Empty suboption?)");
	return;
    }

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
		printf("%s", TELOPT(pointer[0]));
		break;

	    default:
		printf("%s (unsupported)", TELOPT(pointer[0]));
		break;
	}
    }
    else
	printf("%d (unknown)", pointer[ii]);

    switch(pointer[1])
    {
	case TELQUAL_IS:
	    printf(" IS");
	    break;

	case TELQUAL_SEND:
	    printf(" SEND");
	    break;

	case TELQUAL_INFO:
	    printf(" INFO/REPLY");
	    break;

	case TELQUAL_NAME:
	    printf(" NAME");
	    break;
    }

    switch(pointer[0])
    {
	case TELOPT_TTYPE:
	case TELOPT_XDISPLOC:
	    pointer[length] = 0;
	    printf(" \"%s\"", &pointer[2]);
	    break;

	case TELOPT_NEW_ENVIRON:
	    if(pointer[1] == TELQUAL_IS)
	    {
		printf(" ");
		for(ii = 3; ii < length; ii++)
		{
		    switch(pointer[ii])
		    {
			case NEW_ENV_VAR:
			    printf(", ");
			    break;

			case NEW_ENV_VALUE:
			    printf(" = ");
			    break;

			default:
			    printf("%c", pointer[ii]);
			    break;
		    }
		}
	    }
	    break;

	default:
	    for (ii = 2; ii < length; ii++)
		printf(" %.2x", pointer[ii]);
	    break;
    }
    printf("\n");

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
 *  true:	Socket to use to send and receive data.
 *  false:	Failure.
 */
static bool AXP_Telnet_Accept(int sock, AXP_Telnet_Session *ses)
{
    struct sockaddr	theirName;
    int			theirNameSize = sizeof(theirName);
    bool		retVal = true;

    /*
     * We loop forever accepting connections.
     */
    printf("Ready to accept a TELNET connection...\n");
    ses->mySocket = accept(sock, &theirName, &theirNameSize);
    if (ses->mySocket < 0)
	retVal = false;
    else
	printf("A TELNET connection has been accepted...\n");

    /*
     * Return back to the caller.
     */
    return(retVal);
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
static bool AXP_Telnet_Receive(AXP_Telnet_Session *ses, u8 *buf, u32 *bufLen)
{
    bool	retVal = true;

    /*
     * Receive up to a buffers worth of data.  Since we are using a
     * steam protocol, we only may receive part of a complete buffer.
     * A buffer's last character should be a null character.
     */
    printf("Ready to receive data...\n");
    *bufLen = recv(ses->mySocket, buf, *bufLen, 0);
    printf("recv returned %d bytes of data\n", *bufLen);

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
bool AXP_Telnet_Send(AXP_Telnet_Session *ses, u8 *buf, u32 bufLen)
{
    bool	retVal = true;
    int		ii;

    /*
     * Receive up to a buffers worth of data.  Since we are using a
     * steam protocol, we only may receive part of a complete buffer.
     * A buffer's last character should be a null character.
     */
    printf("Sending data...\n\t");
    for (ii = 0; ii < bufLen; ii++)
    {
	printf("%02x(", buf[ii]);
	if (TELCMD_OK(buf[ii]))
	{
	    printf("%s) ", TELCMD(buf[ii++]));
	    printf("%02x(", buf[ii]);
	    if (TELCMD_OK(buf[ii]))
	    {
		printf("%s", TELCMD(buf[ii]));
		if ((buf[ii] == WILL) ||
		    (buf[ii] == WONT) ||
		    (buf[ii] == DO) ||
		    (buf[ii] == DONT))
		{
		    ii++;
		    printf(") %02x(", buf[ii]);
		    if (TELOPT_OK(buf[ii]))
			printf("%s", TELOPT(buf[ii]));
		    else
			printf("%u", buf[ii]);
		}
	    }
	    else
		printf("%u", buf[ii]);
	}
	else
	    printf("%u", buf[ii]);
	printf(") ");
    }
    bufLen = send(ses->mySocket, buf, bufLen, 0);
    printf("\nsend sent %d bytes of data\n", bufLen);

    /*
     * If the sent length is less than or equal to zero, we assume the
     * connection has been terminated for one reason or other (them or us).
     */
    if (bufLen <= 0)
	retVal = false;

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
static bool AXP_Telnet_Reject(AXP_Telnet_Session *ses)
{
    bool	retVal = true;

    /*
     * Close the socket.
     */
    close(ses->mySocket);

    ses->mySocket = 0;

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
static bool AXP_Telnet_Processor(AXP_Telnet_Session *ses, u8 *buf, u32 bufLen)
{
    bool	retVal = true;
    int		ii;

    /*
     * At this point we want to perform some action.
     */
    printf("Just received the following...\n\t");

    ii = 0;
    while (ii < bufLen)
    {
	printf("%02x(", buf[ii]);
	if (TELCMD_OK(buf[ii]))
	{
	    printf("%s", TELCMD(buf[ii]));
	    if (buf[ii] == IAC)
	    {
		ii++;
		switch (buf[ii])
		{
		    case DONT:	/* you are not to use option */
		    case DO:	/* please, you use option */
		    case WONT:	/* I won't use option */
		    case WILL:	/* I will use option */
			printf(") %02x(%s", buf[ii], TELCMD(buf[ii]));
			ii++;
			if (TELOPT_OK(buf[ii]))
			{
			    printf(") %02x(%s", buf[ii], TELOPT(buf[ii]));
			    switch(buf[ii])
			    {
				case TELOPT_BINARY:	/* 8-bit data path */
				    break;

				case TELOPT_ECHO:	/* echo */
				    break;

				case TELOPT_RCP:	/* prepare to reconnect */
				    break;

				case TELOPT_SGA:	/* suppress go ahead */
				    break;

				case TELOPT_NAMS:	/* approximate message size */
				case TELOPT_STATUS:	/* give status */
				case TELOPT_TM:		/* timing mark */
				case TELOPT_RCTE:	/* remote controlled transmission and echo */
				case TELOPT_NAOL:	/* negotiate about output line width */
				case TELOPT_NAOP:	/* negotiate about output page size */
				case TELOPT_NAOCRD:	/* negotiate about CR disposition */
				case TELOPT_NAOHTS:	/* negotiate about horizontal tabstops */
				case TELOPT_NAOHTD:	/* negotiate about horizontal tab disposition */
				case TELOPT_NAOFFD:	/* negotiate about formfeed disposition */
				case TELOPT_NAOVTS:	/* negotiate about vertical tab stops */
				case TELOPT_NAOVTD:	/* negotiate about vertical tab disposition */
				case TELOPT_NAOLFD:	/* negotiate about output LF disposition */
				case TELOPT_XASCII:	/* extended ascic character set */
				case TELOPT_LOGOUT:	/* force logout */
				case TELOPT_BM:		/* byte macro */
				case TELOPT_DET:	/* data entry terminal */
				case TELOPT_SUPDUP:	/* supdup protocol */
				case TELOPT_SUPDUPOUTPUT:/* supdup output */
				case TELOPT_SNDLOC:	/* send location */
				    break;

				case TELOPT_TTYPE:	/* terminal type */
				    break;

				case TELOPT_EOR:	/* end or record */
				case TELOPT_TUID:	/* TACACS user identification */
				case TELOPT_OUTMRK:	/* output marking */
				case TELOPT_TTYLOC:	/* terminal location number */
				case TELOPT_3270REGIME:	/* 3270 regime */
				case TELOPT_X3PAD:	/* X.3 PAD */
				    break;

				case TELOPT_NAWS:	/* window size */
				    break;

				case TELOPT_TSPEED:	/* terminal speed */
				    break;

				case TELOPT_LFLOW:	/* remote flow control */
				case TELOPT_LINEMODE:	/* Linemode option */
				case TELOPT_XDISPLOC:	/* X Display Location */
				case TELOPT_OLD_ENVIRON:/* Old - Environment variables */
				case TELOPT_AUTHENTICATION:/* Authenticate */
				case TELOPT_ENCRYPT:	/* Encryption option */
				case TELOPT_NEW_ENVIRON:/* New - Environment variables */
				    break;

				case TELOPT_EXOPL:	/* extended-options-list */
				    break;
			    }
			}
			else
			    printf(") %02x(%d", buf[ii], buf[ii]);
			break;

		    case SB:	/* interpret as subnegotiation */
		    case GA:	/* you may reverse the line */
		    case EL:	/* erase the current line */
		    case EC:	/* erase the current character */
		    case AYT:	/* are you there */
		    case AO:	/* abort output--but let prog finish */
		    case IP:	/* interrupt process--permanently */
		    case BREAK:	/* break */
		    case DM:	/* data mark--for connect. cleaning */
		    case NOP:	/* nop */
		    case SE:	/* end sub negotiation */
		    case EOR:	/* end of record (transparent mode) */
		    case ABORT:	/* Abort process */
		    case SUSP:	/* Suspend process */
		    case xEOF:	/* End of file: EOF is already used... */
			break;
		}
	    }
	}
	else
	{
	    printf("%u", buf[ii]);
	    retVal = AXP_Telnet_Send(ses, buf, bufLen);
	}
	printf(") ");
	ii++;
    }
    printf("\n");

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
    u8				buffer[AXP_TELNET_MSG_LEN];
    u32				bufferLen;
    int				connSock;
    AXP_Telnet_Session		ses = {.myState = Accept};
    bool			retVal = true;

    while(ses.myState != Finished)
    {
	switch(ses.myState)
	{
	    case Listen:
		retVal = AXP_Telnet_Listener(&connSock);
		ses.myState = retVal ? Accept : Closing;
		break;

	    case Accept:
		retVal = AXP_Telnet_Accept(connSock, &ses);
		if (retVal == true)
		{

		    /*
		     * If the client does not send us any options to be
		     * negotiated, then it probably is not a TELNET client.
		     */
		    bufferLen = AXP_TELNET_MSG_LEN;
		    retVal = AXP_Telnet_Receive(&ses, buffer, &bufferLen);
		    ses.myState = retVal ? Negotiating : Listen;
		}
		else
		    ses.myState = Listen;
		break;

	    case Negotiating:
		break;

	    case Active:
		while (ses.myState == Active)
		{
		    bufferLen = AXP_TELNET_MSG_LEN;
		    retVal = AXP_Telnet_Receive(&ses, buffer, &bufferLen);
		    if (retVal == true)
			retVal = AXP_Telnet_Processor(
						&ses,
						buffer,
						bufferLen);
		    if (retVal == false)
			ses.myState = Inactive;
		}
		break;

	    case Inactive:
		retVal = AXP_Telnet_Reject(&ses);
		ses.myState = Listen;
		break;

	    case Closing:
		retVal = AXP_Telnet_Ignore(connSock);
		ses.myState = Finished;
		break;

	    case Finished:
		break;
	}
    }

    /*
     * Return back to the caller.
     */
    return;
}
