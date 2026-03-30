qemu-system-x86_64 \
  -m 4096 \
  -machine q35 \
  -bios ./OVMF.fd \
  -cdrom ./ktOS.iso \
  -boot d \
  -no-reboot \
  -no-shutdown \
  -d int \
  -D qemu.log
