#!/bin/bash
#make CROSS_COMPILE=/usr/local/openmoko/arm/bin/arm-angstrom-linux-gnueabi-

#S3C6410 (SmartQ5)
CFLAGS="-marm -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -D__LINUX_ARM_ARCH__=6 -march=armv6k -mtune=arm1176jzf-s -msoft-float -Uarm -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-stack-protector -Wdeclaration-after-statement -Wno-pointer-sign"

#android generic
CFLAGS="-Os -marm -LE -mlittle-endian -Wall -Wundef -march=armv5te -mthumb"

CROSS_COMPILE=/usr/local/software/arm-2010q1-188-arm-gnueabi/bin/arm-none-linux-gnueabi-

export CROSS_COMPILE CFLAGS
make && make install
$CROSS_COMPILE'strip' --strip-debug --strip-unneeded system/bin/busybox
chmod +s system/bin/busybox
cp system/bin/busybox ./busybox

#cp busybox /var/git/smartq-initramfs/initramfs/content/bin/

