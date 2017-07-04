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
 *	This source file contains the main function to test the integer load code.
 *
 * Revision History:
 *
 *	V01.000		26-Jun-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "AXP_Utility.h"
#include "AXP_Blocks.h"
#include "AXP_21264_Instructions.h"
#include "AXP_21264_CPU.h"
#include "AXP_21264_Fbox.h"

/*
 * IEEE Format:  One of the following:
 * 		b32	for 32-bit		(IEEE S)
 * 		b64 for 64-bit		(IEEE T)
 * 		b128 for 128-bit	(IEEE X)
 */
typedef enum
{
	ieeeUnknown,
	ieeeS,
	ieeeT,
	ieeeX
} AXP_IEEEFormat;

/*
 * The operation: One of the following:
 * 		+ for add
 * 		- for subtract
 * 		* for multiply
 * 		/ for divide
 * 		*+ for fused multiply-add
 * 		V for square root
 * 		% for remainder
 * 		rfi for round float to int
 * 		cff for convert between different supported floating-point format
 * 		cfi for convert floating-point to integer
 * 		cif for convert integer to floating point
 * 		cfd for convert to decimal character string
 * 		cdf for convert decimal character string to float
 * 		qC for quiet comparison
 * 		sC for signaling comparison
 * 		cp for copy
 * 		~ for negate
 * 		A for abs
 * 		@ for copy-sign
 * 		S for scalb
 * 		L for logb
 * 		Na for next-after
 * 		? for class
 * 		?- for is-signed
 * 		?n for is-normal
 * 		?f for is-finite
 * 		?0 for is-zero
 * 		?s for is-subnormal
 * 		?i for is-inf
 * 		?N for is-nan
 * 		?sN for is-signaling
 * 		?N for is-nan
 * 		<C for min-num
 * 		>C for max-num
 * 		<A for min-num-mag
 * 		>A for max-num-mag
 * 		=quant for same-quantum
 * 		quant for quantize
 * 		Nu for next-up
 * 		Nd for next-down
 * 		eq for equivalent
 */
typedef enum
{
	add,
	subtract,
	multiply,
	divide,
	multiplyAdd,
	squareRoot,
	remainder,
	roundFloatToInt,
	convertFloatToFloat,
	convertFloatToInt,
	convertIntToFloat,
	convertToDecimalStr,
	convertDecimalStrToFloat,
	quietComparison,
	signalingComparison,
	copy,
	negate,
	absoluteValue,
	copySign,
	scalb,
	logb,
	nextAfter,
	class,
	isSigned,
	isNormal,
	isFinite,
	isZero,
	isSubnormal,
	isInfinite,
	isNotANumber,
	isSignaling,
	minNum,
	maxNum,
	minNumMag,
	maxNumMag,
	sameQuantum,
	quantize,
	nextUp,
	nextDown,
	equivalent
} AXP_Operation;

typedef struct
{
	char			*operStr;
	AXP_Operation	oper;
	char			*humanReadable;
} AXP_cvtOperationStr;

#define AXP_NUM_OPER	40

AXP_cvtOperationStr cvtOperStr[AXP_NUM_OPER] =
{
	{"+", add, "add"},
	{"-", subtract, "subtract"},
	{"*", multiply, "multiply"},
	{"/", divide, "divide"},
	{"*+", multiplyAdd, "multiply-add"},
	{"V", squareRoot, "square root"},
	{"%", remainder, "remainder"},
	{"rfi", roundFloatToInt, "round float to integer"},
	{"cff", convertFloatToFloat, "convert float to float"},
	{"cfi", convertFloatToInt, "convert float to integer"},
	{"cif", convertIntToFloat, "convert integer to float"},
	{"cfd", convertToDecimalStr, "convert to decimal string"},
	{"cdf", convertDecimalStrToFloat, "convert decimal string to float"},
	{"qC", quietComparison, "quiet comparison"},
	{"sC", signalingComparison, "signaling comparison"},
	{"cp", copy, "copy"},
	{"~", negate, "negate"},
	{"A", absoluteValue, "absolute value"},
	{"@", copySign, "copy sign"},
	{"S", scalb, "scalb"},
	{"L", logb, "logb"},
	{"Na", nextAfter, "next after"},
	{"?", class, "class"},
	{"?-", isSigned, "is signed"},
	{"?n", isNormal, "is normal"},
	{"?f", isFinite, "is finite"},
	{"?0", isZero, "is zero"},
	{"?s", isSubnormal, "is subnormal"},
	{"?i", isInfinite, "is infinite"},
	{"?N", isNotANumber, "is not a number"},
	{"?sN", isSignaling, "is signaling"},
	{"<C", minNum, "min-num"},
	{">C", maxNum, "max-num"},
	{"<A", minNumMag, "min-num-mag"},
	{">A", maxNumMag, "max-num-mag"},
	{"=quant", sameQuantum, "same quantum"},
	{"quant", quantize, "quantize"},
	{"Nu", nextUp, "next up"},
	{"Nd", nextDown, "next down"},
	{"eq", equivalent, "equivalent"}
};
/*
 * The rounding mode: one of the following:
 * 		> (positive infinity)
 * 		< (negative infinity)
 * 		0 (zero)
 * 		=0 (nearest, ties to even)
 * 		=^ (nearest, ties away from zero)
 */
