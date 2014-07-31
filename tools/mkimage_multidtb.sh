#!/bin/bash

INPUT_BIN=${1}
OUTPUT_BIN="u-boot-multi.bin"
OUT="multi.dtb"
OUT_TMP="multi.tmp"

P="./arch/arm/dts"

DTB=(
"exynos4412-trats2.dtb"
"exynos4412-odroid.dtb"
)
DTB_CNT=2

PAD=4

PWD=`pwd`

echo -en "##\n"
echo -en "## PWD: $PWD\n"
echo -en "## Running script: $0\n##\n"

# Check if given binary exists
if [ -s "$INPUT_BIN" ]; then
	echo -en "## Input binary: $INPUT_BIN\n"
	IN_S=`du -b $INPUT_BIN | awk '{print $1}'`
	IN_S_K=$(($IN_S/1024))
	echo -en "## size: $IN_S[B]; $IN_S_K[K]"
else
	echo "## Input binary: $INPUT_BIN: Not Exists!"
	exit
fi

echo -en "\n##"
echo -en "\n## Preparing multi DTB binary: $OUT"
echo -en "\n##"

echo -en "\n## Multi DTB layout:\n## |"

if [ -e $OUT ]; then
	rm $OUT
fi

touch $OUT
CNT=0
for I in ${DTB[*]}; do
	FILE="$P/$I"
	if [ -e $FILE ]; then
		NAME=`echo $I | tail -c16`
		echo -en " *$NAME "
		S=`du -b $FILE | awk '{print $1}'`
		SIZE[$CNT]=$S
		echo -en "$S B"

		cat $OUT $FILE > $OUT_TMP

		if [ -e padding ]; then rm padding; fi
		touch padding

		if [ $CNT -lt $(($DTB_CNT)) ]; then
			PAD_CNT=$(($PAD - $(($SIZE % $PAD))))
			if [ $PAD_CNT -ge 0 ]; then
				echo -en " | PAD: $PAD_CNT B |"
				rm padding
				dd if=/dev/zero of=./padding bs=1 count=$PAD_CNT 2>/dev/zero
#				echo -en "\n \$\$make padding\n"
			else
				echo -en " |"
			fi
		else
			echo -en " |"
		fi

		cat $OUT_TMP padding > $OUT
	else
		echo -en "$I not found.\nexit\n"
		exit
	fi
	CNT=$(($CNT + 1))
done

rm $OUT_TMP
rm padding

echo -en "\n##\n"
S=`du -b $OUT | awk '{print $1}'`
S_K=$(($S/1024))
echo -en "## OUT: $OUT size: $S[B]; $S_K[K]\n"
echo -en "##\n"
echo -en "## Preparing multi platform binary: $OUTPUT_BIN\n"
echo -en "##\n"
echo -en "## BIN Layout:\n"
echo -en "## | $INPUT_BIN $IN_S_K [K] | $OUT $S_K [K] (align: $PAD [B]) |\n"

cat $INPUT_BIN $OUT > $OUTPUT_BIN
S=`du -b $OUTPUT_BIN | awk '{print $1}'`
S_K=$(($S/1024))

echo -en "##\n"
echo -en "## OUT: $OUTPUT_BIN size: $S[B]; $S_K[K]\n"
echo -en "##\n## Multi platform binary: $OUTPUT_BIN. Done\n"
echo -en "##\n"

