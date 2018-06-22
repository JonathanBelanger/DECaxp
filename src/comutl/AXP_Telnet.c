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
#include "AXP_Telnet.h"
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/*
 * Local Prototypes.
 */
static bool AXP_Telnet_Listener(int *);
static bool AXP_Telnet_Accept(int, int *);
static bool AXP_Telnet_Receive(int, u8 *, u32 *);
static bool AXP_Telnet_Reject(int);
static bool AXP_Telnet_Ignore(int);
static bool AXP_Telnet_Processor(int, u8 *, u32);

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
 *  sessionSock:
 *	A location to receive the socket on which to send and receive data.
 *
 * Return Values:
 *  true:	Socket to use to send and receive data.
 *  false:	Failure.
 */
static bool AXP_Telnet_Accept(int sock, int *sessionSock)
{
    struct sockaddr	theirName;
    int			theirNameSize = sizeof(theirName);
    bool		retVal = true;

    /*
     * We loop forever accepting connections.
     */
    printf("Ready to accept a TELNET connection...\n");
    *sessionSock = accept(sock, &theirName, &theirNameSize);
    if (*sessionSock < 0)
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
 *  sock:
 *	The value of the socket on which to send and receive data.
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
static bool AXP_Telnet_Receive(int sock, u8 *buf, u32 *bufLen)
{
    bool	retVal = true;

    /*
     * Receive up to a buffers worth of data.  Since we are using a
     * steam protocol, we only may receive part of a complete buffer.
     * A buffer's last character should be a null character.
     */
    printf("Ready to receive data...\n");
    *bufLen = recv(sock, buf, *bufLen, 0);
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
 *  sock:
 *	The value of the socket on which to send and receive data.
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
bool AXP_Telnet_Send(int sock, u8 *buf, u32 bufLen)
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
    bufLen = send(sock, buf, bufLen, 0);
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
static bool AXP_Telnet_Reject(int sock)
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
static bool AXP_Telnet_Processor(int sock, u8 *buf, u32 bufLen)
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
	    retVal = AXP_Telnet_Send(sock, buf, bufLen);
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
    u8			buffer[AXP_TELNET_MSG_LEN];
    u32			bufferLen;
    int			connSock, telnetSock;
    AXP_Telnet_States	state = Accept;
    bool		retVal = true;

    while(state != Finished)
    {
	switch(state)
	{
	    case Listen:
		retVal = AXP_Telnet_Listener(&connSock);
		state = retVal ? Accept : Closing;
		break;

	    case Accept:
		retVal = AXP_Telnet_Accept(connSock, &telnetSock);
		if (retVal == true)
		{
		    bufferLen = 0;
		    buffer[bufferLen++] = IAC;
		    buffer[bufferLen++] = DO;
		    buffer[bufferLen++] = TELOPT_ECHO;
		    buffer[bufferLen++] = IAC;
		    buffer[bufferLen++] = DO;
		    buffer[bufferLen++] = TELOPT_NAWS;
		    buffer[bufferLen++] = IAC;
		    buffer[bufferLen++] = DO;
		    buffer[bufferLen++] = TELOPT_LFLOW;
		    buffer[bufferLen++] = IAC;
		    buffer[bufferLen++] = WILL;
		    buffer[bufferLen++] = TELOPT_ECHO;
		    buffer[bufferLen++] = IAC;
		    buffer[bufferLen++] = WILL;
		    buffer[bufferLen++] = TELOPT_SGA;
		    retVal = AXP_Telnet_Send(telnetSock, buffer, bufferLen);
		    state = retVal ? Active : Listen;
		}
		else
		    state = Listen;
		break;

	    case Active:
		while (state == Active)
		{
		    bufferLen = AXP_TELNET_MSG_LEN;
		    retVal = AXP_Telnet_Receive(telnetSock, buffer, &bufferLen);
		    if (retVal == true)
			retVal = AXP_Telnet_Processor(
						telnetSock,
						buffer,
						bufferLen);
		    if (retVal == false)
			state = Inactive;
		}
		break;

	    case Inactive:
		retVal = AXP_Telnet_Reject(telnetSock);
		state = Listen;
		break;

	    case Closing:
		retVal = AXP_Telnet_Ignore(connSock);
		state = Finished;
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