typedef enum
{
	positiveInfinity,
	negativeInfinity,
	zero,
	nearestTiesToEven,
	nearestAwayFromZero
} AXP_RoundingMode;
typedef struct
{
	char				*str;
	AXP_RoundingMode	round;
	char				*humanReadable;
} AXP_cvtRoundingModeStr;

#define AXP_NUM_ROUNDING	5

AXP_cvtRoundingModeStr cvtRoundingStr[AXP_NUM_ROUNDING] =
{
	{">", positiveInfinity, "positive infinity"},
	{"<", negativeInfinity, "negative infinity"},
	{"0", zero, "zero"},
	{"=0", nearestTiesToEven, "nearest ties to even"},
	{"=^", nearestAwayFromZero, "nearest away from zero"}
};

char *msgStr = "\%AXP-%c-%s, %s.\n";
char fatal = 'F';
char error = 'E';
char info = 'I';
char success = 'S';

char *ieeeFormatToStr(AXP_IEEEFormat ieeeFmt)
{
	char *retVal = "IEEE Error";

	switch(ieeeFmt)
	{
		case ieeeS:
			retVal = "'IEEE S'";
			break;

		case ieeeT:
			retVal = "'IEEE T'";
			break;

		case ieeeX:
			retVal = "'IEEE X'";
			break;

		case ieeeUnknown:
			retVal = "'IEEE Unknown'";
			break;
	}
	return(retVal);
}

void printFormat(AXP_IEEEFormat operand, AXP_IEEEFormat result)
{
	if (operand == result)
		printf("operand & result format set to %s\n", ieeeFormatToStr(operand));
	else
	{
		printf("operand format set to %s, ", ieeeFormatToStr(operand));
		printf("result format set to %s\n",  ieeeFormatToStr(result));
	}
	return;
}
/*
 * readNextToken
 * 	This function is called to read from the current position to the next space
 * 	character, returning everything but the last space.  End of line and end of
 * 	file are the same as a space.
 *
 * Input Parameters:
 * 	fp:
 * 		A pointer to the file, from which to read the next token.
 * 	len:
 * 		A value indicating the maximum length of the return string.  The last
 * 		character in the string will be the null character.
 *
 * Output Parameter:
 * 	retStr:
 * 		A pointer to the null-terminated string to be returned back to the
 * 		caller.
 *
 * Return Value:
 * 	1:		The output parameters are ready for processing.
 * 	0:		We have reached the end of the input file.
 * 	-1:		A error occurred (a message will be displayed).
 */
i32 readNextToken(FILE *fp, char *retStr, int len)
{
	i32		retVal = 1;
	int		getC, retStrIdx = 0;;

	getC = fgetc(fp);
	while ((getC != EOF) &&
		   (getC != ' ') &&
		   (getC != '\n') &&
		   (retStrIdx < (len - 1)))
	{
		retStr[retStrIdx++] = (char) getC;
		getC = fgetc(fp);
	}
	retStr[retStrIdx] = '\0';
	if (getC == EOF)
		retVal = 0;

	/*
	 * Return back to the caller.
	 */
	return(retVal);
}

/*
 * parseNextLine
 * 	This function is called to read the next line and parse it into its
 * 	constituent parts.
 *
 * Input Parameters:
 * 	fp:
 * 		A pointer to the file, from which to read the next line.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	1:		The output parameters are ready for processing.
 * 	0:		We have reached the end of the input file.
 * 	-1:		A error occurred (a message will be displayed).
 */
