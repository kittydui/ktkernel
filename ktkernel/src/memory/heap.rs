use core::alloc::{GlobalAlloc, Layout};

use spin::Mutex;

use super::pmm::PMM;
use crate::limine::HHDM;

const PAGE_SIZE: usize = 4096;
const SLAB_MAGIC: u64 = 0x534C4142;
const SIZE_CLASSES: [usize; 6] = [32, 64, 128, 256, 512, 1024];

#[repr(C)]
struct SlabHeader {
    magic: u64,
    free_list: *mut u8,
    next: *mut SlabHeader,
    object_size: usize,
    used_count: usize,
    total_count: usize,
}

#[repr(C)]
struct LargeHeader {
    pages: usize,
    _padding: usize,
}

struct Cache {
    slabs: *mut SlabHeader,
    object_size: usize,
}

pub struct SlabAllocator {
    caches: [Cache; 6],
    hhdm_offset: u64,
}

unsafe impl Send for SlabAllocator {}

impl SlabAllocator {
    const fn new() -> Self {
        Self {
            caches: [
                Cache { slabs: core::ptr::null_mut(), object_size: 32 },
                Cache { slabs: core::ptr::null_mut(), object_size: 64 },
                Cache { slabs: core::ptr::null_mut(), object_size: 128 },
                Cache { slabs: core::ptr::null_mut(), object_size: 256 },
                Cache { slabs: core::ptr::null_mut(), object_size: 512 },
                Cache { slabs: core::ptr::null_mut(), object_size: 1024 },
            ],
            hhdm_offset: 0,
        }
    }

    pub fn initialize(&mut self) {
        let hhdm = HHDM.response().expect("HHDM response missing");
        self.hhdm_offset = hhdm.offset;

        for (i, &size) in SIZE_CLASSES.iter().enumerate() {
            self.caches[i].object_size = size;
            self.caches[i].slabs = core::ptr::null_mut();
        }
    }

    fn find_class(size: usize) -> Option<usize> {
        SIZE_CLASSES.iter().position(|&s| size <= s)
    }

    fn create_slab(&self, size: usize) -> *mut SlabHeader {
        let frame = match PMM.lock().allocate_frame() {
            Some(f) => f,
            None => return core::ptr::null_mut(),
        };

        let slab = (frame + self.hhdm_offset) as *mut SlabHeader;
        unsafe {
            (*slab).magic = SLAB_MAGIC;
            (*slab).object_size = size;
            (*slab).used_count = 0;
            (*slab).next = core::ptr::null_mut();
            (*slab).free_list = core::ptr::null_mut();

            let data_start = slab as u64 + core::mem::size_of::<SlabHeader>() as u64;
            let available = PAGE_SIZE - core::mem::size_of::<SlabHeader>();
            (*slab).total_count = available / size;

            for i in 0..(*slab).total_count {
                let object = (data_start + (i * size) as u64) as *mut *mut u8;
                *object = (*slab).free_list;
                (*slab).free_list = object as *mut u8;
            }
        }

        slab
    }

    fn allocate_from_slab(&mut self, cache_idx: usize) -> *mut u8 {
        let mut slab = self.caches[cache_idx].slabs;

        unsafe {
            while !slab.is_null() && (*slab).free_list.is_null() {
                slab = (*slab).next;
            }

            if slab.is_null() {
                slab = self.create_slab(self.caches[cache_idx].object_size);
                if slab.is_null() {
                    return core::ptr::null_mut();
                }
                (*slab).next = self.caches[cache_idx].slabs;
                self.caches[cache_idx].slabs = slab;
            }

            let object = (*slab).free_list;
            (*slab).free_list = *(object as *const *mut u8);
            (*slab).used_count += 1;
            object
        }
    }

    fn allocate_large(&self, size: usize) -> *mut u8 {
        let total = size + core::mem::size_of::<LargeHeader>();
        let pages = (total + PAGE_SIZE - 1) / PAGE_SIZE;

        let frame = match PMM.lock().allocate_contiguous(pages) {
            Some(f) => f,
            None => return core::ptr::null_mut(),
        };

        let header = (frame + self.hhdm_offset) as *mut LargeHeader;
        unsafe {
            (*header).pages = pages;
            header.add(1) as *mut u8
        }
    }

    pub fn allocate(&mut self, size: usize) -> *mut u8 {
        if size == 0 {
            return core::ptr::null_mut();
        }

        match Self::find_class(size) {
            Some(idx) => self.allocate_from_slab(idx),
            None => self.allocate_large(size),
        }
    }

    pub fn free(&mut self, ptr: *mut u8) {
        if ptr.is_null() {
            return;
        }

        let page = (ptr as u64) & !(PAGE_SIZE as u64 - 1);
        let slab = page as *mut SlabHeader;

        unsafe {
            if (*slab).magic == SLAB_MAGIC {
                *(ptr as *mut *mut u8) = (*slab).free_list;
                (*slab).free_list = ptr;
                (*slab).used_count -= 1;
                return;
            }

            let header = (ptr as *mut LargeHeader).sub(1);
            let phys = header as u64 - self.hhdm_offset;
            PMM.lock().free_contiguous(phys, (*header).pages);
        }
    }
}

pub static SLAB: Mutex<SlabAllocator> = Mutex::new(SlabAllocator::new());

struct KtAllocator;

unsafe impl GlobalAlloc for KtAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let size = layout.size().max(layout.align());
        SLAB.lock().allocate(size)
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        SLAB.lock().free(ptr)
    }
}

#[global_allocator]
static GLOBAL_ALLOCATOR: KtAllocator = KtAllocator;
