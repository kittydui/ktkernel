#![no_std]
#![no_main]
#![feature(abi_x86_interrupt)]
#![feature(custom_test_frameworks)]
#![test_runner(crate::test_runner::test_runner)]
#![reexport_test_harness_main = "test_main"]

use ktmacros::export_symbol;

extern crate alloc;

pub mod test_runner;
pub mod crt;
pub mod limine;
pub mod gdt;
pub mod idt;
pub mod handlers;
pub mod memory;
pub mod utils;
pub mod module;

use utils::serial::{COM1_PORT, Serial};
use core::{panic::PanicInfo, arch::asm};

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kt_main() {
    Serial::new(COM1_PORT).unwrap();
    memory::init();
    gdt::setup_gdt();
    idt::setup_idt();

    #[cfg(test)]
    test_main();


    log!("ktkernel initialized");

    unsafe {
        asm!("int 3");
        asm!("hlt");
    }
}

#[panic_handler]
fn panic_handler(panic: &PanicInfo) -> ! {
    log!("{:#?}", panic);
    loop {}
}
