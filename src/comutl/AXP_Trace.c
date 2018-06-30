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
 *  This source module contains the code required to be able to trace the
 *  running of the Digital Alpha AXP 21264 Emulator.
 *
 * Revision History:
 *
 *  V01.000	27-Jan-2018	Jonathan D. Belanger
 *  Initially written.
 *
 *  V01.001	29-Jun-2018	Jonathan D. Belanger
 *  Added a trace buffer function to dump out a buffer in hexidecimal and in
 *  ASCII.
 */
#include "AXP_Configure.h"
#include "AXP_Utility.h"
#include "AXP_Trace.h"

static char *AXPTRCLOG = "AXP_LOGMASK";
static char *AXPTRCFIL = "AXP_LOGFILE";
AXP_TRCLOG _axp_trc_log_ = 0;
static char _axp_trc_out_[81];
static pthread_once_t _axp_trc_log_once_ = PTHREAD_ONCE_INIT;
static FILE *_axp_trc_fp_;
bool _axp_trc_active_ = false;

/*
 * AXP_TraceInit_Once
 *	This function is called using the pthread_once function to make sure that
 *	only one thread can call this function ever.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	None.
 */
void AXP_TraceInit_Once(void)
{
    char *getEnvStr;

    /*
     * Translate the environment variable to indicate that tracing should
     * occur or not.  Also, where the tracing should go, a file or stdout.
     */
    getEnvStr = getenv(AXPTRCLOG);
    if (getEnvStr != NULL)
	sscanf(getEnvStr, "0x%08x", &_axp_trc_log_);
    if (_axp_trc_log_ != 0)
    {
	getEnvStr = getenv(AXPTRCFIL);
	if (getEnvStr == NULL)
	{
	    strcpy(_axp_trc_out_, "Standard Output");
	    _axp_trc_fp_ = stdout;
	}
	else
	{
	    sscanf(getEnvStr, "%80s", _axp_trc_out_);
	    _axp_trc_fp_ = fopen(_axp_trc_out_, "w");
	}
	_axp_trc_active_ = true;
	AXP_TraceWrite("Digital Alpha AXP 21264 CPU Emulator Trace Utility.");
	AXP_TraceWrite("AXP_TRCLOG = 0x%08x : AXP_TRCFIL = %s", _axp_trc_log_,
	        _axp_trc_out_);
	AXP_TraceWrite("Copyright 2018, Jonathan D. Belanger.");
	AXP_TraceWrite("");
	AXP_TraceConfig();
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_TraceInit
 *	This function is called once to initialize the tracing.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	true:	The pthread_once function was a success.
 *	false:	The pthread_once function returned EINVAL, indicating one of the
 *			parameters supplied on the call is not valid.
 */
bool AXP_TraceInit(void)
{
    bool retVal;

    retVal = pthread_once(&_axp_trc_log_once_, AXP_TraceInit_Once) == 0;

    /*
     * Return back to the caller.
     */
    return (retVal);
}

/*
 * AXP_TraceEnd
 *	This function is called end the tracing.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_TraceEnd(void)
{

    /*
     * Turn off the tracing.
     */
    _axp_trc_active_ = false;

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_TraceWrite
 *	This function is called with a variable list argument and uses vfprintf to
 *	take the format and va_list as parameters.
 *
 * Input Parameters:
 *	fmt:
 *		A pointer to a format string.
 *	...:
 *		A variable number of arguments.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_TraceWrite(char *fmt, ...)
{
    char outBuf[41];
    va_list ap;
    struct timeval now;
    struct tm timeNow;

    /*
     * Write out a time-stamp followed by a colon and a space character
     */
    gettimeofday(&now, NULL);
    strftime(outBuf, sizeof(outBuf), "%H:%M:%S",
	    localtime_r(&now.tv_sec, &timeNow));
    fprintf(_axp_trc_fp_, "%s.%03ld: ", outBuf, (now.tv_usec / 1000));

    /*
     * Now generate the rest of the requested text.
     */
    va_start(ap, fmt);
    vfprintf(_axp_trc_fp_, fmt, ap);
    va_end(ap);

    /*
     * End it with a new-line.
     */
    fprintf(_axp_trc_fp_, "\n");

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_TraceBuffer
 *  This function is called to trace a buffers worth of data.  It will dump out
 *  a hexidecimal set of bytes, followed by those same bytes in ASCII (a '.'
 *  will be used for non-printable characters.
 *
 * Input Parameters:
 *  buf:
 *	A pointer to the buffer of unsigned bytes to be dumped.
 *  bufLen:
 *	A value indicating the number of bytes in the buf parameter.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_TraceBuffer(u8 *buf, u32 bufLen)
{
    char outBuf[80];
    char *fmt1 = "%02x";
    char *fmt2 = " %02x";
    char *fmt3 = "%*c: ";
    char *fmt4 = "%c";
    u32 ii, offset = 0, remLen = bufLen;

    while (remLen > 0)
    {
	sprintf(&outBuf[offset], fmt1, buf[0]);
	for (ii = 1; ((ii < 20) && (ii < remLen)); ii++)
	    offset += sprintf(&outBuf[offset], fmt2, buf[ii]);
	offset += sprintf(&outBuf[offset], fmt3, ' ');
	for (ii = 0; ((ii < 20) && (ii < remLen)); ii++)
	    offset += sprintf(
			&outBuf[offset],
			fmt4,
			(isprint(buf[ii]) ? buf[ii] : '.'));
	AXP_TraceWrite("%s", outBuf);
	remLen -= ii;
    }

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_TraceLock
 *  This function is called by the AXP_TRACE_BEGIN() macro to lock the trace
 *  mutex.  We do this so that multiple calls to AXP_TraceWrite associated with
 *  a single trace opportunity are logged together.
 *
 * Input Parameters:
 *  None.
 *
 * Output Parameters:
 *  None.
 *
 * Return Values:
 *  None.
 */
void AXP_TraceLock(void)
{

    /*
     * Lock the file stream.
     */
    flockfile(_axp_trc_fp_);

    /*
     * Return back to the caller,
     */
    return;
}

/*
 * AXP_TraceUnlock
 *	This function is called by the AXP_TRACE_END() macro to unlock the trace
 *	mutex.  We do this so that multiple calls to AXP_TraceWrite associated with
 *	a single trace opportunity are logged together.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	None.
 *
 * Return Values:
 *	None.
 */
void AXP_TraceUnlock(void)
{

    /*
     * Unlock the file stream.
     */
    funlockfile(_axp_trc_fp_);

    /*
     * Return back to the caller,
     */
    return;
}
