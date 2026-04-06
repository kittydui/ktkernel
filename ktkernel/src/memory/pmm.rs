use spin::Mutex;

use crate::limine::{HHDM, MEMMAP};
use limine::memmap::MEMMAP_USABLE;

const MAX_ORDER: usize = 25;
const MIN_ORDER: usize = 12;
const ORDERS: usize = MAX_ORDER - MIN_ORDER + 1;
const MIN_BLOCK_SIZE: u64 = 1 << MIN_ORDER;
const PAGE_SIZE: u64 = 4096;

#[repr(C)]
struct FreeBlock {
    next: *mut FreeBlock,
}

struct BuddyAllocator {
    free_lists: [*mut FreeBlock; ORDERS],
    hhdm_offset: u64,
}

unsafe impl Send for BuddyAllocator {}

impl BuddyAllocator {
    const fn new() -> Self {
        Self {
            free_lists: [core::ptr::null_mut(); ORDERS],
            hhdm_offset: 0,
        }
    }

    fn get_buddy(&self, address: u64, order: usize) -> u64 {
        address ^ (1u64 << (order + MIN_ORDER))
    }

    fn add_region(&mut self, mut base: u64, mut size: u64) {
        base = (base + MIN_BLOCK_SIZE - 1) & !(MIN_BLOCK_SIZE - 1);
        size &= !(MIN_BLOCK_SIZE - 1);

        while size >= MIN_BLOCK_SIZE {
            let mut order = ORDERS - 1;
            while order > 0 && (1u64 << (order + MIN_ORDER)) > size {
                order -= 1;
            }
            while order > 0 && (base & ((1u64 << (order + MIN_ORDER)) - 1)) != 0 {
                order -= 1;
            }

            let block_size = 1u64 << (order + MIN_ORDER);
            let block = (base + self.hhdm_offset) as *mut FreeBlock;
            unsafe {
                (*block).next = self.free_lists[order];
            }
            self.free_lists[order] = block;

            base += block_size;
            size -= block_size;
        }
    }

    fn split(&mut self, order: usize) {
        let block = self.free_lists[order];
        unsafe {
            self.free_lists[order] = (*block).next;

            let half_size = 1u64 << ((order - 1) + MIN_ORDER);
            let second = (block as u64 + half_size) as *mut FreeBlock;
            (*second).next = self.free_lists[order - 1];
            (*block).next = second;
            self.free_lists[order - 1] = block;
        }
    }

    fn allocate_pages(&mut self, order: usize) -> u64 {
        let mut current = order;
        while current < ORDERS && self.free_lists[current].is_null() {
            current += 1;
        }

        if current >= ORDERS {
            return 0;
        }

        while current > order {
            self.split(current);
            current -= 1;
        }

        let block = self.free_lists[order];
        unsafe {
            self.free_lists[order] = (*block).next;
        }

        block as u64 - self.hhdm_offset
    }

    fn coalesce(&mut self, mut addr: u64, mut order: usize) {
        while order < ORDERS - 1 {
            let buddy_physical = self.get_buddy(addr, order);
            let buddy_virtual = (buddy_physical + self.hhdm_offset) as *mut FreeBlock;

            let mut prev = &mut self.free_lists[order] as *mut *mut FreeBlock;
            let mut curr = self.free_lists[order];
            let mut found = false;

            while !curr.is_null() {
                if curr == buddy_virtual {
                    unsafe { *prev = (*curr).next };
                    found = true;
                    break;
                }
                unsafe {
                    prev = &mut (*curr).next;
                    curr = (*curr).next;
                }
            }

            if !found {
                break;
            }

            addr = addr.min(buddy_physical);
            order += 1;
        }

        let block = (addr + self.hhdm_offset) as *mut FreeBlock;
        unsafe {
            (*block).next = self.free_lists[order];
        }
        self.free_lists[order] = block;
    }

    fn free_pages(&mut self, addr: u64, order: usize) {
        self.coalesce(addr, order);
    }
}

pub struct Pmm {
    buddy: BuddyAllocator,
    total_usable_pages: usize,
    used_pages: usize,
}

impl Pmm {
    const fn new() -> Self {
        Self {
            buddy: BuddyAllocator::new(),
            total_usable_pages: 0,
            used_pages: 0,
        }
    }

    pub fn initialize(&mut self) {
        let hhdm = HHDM.response().expect("HHDM response missing");
        let memmap = MEMMAP.response().expect("memmap response missing");

        self.buddy.hhdm_offset = hhdm.offset;

        for entry in memmap.entries() {
            if entry.type_ == MEMMAP_USABLE {
                self.buddy.add_region(entry.base, entry.length);
                self.total_usable_pages += (entry.length / PAGE_SIZE) as usize;
            }
        }
    }

    pub fn allocate_frame(&mut self) -> Option<u64> {
        let frame = self.buddy.allocate_pages(0);
        if frame != 0 {
            self.used_pages += 1;
            Some(frame)
        } else {
            None
        }
    }

    pub fn allocate_contiguous(&mut self, n: usize) -> Option<u64> {
        if n == 0 {
            return None;
        }

        let order = get_order(n as u64 * PAGE_SIZE);
        let frame = self.buddy.allocate_pages(order);

        if frame != 0 {
            self.used_pages += 1 << order;
            Some(frame)
        } else {
            None
        }
    }

    pub fn free_frame(&mut self, frame: u64) {
        self.buddy.free_pages(frame, 0);
        self.used_pages = self.used_pages.saturating_sub(1);
    }

    pub fn free_contiguous(&mut self, frame: u64, n: usize) {
        let order = get_order(n as u64 * MIN_BLOCK_SIZE);
        self.buddy.free_pages(frame, order);
        self.used_pages = self.used_pages.saturating_sub(1 << order);
    }

    pub fn total_memory(&self) -> usize {
        self.total_usable_pages * PAGE_SIZE as usize
    }

    pub fn used_memory(&self) -> usize {
        self.used_pages * PAGE_SIZE as usize
    }

    pub fn hhdm_offset(&self) -> u64 {
        self.buddy.hhdm_offset
    }
}

fn get_order(size: u64) -> usize {
    if size <= MIN_BLOCK_SIZE {
        return 0;
    }
    bit_length(size - 1) - bit_length(MIN_BLOCK_SIZE - 1)
}

fn bit_length(size: u64) -> usize {
    if size == 0 {
        return 0;
    }
    64 - size.leading_zeros() as usize
}

pub static PMM: Mutex<Pmm> = Mutex::new(Pmm::new());