i32 parseNextLine(
		FILE *fp,
		AXP_Operation *operation,
		AXP_IEEEFormat *operandFmt,
		AXP_IEEEFormat *resultFmt,
		AXP_RoundingMode *roundMode)
{
	bool	found = false;
	i32		retVal = 1;
	char	workingStr[7];
	int		getC;
	int		ii;
	char	remLine[80];

	/*
	 * Get the format information.  One or two of:
	 * 		b32		IEEE S
	 * 		b64		IEEE T
	 * 		b128	IEEE X
	 */
	*operandFmt = *resultFmt = ieeeUnknown;
	getC = fgetc(fp);
	while (getC == 'b')
	{
		getC = fgetc(fp);
		switch (getC)
		{
			case '3':
				if (*resultFmt != ieeeUnknown)
					*resultFmt = ieeeS;
				else
					*operandFmt = *resultFmt = ieeeS;
				getC = fgetc(fp);
				if ((getC != EOF) && (getC != '2'))
				{
					sprintf(
							remLine,
						"format not OK, got 'b3%c', expected 'b32'",
						*((char *) &getC));
					printf(
						msgStr,
						fatal,
						"FMTNOTOK",
						remLine);
					retVal = -1;
				}
				break;

			case '6':
				if (*resultFmt != ieeeUnknown)
					*resultFmt = ieeeT;
				else
					*operandFmt = *resultFmt = ieeeT;
				getC = fgetc(fp);
				if ((getC != EOF) && (getC != '4'))
				{
					sprintf(
						remLine,
						"format not OK, got 'b6%c', expected 'b164'",
						getC);
					printf(
						msgStr,
						fatal,
						"FMTNOTOK",
						remLine);
					retVal = -1;
				}
				break;

			case '1':
				if (*resultFmt != ieeeUnknown)
					*resultFmt = ieeeX;
				else
					*operandFmt = *resultFmt = ieeeX;
				getC = fgetc(fp);
				if ((getC != EOF) && (getC == '2'))
				{
					getC = fgetc(fp);
					if ((getC != EOF) && (getC != '8'))
					{
						sprintf(
							remLine,
							"format not OK, got 'b12%c', expected 'b128'",
							getC);
						printf(
							msgStr,
							fatal,
							"FMTNOTOK",
							remLine);
						retVal = -1;
					}
				}
				else if (getC != EOF)
				{
					sprintf(
						remLine,
						"format not OK, got 'b1%c', expected 'b128'",
						getC);
					printf(
						msgStr,
						fatal,
						"FMTNOTOK",
						remLine);
					retVal = -1;
				}
				break;
		}
		getC = fgetc(fp);
	}
	if (getC == EOF)
	{
		retVal = 0;
		if (*operandFmt != ieeeUnknown)
		{
			printf("Got EOF with");
			printFormat(*operandFmt, *resultFmt);
		}
	}
	else
	{
		if (*operandFmt == ieeeUnknown)
		{
			printf(
				msgStr,
				fatal,
				"FMTUNKNOWN",
				"operand and result formats are unknown");
			retVal = -1;
		}
	}

	/*
	 * Get the operation to be performed.
	 */
	if (retVal == 1)
	{
		getC = ungetc(getC, fp);
		retVal = readNextToken(fp, workingStr, sizeof(workingStr));
		if (retVal == 1)
		{
			for (ii = 0; ii < AXP_NUM_OPER; ii++)
				if (strcmp(cvtOperStr[ii].operStr, workingStr) == 0)
				{
					*operation = cvtOperStr[ii].oper;
					found = true;
					break;
				}
			if (found == false)
			{
				sprintf(
					remLine,
					"floating-point operation (%s) not found",
					workingStr);
				printf(
					msgStr,
					fatal,
					"OPNOTFOUND",
					remLine);
				retVal = -1;
			}
			else if (getC == EOF)
			{
				retVal = 0;
			}
		}
	}

	/*
	 * Get the rounding mode
	 */
	if (retVal == 1)
	{
		retVal = readNextToken(fp, workingStr, sizeof(workingStr));
		if (retVal == 1)
		{
			found = false;
			for (ii = 0; ii < AXP_NUM_ROUNDING; ii++)
				if (strcmp(cvtRoundingStr[ii].str, workingStr) == 0)
				{
					*roundMode = cvtRoundingStr[ii].round;
					found = true;
					break;
				}
			if (found == false)
			{
				sprintf(
					remLine,
					"floating-point rounding mode (%s) not found",
					workingStr);
				printf(
					msgStr,
					fatal,
					"RNDNOTFOUND",
					remLine);
				retVal = -1;
			}
			else if ((getC == EOF) ||
					 (fgets(remLine, sizeof(remLine), fp) == NULL))
			{
				retVal = 0;
			}
		}

	}
#if 0
		else if ((getC == EOF) ||
				 (fgets(remLine, sizeof(remLine), fp) == NULL))
		{
			retVal = 0;
		}
#endif
	return(retVal);
}

