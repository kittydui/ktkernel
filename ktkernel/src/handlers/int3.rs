use x86_64::structures::idt::InterruptStackFrame;

use crate::log;

pub extern "x86-interrupt" fn int3_handler(stack_frame: InterruptStackFrame) {
    log!("INT3 hit {:#?}", stack_frame);
}