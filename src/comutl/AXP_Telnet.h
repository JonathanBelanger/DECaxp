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
#include "AXP_Blocks.h"
#include "AXP_Configure.h"
#include "AXP_Trace.h"
#include "AXP_StateMachine.h"

/*
 * Include the TELNET header file, but make sure certain optional compiling is
 * turned on.
 */
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/*
 * Definitions used in the source file.
 */
#define AXP_TELNET_MSG_LEN	1024
#define AXP_TELNET_DEFAULT_PORT	108

/*
 * Define the states for the TELNET session.
 */
typedef enum
{
    Listen,
    Accept,
    Negotiating,
    Active,
    Inactive,
    Closing,
    Finished
} AXP_Telnet_Session_State;

/*
 * The following definitions are the states for TELNET option processing.  This
 * state machine will prevent infinite looping of option processing.
 */
#define AXP_OPT_NO			0
#define AXP_OPT_WANTNO_SRV		1
#define AXP_OPT_WANTNO_CLI		2
#define AXP_OPT_WANTYES_SRV		3
#define AXP_OPT_WANTYES_CLI		4
#define AXP_OPT_YES			5
#define AXP_OPT_MAX_STATE		6

/*
 * These definitions are used to determine the action being performed, the
 * current state of the option, and whether the option is preferred.
 */
#define AXP_OPT_ACTION(cmd, opt)	((((cmd)-YES_SRV)*2)+(opt).preferred)
#define AXP_OPT_STATE(options, opt)	options[(opt)].state;
#define AXP_OPT_PREFERRED(options, opt)	options[(opt)].preferred;
#define AXP_OPT_SUPPORTED(options, opt)	options[(opt)].supported;
#define AXP_OPT_SET_PREF(options, opt)	\
    options[(opt)].supported = options[(opt)].preferred = true
#define AXP_OPT_SET_SUPP(options, opt)	\
    options[(opt)].supported = true; options[(opt)].preferred = false

/*
 * In addition to the commands, WILL, WONT, DO, and DONT, there are four
 * additional commands that will be utilized as input to the state machine.
 * These additional commands are utilized only within the TELNET server and
 * are never sent or received.
 */
#define	YES_SRV				247
#define NO_SRV				248
#define	YES_CLI				249
#define NO_CLI				250
#define AXP_OPT_MAX_ACTION		(IAC-YES_SRV)*2

/*
 * The following definitions are the states for TELNET receive processing.
 * This state machine is used to process receive data.  It handles data
 * received that could be comprised of data or commands.
 */
#define AXP_RCV_DATA			0
#define AXP_RCV_IAC			1
#define AXP_RCV_CMD			2
#define AXP_RCV_CR			3
#define AXP_RCV_SB			4
#define AXP_RCV_SE			5
#define AXP_RCV_MAX_STATE		6

/*
 * The following are the actions into the Receive State Machine.
 */
#define AXP_ACT_NUL			0	/* '\0' */
#define AXP_ACT_IAC			1	/* IAC command */
#define AXP_ACT_R			2	/* '\r' */
#define AXP_ACT_CMD			3	/* WILL, WONT, DO, DONT cmd */
#define AXP_ACT_SE			4	/* Suboption End command */
#define AXP_ACT_SB			5	/* Suboption Begin command */
#define AXP_ACT_CATCHALL		6	/* Everything else */
#define AXP_ACT_MAX			7

/*
 * This macro determines the action being performed for the TELNET receive
 * state machine.
 */
#define AXP_RCV_ACTION(c)						\
    (((c) == '\0') ? AXP_ACT_NUL :					\
	(((c) == IAC) ? AXP_ACT_IAC :					\
	    (((c) == '\r') ? AXP_ACT_R :				\
		((((c) >= WILL) || ((c) <= DONT)) ? AXP_ACT_CMD :	\
		    (((c) == SE) ? AXP_ACT_SE :				\
			(((c) == SB) ? AXP_ACT_SB :			\
			    AXP_ACT_CATCHALL))))))

typedef struct
{
    u8			state;
    bool		preferred;
    bool		supported;
} AXP_Telnet_OptState;

/*
 * This data structure is used to handle a TELNET session.  It contains
 * state information and negotiated options.
 */
#define AXP_TELNET_SB_LEN	512
#define AXP_TELNET_TTYPE_LEN	32
typedef struct
{

    /*
     * This field needs to be at the top of all data blocks/structures
     * that need to be specifically allocated by the Blocks module.
     */
    AXP_BLOCK_DSC header;

    /*
     * This where the rest of the fields are needed to maintain a TELNET
     * session with a client.
     */
    int				mySocket;
    u8				rcvState;

    /*
     * These are the state objects.  They are used with the appropriate
     * State Machine
     */
    AXP_Telnet_OptState		myOptions[NTELOPTS];
    AXP_Telnet_OptState		theirOptions[NTELOPTS];

    /*
     * The following fields are used for processing sub-options.
     */
    u8				cmd;
    u8				subOptionTType[AXP_TELNET_TTYPE_LEN];
    u8				subOptBuf[AXP_TELNET_SB_LEN];
    u16				subOptBufIdx;
    u16				subOptBufLen;
} AXP_TELNET_SESSION;

/*
 * This macro is used to select the correct options (mine or theirs) being
 * processed.  This is determined by the Action (command) being processed.
 */
#define AXP_TELNET_OPTIONS(ses, action)					\
    ((((action) >= YES_CLI) && ((action) <= WONT))) ?			\
	(ses)->theirOptions : (ses)->myOptions)

/*
 * Function prototypes.
 */
bool AXP_Telnet_Send(AXP_TELNET_SESSION *, u8 *, int);
void AXP_Telnet_Main(void);
void get_State_Machines(AXP_StateMachine ***, AXP_StateMachine ***);

#endif /* AXP_TELNET_H_ */
