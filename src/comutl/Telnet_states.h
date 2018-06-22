#ifndef _TELNET_
#define _TELNET_

#define TELNET_NO				0
#define TELNET_WANTNO_EMPTY		1
#define TELNET_WANTNO_OPPOSITE	2
#define TELNET_WANTYES_EMPTY	3
#define TELNET_WANTYES_OPPOSITE	4
#define TELNET_YES				5
#define TELNET_MAX_STATES		6

typedef struct
{
	u8		state;
	bool	preferred;
} Telnet_States;

#define TELNET_OPTS				50
#define	TELNET_SM_YES_SRV		247
#define TELNET_SM_NO_SRV		248
#define	TELNET_SM_YES_CLI		249
#define TELNET_SM_NO_CLI		250

#define TELNET_BUF_LEN			512

typedef struct
{
	int				sock;
	Telnet_States	options[TELNET_OPTS];
	char			buffer[TELNET_BUF_LEN];
	int				bufferLen;
	char			subOptions[TELNET_BUF_LEN];
	int				subOptIdx;
	u8				sessionState;
} Telnet_Session;

#define TELNET_SM_ENTRY(cmd, opt)	((((cmd)-TELNET_SM_YES_SRV)*2)+(opt).preferred)
#define TELNET_STATE(ses, opt)		(ses)->options[(opt)].state;
#define TELNET_PREFERRED(ses, opt)	(ses)->options[(opt)].preferred;

/*
 * These are the action routines for the option processing.
 */
void Send_DO(Telnet_Session *, u8);
void Send_DONT(Telnet_Session *, u8);
void Send_WILL(Telnet_Session *, u8);
void Send_WONT(Telnet_Session *, u8);

/*
 * This routine is called to process options state machine when we are
 * negotiating them.
 */
void Telnet_Execute_Option_SM(Telnet_Session *ses, u8 cmd, u8 opt)
{
	u8	curState = ses->options[opt].state;

	if ((cmd >= WILL) && (cmd <= DONT))
	{
		if (Telnet_StateMachine[TELNET_SM_ENTRY(cmt, opt)][curState].actionRtn != NULL)
			(Telnet_StateMachine[TELNET_SM_ENTRY(cmt, opt)][curState].actionRtn)(ses, '\0');
		ses->optiond[opt].state = Telnet_StateMachine[TELNET_SM_ENTRY(cmt, opt)][curState].nextState;
	}
	return;
}

/*
 * This routine is called to set the initial state machine state.
 */
void Telnet_Set_Option_SM(Telnet_Session *ses, u8 cmd, u8 opt)
{
	u8	curState = ses->options[opt].state;

	if ((cmd >= TELNET_SM_YES_SRV) && (cmd <= TELNET_SM_NO_CLI))
	{
		if (Telnet_StateMachine[TELNET_SM_ENTRY(cmt, opt)][curState].actionRtn != NULL)
			(Telnet_StateMachine[TELNET_SM_ENTRY(cmt, opt)][curState].actionRtn)(ses, '\0');
		ses->optiond[opt].state = Telnet_StateMachine[TELNET_SM_ENTRY(cmt, opt)][curState].nextState;
	}
	return;
}

typedef struct
{
	u8		nextState;
	void	(*actionRtn)(Telnet_Session *, u8);
} Telnet_SM;

