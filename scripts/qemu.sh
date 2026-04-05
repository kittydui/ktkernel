qemu-system-x86_64 \
  -m 4096 \
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
  #-s -S
