#!/bin/bash
#make CROSS_COMPILE=/usr/local/openmoko/arm/bin/arm-angstrom-linux-gnueabi-

#S3C6410 (SmartQ5)
#CFLAGS="-marm -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -D__LINUX_ARM_ARCH__=6 -march=armv6k -mtune=arm1176jzf-s -msoft-float -Uarm -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-stack-protector -Wdeclaration-after-statement -Wno-pointer-sign"
#CROSS_COMPILE=/usr/local/software/arm-2010q1-188-arm-gnueabi/bin/arm-none-linux-gnueabi-


#android generic
CROSS_COMPILE=/usr/local/software/arm-linux-androideabi-4.4.3/bin/arm-linux-androideabi-

CFLAGS="-Os -DNDEBUG -marm -LE -mlittle-endian -Wall -Wundef -march=armv5te -mthumb "

#regex.h
#CFLAGS+=" -I/froyo/bionic/libc/include/regex.h "
CFLAGS+=" -I'/work/git/froyo/external/clearsilver/util/regex/' "

CFLAGS+=" -D'CONFIG_DEFAULT_MODULES_DIR=\"/system/lib/modules\"' -DANDROID_CHANGES "



LDLIBS="crypto m"
CFLAGS+=" -L./ -L/work/git/froyo/external/clearsilver/perl/blib/arch/auto/ClearSilver/ "

export CROSS_COMPILE CFLAGS LDLIBS

# . busybox_unstripped.out
# exit

#make menuconfig

make && make install
$CROSS_COMPILE'strip' --strip-debug --strip-unneeded system/bin/busybox
chmod +s system/bin/busybox
cp system/bin/busybox ./busybox

