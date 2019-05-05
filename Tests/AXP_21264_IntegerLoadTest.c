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
#include "CommonUtilities/AXP_Utility.h"
#include "CommonUtilities/AXP_Blocks.h"
#include "CPU/AXP_21264_Instructions.h"
#include "CPU/AXP_21264_CPU.h"
#include "CPU/Ebox/AXP_21264_Ebox.h"

/*
 * NOTE:	We need to run a series of tests to utilize the various
 *		load/store instructions at various parts of the load/store
 *		process.  We can use the existing test data.
 */

enum readSt
{
  zeroX,
  hexValue
};

bool readNextHex(FILE *fp, void *hex, int len)
{
  bool		done = false;
  char		ch;
  u8			*one = (u8 *) hex;
  u16			*two = (u16 *) hex;
  u32			*four = (u32 *) hex;
  u64			*eight = (u64 *) hex;
  u8			hexVal;
  int			ret;
  int			cnt;
  enum readSt	state = zeroX;

  *one = 0;
  *two = 0;
  *four = 0;
  *eight = 0;
  cnt = 0;
  while (done == false)
  {
    ret = fgetc(fp);
    if (ret != EOF)
    {
      cnt++;
      ch = (u8) ret;
      if ((state == hexValue) &&
        (((ch >= '0') && (ch <= '9')) ||
         ((ch >= 'a') && (ch <= 'f'))))
      {
        if ((ch >= '0') && (ch <= '9'))
          hexVal = ch - '0';
        else
          hexVal = 0x0a + (ch - 'a');
        switch (len)
        {
          case 1:
            *one = (*one << 4) + hexVal;
            break;

          case 2:
            *two = (*two << 4) + hexVal;
            break;

          case 4:
            *four = (*four << 4) + hexVal;
            break;

          case 8:
            *eight = (*eight << 4) + hexVal;
            break;
        }
      }
      else if (ch == 'x')
        state = hexValue;
      else if (ch != '0')
      {
        if (ch == 0x0d)
          ret = getc(fp);
        done = true;
      }
    }
    else
      done = true;
  }
  return(cnt == ((len * 2) + 3));
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
  char			*fileName = "../tst/LDA_LDAH_TestData.csv";
  char			headerStr[81];
  bool			pass = true;
  u64				ldaAddress, ldahAddress;
  AXP_21264_CPU	*cpu;
  AXP_INSTRUCTION	instr;
  AXP_EXCEPTIONS	retVal;
  int				testCnt = 0;

  /*
   * NOTE: 	The current simulation takes in one instruction at a time.  The
   *			AXP simulator will process four instructions at a time and
   *			potentially out of order.
   */
  printf("\nAXP 21264 Integer Load/Store Tester\n");
  cpu = (AXP_21264_CPU *) AXP_Allocate_Block(AXP_21264_CPU_BLK);
  instr.uniqueID = 0;		/* This will be incremented for each test. */
  instr.aSrc1 = 5;		/* Architectural Register (R05). */
  instr.src1 = 40;		/* Physical Register. */
  instr.aSrc2 = 31;		/* Architectural Register (R31). */
  instr.src2 = 72;		/* Physical Register. */
  instr.aDest = 29;		/* Architectural Register (R29). */
  instr.dest = 31;		/* Physical Register. */
  instr.type_hint_index = 0;
  instr.scbdMask = 0;
  instr.quadword = false;
  instr.stall = false;
  instr.useLiteral = false;
  instr.branchPredict = false;
  instr.literal = 0;
  instr.src2v.r.uq = 0;	/* Only 2 registers (Ra = destination, Rb = source) */
  instr.format = Mem;		/* Memory formatted instruction. */
  instr.type = Load;		/* Load operation. */
  instr.pc.pc = 0x000000007ffe000ll;
  instr.pc.pal = 0;		/* not PALmode. */
  instr.branchPC.pc = 0;
  instr.branchPC.pal = 0;
  instr.state = Retired;	/* All instructions are initially Retired. */

  fp = fopen(fileName, "r");
  if(fp != NULL)
  {
    printf("\n Processing Test Data File: %s\n", fileName);
    fscanf(fp, "%80s", headerStr);		/* read first line */
    headerStr[0] = (char) fgetc(fp);	/* new-line */
    headerStr[0] = (char) fgetc(fp);	/* carriage-return */
    if (readNextHex(fp, (u8 *) &ldahAddress, 8))
    {
      if (readNextHex(fp, (u8 *) &ldaAddress, 8))
      {
        if (readNextHex(fp, (u8 *) &instr.displacement, 2))
        {
          pass = readNextHex(fp, (u8 *) &instr.src1v.r.uq, 8);
        }
        else
          pass = false;
      }
      else
        pass = false;
    }
    else
      pass = false;
    while ((feof(fp) == 0) && (pass == true))
    {

      /*
       * Sign extend the displacement.
       */
      instr.displacement |=
          ((instr.displacement & 0x0000000000008000ll) ?
           0xffffffffffff0000 : 0);
      instr.state = Executing;
      testCnt++;
      retVal = AXP_LDA(cpu, &instr);		/* Call the LDA instruction. */
      if (retVal == NoException)
      {
        if (instr.state == WaitingRetirement)
        {
          if (instr.destv.r.uq != ldaAddress)
          {
            printf("LDA failed: Rbv = 0x%016llx, disp = 0x%04llx, Rav = 0x%016llx (expected: 0x%016llx)\n",
              instr.src1v.r.uq,
              instr.displacement,
              instr.destv.r.uq,
              ldaAddress);
            pass = false;
            testCnt--;
          }
          else
          {
            instr.pc.pc++;
            instr.uniqueID++;
          }
        }
        else
        {
          printf("LDA failed in instruction state!\n");
          pass = false;
          testCnt--;
        }
      }
      else
      {
        printf("LDA failed in instruction return value!\n");
        pass = false;
        testCnt--;
      }
      if (pass == true)
      {
        instr.destv.r.uq = 0;
        instr.state = Executing;
        testCnt++;
        retVal = AXP_LDAH(cpu, &instr);		/* Call the LDAH - Load Address High instruction. */
        if (retVal == NoException)
        {
          if (instr.state == WaitingRetirement)
          {
            if (instr.destv.r.uq != ldahAddress)
            {
              printf("LDAH failed: Rbv = 0x%016llx, disp = 0x%04llx, Rav = 0x%016llx (expected: 0x%016llx)\n",
                instr.src1v.r.uq,
                instr.displacement,
                instr.destv.r.uq, ldahAddress);
              pass = false;
              testCnt--;
            }
            else
            {
              instr.pc.pc++;
              instr.uniqueID++;
            }
          }
          else
          {
            printf("LDAH failed in instruction state!\n");
            pass = false;
            testCnt--;
          }
        }
        else
        {
          printf("LDAH failed in instruction return value!\n");
          pass = false;
          testCnt--;
        }
      }
      if (readNextHex(fp, (u8 *) &ldahAddress, 8))
      {
        if (readNextHex(fp, (u8 *) &ldaAddress, 8))
        {
          if (readNextHex(fp, (u8 *) &instr.displacement, 2))
          {
            pass = readNextHex(fp, (u8 *) &instr.src1v.r.uq, 8);
          }
          else
            pass = false;
        }
        else
          pass = false;
      }
      else
        pass = false;
    }	/* while (!EOF && passing) */
    pass = (feof(fp) != 0);		/* EOF is not a failure. */
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
     printf("\nTest failed (%d passed before failing).  See preceding message.\n", testCnt);

  /*
   * We are done.
   */
  return(0);
}
