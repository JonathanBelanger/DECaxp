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
 *	This header file contains the initialization routines for the System
 *	components.  We do this here to avoid includes that refer to each other
 *	and cause compile time errors.
 *
 * Revision History:
 *
 *	V01.000		02-Jun-2018	Jonathan D. Belanger
 *	Initially written.
 */
#ifndef _AXP_21274_INITRTNS_H_
#define _AXP_21274_INITRTNS_H_

/*
 * Cchip Initialization Function Prototype
 */
void AXP_21274_CchipInit(AXP_21274_SYSTEM *);

/*
 * Pchip Initialization Function Prototype
 */
void AXP_21274_PchipInit(AXP_21274_PCHIP *, u32 id);

/*
 * Dchip Initialization Function Prototype
 */
void AXP_21274_DchipInit(AXP_21274_SYSTEM *);


#endif /* _AXP_21274_INITRTNS_H_ */
