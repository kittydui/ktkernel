#include "subsystems/acpi/rsdp.h"
#include "systems.h"

namespace KtCore
{
bool RSDP::initialize()
{
    KtCore::KPrint("ACPI Version {}\n", this->m_revision);

    // checksum validation

    return true;
}
} // namespace KtCore
