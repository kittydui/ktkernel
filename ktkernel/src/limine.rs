use limine::{RequestsStartMarker, BaseRevision, RequestsEndMarker, request::*};

#[used]
#[unsafe(link_section = ".requests_start")]
pub static REQUESTS_START: RequestsStartMarker = RequestsStartMarker::new();

#[unsafe(link_section = ".requests")]
pub static BASE_REVISION: BaseRevision = BaseRevision::new();
#[unsafe(link_section = ".requests")]
pub static FRAMEBUFFER: FramebufferRequest = FramebufferRequest::new();
#[unsafe(link_section = ".requests")]
pub static MEMMAP: MemmapRequest = MemmapRequest::new();
#[unsafe(link_section = ".requests")]
pub static BOOTLOADER: BootloaderInfoRequest = BootloaderInfoRequest::new();
#[unsafe(link_section = ".requests")]
pub static FIRMWARE: FirmwareTypeRequest = FirmwareTypeRequest::new();
#[unsafe(link_section = ".requests")]
pub static DATE: DateAtBootRequest = DateAtBootRequest::new();
#[unsafe(link_section = ".requests")]
pub static BOOT_TIME: BootloaderPerformanceRequest = BootloaderPerformanceRequest::new();
#[unsafe(link_section = ".requests")]
pub static HHDM: HhdmRequest = HhdmRequest::new();
#[unsafe(link_section = ".requests")]
pub static EXEC_ADDR: ExecutableAddressRequest = ExecutableAddressRequest::new();
#[unsafe(link_section = ".requests")]
pub static EXEC_FILE: ExecutableFileRequest = ExecutableFileRequest::new();
#[unsafe(link_section = ".requests")]
pub static EXEC_CMDLINE: ExecutableCmdlineRequest = ExecutableCmdlineRequest::new();
#[unsafe(link_section = ".requests")]
pub static DTB: DtbRequest = DtbRequest::new();
#[unsafe(link_section = ".requests")]
pub static RSDP: RsdpRequest = RsdpRequest::new();
#[unsafe(link_section = ".requests")]
pub static MP: MpRequest = MpRequest::new(0);
#[cfg(target_arch = "riscv64")]
#[unsafe(link_section = ".requests")]
pub static BSP_HARTID: BspHartidRequest = BspHartidRequest::new();
#[unsafe(link_section = ".requests")]
pub static MODULES: ModulesRequest = ModulesRequest::new();
#[cfg(target_arch = "x86_64")]
#[unsafe(link_section = ".requests")]
pub static KEEP_IOMMU: KeepIommuRequest = KeepIommuRequest::new();
#[unsafe(link_section = ".requests")]
pub static STACK: StackSizeRequest = StackSizeRequest::new(65536);
#[unsafe(link_section = ".requests")]
pub static PAGING: PagingModeRequest = PagingModeRequest::PREFER_MAXIMUM;
#[unsafe(link_section = ".requests")]
pub static SMBIOS: SmbiosRequest = SmbiosRequest::new();
#[unsafe(link_section = ".requests")]
pub static EFI: EfiRequest = EfiRequest::new();
#[unsafe(link_section = ".requests")]
pub static EFI_MEMMAP: EfiMemmapRequest = EfiMemmapRequest::new();

#[used]
#[unsafe(link_section = ".requests_end")]
pub static REQUESTS_END: RequestsEndMarker = RequestsEndMarker::new();