/*
 * openNextFile
 * 	This function is called to open the indicated file and read past the
 * 	header to the first valid input line.  It returns the file pointer.
 *
 * Input Parameters:
 * 	fileName:
 * 		A pointer to a string specifying the file name of the next input file.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	NULL:			There was a problem opening or getting the input file
 * 					ready.
 * 	File Pointer:	Of the now opened and ready to process file.
 */
FILE *openNextFile(char *fileName)
{
	FILE	*fp = NULL;
	char	line[80];
	bool	ready = false;

	fp = fopen(fileName, "r");
	while ((ready == false) && (fp != NULL))
	{
		if (fgets(line, sizeof(line), fp) != NULL)
			ready = strlen(line) == 1;
		else
		{
			fclose(fp);
			fp = NULL;
		}
	}

	/*
	 * Return the file pointer back to the caller.
	 */
	return(fp);
}

/*
 * main
 *	This function is compiled in when unit testing.  It exercises the branch
 *	prediction code, and should be somewhat extensive.
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
int main()
{
	FILE			*fp;
	char			*fileName = "../tst/fpTestData/Basic-Types-Inputs.fptest";
	bool			pass = true;
	AXP_21264_CPU	*cpu;
	AXP_INSTRUCTION	instr;
	AXP_EXCEPTIONS	retValIns;
	AXP_Operation	oper;
	AXP_IEEEFormat	operandFmt;
	AXP_IEEEFormat	resultFmt;
	AXP_RoundingMode roundMode;
	int				testCnt = 0;
	int				retVal;

	/*
	 * NOTE: 	The current simulation takes in one instruction at a time.  The
	 *			AXP simulator will process four instructions at a time and
	 *			potentially out of order.
	 */
	printf("\nAXP 21264 IEEE Floating Point Tester\n");
	cpu = (AXP_21264_CPU *) AXP_Allocate_Block(AXP_21264_CPU_BLK);
	instr.uniqueID = 0;		// This will be incremented for each test.
	instr.aSrc1 = 5;		// Architectural Register (F05).
	instr.src1 = 40;		// Physical Register.
	instr.aSrc2 = 31;		// Architectural Register (F31).
	instr.src2 = 72;		// Physical Register.
	instr.aDest = 29;		// Architectural Register (F29).
	instr.dest = 31;		// Physical Register.
	instr.type_hint_index = 0;
	instr.scbdMask = 0;
	instr.len_stall = 0;
	instr.lockFlagPending = false;
	instr.clearLockPending = false;
	instr.useLiteral = false;
	instr.branchPredict = false;
	instr.literal = 0;
	instr.src2v.r.uq = 0;	// Only 2 registers (Ra = destination, Rb = source)
	instr.lockPhysAddrPending = 0;
	instr.lockVirtAddrPending = 0;
	instr.format = Mem;		// Memory formatted instruction.
	instr.type = Load;		// Load operation.
	instr.pc.pc = 0x000000007ffe000ll;
	instr.pc.pal = 0;		// not PALmode.
	instr.branchPC.pc = 0;
	instr.branchPC.pal = 0;
	instr.state = Retired;	// All instructions are initially Retired.

	fp = openNextFile(fileName);

	if(fp != NULL)
	{
		while ((retVal = parseNextLine(
								fp,
								&oper,
								&operandFmt,
								&resultFmt,
								&roundMode)) != 0)
		{
			switch (retVal)
			{
				case 1:
					pass = true;
					break;

				case -1:
					pass = false;
					break;
			}
		}
		fclose(fp);
	}
	else
	{
		printf("Unable to open test data file: %s\n", fileName);
		pass = false;
	}

	/*
	 * Display the results of the test.
	 */
	 if (pass == true)
		 printf("Test passed.  %d test cases executed.\n", testCnt);
	 else
		 printf("\nTest failed (%d passed before failing).\n", testCnt);
	
	/*
	 * We are done.
	 */
	return(0);
}
