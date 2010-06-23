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
		#CROSS_COMPILER=/pub/toolchains/gcc-4.4.1/bin/arm-none-linux-gnueabi-
		CROSS_COMPILER=/scratchbox/compilers/arm-linux-gnueabi-gcc4.4.1-glibc2.10.1-2009q3-93/bin/arm-none-linux-gnueabi-
		JOBS="-j 4"
	fi
	if [ "$USER" = "dofmind" ]; then
		CROSS_COMPILER=arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
	if [ "$USER" = "prom" ]; then
		CROSS_COMPILER=/opt/toolchains/arm-2009q3/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
	if [ "$USER" = "jaehoon" ]; then
		CROSS_COMPILER=/usr/local/arm/arm-2009q3/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
	if [ "$USER" = "dh09.lee" ]; then
		CROSS_COMPILER=/usr/local/arm/arm-2008q3/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
	if [ "$USER" = "riverful" ]; then
		CROSS_COMPILER=/opt/arm-2009q3/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
}

check_ipl()
{
	if [ "$1" = "mmc" ]; then
		IPL="mmc"
	else
		IPL="onenand"
	fi
}

build_uboot()
{
	if [ "$1" != "mmc" ]; then
		OPT=$*
	fi
	make ARCH=arm CROSS_COMPILE="$CCACHE $CROSS_COMPILER" $JOBS $OPT
}

make_evt_image()
{
	cat "$IPL"_ipl/"$IPL"-ipl-16k-evt0.bin u-boot.bin > u-boot-"$IPL"-evt0.bin
	cat "$IPL"_ipl/"$IPL"-ipl-16k-fused.bin u-boot.bin > u-boot-"$IPL"-evt1-fused.bin
	# To distinguish previous u-boot-onenand.bin, it uses the evt1 suffix
	cp u-boot-"$IPL".bin u-boot-"$IPL"-evt1.bin
}

make_recovery_image()
{
	if [ "$IPL" != "mmc" ]; then
		cat recovery/recovery-evt0.bin u-boot.bin > u-boot-recovery-evt0.bin
		cat recovery/recovery-fused.bin u-boot.bin > u-boot-recovery-evt1-fused.bin
		cp u-boot-recovery.bin u-boot-recovery-evt1.bin
	fi
}

check_ccache
check_users
check_ipl $1

build_uboot $*

make_evt_image
make_recovery_image

if [ "$IPL" != "mmc" ]; then
	size=`ls -al u-boot-onenand.bin | awk -F' ' '{printf $5}'`
	if [ "$size" -ge "262144" ]; then
		echo "u-boot-onenand.bin execced the 256KiB 262144 -> $size"
		exit
	fi
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
	tar cvf system_uboot_evt1-fused.tar u-boot-onenand-evt1-fused.bin
	tar cvf system_uboot.tar u-boot-onenand.bin
	mv -f system_uboot*.tar /home/release
elif [ "$USER" = "prom" ]; then
	tar cvf system_uboot_evt0.tar u-boot-onenand-evt0.bin
	tar cvf system_uboot_evt1.tar u-boot-onenand-evt1.bin
	tar cvf system_uboot_evt1-fused.tar u-boot-onenand-evt1-fused.bin
	tar cvf system_uboot_recovery_evt0.tar u-boot-recovery-evt0.bin
	tar cvf system_uboot_recovery_evt1.tar u-boot-recovery-evt1.bin
	tar cvf system_uboot_recovery_evt1-fused.tar u-boot-recovery-evt1-fused.bin
	mv -f system_uboot* /home/share/Work/bin
elif [ "$USER" = "jaehoon" ]; then
	tar cvf system_uboot_evt0.tar u-boot-onenand-evt0.bin
	tar cvf system_uboot_evt1.tar u-boot-onenand-evt1.bin
	tar cvf system_uboot_evt1-fused.tar u-boot-onenand-evt1-fused.bin
	tar cvf system_uboot_recovery_evt0.tar u-boot-recovery-evt0.bin
	tar cvf system_uboot_recovery_evt1.tar u-boot-recovery-evt1.bin
	tar cvf system_uboot_recovery_evt1-fused.tar u-boot-recovery-evt1-fused.bin
	mv -f system_uboot* /home/jaehoon/shared/new/
fi
