/*
 * Copyright (C) Jonathan D. Belanger 2017.
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
 *	This source file contains the functions needed to implement the
 *	VAX compatibility instructions of the 21264 Alpha AXP CPU.
 *
 * Revision History:
 *
 *	V01.000		19-Jul-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "CommonUtilities/AXP_Configure.h"
#include "21264Processor/Ebox/AXP_21264_Ebox_VAX.h"

/*
 * AXP_RC
 *	This function implements the Read and Clear instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_RC(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Read the flag and store it into the destination register (Ra) and then
     * clear it.
     */
    instr->destv.r.uq = cpu->VAXintrFlag;
    cpu->VAXintrFlag = false;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}

/*
 * AXP_RS
 *	This function implements the Read and Set instruction of the Alpha AXP
 *	processor.
 *
 * Input Parameters:
 *	cpu:
 *		A pointer to the structure containing the information needed to emulate
 *		a single CPU.
 * 	instr:
 * 		A pointer to a structure containing the information needed to execute
 * 		this instruction.
 *
 * Output Parameters:
 * 	instr:
 * 		The contents of this structure are updated, as needed.
 *
 * Return Value:
 * 	No Exception:	Normal successful completion.
 */
AXP_EXCEPTIONS AXP_RS(AXP_21264_CPU *cpu, AXP_INSTRUCTION *instr)
{

    /*
     * Read the flag and store it into the destination register (Ra) and then
     * set it.
     */
    instr->destv.r.uq = cpu->VAXintrFlag;
    cpu->VAXintrFlag = true;

    /*
     * Return back to the caller with any exception that may have occurred.
     */
    return (NoException);
}
