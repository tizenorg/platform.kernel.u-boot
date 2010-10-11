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
		touch onenand_ipl/board/samsung/universal_c210/start.S
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
	if [ "$USER" = "leedonghwa" ]; then
		CROSS_COMPILER=/opt/toolchains/scratchbox/compilers/arm-linux-gnueabi-gcc4.4.1-glibc2.10.1-2009q3-93/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
	if [ "$USER" = "riverful" ]; then
		CROSS_COMPILER=/opt/arm-2009q3/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
	if [ "$USER" = "cwchoi00" ]; then
		CROSS_COMPILER=/opt/arm-2009q3/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
	if [ "$USER" = "donggeun" ]; then
		CROSS_COMPILER=/scratchbox/compilers/arm-linux-gnueabi-gcc4.4.1-glibc2.10.1-2009q3-93/bin/arm-none-linux-gnueabi-
		JOBS="-j 5"
	fi
	if [ "$USER" = "marek" ]; then
		CROSS_COMPILER=/home/marek/dev//arm-2008q3/bin/arm-none-linux-gnueabi-
		TARGET=${TARGET:-s5pc110}
		make clean clobber unconfig mrproper
		make ${TARGET}_universal_config
		JOBS="-j 2"
	fi
	if [ "$USER" = "lukma" ]; then
	    CROSS_COMPILER=/home/lukma/work/arm-2009q3/bin/arm-none-eabi-
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
	set -x
	SOC=`grep BOARD include/config.mk | awk -F'_' '{print $2}'`
	IPL_PREFIX=${IPL}_ipl/${IPL}-ipl

	if [ "$SOC" = "c210" ]; then
		echo "Use the s-boot instead"
	elif [ "$SOC" = "c110" ]; then
		# C110
		cat ${IPL_PREFIX}-16k-evt0.bin u-boot.bin > u-boot-"$IPL"-evt0.bin
		cat ${IPL_PREFIX}-16k-fused.bin u-boot.bin > u-boot-"$IPL"-evt1-fused.bin
		if [ "$IPL" = "mmc" ]; then
			cat ${IPL_PREFIX}-8k-fused.bin u-boot.bin > u-boot-"$IPL"-evt1-fused.bin
		fi
		# To distinguish previous u-boot-onenand.bin, it uses the evt1 suffix
		cp u-boot-"$IPL".bin u-boot-"$IPL"-evt1.bin
	fi
}

check_ccache
check_users
#check_ipl $1

build_uboot $*

#make_evt_image

if [ "$IPL" != "mmc" -a -e "$PWD/u-boot-onenand.bin" ]; then
	size=`ls -al u-boot-onenand.bin | awk -F' ' '{printf $5}'`
	if [ "$size" -ge "262144" ]; then
		echo "u-boot-onenand.bin execced the 256KiB 262144 -> $size"
		exit
	fi
fi

if [ "$USER" = "kmpark" ]; then
	ls -al u-boot*.bin
	cp -f u-boot.bin u-boot-onenand.bin /tftpboot
	ls -al onenand_ipl
	pushd ../images
	./system.sh
	popd
elif [ "$USER" = "dofmind" ]; then
	tar cvf system_uboot.tar u-boot.bin
	mv -f system_uboot*.tar /home/release
elif [ "$USER" = "prom" ]; then
	tar cvf system_uboot.tar u-boot.bin
	mv -f system_uboot* /home/share/Work/bin
elif [ "$USER" = "jaehoon" ]; then
	tar cvf system_uboot.tar u-boot.bin
	mv -f system_uboot* /home/jaehoon/shared/new/
elif [ "$USER" = "leedonghwa" ]; then
	tar cvf system_uboot.tar u-boot.bin
	mv -f system_uboot* /home/leedonghwa/Build-Binaries/
elif [ "$USER" = "cwchoi00" ]; then
	tar cvf system_uboot.tar u-boot.bin
elif [ "$USER" = "donggeun" ]; then
	tar cvf system_uboot.tar u-boot.bin
	mv -f system_uboot*.tar /home/donggeun/workspace/images
elif [ "$USER" = "marek" ]; then
	BOARD=`grep BOARD include/config.mk | awk -F'= ' '{printf $2}'`
	DATE=`date +%Y%m%d`
	echo $BOARD
	if [ "$BOARD" = "universal_c110" ] ; then
		tar cfv universal-uboot-system-${DATE}.tar u-boot-onenand-evt0.bin
		tar cfv universal-uboot-system-${DATE}-evt1-fused.tar u-boot-onenand-evt1-fused.bin
		tar cfv universal-uboot-system-${DATE}-evt1.tar u-boot-onenand-evt1.bin
		cp universal-uboot-system-${DATE}.tar ../image/w1
		cp universal-uboot-system-${DATE}-evt1-fused.tar ../image/w1
		cp universal-uboot-system-${DATE}-evt1.tar ../image/w1
	fi
	if [ "$BOARD" = "universal_c210" ] ; then
		pushd ../s-boot
		./build.sh
		popd
		mv s-boot-onenand.bin u-boot-onenand.bin
		tar cfv universal-uboot-system-${DATE}-c210-sboot.tar u-boot-onenand.bin
		cp universal-uboot-system-${DATE}-c210-sboot.tar ../image/w1
		tar cfv universal-uboot-system-${DATE}-c210-raw.tar u-boot.bin
		cp universal-uboot-system-${DATE}-c210-raw.tar ../image/w1
	fi
elif [ "$USER" = "lukma" ]; then
	tar cvf system_uboot_evt0.tar u-boot-onenand-evt0.bin
	tar cvf system_uboot_evt1.tar u-boot-onenand-evt1.bin
	tar cvf system_uboot_evt1-fused.tar u-boot-onenand-evt1-fused.bin
	cp system_uboot_evt1-fused.tar ../image/w1/system_uboot_evt1-fused-g`git log --pretty=oneline -1 --abbrev-commit | cut -c 1-7`-`date +%Y%m%d`.tar	
fi
