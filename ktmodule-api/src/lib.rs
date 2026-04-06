#![no_std]
#![feature(custom_test_frameworks)]
#![test_runner(crate::test_runner)]
#![reexport_test_harness_main = "test_main"]

#[cfg(test)]
pub fn test_runner(_tests: &[&dyn Fn()]) {}

unsafe extern "C" {
    pub fn kmalloc(size: usize) -> *mut u8;
}

#[repr(u32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum KtStatus {
    Failed = 0xB0000000,
    Success = 0xB0000001,
}

#[repr(C)]
pub struct KtModule {
    pub dispatch_functions: KtDispatch,
    pub private_data: *mut core::ffi::c_void,
}

#[repr(C)]
pub struct KtDispatch {
    pub shutdown: extern "C" fn (*mut KtModule),
    pub read: extern "C" fn (*mut KtModule, *mut u8, usize) -> KtStatus,
    pub write: extern "C" fn (*mut KtModule, *const u8, usize) -> KtStatus,
    pub close: extern "C" fn (*mut KtModule) -> KtStatus,
}

#[repr(transparent)]
pub struct SyncPtr(pub *const u8);
unsafe impl Sync for SyncPtr {}
unsafe impl Send for SyncPtr {}

pub type KtModuleEntry = extern "C" fn (*mut KtModule) -> KtStatus;