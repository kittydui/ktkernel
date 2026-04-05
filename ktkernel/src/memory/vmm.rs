use spin::Mutex;

use super::pmm::PMM;
use crate::limine::HHDM;
use crate::utils::intrin::{invlpg, read_cr3, write_cr3};

pub mod flags {
    pub const PRESENT: u64 = 1 << 0;
    pub const WRITABLE: u64 = 1 << 1;
    pub const USER: u64 = 1 << 2;
    pub const WRITE_THROUGH: u64 = 1 << 3;
    pub const CACHE_DISABLE: u64 = 1 << 4;
    pub const ACCESSED: u64 = 1 << 5;
    pub const DIRTY: u64 = 1 << 6;
    pub const HUGE_PAGE: u64 = 1 << 7;
    pub const GLOBAL: u64 = 1 << 8;
    pub const NX: u64 = 1 << 63;
    pub const ADDRESS_MASK: u64 = 0x000F_FFFF_FFFF_F000;
}

pub struct Vmm {
    pml4_physical: u64,
    hhdm_offset: u64,
}

unsafe impl Send for Vmm {}

impl Vmm {
    const fn new() -> Self {
        Self {
            pml4_physical: 0,
            hhdm_offset: 0,
        }
    }

    pub fn initialize(&mut self) {
        let hhdm = HHDM.response().expect("HHDM response missing");
        self.hhdm_offset = hhdm.offset;
        self.pml4_physical = read_cr3() & flags::ADDRESS_MASK;
    }

    fn get_pte(&self, virt: u64, create: bool) -> Option<*mut u64> {
        let mut table = (self.pml4_physical + self.hhdm_offset) as *mut u64;

        for level in (1..=3).rev() {
            let index = ((virt >> (12 + 9 * level)) & 0x1FF) as usize;

            unsafe {
                let entry = table.add(index);
                if (*entry & flags::PRESENT) == 0 {
                    if !create {
                        return None;
                    }

                    let frame = PMM.lock().allocate_frame()?;
                    let page = (frame + self.hhdm_offset) as *mut u8;
                    core::ptr::write_bytes(page, 0, 4096);

                    *entry = frame | flags::PRESENT | flags::WRITABLE;
                }
                table = ((*entry & flags::ADDRESS_MASK) + self.hhdm_offset) as *mut u64;
            }
        }

        let index = ((virt >> 12) & 0x1FF) as usize;
        Some(unsafe { table.add(index) })
    }

    pub fn map(&self, virt: u64, phys: u64, page_flags: u64) -> bool {
        match self.get_pte(virt, true) {
            Some(pte) => unsafe {
                *pte = (phys & flags::ADDRESS_MASK) | page_flags | flags::PRESENT;
                true
            },
            None => false,
        }
    }

    pub fn unmap(&self, virt: u64) {
        if let Some(pte) = self.get_pte(virt, false) {
            unsafe { *pte = 0 };
            invlpg(virt);
        }
    }

    pub fn virtual_to_physical(&self, virt: u64) -> Option<u64> {
        let pte = self.get_pte(virt, false)?;
        unsafe {
            if (*pte & flags::PRESENT) == 0 {
                return None;
            }
            Some((*pte & flags::ADDRESS_MASK) | (virt & 0xFFF))
        }
    }

    pub fn physical_to_virtual(&self, phys: u64) -> u64 {
        phys + self.hhdm_offset
    }

    pub fn activate(&self) {
        write_cr3(self.pml4_physical);
    }
}

pub static VMM: Mutex<Vmm> = Mutex::new(Vmm::new());