Telnet_SM	Telnet_StateMachine[16][TELNET_MAX_STATES] =
{
	/* YES_SRV	- PREFERRED */
	{
		{TELNET_WANTYES_EMPTY, Send_WILL},
		{TELNET_WANTNO_OPPOSITE, NULL},
		{TELNET_WANTNO_OPPOSITE, NULL},
		{TELNET_WANTYES_EMPTY, NULL},
		{TELNET_WANTYES_EMPTY, NULL},
		{TELNET_YES, NULL}
	},
	/* YES_SRV	- NOT PREFERRED */
	{
		{TELNET_WANTYES_EMPTY, Send_WILL},
		{TELNET_WANTNO_OPPOSITE, NULL},
		{TELNET_WANTNO_OPPOSITE, NULL},
		{TELNET_WANTYES_EMPTY, NULL},
		{TELNET_WANTYES_EMPTY, NULL},
		{TELNET_YES, NULL}
	},
	/* NO_SRV	- PREFERRED */
	{
		{TELNET_NO, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_WANTYES_OPPOSITE, NULL},
		{TELNET_WANTYES_OPPOSITE, NULL},
		{TELNET_WANTNO_EMPTY, Send_WONT}
	},
	/* NO_SRV	- NOT PREFERRED */
	{
		{TELNET_NO, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_WANTYES_OPPOSITE, NULL},
		{TELNET_WANTYES_OPPOSITE, NULL},
		{TELNET_WANTNO_EMPTY, Send_WONT}
	},
	/* YES_CLI	- PREFERRED */
	{
		{TELNET_WANTYES_EMPTY, Send_DO},
		{TELNET_WANTNO_OPPOSITE, NULL},
		{TELNET_WANTNO_OPPOSITE, NULL},
		{TELNET_WANTYES_EMPTY, NULL},
		{TELNET_WANTYES_EMPTY, NULL},
		{TELNET_YES, NULL}
	},
	/* YES_CLI	- NOT PREFERRED */
	{
		{TELNET_WANTYES_EMPTY, Send_DO},
		{TELNET_WANTNO_OPPOSITE, NULL},
		{TELNET_WANTNO_OPPOSITE, NULL},
		{TELNET_WANTYES_EMPTY, NULL},
		{TELNET_WANTYES_EMPTY, NULL},
		{TELNET_YES, NULL}
	},
	/* NO_CLI	- PREFERRED */
	{
		{TELNET_NO, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_WANTYES_OPPOSITE, NULL},
		{TELNET_WANTYES_OPPOSITE, NULL},
		{TELNET_WANTNO_EMPTY, Send_DONT}
	},
	/* NO_CLI	- NOT PREFERRED */
	{
		{TELNET_NO, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_WANTYES_OPPOSITE, NULL},
		{TELNET_WANTYES_OPPOSITE, NULL},
		{TELNET_WANTNO_EMPTY, Send_DONT}
	},
	/* WILL	- PREFERRED */
	{
		{TELNET_YES, Send_DO},
		{TELNET_NO, NULL},
		{TELNET_YES, NULL},
		{TELNET_YES, NULL},
		{TELNET_WANTNO_EMPTY, Send_DONT},
		{TELNET_YES, NULL}
	},
	/* WILL	- NOT PREFERRED */
	{
		{TELNET_NO, Send_DONT},
		{TELNET_NO, NULL},
		{TELNET_YES, NULL},
		{TELNET_YES, NULL},
		{TELNET_WANTNO_EMPTY, Send_DONT},
		{TELNET_YES, NULL}
	},
	/* WONT	- PREFERRED */
	{
		{TELNET_NO, NULL},
		{TELNET_NO, NULL},
		{TELNET_WANTYES_EMPTY, Send_DO},
		{TELNET_NO, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_NO, Send_DONT}
	},
	/* WONT	- NOT PREFERRED */
	{
		{TELNET_NO, NULL},
		{TELNET_NO, NULL},
		{TELNET_WANTYES_EMPTY, Send_DO},
		{TELNET_NO, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_NO, Send_DONT}
	},
	/* DO	- PREFERRED */
	{
		{TELNET_YES, Send_WILL},
		{TELNET_NO, NULL},
		{TELNET_YES, NULL},
		{TELNET_YES, NULL},
		{TELNET_WANTNO_EMPTY, Send_WONT},
		{TELNET_YES, NULL}
	},
	/* DO	- NOT PREFERRED */
	{
		{TELNET_NO, Send_WONT},
		{TELNET_NO, NULL},
		{TELNET_YES, NULL},
		{TELNET_YES, NULL},
		{TELNET_WANTNO_EMPTY, Send_WONT},
		{TELNET_YES, NULL}
	},
	/* DONT	- PREFERRED */
	{
		{TELNET_NO, NULL},
		{TELNET_NO, NULL},
		{TELNET_WANTYES_EMPTY, Send_WILL},
		{TELNET_NO, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_NO, Send_WONT}
	},
	/* DONT	- NOT PREFERRED */
	{
		{TELNET_NO, NULL},
		{TELNET_NO, NULL},
		{TELNET_WANTYES_EMPTY, Send_WILL},
		{TELNET_NO, NULL},
		{TELNET_WANTNO_EMPTY, NULL},
		{TELNET_NO, Send_WONT}
	}
};

#define TELNET_DATA					0
#define TELNET_IAC					1
#define TELNET_CMD					2
#define TELNET_CR					3
#define TELNET_SB					4
#define TELNET_SE					5
#define TELNET_MAX_SES_STATES		6

