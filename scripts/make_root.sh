git clone https://codeberg.org/Limine/Limine.git limine --branch=v10.x-binary --depth=1
make -C limine

mkdir -p root/boot/
cp -v build/ktkernel root/boot/

mkdir -p root/system/modules
mkdir -p root/system/fonts

cp -v config.toml root/system/
cp -v fonts/Lat2-Terminus16.psfu root/system/fonts/
cp -v build/modules/cmos.ktdrv root/system/modules/

tar -cvf root/boot/system.tar -C root/system .

mkdir -p root/boot/limine
cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin root/boot/limine/

mkdir -p root/EFI/BOOT
cp -v limine/BOOTX64.EFI root/EFI/BOOT/
cp -v limine/BOOTIA32.EFI root/EFI/BOOT/

xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
  -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
  -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
  -efi-boot-part --efi-boot-image --protective-msdos-label \
  root -o ktOS.iso

./limine/limine bios-install ktOS.iso
