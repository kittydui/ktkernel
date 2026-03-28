extern "C" [[noreturn]] void ktmain()
{
  while (true) {
    __asm__("hlt");
  }
}