#define TELNET_NULL_PRESENT			0
#define TELNET_IAC_PRESENT			1
#define TELNET_R_PRESENT			2
#define TELNET_CMD_PRESENT			3
#define TELNET_SE_PRESENT			4
#define TELNET_SB_PRESENT			5
#define TELNET_CATCHALL				6

#define TELNET_SES_ENTRY(c)													\
	(((c) == '\0') ? TELNET_NULL_PRESENT :									\
		(((c) == IAC) ? TELNET_IAC_PRESENT :								\
			(((c) == '\r') ? TELNET_R_PRESENT :								\
				((((c) >= WILL) || ((c) <= DONT)) ? TELNET_CMD_PRESENT :	\
					(((c) == SE) ? TELNET_SE_PRESENT :						\
						(((c) == SB) ? TELNET_SB_PRESENT :					\
							TELNET_CATCHALL))))))

/*
 * Action routines for the receive data state machine,
 */
void Echo_Data(Telnet_Session *, u8);
void Process_CMD(Telnet_Session *, u8);
void SubOpt_Accumulate(Telnet_Session *, u8);
void Process_IAC(Telnet_Session *, u8);
void Process_Suboption(Telnet_Session *, u8);

/*
 * This routine is called to process options state machine when we are
 * negotiating them.
 */
void Telnet_Execute_SessionSM(Telnet_Session *ses)
{
	int		ii = 0;
	u8		index;

	while (ii < ses->bufferLen)
	{
		index = TELNET_SES_ENTRY(ses->buffer[ii++]);
		if (Telnet_Session_SM[index][ses->sessionState].actionRtn != NULL)
			(Telnet_Session_SM[index][ses->sessionState].actionRtn)(ses, index);
		ses->sessionState = Telnet_Session_SM[index][ses->sessionState].nextState;
	}
	return;
}

Telnet_SM	Telnet_Session_SM[6][TELNET_MAX_SES_STATES] =
{
	/* NULL PRESENT */
	{
		{TELNET_DATA, Echo_Data},
		{TELNET_DATA, NULL},
		{TELNET_DATA, Process_CMD},
		{TELNET_DATA, NULL},
		{TELNET_SB, SubOpt_Accumulate},
		{TELNET_IAC, Process_IAC}
	},
	/* IAC PRESENT */
	{
		{TELNET_IAC, NULL},
		{TELNET_DATA, NULL},
		{TELNET_DATA, Process_CMD},
		{TELNET_DATA, NULL},
		{TELNET_SE, NULL},
		{TELNET_SB, SubOpt_Accumulate}
	},
	/* R PRESENT */
	{
		{TELNET_CR, NULL},
		{TELNET_DATA, NULL},
		{TELNET_DATA, Process_CMD},
		{TELNET_CR, NULL},
		{TELNET_SB, SubOpt_Accumulate},
		{TELNET_IAC, Process_IAC}
	},
	/* TELNET_CMD_PRESENT */
	{
		{TELNET_DATA, NULL},
		{TELNET_CMD, NULL},
		{TELNET_DATA, Process_CMD},
		{TELNET_DATA, NULL},
		{TELNET_SB, SubOpt_Accumulate},
		{TELNET_IAC, Process_IAC}
	},
	/* TELNET_SE_PRESENT */
	{
		{TELNET_DATA, Echo_Data},
		{TELNET_DATA, NULL},
		{TELNET_DATA, Process_CMD},
		{TELNET_DATA, NULL},
		{TELNET_SB, SubOpt_Accumulate},
		{TELNET_DATA, Process_Suboption}
	},
	/* TELNET_SB_PRESENT */
	{
		{TELNET_DATA, NULL},
		{TS_SB, SB_CLEAR},
		{TELNET_DATA, Process_CMD},
		{TELNET_DATA, NULL},
		{TELNET_SB, SubOpt_Accumulate},
		{TELNET_IAC, Process_IAC}
	},
	/* TELNET_CATCH_ALL */
	{
		{TELNET_DATA, Echo_Data},
		{TELNET_DATA, NULL},
		{TELNET_DATA, Process_CMD},
		{TELNET_DATA, NULL},
		{TELNET_SB, SubOpt_Accumulate},
		{TELNET_IAC, Process_IAC}
	}
};

#endif /* _TELNET_ */
