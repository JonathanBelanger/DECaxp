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
 *
 *	V01.001		12-Jul-2017	Jonathan D. Belanger
 *	Added a skipped test counter.
 */
#include <math.h>
#include <float.h>

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
	addAction,
	subtractAction,
	multiplyAction,
	divideAction,
	multiplyAddAction,
	squareRootAction,
	remainderAction,
	roundFloatToIntAction,
	convertFloatToFloatAction,
	convertFloatToIntAction,
	convertIntToFloatAction,
	convertToDecimalStrAction,
	convertDecimalStrToFloatAction,
	quietComparisonAction,
	signalingComparisonAction,
	copyAction,
	negateAction,
	absoluteValueAction,
	copySignAction,
	scalbAction,
	logbAction,
	nextAfterAction,
	classAction,
	isSignedAction,
	isNormalAction,
	isFiniteAction,
	isZeroAction,
	isSubnormalAction,
	isInfiniteAction,
	isNotANumberAction,
	isSignalingAction,
	minNumAction,
	maxNumAction,
	minNumMagAction,
	maxNumMagAction,
	sameQuantumAction,
	quantizeAction,
	nextUpAction,
	nextDownAction,
	equivalentAction
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
	{"+", addAction, "add"},
	{"-", subtractAction, "subtract"},
	{"*", multiplyAction, "multiply"},
	{"/", divideAction, "divide"},
	{"*+", multiplyAddAction, "multiply-add"},
	{"V", squareRootAction, "square root"},
	{"%", remainderAction, "remainder"},
	{"rfi", roundFloatToIntAction, "round float to integer"},
	{"cff", convertFloatToFloatAction, "convert float to float"},
	{"cfi", convertFloatToIntAction, "convert float to integer"},
	{"cif", convertIntToFloatAction, "convert integer to float"},
	{"cfd", convertToDecimalStrAction, "convert to decimal string"},
	{"cdf", convertDecimalStrToFloatAction, "convert decimal string to float"},
	{"qC", quietComparisonAction, "quiet comparison"},
	{"sC", signalingComparisonAction, "signaling comparison"},
	{"cp", copyAction, "copy"},
	{"~", negateAction, "negate"},
	{"A", absoluteValueAction, "absolute value"},
	{"@", copySignAction, "copy sign"},
	{"S", scalbAction, "scalb"},
	{"L", logbAction, "logb"},
	{"Na", nextAfterAction, "next after"},
	{"?", classAction, "class"},
	{"?-", isSignedAction, "is signed"},
	{"?n", isNormalAction, "is normal"},
	{"?f", isFiniteAction, "is finite"},
	{"?0", isZeroAction, "is zero"},
	{"?s", isSubnormalAction, "is subnormal"},
	{"?i", isInfiniteAction, "is infinite"},
	{"?N", isNotANumberAction, "is not a number"},
	{"?sN", isSignalingAction, "is signaling"},
	{"<C", minNumAction, "min-num"},
	{">C", maxNumAction, "max-num"},
	{"<A", minNumMagAction, "min-num-mag"},
	{">A", maxNumMagAction, "max-num-mag"},
	{"=quant", sameQuantumAction, "same quantum"},
	{"quant", quantizeAction, "quantize"},
	{"Nu", nextUpAction, "next up"},
	{"Nd", nextDownAction, "next down"},
	{"eq", equivalentAction, "equivalent"}
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

typedef enum
{
	none,
	inexact,
	underflow,
	overflow,
	divisionByZero,
	invalid
} AXP_TrappedException;

typedef struct
{
	char					exceptionChar;
	AXP_TrappedException	exception;
	char					*humanReadable;
} AXP_cvtTExceptionChar;

#define AXP_NUM_EXCEPTION	6

AXP_cvtTExceptionChar cvtTExceptionChar[AXP_NUM_EXCEPTION] =
{
	{'\0', none, "none"},
	{'x', inexact, "inexact"},
	{'u', underflow, "underflow"},
	{'o', overflow, "overflow"},
	{'z', divisionByZero, "division by zero"},
	{'i', invalid, "invalid"},
};


#define AXP_NUM_OUT_EXCEPTION	8


