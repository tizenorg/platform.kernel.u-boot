#!/bin/sh

CSA_UBIFS_IMG=csa.ubifs.img
CSA_UBI_IMG=csa.ubi.img
CSA_CFG=csa-cfg.ini
CSA_TAR=csa.tar

build()
{
	mkdir -p csa
	rm -f $CSA_CFG
	echo "[csa-volume]" > $CSA_CFG
	echo "mode=ubi" >> $CSA_CFG
	echo "image=$CSA_UBIFS_IMG" >> $CSA_CFG
	echo "vol_id=5" >> $CSA_CFG
	echo "vol_size=8MiB" >> $CSA_CFG
	echo "vol_type=dynamic" >> $CSA_CFG
	echo "vol_name=csa" >> $CSA_CFG
	echo "vol_flags=autoresize" >> $CSA_CFG
	echo "vol_alignment=1" >> $CSA_CFG
	mkfs.ubifs -d csa -m 4096 -e 256KiB -c 100 -o $CSA_UBIFS_IMG -v
	ubinize -o $CSA_UBI_IMG -p 256KiB -m 4KiB -s 4KiB -v $CSA_CFG

	tar cf $CSA_TAR $CSA_UBI_IMG
}

clean()
{
	rm -rf $CSA_UBIFS_IMG $CSA_UBI_IMG $CSA_CFG $CSA_TAR csa
}

if [ "$1" = "clean" ]; then
	clean
else
	build
fi
