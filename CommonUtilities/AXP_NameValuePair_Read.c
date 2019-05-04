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
 *	This source file contains the functions needed to implement utilities to
 *	process a file containing a name/value pair and return that to the caller.
 *
 *	Revision History:
 *
 *	V01.000		29-Oct-2017	Jonathan D. Belanger
 *	Initially written.
 */
#include "CommonUtilities/AXP_Utility.h"

/*
 * AXP_Open_NVP_File
 * 	This function is called to open the specified file containing name/value
 * 	pair information.
 *
 * Input Parameters:
 * 	filename:
 * 		A string containing the filename to be opened by this function.
 *
 * Output Parameters:
 * 	None.
 *
 * Return Value:
 * 	NULL:	File was not found.
 * 	fp:		The file pointer for the file to be read.
 */
FILE *AXP_Open_NVP_File(char *filename)
{
    FILE *retVal;

    retVal = fopen(filename, "r");

    /*
     * NOTE: We may want to receive a parse table to convert a string name from
     * the name portion of the NVP to a numeric value.
     *
     * For now, just return back to the caller.
     */

    return (retVal);
}

/*
 * AXP_Close_NVP_File
 * 	This function is called to close the specified file previously opened by
 * 	the open function in this module.
 *
 * Input Parameters:
 * 	filePointer:
 * 		A pointer to the opened file.
 *
 * Output Parameters:
 * 	filePointer:
 * 		Set to NULL.
 *
 * Return Value:
 * 	None.
 */
void AXP_Close_NVP_File(FILE *filePointer)
{
    fclose(filePointer);

    /*
     * Return back to the caller.
     */
    return;
}

/*
 * AXP_Read_NVP_File
 * 	This function is called to read the specified file previously opened by
 * 	the open function in this module and return the next name/value pair.
 *
 * Input Parameters:
 * 	filePointer:
 * 		A pointer to the opened file.
 *
 * Output Parameters:
 * 	name:
 * 		A String to receive the name portion of the NVP.
 * 	value:
 * 		An integer to receive the value portion of the NVP.
 *
 * Return Value:
 * 	true:
 * 		Output parameters have been set.
 * 	false:
 * 		End of file read.
 *
 * NOTE:
 * 	The largest value we can currently receive is is an 32-bit unsigned value.
 * 	Also, we do not check the parameters for validity.
 */
bool AXP_Read_NVP_File(FILE *filePointer, char *name, u32 *value)
{
    char readLine[81];
    bool retVal = true;
    int done = 0; /* -1 = EOF, 0 = not done, 1 = name/value present */
    char *savePtr;
    char *token;

    /*
     * Until we get either a name/value pair to return, and end-of-file, or an
     * error, keep looping through the file.
     */
    while (done == 0)
    {
  if (fgets(readLine, sizeof(readLine), filePointer) != NULL)
  {
      char *comment;
      int ii, jj;

      /*
       * First, starting at the ; converting it and all remaining
       * characters to the null-character.
       */
      jj = -1;
      for (ii = 0; ii < sizeof(readLine); ii++)
    if ((readLine[ii] == ';') || (jj > 0))
    {
        readLine[ii] = '\0';
        jj = 1;
    }

      /*
       * Next collapse the line (remove all space and tab characters.
       */
      for (ii = 1; ii < strlen(readLine); ii++)
      {
    if ((readLine[ii - 1] == ' ') || (readLine[ii - 1] == '\t'))
    {
        for (jj = ii - 1; jj < strlen(readLine) - 1; jj++)
      readLine[jj] = readLine[jj + 1];
        jj = strlen(readLine);
        readLine[jj - 1] = '\0';
    }
      }

      /*
       * Remove all text after the comment string and the comment string
       * itself.
       */
      comment = strstr(readLine, "//");
      if (comment != NULL)
      {
    for (ii = (comment - readLine); ii < sizeof(readLine); ii++)
        readLine[ii] = '\0';
      }

      /*
       * If we have anything left of the string, then go and parse it.
       * Otherwise, we need to read the next line.
       */
      if (strlen(readLine) > 0)
      {
    const char delim[2] = "=";

    /*
     * At this point we should have something that looks like
     * "name=value", with nothing before it or after.  First, let's
     * pull out the name.  If we don't find one it is an error.
     */
    savePtr = NULL;
    token = strtok_r(readLine, delim, &savePtr);
    if (token != NULL)
    {
        char *valueStr;

        strcpy(name, token);

        /*
         * Now pull out the value string.  If we don't find one,
         * then return an error.
         */
        valueStr = strtok_r(NULL, delim, &savePtr);
        if (valueStr != NULL)
        {

      /*
       * Convert the value string to an U32 value and mark
       * that we are done and returning a name/value pair.
       */
      *value = (u32) strtol(valueStr, NULL, 0);
      done = 1;
        }
        else
        {
      printf(
              "Parsing error: 'value' not present error at %s, line %d.\n",
              __FILE__, __LINE__);
      done = -1;
        }
    }
    else
    {
        printf(
          "Parsing error: 'name' not present error at %s, line %d.\n",
          __FILE__, __LINE__);
        done = -1;
    }
      }
  }
  else
      done = -1; /* End-Of-File detected. */
    }

    /*
     * If we reached end-of file, then return a value of false
     */
    if (done == -1)
  retVal = false;

    /*
     * Return back to the caller.
     */
    return (retVal);
}
