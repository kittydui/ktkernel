#!/usr/bin/env bash


mkdir -p root/boot/
mkdir -p root/system/modules
mkdir -p root/system/fonts

cp -v target/x86_64-unknown-none/debug/ktkernel root/boot/
cp -v config.toml root/system/
cp -v fonts/Lat2-Terminus16.psfu root/system/fonts/

tar -cvf root/boot/system.tar -C root/system .

mkdir -p root/boot/limine

cp -v misc/limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin root/boot/limine/

mkdir -p root/EFI/BOOT
cp -v limine/BOOTX64.EFI root/EFI/BOOT/
cp -v limine/BOOTIA32.EFI root/EFI/BOOT/

xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
  -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
  -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
  -efi-boot-part --efi-boot-image --protective-msdos-label \
  root -o ktOS.iso

./limine/limine bios-install ktOS.iso

sleep 0.5

clear

qemu-system-x86_64 \
  -m 2048 \
  -machine q35 \
  -bios ./misc/OVMF.fd \
  -cdrom ./ktOS.iso \
  -boot d \
  -no-reboot \
  -no-shutdown \
  -serial stdio \
  -d int \
  -D qemu.log \
  -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
