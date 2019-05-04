# DECaxp Unser Guide

Digital Alpha AXP 21264 Emulator (DECaxp)
User Guide
------------------------------------------

This manual is directly derived from the Alpha AXP 21264 Emulator written by
Jonathan D. Belanger.

Revision/Update Inforamtion:				This is a new document.

Jonathan D. Belanger

March 2018

The information in this publication is subject to change without notice.

JONATHAN D. BELANGER SHALL NOT BE LIABLE FOR TECHNICAL OR EDITORIAL
ERRORS OR OMISSIONS CONTAINED HEREIN, NOR FOR INCIDENTAL OR CONSEQUENTIAL
DAMAGES RESULTING FROM THE FURNISHING, PERFORMANCE, OR USE OF THIS MATERIAL.
THIS INFORMATION IS PROVIDED “AS IS” AND JONATHAN D. BELANGER DISCLAIMS ANY
WARRANTIES, EXPRESS, IMPLIED OR STATUTORY AND EXPRESSLY DISCLAIMS THE IMPLIED
WARRANTIES OF MERCHANTABILITY, FITNESS FOR PARTICULAR PURPOSE, GOOD TITLE AND
AGAINST INFRINGEMENT.

This publication contains information protected by copyright. No part of this
publication may be photocopied or reproduced in any form without prior written
consent from Jonathan D. Belanger.

© 2018 Jonathan D. Belanger.
All rights reserved. Printed in the U.S.A.

COMPAQ, the Compaq logo, the Digital logo, and VAX are registered trademarks of
Hewlett Packard Corporation.

Pentium is a registered trademark of Intel Corporation.

Other product names mentioned herein may be trademarks and/or registered
trademarks of their respective companies.

## Overview

The DECaxp emulator is a completely new architecture and implementation for an
emulated CPU.  The initial implementation has the following characteristics:

* Multi-threaded from the start.
* Based on the Compaq Alpha AXP 21264 Hardware Reference Manual, plus the Alpha
Architecture Reference Manaual.
* Contains an instruction cache (Icache), data cache (Dcache), and 2nd level
cache (Bcache).
* Can execute up to 6 instructions simultaneously.
* Executes instructions out of order and speculatively.
* Implements predictive branching.
* Supports byte/word extension (BWX).
* Supports square-root and floating-point convert extension (FIX).
* Support for the multimedia extension (MVI).
* Support for precise arithmetic trap reporting.

## Configuration

The DECaxp emulator supports a number of possible configurations.  The
configuration file is XML formatted.  Below is an example file:

