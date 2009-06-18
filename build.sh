#!/bin/sh

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

check_users()
{
	USER=`whoami`
	if [ "$USER" = "kmpark" ]; then
		CROSS_COMPILER=/pub/toolchains/gcc-4.3.2/bin/arm-none-linux-gnueabi-
		JOBS="-j 4"
	fi
}

build_uboot()
{
	make ARCH=arm CROSS_COMPILE="$CCACHE $CROSS_COMPILER" $JOBS $*
}

check_ccache
check_users

build_uboot $*

if [ "$USER" = "kmpark" ]; then
	cp -f u-boot.bin /tftpboot
	pushd ../images
	./system.sh
	popd
fi
