KERNELDIR=${KERNELDIR:-../../kernel/linux-arm}
CROSS_COMPILE=${CROSS_COMPILE:-/home/mszyprow/dev/cross/gcc-linaro-4.9-2014.11-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-}

make CROSS_COMPILE=${CROSS_COMPILE} clean mrproper samsung_draco_config all

mkdir -p output

# prepare combined kernel+dtb image:

cat <<-EOF >$KERNELDIR/kernel.its
/dts-v1/;
/ {
	description = "Combined kernel+dtb image";
	#address-cells = <1>;
	images {
		kernel@0 {
			description = "Linux kernel";
			data = /incbin/("arch/arm64/boot/Image.gz");
			type = "kernel";
			arch = "arm64";
			os = "linux";
			compression = "gzip";
			load = <0x20080000>;
			entry = <0x20080000>;
			hash@1 {
				algo = "crc32";
			};
		};
		fdt@0 {
			description = "exynos5433-draco.dtb";
			data = /incbin/("arch/arm64/boot/dts/exynos/exynos5433-draco.dtb");
			type = "flat_dt";
			arch = "arm64";
			compression = "none";
			load = <0x24000000>;
			hash@1 {
				algo = "crc32";
			};
		};
		fdt@1 {
			description = "exynos5433-dracor.dtb";
			data = /incbin/("arch/arm64/boot/dts/exynos/exynos5433-dracor.dtb");
			type = "flat_dt";
			arch = "arm64";
			compression = "none";
			load = <0x24000000>;
			hash@1 {
				algo = "crc32";
			};
		};
	};
	configurations {
		default = "draco@0";
		draco@0 {
			description = "Linux kernel with exynos5433-draco.dtb";
			kernel = "kernel@0";
			fdt = "fdt@0";
		};
		dracor@1 {
			description = "Linux kernel with exynos5433-dracor.dtb";
			kernel = "kernel@0";
			fdt = "fdt@1";
		};
	};
};
EOF

tools/mkimage -f $KERNELDIR/kernel.its output/kernel.img || exit 1
rm $KERNELDIR/kernel.its

# make Android-compatible uboot.img from u-boot.bin+download.bgra image (with empty ramdisk to make tools happy)
dd if=/dev/zero of=output/ramdisk.img bs=4k count=1
dd if=/dev/zero of=output/uboot.bin bs=1024 count=320
dd if=u-boot.bin of=output/uboot.bin conv=notrunc || exit 1
dd if=images/download.bgra.gz of=output/uboot.bin bs=1024 conv=notrunc seek=320 || exit 1
$KERNELDIR/tools/mkbootimg --kernel output/uboot.bin --ramdisk output/ramdisk.img --output output/uboot.img || exit 1

# prepare mixed boot partition image:
dd if=/dev/zero of=output/boot.img bs=1k count=1k
# uboot image at offset 0
dd if=output/uboot.img of=output/boot.img conv=notrunc || exit 1
# kernel FIT image at offset 512KiB
dd if=output/kernel.img of=output/boot.img bs=1024 conv=notrunc seek=512 || exit 1

# use only uboot.img (without kernel) as image for recovery partition
cp output/uboot.img output/recovery.img

tar cf output/system-boot-arm64.tar --strip-components=1 -C output boot.img recovery.img
tar cf output/system-kernel-arm64.tar --strip-components=1 -C output kernel.img
echo
echo Created image:
ls -l output/system-*.tar
