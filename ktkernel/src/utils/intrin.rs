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

#[inline(always)]
pub fn read_cr3() -> u64 {
    let val: u64;
    unsafe {
        asm!("mov {}, cr3", out(reg) val, options(nomem, nostack, preserves_flags));
    }
    val
}

#[inline(always)]
pub fn write_cr3(val: u64) {
    unsafe {
        asm!("mov cr3, {}", in(reg) val, options(nostack, preserves_flags));
    }
}

#[inline(always)]
pub fn invlpg(addr: u64) {
    unsafe {
        asm!("invlpg [{}]", in(reg) addr, options(nostack, preserves_flags));
    }
}
