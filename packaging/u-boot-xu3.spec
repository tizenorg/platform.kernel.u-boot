Name: u-boot-xu3
Version: 2015.04
Release: 0
Summary: Das U-Boot - Tizen bootloader
Group: System/Kernel
License: GPL-2.0+
ExclusiveArch: %{arm}
URL: https://review.tizen.org/git/?p=kernel/u-boot.git
Source0: u-boot-%{version}.tar.bz2
Source1001: u_boot_xu3.manifest

BuildRequires: gcc >= 4.8
BuildRequires: flex
BuildRequires: bison

%description
u-boot - Tizen bootloader for Embedded boards based on ARM processor

#TODO: Describe rpm package information depending on board
%package -n u-boot-pkg
Summary: A bootloader for Embedded system
Group: System/Kernel

%description -n u-boot-pkg
A boot loader for embedded systems.
Das U-Boot is a cross-platform bootloader for embedded systems,
used as the default boot loader by several board vendors.  It is
intended to be easy to port and to debug, and runs on many
supported architectures, including PPC, ARM, MIPS, x86, m68k, NIOS,
and Microblaze.

%package -n u-boot-tools
Summary: Companion tools for Das U-Boot bootloader
Group: System/Kernel
Provides: dtc

%description -n u-boot-tools
This package includes the mkimage program, which allows generation of U-Boot
images in various formats, and the fw_printenv and fw_setenv programs to read
and modify U-Boot's environment.

%ifarch %{arm}
%global use_mmc_storage 1
%endif

%prep
%setup -q -n u-boot-%{version}

%build
cp %{SOURCE1001} .

CONFIG=odroid-xu3_defconfig

make mrproper

# Build dtc
make HOSTCC="gcc $RPM_OPT_FLAGS" -C tools/dtc

# Set configuration
make $CONFIG

# Build tools
make %{?_smp_mflags} HOSTCC="gcc $RPM_OPT_FLAGS" HOSTSTRIP=/bin/true tools

%if 1%{?use_mmc_storage}
make HOSTCC="gcc $RPM_OPT_FLAGS" CONFIG_ENV_IS_IN_MMC=y env
%else
make HOSTCC="gcc $RPM_OPT_FLAGS" env
%endif

# Build u-boot
export PATH="$PATH:tools:tools/dtc/"
make %{?_smp_mflags} EXTRAVERSION=`echo %{vcs} | sed 's/.*u-boot.*#\(.\{9\}\).*/-g\1-TIZEN.org/'`

# Sign u-boot-multi.bin - output is: u-boot-mmc.bin
chmod 755 tools/mkimage_signed.sh
mkimage_signed.sh u-boot-dtb.bin $CONFIG

# Generate params.bin
cp `find . -name "env_common.o"` copy_env_common.o
objcopy -O binary --only-section=.rodata.default_environment `find . -name "copy_env_common.o"`
tr '\0' '\n' < copy_env_common.o > default_envs.txt
mkenvimage -s 16384 -o params.bin default_envs.txt
rm copy_env_common.o default_envs.txt

%install
rm -rf %{buildroot}

# Tools installation
mkdir -p %{buildroot}%{_bindir}
install -p -m 0755 tools/mkimage %{buildroot}%{_bindir}
install -p -m 0755 tools/env/fw_printenv %{buildroot}%{_bindir}
install -p -m 0755 tools/dtc/dtc %{buildroot}%{_bindir}
( cd %{buildroot}%{_bindir}; ln -sf fw_printenv fw_setenv )

# u-boot installation
mkdir -p %{buildroot}/var/tmp/u-boot-xu3
install -d %{buildroot}/var/tmp/u-boot-xu3
install -m 755 u-boot.bin %{buildroot}/var/tmp/u-boot-xu3
install -m 755 u-boot-mmc.bin %{buildroot}/var/tmp/u-boot-xu3
install -m 755 params.bin %{buildroot}/var/tmp/u-boot-xu3

%clean

%files
%manifest u_boot_xu3.manifest
%defattr(-,root,root,-)
/var/tmp/u-boot-xu3

%files -n u-boot-tools
%manifest u_boot_xu3.manifest
%defattr(-,root,root,-)
%{_bindir}/mkimage
%{_bindir}/fw_printenv
%{_bindir}/fw_setenv
%{_bindir}/dtc
