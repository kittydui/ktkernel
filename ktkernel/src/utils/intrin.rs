use core::arch::asm;

#[inline(always)]
pub fn outb(port: u16, val: u8) {
    unsafe {
        asm!("out dx, al",
            in("dx") port,
            in("al") val,
            options(nomem, nostack, preserves_flags));
    }
}

#[inline(always)]
pub fn inb(port: u16) -> u8 {
    let ret: u8;
    unsafe {
        asm!("in al, dx",
            in("dx") port,
            out("al") ret,
            options(nomem, nostack, preserves_flags));
    }
    ret
}
