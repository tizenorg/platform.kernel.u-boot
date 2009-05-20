#!/bin/sh -x

# Set default cross compiler
CROSS_COMPILER=/opt/toolchains/arm-2007q3/bin/arm-none-linux-gnueabi-

# Check this system has ccache
check_ccache()
{
	type ccache
	if [ "$?" -eq "0" ]; then
		CCACHE=ccache
	fi
}

build_uboot()
{
	make ARCH=arm CROSS_COMPILE="$CCACHE $CROSS_COMPILER" $JOBS $* -j 4
}

check_ccache

build_uboot $*
