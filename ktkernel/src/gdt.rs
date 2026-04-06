use x86_64::structures::gdt::{GlobalDescriptorTable, Descriptor, SegmentSelector};
use x86_64::instructions::segmentation::{CS, Segment, DS, ES, FS, GS, SS};
use spin::Once;

static GDT: Once<(GlobalDescriptorTable, Selectors)> = Once::new();

struct Selectors {
    code_selector: SegmentSelector,
    data_selector: SegmentSelector,
}

pub fn setup_gdt() {
    let (gdt, selectors) = GDT.call_once(|| {
        let mut gdt = GlobalDescriptorTable::empty();
        let code_selector = gdt.append(Descriptor::kernel_code_segment());
        let data_selector = gdt.append(Descriptor::kernel_data_segment());
        gdt.append(Descriptor::user_code_segment());
        gdt.append(Descriptor::user_data_segment());
        (gdt, Selectors { code_selector, data_selector })
    });
    gdt.load();

    unsafe {
        DS::set_reg(selectors.data_selector);
        ES::set_reg(selectors.data_selector);
        FS::set_reg(selectors.data_selector);
        GS::set_reg(selectors.data_selector);
        SS::set_reg(selectors.data_selector);

        CS::set_reg(selectors.code_selector);
    }
}