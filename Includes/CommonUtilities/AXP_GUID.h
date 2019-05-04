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
 *  This header file contains the Globally Unique Identifier definition used
 *  by Microsoft and others.  This is in a separate file so that it can be
 *  included in other header files.  This header will also load in the <uuid.h>
 *  include file (so that other files do not need to).
 *
 * Revision History:
 *
 *  V01.000	04-Jul-2018	Jonathan D. Belanger
 *  Initially written.
 */
#ifndef _AXP_VHDX_GUID_H_
#define _AXP_VHDX_GUID_H_

#include "CommonUtilities/AXP_Utility.h"
#include <uuid/uuid.h>

typedef union
{
    uuid_t	uuid;
    struct
    {
  u32	data1;
  u16	data2;
  u16	data3;
  u64	data4;
    };
} AXP_VHDX_GUID;
#define AXP_VHDX_GUID_INIT(name, data_1, data_2, data_3, data_4)	\
    name = (AXP_VHDX_GUID)						\
    {									\
  .data1 = data_1,						\
  .data2 = data_2,						\
  .data3 = data_3,						\
  .data4 = data_4							\
    }


#endif /* _AXP_VHDX_GUID_H_ */