char *cvtEncoding[] =
{
	"Reserved",
	"Zero",
	"Finite",
	"Denormal",
	"Infinity",
	"NotANumber",
	"DirtyZero"
};


char *msgStr = "%%AXP-%c-%s, %s.\n";
char fatal = 'F';
char error = 'E';
char info = 'I';
char success = 'S';

void ieeeFormatToStr(AXP_IEEEFormat ieeeFmt, char *retVal)
{
	switch(ieeeFmt)
	{
		case ieeeS:
			strcpy(retVal,"'IEEE S'");
			break;

		case ieeeT:
			strcpy(retVal, "'IEEE T'");
			break;

		case ieeeX:
			strcpy(retVal, "'IEEE X'");
			break;

		case ieeeUnknown:
			strcpy(retVal, "'IEEE Unknown'");
			break;
	}
	return;
}

void printFormat(AXP_IEEEFormat operand, AXP_IEEEFormat result)
{
	char fmt1[10];
	char fmt2[10];

	ieeeFormatToStr(operand, fmt1);
	if (operand == result)
		printf("operand & result format set to %s\n", fmt1);
	else
	{
		ieeeFormatToStr(result, fmt2);
		printf("operand format set to %s, ", fmt1);
		printf("result format set to %s\n",  fmt2);
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
 * parseOperands
 * 	This function is called to parse the input and output operands, one at a
 * 	time, from the input format to the appropriate IEEE S and IEEE T floating
 * 	point format.
 *
 * Input Parameters:
 * 	inputStr:
 * 		A pointer to a string containing the data to be parsed.
 * 	inputFormat:
 * 		A value indicating the type of data should be present in the inputStr.
 *
 * Output Parameters:
 * 	whichResult:
 * 		A pointer to an to receive an indicator that the result is a
 * 		boolean and can be found in the result parameter.
 * 	floatResult:
 * 		A pointer to a structure containing the format of all possible
 * 		supported floating point types within a 64-bit register.
 * 	result:
 * 		A pointer to a boolean to receive a boolean value as the expected
 * 		result.
 *
 * Return Value:
 * 	1:	success
 * 	-1:	failure
 */
i32 parseOperands(
	char *inputStr,
	AXP_IEEEFormat inputFormat,
	int *whichResults,
	AXP_FP_REGISTER *fpReg,
	bool *result)
{
	int retVal = 1;

	if (inputStr[0] == '-')
		fpReg->fpr.sign = 1;
	else if (inputStr[0] == 'Q')
	{
		*whichResults = 1;
		fpReg->uq = (inputFormat == ieeeS) ? AXP_S_CQ_NAN : AXP_T_CQ_NAN;
		return(retVal);
	}
	else if (inputStr[0] == 'S')
	{
		*whichResults = 1;
		fpReg->uq = (inputFormat == ieeeS) ? AXP_S_CS_NAN : AXP_T_CS_NAN;
		return(retVal);
	}
	if (inputStr[1] == 'I')
	{
		*whichResults = 1;
		fpReg->fpr.exponent = 0x7ff;
	}
	else if (inputStr[1] == 'Z')
	{
		*whichResults = 1;
		fpReg->fpr.exponent = 0;
		fpReg->fpr.fraction = 0;
	}
	else
	{

		/*
	 	 * For now, we only support binary floating point
		 * format and boolean formats.  Also, we assume that
		 * the boolean format is only used for results.
		 */
		if (inputStr[1] == 'x')
		{
			*whichResults = -1;
			*result = (inputStr[2] == '1');
		}
		else
		{
			char *ptr;
			bool normal;

			*whichResults = 1;
			normal = inputStr[1] == '1';
			if (inputFormat == ieeeS)
			{
				u16	exponent;

				fpReg->fpr32.fraction = strtol(&inputStr[3], &ptr, 16);
				if (normal == true)
				{
					ptr++;
					exponent = (atoi(ptr) + AXP_S_BIAS) & 0xff;
					fpReg->fpr32.exponent = ((exponent & 0x80) << 3) | (exponent & 0x7f);
					fpReg->fpr32.exponent |=
						(exponent & 0x80) ?
							((exponent & 0x3f) != 0x3f ? 0 : 0x380) :
							(exponent ? 0x380 : 0);
				}
				else
					fpReg->fpr32.exponent = 0;
				fpReg->fpr32.zero = 0;
			}
			else
			{
				fpReg->fpr.fraction = strtol(&inputStr[3], &ptr, 16);
				ptr++;
				if (normal == true)
					fpReg->fpr.exponent = (atoi(ptr) + AXP_T_BIAS) & 0x3ff;
				else
					fpReg->fpr.exponent = 0;
			}
		}
	}

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
		AXP_RoundingMode *roundMode,
		AXP_TrappedException *exception,
		AXP_FP_REGISTER *src1,
		AXP_FP_REGISTER *src2,
		AXP_FP_REGISTER *src3,
		int				*useResult,
		AXP_FP_REGISTER *dest,
		bool			*result,
		AXP_FBOX_FPCR	*expectedFPCR)
{
	AXP_FP_REGISTER	*nextFPReg[3] = {src1, src2, src3};
	bool			found = false;
	i32				retVal = 1;
	char			workingStr[80];
	int				nextFPRegIdx = 0;
	int				getC;
	int				ii;

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
							workingStr,
						"format not OK, got 'b3%c', expected 'b32'",
						*((char *) &getC));
					printf(
						msgStr,
						fatal,
						"FMTNOTOK",
						workingStr);
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
						workingStr,
						"format not OK, got 'b6%c', expected 'b164'",
						getC);
					printf(
						msgStr,
						fatal,
						"FMTNOTOK",
						workingStr);
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
							workingStr,
							"format not OK, got 'b12%c', expected 'b128'",
							getC);
						printf(
							msgStr,
							fatal,
							"FMTNOTOK",
							workingStr);
						retVal = -1;
					}
				}
				else if (getC != EOF)
				{
					sprintf(
						workingStr,
						"format not OK, got 'b1%c', expected 'b128'",
						getC);
					printf(
						msgStr,
						fatal,
						"FMTNOTOK",
						workingStr);
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
					workingStr,
					"floating-point operation (%s) not found",
					workingStr);
				printf(
					msgStr,
					fatal,
					"OPNOTFOUND",
					workingStr);
				retVal = -1;
			}
			else if (getC == EOF)
				retVal = 0;
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
					workingStr,
					"floating-point rounding mode (%s) not found",
					workingStr);
				printf(
					msgStr,
					fatal,
					"RNDNOTFOUND",
					workingStr);
				retVal = -1;
			}
			else if (getC == EOF)
				retVal = 0;
		}

	}

	/*
	 * Get the trapped exceptions, if any.
	 */
	if (retVal == 1)
	{

		/*
		 * First things first, set the trapped exception to 'none'.
		 */
		*exception = none;

		/*
		 * OK, the trapped expression may or may not be present.  If it is
		 * present, then parse it an read in the next space character, so that
		 * we are ready to parse the input parameters.  IF it is not present,
		 * then un-get it and move on to the next parsing stage.
		 */
		getC = fgetc(fp);
		if (getC != EOF)
		{
			found = false;
			for (ii = 1; ii < AXP_NUM_EXCEPTION; ii++)
			{
				if (cvtTExceptionChar[ii].exceptionChar == getC)
				{
					*exception = cvtTExceptionChar[ii].exception;
					found = true;
					break;
				}
			}

			/*
			 * If we did not find the exception, then we are processing the
			 * first input value.  Put the character back onto the character
			 * stream.  Otherwise, read the next character (it is a space).
			 */
			if (found != true)
				getC = ungetc(getC, fp);
			else
			{
				if (fgetc(fp) == EOF)
					retVal = 0;
			}
		}
		else
			retVal = 0;
	}

	/*
	 * Get the input operands (may be up to 3).
	 */
	src1->uq = src2->uq = src3->uq = 0;
	if (retVal == 1)
	{
		int		tmpBoolResults;
		bool	tmpResults;

		retVal = readNextToken(fp, workingStr, sizeof(workingStr));
		while ((strcmp("->", workingStr) != 0) && (retVal == 1))
		{
			retVal =  parseOperands(
						workingStr,
						*operandFmt,
						&tmpBoolResults,
						nextFPReg[nextFPRegIdx++],
						&tmpResults);
			if (retVal == 1)
				retVal = readNextToken(fp, workingStr, sizeof(workingStr));
		}
	}

	/*
	 * Get the output operand.
	 */
	dest->uq = 0;
	*useResult = 0;
	if (retVal == 1)
	{
		retVal = readNextToken(fp, workingStr, sizeof(workingStr));
		if (retVal == 1)
		{
			if (workingStr[0] != '#')
				retVal =  parseOperands(
					workingStr,
					*resultFmt,
					useResult,
					dest,
					result);
			else
				*useResult += 1;
		}
	}

	/*
	 * Get the output exceptions.
	 */
	if (retVal == 1)
	{

		/*
		 * First things first, set the trapped exception to 'none'.
		 */
		expectedFPCR->sum = 0;
		expectedFPCR->ined = 0;
		expectedFPCR->unfd = 0;
		expectedFPCR->undz = 0;
		expectedFPCR->dyn = 0;
		expectedFPCR->iov = 0;
		expectedFPCR->ine = 0;
		expectedFPCR->unf = 0;
		expectedFPCR->ovf = 0;
		expectedFPCR->dze = 0;
		expectedFPCR->inv = 0;
		expectedFPCR->ovfd = 0;
		expectedFPCR->dzed = 0;
		expectedFPCR->invd = 0;
		expectedFPCR->dnz = 0;
		expectedFPCR->res = 0;

		/*
		 * OK, the trapped expression may or may not be present.  If it is
		 * present, then parse it an read in the next space character, so that
		 * we are ready to parse the input parameters.  IF it is not present,
		 * then un-get it and move on to the next parsing stage.
		 */
		found = false;
		getC = fgetc(fp);
		while ((getC != EOF) && (getC != ' ') && (getC != '\n'))
		{
			switch (getC)
			{
				case 'x':
					expectedFPCR->ine = 1;
					found = true;
					break;

				case 'u':
				case 'v':
				case 'w':
					expectedFPCR->unf = 1;
					found = true;
					break;

				case 'o':
					expectedFPCR->ovf = 1;
					found = true;
					break;

				case 'z':
					expectedFPCR->dze = 1;
					found = true;
					break;

				case 'i':
					expectedFPCR->inv = 1;
					found = true;
					break;
			}

			/*
			 * Get the next character.
			 */
			getC = fgetc(fp);
		}
		if (found == true)
		{
			expectedFPCR->sum = 1;
			*useResult += 1;
		}
		else
			*useResult -= 1;
		if (getC == EOF)
			retVal = 0;
	}
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

