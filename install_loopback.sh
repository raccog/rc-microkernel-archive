#!/bin/bash

if [[ -z $1 ]]; then
    echo "Usage: install_loopback.sh <INSTALL_DISK> <KERNEL_IMAGE>"
    exit 1
fi

MOUNTDIR=.cache/img_mount
LIMINEDIR=.cache/limine
INSTALL_DISK=$1
KERNEL_IMAGE=$2

KERNEL_IMAGE_FINAL=rc-microkernel.elf

dd if=/dev/zero bs=1M count=0 seek=64 of=${INSTALL_DISK}
parted -s ${INSTALL_DISK} mklabel gpt
parted -s ${INSTALL_DISK} mkpart ESP fat32 2048s 100%
parted -s ${INSTALL_DISK} set 1 esp on
${LIMINEDIR}/limine-deploy ${INSTALL_DISK}
USED_LOOPBACK=$(sudo losetup -Pf --show ${INSTALL_DISK})
sudo mkfs.fat -F 32 ${USED_LOOPBACK}p1
mkdir -p ${MOUNTDIR}
sudo mount ${USED_LOOPBACK}p1 ${MOUNTDIR}
sudo mkdir -p ${MOUNTDIR}/EFI/BOOT
sudo cp -v limine.cfg ${LIMINEDIR}/limine.sys $MOUNTDIR
sudo cp -v ${KERNEL_IMAGE} ${MOUNTDIR}/$KERNEL_IMAGE_FINAL
sudo cp -v ${LIMINEDIR}/BOOTX64.EFI ${MOUNTDIR}/EFI/BOOT
sync
sudo umount ${MOUNTDIR}
sudo losetup -d $USED_LOOPBACK
