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
 *	This source file contains the main function to test the code dumping code.
 *
 *	Revision History:
 *
 *	V01.000		06-Nov-2017	Jonathan D. Belanger
 *	Initially written.
 *
 *	V01.001		07-Nov-2017	Jonathan D. Belanger
 *	Added functionality to the dump function that changes the functions
 *	parameters and return value.
 *
 */
#include "CommonUtilities/AXP_Dumps.h"

/*
 * Define some values for 1 and 8 Meg
 */
#define EIGHT_M	(ONE_M * 8)

/*
 * Allocate memory in which to store the instructions read from a file.
 */
u8	memory[EIGHT_M];

/*
 * AXP_21264_LoadMemory
 *	This function is called to open a file containing binary data representing
 *	Alpha AXP code and load that data into the memory buffer.
 *
 * Input Parameters:
 *	fileName:
 *		A string containing the name of the binary file to open.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	Bytes loaded (0 means none/error).
 */
int AXP_21264_LoadMemory(char *fileName)
{
  bool	done = false;
  FILE	*fp;
  int		ii = 0;
  u32		scratch;

  fp = fopen(fileName, "r");
  if (fp != NULL)
  {

    /*
     * Skip the fist 576 (0x240) bytes of the file.
     */
    for (ii = 0; ii < 0x240; ii++)
    {
      if (feof(fp))
        break;
      fread(&scratch, 1, 1, fp);
    }

    /*
     * Now start reading the file.
     */
    ii = 0;
    while ((feof(fp) == 0) && (done == false))
    {
      fread(&memory[ii++], 1, 1, fp);
      if (ii >= EIGHT_M)
      {
        printf(
          "Input file %s is too big for %d Meg of memory.\n",
          fileName,
          (EIGHT_M/ONE_M));
        ii = 0;
        done = true;
      }
    }
    fclose(fp);
  }
  else
  {
    printf("Unable to open file: %s\n", fileName);
    ii = 0;
  }
  return(ii-1);	/* remove the EOF */
}

/*
 * main
 *	This function is the main function called to exercise the Digital AXP
 *	Alpha 21264 instruction dump code.
 *
 * Input Parameters:
 *	None.
 *
 * Output Parameters:
 *	None.
 *
 * Return Value:
 *	0 for success
 *	<0 for failure
 */
int main()
{
  char 	decodedLine[256];
  u32		*instr;
  AXP_PC	pc;
  int		retVal = 0;
  int		totalBytesRead = 0;
  int		totalInstructions = 0;
  int		ii;

  printf("\nAXP 21264 Instruction Dumping Tester\n");
  totalBytesRead = AXP_LoadExecutable(
              "../dat/cl67srmrom.exe",
              memory,
              sizeof(memory));
  if (totalBytesRead > 0)
  {
    totalInstructions = totalBytesRead / sizeof(AXP_INS_FMT);
    instr = (u32 *) &memory[0];
    pc.pal = 1;					/* Set the PALmode bit */
    pc.res = 0;
    for (ii = 0; ii < totalInstructions; ii++)
    {
      pc.pc = ii;
      AXP_Decode_Instruction(
        &pc,
        (AXP_INS_FMT) instr[ii],
        false,
        decodedLine);
      printf("%s\n", decodedLine);
    }
  }
  else
    retVal = -1;
  return(retVal);
}