/****************************************************************************
 *	The following code represents, either functions/macros that do not		*
 *	exist, or are not implemented correctly in the C RTL.  I am, therefore,	*
 *	implementing my own (corrected or implemented).							*
 ****************************************************************************/

/*
 * isSignaling
 * 	This function seems to be missing from the GCC C RTL.  So, I'm going to
 * 	implement one of my own.  I've up-cased the second 'S' so that there will
 * 	not be a conflict if there is an 'issignaling' function in the C RTL in the
 * 	future.  Hey, look at me future-proofing this.  Go me.
 *
 * Input Parameters:
 * 	fpv:
 * 		A value containing the float to be tested.
 *
 * Output Paramteres:
 * 	None.
 *
 * Return Value:
 * 	true:	If the value in fpv is a signaling NaN.
 * 	false:	If the value is fpv is not a signaling NaN.
 */
bool isSignaling(double fpv)
{
	AXP_FPR_QNAN_REGISTER *Tfpv = (AXP_FPR_QNAN_REGISTER *) &fpv;

	return((isnan(fpv)) && (Tfpv->quiet == 0));
}

/*
 * isSigned
 * 	This function seems to be missing from the GCC C RTL.  So, I'm going to
 * 	implement one of my own.  I've up-cased the second 'S' so that there will
 * 	not be a conflict if there is an 'issigned' function in the C RTL in the
 * 	future.
 *
 * Input Parameters:
 * 	fpv:
 * 		A value containing the float to be tested.
 *
 * Output Paramteres:
 * 	None.
 *
 * Return Value:
 * 	true:	If the value in fpv is a signed.
 * 	false:	If the value is fpv is not signed.
 */
