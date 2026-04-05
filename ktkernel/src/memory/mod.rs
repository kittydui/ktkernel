pub mod pmm;
pub mod vmm;
pub mod heap;

use pmm::PMM;
use vmm::VMM;
use heap::SLAB;

pub fn init() {
    PMM.lock().initialize();
    VMM.lock().initialize();
    SLAB.lock().initialize();
}
