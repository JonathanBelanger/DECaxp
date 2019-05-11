#! /usr/bin/bash
#
# Copyright (C) Jonathan D. Belanger 2019.
# All Rights Reserved.
#
# This software is furnished under a license and may be used and copied only
# in accordance with the terms of such license and with the inclusion of the
# above copyright notice.  This software or any other copies thereof may not
# be provided or otherwise made available to any other person.  No title to
# and ownership of the software is hereby transferred.
#
# The information in this software is subject to change without notice and
# should not be construed as a commitment by the author or co-authors.
#
# The author and any co-authors assume no responsibility for the use or
# reliability of this software.
#
# Description:
#
#  This script file contains the steps needed to compile DECaxp
#
# Revision History:
#
#  V01.000 04-May-2019 Jonathan D. Belanger
#  Initially written.
#

#
# Before we go too far, save some information we will want later.
#
CUR_DIR=$PWD

#
# Get the directory where this build script resides.
#
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
    DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"

#
# Change our directory to the location of the build script.
#
cd $DIR

#
# Set the default for CMAKE_BUILD_TYPE
#
BLD_TYPE="Release"

#
# Usage function
#
usage()
{
    echo ""
    echo "Usage: $(basename -- $0) [options]"
    echo ""
    echo "Options:"
    echo " -h, --help     display this help"
    echo " -d, --debug    set CMAKE_BUILD_TYPE=Debug (default = Release)"
    echo " -c, --clean    run make clean followed by make"
    echo " -r, --reset    delete the build directory and run make clean followed by make"
    echo ""
    echo "If the build directory exists, and --reset is not specified then only make will be run"
    echo ""
    exit 0
}

#
# Let's define some options that can be used on the command-line
#
OPTS=$(getopt -o hdrc --longoptions help,debug,clean,reset -n "$(basename "$0")" -- "$@")
eval set --$OPTS
while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)
            usage
            ;;
        -d|--debug)
            BLD_TYPE="Debug"
            shift
            ;;
        -r|--reset)
            [[ -d build ]] && rm -rf build/
            shift
            ;;
        -c|--clean)
            [[ -d build ]] && cd build/ && make clean && cd ..
            shift
            ;;
        --)
            shift
            ;;
        *)
            echo "Invalid option, $1"
            usage
            ;;

    esac
done

#
# OK, if the build directory does not exist, then create it, cd into it,a nd run CMake
# Otherwise, just cd into the build directory.
#
if [[ ! -d build ]]; then
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=$BLD_TYPE ..
else
    cd build
fi

#
# At the very least, we need to run make from within the build directory.
#
make

#
# We are all done, change directory back to where we came from and exit out normally.
#
cd $CUR_DIR
exit 0
