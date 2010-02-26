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
	if [ "$USER" = "dh09.lee" ]; then
		CROSS_COMPILER=/usr/local/arm/arm-2008q3/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
}

build_uboot()
{
	make ARCH=arm CROSS_COMPILE="$CCACHE $CROSS_COMPILER" $JOBS $*
}

make_evt_image()
{
	cat onenand_ipl/onenand-ipl-16k-evt0.bin u-boot.bin > u-boot-onenand-evt0.bin
	# To distinguish previous u-boot-onenand.bin, it uses the evt1 suffix
	cp u-boot-onenand.bin u-boot-onenand-evt1.bin
}

check_ccache
check_users

build_uboot $*

make_evt_image

size=`ls -al u-boot-onenand.bin | awk -F' ' '{printf $5}'`
if [ "$size" -ge "262144" ]; then
	echo "u-boot-onenand.bin execced the 256KiB 262144 -> $size"
	exit
fi

if [ "$USER" = "kmpark" ]; then
	ls -al u-boot.bin u-boot-onenand.bin u-boot-onenand-evt0.bin
	# To prevent wrong program
	cp -f u-boot-onenand-evt0.bin u-boot-onenand.bin
	cp -f u-boot.bin u-boot-onenand.bin u-boot-onenand-evt0.bin /tftpboot
	ls -al onenand_ipl
	pushd ../images
	./system.sh
	popd
elif [ "$USER" = "dofmind" ]; then
	tar cvf system_uboot_evt0.tar u-boot-onenand-evt0.bin
	tar cvf system_uboot_evt1.tar u-boot-onenand-evt1.bin
	tar cvf system_uboot.tar u-boot-onenand.bin
	mv -f system_uboot*.tar /home/work
elif [ "$USER" = "prom" ]; then
	tar cvf system_uboot_evt0.tar u-boot-onenand-evt0.bin
	tar cvf system_uboot_evt1.tar u-boot-onenand-evt1.bin
	mv -f system_uboot* /home/share/Work/bin
fi
