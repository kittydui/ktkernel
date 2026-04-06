use spin::Once;
use x86_64::structures::idt::InterruptDescriptorTable;
use x86_64::instructions::interrupts;

use crate::handlers::int3::int3_handler;

static IDT: Once<InterruptDescriptorTable> = Once::new();

pub fn setup_idt() {
    let idt = IDT.call_once(|| {
        let mut idt = InterruptDescriptorTable::new();
        idt.breakpoint.set_handler_fn(int3_handler);
        idt
    });
    
    idt.load();
    interrupts::enable();
}