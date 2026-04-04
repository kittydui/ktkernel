#![no_std]
#![no_main]

use core::{panic::PanicInfo, arch::asm};

pub mod crt;
pub mod limine;
pub mod gdt;
pub mod idt;
pub mod handlers;
pub mod memory;
pub mod utils;
pub mod module;

use utils::serial::{COM1_PORT, Serial};

#[unsafe(no_mangle)]
pub unsafe extern "C" fn kt_main() {
    Serial::new(COM1_PORT).unwrap();

    log!("ktkernel initialized");

    asm!("hlt");
}

#[panic_handler]
fn panic_handler(_: &PanicInfo) -> ! { loop {} }
