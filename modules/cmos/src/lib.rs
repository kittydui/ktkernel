#![no_std]
#![no_main]
#![feature(custom_test_frameworks)]
#![test_runner(crate::test_runner)]
#![reexport_test_harness_main = "test_main"]

#[cfg(test)]
pub fn test_runner(_tests: &[&dyn Fn()]) {}

use ktmacros::kmodule_entry;
use ktmodule_api::{KtModule, KtModuleEntry, KtStatus, SyncPtr};

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

#[kmodule_entry("cmos_driver")]
pub extern "C" fn cmos_driver_init(_module: *mut KtModule) -> KtStatus {
    KtStatus::Success
}