```xml
<?xml version="1.0" encoding="utf-8"?>
<!-- This is the first definition of a DECaxp emulated
    system -->
<DECaxp>
  <!-- The Owner field is for informational purposes only -->
  <Owner>
    <Name>
      <First>Jonathan</First>
      <MI>D.</MI>
      <Last>Belanger</Last>
      <Suffix />
    </Name>
    <CreationDate>02-Jan-2018</CreationDate>
    <ModifyDate>28-Jan-2018</ModifyDate>
  </Owner>
  <!-- This is where the real emulation definitions begin -->
  <System>
    <!-- This is the computer system information -->
    <Model>
      <Name>Compaq ES40</Name>
      <Model>ES40</Model>
    </Model>
    <!--
        This defines where various information is stored
        The InitFile is the file containing the code that
          initializes the CPUs, prior to loading the
          Console
        This PALImage is the file contains the executable
          image that is used to burn in the PALcode
        The ROMImage is the file that is burnt in from
          the PALImage
        The NVRamFile is the file that contains the last
          saved Non-Volitile RAM information
    -->
    <SROM>
      <InitFile>
        ../dat/CPU Initialization Load File.dat
      </InitFile>
      <PALImage>../dat/cl67srmrom.exe</PALImage>
      <ROMImage>
        ../dat/DEC_Alpha_AXP_Code_Compiler.rom
      </ROMImage>
      <NVRamFile>../dat/DECaxp 21264.nvr</NVRamFile>
      <CboxCSRFile>
        ../dat/AXP_21264_Cbox_CSR.nvp
      </CboxCSRFile>
    </SROM>
    <!--
        This defines the actual CPUs.
        The number of CPUs that can be defined is
          determined by the System/Model information
        The Generation contains what version of the
          Digitial Alpha AXP CPU we are emulating
        The Pass contains the manufacturing pass for the
          generation of the CPU
    -->
    <CPUs>
      <Count>1</Count>
      <Generation>EV68CB</Generation>
      <Pass>5</Pass>
    </CPUs>
    <!--
      This defines the individual memory modules and their
      size.  In reality the sizes are summed for total
      memory size.  The individual DIMMs are simulated.
    -->
    <DIMMs>
      <Count>4</Count>
      <Size>4.0GB</Size>
    </DIMMs>
    <!--
      This is where this the disk files are defined.  The
      type determines whether the device is read-only or
      read-write.
    -->
    <Disks>
      <Disk number="1">
        <Type>Disk</Type>
        <Name>RZ02</Name>
        <Size>100.0GB</Size>
        <File>../dat/RZ02-1.dsk</File>
      </Disk>
      <Disk number="2">
        <Type>CDROM</Type>
        <Name>RZCD02</Name>
        <Size>100.0GB</Size>
        <File>../dat/RZCD02-1.dsk</File>
      </Disk>
    </Disks>
    <!--
      The console definition contains the information
      required to be able to have a telnet terminal
      connect to the emulator as the console.
    -->
    <Console>
      <Port>780</Port>
    </Console>
    <!--
      The network definition contains the adapter name and
      the MAC address to be assigned to it.
    -->
    <Networks>
      <Network number="1">
        <Name>es04</Name>
        <MAC>08-00-20-01-23-45</MAC>
      </Network>
    </Networks>
    <!--
      This is a place holder for Printer definitions.
    -->
    <Printers>
      <Printer number="1" />
    </Printers>
    <!--
      This is a place holder for Tape definitions.
    -->
    <Tapes>
      <Tape number="1" />
    </Tapes>
  </System>
</DECaxp>
```

## Trace Logging

For some people, the way trace logging is implemented will look familiar.  It
it is based on similar functionality found in the DEC SNA Product Set.  There
are two environment variables that control the logging.

### AXP_LOGMASK

This is first environment variable.  If it is not defined or is defined to all
zeros (if there is an error in converting the environment variable, 0 is
assumed).  This variable has the following format/definition:

  `export AXP_LOGMASK=0xffffffff'

This variable is defined as a hexidecimal value, where each bit represents some
level of logging to be performed.  The following set of bits are used:

* 0x0000000f is for the Common Utilties layer.
* 0x0000fff0 is for the CPU layer.
* 0x0fff0000 is for the System layer.
* 0xf0000000 is reserved.

Within each set of bits, the individual bits represent the following (note:
some tracing bits are not used in all layers):

The following are supported by all layers (for layers that utilize more than one
nibble, it is assumed that the higher bits are clear for the purposes of these
definitions):

* b'0001' is for tracing function calls and returns.
* b'0010' is for tracing buffer data.  This includes various configuration
data.
* b'0100' is for option 1 tracing.  This is information includes items not
included in the fist two bits, and is one level of detail deeper.
* b'1000' is for option 2 tracing.  This information can be very details and
has the potential to slow the emulation down significantly.

The following are supported by the CPU layer:

* b'0000 0100 0000' is for the caches (Icache, Dcache, and Bcache).
* b'0000 1000 0000' is for the Instruction decoder and scheduler (Ibox).
* b'0001 0000 0000' is for the Integer Pipelines (Ebox - 4 clusters, L0, L1,
U0, and U1).
* b'0002 0000 0000' is for the Floating-Point Pipelines (Fbox - 2 clusters,
Multiply, Other),
* b'0004 0000 0000' is for the Memory Manager (Mbox).
* b'0008 0000 0000' is for the System Interface (Cbox).


The following are supported by the Ibox, as the Cbox:

* b'0000 0001 0000' is for instructions to be traced after they have been
retired or loaded into the Icache.  For the Ibox, ihe contents of the
associated registers are display with their updated value.  The Cbox does not
display the registers.

### AXP_LOGFILE

This is the other environment variable.  This variable only has meaning when
the above environment variable is defined to a value that, when translated to
a number, is not zero.  If `AXP_LOGMASK` is defined and this variable is
not, then the emulator will write to standard output.  This variable represent
a filename, including path, where the log information is to be written.  It has
the following format:

  `export AXP_LOGFILE=log/tracefile.txt'

