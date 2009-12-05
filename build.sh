#!/bin/sh

# Set default cross compiler
CROSS_COMPILER=/opt/toolchains/arm-2008q3/bin/arm-none-linux-gnueabi-

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
	if [ "$USER" = "dofmind" ]; then
		CROSS_COMPILER=/opt/toolchains/arm-2008q3/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
	if [ "$USER" = "prom" ]; then
		CROSS_COMPILER=/opt/toolchains/arm-2008q3/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
	if [ "$USER" = "jaehoon" ]; then
		CROSS_COMPILER=/usr/local/arm/arm-2008q3/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
}

build_uboot()
{
	make ARCH=arm CROSS_COMPILE="$CCACHE $CROSS_COMPILER" $JOBS $*
}

check_ccache
check_users

build_uboot $*

size=`ls -al u-boot-onenand.bin | awk -F' ' '{printf $5}'`
if [ "$size" -eq "262144" ]; then
	echo "u-boot-onenand.bin execced the 256KiB -> $size"
	exit
fi

if [ "$USER" = "kmpark" ]; then
	cp -f u-boot.bin u-boot-onenand.bin /tftpboot
	ls -al u-boot.bin u-boot-onenand.bin
	ls -al onenand_ipl
	pushd ../images
	./system.sh
	popd
elif [ "$USER" = "dofmind" ]; then
	tar cvf system_uboot.tar u-boot-onenand.bin
	mv -f system_uboot.tar /home/work
elif [ "$USER" = "prom" ]; then
	tar cvf system_uboot.tar u-boot-onenand.bin
fi