bool isSigned(double fpv)
{
	AXP_FPR_REGISTER *Tfpv = (AXP_FPR_REGISTER *) &fpv;

	return((isnan(fpv) == false) && (Tfpv->sign == 1));
}

/*
 * isInf
 * 	This function seems to be incorrect in the GCC C RTL.  The C RTL one does
 * 	not correctly detect -Inf.  So, I'm going to implement one of my own,
 * 	corrected version.  I've up-cased the second 'I' so that there will not be
 * 	a conflict if with the one provided by the C RTL.
 *
 * Input Parameters:
 * 	fpv:
 * 		A value containing the float to be tested.
 *
 * Output Paramteres:
 * 	None.
 *
 * Return Value:
 * 	true:	If the value in fpv is Infinity (+/1).
 * 	false:	If the value is fpv is not Infinity (+/1).
 */
bool isInf(double fpv)
{
	AXP_FPR_REGISTER *fpr = (AXP_FPR_REGISTER *) &fpv;

	return(AXP_FP_ENCODE(fpr, true) == Infinity);
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
	AXP_TrappedException trappedException;
	AXP_FP_REGISTER	src3v;
	AXP_FP_REGISTER	expectedResults;
	AXP_FP_ENCODING encoding;
	int				useResults;
	bool			results;
	AXP_FBOX_FPCR	FPCR;
	int				testCnt = 0, passed = 0, failed = 0, skipped = 0;
	int				retVal;
	double			*Ra, *Rb, *Rc, *expectedRc;

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
	instr.aSrc2 = 6;		// Architectural Register (F06).
	instr.src2 = 41;		// Physical Register.
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
	instr.lockPhysAddrPending = 0;
	instr.lockVirtAddrPending = 0;
	instr.format = FP;		// Memory formatted instruction.
	instr.type = Other;		// Load operation.
	instr.pc.pc = 0x000000007ffe000ll;
	instr.pc.pal = 0;		// not PALmode.
	instr.branchPC.pc = 0;
	instr.branchPC.pal = 0;
	instr.state = Retired;	// All instructions are initially Retired.
	*((u64 *) &instr.insFpcr) = 0;
	*((u64 *) &instr.excSum) = 0;
	instr.excRegMask = 0;	// Exception Register Mask
	instr.loadCompletion = NULL;

	fp = openNextFile(fileName);

	if(fp != NULL)
	{
		Ra = (double *) &instr.src1v.fp.uq;
		Rb = (double *) &instr.src2v.fp.uq;
		Rc = (double *) &instr.destv.fp.uq;
		expectedRc = (double *) &expectedResults;
		while ((retVal = parseNextLine(
								fp,
								&oper,
								&operandFmt,
								&resultFmt,
								&roundMode,
								&trappedException,
								&instr.src1v.fp,
								&instr.src2v.fp,
								&src3v,
								&useResults,
								&expectedResults,
								&results,
								&FPCR)) != 0)
		{
			testCnt++;
			switch (retVal)
			{
				case 1:
					encoding = AXP_FP_ENCODE(&instr.src1v.fp.fpr, true);
					switch (oper)
					{
						/*
						 * TODO: It may be worth utilizing the comparison
						 * instructions (CMPTxx) below, instead of the compiler
						 * provided ones.
						 *
						 * TODO: We should look at also executing the
						 * instructions for T, G, and F floats.  This is
						 * starting to look like a separate testing function.
						 */
						case addAction:
						case copySignAction:
							printf("%7d: ", testCnt);
							if (oper == addAction)
								printf("ADDS");
							else
								printf("CPYS");
							printf("(%d): Ra: %f, Rb: %f; expecting %f (0x%016llx) --> ",
								useResults, *Ra, *Rb, *expectedRc, *((u64 *) &FPCR));
							instr.opcode = FLTI;
							memset(&instr.insFpcr, 0, sizeof(u64));
							if (oper == addAction)
							{
								instr.function = AXP_FUNC_ADDS;
								retValIns = AXP_ADDS(cpu, &instr);
							}
							else
							{
								instr.function = AXP_FUNC_CPYS;
								retValIns = AXP_CPYS(cpu, &instr);
							}
							if (useResults == 2)
							{

								/*
								 * We are only expected an exception.  No need
								 * to compare the expected Rc to the returned
								 * Rc.  Just compare the expected FPCR to the
								 * one in the instruction.
								 */
								if (memcmp(&FPCR, &instr.insFpcr, sizeof(FPCR)) == 0)
								{
									passed++;
									printf("passed");
								}
								else
								{
									pass = false;
									failed++;
									printf(" failed FPCR: 0x%016llx",
										*((u64 *) &instr.insFpcr));
								}
							}
							else if ((useResults == 1) || (useResults == 3))
							{

								/*
								 * OK, we got 2 things to check out.  First,
								 * that the returned Rc is equal to the
								 * expected Rc.
								 */
								if (*expectedRc == *Rc)
								{

									/*
									 * OK, the values matched.  Now we need to
									 * check the returned exceptions.
									 */
									if (memcmp(&FPCR, &instr.insFpcr, sizeof(FPCR)) == 0)
									{
										passed++;
										printf("passed");
									}
									else
									{
										pass = false;
										failed++;
										printf(
											"Rc values matched, exception failed FPCR: 0x%016llx",
											*((u64 *) &instr.insFpcr));
									}
								}
								else
								{

									/*
									 * OK, the values did not match.  If we
									 * were supposed to get exceptions, then
									 * let's check for their correctness.
									 * Otherwise, we failed.
									 */
									if (useResults == 3)
									{
										pass = false;
										failed++;

										/*
										 * OK, the values did not matched.  Now
										 * we need to check the returned
										 * exceptions.
										 */
										if (memcmp(&FPCR, &instr.insFpcr, sizeof(FPCR)) == 0)
										{
											pass = false;
											failed++;
											printf(
												"Rc %f, FPCRs matched.", *Rc);
										}
										else
										{
											pass = false;
											failed++;
											printf(
												"Nothing matched, Rc: %f, FPCR: 0x%016llx",
												*Rc,
												*((u64 *) &instr.insFpcr));
										}

									}
									else
									{
										pass = false;
										failed++;
										printf(" failed Rc: %f ( 0x%016llx)",
											*Rc, *((u64 *) &instr.insFpcr));
									}
								}
							}
							else
							{

								/*
								 * We were not expecting anything.  There is
								 * probably some test that can occur, but I
								 * have no idea what it might be.  I'm just
								 * going to blindly assume we passed.
								 */
								passed++;
								printf("passed");
							}
							printf(", AXP Exception: %d\n", retValIns);
							break;

						case isSignedAction:
							printf("%7d: isSigned: %f, expecting %d --> ",
								testCnt+4, *Ra, results);
							if (isSigned(*Ra) == results)
							{
								passed++;
								printf("passed\n");
							}
							else
							{
								pass = false;
								failed++;
								printf("failed (0x%016llx)\n",
									instr.src1v.fp.uq);
							}
							break;

						case isNormalAction:
							printf("%7d: isNormal (%s): %f, expecting %d --> ",
								testCnt+4, cvtEncoding[encoding], *Ra, results);
							if (isnormal(*Ra) == results)
							{
								passed++;
								printf("passed\n");
							}
							else
							{
								pass = false;
								failed++;
								printf("failed (0x%016llx)\n",
									instr.src1v.fp.uq);
							}
							break;

						case isFiniteAction:
							printf("%7d: isFinite (%s): %f, expecting %d --> ",
								testCnt+4, cvtEncoding[encoding], *Ra, results);
							if (isfinite(*Ra) == results)
							{
								passed++;
								printf("passed\n");
							}
							else
							{
								pass = false;
								failed++;
								printf("failed (0x%016llx)\n",
									instr.src1v.fp.uq);
							}
							break;

						case isZeroAction:
							printf("%7d: isZero (%s): %f, expecting %d --> ",
								testCnt+4, cvtEncoding[encoding], *Ra, results);
							if ((*Ra == 0.0) == results)
							{
								passed++;
								printf("passed\n");
							}
							else
							{
								pass = false;
								failed++;
								printf("failed (0x%016llx)\n",
									instr.src1v.fp.uq);
							}
							break;

						case isSubnormalAction:
							printf("%7d: isSubnormal (%s): %f, expecting %d --> ",
									testCnt+4, cvtEncoding[encoding], *Ra, results);
							if ((fpclassify(*Ra) == FP_SUBNORMAL) == results)
							{
								passed++;
								printf("passed\n");
							}
							else
							{
								pass = false;
								failed++;
								printf("failed (0x%016llx)\n",
									instr.src1v.fp.uq);
							}
							break;

						case isInfiniteAction:
							printf("%7d: isInfinite (%s): %f, expecting %d --> ",
								testCnt+4, cvtEncoding[encoding], *Ra, results);
							if (isInf(*Ra) == results)
							{
								passed++;
								printf("passed\n");
							}
							else
							{
								pass = false;
								failed++;
								printf("failed (0x%016llx)\n",
									instr.src1v.fp.uq);
							}
							break;

						case isNotANumberAction:
							printf("%7d: isNotANumber (%s): %f, expecting %d --> ",
								testCnt+4, cvtEncoding[encoding], *Ra, results);
							if (isnan(*Ra) == results)
							{
								passed++;
								printf("passed\n");
							}
							else
							{
								pass = false;
								failed++;
								printf("failed (0x%016llx)\n",
									instr.src1v.fp.uq);
							}
							break;

						case isSignalingAction:
							printf("%7d: isSignaling (%s): %f, expecting %d --> ",
								testCnt+4, cvtEncoding[encoding], *Ra, results);
							if (isSignaling(*Ra) == results)
							{
								passed++;
								printf("passed\n");
							}
							else
							{
								pass = false;
								failed++;
								printf("failed (0x%016llx)\n",
									instr.src1v.fp.uq);
							}
							break;

						/*
						 * These are not yet implemented.
						 */
						case subtractAction:
						case multiplyAction:
						case divideAction:
						case multiplyAddAction:
						case squareRootAction:
						case remainderAction:
						case roundFloatToIntAction:
						case convertFloatToFloatAction:
						case convertFloatToIntAction:
						case convertIntToFloatAction:
						case convertToDecimalStrAction:
						case convertDecimalStrToFloatAction:
						case quietComparisonAction:
						case signalingComparisonAction:
						case copyAction:
						case negateAction:
						case absoluteValueAction:
						case scalbAction:
						case logbAction:
						case nextAfterAction:
						case classAction:
						case minNumAction:
						case maxNumAction:
						case minNumMagAction:
						case maxNumMagAction:
						case sameQuantumAction:
						case quantizeAction:
						case nextUpAction:
						case nextDownAction:
						case equivalentAction:
							skipped++;
							break;
					}
					break;

				case -1:
					printf("%7d: >>>>>> ReadNextLine Failed <<<<<\n",
						testCnt+4);
					pass = false;
					failed++;
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

	float			sVal = 3.1415926;
	AXP_S_MEMORY	*Sbits = (AXP_S_MEMORY *) & sVal;
	double			tVal = -3.1415926;
	AXP_T_MEMORY	*Tbits = (AXP_T_MEMORY *) &tVal;
	printf("S: 0x%08x (s: %d, exp: 0x%02x, frac: 0x%06x)\n",
		*((u32 *) &sVal), Sbits->sign, Sbits->exponent, Sbits->fraction);
	printf("T: 0x%016llx (s: %d, exp: 0x%03x, frac: 0x%013llx)\n",
		*((u64 *) &tVal), Tbits->sign, Tbits->exponent, (u64) Tbits->fraction);

	/*
	 * Display the results of the test.
	 */
	 if (pass == true)
		 printf("\n% d tests passed, %d tests skipped, and %d test cases executed.\n",
			passed,
			skipped,
			testCnt);
	 else
		 printf("\n%d tests passed, %d failed, and %d skipped, with a total of %d tests.\n",
			passed,
			failed,
			skipped,
			testCnt);
	
	/*
	 * We are done.
	 */
	return(0);
}
