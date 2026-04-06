#[repr(transparent)]
pub struct SyncPtr<T>(pub *const T);

unsafe impl<T> Sync for SyncPtr<T> {}
unsafe impl<T> Send for SyncPtr<T> {}


#[repr(C)]
pub struct KernelSymbol {
    pub value: SyncPtr<()>,
    pub name: SyncPtr<u8>,
}